#pragma once

#include <string>
#include <vector>

#include "SearchEngine.hxx"

struct SDL_Window;
typedef void* SDL_GLContext;

enum class SearchMode
{
    Pattern,
    InvertedIndex,
    TimeRange
};

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
    int search_mode_index;

    double last_load_time_ms;
    double last_search_time_ms;

    bool has_loaded_file;
    std::string loaded_file_name;

    std::string dialog_error;

    char ts_regex[256];
    char ts_format[128];
    char time_from[64];
    char time_to[64];
    bool ts_index_built;
    std::string ts_error;
    bool ts_error_is_ok;
    int  preset_index;

private:
    bool init();
    void shutdown();

    void process_events();
    void render();

    void draw_menu_bar();
    void draw_load_panel();
    void draw_search_panel();
    void draw_results_panel();
    void draw_range_panel();

    void open_native_file_dialog();
    void load_file(const std::string& path);
    void run_search();

public:
    Application();
    ~Application();

    int run();
};