#include "InvertedIndex.hxx"
#include "config.hxx"
#include <algorithm>

/**
 *  __intersect
 *  Return the intersection of two std::vector<int>.
 */
static void intersect_inplace(
    std::vector<size_t>& result,
    const std::vector<size_t>& other)
{
    size_t write = 0;
    size_t i = 0;
    size_t j = 0;

    while (i < result.size() && j < other.size())
    {
        if (result[i] == other[j])
        {
            result[write++] = result[i];
            ++i;
            ++j;
        }
        else if (result[i] < other[j])
        {
            ++i;
        }
        else
        {
            ++j;
        }
    }

    result.resize(write);
}

void InvertedIndex::add(const std::string& token, size_t logID)
{
    auto& vec = index[token];

    if (vec.empty() || vec.back() != logID)
        vec.push_back(logID);
}

const std::vector<size_t> InvertedIndex::search(const std::string& token) const
{
    std::unordered_map<std::string, std::vector<size_t>>::const_iterator it = index.find(token);

    static const std::vector<size_t> empty;
    return (it != index.end()) ? it->second : empty;
}

const std::vector<size_t> InvertedIndex::intersect(const std::vector<std::string>& tokens) const
{
    /**
     *  This functions performs the following steps:
     *  
     *  1. Create a new std::vector result.
     *  2. If there are no tokens to search for, return an empty vector.
     *  3. For each token, get the indices and store them. If any of the tokens lacks an index, return an empty vector.
     *  4. Sort the lists by their size.
     *  5. Intersect the lists, breaking early if the result is ever empty.
     */
    std::vector<size_t> result;

    if(tokens.empty()) 
        return result;

    std::vector<const std::vector<size_t>*> lists;
    lists.reserve(tokens.size());

    for(std::vector<std::string>::const_iterator token = tokens.begin(); token != tokens.end(); token++)
    {
        std::unordered_map<std::string, std::vector<size_t>>::const_iterator it = index.find(*token);
        if (it == index.end())
            return result;

        lists.push_back(&it->second);
    }

    if (lists[0]->empty())
        return {};

    std::sort(lists.begin(), lists.end(), [](const std::vector<size_t>* a, const std::vector<size_t>* b) { return a->size() < b->size(); });

    result = *lists[0];

    for (size_t i = 1; i < lists.size(); ++i)
    {
        intersect_inplace(result, *lists[i]);

        if (result.empty())
            break;
    }

    return result;
}

void print(const std::unordered_set<size_t>& set)
{
    bool first = true;
    std::cout << "{";

    for(std::unordered_set<size_t>::const_iterator i = set.begin(); i != set.end(); i++)
    {
        if (!first)
            std::cout << ", ";
        std::cout << *i;
        first = false;
    }
    
    std::cout << "}";        
}

void print(const std::vector<size_t>& vector)
{
    bool first = true;
    std::cout << "{";

    for(std::vector<size_t>::const_iterator i = vector.begin(); i != vector.end(); i++)
    {
        if (!first)
            std::cout << ", ";
        std::cout << *i;
        first = false;
    }
    
    std::cout << "}";
}

void print(const InvertedIndex& index)
{
    const std::unordered_map<std::string, std::vector<size_t>>& storage = index.index;

    std::cout << "Index:" << std::endl;
    for (std::unordered_map<std::string, std::vector<size_t>>::const_iterator i = storage.begin(); i != storage.end(); i++)
    {
        std::cout << "\"" << i->first << "\" : "; 
        print(i->second); 
        std::cout << std::endl;
    }
}