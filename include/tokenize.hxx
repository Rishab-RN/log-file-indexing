#pragma once

#include <vector>
#include <string>
#include <string_view>

/**
 *  tokenize
 *  Convert a string into a list of words.
 *  All uppercase letters become lowercase.
 *  Special characters are removed.
 */
std::vector<std::string> tokenize(std::string_view line);