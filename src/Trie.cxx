#include "Trie.hxx"

Trie::Trie()
{
    root = std::make_unique<TrieNode>();
}

void Trie::insert(const std::string& word)
{
    TrieNode* cur = root.get();

    for(size_t i = 0; i < word.size(); i++)
    {
        int j = word[i] - 'a';
        if(cur->children[j] == nullptr)
        {
            cur->children[j] = std::make_unique<TrieNode>();
        }

        cur = cur->children[j].get();
    }

    cur->is_word = true;
}

bool Trie::search(const std::string& word)
{
    TrieNode* cur = root.get();

    for(size_t i = 0; i < word.size(); i++)
    {
        int j = word[i] - 'a';
        if(cur->children[j] == nullptr)
            return false;

        cur = cur->children[j].get();
    }

    return cur->is_word;
}

bool Trie::starts_with(const std::string& word)
{
    TrieNode* cur = root.get();

    for(size_t i = 0; i < word.size(); i++)
    {
        int j = word[i] - 'a';
        if(cur->children[j] == nullptr)
            return false;

        cur = cur->children[j].get();
    }

    return true;
}

void Trie::dfs(TrieNode* node, std::string current, std::vector<std::string>& results)
{
    if(node->is_word)
        results.push_back(current);

    for(size_t i = 0; i < 26; i++)
    {
        if(node->children[i])
        {
            dfs(node->children[i].get(), current + char('a' + i), results);
        }
    }
}

std::vector<std::string> Trie::autocomplete(const std::string& prefix)
{
    TrieNode* cur = root.get();

    std::vector<std::string> results;

    for(size_t i = 0; i < prefix.size(); i++)
    {
        int j = prefix[i] - 'a';

        if(cur->children[j] == nullptr)
            return results;

        cur = cur->children[j].get();
    }

    dfs(cur, prefix, results);

    return results;
}

bool Trie::__delete(std::unique_ptr<TrieNode>& node, const std::string& word, int depth)
{
    if(!node)
        return false;

    if(depth == word.size()) 
    {
        if(!node->is_word) 
            return false;

        node->is_word = false;
        
        return is_empty(node.get()); 
    }

    int index = word[depth] - 'a';
    if (__delete(node->children[index], word, depth + 1)) {
        node->children[index].reset();
        
        return !node->is_word && is_empty(node.get());
    }
    
    return false;
}

bool Trie::delete_word(const std::string& word) 
{
    return __delete(root, word, 0);
}

bool Trie::is_empty(TrieNode* node) 
{
    for (int i = 0; i < 26; i++) 
    {
        if (node->children[i]) return false;
    }

    return true;
}