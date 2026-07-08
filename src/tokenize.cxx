#include "tokenize.hxx"
#include <cctype>

std::vector<std::string> tokenize(std::string_view line)
{
    std::vector<std::string> words;
    
    // Reserve an estimated capacity to prevent vector reallocations.
    // Assuming an average word length of ~6 characters including delimiters.
    words.reserve(line.size() / 6);

    std::string current_word;
    // Reserve space for a typical word to avoid reallocation per character
    current_word.reserve(32); 

    for (char c : line)
    {
        unsigned char uc = static_cast<unsigned char>(c);
        
        if (std::isalnum(uc))
        {
            // Valid character: lower-case it and append to our current word
            current_word.push_back(static_cast<char>(std::tolower(uc)));
        }
        else
        {
            // Non-alphanumeric character acts as our delimiter.
            // If we have built a word, push it to the vector and reset.
            if (!current_word.empty())
            {
                words.push_back(current_word);
                current_word.clear();
            }
        }
    }

    // Capture the final word if the line ends with an alphanumeric character
    if (!current_word.empty())
    {
        words.push_back(current_word);
    }

    return words;
}