#pragma once

#include <string>
#include <vector>
#include <filesystem>

#include "SearchEngine.hxx"

struct SDL_Window;
typedef void* SDL_GLContext;

class Application
{
private:
    SearchEngine engine;

    SDL_Window* window;
    SDL_GLContext gl_context;

    bool running;

    char file_path[512];
    char query[256];

    std::vector<size_t> results;
    std::vector<std::string> suggestions;

    int algorithm_index;

    double last_load_time_ms;
    double last_search_time_ms;

    bool has_loaded_file;
    std::string loaded_file_name;

    bool show_file_browser;
    std::filesystem::path browse_path;
    std::vector<std::filesystem::directory_entry> browse_entries;
    std::string browse_error;

private:
    bool init();
    void shutdown();

    void process_events();
    void render();

    void draw_menu_bar();
    void draw_load_panel();
    void draw_search_panel();
    void draw_results_panel();
    void draw_file_browser();

    void refresh_browse_entries();
    void load_file(const std::string& path);

public:
    Application();
    ~Application();

    int run();
};