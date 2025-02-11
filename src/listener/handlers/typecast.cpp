/**
 * Copyright (C) 2025 rail5
 * Bash++: Bash with classes
 */

#ifndef SRC_LISTENER_HANDLERS_TYPECAST_CPP_
#define SRC_LISTENER_HANDLERS_TYPECAST_CPP_

#include "../BashppListener.h"

void BashppListener::enterTypecast(BashppParser::TypecastContext *ctx) {
	skip_comment
	skip_syntax_errors
	skip_singlequote_string

	std::shared_ptr<bpp::bpp_code_entity> current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());

	if (current_code_entity == nullptr) {
		antlr4::tree::TerminalNode* cast_keyword;

		if (ctx->KEYWORD_CAST() != nullptr) {
			cast_keyword = ctx->KEYWORD_CAST();
		} else if (ctx->KEYWORD_DOWNCAST() != nullptr) {
			cast_keyword = ctx->KEYWORD_DOWNCAST();
		} else if (ctx->KEYWORD_UPCAST() != nullptr) {
			cast_keyword = ctx->KEYWORD_UPCAST();
		}

		throw_syntax_error(cast_keyword, "Typecast outside of code entity");
	}

	std::shared_ptr<bpp::bpp_typecast> typecast = std::make_shared<bpp::bpp_typecast>();
	typecast->set_containing_class(entity_stack.top()->get_containing_class());
	typecast->inherit(current_code_entity);

	entity_stack.push(typecast);
}

void BashppListener::exitTypecast(BashppParser::TypecastContext *ctx) {
	skip_comment
	skip_syntax_errors
	skip_singlequote_string

	std::shared_ptr<bpp::bpp_typecast> typecast = std::dynamic_pointer_cast<bpp::bpp_typecast>(entity_stack.top());

	if (typecast == nullptr) {
		throw internal_error("Typecast context was not found in the entity stack");
	}

	entity_stack.pop();

	std::shared_ptr<bpp::bpp_code_entity> current_code_entity = std::dynamic_pointer_cast<bpp::bpp_code_entity>(entity_stack.top());

	int cast_type = 0; // 0 = cast, 1 = downcast, 2 = upcast

	if (ctx->KEYWORD_CAST() != nullptr) {
		cast_type = 0;
	} else if (ctx->KEYWORD_DOWNCAST() != nullptr) {
		cast_type = 1;
	} else if (ctx->KEYWORD_UPCAST() != nullptr) {
		cast_type = 2;
	}

	std::shared_ptr<bpp::bpp_class> class_to_cast_to = current_code_entity->get_class(ctx->IDENTIFIER()->getText());

	if (class_to_cast_to == nullptr) {
		throw_syntax_error(ctx->IDENTIFIER(), "Class " + ctx->IDENTIFIER()->getText() + " not found");
	}

	bool cast_ok = false;
	bool casting_to_pointer = ctx->ASTERISK() != nullptr;
	std::string cast_type_str = "cast";

	std::shared_ptr<bpp::bpp_object> object_to_cast = typecast->get_object_to_cast();

	switch (cast_type) {
		case 0:
			if (object_to_cast == nullptr) {
				cast_ok = casting_to_pointer;
			} else {
				cast_ok = (object_to_cast->is_pointer() == casting_to_pointer);
			}
			break;
		case 1:
			cast_type_str = "downcast";
			if (object_to_cast == nullptr) {
				cast_ok = false;
			} else {
				cast_ok = (object_to_cast->is_pointer() == casting_to_pointer)
					&& class_to_cast_to->is_child_of(object_to_cast->get_class());
			}
			break;
		case 2:
			cast_type_str = "upcast";
			if (object_to_cast == nullptr) {
				cast_ok = false;
			} else {
				cast_ok = (object_to_cast->is_pointer() == casting_to_pointer)
					&& object_to_cast->get_class()->is_child_of(class_to_cast_to);
			}
			break;
	}

	if (!cast_ok) {
		std::string to_asterisk = casting_to_pointer ? "*" : "";
		std::string from_asterisk = (object_to_cast != nullptr && object_to_cast->is_pointer()) ? "*" : "";
		std::string from_name = object_to_cast != nullptr ? object_to_cast->get_class()->get_name() : "primitive";
		throw_syntax_error_from_exitRule(ctx->IDENTIFIER(), "Invalid " + cast_type_str + " from " + from_name + from_asterisk + " to " + class_to_cast_to->get_name() + to_asterisk);
	}

	// Are we in a value assignment?
	std::shared_ptr<bpp::bpp_value_assignment> value_assignment_entity = std::dynamic_pointer_cast<bpp::bpp_value_assignment>(current_code_entity);
	if (value_assignment_entity != nullptr) {
		value_assignment_entity->set_nonprimitive_object(std::dynamic_pointer_cast<bpp::bpp_entity>(object_to_cast));
		value_assignment_entity->set_nonprimitive_assignment(!object_to_cast->is_pointer());
	}

	current_code_entity->add_code_to_previous_line(typecast->get_pre_code());
	current_code_entity->add_code_to_next_line(typecast->get_post_code());
	current_code_entity->add_code(typecast->get_code());
}

#endif // SRC_LISTENER_HANDLERS_TYPECAST_CPP_
