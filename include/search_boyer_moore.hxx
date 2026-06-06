#pragma once

#include <string>
#include <vector>
#include "config.hxx"

/* bad shift table */
static std::vector<size_t> bad_char_table(NUMBER_OF_CHARS);

/**
 *  compute_bad_char_table
 *  Computes the bad character table for Boyer Moore.
 */
static inline void compute_bad_char_table(std::vector<size_t>& shift, const std::string& pattern)
{
    std::fill(shift.begin(), shift.end(), pattern.size());

    for(size_t j = 0; j < pattern.size(); j++)
        bad_char_table[(unsigned char)(pattern[j])] = j;

    return;
}

static inline std::vector<size_t> compute_suffix(const std::string& pattern)
{
    const size_t n = pattern.size();
    std::vector<size_t> suffix(n);

    if (n == 0)
        return suffix;

    suffix[n - 1] = n;

    ptrdiff_t g = static_cast<ptrdiff_t>(n - 1);
    size_t f = 0;

    for (size_t i = n - 1; i-- > 0;)
    {
        if (static_cast<ptrdiff_t>(i) > g && suffix[i + n - 1 - f] < static_cast<size_t>(i - g))
        {
            suffix[i] = suffix[i + n - 1 - f];
        }
        else
        {
            if (static_cast<ptrdiff_t>(i) < g)
                g = static_cast<ptrdiff_t>(i);

            f = i;

            while (g >= 0 && pattern[static_cast<size_t>(g)] == pattern[static_cast<size_t>(g + n - 1 - f)])
                g--;

            suffix[i] = f - static_cast<size_t>(g);
        }
    }

    return suffix;
}

static inline std::vector<size_t> compute_good_suffix_table(const std::string& pattern)
{
    const size_t n = pattern.size();

    std::vector<size_t> suffix = compute_suffix(pattern);
    std::vector<size_t> good_suffix_table(pattern.size(), pattern.size());

    size_t j = 0;

    for(size_t i = 0; i-- > 0; )
    {
        if(suffix[i] == i + 1)
        {
            while(j < n - 1 - i)
            {
                if(good_suffix_table[j] == n)
                    good_suffix_table[j] = n - 1 - i;

                j++;
            }
        }
    }

    for(size_t i = 0; i < n - 1; i++)
    {
        good_suffix_table[n - 1 - suffix[i]] = n - 1 - i;
    }

    return good_suffix_table;
}

/**
 *  search_boyer_moore
 *  Return the indices of all occurances of pattern withing string
 *  Uses Boyer Moore string matching.
 */
std::vector<size_t> search_boyer_moore(const std::string &str, const std::string &pattern);
