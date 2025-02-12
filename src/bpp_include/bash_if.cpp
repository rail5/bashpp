/**
 * Copyright (C) 2025 rail5
 * Bash++: Bash with classes
 */

#ifndef SRC_BPP_INCLUDE_BASH_IF_CPP_
#define SRC_BPP_INCLUDE_BASH_IF_CPP_

#include "bpp.h"

namespace bpp {

bash_if::bash_if() {}

void bash_if::add_conditional_branch_pre_code(const std::string& pre_code) {
	conditional_branch_pre_code += pre_code;
}

void bash_if::add_conditional_branch_post_code(const std::string& post_code) {
	conditional_branch_post_code += post_code;
}

void bash_if::new_branch() {
	conditional_branches.push_back(std::make_pair("", ""));
}

void bash_if::add_condition_code(const std::string& condition_code) {
	conditional_branches.back().first += condition_code;
}

void bash_if::add_branch_code(const std::string& branch_code) {
	conditional_branches.back().second += branch_code;
}

std::string bash_if::get_conditional_branch_pre_code() const {
	return conditional_branch_pre_code;
}

std::string bash_if::get_conditional_branch_post_code() const {
	return conditional_branch_post_code;
}

const std::vector<std::pair<std::string, std::string>>& bash_if::get_conditional_branches() const {
	return conditional_branches;
}

} // namespace bpp

#endif // SRC_BPP_INCLUDE_BASH_IF_CPP_
