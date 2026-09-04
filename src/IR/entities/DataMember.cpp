/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "DataMember.h"

namespace bpp::IR {

void DataMember::addReferencePosition(const SymbolPosition& pos) {
	Entity::addReferencePosition(pos);
	if (auto parent = getParentDatamember()) {
		parent->addReferencePosition(pos);
	}
}

PRETTYPRINT_IMPLEMENTATION(DataMember, {
	std::string indent(indentation_level * PRETTYPRINT_INDENTATION_AMOUNT, ' ');

	os << indent;
	os << "(DataMember: " << getName() << " [";

	switch (getScope()) {
		case VisibilityScope::PUBLIC: os << "public"; break;
		case VisibilityScope::PRIVATE: os << "private"; break;
		case VisibilityScope::PROTECTED: os << "protected"; break;
		case VisibilityScope::INACCESSIBLE: os << "inaccessible"; break;
		default: os << "<error_scope>"; break;
	}

	if (isPrimitive()) {
		os << ", primitive";
		if (isArray()) os << ", array";
	} else {
		bpp_assert(!getType().expired(), "DataMember is not primitive but has no type");
		os << ", " << getType().lock()->getName();
		if (isPointer()) os << ", pointer";
	}

	os << "]";

	if (getInitialValue().has_value()) {
		os << "\n" << indent << std::string(PRETTYPRINT_INDENTATION_AMOUNT, ' ') << "=\n";
		getInitialValue().value()->prettyPrint(os, indentation_level + 1);
		os << indent;
	}

	os << ")\n";

	return os;
});

} // namespace bpp::IR
