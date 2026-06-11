/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <AST/ASTNode.h>
#include <AST/Nodes/RawText.h>
#include <cstdint>

namespace bpp::AST {

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
		constexpr explicit StringType(bpp::AST::NodeType type) : ASTNode(type) {}
		/**
		 * @brief Adds text to the string, either by appending to the last RawText child or creating a new one.
		 * The result should be that the node contains an alternating sequence of RawText nodes and interpolations,
		 * each RawText node holding a contiguous segment of text.
		 * 
		 * @param text The text to add.
		 */
		void addText(const AST::Token<std::string>& text) {
			auto lastChild = getLastChild();
			if (lastChild && lastChild->getType() == bpp::AST::NodeType::RawText) {
				std::static_pointer_cast<AST::RawText>(lastChild)->appendText(text);
			} else {
				auto rawTextNode = std::make_shared<AST::RawText>();
				rawTextNode->setText(text);
				addChild(rawTextNode);
			}
		}

		void addText(const std::string& text) {
			auto lastChild = getLastChild();
			if (lastChild && lastChild->getType() == bpp::AST::NodeType::RawText) {
				std::static_pointer_cast<AST::RawText>(lastChild)->appendText(text);
			} else {
				auto rawTextNode = std::make_shared<AST::RawText>();
				AST::Token<std::string> token(text, UINT32_MAX, UINT32_MAX); // Line and column unknown
				rawTextNode->setText(token);
				addChild(rawTextNode);
			}
		}

		std::ostream& prettyPrint(std::ostream& os, size_t indentation_level = 0) const override = 0; // Pure virtual, to prevent direct instantiation
};

} // namespace bpp::AST
