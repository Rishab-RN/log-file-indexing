#include "search_horspool.hxx"


HorspoolMatcher::HorspoolMatcher(
    const std::string& pattern)
    : pattern(pattern)
{
    build_shift_table();
}

void HorspoolMatcher::build_shift_table()
{
    shift.fill(pattern.size());

    if(pattern.empty())
        return;

    for(size_t i = 0; i + 1 < pattern.size(); ++i)
    {
        shift[(unsigned char)pattern[i]]
            = pattern.size() - i - 1;
    }
}

bool HorspoolMatcher::contains(
    std::string_view text) const
{
    const size_t m = pattern.size();
    const size_t n = text.size();

    if(m == 0)
        return true;

    if(n < m)
        return false;

    size_t i = m - 1;

    while(i < n)
    {
        size_t k = 0;

        while(k < m &&
              pattern[m - 1 - k]
                == text[i - k])
        {
            ++k;
        }

        if(k == m)
            return true;

        i += shift[
            (unsigned char)text[i]];
    }

    return false;
}

std::vector<size_t>
HorspoolMatcher::find_all(
    std::string_view text) const
{
    std::vector<size_t> result;

    const size_t m = pattern.size();
    const size_t n = text.size();

    if(m == 0 || n < m)
        return result;

    size_t i = m - 1;

    while(i < n)
    {
        size_t k = 0;

        while(k < m &&
              pattern[m - 1 - k]
                == text[i - k])
        {
            ++k;
        }

        if(k == m)
        {
            result.push_back(
                i - m + 1);
        }

        i += shift[
            (unsigned char)text[i]];
    }

    return result;
}

std::vector<size_t>
search_horspool(
    std::string_view text,
    std::string_view pattern)
{
    std::string pattern_str(pattern);
    HorspoolMatcher matcher(pattern_str);

    return matcher.find_all(text);
}