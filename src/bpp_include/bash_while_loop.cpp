/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#include "bpp.h"

namespace bpp {

bash_while_loop::bash_while_loop() {}

void bash_while_loop::set_while_condition(std::shared_ptr<bpp::bash_while_condition> while_condition) {
	this->while_condition = while_condition;
}

std::shared_ptr<bpp::bash_while_condition> bash_while_loop::get_while_condition() const {
	return while_condition;
}

std::string bash_while_loop::get_code() const {
	return nextline_buffer;
}

std::string bash_while_loop::get_pre_code() const {
	std::shared_ptr<std::ostringstream> ss = std::dynamic_pointer_cast<std::ostringstream>(code);
	if (ss == nullptr) {
		return "";
	}
	return ss->str();
}

std::string bash_while_loop::get_post_code() const {
	return postline_buffer;
}

} // namespace bpp
