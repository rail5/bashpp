#pragma once

#include "../ASTNode.h"
#include "RawText.h"

namespace AST {

class DoublequotedString : public ASTNode {
	public:
		DoublequotedString() {
			type = AST::NodeType::DoublequotedString;
		}

		/**
		 * @brief Adds text to the double-quoted string, either by appending to the last RawText child or creating a new one.
		 * The result should be that the DoublequotedString node contains an alternating sequence of RawText nodes and interpolations,
		 * each RawText node holding a contiguous segment of text.
		 * 
		 * @param text The text to add.
		 */
		void addText(const std::string& text) {
			auto lastChild = getLastChild();
			if (lastChild && lastChild->getType() == AST::NodeType::RawText) {
				std::dynamic_pointer_cast<AST::RawText>(lastChild)->appendText(text);
			} else {
				auto rawTextNode = std::make_shared<AST::RawText>();
				rawTextNode->setText(text);
				addChild(rawTextNode);
			}
		}

		std::ostream& prettyPrint(std::ostream& os, int indentation_level = 0) const override {
			std::string indent(indentation_level * 2, ' ');
			os << indent << "(DoublequotedString \"\n";
			for (const auto& child : children) {
				child->prettyPrint(os, indentation_level + 1);
			}
			os << indent << "\")" << std::endl;
			return os;
		}
};

} // namespace AST
