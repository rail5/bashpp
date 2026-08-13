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

	bool self_reference = ids.front() == "this" || ids.front() == "super";
	bool super = ids.front() == "super";

	std::string first_id(ids.front());
	if (super) first_id = "this"; // For the purpose of looking up the object, treat @super as @this

	auto obj = context->get_object(first_id);

	if (obj == nullptr) {
		EntityResolutionError result;
		if (self_reference) {
			result.message = "Cannot use @this or @super outside of a class context";
		} else {
			result.message = "Object not found: " + std::string(ids.front());
		}
		if constexpr (provide_diagnostics) {
			result.token = ids.front();
			program->add_diagnostic({
				{file},
				ids.front().getLine(), ids.front().getCharPositionInLine(), ids.front().getValue().size(),
				result.message,
			});
		}
		return std::unexpected(std::move(result));
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
			EntityResolutionError result;
			result.message = this_class->get_name() + " has no parent class to reference with @super";
			if constexpr (provide_diagnostics) {
				result.token = ids.front();
				program->add_diagnostic({
					{file},
					ids.front().getLine(), ids.front().getCharPositionInLine(), ids.front().getValue().size(),
					result.message,
				});
			}
			return std::unexpected(std::move(result));
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
			EntityResolutionError result;
			result.message = "Invalid identifier: " + id + " (Bash++ identifiers cannot contain double underscores)";
			if constexpr (provide_diagnostics) {
				result.token = current_token;
				program->add_diagnostic({
					{file},
					current_token.getLine(), current_token.getCharPositionInLine(), current_token.getValue().size(),
					result.message,
				});
			}
			return std::unexpected(std::move(result));
		}

		auto data_member = current_class->get_datamember(id, context);
		auto method = current_class->get_method(id, context);

		if (data_member) {
			chain.append(data_member.value());
			current_class = data_member.value()->get_type().lock();
			if (current_class == nullptr && !ids.empty()) {
				EntityResolutionError result;
				result.message = "Unexpected identifier after primitive object reference";
				if constexpr (provide_diagnostics) {
					result.token = current_token;
					program->add_diagnostic({
						{file},
						current_token.getLine(), current_token.getCharPositionInLine(), current_token.getValue().size(),
						result.message,
					});
				}
				return std::unexpected(std::move(result));
			}
		} else if (method) {
			if (!ids.empty()) {
				EntityResolutionError result;
				result.message = "Unexpected identifier after method reference";
				if constexpr (provide_diagnostics) {
					result.token = current_token;
					program->add_diagnostic({
						{file},
						current_token.getLine(), current_token.getCharPositionInLine(), current_token.getValue().size(),
						result.message,
					});
				}
				return std::unexpected(std::move(result));
			}
			chain.set_method(method.value());
		} else if (data_member.error() == LookupError::INACCESSIBLE || method.error() == LookupError::INACCESSIBLE) {
			EntityResolutionError result;
			result.message = id + " is inaccessible in this context";
			if constexpr (provide_diagnostics) {
				result.token = current_token;
				program->add_diagnostic({
					{file},
					current_token.getLine(), current_token.getCharPositionInLine(), current_token.getValue().size(),
					result.message,
				});
			}
			return std::unexpected(std::move(result));
		} else {
			auto latest_entity = obj;
			if (!chain.empty()) latest_entity = chain.get_final_object().lock();
			EntityResolutionError result;
			result.message = latest_entity->get_name() + " has no member named " + id;
			if constexpr (provide_diagnostics) {
				result.token = current_token;
				program->add_diagnostic({
					{file},
					current_token.getLine(), current_token.getCharPositionInLine(), current_token.getValue().size(),
					result.message,
				});
			}
			return std::unexpected(std::move(result));
		}
	}

	return chain;
}

} // namespace bpp::IR
