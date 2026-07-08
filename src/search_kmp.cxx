#include "search_kmp.hxx"

KMPMatcher::KMPMatcher(
    const std::string& pattern)
    : pattern(pattern),
      lps(pattern.size())
{
    compute_lps(pattern, lps);
}

bool KMPMatcher::contains(
    std::string_view text) const
{
    if(text.size() < pattern.size())
    return false;

    if(pattern.empty())
        return true;

    size_t i = 0;
    size_t j = 0;

    const size_t n = text.size();
    const size_t m = pattern.size();

    while(i < n)
    {
        if(text[i] == pattern[j])
        {
            ++i;
            ++j;

            if(j == m)
                return true;
        }
        else if(j)
        {
            j = lps[j - 1];
        }
        else
        {
            ++i;
        }
    }

    return false;
}


std::vector<size_t>
KMPMatcher::find_all(
    std::string_view text) const
{
    std::vector<size_t> result;

    if(pattern.empty())
        return result;

    size_t i = 0;
    size_t j = 0;

    const size_t n = text.size();
    const size_t m = pattern.size();

    while(i < n)
    {
        if(text[i] == pattern[j])
        {
            ++i;
            ++j;

            if(j == m)
            {
                result.push_back(i - m);
                j = lps[j - 1];
            }
        }
        else if(j)
        {
            j = lps[j - 1];
        }
        else
        {
            ++i;
        }
    }

    return result;
}

std::vector<size_t> search_kmp(
    std::string_view text,
    std::string_view pattern)
{
    std::string pattern_str(pattern);
    KMPMatcher matcher(pattern_str);

    return matcher.find_all(text);
}