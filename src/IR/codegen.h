/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <string>
#include <vector>
#include <iterator>

#include <IR/bpp.h>

#include <include/BashVersion.h>
#include <include/OutputStream.h>

namespace bpp::CodeGen {

/**
 * @brief Container for compiled (generated) code
 *
 * A CodeSegment contains three inner containers: pre-code, main code, and post-code.
 *
 * The `main code` is the code that the caller is interested in executing.
 * The `pre-code` container should hold code that is necessary in order "set up" for the main code to execute correctly, but which is not of interest to the caller.
 * The `post-code` container should hold code that is necessary to "clean up" after the main code has executed.
 *
 *
 * E.g.: Assume a CodeSegment holding the generated code for a Supershell command substitution (before Bash 5.3's native supershell implementation):
 * - The pre-code contains the definition of the supershell function, AND the call to this function which instructs it to store its output in a temporary variable.
 * - The post-code contains the cleanup of both the temporary variable and that supershell function.
 * - The main code contains the reference to the temporary variable which holds the output of the supershell command substitution.
 *
 * The caller is *only* interested in the result of the supershell.
 * The caller may, for example, be handling a ValueAssignment: `@object.dataMember=@(supershell)`
 * In this case, the pre-code floats above the assignment, the post-code is pushed down below it,
 * and the assignment is transformed to `@object.dataMember=${temporary_variable_holding_supershell_output}`.
 * 
 */
class CodeSegment {
	private:
		/// The code that should be executed before the main code of this segment (e.g., allocation of temporary variables)
		std::vector<std::string> pre_code;
		/// The main code of this segment: i.e., the code that the caller is interested in executing
		std::vector<std::string> main_code;
		/// The code that should be executed after the main code of this segment (e.g., deallocation of temporary variables)
		std::vector<std::string> post_code;
	public:
		// Moves: preferred
		// Use wherever possible, to avoid unnecessary copies of code segments
		void add_pre_code(std::string&& code) { pre_code.push_back(std::move(code)); }
		void add_main_code(std::string&& code) { main_code.push_back(std::move(code)); }
		void add_post_code(std::string&& code) { post_code.push_back(std::move(code)); }

		void add_pre_code(std::vector<std::string>&& code)  { pre_code.insert(pre_code.end(),   std::make_move_iterator(code.begin()), std::make_move_iterator(code.end())); }
		void add_main_code(std::vector<std::string>&& code) { main_code.insert(main_code.end(), std::make_move_iterator(code.begin()), std::make_move_iterator(code.end())); }
		void add_post_code(std::vector<std::string>&& code) { post_code.insert(post_code.end(), std::make_move_iterator(code.begin()), std::make_move_iterator(code.end())); }

		// Copies
		// Use only when necessary (e.g., copying code from const entities)
		void copy_to_pre_code(const std::string& code) { pre_code.push_back(code); }
		void copy_to_main_code(const std::string& code) { main_code.push_back(code); }
		void copy_to_post_code(const std::string& code) { post_code.push_back(code); }

		void copy_to_pre_code(const std::vector<std::string>& code)  { pre_code.insert(pre_code.end(),   code.begin(), code.end()); }
		void copy_to_main_code(const std::vector<std::string>& code) { main_code.insert(main_code.end(), code.begin(), code.end()); }
		void copy_to_post_code(const std::vector<std::string>& code) { post_code.insert(post_code.end(), code.begin(), code.end()); }

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
		 * @brief Absorb all code from another CodeSegment into the pre-code of this segment
		 * 
		 * @param other The other CodeSegment to absorb
		 */
		void absorb_all_to_pre(CodeSegment&& other) {
			add_pre_code(std::move(other.pre_code));
			add_pre_code(std::move(other.main_code));
			add_pre_code(std::move(other.post_code));
		}

		/**
		 * @brief Absorb all code from another CodeSegment into the post-code of this segment
		 * 
		 * @param other The other CodeSegment to absorb
		 */
		void absorb_all_to_post(CodeSegment&& other) {
			add_post_code(std::move(other.pre_code));
			add_post_code(std::move(other.main_code));
			add_post_code(std::move(other.post_code));
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

		void move_pre_code(bpp::CodeGen::OutputStream& os, bool flush) {
			for (auto& part : pre_code) os << std::move(part);
			pre_code.clear();
			if (flush) os.flush();
		}
		void move_main_code(bpp::CodeGen::OutputStream& os, bool flush) {
			for (auto& part : main_code) os << std::move(part);
			main_code.clear();
			if (flush) os.flush();
		}
		void move_post_code(bpp::CodeGen::OutputStream& os, bool flush) {
			for (auto& part : post_code) os << std::move(part);
			post_code.clear();
			if (flush) os.flush();
		}
		void move_full_code(bpp::CodeGen::OutputStream& os) {
			move_pre_code(os, false);
			move_main_code(os, false);
			move_post_code(os, false);
			os.flush();
		}

		friend bpp::CodeGen::OutputStream& operator<<(bpp::CodeGen::OutputStream& os, CodeSegment&& code_segment) {
			std::move(code_segment).move_full_code(os);
			return os;
		}
};

struct CodeGenState {
	BashVersion target_bash_version{5, 2};
	std::shared_ptr<const bpp::IR::Method> current_method = nullptr;
	std::shared_ptr<const bpp::IR::Class> current_class = nullptr;
	std::uint64_t nested_bash_function_depth = 0;
	std::uint64_t nested_supershell_depth = 0;
	std::uint64_t dynamic_cast_counter = 0;
	std::uint64_t supershell_counter = 0;

	bool in_class() const { return current_class != nullptr; }
	bool in_method() const { return current_method != nullptr; }

	bool should_declare_local() const {
		return in_class() || in_method() || nested_bash_function_depth > 0;
	}

	bool should_localize_object_instantiation() const {
		return should_declare_local() && nested_supershell_depth == 0;
	}
};

} // namespace bpp::CodeGen
