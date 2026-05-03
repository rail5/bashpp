/*
 * Copyright (C) 2025 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "bpp.h"
#include "bpp_codegen.h"

namespace bpp {

/**
 * @brief Add code to the code entity
 * 
 * This function adds code to the code entity's primary buffer.
 * If the code contains a newline character, all code buffers are flushed.
 * This ensures that the pre- and post- code relevant to *each particular line* of code is placed before and after the relevant line of code.
 * 
 * @param code The code to add
 * @param add_newline Whether to add a newline character after the code (default: true)
 */
void bpp_code_entity::add_code(const std::string& code, bool add_newline) {
	if (buffers_flushed && code == "\n") {
		return;
	}
	// If the code has a newline char, flush the nextline_buffer and the postline_buffer
	size_t newline_index = code.find('\n');
	if (newline_index != std::string::npos && add_newline && (newline_index == 0 || code[newline_index - 1] != '\\')) {
		flush_nextline_buffer();

		*this->code << code << std::flush;
		if (code.back() != '\n') {
			*this->code << "\n" << std::flush;
		}

		flush_postline_buffer();
		buffers_flushed = true;
		return;
	}

	// Otherwise, add the code to the nextline_buffer
	nextline_buffer += code;
	buffers_flushed = false;
}

/**
 * @brief Add code to the code entity's pre-code buffer
 */
void bpp_code_entity::add_code_to_previous_line(const std::string& code) {
	*this->code << code << std::flush;
	buffers_flushed = false;
}

/**
 * @brief Add code to the code entity's post-code buffer
 */
void bpp_code_entity::add_code_to_next_line(const std::string& code) {
	postline_buffer += code;
	buffers_flushed = false;
}

void bpp_code_entity::flush_nextline_buffer() {
	if (!nextline_buffer.empty()) {
		*code << nextline_buffer << std::flush;
		if (nextline_buffer.back() != '\n') {
			*code << "\n" << std::flush;
		}
	}
	nextline_buffer = "";
}

void bpp_code_entity::flush_postline_buffer() {
	if (!postline_buffer.empty()) {
		*code << postline_buffer << std::flush;
		if (postline_buffer.back() != '\n') {
			*code << "\n" << std::flush;
		}
	}
	postline_buffer = "";
}

void bpp_code_entity::flush_code_buffers() {
	flush_nextline_buffer();
	flush_postline_buffer();
	buffers_flushed = true;
}

void bpp_code_entity::clear_all_buffers() {
	nextline_buffer = "";
	postline_buffer = "";
	std::shared_ptr<std::ostringstream> ss = std::dynamic_pointer_cast<std::ostringstream>(code);
	if (ss != nullptr) {
		ss->str("");
	}
	buffers_flushed = false;
}

/**
 * @brief Add an object to the code entity
 * 
 * Adding an object to a code entity involves adding the necessary code to create the object.
 * Unlike in the case where we add an object to a non-code entity, where we only need to update the object map.
 * 
 * This function generates the code necessary to create the object, including calling its constructor if it exists.
 * 
 * @param object The object to add
 */
bool bpp_code_entity::add_object(std::shared_ptr<bpp_object> object, bool make_local) {
	std::string name = object->get_name();
	if (objects.contains(name) || local_objects.contains(name)) {
		return false;
	}

	// Verify that the type of the object is a valid class
	std::string type = object->get_class()->get_name();
	if (!classes.contains(type)) {
		return false;
	}

	local_objects[name] = object;

	// Add the code for the object
	std::string object_code;

	// Is it a pointer?
	if (object->is_pointer()) {
		if (make_local) {
			object_code += "local ";
		}
		object_code += object->get_address() + "=" + object->get_assignment_value() + "\n";
	} else {
		if (object->get_copy_from() != nullptr) {
			auto copy_call = generate_method_call_code(
				object->get_address(),
				"__copy",
				object->get_class(),
				false,
				get_containing_program().lock()
			);
			object_code += copy_call.pre_code + "\n";
			object_code += copy_call.code + " " + object->get_copy_from()->get_address() + "\n";
			object_code += copy_call.post_code + "\n";
		} else {
			object_code += generate_new_code(object->get_address(), object->get_class(), make_local, true).full_code() + "\n";
			// Call the constructor if it exists
			if (object->get_class()->get_method_UNSAFE("__constructor") != nullptr) {
				auto constructor_code = generate_constructor_call_code(object->get_address(), object->get_class());
				object_code += constructor_code.full_code();
			}
		}
	}

	*code << object_code << std::flush;
	return true;
}

/**
 * @brief Destruct all local objects in the code entity
 *
 * This function generates the code necessary to destruct all local objects in the code entity, and adds it to the code buffer.
 * That code calls the destructor for each local object, if it exists, and then deletes the object.
 *
 * It does not destruct local pointers, whose memory management is the responsibility of the programmer.
 *
 * This function is called by the compiler at the end of the scope of code entities which hold "block-local" scope
 * Class methods, bash functions, subshells, or simple blocks ("{ ... }") have block-local scope
 *   (i.e., their local objects are only accessible within the block of code they are defined in, and not in any nested blocks)
 * Supershells, by contrast, do not have block-local scope, and objects instantiated within them are expected to persist
 *  until the end of the containing method or function
 * 
 * @param program Pointer to the bpp_program
 */
void bpp_code_entity::destruct_local_objects(std::shared_ptr<bpp_program> program) {
	for (auto& [name, object] : local_objects) {
		if (object->is_pointer()) continue;
		code_segment delete_code = generate_delete_code(object, object->get_address(), program);
		*code << delete_code.full_code() << "\n" << std::flush;
	}
}

/**
 * @brief Return the contents of the main code buffer as a string
 */
std::string bpp_code_entity::get_code() const {
	std::shared_ptr<std::ostringstream> ss = std::dynamic_pointer_cast<std::ostringstream>(code);
	if (ss == nullptr) {
		return "";
	}
	return ss->str();
}

/**
 * @brief Return the contents of the pre-code buffer as a string
 */
std::string bpp_code_entity::get_pre_code() const {
	return nextline_buffer;
}

/**
 * @brief Return the contents of the post-code buffer as a string
 */
std::string bpp_code_entity::get_post_code() const {
	return postline_buffer;
}

void bpp_code_entity::set_requires_perfect_forwarding(bool require) {
	requires_perfect_forwarding = require;
}

bool bpp_code_entity::get_requires_perfect_forwarding() const {
	return requires_perfect_forwarding;
}

/**
 * @brief Take ownership of all local objects in another entity, by moving them into this entity's local object map
 *
 * This is used when exiting a supershell, since the parent code entity of the supershell should be responsible
 * for managing the lifetime of objects created within the supershell, as per the spec.
 * 
 * @param entity The entity to adopt the local objects of
 */
void bpp_code_entity::adopt(std::shared_ptr<bpp_entity> entity) {
	for (const auto& [name, object] : entity->get_local_objects()) {
		bpp_assert(
			!(local_objects.contains(name) || objects.contains(name)),
			"Name conflict when adopting local objects from supershell: " + name
		);
		local_objects[name] = object;
	}
}

} // namespace bpp
