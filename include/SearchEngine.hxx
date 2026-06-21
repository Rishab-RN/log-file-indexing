#pragma once

#include <fstream>

#include "config.hxx"

#include "LogFile.hxx"
#include "InvertedIndex.hxx"
#include "Trie.hxx"

#include "tokenize.hxx"
#include "search_naive.hxx"
#include "search_kmp.hxx"
#include "search_horspool.hxx"
#include "search_boyer_moore.hxx"

enum class StringSearchAlgorithm
{
    Naive,
    KMP,
    Horspool,
    BoyerMoore
};

class SearchEngine
{
private:
    LogFile logs;
    InvertedIndex index;
    Trie trie;

public:

    /**
     *  load
     *  Reads a file and generates the data structures.
     */
    void load(const std::string& file);

    /**
     *  search_token
     *  Finds all logs containing a token.
     */
    const std::vector<size_t>& search_token(const std::string& token);

    /**
     *  search_and
     *  Finds all logs containing all tokens.
     */
    const std::vector<size_t> search_and(const std::vector<std::string>& tokens);

    /**
     *  autocomplete
     *  Find autocomplete results for a prefix.
     */
    std::vector<std::string> autocomplete(const std::string& prefix);

    void print_results(const std::vector<size_t>& ids);

    std::vector<size_t> search_text(const std::string& pattern, StringSearchAlgorithm algorithm);

    const LogFile& get_logs() const
    {
        return logs;
    }

};

