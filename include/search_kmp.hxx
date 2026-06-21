#pragma once

#include <string>
#include <vector>

#include "StringMatcher.hxx"

inline void compute_lps(
    const std::string& pattern,
    std::vector<size_t>& lps)
{
    if(pattern.empty())
        return;

    lps[0] = 0;

    size_t len = 0;

    for(size_t i = 1; i < pattern.size(); )
    {
        if(pattern[i] == pattern[len])
        {
            lps[i++] = ++len;
        }
        else if(len)
        {
            len = lps[len - 1];
        }
        else
        {
            lps[i++] = 0;
        }
    }
}

class KMPMatcher : public StringMatcher
{
private:
    std::string pattern;
    std::vector<size_t> lps;

public:
    explicit KMPMatcher(const std::string& pattern);

    bool contains(
        const std::string& text) const override;

    std::vector<size_t> find_all(
        const std::string& text) const override;
};

/*
 * Legacy convenience wrapper.
 */
std::vector<size_t> search_kmp(
    const std::string& text,
    const std::string& pattern);