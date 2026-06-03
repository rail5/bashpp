/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <vector>
#include <memory>
#include <string>

#include "bpp.h"
#include "bpp_code_entity.h"

namespace bpp {

/**
 * @class bpp_method
 * 
 * @brief A method in a class
 */
class bpp_method : public bpp_code_entity {
	private:
		std::vector<std::shared_ptr<bpp_method_parameter>> parameters;
		bpp_scope scope = bpp_scope::SCOPE_PUBLIC;
		bool m_is_virtual = false;
		bool m_is_overridable = false;
		bool inherited = false;
		bool add_object_as_parameter(std::shared_ptr<bpp_object> object);
		std::string last_override; // Name of the latest class to override this virtual method
	public:
		virtual bool add_parameter(std::shared_ptr<bpp_method_parameter> parameter);
		void set_scope(bpp_scope scope);
		void set_virtual(bool is_virtual);
		void set_overridable(bool is_overridable);
		void set_inherited(bool is_inherited);
		void set_last_override(const std::string& class_name);
		void set_overridden_method(std::weak_ptr<bpp_method> method);
		bool add_object(std::shared_ptr<bpp_object> object, bool make_local) override;

		std::vector<std::shared_ptr<bpp_method_parameter>> get_parameters() const;
		bpp_scope get_scope() const;
		bool is_virtual() const;
		bool is_overridable() const;
		bool is_inherited() const;
		std::string get_last_override() const;
};

/**
 * @class bpp_method_parameter
 * 
 * @brief A parameter in a method
 */
class bpp_method_parameter : public bpp_entity {
	public:
		explicit bpp_method_parameter(const std::string& name);

		void set_class(std::shared_ptr<bpp_class>);
};

/**
 * @var inaccessible_method
 * @brief A placeholder for an inaccessible method of a class (scope handling)
 */
inline const std::shared_ptr<bpp_method> inaccessible_method = std::make_shared<bpp_method>();

} // namespace bpp
