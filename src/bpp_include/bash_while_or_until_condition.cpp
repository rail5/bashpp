/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#include "bpp.h"

namespace bpp {

bash_while_or_until_condition::bash_while_or_until_condition() {}

void bash_while_or_until_condition::increment_supershell_count() {
	supershell_count++;
}

void bash_while_or_until_condition::add_supershell_function_call(const std::string& function_call) {
	supershell_function_calls.push_back(function_call);
}

std::vector<std::string> bash_while_or_until_condition::get_supershell_function_calls() const {
	return supershell_function_calls;
}

} // namespace bpp
