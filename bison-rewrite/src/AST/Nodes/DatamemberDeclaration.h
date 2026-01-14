#pragma once

#include "../ASTNode.h"
#include <optional>

namespace AST {

class DatamemberDeclaration : public ASTNode {
	public:
		enum class AccessModifier {
			PUBLIC,
			PROTECTED,
			PRIVATE
		};
	protected:
		AccessModifier m_ACCESSMODIFIER;
		std::optional<std::string> m_TYPE;
		std::optional<std::string> m_IDENTIFIER;

	public:
		DatamemberDeclaration() {
			type = AST::NodeType::DatamemberDeclaration;
		}

		AccessModifier ACCESSMODIFIER() const {
			return m_ACCESSMODIFIER;
		}

		void setAccessModifier(AccessModifier accessmodifier) {
			m_ACCESSMODIFIER = accessmodifier;
		}

		const std::optional<std::string>& TYPE() const {
			return m_TYPE;
		}

		void setType(const std::string& type) {
			if (!type.empty()) m_TYPE = type;
		}

		void clearType() {
			m_TYPE = std::nullopt;
		}

		const std::optional<std::string>& IDENTIFIER() const {
			return m_IDENTIFIER;
		}

		void setIdentifier(const std::string& identifier) {
			if (!identifier.empty()) m_IDENTIFIER = identifier;
		}

		void clearIdentifier() {
			m_IDENTIFIER = std::nullopt;
		}

		std::ostream& prettyPrint(std::ostream& os, int indentation_level = 0) const override {
			std::string indent(indentation_level * 2, ' ');
			os << indent << "(DatamemberDeclaration\n"
				<< indent << "  ";
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
			if (m_TYPE.has_value()) {
				os << "@" << m_TYPE.value() << " ";
			}
			if (m_IDENTIFIER.has_value()) {
				os << m_IDENTIFIER.value();
			}
			os << "\n";
			for (const auto& child : children) {
				child->prettyPrint(os, indentation_level + 1);
			}
			os << indent << ")" << std::endl;
			return os;
		}
};

} // namespace AST
