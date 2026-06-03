/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <string>

#include "bpp.h"
#include "bpp_object.h"

namespace bpp {

/**
 * @class bpp_datamember
 * 
 * @brief A data member in a class
 */
class bpp_datamember : public bpp_object {
	private:
		std::string default_value;
		bpp_scope scope = bpp_scope::SCOPE_PRIVATE;
		bool array = false;
	public:
		void set_default_value(const std::string& default_value);
		void set_scope(bpp_scope scope);
		void set_array(bool is_array);

		std::string get_address() const override;
		std::string get_default_value() const;
		bpp_scope get_scope() const;
		bool is_array() const;
};


/**
 * @var inaccessible_datamember
 * @brief A placeholder for an inaccessible data member of a class (scope handling)
 */
inline const std::shared_ptr<bpp_datamember> inaccessible_datamember = std::make_shared<bpp_datamember>();

} // namespace bpp
