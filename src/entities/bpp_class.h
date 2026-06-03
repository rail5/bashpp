/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <memory>
#include <vector>
#include <string>

#include "bpp.h"
#include "bpp_entity.h"

namespace bpp {

/**
 * @class bpp_class
 * 
 * @brief A class in Bash++
 */
class bpp_class : public bpp_entity, public std::enable_shared_from_this<bpp_class> {
	private:
		std::vector<std::shared_ptr<bpp_method>> methods;
		std::vector<std::shared_ptr<bpp_datamember>> datamembers;
		bool has_custom_toPrimitive = false;

		void remove_default_toPrimitive();
		void add_default_toPrimitive();
	public:
		std::weak_ptr<bpp_class> get_containing_class() override;
		bool set_containing_class(std::weak_ptr<bpp::bpp_class> containing_class) override;

		std::shared_ptr<bpp_class> get_class() override;

		void set_name(const std::string& name) override;
		bool add_method(std::shared_ptr<bpp_method> method);
		bool add_datamember(std::shared_ptr<bpp_datamember> datamember);

		const std::vector<std::shared_ptr<bpp_method>>& get_methods() const;
		const std::vector<std::shared_ptr<bpp_datamember>>& get_datamembers() const;

		std::shared_ptr<bpp_method> get_method(const std::string& name, std::shared_ptr<bpp_entity> context);
		std::shared_ptr<bpp_method> get_method_UNSAFE(const std::string& name);
		std::shared_ptr<bpp_datamember> get_datamember(const std::string& name, std::shared_ptr<bpp_entity> context);
		std::shared_ptr<bpp_datamember> get_datamember_UNSAFE(const std::string& name);

		using bpp_entity::inherit;
		void inherit(std::shared_ptr<bpp_class> parent) override;
		std::shared_ptr<bpp::bpp_class> get_parent();

		std::vector<std::shared_ptr<bpp_class>> get_all_known_classes() const override;
};

} // namespace bpp
