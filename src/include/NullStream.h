/**
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * Licensed under the GNU General Public License v3.0 or later (GPL-3.0-or-later)
 */

#pragma once

#include <streambuf>
#include <ostream>

/**
 * @class NullBuffer
 * @brief A stream buffer that discards all output
 * 
 */
class NullBuffer : public std::streambuf {
	protected:
		int overflow(int c) override {
			return traits_type::not_eof(c); // Indicate success, but discard the character
		}

		std::streamsize xsputn(const char* /*s*/, std::streamsize n) override {
			return n; // Indicate success, but discard the characters
		}
};

/**
 * @class NullOStream
 * @brief An output stream that discards all output
 * This is used in the language server to parse programs without generating any compiled code,
 * by setting the Listener's code_buffer to an instance of NullOStream.
 *
 * In the future, it's likely that AST traversal will be separated from code generation,
 * which will eliminate the need for this class.
 */
class NullOStream : public std::ostream {
	private:
		inline static NullBuffer null_buffer;
	public:
		NullOStream() : std::ostream(&null_buffer) {}
};
