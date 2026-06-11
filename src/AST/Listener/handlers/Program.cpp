/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <AST/Listener/Listener.h>

#include <iostream>

namespace bpp::AST {

template <>
void Listener::enter(std::shared_ptr<Program> node) {
	std::cout << "Entered program node" << std::endl;
}

template <>
void Listener::exit(std::shared_ptr<Program> node) {
	std::cout << "Exited program node" << std::endl;
}

} // namespace bpp::AST
