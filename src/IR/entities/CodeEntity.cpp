/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "CodeEntity.h"

namespace bpp::IR {

void CodeEntity::add(const RawCode& child) {
	// If the previous child is also raw code, merge this raw code with the previous one.
	if (!children.empty() && std::holds_alternative<RawCode>(children.back())) {
		std::get<RawCode>(children.back()) += child;
	} else {
		children.emplace_back(child);
	}
}

void CodeEntity::add(const std::shared_ptr<Entity>& child) {
	children.emplace_back(child);
}

std::ostream& CodeEntity::prettyPrint(std::ostream& os, size_t indentation_level) const {
	std::string indent(indentation_level * 4, ' ');
	os << indent << "(" << (name.empty() ? "" : name)
		<< "\n";
	for (const auto& child : children) {
		if (std::holds_alternative<RawCode>(child)) {
			auto str = std::get<RawCode>(child);
			// Replace all newlines in str with "\n"
			os << indent << "    ";
			for (const char c : str) {
				switch (c) {
					case '\n': os << "\\n"; break;
					case '\t': os << "\\t"; break;
					case '\r': os << "\\r"; break;
					default: os << c; break;
				}
			}
			os << "\n";
		} else if (std::holds_alternative<std::shared_ptr<Entity>>(child)) {
			std::get<std::shared_ptr<Entity>>(child)->prettyPrint(os, indentation_level + 1);
		}
	}
	os << indent << ")\n";
	return os;
}

} // namespace bpp::IR
