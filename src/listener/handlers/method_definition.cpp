/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#include "../BashppListener.h"

void BashppListener::enterMethod_definition(BashppParser::Method_definitionContext *ctx) {
	skip_syntax_errors
	// Verify we're in a class
	std::shared_ptr<bpp::bpp_class> current_class = std::dynamic_pointer_cast<bpp::bpp_class>(entity_stack.top());
	if (current_class == nullptr) {
		throw_syntax_error(ctx->KEYWORD_METHOD(), "Method definition outside of class body");
	}

	std::string method_name = ctx->IDENTIFIER()->getText();

	// Add the method to entity stack
	std::shared_ptr<bpp::bpp_method> method = std::make_shared<bpp::bpp_method>(method_name);
	method->inherit(program);
	method->set_containing_class(current_class);

	// Set the method's scope
	BashppParser::Class_member_or_methodContext* parent = dynamic_cast<BashppParser::Class_member_or_methodContext*>(ctx->parent);
	if (parent == nullptr) {
		throw internal_error("Parent context is not a Class_member_or_methodContext", ctx);
	}
	if (parent->KEYWORD_PUBLIC() != nullptr) {
		method->set_scope(bpp::bpp_scope::SCOPE_PUBLIC);
	} else if (parent->KEYWORD_PROTECTED() != nullptr) {
		method->set_scope(bpp::bpp_scope::SCOPE_PROTECTED);
	} else {
		method->set_scope(bpp::bpp_scope::SCOPE_PRIVATE);
	}

	// If the method is toPrimitive, verify that the scope is public
	if (method->get_name() == "toPrimitive" && method->get_scope() != bpp::bpp_scope::SCOPE_PUBLIC) {
		throw_syntax_error(ctx->IDENTIFIER(), "toPrimitive method must be public");
		return;
	}

	// Verify that the method name does not contain a double underscore
	if (method_name.find("__") != std::string::npos) {
		throw_syntax_error(ctx->IDENTIFIER(), "Invalid method name: " + method_name + "\nBash++ identifiers cannot contain double underscores");
	}

	// Virtual?
	if (parent->KEYWORD_VIRTUAL() != nullptr) {
		method->set_virtual(true);
	}

	if (!current_class->add_method(method)) {
		throw_syntax_error(ctx->IDENTIFIER(), "Method redefinition: " + method->get_name());
	}

	entity_stack.push(method);
	in_method = true;
}

void BashppListener::exitMethod_definition(BashppParser::Method_definitionContext *ctx) {
	skip_syntax_errors
	// Get the method from the entity stack
	std::shared_ptr<bpp::bpp_method> method = std::dynamic_pointer_cast<bpp::bpp_method>(entity_stack.top());
	entity_stack.pop();

	// Call destructors for any objects created in the method before we exit it
	method->destruct_local_objects(program);
	in_method = false;
}
