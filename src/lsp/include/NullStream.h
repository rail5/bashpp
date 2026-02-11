/**
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * Licensed under the GNU General Public License v3.0 or later (GPL-3.0-or-later)
 */

#pragma once

#include <streambuf>
#include <ostream>
#include <sstream>

class NullBuffer : public std::streambuf {
	protected:
		int overflow(int c) override {
			return traits_type::not_eof(c); // Indicate success, but discard the character
		}
};

class NullOStream : public std::ostream {
	private:
		NullBuffer null_buffer;
	public:
		NullOStream() : std::ostream(&null_buffer) {}
};

class NullOStringStream : public std::ostringstream {
	public:
		NullOStringStream() = default;

		template <typename T>
		NullOStringStream& operator<<(const T& value) {
			// Do nothing, effectively discarding the output
			return *this;
		}

		NullOStringStream& write(const char* s, std::streamsize n) {
			// Do nothing, effectively discarding the output
			return *this;
		}

		// Override str() to return an empty string
		inline std::string str() const {
			return ""; // Always return an empty string
		}
};

