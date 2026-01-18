/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#include "../BashppListener.h"

void BashppListener::enterDatamemberDeclaration(std::shared_ptr<AST::DatamemberDeclaration> node) {
	skip_syntax_errors
	std::shared_ptr<bpp::bpp_class> current_class = std::dynamic_pointer_cast<bpp::bpp_class>(entity_stack.top());

	std::shared_ptr<bpp::bpp_datamember> new_datamember = std::make_shared<bpp::bpp_datamember>();
	new_datamember->set_containing_class(current_class);
	entity_stack.push(new_datamember);

	auto scope = node->ACCESSMODIFIER();

	if (current_class == nullptr) {
		entity_stack.pop();
		throw_syntax_error(node, "Member declaration outside of class");
	}

	/**
	 * This will either be:
	 * 	1. A primitive
	 * 	2. An object
	 * 	3. A pointer
	 * If it's a primitive, then IDENTIFIER will be set
	 * If it's an object, then object_instantiation will be set, and we'll handle that in the object_instantiation rule
	 * If it's a pointer, then pointer_declaration will be set, and we'll handle that in the pointer_declaration rule
	 */

	new_datamember->set_class(primitive); // Set the class to primitive by default (until changed by another parser rule)

	auto id = node->IDENTIFIER();

	if (id.has_value()) {
		// It's a primitive
		std::string member_name = id.value().getValue();
		new_datamember->set_name(member_name);

		new_datamember->set_definition_position(
			source_file,
			id.value().getLine() - 1,
			id.value().getCharPositionInLine()
		);

		// Verify the name doesn't contain a double underscore
		if (member_name.find("__") != std::string::npos) {
			entity_stack.pop();
			throw_syntax_error(id.value(), "Invalid member name: " + member_name + "\nBash++ identifiers cannot contain double underscores");
		}
	}
}

void BashppListener::exitDatamemberDeclaration(std::shared_ptr<AST::DatamemberDeclaration> node) {
	skip_syntax_errors
	std::shared_ptr<bpp::bpp_datamember> new_datamember = std::dynamic_pointer_cast<bpp::bpp_datamember>(entity_stack.top());
	if (new_datamember == nullptr) {
		throw internal_error("entity_stack top is not a bpp_datamember");
	}

	entity_stack.pop();

	std::shared_ptr<bpp::bpp_class> current_class = std::dynamic_pointer_cast<bpp::bpp_class>(entity_stack.top());

	current_class->add_datamember(new_datamember);
}
