#include "search_boyer_moore.hxx"
#include <algorithm>

std::vector<size_t> search_boyer_moore(const std::string &str, const std::string &pattern)
{
    std::vector<size_t> indices;
    if(pattern.empty() || str.size() < pattern.size())
        return indices;

    int m = pattern.size();
    int n = str.size();

    std::vector<int> badChar(256, -1);
    for(int i = 0; i < m; i++)
    {
        badChar[static_cast<unsigned char>(pattern[i])] = i;
    }

    int s = 0; // shift of the pattern
    while(s <= (n - m))
    {
        int j = m - 1;

        while(j >= 0 && pattern[j] == str[s + j])
            j--;

        if(j < 0)
        {
            indices.push_back(s);
            // Shift pattern so that the next character in text aligns with the last occurrence of it in pattern
            s += (s + m < n) ? m - badChar[static_cast<unsigned char>(str[s + m])] : 1;
        }
        else
        {
            s += std::max(1, j - badChar[static_cast<unsigned char>(str[s + j])]);
        }
    }

    return indices;
}
