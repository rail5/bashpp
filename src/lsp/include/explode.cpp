/***
 * Copyright (C) 2024 rail5
*/

#include "explode.h"

/**
 * @brief Splits a string into a vector of strings based on a delimiter.
 * 
 * This function mimics the behavior of PHP's `explode` function.
 * It splits the input string into separate strings using the provided delimiter.
 * If `can_escape` is true, it does not split where the delimiter is backslash-escaped
 * 		and removes said backslash from the string
 * If `maximum_number_of_elements` is set to a value greater than zero, it limits how many elements can be split apart.
 * 		If the limit is reached, the rest of the string is appended to the last element.
 * 		If `maximum_number_of_elements` is zero, it splits the string into as many elements as it needs to.
 * If `preserve_empty` is true, it preserves empty strings in the output vector.
 * 		E.g, if the input string is "Test,,one,,two,,three" and the delimiter is ",",
 * 		it will return {"Test", "", "one", "", "two", "", "three"}.
 * 		If `preserve_empty` is false, it will return {"Test", "one", "two", "three"}.
 */
std::vector<std::string> explode(
	const std::string& input,
	char delimiter,
	bool can_escape,
	int maximum_number_of_elements,
	bool preserve_empty
) {
	std::vector<std::string> result;
	
	bool escaped = false;

	std::string current_element;
	for (auto& c : input) {
		if (c == '\\') {
			if (escaped) {
				// If we are already escaped or escaping is not allowed, treat it as a normal character
				current_element += '\\';
				current_element += c;
				escaped = false; // Reset the escaped state
				continue;
			}

			if (!can_escape) {
				current_element += c; // Add the backslash as a normal character
				escaped = false;
			} else {
				escaped = true;
			}
			continue;
		}

		if (c == delimiter) {
			if (maximum_number_of_elements > 0 && result.size() >= static_cast<size_t>(maximum_number_of_elements - 1)) {
				// If we have reached the maximum number of elements, append the rest of the string to the last element
				if (escaped) {
					current_element += '\\';
					escaped = false;
				}
				current_element += c; // Add the delimiter to the last element
				continue;
			}

			if (escaped) {
				current_element += c;
				escaped = false;
			} else {
				// If we are not escaped, treat it as a delimiter
				if (!current_element.empty() || preserve_empty) {
					result.push_back(current_element);
					current_element.clear();
				}
			}
			continue;
		}

		// If we reach here, it means we are not dealing with an escape character or a delimiter
		if (escaped) {
			current_element += '\\';
			escaped = false;
		}
		current_element += c;
	}
	if (!current_element.empty() || preserve_empty) {
		result.push_back(current_element);
	}
	return result;
}
