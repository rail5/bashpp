/**
 * Copyright (C) 2025 rail5
 * Bash++: Bash with classes
 */

#ifndef SRC_BPP_INCLUDE_BPP_CODE_ENTITY_CPP_
#define SRC_BPP_INCLUDE_BPP_CODE_ENTITY_CPP_

#include "bpp.h"

namespace bpp {

bpp_code_entity::bpp_code_entity() {}

void bpp_code_entity::add_code(std::string code) {
	// If the code has a newline char, flush the nextline_buffer and the postline_buffer
	if (code.find("\n") != std::string::npos) {
		flush_nextline_buffer();

		this->code += code;
		if (code.back() != '\n') {
			this->code += "\n";
		}
		
		flush_postline_buffer();
		return;
	}

	// Otherwise, add the code to the nextline_buffer
	nextline_buffer += code;
}

void bpp_code_entity::add_code_to_previous_line(std::string code) {
	this->code += code;
}

void bpp_code_entity::add_code_to_next_line(std::string code) {
	postline_buffer += code;
}

void bpp_code_entity::flush_nextline_buffer() {
	if (!nextline_buffer.empty()) {
		code += nextline_buffer;
		if (nextline_buffer.back() != '\n') {
			code += "\n";
		}
	}
	nextline_buffer = "";
}

void bpp_code_entity::flush_postline_buffer() {
	if (!postline_buffer.empty()) {
		code += postline_buffer;
		if (postline_buffer.back() != '\n') {
			code += "\n";
		}
	}
	postline_buffer = "";
}

void bpp_code_entity::flush_code_buffers() {
	flush_nextline_buffer();
	flush_postline_buffer();
}

std::string bpp_code_entity::get_code() const {
	return code;
}

std::string bpp_code_entity::get_pre_code() const {
	return "";
}

std::string bpp_code_entity::get_post_code() const {
	return "";
}

} // namespace bpp

#endif // SRC_BPP_INCLUDE_BPP_CODE_ENTITY_CPP_