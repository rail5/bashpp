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
#include <deque>
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
				std::vector<std::weak_ptr<Object>> chain;
				std::optional<std::weak_ptr<Method>> method = std::nullopt;
			public:
				explicit ReferenceChain(std::shared_ptr<Object> root) { chain.push_back(root); }
				void append(std::shared_ptr<DataMember> datamember) { chain.push_back(datamember); }
				std::weak_ptr<Object> get_root() const { return chain.front(); }
				std::weak_ptr<Object> get_final_object() const { return chain.back(); }
				bool has_method() const { return method.has_value(); }
				void set_method(std::weak_ptr<Method> m) { method = std::move(m); }
				std::weak_ptr<Method> get_method() const { return method.value_or(std::weak_ptr<Method>()); }
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
concept DequeOfStringsOrStringViews =
	std::is_same_v<std::remove_cvref_t<T>, std::deque<std::string>>
	|| std::is_same_v<std::remove_cvref_t<T>, std::deque<std::string_view>>;

template <typename T>
concept DequeOfASTTokens = std::is_same_v<std::remove_cvref_t<T>, std::deque<AST::Token<std::string>>>;

template <typename T>
concept EitherStringsOrASTTokens = DequeOfStringsOrStringViews<T> || DequeOfASTTokens<T>;


std::expected<ObjectReference::ReferenceChain, EntityResolutionError> resolve_entity(
	std::filesystem::path file,
	std::shared_ptr<Entity> context,
	EitherStringsOrASTTokens auto&& ids
) {
	bpp_assert(context != nullptr, "resolve_entity() should be called with a non-null context pointer");
	auto program = context->get_containing_program().lock();
	bpp_assert(program != nullptr, "resolve_entity() should be called with a context that is part of a program");

	// If 'ids' is a container of AST::Token<std::string>, then we can provide diagnostics to the containing program
	// If not, then we'll skip that
	constexpr bool provide_diagnostics = DequeOfASTTokens<decltype(ids)>;

	// Helper for failures:
	auto fail = [&](std::string message, auto const& token) -> std::unexpected<EntityResolutionError> {
		EntityResolutionError result;
		result.message = std::move(message);

		if constexpr (provide_diagnostics) {
			result.token = token;
			program->add_diagnostic({
				{file},
				token.getLine(), token.getCharPositionInLine(),
				static_cast<std::uint32_t>(token.getValue().size()),
				result.message,
			});
		}

		return std::unexpected(std::move(result));
	};

	bool self_reference = ids.front() == "this" || ids.front() == "super";
	bool super = ids.front() == "super";

	std::string first_id(ids.front());
	if (super) first_id = "this"; // For the purpose of looking up the object, treat @super as @this

	auto obj = context->get_object(first_id);

	if (obj == nullptr) {
		return fail(
			self_reference
				? "Cannot use @this or @super outside of a class context"
				: "Object not found: " + std::string(ids.front()),
			ids.front()
		);
	}

	if constexpr (provide_diagnostics) {
		obj->add_reference_position({file, ids.front().getLine(), ids.front().getCharPositionInLine()});
	}

	// Special-case: if @super, create a "faux" object as a copy of the 'this' pointer,
	// but with the type of the parent class.
	if (super) {
		const auto this_class = obj->get_type().lock();
		bpp_assert(this_class != nullptr, "Object has no type in resolve_entity()");
		const auto parent_class = this_class->get_parent_class();
		if (parent_class == nullptr) {
			return fail(this_class->get_name() + " has no parent class to reference with @super", ids.front());
		}
		auto faux_object = std::make_shared<Object>(*obj);
		faux_object->set_name("super");
		faux_object->set_type(parent_class);
		obj = std::move(faux_object);
	}

	auto current_class = obj->get_type().lock();
	bpp_assert(context != nullptr, "Object has no type in resolve_entity()");
	ids.pop_front();

	ObjectReference::ReferenceChain chain(obj);

	while (!ids.empty()) {
		const auto current_token = ids.front();
		const std::string id(current_token);
		ids.pop_front();

		if (id.contains("__")) {
			return fail("Invalid identifier: " + id + " (Bash++ identifiers cannot contain double underscores)", current_token);
		}

		auto data_member = current_class->get_datamember(id, context);
		auto method = current_class->get_method(id, context);

		if (data_member) {
			chain.append(data_member.value());
			current_class = data_member.value()->get_type().lock();
			if (current_class == nullptr && !ids.empty()) {
				return fail("Unexpected identifier after primitive object reference", ids.front());
			}
		} else if (method) {
			if (!ids.empty()) {
				return fail("Unexpected identifier after method reference", ids.front());
			}
			chain.set_method(method.value());
		} else if (data_member.error() == LookupError::INACCESSIBLE || method.error() == LookupError::INACCESSIBLE) {
			return fail(id + " is inaccessible in this context", current_token);
		} else {
			auto latest_entity = obj;
			if (!chain.empty()) latest_entity = chain.get_final_object().lock();
			return fail(latest_entity->get_name() + " has no member named " + id, current_token);
		}
	}

	return chain;
}

} // namespace bpp::IR
