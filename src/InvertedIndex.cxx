#include "InvertedIndex.hxx"
#include <algorithm>

/**
 *  __merge_and_dedup_vectors
 *  Merge two sorted vectors and remove duplicates.
 */
static std::vector<size_t> __merge_and_dedup_vectors(const std::vector<size_t>& vector1, const std::vector<size_t>& vector2);

/**
 *  __intersect
 *  Return the intersection of two std::vector<int>.
 */
static std::vector<size_t> __intersect(const std::vector<size_t>& a, const std::vector<size_t>& b)
{
    std::vector<size_t> result;

    size_t i = 0, j = 0;

    while(i < a.size() && j < b.size())
    {
        if(a[i] == b[j])
        {
            result.push_back(a[i]);
            i++;
            j++;
        }
        else if(a[i] < b[j])
        {
            i++;
        }
        else
        {
            j++;
        }
    }
    return result;
}

void InvertedIndex::add(const std::string& token, size_t logID)
{
    buffer[token].insert(logID);
}

const std::vector<size_t>& InvertedIndex::search(const std::string& token) const
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

    for(std::vector<std::string>::const_iterator token = tokens.begin(); token != tokens.end(); token++)
    {
        std::unordered_map<std::string, std::vector<size_t>>::const_iterator it = index.find(*token);
        if (it == index.end())
            return result;

        lists.push_back(&it->second);
    }

    std::sort(lists.begin(), lists.end(), [](const std::vector<size_t>* a, const std::vector<size_t>* b) { return a->size() < b->size(); });

    result = *lists[0];

    for (size_t i = 1; i < lists.size(); i++)
    {
        const std::vector<size_t>& cur = *lists[i];
        
        result = __intersect(result, cur);

        if (result.empty()) break;
    }

    return result;
}

void InvertedIndex::merge()
{
    /**
     *  This function performs the following steps:
     *  
     *  For each item in the buffer:
     *      1. Convert the std::unordered_set into a std::vector
     *      2. Sort the buffer vector
     *      3. Find the vector for the token in index, or create a new vector if the token isn't in index
     *      4. Merge the two vectors and remove duplicates.
     *      5. Assign the merged vector to the old index vector.
     */
    for(std::unordered_map<std::string, std::unordered_set<size_t>>::iterator i = buffer.begin(); i != buffer.end(); i++)
    {
        std::string token = i->first;
        std::unordered_set<size_t>& set = i->second;

        std::vector<size_t> buffer_vector(set.begin(), set.end());

        std::sort(buffer_vector.begin(), buffer_vector.end());

        std::vector<size_t>& index_vector = index[token];

        std::vector<size_t> merged_vector = __merge_and_dedup_vectors(index_vector, buffer_vector);

        index_vector = std::move(merged_vector);

        set.clear();     
    }

    buffer.clear();
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
    const std::unordered_map<std::string, std::unordered_set<size_t>>& buffer = index.buffer;

    std::cout << "Buffer:" << std::endl;
    for (std::unordered_map<std::string, std::unordered_set<size_t>>::const_iterator i = buffer.begin(); i != buffer.end(); i++)
    {
        std::cout << "\"" << i->first << "\" : "; 
        print(i->second); 
        std::cout << std::endl;
    }

    const std::unordered_map<std::string, std::vector<size_t>>& storage = index.index;

    std::cout << "Index:" << std::endl;
    for (std::unordered_map<std::string, std::vector<size_t>>::const_iterator i = storage.begin(); i != storage.end(); i++)
    {
        std::cout << "\"" << i->first << "\" : "; 
        print(i->second); 
        std::cout << std::endl;
    }
}

std::vector<size_t> __merge_and_dedup_vectors(const std::vector<size_t>& vector1, const std::vector<size_t>& vector2)
{
    std::vector<size_t> result;

    if(vector1.empty() && vector2.empty())
        return result;

    result.reserve(vector1.size() + vector2.size());

    size_t i = 0, j = 0;

    if(!(vector1.empty() || vector2.empty()))
    {

        if(vector1[i] < vector2[j])
        {
            result.push_back(vector1[i]);
            i++;
        }
        else
        {
            result.push_back(vector2[j]);
            j++;
        }

        while(i < vector1.size() && j < vector2.size())
        {
            if(vector1[i] < vector2[j])
            {
                if(result.back() != vector1[i])
                    result.push_back(vector1[i]);
                i++;
            }
            else if(vector2[j] < vector1[i])
            {
                if(result.back() != vector2[j])
                    result.push_back(vector2[j]);
                j++;
            }
            else
            {
                if(result.back() != vector1[i])
                    result.push_back(vector1[i]);

                i++;
                j++;
            }
        }
    }

    if (result.empty())
    {
        if (i < vector1.size())
            result.push_back(vector1[i++]);
        else if (j < vector2.size())
            result.push_back(vector2[j++]);
    }

    while(i < vector1.size())
    {
        if(result.back() != vector1[i])
            result.push_back(vector1[i]);
        i++;
    }

    while(j < vector2.size())
    {
        if(result.back() != vector2[j])
            result.push_back(vector2[j]);
        j++;
    }

    return result;
} 