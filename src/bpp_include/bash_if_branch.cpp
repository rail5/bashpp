/**
 * Copyright (C) 2025 rail5
 * Bash++: Bash with classes
 */

#ifndef SRC_BPP_INCLUDE_BASH_IF_BRANCH_CPP_
#define SRC_BPP_INCLUDE_BASH_IF_BRANCH_CPP_

#include "bpp.h"

namespace bpp {

bash_if_branch::bash_if_branch() {}

void bash_if_branch::set_if_statement(std::shared_ptr<bpp::bash_if> if_statement) {
	this->if_statement = if_statement;
}

std::shared_ptr<bpp::bash_if> bash_if_branch::get_if_statement() const {
	return if_statement;
}

} // namespace bpp

#endif // SRC_BPP_INCLUDE_BASH_IF_BRANCH_CPP_