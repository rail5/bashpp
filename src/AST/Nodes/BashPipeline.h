/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <AST/ASTNode.h>
#include <AST/Nodes/StringType.h>
#include <AST/Nodes/BashCommand.h>
#include <AST/Nodes/BashBreakOrContinueCommand.h>

namespace bpp::AST {

class BashPipeline : public StringType {
	public:
		/**
		 * @brief Mark all child BashCommand nodes as *not* being early (global or local) exit points.
		 * This is used when 'return', 'exit', 'exec', 'break', or 'continue' are part of pipelines with more than one element,
		 *   in which case they do not cause the program or function to exit early.
		 */
		void unmarkAllExitPaths() {
			for (auto& child : children) {
				if (child->getType() == bpp::AST::NodeType::BashCommand) {
					auto command = std::static_pointer_cast<BashCommand>(child);
					command->setExitPointType(ExitPointType::NO_EXIT);
				} else if (child->getType() == bpp::AST::NodeType::BashBreakOrContinueCommand) {
					auto command = std::static_pointer_cast<BashBreakOrContinueCommand>(child);
					command->unmarkExitPath();
				}
			}
		}

		constexpr BashPipeline() : StringType(bpp::AST::NodeType::BashPipeline) {}
		PRETTYPRINT_OVERRIDE({
			std::string indent(indentation_level * PRETTYPRINT_INDENTATION_AMOUNT, ' ');
			os << indent << "(BashPipeline";
			for (const auto& child : children) {
				os << std::endl;
				child->prettyPrint(os, indentation_level + 1);
			}
			os << ")" << std::flush;
			return os;
		})
};

} // namespace bpp::AST
