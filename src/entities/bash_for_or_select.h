/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <string>

#include "bpp.h"
#include "bpp_code_entity.h"

namespace bpp {

/**
 * @class bash_for_or_select
 * 
 * @brief A for loop or select statement in Bash++
 * 
 * This entity gets pushed onto the entity stack when a for loop or select statement is encountered in Bash++ code.
 * 
 * The 'bash_' prefix signifies that this is used to parse ordinary Bash code, not anything specific to Bash++
 */
class bash_for_or_select : public bpp_code_entity {
	private:
		std::string header_pre_code;
		std::string header_post_code;
		std::string header_code;
	public:
		void set_header_pre_code(const std::string& pre_code);
		void set_header_post_code(const std::string& post_code);
		void set_header_code(const std::string& code);

		const std::string& get_header_pre_code() const;
		const std::string& get_header_post_code() const;
		const std::string& get_header_code() const;
};

} // namespace bpp
