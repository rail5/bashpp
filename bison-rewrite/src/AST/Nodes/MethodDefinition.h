/**
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 */

#pragma once

#include "../ASTNode.h"
#include "../AccessModifier.h"
#include <optional>

namespace AST {

class MethodDefinition : public ASTNode {
	public:
		struct Parameter {
			std::optional<std::string> type;
			std::string name;
			bool pointer = true; // Required. Methods cannot accept non-primitive objects as arguments
		};
	protected:
		bool m_VIRTUAL = false;
		AccessModifier m_ACCESSMODIFIER;
		std::string m_NAME;
		std::vector<Parameter> m_PARAMETERS;

	public:
		MethodDefinition() {
			type = AST::NodeType::MethodDefinition;
		}

		const std::string& NAME() const {
			return m_NAME;
		}
		void setName(const std::string& name) {
			m_NAME = name;
		}

		bool VIRTUAL() const {
			return m_VIRTUAL;
		}
		void setVirtual(bool is_virtual) {
			m_VIRTUAL = is_virtual;
		}

		AccessModifier ACCESSMODIFIER() const {
			return m_ACCESSMODIFIER;
		}
		void setAccessModifier(AccessModifier accessmodifier) {
			m_ACCESSMODIFIER = accessmodifier;
		}

		const std::vector<Parameter>& PARAMETERS() const {
			return m_PARAMETERS;
		}
		void addParameter(const Parameter& parameter) {
			m_PARAMETERS.push_back(parameter);
		}
		void addParameters(const std::vector<Parameter>& parameters) {
			m_PARAMETERS.insert(m_PARAMETERS.end(), parameters.begin(), parameters.end());
		}

		std::ostream& prettyPrint(std::ostream& os, int indentation_level = 0) const override {
			std::string indent(indentation_level * 2, ' ');
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
				if (param.type.has_value()) {
					os << "@" << param.type.value() << (param.pointer ? "*" : "") << " ";
				}
				os << param.name;
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
