#include "search_boyer_moore.hxx"

std::vector<size_t> search_boyer_moore(const std::string& str, const std::string& pattern)
{
    size_t m = str.size();
    size_t n = pattern.size();

    std::vector<size_t> indices;

    if(n == 0 || n > m)
        return indices;

    compute_bad_char_table(bad_char_table, pattern);
    std::vector<size_t> good_shift = compute_good_suffix_table(pattern);

    size_t shift = 0;

    while(shift <= m - n)
    {
        size_t j = n;

        while(j > 0 && pattern[j - 1] == str[shift + j  - 1])
        {
            j--;
        }

        if(j == 0)
        {
            indices.push_back(shift);

            shift += good_shift[0];
        }
        else
        {
            j--;

            const unsigned char bad = static_cast<unsigned char>(str[shift + j]);

            ptrdiff_t bad_shift = static_cast<ptrdiff_t>(j) - static_cast<ptrdiff_t>(bad_char_table[bad]);

            if (bad_shift < 1)
                bad_shift = 1;

            const size_t good_suffix_shift = good_shift[j];

            shift += std::max(static_cast<size_t>(bad_shift), good_suffix_shift);
        }
    }

    return indices;
}