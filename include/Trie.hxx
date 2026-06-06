#pragma once

#include <vector>
#include <string>
#include <memory>

/**
 *  class TrieNode
 *  A node that forms the basic unit of a Trie.
 */
class TrieNode
{
public:
    std::unique_ptr<TrieNode> children[26] = {nullptr};
    bool is_word = false;
};

/**
 *  class Trie
 *  A complete Trie implementation
 */
class Trie
{
private:
    std::unique_ptr<TrieNode> root;

public:

    Trie();

    /**
     *  insert
     *  Inserts a word into the Trie.
     */
    void insert(const std::string& word);

    /**
     *  search
     *  Searches if a given word exists within the Trie.
     */
    bool search(const std::string& word);

    /**
     *  starts_with
     *  Searches if a given prefix exists within the Trie.
     */
    bool starts_with(const std::string& word);

    /**
     *  autocomplete
     *  Return all possible words a prefix can be part of.
     */
    std::vector<std::string> autocomplete(const std::string& prefix);

    /**
     *  delete_word
     *  Delete a word from a Trie.
     */
    bool delete_word(const std::string& word);

private:

    /**
     *  dfs
     *  Helper depth-first-search function.
     *  DFS is how you find words in a Trie.
     */
    void dfs(TrieNode* node, std::string current, std::vector<std::string>& results);

    /**
     *  __delete
     *  Helps delete a word from a Trie.
     */
    bool __delete(std::unique_ptr<TrieNode>& node, const std::string& word, int depth);

    /**
     *  is_empty
     *  Helps check if  a node is empty.
     */
    bool is_empty(TrieNode* node);
};