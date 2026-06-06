#include <iostream>
#include <fstream>
#include <string>

#include "tokenize.hxx"
#include "InvertedIndex.hxx"

#include "search_naive.hxx"
#include "search_kmp.hxx"
#include "search_horspool.hxx"
#include "search_boyer_moore.hxx"

int main(void)
{
	
	std::string s = "Hello world 123 ABC $6%";
	std::vector<std::string> words = tokenize(s);

	for(size_t i = 0; i < words.size(); i++)
		std::cout << words[i] << std::endl;

	InvertedIndex index;
	for(size_t i = 0; i < words.size(); i++)
	{
		index.add(words[i], 1);
		index.add(words[i], 2);
		index.add(words[i], 3);
	}

	print(index);
	
	index.merge();

	std::cout << "After merging" << std::endl;
	print(index);

	index.add("123", 4);
	index.add("123", 5);
	index.add("123", 6);

	index.add("world", 5);
	index.add("world", 6);

	print(index);

	index.merge();
	print(index);

	std::vector<size_t> res = index.search("123");
	std::cout << "Result of search for \"123\": ";
	print(res);
	std::cout << std::endl;

	res = index.search("1234");
	std::cout << "Result of search for \"1234\": ";
	print(res);
	std::cout << std::endl;

	std::vector<std::string> s1 = {"123", "world"};
	res = index.intersect(s1);
	print(res);

	std::string s2 = "hello world old cold sold";
	std::cout << s2 << std::endl;
	print(search_naive(s2, "ld"));
	std::cout << std::endl;
	std::vector<size_t> r = search_kmp(s2, "ld");
	print(r);
	std::cout << std::endl;
	std::vector<size_t> q = search_horspool(s2, "ld");
	print(q);
	std::cout << std::endl;
	q = search_boyer_moore(s2, "ld");
	print(q);
	std::cout << std::endl;

	return 0;
}