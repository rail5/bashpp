/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#include <string>

#ifndef SRC_BPP_INCLUDE_REPLACE_ALL_CPP_
#define SRC_BPP_INCLUDE_REPLACE_ALL_CPP_

/**
 * @brief Replace all occurences of a substring in a string
 * @param str The string to search in
 * @param from The substring to search for
 * @param to The substring to replace with
 */
std::string replace_all(std::string str, const std::string& from, const std::string& to) {
	if (from.empty())
		return str;
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length();
	}
	return str;
}

#endif // SRC_BPP_INCLUDE_REPLACE_ALL_CPP_
