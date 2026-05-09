/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <AST/ASTNode.h>

namespace AST {

class RequiresStatement : public ASTNode {
	protected:
		AST::Token<std::string> required_command;
		std::optional<AST::Token<std::string>> required_arguments = std::nullopt;
	public:
		static constexpr AST::NodeType static_type = AST::NodeType::RequiresStatement;
		constexpr AST::NodeType getType() const override { return static_type; }

		std::ostream& prettyPrint(std::ostream& os, size_t indentation_level = 0) const override {
			std::string indent(indentation_level * PRETTYPRINT_INDENTATION_AMOUNT, ' ');
			os << indent << "(RequiresStatement @requires";
			for (const auto& child : children) {
				os << std::endl;
				child->prettyPrint(os, indentation_level + 1);
			}
			os << ")" << std::flush;
			return os;
		}

		const AST::Token<std::string>& getRequiredCommand() const {
			return required_command;
		}
		void setRequiredCommand(const AST::Token<std::string>& command) {
			this->required_command = command;
		}

		const std::optional<AST::Token<std::string>>& getRequiredArguments() const {
			return required_arguments;
		}
		void setRequiredArguments(const AST::Token<std::string>& arguments) {
			if (!arguments.getValue().empty()) this->required_arguments = arguments;
		}
		bool hasRequiredArguments() const {
			return required_arguments.has_value();
		}
};

} // namespace AST
