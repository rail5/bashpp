/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <cstdint>
#include <filesystem>

namespace bpp {
constexpr std::filesystem::path get_standard_library_path() { return {"/usr/lib/bpp/stdlib/"}; }
} // namespace bpp

namespace bpp::IR {

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
class BashForOrSelectStatement;
class BashArithmeticForStatement;
class BashIfStatement;
class BashIfBranch;
class BashIfCondition;
class BashWhileOrUntilStatement;
class BashBreakOrContinueCommand;
class BashCaseStatement;
class BashCasePattern;
class Block;
class DeleteStatement;
class Program;
class IncludeStatement;
class Class;
class Method;
class Object;
class DataMember;
class MethodParameter;
class ThisPtr;
class CopyFromParam;
class RequestedAddressParam;
class DynamicCast;
class ObjectAssignment;
class ObjectInstantiation;
class ObjectReference;
class RawSubshell;
class String;
class SubshellSubstitution;
class Supershell;
class ValueAssignment;
class TypeofExpression;

namespace Builtins {
class SystemFunction;
class SystemMethod;
} // namespace Builtins

namespace Components {
class Named;
class Addressable;
class ClassMember;
} // namespace Components

template <class T>
concept NamedEntity = std::is_base_of_v<Entity, T> && std::is_base_of_v<Components::Named, T>;

} // namespace bpp::IR
