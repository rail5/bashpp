/**
 * Copyright (C) 2025 rail5
 * Bash++: Bash with classes
 */

#ifndef SRC_BPP_INCLUDE_BPP_CODE_ENTITY_CPP_
#define SRC_BPP_INCLUDE_BPP_CODE_ENTITY_CPP_

#include "bpp.h"

namespace bpp {

bpp_code_entity::bpp_code_entity() {}

void bpp_code_entity::add_code(const std::string& code, bool add_newline) {
	if (buffers_flushed && code == "\n") {
		return;
	}
	// If the code has a newline char, flush the nextline_buffer and the postline_buffer
	size_t newline_index = code.find("\n");
	if (newline_index != std::string::npos && add_newline && (newline_index == 0 || code[newline_index - 1] != '\\')) {
		flush_nextline_buffer();

		*this->code << code << std::flush;
		if (code.back() != '\n') {
			*this->code << "\n" << std::flush;
		}
		
		flush_postline_buffer();
		buffers_flushed = true;
		return;
	}

	// Otherwise, add the code to the nextline_buffer
	nextline_buffer += code;
	buffers_flushed = false;
}

void bpp_code_entity::add_code_to_previous_line(const std::string& code) {
	*this->code << code << std::flush;
	buffers_flushed = false;
}

void bpp_code_entity::add_code_to_next_line(const std::string& code) {
	postline_buffer += code;
	buffers_flushed = false;
}

void bpp_code_entity::flush_nextline_buffer() {
	if (!nextline_buffer.empty()) {
		*code << nextline_buffer << std::flush;
		if (nextline_buffer.back() != '\n') {
			*code << "\n" << std::flush;
		}
	}
	nextline_buffer = "";
}

void bpp_code_entity::flush_postline_buffer() {
	if (!postline_buffer.empty()) {
		*code << postline_buffer << std::flush;
		if (postline_buffer.back() != '\n') {
			*code << "\n" << std::flush;
		}
	}
	postline_buffer = "";
}

void bpp_code_entity::flush_code_buffers() {
	flush_nextline_buffer();
	flush_postline_buffer();
	buffers_flushed = true;
}

void bpp_code_entity::clear_all_buffers() {
	nextline_buffer = "";
	postline_buffer = "";
	std::shared_ptr<std::ostringstream> ss = std::dynamic_pointer_cast<std::ostringstream>(code);
	if (ss != nullptr) {
		ss->str("");
	}
	buffers_flushed = false;
}

std::string bpp_code_entity::get_code() const {
	std::shared_ptr<std::ostringstream> ss = std::dynamic_pointer_cast<std::ostringstream>(code);
	if (ss == nullptr) {
		return "";
	}
	return ss->str();
}

std::string bpp_code_entity::get_pre_code() const {
	return "";
}

std::string bpp_code_entity::get_post_code() const {
	return "";
}

} // namespace bpp

#endif // SRC_BPP_INCLUDE_BPP_CODE_ENTITY_CPP_