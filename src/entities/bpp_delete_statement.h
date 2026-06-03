/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <memory>

#include "bpp.h"
#include "bpp_string.h"

namespace bpp {

/**
 * @class bpp_delete_statement
 * 
 * @brief A delete statement in Bash++
 * 
 * This entity gets pushed onto the entity stack when we encounter a '@delete' statement in Bash++ code.
 * 
 * It contains a pointer to the object that we intend to delete
 * 
 * The 'bpp_' prefix signifies that this is used to parse a statement type which is unique to Bash++
 */
class bpp_delete_statement : public bpp_string {
	private:
		std::shared_ptr<bpp::bpp_object> object_to_delete;
	public:
		void set_object_to_delete(std::shared_ptr<bpp::bpp_object> object);
		std::shared_ptr<bpp::bpp_object> get_object_to_delete() const;
};

} // namespace bpp
