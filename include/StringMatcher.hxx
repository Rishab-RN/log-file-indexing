#pragma once

class StringMatcher
{
public:
    virtual ~StringMatcher() = default;

    virtual bool contains(
        const std::string& text) const = 0;

    virtual std::vector<size_t> find_all(
        const std::string& text) const = 0;
};