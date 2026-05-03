/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <AST/ASTNode.h>

namespace AST {

class SubshellSubstitution : public ASTNode {
	protected:
		bool is_cat_replacement = false;
	public:
		static constexpr AST::NodeType static_type = AST::NodeType::SubshellSubstitution;
		constexpr AST::NodeType getType() const override { return static_type; }

		void setIsCatReplacement(bool value) {
			is_cat_replacement = value;
		}
		bool isCatReplacement() const {
			return is_cat_replacement;
		}

		std::ostream& prettyPrint(std::ostream& os, size_t indentation_level = 0) const override {
			std::string indent(indentation_level * PRETTYPRINT_INDENTATION_AMOUNT, ' ');
			os << indent << "(SubshellSubstitution ";
			if (is_cat_replacement) os << "[cat replacement] ";
			os << "$(";
			for (const auto& child : children) {
				os << std::endl;
				child->prettyPrint(os, indentation_level + 1);
			}
			os << "))" << std::flush;
			return os;
		}
};

} // namespace AST
