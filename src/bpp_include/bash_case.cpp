/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#include "bpp.h"

namespace bpp {

bash_case::bash_case() {}

void bash_case::add_case(const std::string& case_) {
	cases += case_;
}

const std::string& bash_case::get_cases() const {
	return cases;
}

} // namespace bpp
