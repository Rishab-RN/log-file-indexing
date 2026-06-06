#pragma once

#include <string>
#include <vector>

static inline void compute_lps(const std::string &pattern, std::vector<size_t>& lps)
{
    size_t len = 0, i = 1;
    
    lps[0] = 0;

    while(i < pattern.size())
    {
        if(pattern[i] == pattern[len])
        {
            len++;
            lps[i] = len;
            i++;
        }
        else
        {
            if(len != 0)
                len = lps[len - 1];
            else
            {
                lps[i] = 0;
                i++;
            }
        }
    }
}

/**
 *  search_kmp
 *  Return the indices of all occurances of pattern withing string
 *  Uses KMP string matching.
 */
std::vector<size_t> search_kmp(const std::string &str, const std::string &pattern);