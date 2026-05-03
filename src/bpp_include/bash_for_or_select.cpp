/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "bpp.h"

namespace bpp {

void bash_for_or_select::set_header_pre_code(const std::string& pre_code) {
	header_pre_code = pre_code;
}

void bash_for_or_select::set_header_post_code(const std::string& post_code) {
	header_post_code = post_code;
}

void bash_for_or_select::set_header_code(const std::string& code) {
	header_code = code;
}

const std::string& bash_for_or_select::get_header_pre_code() const {
	return header_pre_code;
}

const std::string& bash_for_or_select::get_header_post_code() const {
	return header_post_code;
}

const std::string& bash_for_or_select::get_header_code() const {
	return header_code;
}

} // namespace bpp
