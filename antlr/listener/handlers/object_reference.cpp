/**
 * Copyright (C) 2025 rail5
 * Bash++: Bash with classes
 */

#ifndef ANTLR_LISTENER_HANDLERS_OBJECT_REFERENCE_CPP_
#define ANTLR_LISTENER_HANDLERS_OBJECT_REFERENCE_CPP_

#include "../BashppListener.h"

void BashppListener::enterObject_reference(BashppParser::Object_referenceContext *ctx) {
	skip_comment
	skip_singlequote_string

	/**
	 * Object references take the form
	 * 	@IDENTIFIER.IDENTIFIER.IDENTIFIER...
	 * Where each IDENTIFIER following a dot is a member of the object referenced by the preceding IDENTIFIER
	 * 
	 * This reference may resolve to either a data member or a method
	 * If it resolves to a data member:
	 * 		- If that data member is primitive, we're treating this as an rvalue
	 * 		- If that data member is nonprimitive, this is a method call to .toPrimitive
	 * 		- If that data member doesn't exist, throw an error
	 */

	object_preaccess_code.clear();
	object_postaccess_code.clear();
	object_access_code.clear();

	std::shared_ptr<bpp::bpp_object> referenced_object = program->get_object(ctx->IDENTIFIER(0)->getText());
	if (referenced_object == nullptr) {
		throw_syntax_error(ctx->IDENTIFIER(0), "Object not found: " + ctx->IDENTIFIER(0)->getText());
	}

	std::vector<std::shared_ptr<bpp::bpp_entity>> object_chain;
	object_chain.push_back(referenced_object);
	std::shared_ptr<bpp::bpp_entity> current_context = referenced_object;

	bool can_descend = true;

	for (size_t i = 1; i < ctx->IDENTIFIER().size(); i++) {
		if (!can_descend) {
			throw_syntax_error(ctx->IDENTIFIER(i), "Cannot descend further");
		}

		bool is_datamember = false;
		bool is_method = false;

		std::shared_ptr<bpp::bpp_datamember> referenced_datamember = std::dynamic_pointer_cast<bpp::bpp_datamember>(current_context->get_class()->get_datamember(ctx->IDENTIFIER(i)->getText()));
		std::shared_ptr<bpp::bpp_method> referenced_method = std::dynamic_pointer_cast<bpp::bpp_method>(current_context->get_class()->get_method(ctx->IDENTIFIER(i)->getText()));

		if (referenced_datamember != nullptr) {
			is_datamember = true;
			object_chain.push_back(referenced_datamember);
			current_context = referenced_datamember;
		} else if (referenced_method != nullptr) {
			is_method = true;
			can_descend = false;
			object_chain.push_back(referenced_method);
		}
	}

	// Check the type of the last element in the object chain
	std::shared_ptr<bpp::bpp_object> final_object = std::dynamic_pointer_cast<bpp::bpp_object>(object_chain[object_chain.size() - 1]);
	std::shared_ptr<bpp::bpp_method> final_method = std::dynamic_pointer_cast<bpp::bpp_method>(object_chain[object_chain.size() - 1]);

	if (final_object != nullptr && final_object->get_class() != primitive) {
		// Call to .toPrimitive on a non-primitive data member
		// Add the toPrimitive method to the object_chain
		// Set final_method = toPrimitive
		// Set final_datamember = nullptr
		final_method = final_object->get_class()->get_method("toPrimitive");
		object_chain.push_back(final_method);
		final_object = nullptr;
	}

	bool must_declare_temporary_variables = false;
	bool declared_first_temporary_variable = false;

	for (size_t i = 2; i < object_chain.size() - 1; i++) {
		std::string indirection = declared_first_temporary_variable ? "!" : "";
		std::string temporary_variable_lvalue;
		std::string temporary_variable_rvalue;
		std::string temporary_variable = "bpp__" + referenced_object->get_class()->get_name() + "__" + referenced_object->get_name();
		for (size_t j = 1; j < i; j++) {
			temporary_variable += "__" + object_chain[j]->get_name();
		}
		temporary_variable_lvalue = temporary_variable + "__" + object_chain[i]->get_name();
		temporary_variable_rvalue = "${" + indirection + temporary_variable + "}__" + object_chain[i]->get_name();
		object_preaccess_code += temporary_variable_lvalue + "=\"" + temporary_variable_rvalue + "\"\n";
		object_postaccess_code += "unset " + temporary_variable_lvalue + "\n";
		declared_first_temporary_variable = true;
	}
	/**
	 * By this point, the deepest reference in the chain that we have access to is:
	 * ${bpp__objectClass__object1__object2__object3__....objectN-1}
	 * Where objectN (the next one, the one we don't have yet) is either a primitive or a method
	 */

	if (final_object != nullptr) {
		std::string indirection = declared_first_temporary_variable ? "!" : "";
		std::string temporary_variable_lvalue;
		std::string temporary_variable_rvalue;
		std::string temporary_variable = "bpp__" + referenced_object->get_class()->get_name() + "__" + referenced_object->get_name();
		for (size_t i = 1; i < object_chain.size() - 1; i++) {
			temporary_variable += "__" + object_chain[i]->get_name();
		}

		temporary_variable_lvalue = temporary_variable + "__" + final_object->get_name();
		temporary_variable_rvalue = "${" + indirection + temporary_variable + "}__" + final_object->get_name();

		if (object_chain.size() > 2) {
			object_preaccess_code += temporary_variable_lvalue + "=\"" + temporary_variable_rvalue + "\"\n";
			object_postaccess_code += "unset " + temporary_variable_lvalue + "\n";
			indirection = "!";
		}
		object_access_code = "${" + indirection + temporary_variable_lvalue + "}";
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

		std::string object_address = "bpp__" + referenced_object->get_class()->get_name() + "__" + referenced_object->get_name();

		for (size_t i = 1; i < object_chain.size() - 1; i++) {
			object_address += "__" + object_chain[i]->get_name();
		}

		std::string indirection_start = "";
		std::string indirection_end = "";

		if (object_chain.size() > 2) {
			indirection_start = "${";
			indirection_end = "}";
		}

		if (object_chain.size() > 3) {
			indirection_start += "!";
		}
		
		std::string method_call = "bpp__" + penultimate_class->get_name() + "__" + final_method->get_name();

		object_preaccess_code += "function ____runSupershellFunc() {\n";
		object_preaccess_code += "	" + method_call + " \"" + indirection_start + object_address + indirection_end +"\" 1\n";
		object_preaccess_code += "}\n";
		object_preaccess_code += "bpp____supershell ____supershellOutput ____runSupershellFunc\n";
		object_access_code = "${____supershellOutput}";
		object_postaccess_code += "unset ____supershellOutput\n";
		object_postaccess_code += "unset -f ____runSupershellFunc\n";
	} else {
		throw internal_error("Terminal entity in the object chain is neither an object nor a method");
	}
}

void BashppListener::exitObject_reference(BashppParser::Object_referenceContext *ctx) {
	skip_comment
	skip_singlequote_string

	// If we're not in any broader context, simply add the object reference to the program
	program->add_code_to_previous_line(object_preaccess_code);
	program->add_code_to_next_line(object_postaccess_code);
	program->add_code(object_access_code);
}

#endif // ANTLR_LISTENER_HANDLERS_OBJECT_REFERENCE_CPP_