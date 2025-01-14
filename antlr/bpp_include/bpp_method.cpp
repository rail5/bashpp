/**
 * Copyright (C) 2025 rail5
 * Bash++: Bash with classes
 */

#ifndef ANTLR_BPP_INCLUDE_BPP_METHOD_CPP_
#define ANTLR_BPP_INCLUDE_BPP_METHOD_CPP_

#include "bpp.h"

namespace bpp {

bpp_method::bpp_method(std::string name) : name(name) {}

bool bpp_method::add_parameter(bpp_method_parameter parameter) {
	std::string name = parameter.get_name();
	for (auto& p : parameters) {
		if (p.get_name() == name) {
			return false;
		}
	}
	parameters.push_back(parameter);
	return true;
}

void bpp_method::set_name(std::string name) {
	this->name = name;
}

void bpp_method::set_method_body(std::string method_body) {
	this->method_body = method_body;
}

void bpp_method::set_scope(bpp_scope scope) {
	this->scope = scope;
}

void bpp_method::set_virtual(bool is_virtual) {
	m_is_virtual = is_virtual;
}

std::string bpp_method::get_name() const {
	return name;
}

std::vector<bpp_method_parameter> bpp_method::get_parameters() const {
	return parameters;
}

std::string bpp_method::get_method_body() const {
	return method_body;
}

bpp_scope bpp_method::get_scope() const {
	return scope;
}

bool bpp_method::is_virtual() const {
	return m_is_virtual;
}

std::string bpp_method::get_signature() const {
	/**
	 * The format for method signatures is:
	 * <method_name>____<parameter1_type>__<parameter2_type>__...__<parameterN_type>
	 */
	std::string signature = name;
	signature += "__";
	for (auto& p : parameters) {
		signature += "__";
		signature += p.get_type();
	}
	return signature;
}

} // namespace bpp

#endif // ANTLR_BPP_INCLUDE_BPP_METHOD_CPP
