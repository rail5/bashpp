/**
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * Licensed under the GNU General Public License v3.0 or later (GPL-3.0-or-later)
 */

#pragma once

#include <AST/ASTNode.h>
#include <optional>

namespace AST {

class IncludeStatement : public ASTNode {
	public:
		enum class IncludeKeyword {
			INCLUDE,
			INCLUDE_ONCE
		};

		enum class IncludeType {
			STATIC,
			DYNAMIC
		};

		enum class PathType {
			ANGLEBRACKET,
			QUOTED
		};
	protected:
		AST::Token<IncludeKeyword> m_KEYWORD;
		AST::Token<IncludeType> m_TYPE;
		PathType m_PATHTYPE;
		AST::Token<std::string> m_PATH;
		std::optional<AST::Token<std::string>> m_ASPATH;
	public:
		static constexpr AST::NodeType static_type = AST::NodeType::IncludeStatement;
		constexpr AST::NodeType getType() const override { return static_type; }

		const AST::Token<IncludeKeyword>& KEYWORD() const {
			return m_KEYWORD;
		}

		void setKeyword(const AST::Token<IncludeKeyword>& keyword) {
			m_KEYWORD = keyword;
		}

		const AST::Token<IncludeType>& TYPE() const {
			return m_TYPE;
		}

		void setType(const AST::Token<IncludeType>& type) {
			m_TYPE = type;
		}

		const PathType& PATHTYPE() const {
			return m_PATHTYPE;
		}

		void setPathType(const PathType& pathtype) {
			m_PATHTYPE = pathtype;
		}

		const AST::Token<std::string>& PATH() const {
			return m_PATH;
		}

		void setPath(const AST::Token<std::string>& path) {
			m_PATH = path;
		}

		const std::optional<AST::Token<std::string>>& ASPATH() const {
			return m_ASPATH;
		}

		void setAsPath(const AST::Token<std::string>& aspath) {
			if (!aspath.getValue().empty()) m_ASPATH = aspath;
		}

		void clearAsPath() {
			m_ASPATH = std::nullopt;
		}

		std::ostream& prettyPrint(std::ostream& os, int indentation_level = 0) const override {
			std::string indent(indentation_level * PRETTYPRINT_INDENTATION_AMOUNT, ' ');
			os << indent << "(IncludeStatement\n"
				<< indent << "  @" << ((m_KEYWORD.getValue() == IncludeKeyword::INCLUDE) ? "include" : "include_once") << " "
				<< ((m_TYPE.getValue() == IncludeType::STATIC) ? "static" : "dynamic") << " "
				<< ((m_PATHTYPE == PathType::ANGLEBRACKET) ? "<" : "\"") << m_PATH
				<< ((m_PATHTYPE == PathType::ANGLEBRACKET) ? ">" : "\"");
			if (m_ASPATH.has_value()) {
				os << " as \"" << m_ASPATH.value() << "\"";
			}
			os << ")" << std::flush;
			return os;
		}
};

} // namespace AST
