/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#ifndef SRC_BPP_INCLUDE_BASH_WHILE_CONDITION_CPP_
#define SRC_BPP_INCLUDE_BASH_WHILE_CONDITION_CPP_

#include "bpp.h"

namespace bpp {

bash_while_condition::bash_while_condition() {}

void bash_while_condition::increment_supershell_count() {
	supershell_count++;
}

void bash_while_condition::add_supershell_function_call(const std::string& function_call) {
	supershell_function_calls.push_back(function_call);
}

std::vector<std::string> bash_while_condition::get_supershell_function_calls() const {
	return supershell_function_calls;
}

} // namespace bpp

#endif // SRC_BPP_INCLUDE_BASH_WHILE_CONDITION_CPP_
