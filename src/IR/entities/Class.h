/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <IR/bpp.h>
#include <IR/entities/Entity.h>

namespace bpp::IR {

template <typename T>
concept ClassMember = std::is_same_v<T, Method> || std::is_same_v<T, DataMember>;

class Class : public Entity, public std::enable_shared_from_this<Class> {
	private:
		std::string name;
		std::weak_ptr<Class> parent_class;

		std::vector<std::shared_ptr<Method>> methods;
		std::vector<std::shared_ptr<DataMember>> datamembers;

		template <ClassMember T>
		std::shared_ptr<T> get_member(const std::string& name, std::shared_ptr<Entity> context) const;
	public:
		Class() = delete;
		explicit Class(const std::string& name) : name(name) {}

		const std::string& get_name() const { return name; }
		void set_name(const std::string& name) { this->name = name; }

		std::weak_ptr<Class> get_containing_class() override { return weak_from_this(); }
		std::weak_ptr<const Class> get_containing_class_const() const override { return weak_from_this(); }

		bool add_method(std::shared_ptr<Method> method);
		bool add_datamember(std::shared_ptr<DataMember> datamember);

		std::shared_ptr<Method> get_method(const std::string& name, std::shared_ptr<Entity> context) const;
		std::shared_ptr<DataMember> get_datamember(const std::string& name, std::shared_ptr<Entity> context) const;
		std::shared_ptr<Method> get_method_UNSAFE(const std::string& name) const;
		std::shared_ptr<DataMember> get_datamember_UNSAFE(const std::string& name) const;

		const std::vector<std::shared_ptr<Method>>& get_methods() const { return methods; }
		const std::vector<std::shared_ptr<DataMember>>& get_datamembers() const { return datamembers; }

		using Entity::inherit;
		void inherit(std::shared_ptr<Class> parent);
		
		std::shared_ptr<Class> get_parent_class() const { return parent_class.lock(); }

		bool is_derived_from(std::shared_ptr<const Class> other) const;

		bpp::CodeGen::CodeSegment generate_code(bpp::CodeGen::CodeGenState* state) const override;

		PRETTYPRINT_OVERRIDE();
};

} // namespace bpp::IR
