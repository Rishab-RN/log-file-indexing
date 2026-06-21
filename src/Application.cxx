#include "Application.hxx"

#include <SDL.h>
#include <SDL_opengl.h>

#include <chrono>
#include <system_error>
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

Application::Application()
    : window(nullptr),
      gl_context(nullptr),
      running(true),
      algorithm_index(0),
      last_load_time_ms(0.0),
      last_search_time_ms(0.0),
      has_loaded_file(false),
      show_file_browser(false)
{
    file_path[0] = '\0';
    query[0] = '\0';

    std::error_code ec;
    browse_path = std::filesystem::current_path(ec);
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
                show_file_browser = true;
                refresh_browse_entries();
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

void Application::refresh_browse_entries()
{
    browse_entries.clear();
    browse_error.clear();

    std::error_code ec;

    for(const std::filesystem::directory_entry& entry :
        std::filesystem::directory_iterator(browse_path, ec))
    {
        browse_entries.push_back(entry);
    }

    if(ec)
    {
        browse_error = ec.message();
    }

    std::sort(
        browse_entries.begin(),
        browse_entries.end(),
        [](const std::filesystem::directory_entry& a,
           const std::filesystem::directory_entry& b)
        {
            bool a_dir = a.is_directory();
            bool b_dir = b.is_directory();

            if(a_dir != b_dir)
                return a_dir > b_dir;

            return a.path().filename().string() <
                   b.path().filename().string();
        });
}

void Application::draw_file_browser()
{
    if(!show_file_browser)
        return;

    ImGui::SetNextWindowSize(ImVec2(600, 500), ImGuiCond_FirstUseEver);

    if(ImGui::Begin("Open Log File", &show_file_browser))
    {
        ImGui::TextUnformatted(browse_path.string().c_str());

        ImGui::Separator();

        if(ImGui::Button(".. (Up)"))
        {
            if(browse_path.has_parent_path())
            {
                browse_path = browse_path.parent_path();
                refresh_browse_entries();
            }
        }

        ImGui::SameLine();

        if(ImGui::Button("Refresh"))
        {
            refresh_browse_entries();
        }

        if(!browse_error.empty())
        {
            ImGui::TextColored(
                ImVec4(0.8f, 0.1f, 0.1f, 1.0f),
                "%s",
                browse_error.c_str());
        }

        ImGui::BeginChild(
            "FileList",
            ImVec2(0, -40),
            true);

        for(const std::filesystem::directory_entry& entry : browse_entries)
        {
            std::string name = entry.path().filename().string();

            if(entry.is_directory())
            {
                std::string label = "[DIR] " + name;

                if(ImGui::Selectable(label.c_str()))
                {
                    browse_path = entry.path();
                    refresh_browse_entries();
                }
            }
            else
            {
                bool selected =
                    (name == std::filesystem::path(file_path).filename().string());

                if(ImGui::Selectable(name.c_str(), selected))
                {
                    std::filesystem::path full = entry.path();

                    std::snprintf(
                        file_path,
                        sizeof(file_path),
                        "%s",
                        full.string().c_str());
                }
            }
        }

        ImGui::EndChild();

        ImGui::Separator();

        ImGui::TextUnformatted("Selected:");
        ImGui::SameLine();
        ImGui::TextUnformatted(file_path);

        ImGui::SameLine(ImGui::GetWindowWidth() - 110);

        bool can_load = file_path[0] != '\0';

        if(!can_load)
            ImGui::BeginDisabled();

        if(ImGui::Button("Load", ImVec2(90, 0)))
        {
            load_file(file_path);
            show_file_browser = false;
        }

        if(!can_load)
            ImGui::EndDisabled();
    }

    ImGui::End();
}

void Application::load_file(const std::string& path)
{
    auto start = std::chrono::high_resolution_clock::now();

    engine.load(path);

    auto end = std::chrono::high_resolution_clock::now();

    last_load_time_ms =
        std::chrono::duration<double, std::milli>(end - start).count();

    has_loaded_file = true;
    loaded_file_name = std::filesystem::path(path).filename().string();

    results.clear();
    suggestions.clear();
}

void Application::draw_load_panel()
{
    ImGui::SeparatorText("Log File");

    if(ImGui::Button("Browse...", ImVec2(120, 0)))
    {
        show_file_browser = true;
        refresh_browse_entries();
    }

    ImGui::SameLine();

    ImGui::InputText(
        "Path",
        file_path,
        sizeof(file_path));

    ImGui::SameLine();

    bool can_load = file_path[0] != '\0';

    if(!can_load)
        ImGui::BeginDisabled();

    if(ImGui::Button("Load"))
    {
        load_file(file_path);
    }

    if(!can_load)
        ImGui::EndDisabled();

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

void Application::draw_search_panel()
{
    ImGui::SeparatorText("Search");

    if(!has_loaded_file)
        ImGui::BeginDisabled();

    ImGui::TextUnformatted("Algorithm:");
    ImGui::SameLine();

    ImGui::SetNextItemWidth(160);

    ImGui::Combo(
        "##Algorithm",
        &algorithm_index,
        ALGORITHM_NAMES,
        IM_ARRAYSIZE(ALGORITHM_NAMES));

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
        StringSearchAlgorithm algo;

        switch(algorithm_index)
        {
            case 0: algo = StringSearchAlgorithm::Naive; break;
            case 1: algo = StringSearchAlgorithm::KMP; break;
            case 2: algo = StringSearchAlgorithm::Horspool; break;
            default: algo = StringSearchAlgorithm::BoyerMoore; break;
        }

        auto start = std::chrono::high_resolution_clock::now();

        results = engine.search_text(query, algo);

        auto end = std::chrono::high_resolution_clock::now();

        last_search_time_ms =
            std::chrono::duration<double, std::milli>(end - start).count();
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

    const std::vector<std::string>& all_logs =
        engine.get_logs().get_all_logs();

    for(size_t id : results)
    {
        size_t line_number = id + 1;

        if(id < all_logs.size())
        {
            ImGui::Text(
                "Line %zu:",
                line_number);

            ImGui::SameLine();

            ImGui::TextWrapped("%s", all_logs[id].c_str());
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

    ImGui::SetNextWindowPos(
        ImVec2(viewport->WorkPos.x, viewport->WorkPos.y + 0));
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

    draw_file_browser();

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