cmake_minimum_required(VERSION 3.23)

project(log-file-indexing)

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

add_executable(main)

target_sources(main
    PRIVATE
        src/main.cxx
)

target_link_libraries(main
    PRIVATE
    tokenize
    InvertedIndex
    search_naive
    search_kmp
    search_horspool
    search_boyer_moore
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