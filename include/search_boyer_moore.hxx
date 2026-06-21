#pragma once

#include <array>
#include <string>
#include <vector>

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

    bool contains(const std::string& text) const override;

    std::vector<size_t> find_all(
        const std::string& text) const override;
};

std::vector<size_t> search_boyer_moore(
    const std::string& text,
    const std::string& pattern);