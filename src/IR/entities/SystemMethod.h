/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <IR/bpp.h>
#include <IR/entities/Method.h>

namespace bpp::IR::Builtins {

/**
 * @brief A Bash++ system method, which is a special type of method whose contents are automatically generated for each class that contains it,
 * based on the state of the class entity.
 * The system methods represented by this class are __new, __delete, and __copy.
 * 
 */
class SystemMethod : public Method {
	public:
		enum class Type : std::uint8_t {
			NEW,
			DELETE,
			COPY,
		};
		SystemMethod() = delete;
		explicit SystemMethod(Type type) : type(type) {
			switch (type) {
				case Type::NEW: set_name("__new"); break;
				case Type::DELETE: set_name("__delete"); break;
				case Type::COPY: set_name("__copy"); break;
				default: throw bpp::ErrorHandling::InternalError("Unknown SystemMethodType");
			}
		}

		bpp::CodeGen::CodeSegment generate_inline_code(bpp::CodeGen::CodeGenState* state, bool localize, std::shared_ptr<const Object> obj = nullptr) const;
		bpp::CodeGen::CodeSegment generate_code(bpp::CodeGen::CodeGenState* state) const override;
		PRETTYPRINT_OVERRIDE();

	private:
		 Type type = Type::NEW;

		 bpp::CodeGen::CodeSegment generate_inline_new_code(bpp::CodeGen::CodeGenState* state, bool localize, std::shared_ptr<const Object> obj) const;
		 bpp::CodeGen::CodeSegment generate_inline_delete_code(bpp::CodeGen::CodeGenState* state, bool localize, std::shared_ptr<const Object> obj) const;
		 bpp::CodeGen::CodeSegment generate_inline_copy_code(bpp::CodeGen::CodeGenState* state, bool localize, std::shared_ptr<const Object> obj) const;
};

} // namespace bpp::IR::Builtins
