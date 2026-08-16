#include "search_naive.hxx"

NaiveMatcher::NaiveMatcher(
    const std::string& pattern)
    : pattern(pattern)
{
}

bool NaiveMatcher::contains(
    std::string_view text) const
{
    if(pattern.empty())
        return true;

    if(text.size() < pattern.size())
        return false;

    const size_t n = text.size();
    const size_t m = pattern.size();

    for(size_t i = 0; i <= n - m; ++i)
    {
        size_t j = 0;

        while(j < m &&
              text[i + j] == pattern[j])
        {
            ++j;
        }

        if(j == m)
            return true;
    }

    return false;
}

std::vector<size_t>
NaiveMatcher::find_all(
    std::string_view text) const
{
    std::vector<size_t> result;

    if(pattern.empty())
        return result;

    if(text.size() < pattern.size())
        return result;

    const size_t n = text.size();
    const size_t m = pattern.size();

    for(size_t i = 0; i <= n - m; ++i)
    {
        size_t j = 0;

        while(j < m &&
              text[i + j] == pattern[j])
        {
            ++j;
        }

        if(j == m)
        {
            result.push_back(i);
        }
    }

    return result;
}