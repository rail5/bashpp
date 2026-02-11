/**
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * Licensed under the GNU General Public License v3.0 or later (GPL-3.0-or-later)
 */

#pragma once

#include <cstdint>
#include <string>
#include <iostream>

/**
 * @class ParserPosition
 * @brief Represents a single point in a source file
 *
 * This is roughly an imitation of the default Bison parser position type,
 * except that it uses unsigned integers for line and column numbers (since they'll never be negative)
 * and those numbers are 0-indexed (where, in Bison's default, they are 1-indexed).
 * 
 */
struct ParserPosition {
	const std::string* filename;
	uint32_t line = 0;
	uint32_t column = 0;

	explicit ParserPosition(const std::string* f = nullptr, uint32_t l = 0, uint32_t c = 0)
		: filename(f), line(l), column(c) {}

	inline void initialize(const std::string* fn = nullptr, uint32_t l = 0, uint32_t c = 0) {
		filename = fn;
		line = l;
		column = c;
	}

	/**
	 * @brief Increment the line number by count, resetting the column to 0
	 * 
	 * @param count The number of lines to advance
	 */
	inline void lines(uint32_t count = 1) {
		if (count) {
			column = 0;
			line += count;
		}
	}

	/**
	 * @brief Increment the column number by count
	 * 
	 * @param count The number of columns to advance
	 */
	inline void columns(uint32_t count = 1) {
		column += count;
	}
};

inline ParserPosition& operator+=(ParserPosition& lhs, uint32_t rhs) {
	lhs.columns(rhs);
	return lhs;
}

inline ParserPosition operator+(ParserPosition lhs, uint32_t rhs) {
	return lhs += rhs;
}

inline std::ostream& operator<<(std::ostream& os, const ParserPosition& pos) {
	if (pos.filename) {
		os << *(pos.filename) << ':';
	}
	return os << pos.line << '.' << pos.column;
}

/**
 * @class ParserLocation
 * @brief Represents a range in a source file, from a start position to an end position
 * 
 */
struct ParserLocation {
	ParserPosition begin;
	ParserPosition end;

	/**
	 * @brief Construct location from 'begin' to 'end' positions
	 * 
	 * @param b begin position
	 * @param e end position
	 */
	explicit ParserLocation(const ParserPosition& b, const ParserPosition& e)
		: begin(b), end(e) {}
	
	/**
	 * @brief Construct a 0-width location at position 'p'
	 * 
	 * @param p position
	 */
	explicit ParserLocation(const ParserPosition& p = ParserPosition())
		: begin(p), end(p) {}
	
	/**
	 * @brief Construct a 0-width location at file 'f', line 'l', column 'c'
	 *
	 * @param f filename
	 * @param l line number
	 * @param c column number
	 * 
	 */
	explicit ParserLocation(const std::string* f, uint32_t l = 0, uint32_t c = 0)
		: begin(f, l, c), end(f, l, c) {}
	
	inline void initialize(const std::string* f = nullptr, uint32_t l = 0, uint32_t c = 0) {
		begin.initialize(f, l, c);
		end = begin;
	}

	/**
	 * @brief Reset initial position to the end position
	 * 
	 */
	inline void step() {
		begin = end;
	}

	inline void columns(uint32_t count = 1) {
		end.columns(count);
	}

	inline void lines(uint32_t count = 1) {
		end.lines(count);
	}
};

inline ParserLocation& operator+=(ParserLocation& lhs, uint32_t rhs) {
	lhs.columns(rhs);
	return lhs;
}

inline ParserLocation operator+(ParserLocation lhs, uint32_t rhs) {
	return lhs += rhs;
}

inline std::ostream& operator<<(std::ostream& os, const ParserLocation& loc) {
	uint32_t end_col = 0 < loc.end.column ? loc.end.column - 1 : 0;
	os << loc.begin;
	if (loc.end.filename && (!loc.begin.filename || *loc.begin.filename != *loc.end.filename)) {
		os << '-' << *(loc.end.filename) << ':' << loc.end.line << '.' << end_col;
	} else if (loc.begin.line < loc.end.line) {
		os << '-' << loc.end.line << '.' << end_col;
	} else if (loc.begin.column < end_col) {
		os << '-' << end_col;
	}
	return os;
}
