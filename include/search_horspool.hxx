#pragma once

#include <string>
#include <vector>
#include "config.hxx"

static std::vector<size_t> shift(NUMBER_OF_CHARS);

/**
 *  compute_shift_table
 *  Computes the shift table for Horspool's algorithm.
 */
static inline void compute_shift_table(std::vector<size_t>& shift, const std::string& pattern)
{
    std::fill(shift.begin(), shift.end(), pattern.size());

    for(size_t i = 0; i < pattern.size() - 1; i++)
        shift[(unsigned char)pattern[i]] = pattern.size() - i - 1;

    return;
}

/**
 *  search_horspool
 *  Return the indices of all occurances of pattern withing string
 *  Uses Horspool's algorithm.
 */
std::vector<size_t> search_horspool(const std::string& str, const std::string& pattern);