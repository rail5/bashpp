/**
 * Copyright (C) 2025 rail5
 * Bash++: Bash with classes
 */

#ifndef SRC_BPP_INCLUDE_BPP_STRING_CPP_
#define SRC_BPP_INCLUDE_BPP_STRING_CPP_

#include "bpp.h"

namespace bpp {

bpp_string::bpp_string() {}

void bpp_string::add_code(const std::string& code, bool add_newline) {
	nextline_buffer += code;
}

void bpp_string::add_code_to_previous_line(const std::string& code) {
	*this->code << code << std::flush;
}

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

#endif // SRC_BPP_INCLUDE_BPP_STRING_CPP_
