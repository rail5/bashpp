/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <memory>
#include <string>
#include <sstream>

#include "bpp.h"
#include "bpp_entity.h"

namespace bpp {

/**
 * @class bpp_code_entity
 * @brief An entity which can contain code
 * 
 * Such as a method, a supershell, or the program itself
 * 
 * This class provides the basic functionality for entities which can contain code
 * Including 3 distinct code buffers:
 * - pre_code: Code that should be executed before the main code
 * - code: The main code
 * - post_code: Code that should be executed after the main code
 * 
 * Generally, the pre_code and post_code are used to set up and clean up the environment
 * 
 * This class also provides the ability to add code to the pre_code, code, and post_code buffers
 * And to flush those buffers when necessary
 */
class bpp_code_entity : public bpp_entity {
	protected:
		std::shared_ptr<std::ostream> code = std::make_shared<std::ostringstream>();
		std::string nextline_buffer;
		std::string postline_buffer;
		bool buffers_flushed = false;

		/**
		 * @var requires_perfect_forwarding
		 * @brief Signals to bash_command_sequence entities whether they should operate in perfect forwarding mode
		 * I.e., whether this entity has special need to separate its pre- and post-code from its main code
		 */
		bool requires_perfect_forwarding = false;
	public:
		virtual void add_code(const std::string& code, bool add_newline = true);
		virtual void add_code_to_previous_line(const std::string& code);
		virtual void add_code_to_next_line(const std::string& code);

		bool add_object(std::shared_ptr<bpp_object> object, bool make_local = false) override;

		virtual void flush_nextline_buffer();
		virtual void flush_postline_buffer();
		virtual void flush_code_buffers();

		virtual void clear_all_buffers();

		void destruct_local_objects(std::shared_ptr<bpp_program> program);

		virtual std::string get_code() const;
		virtual std::string get_pre_code() const;
		virtual std::string get_post_code() const;

		void set_requires_perfect_forwarding(bool require);
		bool get_requires_perfect_forwarding() const;

		void adopt(std::shared_ptr<bpp_entity> entity);
};

} // namespace bpp
