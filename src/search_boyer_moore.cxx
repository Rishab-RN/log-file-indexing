#include "search_boyer_moore.hxx"

#include <algorithm>

static std::vector<size_t>
compute_suffix(const std::string& pattern)
{
    const size_t n = pattern.size();

    std::vector<size_t> suffix(n);

    if(n == 0)
        return suffix;

    suffix[n - 1] = n;

    ptrdiff_t g = static_cast<ptrdiff_t>(n - 1);
    size_t f = 0;

    for(size_t i = n - 1; i-- > 0;)
    {
        if(static_cast<ptrdiff_t>(i) > g &&
           suffix[i + n - 1 - f] < static_cast<size_t>(i - g))
        {
            suffix[i] = suffix[i + n - 1 - f];
        }
        else
        {
            if(static_cast<ptrdiff_t>(i) < g)
                g = static_cast<ptrdiff_t>(i);

            f = i;

            while(g >= 0 &&
                  pattern[static_cast<size_t>(g)] ==
                  pattern[static_cast<size_t>(g + n - 1 - f)])
            {
                --g;
            }

            suffix[i] = f - static_cast<size_t>(g);
        }
    }

    return suffix;
}

static std::vector<size_t>
compute_good_suffix(const std::string& pattern)
{
    const size_t n = pattern.size();

    if(n == 0)
        return {};

    std::vector<size_t> suffix =
        compute_suffix(pattern);

    std::vector<size_t> good_suffix(n, n);

    size_t j = 0;

    for(size_t i = n; i-- > 0;)
    {
        if(suffix[i] == i + 1)
        {
            while(j < n - 1 - i)
            {
                if(good_suffix[j] == n)
                    good_suffix[j] = n - 1 - i;

                ++j;
            }
        }
    }

    for(size_t i = 0; i + 1 < n; ++i)
    {
        good_suffix[n - 1 - suffix[i]] =
            n - 1 - i;
    }

    return good_suffix;
}

BoyerMooreMatcher::BoyerMooreMatcher(
    const std::string& pattern)
    : pattern(pattern)
{
    build_bad_char();
    build_good_suffix();
}

void BoyerMooreMatcher::build_bad_char()
{
    bad_char.fill(-1);

    for(size_t i = 0; i < pattern.size(); ++i)
    {
        bad_char[
            static_cast<unsigned char>(pattern[i])
        ] = static_cast<int>(i);
    }
}

void BoyerMooreMatcher::build_good_suffix()
{
    good_suffix = compute_good_suffix(pattern);
}

bool BoyerMooreMatcher::contains(
    const std::string& text) const
{
    const size_t m = text.size();
    const size_t n = pattern.size();

    if(n == 0)
        return true;

    if(n > m)
        return false;

    size_t shift = 0;

    while(shift <= m - n)
    {
        size_t j = n;

        while(j > 0 &&
              pattern[j - 1] ==
              text[shift + j - 1])
        {
            --j;
        }

        if(j == 0)
        {
            return true;
        }

        --j;

        const unsigned char bad =
            static_cast<unsigned char>(
                text[shift + j]);

        int bad_shift =
            static_cast<int>(j)
            - bad_char[bad];

        if(bad_shift < 1)
            bad_shift = 1;

        shift += std::max(
            static_cast<size_t>(bad_shift),
            good_suffix[j]);
    }

    return false;
}

std::vector<size_t>
BoyerMooreMatcher::find_all(
    const std::string& text) const
{
    std::vector<size_t> result;

    const size_t m = text.size();
    const size_t n = pattern.size();

    if(n == 0 || n > m)
        return result;

    size_t shift = 0;

    while(shift <= m - n)
    {
        size_t j = n;

        while(j > 0 &&
              pattern[j - 1] ==
              text[shift + j - 1])
        {
            --j;
        }

        if(j == 0)
        {
            result.push_back(shift);

            shift += std::max(
                static_cast<size_t>(1),
                good_suffix[0]);
        }
        else
        {
            --j;

            const unsigned char bad =
                static_cast<unsigned char>(
                    text[shift + j]);

            int bad_shift =
                static_cast<int>(j)
                - bad_char[bad];

            if(bad_shift < 1)
                bad_shift = 1;

            shift += std::max(
                static_cast<size_t>(bad_shift),
                good_suffix[j]);
        }
    }

    return result;
}

std::vector<size_t>
search_boyer_moore(
    const std::string& text,
    const std::string& pattern)
{
    BoyerMooreMatcher matcher(pattern);

    return matcher.find_all(text);
}