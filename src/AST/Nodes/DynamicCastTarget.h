/**
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 */

#pragma once

#include <AST/ASTNode.h>
#include <optional>

namespace AST {

class DynamicCastTarget : public ASTNode {
	protected:
		std::optional<AST::Token<std::string>> m_TARGETTYPE;
	public:
		static constexpr AST::NodeType static_type = AST::NodeType::DynamicCastTarget;
		constexpr AST::NodeType getType() const override { return static_type; }

		void setTargetType(const AST::Token<std::string>& target_type) {
			m_TARGETTYPE = target_type;
		}
		const std::optional<AST::Token<std::string>>& TARGETTYPE() const {
			return m_TARGETTYPE;
		}

		std::ostream& prettyPrint(std::ostream& os, int indentation_level = 0) const override {
			std::string indent(indentation_level * PRETTYPRINT_INDENTATION_AMOUNT, ' ');
			os << indent << "(DynamicCastTarget"
				<< (m_TARGETTYPE.has_value() ? " " + m_TARGETTYPE->getValue() : "");
			for (const auto& child : children) {
				os << std::endl;
				child->prettyPrint(os, indentation_level + 1);
			}
			os << ")" << std::flush;
			return os;
		}
};

} // namespace AST
