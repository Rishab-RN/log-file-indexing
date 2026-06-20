cmake_minimum_required(VERSION 3.23)

project(log-file-indexing)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Release)

find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)

add_library(config INTERFACE)

target_sources(config INTERFACE
    INTERFACE
        FILE_SET HEADERS
        BASE_DIRS
            include
        FILES
            include/config.hxx
)

target_include_directories(config
    INTERFACE
        include
)

add_library(tokenize)

target_sources(tokenize
    PRIVATE
    src/tokenize.cxx

    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS
            include
        FILES
            include/tokenize.hxx
)

add_library(search_naive)

target_sources(search_naive
    PRIVATE
    src/search_naive.cxx
    
    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS
            include
        FILES
            include/search_naive.hxx
)

add_library(search_kmp)

target_sources(search_kmp
    PRIVATE
    src/search_kmp.cxx
    
    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS
            include
        FILES
            include/search_kmp.hxx
)

add_library(InvertedIndex)

target_sources(InvertedIndex
    PRIVATE
    src/InvertedIndex.cxx

    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS
            include
        FILES
            include/InvertedIndex.hxx
)

add_library(search_horspool)

target_sources(search_horspool
    PRIVATE
    src/search_horspool.cxx
    
    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS
            include
        FILES
            include/search_horspool.hxx
)

add_library(search_boyer_moore)

target_sources(search_boyer_moore
    PRIVATE
    src/search_boyer_moore.cxx
    
    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS
            include
        FILES
            include/search_boyer_moore.hxx
)

add_library(Trie)

target_sources(Trie
    PRIVATE
    src/Trie.cxx

    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS
            include
        FILES
            include/Trie.hxx
)

add_library(LogFile)

target_sources(LogFile
    PRIVATE
        src/LogFile.cxx

    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS
            include
        FILES
            include/LogFile.hxx
)

add_library(SearchEngine)

target_sources(SearchEngine
    PRIVATE
        src/SearchEngine.cxx

    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS
            include
        FILES
            include/SearchEngine.hxx
)

add_executable(main)

target_sources(main
    PRIVATE
        src/main.cxx
        external/imgui/imgui.cpp
        external/imgui/imgui_draw.cpp
        external/imgui/imgui_tables.cpp
        external/imgui/imgui_widgets.cpp

        external/imgui/backends/imgui_impl_sdl2.cpp
        external/imgui/backends/imgui_impl_opengl3.cpp
)

target_include_directories(main
    PRIVATE
        include

        external/imgui
        external/imgui/backends

        ${SDL2_INCLUDE_DIRS}
)

target_link_libraries(main 
    PRIVATE
        SDL2::SDL2
        OpenGL::GL
)

target_link_libraries(main
    PRIVATE
    SearchEngine
)

target_link_libraries(InvertedIndex
    PRIVATE
    config
)

target_link_libraries(search_horspool 
    PRIVATE 
    config
)

target_link_libraries(search_boyer_moore 
    PRIVATE 
    config
)

target_link_libraries(SearchEngine
    PRIVATE
    LogFile
    InvertedIndex
    Trie
    tokenize
)