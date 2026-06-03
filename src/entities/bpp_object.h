/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <string>
#include <memory>

#include "bpp.h"
#include "bpp_entity.h"

namespace bpp {

/**
 * @class bpp_object
 * 
 * @brief An object in Bash++
 */
class bpp_object : public bpp_entity {
	protected:
		std::string address;
		std::string assignment_value;
		std::string pre_access_code;
		std::string post_access_code;
		bool m_is_pointer = false;
		std::shared_ptr<bpp::bpp_object> copy_from = nullptr;
	public:
		void set_class(std::shared_ptr<bpp_class> object_class);
		void set_pointer(bool is_pointer);
		void set_address(const std::string& address);
		void set_assignment_value(const std::string& assignment_value);
		void set_pre_access_code(const std::string& pre_access_code);
		void set_post_access_code(const std::string& post_access_code);
		void set_nullptr();

		std::string get_address() const override;
		std::string get_assignment_value() const;
		std::string get_pre_access_code() const;
		std::string get_post_access_code() const;
		std::shared_ptr<bpp::bpp_object> get_copy_from() const;

		bool is_pointer() const;
};

} // namespace bpp
