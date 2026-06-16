#include "SearchEngine.hxx"
#include <algorithm>

void SearchEngine::load(const std::string& file)
{
    std::ifstream inputFile(file);
    if (inputFile.is_open())
    {
        std::string line;
        while(std::getline(inputFile, line))
        {
            size_t logID = logs.add_log(line);
            std::vector<std::string> tokens = tokenize(line);

            for(const std::string& token : tokens)
            {

                index.add(token, logID);

                std::string processed_token = token;
                
                std::transform(processed_token.begin(), processed_token.end(), 
                               processed_token.begin(), ::tolower);

                bool alpha_only = !processed_token.empty() && std::all_of(processed_token.begin(), processed_token.end(), [](unsigned char c)
                {
                    return std::isalpha(c);
                });

                if(alpha_only)
                {
                    trie.insert(processed_token);
                }
            }
        }
        index.merge();
        inputFile.close();
    }
    else
    {
        std::cerr << "Error opening file." << std::endl;
    }
}


const std::vector<size_t>& SearchEngine::search_token(const std::string& token)
{
    return index.search(token);
}


const std::vector<size_t> SearchEngine::search_and(const std::vector<std::string>& tokens)
{
    return index.intersect(tokens);
}

std::vector<std::string> SearchEngine::autocomplete(const std::string& prefix)
{
    return trie.autocomplete(prefix);
}


void SearchEngine::print_results(const std::vector<size_t>& ids)
{
    for(std::vector<size_t>::const_iterator id = ids.begin(); id != ids.end(); id++)
    {
        std::cout << "[" << *id << "] " << logs.get_log(*id) << '\n';
    }

    return;
}