#include <vector>
#include <string>
#include <sstream>

std::vector<std::string> explode(std::string const &s, char delimiter) {
	std::vector<std::string> result;
	std::istringstream iss(s);
	
	for (std::string token; std::getline(iss, token, delimiter); ) {
		result.push_back(std::move(token));
	}
	
	return result;
}
