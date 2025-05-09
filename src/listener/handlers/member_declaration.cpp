/**
* Copyright (C) 2025 rail5
* Bash++: Bash with classes
*/

#ifndef SRC_LISTENER_HANDLERS_MEMBER_DECLARATION_CPP_
#define SRC_LISTENER_HANDLERS_MEMBER_DECLARATION_CPP_

#include "../BashppListener.h"

void BashppListener::enterMember_declaration(BashppParser::Member_declarationContext *ctx) {
	skip_syntax_errors
	skip_singlequote_string

	std::shared_ptr<bpp::bpp_class> current_class = std::dynamic_pointer_cast<bpp::bpp_class>(entity_stack.top());

	std::shared_ptr<bpp::bpp_datamember> new_datamember = std::make_shared<bpp::bpp_datamember>();
	new_datamember->set_containing_class(current_class);
	entity_stack.push(new_datamember);

	antlr4::tree::TerminalNode* scope_keyword = nullptr;

	// Get visibility
	BashppParser::Class_member_or_methodContext* parent = dynamic_cast<BashppParser::Class_member_or_methodContext*>(ctx->parent);
	if (parent == nullptr) {
		throw internal_error("Parent context is not a Class_member_or_methodContext", ctx);
	}
	// One of KEYWORD_PUBLIC, KEYWORD_PRIVATE, KEYWORD_PROTECTED will be set
	if (parent->KEYWORD_PUBLIC() != nullptr) {
		new_datamember->set_scope(bpp::SCOPE_PUBLIC);
		scope_keyword = parent->KEYWORD_PUBLIC();
	} else if (parent->KEYWORD_PRIVATE() != nullptr) {
		new_datamember->set_scope(bpp::SCOPE_PRIVATE);
		scope_keyword = parent->KEYWORD_PRIVATE();
	} else if (parent->KEYWORD_PROTECTED() != nullptr) {
		new_datamember->set_scope(bpp::SCOPE_PROTECTED);
		scope_keyword = parent->KEYWORD_PROTECTED();
	}

	if (current_class == nullptr) {
		entity_stack.pop();
		throw_syntax_error(scope_keyword, "Member declaration outside of class");
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

		// Verify the name doesn't contain a double underscore
		if (member_name.find("__") != std::string::npos) {
			entity_stack.pop();
			throw_syntax_error(ctx->IDENTIFIER(), "Invalid member name: " + member_name + "\nBash++ identifiers cannot contain double underscores");
		}
	}
}

void BashppListener::exitMember_declaration(BashppParser::Member_declarationContext *ctx) {
	skip_syntax_errors
	skip_singlequote_string

	std::shared_ptr<bpp::bpp_datamember> new_datamember = std::dynamic_pointer_cast<bpp::bpp_datamember>(entity_stack.top());
	if (new_datamember == nullptr) {
		throw internal_error("entity_stack top is not a bpp_datamember", ctx);
	}

	entity_stack.pop();

	std::shared_ptr<bpp::bpp_class> current_class = std::dynamic_pointer_cast<bpp::bpp_class>(entity_stack.top());

	current_class->add_datamember(new_datamember);
}

#endif // SRC_LISTENER_HANDLERS_MEMBER_DECLARATION_CPP_
