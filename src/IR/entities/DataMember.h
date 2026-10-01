/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <IR/bpp.h>
#include <IR/entities/Entity.h>
#include <IR/entities/Object.h>
#include <IR/entities/components/ClassMember.h>

namespace bpp::IR {

/**
 * @brief A data member in a class
 *
 * Although this inherits from Object, it can also be a primitive.
 * The case in which the data member is a primitive is represented by type == nullptr.
 */
class DataMember : public Object, public Components::ClassMember {
	private:
		bool m_is_array = false;
	public:
		/// Addresses of data members can only be returned as suffixes to be appended to the address of the containing object.
		std::string getAddress() const override { return "__" + getName(); }

		void setIsArray(bool is_array) { this->m_is_array = is_array; }
		bool isArray() const { return m_is_array; }

		void setParentDatamember(const DataMember* parent_datamember) { setParentMember(parent_datamember); }
		const DataMember* getParentDatamember() const { return static_cast<const DataMember*>(getParentMember()); }

		PRETTYPRINT_OVERRIDE();
};

} // namespace bpp::IR
