#include "Application.hxx"

#ifdef _WIN32
#include <windows.h>
#include <commdlg.h>
#define popen _popen
#define pclose _pclose
#elif defined(__linux__)
// Linux relies on the existing run_command implementation
#endif

#include <SDL.h>
#include <SDL_opengl.h>

#include <chrono>
#include <cstdio>
#include <array>
#include <sstream>
#include <algorithm>

#include "imgui.h"
#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_opengl3.h"

static const char* ALGORITHM_NAMES[] =
{
    "Naive",
    "KMP",
    "Horspool",
    "Boyer-Moore"
};

static const char* SEARCH_MODE_NAMES[] =
{
    "Pattern Search (substring)",
    "Token Search (inverted index)",
    "Time Range Query"
};

/**
 *  LogPreset
 *  A named, ready-to-use timestamp regex + strftime format pair.
 *  format is empty ("") for Unix-epoch-integer captures.
 */
struct LogPreset
{
    const char* name;
    const char* regex;
    const char* format;
    const char* hint;
};

static const LogPreset LOG_PRESETS[] =
{
    // Apache
    { "Apache Error Log",
      "\\[[A-Za-z]{3} ([A-Za-z]{3}\\s+\\d{1,2} \\d{2}:\\d{2}:\\d{2} \\d{4})\\]",
      "%b %d %H:%M:%S %Y",
      "[Thu Jun 09 06:07:04 2005]" },

    { "Apache / Nginx Access Log",
      "\\[(\\d{2}/[A-Za-z]{3}/\\d{4}:\\d{2}:\\d{2}:\\d{2})",
      "%d/%b/%Y:%H:%M:%S",
      "[10/Oct/2000:13:55:36 -0700]" },

    // ISO / Datetime variants
    { "ISO 8601 with T",
      "(\\d{4}-\\d{2}-\\d{2}T\\d{2}:\\d{2}:\\d{2})",
      "%Y-%m-%dT%H:%M:%S",
      "2024-01-15T08:00:00  (Docker, K8s, AWS, GCP)" },

    { "Datetime with space",
      "(\\d{4}-\\d{2}-\\d{2} \\d{2}:\\d{2}:\\d{2})",
      "%Y-%m-%d %H:%M:%S",
      "2024-01-15 08:00:00  (OpenStack, Windows, HPC, Hadoop, Log4j, Zookeeper, Python)" },

    // Syslog family
    { "Syslog / Linux / Mac / OpenSSH / Cisco",
      "([A-Za-z]{3}\\s+\\d{1,2} \\d{2}:\\d{2}:\\d{2})",
      "%b %d %H:%M:%S",
      "Jan 15 08:00:00  or  Dec  9 07:07:38  (no year, defaults to 1970)" },

    // Nginx
    { "Nginx Error Log",
      "(\\d{4}/\\d{2}/\\d{2} \\d{2}:\\d{2}:\\d{2})",
      "%Y/%m/%d %H:%M:%S",
      "2024/01/15 08:00:00" },

    // Apache Spark
    { "Apache Spark",
      "(\\d{2}/\\d{2}/\\d{2} \\d{2}:\\d{2}:\\d{2})",
      "%y/%m/%d %H:%M:%S",
      "15/10/18 18:01:22" },

    // HDFS
    { "HDFS / NameNode",
      "(\\d{6} \\d{6})",
      "%m%d%y %H%M%S",
      "081109 203518  (MMDDYY HHMMSS)" },

    // MySQL
    { "MySQL Error Log",
      "(\\d{6} \\d{2}:\\d{2}:\\d{2})",
      "%y%m%d %H:%M:%S",
      "151020 19:30:06  (YYMMDD)" },

    // Android
    { "Android Logcat",
      "(\\d{2}-\\d{2} \\d{2}:\\d{2}:\\d{2})",
      "%m-%d %H:%M:%S",
      "01-03 22:00:00  (no year, defaults to 1970)" },

    // HealthApp
    { "HealthApp",
      "(\\d{8}-\\d{2}:\\d{2}:\\d{2})",
      "%Y%m%d-%H:%M:%S",
      "20171223-22:15:45" },

    // Proxifier
    { "Proxifier",
      "\\[(\\d{2}\\.\\d{2} \\d{2}:\\d{2}:\\d{2})\\]",
      "%d.%m %H:%M:%S",
      "[03.07 18:22:33]  (DD.MM, no year)" },

    // Unix Epoch (BGL, Thunderbird)
    { "Unix Epoch (seconds)",
      "(\\d{9,10})(?!\\d)",
      "",
      "1117623909  (BGL, Thunderbird)  -- format field is ignored" },
};

static const int NUM_PRESETS = (int)(sizeof(LOG_PRESETS) / sizeof(LOG_PRESETS[0]));

/**
 *  preset_getter
 *  ImGui Combo callback: returns the display name for preset at index idx.
 */
static const char* preset_getter(void* data, int idx)
{
    return ((const LogPreset*)data)[idx].name;
}

/**
 *  run_command
 *  Runs a shell command and returns its trimmed stdout.
 *  Used to shell out to powershell.exe / wslpath on WSL.
 */
static std::string run_command(const std::string& cmd)
{
    std::array<char, 512> buffer;
    std::string result;

    FILE* pipe = popen(cmd.c_str(), "r");

    if(!pipe)
        return result;

    while(fgets(buffer.data(), (int)buffer.size(), pipe) != nullptr)
    {
        result += buffer.data();
    }

    pclose(pipe);

    while(!result.empty() &&
          (result.back() == '\n' || result.back() == '\r' || result.back() == ' '))
    {
        result.pop_back();
    }

    return result;
}

Application::Application()
    : window(nullptr),
      gl_context(nullptr),
      running(true),
      algorithm_index(0),
      search_mode_index(0),
      last_load_time_ms(0.0),
      last_search_time_ms(0.0),
      has_loaded_file(false),
      ts_index_built(false),
      ts_error_is_ok(false),
      preset_index(0)
{
    file_path[0] = '\0';
    query[0]     = '\0';
    ts_regex[0]  = '\0';
    ts_format[0] = '\0';
    time_from[0] = '\0';
    time_to[0]   = '\0';
}

Application::~Application()
{
    shutdown();
}

bool Application::init()
{
    if(SDL_Init(SDL_INIT_VIDEO) != 0)
        return false;

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

    window = SDL_CreateWindow(
        "Log Search",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        1200,
        800,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

    if(!window)
        return false;

    gl_context = SDL_GL_CreateContext(window);

    IMGUI_CHECKVERSION();

    ImGui::CreateContext();

    ImGui::StyleColorsLight();

    ImGuiStyle& style = ImGui::GetStyle();

    style.WindowRounding = 4.0f;
    style.FrameRounding = 4.0f;
    style.ScrollbarRounding = 4.0f;
    style.GrabRounding = 4.0f;
    style.WindowPadding = ImVec2(10, 10);
    style.FramePadding = ImVec2(6, 4);
    style.ItemSpacing = ImVec2(8, 8);

    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init("#version 330");

    return true;
}

void Application::shutdown()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();

    ImGui::DestroyContext();

    if(gl_context)
        SDL_GL_DeleteContext(gl_context);

    if(window)
        SDL_DestroyWindow(window);

    SDL_Quit();
}

void Application::process_events()
{
    SDL_Event event;

    while(SDL_PollEvent(&event))
    {
        ImGui_ImplSDL2_ProcessEvent(&event);

        if(event.type == SDL_QUIT)
        {
            running = false;
        }
    }
}

void Application::draw_menu_bar()
{
    if(ImGui::BeginMainMenuBar())
    {
        if(ImGui::BeginMenu("File"))
        {
            if(ImGui::MenuItem("Open..."))
            {
                open_native_file_dialog();
            }

            ImGui::Separator();

            if(ImGui::MenuItem("Exit"))
            {
                running = false;
            }

            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
}

/**
 *  open_native_file_dialog
 *  Shells out to the native Windows file picker through powershell.exe,
 *  then converts the returned Windows path to a WSL path with wslpath.
 *  This avoids walking the filesystem ourselves (and the segfault that
 *  came with it) and gives a real Explorer window instead.
 */
void Application::open_native_file_dialog()
{
    dialog_error.clear();
    std::string selected_path;

#ifdef _WIN32
    // --- NATIVE WINDOWS IMPLEMENTATION ---
    char filename[MAX_PATH] = {0};
    
    OPENFILENAMEA ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL; // If you have a GLFW window handle, you can cast it here
    
    // Windows requires filters to be separated by null characters (\0)
    ofn.lpstrFilter = "Log/Text files (*.log;*.txt)\0*.log;*.txt\0All files (*.*)\0*.*\0";
    ofn.lpstrFile = filename;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrTitle = "Select a log file";
    ofn.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST;

    if (GetOpenFileNameA(&ofn))
    {
        selected_path = filename;
    }

#elif defined(__linux__)
    // --- NATIVE LINUX IMPLEMENTATION ---
    // Zenity is the standard GTK dialog tool shipped with most Linux desktop environments.
    std::string command = 
        "zenity --file-selection --title=\"Select a log file\" "
        "--file-filter=\"Log/Text files | *.log *.txt\" "
        "--file-filter=\"All files | *\" 2>/dev/null";
    
    selected_path = run_command(command);
    
    if (!selected_path.empty())
    {
        // Strip trailing newline from zenity output
        selected_path.erase(selected_path.find_last_not_of(" \n\r\t") + 1);
    }
#endif

    if (selected_path.empty())
    {
        // User cancelled the dialog, or the command failed.
        return; 
    }

    // Safely copy the selected path into your existing file_path char array
    std::snprintf(
        file_path,
        sizeof(file_path),
        "%s",
        selected_path.c_str());

    load_file(file_path);
}

void Application::load_file(const std::string& path)
{
    if(path.empty())
        return;

    auto start = std::chrono::high_resolution_clock::now();

    engine.load(path);

    auto end = std::chrono::high_resolution_clock::now();

    last_load_time_ms =
        std::chrono::duration<double, std::milli>(end - start).count();

    has_loaded_file = true;
    loaded_file_name = path;

    size_t slash = loaded_file_name.find_last_of("/\\");

    if(slash != std::string::npos)
    {
        loaded_file_name = loaded_file_name.substr(slash + 1);
    }

    results.clear();
    suggestions.clear();
}

void Application::draw_load_panel()
{
    ImGui::SeparatorText("Log File");

    if(ImGui::Button("Browse... (opens Windows Explorer)", ImVec2(280, 0)))
    {
        open_native_file_dialog();
    }

    ImGui::SameLine();

    if(ImGui::Button("Load"))
    {
        load_file(file_path);
    }

    ImGui::InputText(
        "Path",
        file_path,
        sizeof(file_path));

    if(!dialog_error.empty())
    {
        ImGui::TextColored(
            ImVec4(0.8f, 0.1f, 0.1f, 1.0f),
            "%s",
            dialog_error.c_str());
    }

    if(has_loaded_file)
    {
        ImGui::TextColored(
            ImVec4(0.1f, 0.5f, 0.1f, 1.0f),
            "Loaded \"%s\" (%zu lines) in %.3f ms",
            loaded_file_name.c_str(),
            engine.get_logs().get_all_logs().size(),
            last_load_time_ms);
    }
    else
    {
        ImGui::TextDisabled("No file loaded yet.");
    }
}

/**
 *  run_search
 *  Dispatches to either the inverted-index token search (single token
 *  -> search_token, multiple whitespace-separated tokens -> search_and)
 *  or to one of the raw string-matching algorithms over the full text.
 */
void Application::run_search()
{
    auto start = std::chrono::high_resolution_clock::now();

    SearchMode mode = (search_mode_index == 0)
        ? SearchMode::Pattern
        : SearchMode::InvertedIndex;

    if(mode == SearchMode::Pattern)
    {
        StringSearchAlgorithm algo;

        switch(algorithm_index)
        {
            case 0: algo = StringSearchAlgorithm::Naive; break;
            case 1: algo = StringSearchAlgorithm::KMP; break;
            case 2: algo = StringSearchAlgorithm::Horspool; break;
            default: algo = StringSearchAlgorithm::BoyerMoore; break;
        }

        results = engine.search_text(query, algo);
    }
    else
    {
        std::vector<std::string> tokens;
        std::istringstream stream(query);
        std::string token;

        while(stream >> token)
        {
            tokens.push_back(token);
        }

        if(tokens.empty())
        {
            results.clear();
        }
        else if(tokens.size() == 1)
        {
            results = engine.search_token(tokens.front());
        }
        else
        {
            results = engine.search_and(tokens);
        }
    }

    auto end = std::chrono::high_resolution_clock::now();

    last_search_time_ms =
        std::chrono::duration<double, std::milli>(end - start).count();
}

void Application::draw_search_panel()
{
    ImGui::SeparatorText("Search");

    if(!has_loaded_file)
        ImGui::BeginDisabled();

    ImGui::TextUnformatted("Mode:");
    ImGui::SameLine();

    ImGui::SetNextItemWidth(260);

    ImGui::Combo(
        "##SearchMode",
        &search_mode_index,
        SEARCH_MODE_NAMES,
        IM_ARRAYSIZE(SEARCH_MODE_NAMES));

    if(search_mode_index == 0)
    {
        ImGui::SameLine();

        ImGui::TextUnformatted("Algorithm:");
        ImGui::SameLine();

        ImGui::SetNextItemWidth(160);

        ImGui::Combo(
            "##Algorithm",
            &algorithm_index,
            ALGORITHM_NAMES,
            IM_ARRAYSIZE(ALGORITHM_NAMES));
    }
    else if(search_mode_index == 1)
    {
        ImGui::TextDisabled(
            "Multiple words are AND-ed together (all tokens must be present).");
    }

    // Time Range mode: end the outer disable group and delegate entirely.
    if(search_mode_index == 2)
    {
        if(!has_loaded_file)
            ImGui::EndDisabled();

        draw_range_panel();
        return;
    }

    if(ImGui::InputText(
        "Query",
        query,
        sizeof(query)))
    {
        suggestions = engine.autocomplete(query);
    }

    ImGui::SameLine();

    if(ImGui::Button("Search"))
    {
        run_search();
    }

    if(!suggestions.empty())
    {
        ImGui::TextUnformatted("Suggestions:");

        ImGui::BeginChild(
            "Suggestions",
            ImVec2(0, 100),
            true);

        for(const std::string& word : suggestions)
        {
            if(ImGui::Selectable(word.c_str()))
            {
                std::snprintf(
                    query,
                    sizeof(query),
                    "%s",
                    word.c_str());

                suggestions = engine.autocomplete(query);
            }
        }

        ImGui::EndChild();
    }

    if(!has_loaded_file)
        ImGui::EndDisabled();
}

void Application::draw_results_panel()
{
    ImGui::SeparatorText("Results");

    if(has_loaded_file && !query[0])
    {
        ImGui::TextDisabled("Type a query and press Search.");
    }

    ImGui::Text(
        "%zu match(es) found in %.3f ms",
        results.size(),
        last_search_time_ms);

    ImGui::BeginChild(
        "Results",
        ImVec2(0, 0),
        true);

    // 1. Update to auto (or const std::vector<std::string_view>&)
    const auto& all_logs = engine.get_logs().get_all_logs();

    std::vector<size_t> sorted_results = results;
    std::sort(sorted_results.begin(), sorted_results.end());

    for(size_t id : sorted_results)
    {
        size_t line_number = id + 1;

        if(id < all_logs.size())
        {
            ImGui::Text(
                "Line %zu:",
                line_number);

            ImGui::SameLine();

            // 2. Safely print the string_view using %.*s
            // We pass the length as an int, followed by the raw data pointer.
            ImGui::TextWrapped(
                "%.*s", 
                static_cast<int>(all_logs[id].length()), 
                all_logs[id].data());
        }
        else
        {
            ImGui::Text("Line %zu", line_number);
        }
    }

    ImGui::EndChild();
}

void Application::render()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();

    ImGui::NewFrame();

    draw_menu_bar();

    ImGuiViewport* viewport = ImGui::GetMainViewport();

    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);

    ImGui::Begin(
        "Search Engine",
        nullptr,
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove);

    draw_load_panel();

    ImGui::Spacing();

    draw_search_panel();

    ImGui::Spacing();

    draw_results_panel();

    ImGui::End();

    ImGui::Render();

    glViewport(
        0,
        0,
        (int)ImGui::GetIO().DisplaySize.x,
        (int)ImGui::GetIO().DisplaySize.y);

    glClearColor(0.95f, 0.95f, 0.95f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    ImGui_ImplOpenGL3_RenderDrawData(
        ImGui::GetDrawData());

    SDL_GL_SwapWindow(window);
}

int Application::run()
{
    if(!init())
        return 1;

    while(running)
    {
        process_events();
        render();
    }

    return 0;
}

void Application::draw_range_panel()
{
    ImGui::SeparatorText("Timestamp Index");

    if(!has_loaded_file)
        ImGui::BeginDisabled();

    // Preset combo — auto-applies on selection change.
    ImGui::SetNextItemWidth(340);

    if(ImGui::Combo(
        "##preset",
        &preset_index,
        preset_getter,
        (void*)LOG_PRESETS,
        NUM_PRESETS))
    {
        // Selection changed: immediately update regex + format and invalidate
        // any previously built index so stale results cannot be returned.
        std::snprintf(ts_regex,  sizeof(ts_regex),  "%s", LOG_PRESETS[preset_index].regex);
        std::snprintf(ts_format, sizeof(ts_format), "%s", LOG_PRESETS[preset_index].format);
        ts_index_built = false;
        ts_error.clear();
    }

    ImGui::SameLine();

    if(ImGui::Button("Reset##loadpreset"))
    {
        // Explicit reset: useful after manually editing the regex/format fields.
        std::snprintf(ts_regex,  sizeof(ts_regex),  "%s", LOG_PRESETS[preset_index].regex);
        std::snprintf(ts_format, sizeof(ts_format), "%s", LOG_PRESETS[preset_index].format);
        ts_index_built = false;
        ts_error.clear();
    }

    ImGui::SameLine();

    if(ImGui::Button("Auto-Detect"))
    {
        const auto& all_logs = engine.get_logs().get_all_logs();
        size_t scan_limit = std::min(all_logs.size(), (size_t)200);

        int best_preset = -1;
        int best_count  = 0;

        for(int p = 0; p < NUM_PRESETS; ++p)
        {
            std::regex re;

            try { re = std::regex(LOG_PRESETS[p].regex); }
            catch(...) { continue; }

            int count = 0;
            std::smatch match;

            for(size_t i = 0; i < scan_limit; ++i)
            {
                std::string line(all_logs[i]);

                if(std::regex_search(line, match, re) && match.size() >= 2)
                    ++count;
            }

            if(count > best_count)
            {
                best_count  = count;
                best_preset = p;
            }
        }

        if(best_preset >= 0 && best_count > 0)
        {
            preset_index = best_preset;
            std::snprintf(ts_regex,  sizeof(ts_regex),  "%s", LOG_PRESETS[best_preset].regex);
            std::snprintf(ts_format, sizeof(ts_format), "%s", LOG_PRESETS[best_preset].format);

            ts_error      = std::string("Auto-detected: ") + LOG_PRESETS[best_preset].name +
                            "  (" + std::to_string(best_count) + "/" +
                            std::to_string(scan_limit) + " lines matched)";
            ts_error_is_ok = true;
        }
        else
        {
            ts_error       = "Auto-detect found no matching format. Select a preset manually.";
            ts_error_is_ok = false;
        }
    }

    if(ImGui::IsItemHovered() && preset_index >= 0 && preset_index < NUM_PRESETS)
        ImGui::SetTooltip("Example: %s", LOG_PRESETS[preset_index].hint);

    ImGui::SetNextItemWidth(400);
    ImGui::InputText("Regex (capture group 1)##tsregex", ts_regex, sizeof(ts_regex));

    ImGui::SetNextItemWidth(260);
    ImGui::InputText("Format string##tsformat",          ts_format, sizeof(ts_format));

    ImGui::SameLine();

    if(ImGui::Button("Build Index"))
    {
        ts_error.clear();
        engine.set_timestamp_regex(ts_regex, ts_format);
        ts_index_built = engine.has_timestamp_index();

        if(ts_index_built)
        {
            ts_error       = std::string("Index built: ") +
                             std::to_string(engine.timestamp_count()) + " lines indexed.";
            ts_error_is_ok = true;
        }
        else
        {
            ts_error       = "No lines matched the regex. Check the regex and format string.";
            ts_error_is_ok = false;
        }
    }

    if(ts_index_built)
    {
        ImGui::TextDisabled("Ready. Enter a time range below and click Search.");
    }
    else
    {
        ImGui::TextDisabled("Select a preset, click Load, then click Build Index.");
    }

    ImGui::Spacing();
    ImGui::SeparatorText("Time Range Query");

    if(!ts_index_built)
        ImGui::BeginDisabled();

    ImGui::SetNextItemWidth(220);
    ImGui::InputText("From##timefrom", time_from, sizeof(time_from));

    ImGui::SameLine();

    ImGui::SetNextItemWidth(220);
    ImGui::InputText("To##timeto", time_to, sizeof(time_to));

    ImGui::SameLine();

    if(ImGui::Button("Search##rangesearch"))
    {
        auto t1 = std::chrono::high_resolution_clock::now();

        time_t t_start = parse_timestamp(time_from, ts_format);
        time_t t_end   = parse_timestamp(time_to,   ts_format);

        if(t_start == static_cast<time_t>(-1) || t_end == static_cast<time_t>(-1))
        {
            ts_error       = "Could not parse timestamp. Use the same format as the format string.";
            ts_error_is_ok = false;
        }
        else
        {
            ts_error.clear();
            results = engine.range_query(t_start, t_end);
        }

        auto t2 = std::chrono::high_resolution_clock::now();
        last_search_time_ms = std::chrono::duration<double, std::milli>(t2 - t1).count();
    }

    if(!ts_index_built)
        ImGui::EndDisabled();

    ImGui::TextDisabled("Enter timestamps in the same format as the format string above.");

    if(!ts_error.empty())
    {
        ImVec4 color = ts_error_is_ok
            ? ImVec4(0.1f, 0.5f, 0.1f, 1.0f)
            : ImVec4(0.8f, 0.1f, 0.1f, 1.0f);

        ImGui::TextColored(color, "%s", ts_error.c_str());
    }

    if(!has_loaded_file)
        ImGui::EndDisabled();
}