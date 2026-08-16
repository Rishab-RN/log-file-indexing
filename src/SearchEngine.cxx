#include "SearchEngine.hxx"
#include <algorithm>
#include <iostream>

void SearchEngine::load(const std::string& file)
{
    // 1. Delegate file loading and memory allocation directly to LogFile
    if (!logs.load_from_disk(file))
    {
        std::cerr << "Error opening file." << std::endl;
        return;
    }

    // 2. Fetch the newly created string_views
    const auto& all_logs = logs.get_all_logs();

    // 3. Build Index and Trie
    for(size_t logID = 0; logID < all_logs.size(); ++logID)
    {
        std::string_view line_view = all_logs[logID];

        // NOTE: If tokenize() currently expects a `const std::string&`, 
        // we convert the view to a string here. For maximum future performance, 
        // update tokenize() to accept a `std::string_view` directly!
        std::vector<std::string> tokens = tokenize(std::string(line_view));

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
    // std::cout handles std::string_view perfectly natively
    for(size_t id : ids)
    {
        std::cout << "[" << id << "] " << logs.get_log(id) << '\n';
    }
}

std::vector<size_t> SearchEngine::search_text(const std::string& pattern, StringSearchAlgorithm algorithm)
{
    std::vector<size_t> result;

    // all_logs is now a vector of std::string_view
    const auto& all_logs = logs.get_all_logs();

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
        // IMPORTANT: Update your Matcher classes so `contains` 
        // accepts a `std::string_view` instead of `const std::string&`.
        if(matcher->contains(all_logs[i]))
        {
            result.push_back(i);
        }
    }

    return result;
}

void SearchEngine::set_timestamp_regex(const std::string& regex_str, const std::string& format_str)
{
    ts_regex_str  = regex_str;
    ts_format_str = format_str;

    std::regex  re(ts_regex_str);
    std::smatch match;

    const auto& all_logs = logs.get_all_logs();

    std::vector<TimestampEntry> entries;
    entries.reserve(all_logs.size());

    for(size_t i = 0; i < all_logs.size(); ++i)
    {
        std::string line(all_logs[i]);

        if(!std::regex_search(line, match, re) || match.size() < 2)
            continue;

        time_t ts = parse_timestamp(match[1].str(), ts_format_str);

        if(ts == static_cast<time_t>(-1))
            continue;

        entries.push_back({ ts, i });
    }

    seg_tree.build(entries);
}

std::vector<size_t> SearchEngine::range_query(time_t t_start, time_t t_end)
{
    return seg_tree.range_query(t_start, t_end);
}