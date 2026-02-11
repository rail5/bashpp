/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
* Licensed under the GNU General Public License v3.0 or later (GPL-3.0-or-later)
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
