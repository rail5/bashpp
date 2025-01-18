/**
 * Copyright (C) 2025 rail5
 * Bash++: Bash with classes
 */

#ifndef SRC_LISTENER_HANDLERS_SELF_REFERENCE_CPP_
#define SRC_LISTENER_HANDLERS_SELF_REFERENCE_CPP_

#include "../BashppListener.h"

void BashppListener::enterSelf_reference(BashppParser::Self_referenceContext *ctx) {
	skip_comment
	skip_syntax_errors
	skip_singlequote_string

	/**
	 * Self references take the form
	 * 	@this.IDENTIFIER.IDENTIFIER...
	 * Where each IDENTIFIER following a dot is a member of the object referenced by the preceding IDENTIFIER
	 * 
	 * This reference may resolve to either an object or a method
	 * If it's a primitive object, treat this as an rvalue and get the value of the primitive object
	 * If it's a non-primitive object, this is a method call to .toPrimitive
	 * If it's a method, call the method in a supershell and substitute the result
	 */
	std::shared_ptr<bpp::bpp_class> current_class = entity_stack.top()->get_containing_class().lock();
	// Get the current code entity
	std::shared_ptr<bpp::bpp_code_entity> current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());

	std::shared_ptr<bpp::bpp_string> self_reference_entity = std::make_shared<bpp::bpp_string>();
	self_reference_entity->set_containing_class(current_class);
	self_reference_entity->inherit(current_code_entity);
	entity_stack.push(self_reference_entity);

	if (current_class == nullptr) {
		entity_stack.pop();
		throw_syntax_error(ctx->KEYWORD_THIS(), "Self reference outside of class");
	}

	if (ctx->IDENTIFIER().size() == 0) {
		self_reference_entity->add_code("${__objectAddress}");
		return;
	}

	std::vector<std::shared_ptr<bpp::bpp_entity>> object_chain;

	bool can_descend = true;

	if (ctx->IDENTIFIER().size() >= 1) {
		std::shared_ptr<bpp::bpp_datamember> referenced_datamember = current_class->get_datamember(ctx->IDENTIFIER(0)->getText());
		std::shared_ptr<bpp::bpp_method> referenced_method = current_class->get_method(ctx->IDENTIFIER(0)->getText());

		if (referenced_datamember != nullptr) {
			object_chain.push_back(referenced_datamember);
			self_reference_entity->add_code_to_previous_line("this__" + referenced_datamember->get_name() + "=\"${__objectAddress}__" + referenced_datamember->get_name() + "\"\n");
			self_reference_entity->add_code_to_next_line("unset this__" + referenced_datamember->get_name() + "\n");
			if (ctx->IDENTIFIER().size() == 1) {
				if (referenced_datamember->get_class() == primitive || referenced_datamember->is_pointer()) {
					self_reference_entity->add_code("${!this__" + referenced_datamember->get_name() + "}");
					return;
				}
			}
		} else if (referenced_method != nullptr) {
			object_chain.push_back(referenced_method);
			can_descend = false;
		} else {
			entity_stack.pop();
			throw_syntax_error(ctx->IDENTIFIER(0), "Member not found: " + ctx->IDENTIFIER(0)->getText());
		}
	}

	for (size_t i = 1; i < ctx->IDENTIFIER().size(); i++) {
		if (!can_descend) {
			entity_stack.pop();
			throw_syntax_error(ctx->IDENTIFIER(i), "Cannot descend further");
		}

		std::shared_ptr<bpp::bpp_datamember> referenced_datamember = std::dynamic_pointer_cast<bpp::bpp_datamember>(object_chain.back()->get_class()->get_datamember(ctx->IDENTIFIER(i)->getText()));
		std::shared_ptr<bpp::bpp_method> referenced_method = std::dynamic_pointer_cast<bpp::bpp_method>(object_chain.back()->get_class()->get_method(ctx->IDENTIFIER(i)->getText()));

		if (referenced_datamember != nullptr) {
			object_chain.push_back(referenced_datamember);
		} else if (referenced_method != nullptr) {
			can_descend = false;
			object_chain.push_back(referenced_method);
		} else {
			entity_stack.pop();
			throw_syntax_error(ctx->IDENTIFIER(i), "Member not found: " + ctx->IDENTIFIER(i)->getText());
		}
	}

	// Check the type of the last element in the object chain
	std::shared_ptr<bpp::bpp_object> final_object = std::dynamic_pointer_cast<bpp::bpp_object>(object_chain.back());
	std::shared_ptr<bpp::bpp_method> final_method = std::dynamic_pointer_cast<bpp::bpp_method>(object_chain.back());

	if (final_object != nullptr && final_object->get_class() != primitive && !final_object->is_pointer()) {
		// Call to .toPrimitive on a non-primitive data member
		// Add the toPrimitive method to the object_chain
		// Set final_method = toPrimitive
		// Set final_datamember = nullptr
		final_method = final_object->get_class()->get_method("toPrimitive");
		object_chain.push_back(final_method);
		final_object = nullptr;
	}

	bool declared_first_temporary_variable = false;

	for (size_t i = 1; i < object_chain.size() - 1; i++) {
		std::string indirection = "!";
		std::string temporary_variable_lvalue;
		std::string temporary_variable_rvalue;
		std::string temporary_variable = "this";
		for (size_t j = 1; j < i; j++) {
			temporary_variable += "__" + object_chain[j]->get_name();
		}
		temporary_variable_lvalue = temporary_variable + "__" + object_chain[i]->get_name();
		temporary_variable_rvalue = "${" + indirection + temporary_variable + "}__" + object_chain[i]->get_name();

		if (object_chain.size() > 2) {
			self_reference_entity->add_code_to_previous_line(temporary_variable_lvalue + "=\"" + temporary_variable_rvalue + "\"\n");
			self_reference_entity->add_code_to_next_line("unset " + temporary_variable_lvalue + "\n");
			indirection = "!";
		}
		self_reference_entity->add_code("${" + indirection + temporary_variable_lvalue + "}");
	}
	/**
	 * By this point, the deepest reference in the chain that we have access to is:
	 * ${this__object1__object2__object3__....objectN-1}
	 * Where objectN (the next one, the one we don't have yet) is either a primitive or a method
	 */

	if (final_object != nullptr) {
		std::string indirection = "!";
		std::string temporary_variable_lvalue;
		std::string temporary_variable_rvalue;
		std::string temporary_variable = "this";
		for (size_t i = 0; i < object_chain.size() - 1; i++) {
			temporary_variable += "__" + object_chain[i]->get_name();
		}

		temporary_variable_lvalue = temporary_variable + "__" + final_object->get_name();
		temporary_variable_rvalue = "${" + indirection + temporary_variable + "}__" + final_object->get_name();

		if (object_chain.size() > 1) {
			self_reference_entity->add_code_to_previous_line(temporary_variable_lvalue + "=\"" + temporary_variable_rvalue + "\"\n");
			self_reference_entity->add_code_to_next_line("unset " + temporary_variable_lvalue + "\n");
			indirection = "!";
		}
		self_reference_entity->add_code("${" + indirection + temporary_variable_lvalue + "}");
	} else if (final_method != nullptr) {
		// Call the given method in a supershell

		// Get the penultimate object in the chain
		std::shared_ptr<bpp::bpp_entity> penultimate_object = object_chain[object_chain.size() - 2];
		if (penultimate_object == nullptr) {
			throw internal_error("Penultimate entity in the object chain is not an object");
		}
		// Get its class
		std::shared_ptr<bpp::bpp_class> penultimate_class = penultimate_object->get_class();
		if (penultimate_class == nullptr) {
			throw internal_error("Penultimate entity in the object chain does not have a class");
		}

		std::string object_address = "this";

		for (size_t i = 1; i < object_chain.size() - 1; i++) {
			object_address += "__" + object_chain[i]->get_name();
		}

		std::string indirection_start = "";
		std::string indirection_end = "";

		if (object_chain.size() > 1) {
			indirection_start = "${";
			indirection_end = "}";
		}

		if (object_chain.size() > 2) {
			indirection_start += "!";
		}
		
		std::string method_call = "bpp__" + penultimate_class->get_name() + "__" + final_method->get_name();

		self_reference_entity->add_code_to_previous_line("function ____runSupershellFunc() {\n");
		self_reference_entity->add_code_to_previous_line("	" + method_call + " \"" + indirection_start + object_address + indirection_end +"\" 1\n");
		self_reference_entity->add_code_to_previous_line("}\n");
		self_reference_entity->add_code_to_previous_line("bpp____supershell ____supershellOutput ____runSupershellFunc\n");
		self_reference_entity->add_code("${____supershellOutput}");
		self_reference_entity->add_code_to_next_line("unset ____supershellOutput\n");
		self_reference_entity->add_code_to_next_line("unset -f ____runSupershellFunc\n");
	} else {
		throw internal_error("Terminal entity in the object chain is neither an object nor a method");
	}
}

void BashppListener::exitSelf_reference(BashppParser::Self_referenceContext *ctx) {
	skip_comment
	skip_syntax_errors
	skip_singlequote_string

	std::shared_ptr<bpp::bpp_string> self_reference_entity = std::dynamic_pointer_cast<bpp::bpp_string>(entity_stack.top());
	entity_stack.pop();
	if (self_reference_entity == nullptr) {
		throw internal_error("Self reference context was not found in the entity stack");
	}

	// If we're not in any broader context, simply add the object reference to the current code entity
	std::shared_ptr<bpp::bpp_code_entity> current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());
	if (current_code_entity != nullptr) {
		current_code_entity->add_code_to_previous_line(self_reference_entity->get_pre_code());
		current_code_entity->add_code_to_next_line(self_reference_entity->get_post_code());
		current_code_entity->add_code(self_reference_entity->get_code());
		return;
	}
}

#endif // SRC_LISTENER_HANDLERS_SELF_REFERENCE_CPP_
