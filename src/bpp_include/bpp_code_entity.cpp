/**
 * Copyright (C) 2025 rail5
 * Bash++: Bash with classes
 */

#ifndef SRC_BPP_INCLUDE_BPP_CODE_ENTITY_CPP_
#define SRC_BPP_INCLUDE_BPP_CODE_ENTITY_CPP_

#include "bpp.h"

namespace bpp {

bpp_code_entity::bpp_code_entity() {}

bool bpp_code_entity::add_class(std::shared_ptr<bpp_class> class_) {
	std::string name = class_->get_name();
	if (classes.find(name) != classes.end()) {
		return false;
	}
	classes[name] = class_;
	return true;
}

bool bpp_code_entity::add_object(std::shared_ptr<bpp_object> object) {
	std::string name = object->get_name();
	if (objects.find(name) != objects.end()) {
		return false;
	}

	// Verify that the type of the object is a valid class
	std::string type = object->get_class()->get_name();
	if (classes.find(type) == classes.end()) {
		return false;
	}

	objects[name] = object;
	return true;
}

void bpp_code_entity::add_code(std::string code) {
	// If the code has a newline char, flush the nextline_buffer and the postline_buffer
	if (code.find("\n") != std::string::npos) {
		flush_nextline_buffer();

		this->code += code;
		if (code.back() != '\n') {
			this->code += "\n";
		}
		
		flush_postline_buffer();
		return;
	}

	// Otherwise, add the code to the nextline_buffer
	nextline_buffer += code;
}

void bpp_code_entity::add_code_to_previous_line(std::string code) {
	this->code += code;
}

void bpp_code_entity::add_code_to_next_line(std::string code) {
	postline_buffer += code;
}

void bpp_code_entity::flush_nextline_buffer() {
	if (!nextline_buffer.empty()) {
		code += nextline_buffer;
		if (nextline_buffer.back() != '\n') {
			code += "\n";
		}
	}
	nextline_buffer = "";
}

void bpp_code_entity::flush_postline_buffer() {
	if (!postline_buffer.empty()) {
		code += postline_buffer;
		if (postline_buffer.back() != '\n') {
			code += "\n";
		}
	}
	postline_buffer = "";
}

void bpp_code_entity::flush_code_buffers() {
	flush_nextline_buffer();
	flush_postline_buffer();
}

std::vector<std::shared_ptr<bpp_class>> bpp_code_entity::get_classes() const {
	std::vector<std::shared_ptr<bpp_class>> result;
	for (auto& c : classes) {
		result.push_back(c.second);
	}
	return result;
}

std::vector<std::shared_ptr<bpp_object>> bpp_code_entity::get_objects() const {
	std::vector<std::shared_ptr<bpp_object>> result;
	for (auto& o : objects) {
		result.push_back(o.second);
	}

	for (auto& o : local_objects) {
		result.push_back(o.second);
	}
	return result;
}

std::string bpp_code_entity::get_code() const {
	return code;
}

std::string bpp_code_entity::get_pre_code() const {
	return "";
}

std::string bpp_code_entity::get_post_code() const {
	return "";
}

std::shared_ptr<bpp::bpp_class> bpp_code_entity::get_class(std::string name) {
	if (classes.find(name) == classes.end()) {
		return nullptr;
	}
	return classes[name];
}

std::shared_ptr<bpp::bpp_object> bpp_code_entity::get_object(std::string name) {
	if (local_objects.find(name) != local_objects.end()) {
		return local_objects[name];
	}
	
	if (objects.find(name) != objects.end()) {
		return objects[name];
	}

	return nullptr;
}

void bpp_code_entity::inherit(std::shared_ptr<bpp_code_entity> parent) {
	for (auto& c : parent->get_classes()) {
		classes[c->get_name()] = c;
	}
	for (auto& o : parent->get_objects()) {
		objects[o->get_name()] = o;
	}
}

} // namespace bpp

#endif // SRC_BPP_INCLUDE_BPP_CODE_ENTITY_CPP_