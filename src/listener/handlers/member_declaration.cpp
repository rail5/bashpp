/**
 * Copyright (C) 2025 rail5
 * Bash++: Bash with classes
 */

#ifndef ANTLR_LISTENER_HANDLERS_MEMBER_DECLARATION_CPP_
#define ANTLR_LISTENER_HANDLERS_MEMBER_DECLARATION_CPP_

#include "../BashppListener.h"

void BashppListener::enterMember_declaration(BashppParser::Member_declarationContext *ctx) {
	skip_comment
	skip_syntax_errors
	skip_singlequote_string

	std::shared_ptr<bpp::bpp_class> current_class = std::dynamic_pointer_cast<bpp::bpp_class>(entity_stack.top());

	if (current_class == nullptr) {
		throw_syntax_error(ctx->AT(), "Member declaration outside of class");
	}

	std::shared_ptr<bpp::bpp_datamember> new_datamember = std::make_shared<bpp::bpp_datamember>();
	entity_stack.push(new_datamember);

	// Get visibility
	// One of KEYWORD_PUBLIC, KEYWORD_PRIVATE, KEYWORD_PROTECTED will be set
	if (ctx->KEYWORD_PUBLIC() != nullptr) {
		new_datamember->set_scope(bpp::SCOPE_PUBLIC);
	} else if (ctx->KEYWORD_PRIVATE() != nullptr) {
		new_datamember->set_scope(bpp::SCOPE_PRIVATE);
	} else if (ctx->KEYWORD_PROTECTED() != nullptr) {
		new_datamember->set_scope(bpp::SCOPE_PROTECTED);
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

	if (ctx->IDENTIFIER() != nullptr) {
		// It's a primitive
		std::string member_name = ctx->IDENTIFIER()->getText();
		new_datamember->set_name(member_name);
	}
}

void BashppListener::exitMember_declaration(BashppParser::Member_declarationContext *ctx) {
	skip_comment
	skip_syntax_errors
	skip_singlequote_string

	std::shared_ptr<bpp::bpp_datamember> new_datamember = std::dynamic_pointer_cast<bpp::bpp_datamember>(entity_stack.top());
	if (new_datamember == nullptr) {
		throw internal_error("entity_stack top is not a bpp_datamember");
	}

	entity_stack.pop();

	std::shared_ptr<bpp::bpp_class> current_class = std::dynamic_pointer_cast<bpp::bpp_class>(entity_stack.top());

	current_class->add_datamember(new_datamember);
}

#endif // ANTLR_LISTENER_HANDLERS_MEMBER_DECLARATION_CPP_
