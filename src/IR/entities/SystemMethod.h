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
 * Only __new can be inlined.
 *
 * __delete implicitly calls the object's destructors, if any exist, before freeing the object's memory.
 * NOTE: This means that the compiler should never call destructors explicitly.
 *
 * __constructor and __destructor are not represented because their contents are user-defined.
 */
class SystemMethod : public Method {
	public:
		enum class Type : std::uint8_t {
			NEW,
			DELETE,
			COPY,
		};
		SystemMethod() = delete;
		explicit SystemMethod(Type type) : Method(getNameForType(type)), type(type) {}

		bpp::CodeGen::CodeSegment generateInlineNewCode(bpp::CodeGen::CodeGenState* state, bool localize, const Object* obj = nullptr) const;
		bpp::CodeGen::CodeSegment generateCode(bpp::CodeGen::CodeGenState* state) const override;
		PRETTYPRINT_OVERRIDE();

	private:
		Type type = Type::NEW;

		bpp::CodeGen::CodeSegment generateCopyCode(bpp::CodeGen::CodeGenState* state) const;
		bpp::CodeGen::CodeSegment generateDeleteCode(bpp::CodeGen::CodeGenState* state) const;

		static std::string getNameForType(Type type) {
			switch (type) {
				case Type::NEW: return "__new";
				case Type::DELETE: return "__delete";
				case Type::COPY: return "__copy";
				default: throw bpp::ErrorHandling::InternalError("Unknown SystemMethodType");
			}
		}
};

} // namespace bpp::IR::Builtins
