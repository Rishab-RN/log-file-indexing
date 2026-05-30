#include "search_horspool.hxx"
#include <algorithm>

std::vector<size_t> search_horspool(const std::string &str, const std::string &pattern)
{
    std::vector<size_t> indices;
    if(pattern.empty() || str.size() < pattern.size())
        return indices;

    size_t m = pattern.size();
    size_t n = str.size();

    // Shift table for 256 characters
    std::vector<size_t> shift(256, m);

    // Populate shift table
    for(size_t i = 0; i < m - 1; ++i)
    {
        shift[static_cast<unsigned char>(pattern[i])] = m - 1 - i;
    }

    size_t i = m - 1;
    while(i < n)
    {
        size_t k = 0;
        while(k < m && pattern[m - 1 - k] == str[i - k])
        {
            k++;
        }
        
        if(k == m)
        {
            indices.push_back(i - m + 1);
        }
        
        i += shift[static_cast<unsigned char>(str[i])];
    }
    
    return indices;
}
