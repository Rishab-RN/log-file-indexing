#pragma once

#include <array>
#include <string>
#include <vector>
#include <string_view>

#include "StringMatcher.hxx"

class BoyerMooreMatcher : public StringMatcher
{
private:
    std::string pattern;

    std::array<int, 256> bad_char;
    std::vector<size_t> good_suffix;

    void build_bad_char();
    void build_good_suffix();

public:
    explicit BoyerMooreMatcher(const std::string& pattern);

    // Update 'text' to std::string_view passed by value
    bool contains(std::string_view text) const override;

    // Update 'text' to std::string_view passed by value
    std::vector<size_t> find_all(std::string_view text) const override;
};

// For the standalone function, both text and pattern can be views
std::vector<size_t> search_boyer_moore(
    std::string_view text,
    std::string_view pattern);