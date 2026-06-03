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
* @class bpp_string
* 
* @brief The practical difference between bpp_code_entity and bpp_string is how we handle the code buffers.
*
* When you call add_code() on a bpp_code_entity, it will flush the pre_code and post_code buffers.
*
* Meaning that we're treating the pre_code and post_code buffers as the pre- and post-code for *this particular line* of code.
* So when we finish writing this line of code, we should ensure that this line of code is preceded by its pre-code, and followed by its post-code
* 
* In a bpp_code_entity, the code:
*
* 	echo @this.dataMember
*
* Becomes:
*
* 	{pre-code necessary to fetch @this.dataMember}
* 	echo ${the resolved reference}
* 	{post-code necessary to clear the memory}
* 
* In a bpp_string, however, calling add_code() does not ever flush those buffers.
* 	Why?
*
* Because, if you imagine we're literally dealing with an actual string (although the same rules apply in some other cases),
* 	it's important not to muddy up the contents of the string with a bunch of pre- and post-code.
* 	If, for example, there are object references within the string, they should be resolved BEFORE the entire string,
* 	And cleared AFTER the entire string
* 
* This ensures that for a bpp_string, the code:
*
* 	echo "This is a very long string
* 		That spans multiple lines
* 		And has a reference to @this.dataMember"
*
* Becomes:
*
* 	{pre-code necessary to fetch @this.dataMember}
* 	echo "This is a very long string
* 		That spans multiple lines
* 		And has a reference to ${the resolved reference}"
* 	{post-code necessary to clear the memory}
* 
* So, bpp_string gives us more direct control over where the pre- and post-code is added
* 
* In some other part of the compiler where we're parsing the above example string,
* the code might look something like:
*
* 	entity->add_code_to_previous_line(pre_code);
* 	entity->add_code_to_next_line(post_code);
* 		^ These two lines prepare the code buffers in the code_entity
* 	entity->add_code("echo \"This is a very long string....."); // etc
* 		^ This line adds the code & *maybe* flushes the buffers we just prepared
*
* Where "entity" may be a bpp_code_entity or a bpp_string
* 
* In general, parser rules which handle things such as object references
* 	don't pay attention to whether they're dealing with a bpp_code_entity or a bpp_string.
* The code necessary to fetch an object reference (for example) is simply added to the pre-code buffer,
* 	and the code necessary to clear that memory is simply added to the post-code buffer.
* The question of whether or when those buffers should be flushed is left to the entity itself (code_entity or string).
*/
class bpp_string : public bpp_code_entity {
	public:
		void add_code(const std::string& code, bool add_newline = true) override;
		void add_code_to_previous_line(const std::string& code) override;
		void add_code_to_next_line(const std::string& code) override;

		std::string get_code() const override;
		std::string get_pre_code() const override;
		std::string get_post_code() const override;
};

} // namespace bpp
