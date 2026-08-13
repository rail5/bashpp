/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <AST/ASTNode.h>

namespace bpp::AST {

class ObjectReference : public ASTNode {
	protected:
		std::vector<AST::Token<std::string>> m_IDENTIFIERS = {AST::Token<std::string>()};
		bool m_has_hashkey = false;
		bool m_lvalue = false;
		bool m_self_reference = false;
		bool m_ptr_dereference = false;
		bool m_address_of = false;
	public:
		constexpr ObjectReference() : ASTNode(bpp::AST::NodeType::ObjectReference) {}

		void setRootIdentifier(const AST::Token<std::string>& identifier) {
			if (m_IDENTIFIERS.empty()) {
				m_IDENTIFIERS.push_back(identifier);
			} else {
				m_IDENTIFIERS[0] = identifier;
			}
		}

		void addIdentifier(const AST::Token<std::string>& identifier) {
			m_IDENTIFIERS.push_back(identifier);
		}
		const std::vector<AST::Token<std::string>>& IDENTIFIERS() const {
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

		void setPointerDereference(bool ptr_dereference) {
			m_ptr_dereference = ptr_dereference;
		}
		bool isPointerDereference() const {
			return m_ptr_dereference;
		}

		void setAddressOf(bool address_of) {
			m_address_of = address_of;
		}
		bool isAddressOf() const {
			return m_address_of;
		}

		PRETTYPRINT_OVERRIDE({
			std::string indent(indentation_level * PRETTYPRINT_INDENTATION_AMOUNT, ' ');
			os << indent << "(ObjectReference ["
				<< (m_lvalue ? "lvalue" : "rvalue")
				<< (m_self_reference ? ", self" : "")
				<< "]\n"
				<< indent << "  "
				<< (m_address_of ? "&" : "")
				<< (m_ptr_dereference ? "*" : "")
				<< "@";

			if (m_has_hashkey) {
				os << "#";
			}

			for (auto it = m_IDENTIFIERS.begin(); it != m_IDENTIFIERS.end(); ++it) {
				os << it->getValue();
				if (std::next(it) != m_IDENTIFIERS.end()) os << ".";
			}

			for (const auto& child : children) {
				os << std::endl;
				child->prettyPrint(os, indentation_level + 1);
			}
			os << ")" << std::flush;
			return os;
		})
};

} // namespace bpp::AST
