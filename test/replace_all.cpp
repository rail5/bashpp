#include <string>

#ifndef REPLACE_ALL_CPP_
#define REPLACE_ALL_CPP_

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

#endif // REPLACE_ALL_CPP_
