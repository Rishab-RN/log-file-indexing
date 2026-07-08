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
    "Token Search (inverted index)"
};

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
      has_loaded_file(false)
{
    file_path[0] = '\0';
    query[0] = '\0';
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
    else
    {
        ImGui::TextDisabled(
            "Multiple words are AND-ed together (all tokens must be present).");
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