#include "search_boyer_moore.hxx"
#include <algorithm>

static std::vector<int> BadCharTable(const std::string& pattern)
{
    int m = pattern.size();
    std::vector<int> BadChar(256, -1);

    for(int j = 0; j < m; j++)
        BadChar[static_cast<unsigned char>(pattern[j])] = j;

    return BadChar;
}

static std::vector<int> GoodSuffixTable(const std::string& pattern)
{
    int m = pattern.size();
    std::vector<int> GoodSuffix(m + 1, 0);
    std::vector<int> bpos(m + 2, 0);

    int i = m, j = m + 1;
    bpos[i] = j;

    while(i > 0)
    {
        while(j <= m && pattern[i - 1] != pattern[j - 1])
        {
            if(GoodSuffix[j] == 0)
                GoodSuffix[j] = j - i;
            j = bpos[j];
        }
        i--;
        j--;
        bpos[i] = j;
    }

    j = bpos[0];
    for(i = 0; i <= m; i++)
    {
        if(GoodSuffix[i] == 0)
            GoodSuffix[i] = j;
        if(i == j)
            j = bpos[j];
    }

    return GoodSuffix;
}

std::vector<size_t> search_boyer_moore(const std::string& str, const std::string& pattern)
{
    std::vector<size_t> indices;

    if(pattern.empty() || str.size() < pattern.size())
        return indices;

    int m = pattern.size();
    int n = str.size();

    std::vector<int> BadChar   = BadCharTable(pattern);
    std::vector<int> GoodSuffix = GoodSuffixTable(pattern);

    int s = 0;
    while(s <= n - m)
    {
        int j = m - 1;

        while(j >= 0 && pattern[j] == str[s + j])
            j--;

        if(j < 0)
        {
            indices.push_back(s);
            s += GoodSuffix[0];
        }
        else
        {
            int badCharShift    = j - BadChar[static_cast<unsigned char>(str[s + j])];
            int goodSuffixShift = GoodSuffix[j + 1];
            s += std::max(badCharShift, goodSuffixShift);
        }
    }

    return indices;
}
