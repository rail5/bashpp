/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <IR/bpp.h>
#include <IR/entities/CodeEntity.h>
#include <IR/entities/Program.h>
#include <IR/entities/Object.h>
#include <IR/entities/DataMember.h>
#include <IR/entities/Method.h>
#include <AST/Token.h>

#include <error/InternalError.h>

#include <memory>
#include <filesystem>
#include <span>
#include <ranges>
#include <expected>

/**
 * @brief A reference to a non-primitive object in the IR
 */
namespace bpp::IR {

class ObjectReference : public CodeEntity {
	public:
		/**
		 * @brief A chain starting from a root object and following a series of data member accesses to reach a final object.
		 *
		 * Each element after the root must be a pointer to a DataMember of the previous object in the chain.
		 */
		class ReferenceChain {
			private:
				std::vector<std::weak_ptr<const Object>> chain;
				std::optional<std::weak_ptr<const Method>> method = std::nullopt;
			public:
				explicit ReferenceChain(std::shared_ptr<const Object> root) { chain.push_back(root); }
				void append(std::shared_ptr<const DataMember> datamember) { chain.push_back(datamember); }
				std::weak_ptr<const Object> get_root() const { return chain.front(); }
				std::weak_ptr<const Object> get_final_object() const { return chain.back(); }
				bool has_method() const { return method.has_value(); }
				void set_method(std::weak_ptr<const Method> m) { method = std::move(m); }
				std::weak_ptr<const Method> get_method() const { return method.value_or(std::weak_ptr<const Method>()); }
				std::size_t size() const { return chain.size(); }
				bool empty() const { return chain.empty(); }

				// Iterators
				auto begin() { return chain.begin(); }
				auto end() { return chain.end(); }
				auto begin() const { return chain.begin(); }
				auto end() const { return chain.end(); }

				ReferenceChain() = default;
				~ReferenceChain() = default;
				ReferenceChain(const ReferenceChain& other) = default;
				ReferenceChain& operator=(const ReferenceChain& other) = default;
				ReferenceChain(ReferenceChain&& other) noexcept = default;
				ReferenceChain& operator=(ReferenceChain&& other) noexcept = default;
		};

		const ReferenceChain& get_reference_chain() const { return reference; }
		void set_reference_chain(ReferenceChain&& chain) { reference = std::move(chain); }

		bpp::CodeGen::CodeSegment generate_code(bpp::CodeGen::CodeGenState* state) const override;
		PRETTYPRINT_OVERRIDE();

		ObjectReference() = default;
		~ObjectReference() = default;
		ObjectReference(const ObjectReference& other) = default;
		ObjectReference& operator=(const ObjectReference& other) = default;
		ObjectReference(ObjectReference&& other) noexcept = default;
		ObjectReference& operator=(ObjectReference&& other) noexcept = default;
	private:
		ReferenceChain reference;
};


struct EntityResolutionError {
	std::string message;
	std::optional<AST::Token<std::string>> token = std::nullopt;
};

template <typename T>
concept StringLike =
	std::same_as<std::remove_cvref_t<T>, std::string>
	|| std::same_as<std::remove_cvref_t<T>, std::string_view>;

template <typename T>
concept TokenLike = std::same_as<std::remove_cvref_t<T>, AST::Token<std::string>>;

template <typename T>
concept IdentifierElement = StringLike<T> || TokenLike<T>;

template <typename T>
requires IdentifierElement<T>
std::expected<std::shared_ptr<ObjectReference>, EntityResolutionError> resolve_entity(
	std::filesystem::path file,
	std::shared_ptr<const Entity> context,
	std::span<T> ids
) {
	bpp_assert(context != nullptr, "resolve_entity() should be called with a non-null context pointer");
	auto program = context->get_containing_program().lock();
	bpp_assert(program != nullptr, "resolve_entity() should be called with a context that is part of a program");
	bpp_assert(!std::ranges::empty(ids), "resolve_entity() should be called with at least one identifier");

	std::shared_ptr<ObjectReference> result = std::make_shared<ObjectReference>();

	constexpr bool provide_diagnostics = TokenLike<T>;

	auto fail = [&](std::string message, auto const& token) -> std::unexpected<EntityResolutionError> {
		EntityResolutionError result;
		result.message = std::move(message);

		if constexpr (provide_diagnostics) {
			result.token = token;
		}

		return std::unexpected(std::move(result));
	};

	const auto first = ids.front();
	std::string first_id(first);
	bool self_reference = first_id == "this" || first_id == "super";
	bool super = first_id == "super";

	auto obj = context->get_object(first_id);

	if (self_reference) {
		if (auto containing_class = context->get_containing_class().lock()) {
			obj = super ? containing_class->get_super_ptr() : containing_class->get_this_ptr();
			if (!obj && super) return fail(containing_class->get_name() + " has no parent class to reference with @super", first);
		} else {
			return fail("Cannot use @this or @super outside of a class context", first);
		}
	}

	if (obj == nullptr) {
		return fail("Object not found: " + first_id, first);
	}

	if constexpr (provide_diagnostics) {
		obj->add_reference_position(SymbolPosition{file, first.getLine(), first.getCharPositionInLine()});
		obj->mark_referenced_by(result);
	}

	auto current_class = obj->get_type().lock();
	bpp_assert(current_class != nullptr, "Object has no type in resolve_entity()");
	auto remaining = ids.subspan(1);

	ObjectReference::ReferenceChain chain(obj);

	while (!std::ranges::empty(remaining)) {
		const auto current_token = remaining.front();
		const std::string id(current_token);
		remaining = remaining.subspan(1);

		if (id.contains("__")) {
			return fail("Invalid identifier: " + id + " (Bash++ identifiers cannot contain double underscores)", current_token);
		}

		auto data_member = current_class->get_datamember(id, context);
		auto method = current_class->get_method(id, context);

		if (data_member) {
			chain.append(data_member.value());
			current_class = data_member.value()->get_type().lock();

			if constexpr (provide_diagnostics) {
				data_member.value()->add_reference_position(SymbolPosition{file, current_token.getLine(), current_token.getCharPositionInLine()});
				data_member.value()->mark_referenced_by(result);
			}

			if (current_class == nullptr && !std::ranges::empty(remaining)) {
				return fail("Unexpected identifier after primitive object reference", remaining.front());
			}
		} else if (method) {
			if constexpr (provide_diagnostics) {
				method.value()->add_reference_position(SymbolPosition{file, current_token.getLine(), current_token.getCharPositionInLine()});
				method.value()->mark_referenced_by(result);
			}
			if (!std::ranges::empty(remaining)) {
				return fail("Unexpected identifier after method reference", remaining.front());
			}
			chain.set_method(method.value());
		} else if (data_member.error() == LookupError::INACCESSIBLE || method.error() == LookupError::INACCESSIBLE) {
			return fail(id + " is inaccessible in this context", current_token);
		} else {
			std::shared_ptr<const Object> latest_entity = obj;
			if (!chain.empty()) latest_entity = chain.get_final_object().lock();
			return fail(latest_entity->get_name() + " has no member named " + id, current_token);
		}
	}

	result->set_reference_chain(std::move(chain));

	return result;
}

} // namespace bpp::IR
