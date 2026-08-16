#pragma once

#include <string>
#include <vector>

#include "StringMatcher.hxx"

class NaiveMatcher : public StringMatcher
{
private:
    std::string pattern;

public:
    explicit NaiveMatcher(
        const std::string& pattern);

    bool contains(
        std::string_view text) const override;

    std::vector<size_t> find_all(
        std::string_view text) const override;
};