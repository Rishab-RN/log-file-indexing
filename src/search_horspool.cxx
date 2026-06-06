#include "search_horspool.hxx"

std::vector<size_t> search_horspool(const std::string& str, const std::string& pattern)
{
    compute_shift_table(shift, pattern);

    std::vector<size_t> result;

    size_t i = pattern.size() - 1;
    size_t k;
    
    while(i < str.size())
    {
        k = 0;
        
        while(k < pattern.size() && pattern[pattern.size() - k - 1] == str[i - k])
            k++;

        if(k == pattern.size())
            result.push_back(i - pattern.size() + 1);

        i += shift[(unsigned char)str[i]];
    }

    return result;
}