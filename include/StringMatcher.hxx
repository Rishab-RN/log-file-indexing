#pragma once

#include <string_view>

class StringMatcher
{
public:
    virtual ~StringMatcher() = default;

    virtual bool contains(
        std::string_view text) const = 0;

    virtual std::vector<size_t> find_all(
        std::string_view text) const = 0;
};