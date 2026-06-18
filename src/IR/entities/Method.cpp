/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "Method.h"

#include <IR/entities/Object.h>

namespace bpp::IR {

bool Method::add_parameter(std::shared_ptr<Object> parameter) {
	for (const auto& p : parameters) {
		if (p->get_name() == parameter->get_name()) return false; // Parameter with this name already exists
	}

	parameters.push_back(parameter);
	return true;
}

std::ostream& Method::prettyPrint(std::ostream& os, size_t indentation_level) const {
	std::string indent(indentation_level * 4, ' ');
	os << indent << "(Method: " << name;
	switch (scope) {
		case VisibilityScope::INACCESSIBLE: os << ", scope: INACCESSIBLE"; break;
		case VisibilityScope::PUBLIC: os << ", scope: PUBLIC"; break;
		case VisibilityScope::PRIVATE: os << ", scope: PRIVATE"; break;
		case VisibilityScope::PROTECTED: os << ", scope: PROTECTED"; break;
	}
	os << ", is_virtual: " << (m_is_virtual ? "true" : "false")
		<< ", is_overridable: " << (m_is_overridable ? "true" : "false")
		<< ", is_inherited: " << (m_is_inherited ? "true" : "false")
		<< "\n";
	for (const auto& param : parameters) {
		param->prettyPrint(os, indentation_level + 1);
	}
	CodeEntity::prettyPrint(os, indentation_level + 1);
	os << indent << ")\n";
	return os;
}

} // namespace bpp::IR
