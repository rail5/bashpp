/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
* Licensed under the GNU General Public License v3.0 or later (GPL-3.0-or-later)
 */

#include "bpp.h"

namespace bpp {

bpp_string::bpp_string() {}

/**
 * @brief Add code to the primary buffer
 * 
 * @param code The code to add
 * @param add_newline Ignored in the case of bpp_string
 */
void bpp_string::add_code(const std::string& code, bool add_newline) {
	nextline_buffer += code;
}

/**
 * @brief Add code to the pre-code buffer
 */
void bpp_string::add_code_to_previous_line(const std::string& code) {
	*this->code << code << std::flush;
}

/**
 * @brief Add code to the post-code buffer
 */
void bpp_string::add_code_to_next_line(const std::string& code) {
	postline_buffer += code;
}

std::string bpp_string::get_code() const {
	return nextline_buffer;
}

std::string bpp_string::get_pre_code() const {
	std::shared_ptr<std::ostringstream> ss = std::dynamic_pointer_cast<std::ostringstream>(code);
	if (ss == nullptr) {
		return "";
	}
	return ss->str();
}

std::string bpp_string::get_post_code() const {
	return postline_buffer;
}

} // namespace bpp
