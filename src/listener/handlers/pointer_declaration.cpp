/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#ifndef SRC_LISTENER_HANDLERS_POINTER_DECLARATION_CPP_
#define SRC_LISTENER_HANDLERS_POINTER_DECLARATION_CPP_

#include "../BashppListener.h"

void BashppListener::enterPointer_declaration(BashppParser::Pointer_declarationContext *ctx) {
	skip_comment
	skip_syntax_errors
	skip_singlequote_string

	/**
	 * The pointer type will be stored in one of either IDENTIFIER_LVALUE or IDENTIFIER(0)
	 * If IDENTIFIER_LVALUE, then the pointer name will be in IDENTIFIER(0)
	 * If IDENTIFIER(0), then the pointer name will be in IDENTIFIER(1)
	 */

	// Verify that we're in a place where an object *can* be instantiated
	std::shared_ptr<bpp::bpp_class> current_class = std::dynamic_pointer_cast<bpp::bpp_class>(entity_stack.top());
	if (current_class != nullptr) {
		throw_syntax_error(ctx->AT(), "Stray pointer declaration inside class body.\nDid you mean to declare a data member?\nIf so, start by declaring the data member with a visibility keyword (@public, @private, @protected)");
	}

	// Actually get the containing class
	current_class = entity_stack.top()->get_containing_class().lock();

	antlr4::tree::TerminalNode* object_type = nullptr;
	antlr4::tree::TerminalNode* object_name = nullptr;

	if (ctx->IDENTIFIER_LVALUE() != nullptr) {
		// The object type is in IDENTIFIER_LVALUE
		object_type = ctx->IDENTIFIER_LVALUE();
		object_name = ctx->IDENTIFIER(0);
	} else {
		// The object type is in IDENTIFIER(0)
		object_type = ctx->IDENTIFIER(0);
		object_name = ctx->IDENTIFIER(1);
	}

	std::string object_type_text = object_type->getText();
	std::string object_name_text = object_name->getText();

	// Get the current code entity
	std::shared_ptr<bpp::bpp_code_entity> current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());
	if (current_code_entity == nullptr) {
		current_code_entity = program;
	}

	// Are we in a data member declaration?
	std::shared_ptr<bpp::bpp_datamember> current_datamember = std::dynamic_pointer_cast<bpp::bpp_datamember>(entity_stack.top());

	std::shared_ptr<bpp::bpp_class> object_class = current_code_entity->get_class(object_type_text);

	// Verify that the object's class exists
	if (object_class == nullptr) {
		// Is it the current class?
		std::shared_ptr<bpp::bpp_class> containing_class = (current_datamember == nullptr) ? nullptr : current_datamember->get_containing_class().lock();
		if (containing_class != nullptr && containing_class->get_name() == object_type_text) {
			object_class = containing_class;
		} else {
			throw_syntax_error(ctx->AT(), "Class not found: " + object_type_text);
		}
	}

	std::shared_ptr<bpp::bpp_object> new_object = std::make_shared<bpp::bpp_object>(object_name_text);
	new_object->set_pointer(true);
	new_object->set_containing_class(current_class);
	entity_stack.push(new_object);

	new_object->set_class(object_class);
	new_object->set_address("bpp____ptr__" + new_object->get_class()->get_name() + "__" + new_object->get_name());

	// Verify that the object's name is not already in use (or a protected keyword)
	if (protected_keywords.find(new_object->get_name()) != protected_keywords.end()) {
		entity_stack.pop();
		throw_syntax_error(object_name, "Invalid object name: " + new_object->get_name());
	}
	if (current_code_entity->get_class(new_object->get_name()) != nullptr) {
		entity_stack.pop();
		throw_syntax_error(object_name, "Class already exists: " + new_object->get_name());
	}
	if (current_code_entity->get_object(new_object->get_name()) != nullptr) {
		entity_stack.pop();
		throw_syntax_error(object_name, "Object already exists: " + new_object->get_name());
	}
}

void BashppListener::exitPointer_declaration(BashppParser::Pointer_declarationContext *ctx) {
	skip_comment
	skip_syntax_errors
	skip_singlequote_string

	std::shared_ptr<bpp::bpp_object> new_object = std::dynamic_pointer_cast<bpp::bpp_object>(entity_stack.top());
	if (new_object == nullptr) {
		throw internal_error("entity_stack top is not a bpp_object", ctx);
	}

	entity_stack.pop();

	// If there is no assignment value set, set it to nullptr
	if (new_object->get_assignment_value().empty()) {
		new_object->set_nullptr();
	}

	std::shared_ptr<bpp::bpp_datamember> current_datamember = std::dynamic_pointer_cast<bpp::bpp_datamember>(entity_stack.top());
	if (current_datamember != nullptr) {
		// We're midway through a class member declaration
		// The data for this object should be moved to the datamember
		current_datamember->set_class(new_object->get_class());
		current_datamember->set_name(new_object->get_name());
		current_datamember->set_pointer(true);
		current_datamember->set_pre_access_code(new_object->get_pre_access_code());
		current_datamember->set_post_access_code(new_object->get_post_access_code());
		current_datamember->set_default_value(new_object->get_assignment_value());
		return;
	}

	// Add the object to the current code entity
	std::shared_ptr<bpp::bpp_code_entity> current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());
	if (current_code_entity != nullptr) {
		current_code_entity->add_code_to_previous_line(new_object->get_pre_access_code());
		current_code_entity->add_code_to_next_line(new_object->get_post_access_code());
		current_code_entity->add_object(new_object);
		return;
	}
}

#endif // SRC_LISTENER_HANDLERS_POINTER_DECLARATION_CPP_
