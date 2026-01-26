/**
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 */

#pragma once

#include <AST/ASTNode.h>
#include <AST/AccessModifier.h>
#include <optional>

namespace AST {

class MethodDefinition : public ASTNode {
	public:
		struct Parameter {
			std::optional<AST::Token<std::string>> type;
			AST::Token<std::string> name;
			bool pointer = true; // Required. Methods cannot accept non-primitive objects as arguments
		};
	protected:
		bool m_VIRTUAL = false;
		AST::Token<AccessModifier> m_ACCESSMODIFIER;
		AST::Token<std::string> m_NAME;
		std::vector<AST::Token<Parameter>> m_PARAMETERS;

	public:
		static constexpr AST::NodeType static_type = AST::NodeType::MethodDefinition;
		constexpr AST::NodeType getType() const override { return static_type; }

		const AST::Token<std::string>& NAME() const {
			return m_NAME;
		}
		void setName(const AST::Token<std::string>& name) {
			m_NAME = name;
		}

		bool VIRTUAL() const {
			return m_VIRTUAL;
		}
		void setVirtual(bool is_virtual) {
			m_VIRTUAL = is_virtual;
		}

		const AST::Token<AccessModifier>& ACCESSMODIFIER() const {
			return m_ACCESSMODIFIER;
		}
		void setAccessModifier(const AST::Token<AccessModifier>& accessmodifier) {
			m_ACCESSMODIFIER = accessmodifier;
		}

		const std::vector<AST::Token<Parameter>>& PARAMETERS() const {
			return m_PARAMETERS;
		}
		void addParameter(const AST::Token<Parameter>& parameter) {
			m_PARAMETERS.push_back(parameter);
		}
		void addParameters(const std::vector<AST::Token<Parameter>>& parameters) {
			m_PARAMETERS.insert(m_PARAMETERS.end(), parameters.begin(), parameters.end());
		}

		std::ostream& prettyPrint(std::ostream& os, int indentation_level = 0) const override {
			std::string indent(indentation_level * PRETTYPRINT_INDENTATION_AMOUNT, ' ');
			os << indent << "(MethodDefinition\n"
				<< indent << "  " << (m_VIRTUAL ? "@virtual " : "");
			switch (m_ACCESSMODIFIER) {
				case AccessModifier::PUBLIC:
					os << "@public ";
					break;
				case AccessModifier::PROTECTED:
					os << "@protected ";
					break;
				case AccessModifier::PRIVATE:
					os << "@private ";
					break;
			}
			os << "@method " << m_NAME;
			for (const auto& param : m_PARAMETERS) {
				os << " ";
				if (param.getValue().type.has_value()) {
					os << "@" << param.getValue().type.value() << (param.getValue().pointer ? "*" : "") << " ";
				}
				os << param.getValue().name;
			}
			for (const auto& child : children) {
				os << std::endl;
				child->prettyPrint(os, indentation_level + 1);
			}
			os << ")" << std::flush;
			return os;
		}
};

} // namespace AST
