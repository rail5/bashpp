/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#ifndef SRC_BPP_INCLUDE_BASH_FOR_CPP_
#define SRC_BPP_INCLUDE_BASH_FOR_CPP_

#include "bpp.h"

namespace bpp {

bash_for::bash_for() {}

void bash_for::set_header_pre_code(const std::string& pre_code) {
	header_pre_code = pre_code;
}

void bash_for::set_header_post_code(const std::string& post_code) {
	header_post_code = post_code;
}

void bash_for::set_header_code(const std::string& code) {
	header_code = code;
}

const std::string& bash_for::get_header_pre_code() const {
	return header_pre_code;
}

const std::string& bash_for::get_header_post_code() const {
	return header_post_code;
}

const std::string& bash_for::get_header_code() const {
	return header_code;
}

} // namespace bpp

#endif // SRC_BPP_INCLUDE_BASH_FOR_CPP_
