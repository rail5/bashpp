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

class ObjectReference : public CodeEntity, public std::enable_shared_from_this<ObjectReference> {
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
				std::weak_ptr<const Object> getRoot() const { return chain.front(); }
				std::weak_ptr<const Object> getFinalObject() const { return chain.back(); }
				bool hasMethod() const { return method.has_value(); }
				void setMethod(std::weak_ptr<const Method> m) { method = std::move(m); }
				std::weak_ptr<const Method> getMethod() const { return method.value_or(std::weak_ptr<const Method>()); }
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

		const ReferenceChain& getReferenceChain() const { return reference; }
		void setReferenceChain(ReferenceChain&& chain) { reference = std::move(chain); }

		bool isMethodCall() const { return reference.hasMethod(); }

		bool isPrimitive() const {
			if (isMethodCall()) return false;
			auto final_object = reference.getFinalObject().lock();
			bpp_assert(final_object != nullptr, "ObjectReference::isPrimitive() called on a reference chain with a null final object");
			return final_object->isPrimitive();
		}

		bool isPointer() const {
			if (!isPrimitive()) return false;
			auto final_object = reference.getFinalObject().lock();
			bpp_assert(final_object != nullptr, "ObjectReference::isPointer() called on a reference chain with a null final object");
			return final_object->isPointer();
		}

		bool isNonprimitive() const {
			if (isMethodCall()) return false;
			auto final_object = reference.getFinalObject().lock();
			bpp_assert(final_object != nullptr, "ObjectReference::isNonprimitive() called on a reference chain with a null final object");
			return !isPrimitive();
		}

		/**
		 * @brief This amends the object reference to include a call to the final object's toPrimitive method.
		 */
		void addToPrimitiveCall() {
			bpp_assert(!reference.empty(), "ObjectReference::addToPrimitiveCall() called on an empty reference chain");
			auto final_object = reference.getFinalObject().lock();
			bpp_assert(final_object != nullptr, "ObjectReference::addToPrimitiveCall() called on a reference chain with a null final object");
			bpp_assert(!final_object->isPrimitive() || final_object->isPointer(), "ObjectReference::addToPrimitiveCall() called on a reference chain with a primitive final object");
			auto final_class = final_object->getType().lock();
			bpp_assert(final_class != nullptr, "ObjectReference::addToPrimitiveCall(): final object has no type");
			auto to_primitive_method = final_class->getMethod_UNSAFE("toPrimitive");
			bpp_assert(to_primitive_method != nullptr, "ObjectReference::addToPrimitiveCall(): final object's class has no toPrimitive method");
			reference.setMethod(to_primitive_method);
		}

		void setLvalue(bool lvalue) { this->lvalue = lvalue; }
		bool isLvalue() const { return lvalue; }
		void setAddressOf(bool address_of) { this->address_of = address_of; }
		bool isAddressOf() const { return address_of; }

		bpp::CodeGen::CodeSegment generateCode(bpp::CodeGen::CodeGenState* state) const override;
		PRETTYPRINT_OVERRIDE();

		ObjectReference() = default;
		~ObjectReference() = default;
		ObjectReference(const ObjectReference& other) = default;
		ObjectReference& operator=(const ObjectReference& other) = default;
		ObjectReference(ObjectReference&& other) noexcept = default;
		ObjectReference& operator=(ObjectReference&& other) noexcept = default;
	private:
		ReferenceChain reference;
		bool lvalue = false;
		bool address_of = false;
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
	auto program = context->getContainingProgram().lock();
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

	auto obj = context->getObject(first_id);

	if (self_reference) {
		if (auto containing_class = context->getContainingClass().lock()) {
			obj = super ? containing_class->getSuperPtr() : containing_class->getThisPtr();
			if (!obj && super) return fail(containing_class->getName() + " has no parent class to reference with @super", first);
		} else {
			return fail("Cannot use @this or @super outside of a class context", first);
		}
	}

	if (obj == nullptr) {
		return fail("Object not found: " + first_id, first);
	}

	if constexpr (provide_diagnostics) {
		obj->addReferencePosition(SymbolPosition{file, first.getLine(), first.getCharPositionInLine()});
		obj->markReferencedBy(result);
	}

	auto current_class = obj->getType().lock();
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

		auto data_member = current_class->getDatamember(id, context);
		auto method = current_class->getMethod(id, context);

		if (data_member) {
			chain.append(data_member.value());
			current_class = data_member.value()->getType().lock();

			if constexpr (provide_diagnostics) {
				data_member.value()->addReferencePosition(SymbolPosition{file, current_token.getLine(), current_token.getCharPositionInLine()});
				data_member.value()->markReferencedBy(result);
			}

			if (current_class == nullptr && !std::ranges::empty(remaining)) {
				return fail("Unexpected identifier after primitive object reference", remaining.front());
			}
		} else if (method) {
			if constexpr (provide_diagnostics) {
				method.value()->addReferencePosition(SymbolPosition{file, current_token.getLine(), current_token.getCharPositionInLine()});
				method.value()->markReferencedBy(result);
			}
			if (!std::ranges::empty(remaining)) {
				return fail("Unexpected identifier after method reference", remaining.front());
			}
			chain.setMethod(method.value());
		} else if (data_member.error() == LookupError::INACCESSIBLE || method.error() == LookupError::INACCESSIBLE) {
			return fail(id + " is inaccessible in this context", current_token);
		} else {
			std::shared_ptr<const Object> latest_entity = obj;
			if (!chain.empty()) latest_entity = chain.getFinalObject().lock();
			return fail(latest_entity->getName() + " has no member named " + id, current_token);
		}
	}

	result->setReferenceChain(std::move(chain));

	return result;
}

} // namespace bpp::IR
