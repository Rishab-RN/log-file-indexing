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
        const std::string& text) const override;

    std::vector<size_t> find_all(
        const std::string& text) const override;
};