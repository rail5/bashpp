/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
* Licensed under the GNU General Public License v3.0 or later (GPL-3.0-or-later)
 */

#include "bpp.h"

namespace bpp {

void bash_command_sequence::join() {
	if (perfect_forwarding) return; // Disable joining when perfect forwarding is enabled
	// Move the contents of each buffer into temporary strings
	auto ss = std::dynamic_pointer_cast<std::ostringstream>(code);
	std::string pre_code;
	if (ss != nullptr) {
		pre_code = ss->str();
		ss->str("");
	}

	std::string post_code = std::move(postline_buffer);
	postline_buffer.clear();

	std::string main_code = std::move(nextline_buffer);
	nextline_buffer.clear();


	// Don't bother adding the braces if there's no code to contain in them
	if (pre_code.empty() && main_code.empty() && post_code.empty()) return;

	std::string enclose_open = (contains_multiple_commands) ? "{\n" : "";
	std::string enclose_close = (contains_multiple_commands) ? "\n}" : "";

	joined_code += enclose_open
		+ (pre_code.empty() ? "" : pre_code + "\n")
		+ (main_code.empty() ? "" : main_code)
		+ (post_code.empty() ? "" : "\n____ret=$?\n" + post_code + "\nbpp____repeat $____ret")
		+ enclose_close;
}

/**
 * @brief Add a connective to the command sequence, finalizing the previously-received pipeline.
 * Initially, ->add_code(some_pipeline) will store (for example) "echo hello, world" in the buffer for this entity.
 * Subsequently calling ->add_connective() will turn that buffer into:
 *
 * 		{
 *
 * 		_pre-code_
 *
 *		echo hello, world
 *
 * 		_post-code_
 *
 * 		} _CONNECTIVE_ 
 *
 * Preparing to receive the next pipeline
 *
 * This splits each component of the command sequence across separate lines,
 * which ensures that the relevant pre- and post-code for each component is executed IF AND ONLY IF that component is executed
 *
 * Without doing this, a sequence such as `false && @object.method` would execute the pre- and post-code for `@object.method`,
 * even though the method is guaranteed to never be called.
 *
 * To give an extreme example, `false && @(rm -rf /)` would be catastrophic without this handling,
 * as the pre-code necessary for the supershell involves executing the command inside of it
 * and storing its output in a temporary variable to be substituted in place of the original expression.
 * This would, in this case, execute `rm -rf /` regardless of the `false &&` at the start of the command sequence.
 *
 * By handling it in this way, however, the pre- and post-code for each component is only executed if that component is executed.
 * 
 * @param is_and True if the connective is '&&', false if it is '||'
 */
void bash_command_sequence::add_connective(bool is_and) {
	if (perfect_forwarding) {
		bpp_string::add_code(is_and ? " && " : " || ", false);
		return;
	}

	if (!contains_multiple_commands) {
		joined_code = "{\n" + joined_code + "\n}";
		contains_multiple_commands = true;
	}
	joined_code += (is_and ? " && " : " || ");
}

void bash_command_sequence::add_code(const std::string& code, bool add_newline) {
	// On a bash_command_sequence, this method should only ever be called when adding a COMPLETE pipeline
	if (perfect_forwarding) {
		bpp_string::add_code(code, add_newline);
		return;
	}

	nextline_buffer += code;
	join();
}

std::string bash_command_sequence::get_pre_code() const {
	return perfect_forwarding ? bpp_string::get_pre_code() : "";
}

std::string bash_command_sequence::get_post_code() const {
	return perfect_forwarding ? bpp_string::get_post_code() : "";
}

std::string bash_command_sequence::get_code() const {
	return perfect_forwarding ? bpp_string::get_code() : joined_code;
}

void bash_command_sequence::set_perfect_forwarding(bool enable) {
	perfect_forwarding = enable;
}

} // namespace bpp
