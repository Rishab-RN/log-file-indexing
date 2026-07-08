#include "SearchEngine.hxx"
#include <algorithm>
#include <memory>

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
        inputFile.close();
    }
    else
    {
        std::cerr << "Error opening file." << std::endl;
    }
}


const std::vector<size_t> SearchEngine::search_token(const std::string& token)
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

std::vector<size_t> SearchEngine::search_text(const std::string& pattern, StringSearchAlgorithm algorithm)
{
    std::vector<size_t> result;

    const std::vector<std::string>& all_logs =
        logs.get_all_logs();

    std::unique_ptr<StringMatcher> matcher;

    switch(algorithm)
    {
        case StringSearchAlgorithm::Naive:
            matcher = std::make_unique<NaiveMatcher>(pattern);
            break;

        case StringSearchAlgorithm::KMP:
            matcher = std::make_unique<KMPMatcher>(pattern);
            break;

        case StringSearchAlgorithm::Horspool:
            matcher = std::make_unique<HorspoolMatcher>(pattern);
            break;

        case StringSearchAlgorithm::BoyerMoore:
            matcher = std::make_unique<BoyerMooreMatcher>(pattern);
            break;
    }

    result.reserve(1024);

    for(size_t i = 0; i < all_logs.size(); ++i)
    {
        if(matcher->contains(all_logs[i]))
        {
            result.push_back(i);
        }
    }

    return result;
}