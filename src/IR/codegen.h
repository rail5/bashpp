/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <string>
#include <vector>
#include <stack>
#include <unordered_map>
#include <iterator>
#include <iostream>

#include <IR/bpp.h>

namespace bpp::CodeGen {

/**
 * @brief Container for compiled (generated) code
 * 
 */
class CodeSegment {
	private:
		/// The code that should be executed before the main code of this segment (e.g., allocation of temporary variables)
		std::vector<std::string> pre_code;
		/// The main code of this segment
		std::vector<std::string> main_code;
		/// The code that should be executed after the main code of this segment (e.g., deallocation of temporary variables)
		std::vector<std::string> post_code;
	public:
		// Moves: preferred
		// When code segments merge from other code segments, these overloads are selected
		void add_pre_code(std::string&& code) { pre_code.push_back(std::move(code)); }
		void add_main_code(std::string&& code) { main_code.push_back(std::move(code)); }
		void add_post_code(std::string&& code) { post_code.push_back(std::move(code)); }

		void add_pre_code(std::vector<std::string>&& code)  { pre_code.insert(pre_code.end(),   std::make_move_iterator(code.begin()), std::make_move_iterator(code.end())); }
		void add_main_code(std::vector<std::string>&& code) { main_code.insert(main_code.end(), std::make_move_iterator(code.begin()), std::make_move_iterator(code.end())); }
		void add_post_code(std::vector<std::string>&& code) { post_code.insert(post_code.end(), std::make_move_iterator(code.begin()), std::make_move_iterator(code.end())); }

		// Copies
		// When code segments pull code from entities, these overloads are selected
		void add_pre_code(const std::string& code) { pre_code.push_back(code); }
		void add_main_code(const std::string& code) { main_code.push_back(code); }
		void add_post_code(const std::string& code) { post_code.push_back(code); }

		void add_pre_code(const std::vector<std::string>& code)  { pre_code.insert(pre_code.end(),   code.begin(), code.end()); }
		void add_main_code(const std::vector<std::string>& code) { main_code.insert(main_code.end(), code.begin(), code.end()); }
		void add_post_code(const std::vector<std::string>& code) { post_code.insert(post_code.end(), code.begin(), code.end()); }

		/**
		 * @brief Absorb all code from another CodeSegment into the main code of this segment
		 * 
		 * @param other The other CodeSegment to absorb
		 */
		void absorb_all_to_main(CodeSegment&& other) {
			add_main_code(std::move(other.pre_code));
			add_main_code(std::move(other.main_code));
			add_main_code(std::move(other.post_code));
		}

		/**
		 * @brief Absorb all code from another CodeSegment into this segment,
		 * such that the pre-code, main code, and post-code of the other segment are merged with the corresponding parts of this segment.
		 * 
		 * @param other The other CodeSegment to absorb
		 */
		void egalitarian_merge(CodeSegment&& other) {
			add_pre_code(std::move(other.pre_code));
			add_main_code(std::move(other.main_code));
			add_post_code(std::move(other.post_code));
		}

		std::vector<std::string> get_pre_code() const { return pre_code; }
		std::vector<std::string> get_main_code() const { return main_code; }
		std::vector<std::string> get_post_code() const { return post_code; }

		std::ostream& write_pre_code(std::ostream& os, bool flush) const {
			for (const auto& part : pre_code) os << part;
			return flush ? os << std::flush : os;
		}
		std::ostream& write_main_code(std::ostream& os, bool flush) const {
			for (const auto& part : main_code) os << part;
			return flush ? os << std::flush : os;
		}
		std::ostream& write_post_code(std::ostream& os, bool flush) const {
			for (const auto& part : post_code) os << part;
			return flush ? os << std::flush : os;
		}
		std::ostream& write_full_code(std::ostream& os) const {
			write_pre_code(os, false);
			write_main_code(os, false);
			write_post_code(os, false);
			return os << std::flush;
		}

		friend std::ostream& operator<<(std::ostream& os, const CodeSegment& code_segment) {
			return code_segment.write_full_code(os);
		}
};

struct CodeGenState {
	bool in_method = false;
	bool in_class = false;
	std::stack<std::monostate> bash_function_stack;
	std::stack<std::monostate> supershell_stack;
	uint64_t dynamic_cast_counter = 0;
	uint64_t object_counter = 0;

	std::unordered_map<std::shared_ptr<const bpp::IR::Object>, std::string> object_addresses;

	bool should_declare_local() const {
		return in_class || in_method || !bash_function_stack.empty();
	}

	bool should_localize_object_instantiation() const {
		return should_declare_local() && supershell_stack.empty();
	}
};

} // namespace bpp::CodeGen
