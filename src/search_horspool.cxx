#include "search_horspool.hxx"

static std::vector<size_t> ShiftTable(const std::string& pattern)
{
    size_t m = pattern.size();
    std::vector<size_t> Shift(256, m);

    for(size_t j = 0; j < m - 1; j++)
        Shift[static_cast<unsigned char>(pattern[j])] = m - 1 - j;

    return Shift;
}

std::vector<size_t> search_horspool(const std::string& str, const std::string& pattern)
{
    std::vector<size_t> indices;

    if(pattern.empty() || str.size() < pattern.size())
        return indices;

    size_t m = pattern.size();
    size_t n = str.size();

    std::vector<size_t> Shift = ShiftTable(pattern);

    size_t i = m - 1;
    while(i <= n - 1)
    {
        size_t k = 0;

        while(k <= m - 1 && pattern[m - 1 - k] == str[i - k])
            k++;

        if(k == m)
            indices.push_back(i - m + 1);

        i += Shift[static_cast<unsigned char>(str[i])];
    }

    return indices;
}
