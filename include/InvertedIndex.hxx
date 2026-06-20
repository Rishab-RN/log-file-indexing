#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>

/**
 *  InvertedIndex
 *  This class wraps around a std::unordered_map<std::string, std::vector<int>>
 *  Stores words along with the IDs of the lines they appear in.
 *  It also provides some useful methods.
 */

class InvertedIndex
{
private:
    std::unordered_map<std::string, std::vector<size_t>> index;

public:
    /**
     *  add
     *  Adds an ID to the buffer for the corresponding token.
     */
    void add(const std::string& token, size_t logID);

    /**
     *  search
     *  Find the logIDs for a given token
     *  Returns an empty vector if not found. 
     */
    const std::vector<size_t>& search(const std::string& token) const;

    /**
     *  intersect
     *  Return a vector containing the intersection of all vectors that belong to tokens
     *
     */
    const std::vector<size_t> intersect(const std::vector<std::string>& tokens) const;

    /**
     *  merge
     *  Merge buffer with index
     */
    void merge();

    friend void print(const InvertedIndex& index);
    friend void print(const std::unordered_set<size_t>& set);
};

/**
 *  print
 *  Print an std::unordered_set<int>.
 */
void print(const std::unordered_set<size_t>& set);

/**
 *  print
 *  Print an std::vector<size_t>.
 */
void print(const std::vector<size_t>& vector);

/**
 *  print
 *  Print an InvertedIndex.
 */
void print(const InvertedIndex& index);