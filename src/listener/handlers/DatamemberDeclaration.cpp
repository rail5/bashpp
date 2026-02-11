/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
* Licensed under the GNU General Public License v3.0 or later (GPL-3.0-or-later)
 */

#include <listener/BashppListener.h>

void BashppListener::enterDatamemberDeclaration(std::shared_ptr<AST::DatamemberDeclaration> node) {
	std::shared_ptr<bpp::bpp_class> current_class = std::dynamic_pointer_cast<bpp::bpp_class>(entity_stack.top());

	std::shared_ptr<bpp::bpp_datamember> new_datamember = std::make_shared<bpp::bpp_datamember>();
	new_datamember->set_containing_class(current_class);
	entity_stack.push(new_datamember);

	auto scope = node->ACCESSMODIFIER();
	switch (scope.getValue()) {
		case AST::AccessModifier::PUBLIC:
			new_datamember->set_scope(bpp::bpp_scope::SCOPE_PUBLIC);
			break;
		case AST::AccessModifier::PROTECTED:
			new_datamember->set_scope(bpp::bpp_scope::SCOPE_PROTECTED);
			break;
		case AST::AccessModifier::PRIVATE:
			new_datamember->set_scope(bpp::bpp_scope::SCOPE_PRIVATE);
			break;
	}

	if (current_class == nullptr) {
		entity_stack.pop();
		throw bpp::ErrorHandling::SyntaxError(this, node, "Member declaration outside of class");
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
			id.value().getLine(),
			id.value().getCharPositionInLine()
		);

		// Verify the name doesn't contain a double underscore
		if (member_name.find("__") != std::string::npos) {
			entity_stack.pop();
			throw bpp::ErrorHandling::SyntaxError(this, id.value(), "Invalid member name: " + member_name + "\nBash++ identifiers cannot contain double underscores");
		}
	}
}

void BashppListener::exitDatamemberDeclaration(std::shared_ptr<AST::DatamemberDeclaration> node) {
	std::shared_ptr<bpp::bpp_datamember> new_datamember = std::dynamic_pointer_cast<bpp::bpp_datamember>(entity_stack.top());
	if (new_datamember == nullptr) {
		throw bpp::ErrorHandling::InternalError("entity_stack top is not a bpp_datamember");
	}

	entity_stack.pop();

	std::shared_ptr<bpp::bpp_class> current_class = std::dynamic_pointer_cast<bpp::bpp_class>(entity_stack.top());

	if (!current_class->add_datamember(new_datamember)) {
		// Throw a more specific error: with what does this name conflict?
		auto existing_datamember = current_class->get_datamember_UNSAFE(new_datamember->get_name());
		auto existing_method = current_class->get_method_UNSAFE(new_datamember->get_name());
		if (existing_datamember != nullptr) {
			throw bpp::ErrorHandling::SyntaxError(this, node,
				"Data member name has already been used: @" + current_class->get_name() + "." + new_datamember->get_name());
		}

		if (existing_method != nullptr) {
			throw bpp::ErrorHandling::SyntaxError(this, node, 
				"Data member name conflicts with existing method name: @" + current_class->get_name() + "." + new_datamember->get_name());
		}

		// Generic error if we can't determine the cause
		throw bpp::ErrorHandling::SyntaxError(this, node, "Failed to add data member: " + new_datamember->get_name());
	}
}
