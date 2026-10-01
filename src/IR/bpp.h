/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <cstdint>
#include <filesystem>

namespace bpp {
constexpr std::filesystem::path get_standard_library_path() {
	return {"/usr/lib/bpp/stdlib/"};
}
} // namespace bpp

namespace bpp::IR {

struct SymbolPosition {
	std::filesystem::path file;
	std::uint64_t line = 0;
	std::uint64_t column = 0;

	SymbolPosition() = default;
	SymbolPosition(const std::filesystem::path& file, std::uint64_t line, std::uint64_t column)
		: file(file), line(line), column(column) {}
};

enum class VisibilityScope : std::uint8_t {
	PUBLIC,
	PROTECTED,
	PRIVATE,
	INACCESSIBLE,
};

enum class LookupError : std::uint8_t {
	NOT_FOUND,
	INACCESSIBLE,
};

enum class NameConflictError : std::uint8_t {
	EXISTING_CLASS,
	EXISTING_OBJECT,
	EXISTING_METHOD,
	EXISTING_DATAMEMBER,
	EXISTING_PARAMETER,
};

// Forward decl. entity types:
class Entity;
class CodeEntity;
class BashFunction;
class BashPipeline;
class Program;
class IncludedProgram;
class Class;
class Method;
class Object;
class DataMember;
class MethodParameter;
class ThisPtr;

class DynamicCast;
class ObjectAssignment;
class ObjectInstantiation;
class ObjectReference;
class RawSubshell;
class String;
class SubshellSubstitution;
class Supershell;
class ValueAssignment;

namespace Components {

class Named;
class Addressable;
class ClassMember;

} // namespace Components

} // namespace bpp::IR
