/**
 * Bash++: The Bourne-Again Shell with Classes
 * Copyright (C) 2024 rail5
 */

#include <iostream>
#include <string>
#include <vector>
#include <fstream>

#include "token.cpp"
#include "tokenizer.cpp"

int main(int argc, char* argv[]) {
	if (argc < 2) {
		std::cerr << "Usage: " << argv[0] << " <filename>" << std::endl;
		return 1;
	}
	
	std::ifstream file(argv[1]);
	if (!file.is_open()) {
		std::cerr << "Error: Could not open file " << argv[1] << std::endl;
		return 1;
	}

	bashpp::Tokenizer tokenizer;
	
	std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	file.close();
	
	tokenizer.parse(content);

	std::cout << "Tokens:" << std::endl;
	for (auto& token : tokenizer.getTokens()) {
		std::cout << token << std::endl;
	}
	
	return 0;
}
