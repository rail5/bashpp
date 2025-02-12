/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#ifndef SRC_BPP_INCLUDE_BASH_CASE_PATTERN_CPP_
#define SRC_BPP_INCLUDE_BASH_CASE_PATTERN_CPP_

#include "bpp.h"

namespace bpp {

bash_case_pattern::bash_case_pattern() {}

void bash_case_pattern::set_pattern(const std::string& pattern) {
	this->pattern = pattern;
}

const std::string& bash_case_pattern::get_pattern() const {
	return pattern;
}

} // namespace bpp

#endif // SRC_BPP_INCLUDE_BASH_CASE_PATTERN_CPP_
