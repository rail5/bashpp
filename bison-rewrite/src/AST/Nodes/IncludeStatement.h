#pragma once

#include "../ASTNode.h"
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
		IncludeKeyword m_KEYWORD;

		IncludeType m_TYPE;

		PathType m_PATHTYPE;

		std::string m_PATH;

		std::optional<std::string> m_ASPATH;
	public:
		IncludeStatement() {
			type = AST::NodeType::IncludeStatement;
		}

		IncludeKeyword KEYWORD() const {
			return m_KEYWORD;
		}

		void setKeyword(IncludeKeyword keyword) {
			m_KEYWORD = keyword;
		}

		IncludeType TYPE() const {
			return m_TYPE;
		}

		void setType(IncludeType type) {
			m_TYPE = type;
		}

		PathType PATHTYPE() const {
			return m_PATHTYPE;
		}

		void setPathType(PathType pathtype) {
			m_PATHTYPE = pathtype;
		}

		const std::string& PATH() const {
			return m_PATH;
		}

		void setPath(const std::string& path) {
			m_PATH = path;
		}

		const std::optional<std::string>& ASPATH() const {
			return m_ASPATH;
		}

		void setAsPath(const std::string& aspath) {
			if (!aspath.empty()) m_ASPATH = aspath;
		}

		void clearAsPath() {
			m_ASPATH = std::nullopt;
		}

		std::ostream& prettyPrint(std::ostream& os, int indentation_level = 0) const override {
			std::string indent(indentation_level * 2, ' ');
			std::string keyword_string = (m_KEYWORD == IncludeKeyword::INCLUDE) ? "include" : "include_once";
			std::string type_string = (m_TYPE == IncludeType::STATIC) ? "static" : "dynamic";
			std::string path_string = (m_PATHTYPE == PathType::ANGLEBRACKET) ? "<" + m_PATH + ">" : "\"" + m_PATH + "\"";
			std::string aspath_string = m_ASPATH.has_value() ? " as \"" + m_ASPATH.value() + "\"" : "";

			return os << indent << "(IncludeStatement\n"
				<< indent << "  @" << keyword_string << " " << type_string << " " << path_string << aspath_string << "\n"
				<< indent << ")" << std::endl;
		}
};

} // namespace AST
