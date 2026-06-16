#include <iostream>
#include <sstream>
#include <string>

#include "SearchEngine.hxx"

int main()
{
    SearchEngine engine;

    std::string line;

    while(true)
    {
        std::cout << "> ";

        if(!std::getline(std::cin, line))
            break;

		std::vector<std::string> tokens;

		std::istringstream iss(line);

		std::string token;

		while(iss >> token)
		{
			tokens.push_back(token);
		}

		if(tokens.empty())
		{
			continue;
		}

		if(tokens[0] == "exit")
		{
			break;
		}

		if(tokens[0] == "load")
		{
			if(tokens.size() < 2)
			{
				std::cout << "Usage: load <file>" << std::endl;
				continue;
			}

			engine.load(tokens[1]);

			std::cout << "Loaded file" << std::endl;
		}
		else if(tokens[0] == "search")
		{
			if(tokens.size() < 2)
			{
				std::cout
					<< "Usage: search <token>" << std::endl;
				continue;
			}

			for(std::vector<std::string>::const_iterator token = tokens.begin(); token != tokens.end(); token++)
			{
				std::cout << *token << std::endl;
			}

			std::vector<size_t> results = engine.search_token(tokens[1]);

			engine.print_results(results);
		}
		else if(tokens[0] == "and")
		{
			if(tokens.size() < 3)
			{
				std::cout << "Usage: and word1 word2 ..." << std::endl;
				continue;
			}

			std::vector<std::string> query(tokens.begin()+1, tokens.end());
			std::vector<size_t> results = engine.search_and(query);

			engine.print_results(results);
		}
		else if(tokens[0] == "autocomplete")
		{
			if(tokens.size() < 2)
			{
				std::cout << "Usage: autocomplete <prefix>" << std::endl;
				continue;
			}

			std::vector<std::string> words = engine.autocomplete(tokens[1]);

			for(std::vector<std::string>::const_iterator word = words.begin(); word != words.end(); word++)
			{
				std::cout << *word << std::endl;
			}
		}
		else if(tokens[0] == "help")
		{
			std::cout << "Commands:" << std::endl;
			std::cout << "load <file>" << std::endl;
			std::cout << "search <token>" << std::endl;
			std::cout << "and <token1> <token2> ..." << std::endl;
			std::cout << "autocomplete <prefix>" << std::endl;
			std::cout << "help" << std::endl;
			std::cout << "exit" << std::endl;
		}
		else
		{
			std::cout
				<< "Unknown command\n";
		}
    }

	return 0;
}