#pragma once

#include <fstream>
#include <string>
#include <vector>
#include <memory>
#include <regex>

#include "config.hxx"

#include "LogFile.hxx"
#include "InvertedIndex.hxx"
#include "Trie.hxx"
#include "SegmentTree.hxx"

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
    SegmentTree seg_tree;

    std::string ts_regex_str;
    std::string ts_format_str;

public:
    void load(const std::string& file);

    const std::vector<size_t> search_token(const std::string& token);
    const std::vector<size_t> search_and(const std::vector<std::string>& tokens);
    std::vector<std::string> autocomplete(const std::string& prefix);

    void print_results(const std::vector<size_t>& ids);

    std::vector<size_t> search_text(const std::string& pattern, StringSearchAlgorithm algorithm);

    /**
     *  set_timestamp_regex
     *  Parse every stored log line using regex_str to extract a timestamp,
     *  then format_str to convert it to time_t. Builds the SegmentTree.
     */
    void set_timestamp_regex(const std::string& regex_str, const std::string& format_str);

    /**
     *  range_query
     *  Return all logIDs whose timestamp falls in [t_start, t_end] inclusive.
     *  Requires set_timestamp_regex() to have been called first.
     */
    std::vector<size_t> range_query(time_t t_start, time_t t_end);

    /**
     *  has_timestamp_index
     *  Returns true if the segment tree has been built.
     */
    inline bool has_timestamp_index() const
    {
        return seg_tree.is_built();
    }

    /**
     *  timestamp_count
     *  Number of log lines that had a successfully parsed timestamp.
     */
    inline size_t timestamp_count() const
    {
        return seg_tree.size();
    }

    const LogFile& get_logs() const
    {
        return logs;
    }
};