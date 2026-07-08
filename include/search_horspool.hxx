#pragma once

#include <array>
#include <string>
#include <vector>

#include "StringMatcher.hxx"

class HorspoolMatcher : public StringMatcher
{
private:
    std::string pattern;

    std::array<size_t,256> shift;

    void build_shift_table();

public:
    explicit HorspoolMatcher(
        const std::string& pattern);

    bool contains(
        std::string_view text) const override;

    std::vector<size_t> find_all(
        std::string_view text) const override;
};

std::vector<size_t> search_horspool(
    std::string_view text,
    std::string_view pattern);