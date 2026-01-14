#pragma once

#include "../ASTNode.h"

namespace AST {

class ObjectReference : public ASTNode {
	protected:
		std::string m_IDENTIFIER;
		std::vector<std::string> m_IDENTIFIERS;
		bool m_has_hashkey = false;
		bool m_lvalue = false;
		bool m_self_reference = false;
	public:
		ObjectReference() {
			type = AST::NodeType::ObjectReference;
		}

		void setIdentifier(const std::string& identifier) {
			m_IDENTIFIER = identifier;
		}
		const std::string& IDENTIFIER() const {
			return m_IDENTIFIER;
		}

		void addIdentifier(const std::string& identifier) {
			m_IDENTIFIERS.push_back(identifier);
		}
		const std::vector<std::string>& IDENTIFIERS() const {
			return m_IDENTIFIERS;
		}

		void setHasHashkey(bool has_hashkey) {
			m_has_hashkey = has_hashkey;
		}
		bool hasHashkey() const {
			return m_has_hashkey;
		}

		void setLvalue(bool lvalue) {
			m_lvalue = lvalue;
		}
		bool isLvalue() const {
			return m_lvalue;
		}

		void setSelfReference(bool self_reference) {
			m_self_reference = self_reference;
		}
		bool isSelfReference() const {
			return m_self_reference;
		}

		std::ostream& prettyPrint(std::ostream& os, int indentation_level = 0) const override {
			std::string indent(indentation_level * 2, ' ');
			os << indent << "(ObjectReference [";

			os << (m_lvalue ? "lvalue" : "rvalue");
			os << (m_self_reference ? ", self" : "");

			os << "]\n"
				<< indent << "  @";

			if (m_has_hashkey) {
				os << "#";
			}

			os << m_IDENTIFIER;
			for (const auto& id : m_IDENTIFIERS) {
				os << "." << id;
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
