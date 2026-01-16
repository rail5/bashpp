/**
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 */

#pragma once

#include "../ASTNode.h"
#include "RawText.h"

namespace AST {

/**
 * @class StringType
 * @brief Base class for string-type nodes in the AST
 *
 * This class is only to be used as a common ancestor for string-related nodes, like DoublequotedString.
 * It should not be used directly to create AST nodes.
 * 
 */
class StringType : public ASTNode {
	public:
		StringType() {
			type = AST::NodeType::ERROR_TYPE;
		}

		/**
		 * @brief Adds text to the string, either by appending to the last RawText child or creating a new one.
		 * The result should be that the node contains an alternating sequence of RawText nodes and interpolations,
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

		std::ostream& prettyPrint(std::ostream& os, int indentation_level = 0) const = 0; // Pure virtual, to prevent direct instantiation
};

} // namespace AST
