/**
 * Copyright (C) 2025 rail5
 * Bash++: Bash with classes
 */

#ifndef SRC_BPP_INCLUDE_BASH_WHILE_CPP_
#define SRC_BPP_INCLUDE_BASH_WHILE_CPP_

#include "bpp.h"

namespace bpp {

bash_while::bash_while() {}

void bash_while::increment_supershell_count() {
	supershell_count++;
}

void bash_while::add_supershell_function_call(const std::string& function_call) {
	supershell_function_calls.push_back(function_call);
}

int bash_while::get_supershell_count() const {
	return supershell_count;
}

std::vector<std::string> bash_while::get_supershell_function_calls() const {
	return supershell_function_calls;
}

} // namespace bpp

#endif // SRC_BPP_INCLUDE_BASH_WHILE_CPP_