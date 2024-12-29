/***
 * Copyright (C) 2024 rail5
*/

#ifndef SSTREAM
	#define SSTREAM
	#include <sstream>
#endif

#ifndef VECTOR
	#define VECTOR	
	#include <vector>
#endif

std::vector<std::string> explode(std::string const &input, char delimiter, bool can_escape = false, int maximum_number_of_elements = 0) {
	/***
	vector<string> explode(string &input, char delimiter, bool can_escape, int maximum_number_of_elements):
		Similar to PHP's "explode" function
		
		Splits the input string into separate strings, using the provided delimiter
			Example:
				string input = "Test,one,two,three";
				explode(input, ",");
			Would return the following vector:
				{"Test", "one", "two", "three"}
		
		If can_escape == true, then we do NOT split where the delimiter is backslash-escaped.
			Example:
				string input = "Test,one\,two,three";
				explode(input, ",", true);
			Would return the following vector:
				{"Test", "one,two", "three"}
				
		The "maximum_number_of_elements" parameter, if set higher than zero, limits how many elements we can split apart
			Example:
				string input = "Test,one,two,three";
				explode(input, ",", 3);
			Would return the following vector:
				{"Test", "one", "two,three"}
	***/
	// Create the vector
	std::vector<std::string> output_vector;
	
	// Create a stringstream from the input
	std::istringstream input_string_stream(input);

	std::string buffer = "";
	
	// Cycle through the stringstream and push back the substrings
	for (std::string token; std::getline(input_string_stream, token, delimiter); ) {
		if (can_escape) {
			// Need to process escaped delimiters
			if (token[token.length()-1] == '\\') {
				// Escaped delimiter, move to buffer until we hit one that isn't escaped or we run out of text
				buffer += token.replace(token.length()-1, 1, 1, delimiter); // Remove the backslash, re-insert the delimiter
				token = "";
			} else {
				// Real delimiter (not escaped), push a concatenated string of 'the buffer' + 'token' into our vector, and then clear the buffer
				std::string concatenated = buffer + std::move(token);
				output_vector.push_back(std::move(concatenated));
				buffer = "";
			}
		} else {
			// Don't care about escaped chars, just push the token into the vector
			output_vector.push_back(std::move(token));
		}
	}
	// We've run out of text to process. Is the buffer empty?
	if (buffer != "") {
		// If not, we need to push it into the vector as well
		output_vector.push_back(std::move(buffer));
	}
	
	// If maximum_number_of_elements is set, we want to recombine any splits after the maximum
	if (maximum_number_of_elements > 0) {
		// Set the highest index (counting from zero) we want the vector to have
		int last_permissible_element = maximum_number_of_elements - 1;
		
		// Store the current actual highest index of the vector
		int last_element = output_vector.size() - 1;
		
		// Cycle through the vector and combine into the last_permissible_element
		// (re-inserting the delimiter character between previously split elements)
		// Then delete the higher elements
		for (int i = maximum_number_of_elements; i <= last_element; i++) {
			output_vector[last_permissible_element] = output_vector[last_permissible_element] + delimiter + output_vector[last_permissible_element + 1];
			output_vector.erase(output_vector.begin() + (last_permissible_element + 1));
		}
		
		// If the input string originally ended with the delimiter char,
		// Let's put that back in place at the end
		if (input[input.length()-1] == delimiter) {
			output_vector[last_permissible_element] = output_vector[last_permissible_element] + delimiter;
		}
	}
	
	return output_vector;
}
