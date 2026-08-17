/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "DataMember.h"

namespace bpp::IR {

void DataMember::add_reference_position(const SymbolPosition& pos) {
	Entity::add_reference_position(pos);
	if (auto parent = parent_datamember.lock()) {
		parent->add_reference_position(pos);
	}
}

PRETTYPRINT_IMPLEMENTATION(DataMember, {
	std::string indent(indentation_level * PRETTYPRINT_INDENTATION_AMOUNT, ' ');

	os << indent;
	os << "(DataMember: " << get_name() << " [";

	switch (scope) {
		case VisibilityScope::PUBLIC: os << "public"; break;
		case VisibilityScope::PRIVATE: os << "private"; break;
		case VisibilityScope::PROTECTED: os << "protected"; break;
		case VisibilityScope::INACCESSIBLE: os << "inaccessible"; break;
		default: os << "<error_scope>"; break;
	}

	if (is_primitive()) {
		os << ", primitive";
		if (is_array()) os << ", array";
	} else {
		bpp_assert(!type.expired(), "DataMember is not primitive but has no type");
		os << ", " << type.lock()->get_name();
		if (is_pointer()) os << ", pointer";
	}

	os << "]";

	if (get_initial_value().has_value()) {
		os << "\n" << indent << std::string(PRETTYPRINT_INDENTATION_AMOUNT, ' ') << "=\n";
		get_initial_value().value()->prettyPrint(os, indentation_level + 1);
		os << indent;
	}

	os << ")\n";

	return os;
});

} // namespace bpp::IR
