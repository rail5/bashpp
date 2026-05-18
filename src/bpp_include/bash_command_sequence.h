/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <string>

#include "bpp.h"
#include "bpp_string.h"

namespace bpp {

/**
 * @class bash_command_sequence
 *
 * @brief A sequence of bash commands connected by '&&' and '||'
 *
 * This entity type has two modes of operation:
 *
 *  - WITHOUT perfect_forwarding, the code buffer is consolidated into the single 'joined_code' buffer,
 *    which serves the purpose of pairing each command in the sequence with its respective pre- and post-code.
 *    This ensures that each command's pre- and post-code is only executed if that command is executed.
 *
 *  - WITH perfect_forwarding, the code buffer is split into the usual three distinct buffers of a bpp_string.
 *    These buffers are needed when we want to use bash_command_sequence as a bpp_string,
 *    i.e., in special cases where we do in fact need to separate the pre- and post-code from the main code.
 *    An example of such a case is in arithmetic substitution, e.g. $(( @object.method + 5 )).
 *    In that example case, it's important that the pre- and post-code for @object.method
 *    is not put **inside** the arithmetic expression, but rather before and after the entire expression.
 * 
 */
class bash_command_sequence : public bpp_string {
	protected:
		std::string joined_code;
		bool contains_multiple_commands = false;

		bool perfect_forwarding = false;

		void join();
	public:
		void add_connective(bool is_and);

		void add_code(const std::string& code, bool add_newline = true) override;

		std::string get_code() const override;
		std::string get_pre_code() const override;
		std::string get_post_code() const override;

		void set_perfect_forwarding(bool enable);
};

} // namespace bpp
