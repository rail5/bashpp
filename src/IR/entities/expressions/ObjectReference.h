/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <IR/bpp.h>
#include <IR/entities/expressions/String.h>
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

namespace bpp::IR {

/**
 * @brief A reference to a non-primitive object in the IR
 *
 * E.g., @object.member
 */
class ObjectReference : public StringType, public std::enable_shared_from_this<ObjectReference> {
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
				void removeMethod() { method.reset(); }
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
		std::weak_ptr<const Object> getFinalObject() const { return reference.getFinalObject(); }

		/**
		 * @brief Whether this reference is ultimately primitive
		 * This check takes into account semantics such as pointer dereferencing (*@obj) and address-of (&@obj), as well as the fact that pointers are primitives
		 *
		 * E.g.: &@obj is primitive, even if @obj is non-primitive
		 * And: *@ptr (if .toPrimitive is not implied) is non-primitive, even though pointers are primitives
		 * And: @obj.method is primitive, because the output of a method is always primitive
		 *
		 * If you want to check whether the referenced object *itself* is primitive (disregarding language semantics), check on getFinalObject()
		 * @return true if the reference is ultimately primitive, false if not
		 */
		bool isPrimitive() const {
			auto final_object = getFinalObject().lock();
			bpp_assert(final_object != nullptr, "Final object is null");
			if (isAddressOf()) return true; // '&' transforms everything into a primitive
			if (reference.hasMethod()) return true; // The output of a method is a primitive
			if (final_object->isPointer() && isPointerDereference()) return false; // Pointers are primitives, but dereferenced pointers are objects
			if (final_object->isPrimitive()) return true;

			return false;
		}

		/**
		 * @brief Whether this reference is ultimately a pointer
		 * This check takes into account semantics such as pointer dereferencing (*@obj) and address-of (&@obj)
		 *
		 * E.g.: &@ptr is not a pointer, even though @ptr is a pointer
		 * And: *@ptr is not a pointer, even though @ptr is a pointer
		 *
		 * If you want to check whether the referenced object *itself* is a pointer (disregarding language semantics), check on getFinalObject()
		 * @return true if the reference is ultimately a pointer, false if not
		 */
		bool isPointer() const {
			auto final_object = reference.getFinalObject().lock();
			bpp_assert(final_object != nullptr, "Final object is null");
			if (isAddressOf()) return false;
			if (reference.hasMethod()) return false;

			return final_object->isPointer() && !isPointerDereference();
		}

		/**
		 * @brief Whether this reference is ultimately non-primitive
		 * This is the inverse of isPrimitive().
		 *
		 * @return true if the reference is ultimately non-primitive, false if not
		 */
		bool isNonprimitive() const { return !isPrimitive(); }

		/**
		 * @brief This amends the object reference to include a call to a method of the final object in the reference chain.
		 *
		 * This function is UNSAFE in several ways:
		 *  1. It does not check accessibility restrictions of the requested method against current context
		 *  2. It will throw an InternalError if the requested method does not exist
		 *  3. It will throw an InternalError if the final object is primitive
		 *
		 * It is the caller's responsibility to ensure safe use of this function.
		 *
		 * This function should only be used to call methods that are guaranteed to exist, such as toPrimitive, __new, etc.
		 * 
		 * @param method_name The name of the method to call on the final object in the reference chain
		 */
		void addMethodCall_UNSAFE(const std::string& method_name) {
			bpp_assert(!reference.empty(), "Reference chain is empty");
			auto final_object = reference.getFinalObject().lock();
			bpp_assert(final_object != nullptr, "Final object is null");
			bpp_assert(isNonprimitive() || isPointer(), "Final object is primitive");
			auto final_class = final_object->getType().lock();
			bpp_assert(final_class != nullptr, "Final object has no type");
			auto method = final_class->getMethod_UNSAFE(method_name);
			bpp_assert(method != nullptr, "Final object's class has no method named " + method_name);
			reference.setMethod(method);
		}

		void setLvalue(bool lvalue) { this->lvalue = lvalue; }
		bool isLvalue() const { return lvalue; }
		void setAddressOf(bool address_of) { this->address_of = address_of; }
		bool isAddressOf() const { return address_of; }
		void setPointerDereference(bool pointer_dereference) { this->pointer_dereference = pointer_dereference; }
		bool isPointerDereference() const { return pointer_dereference; }
		void setHasHashkey(bool has_hashkey) { this->has_hashkey = has_hashkey; }
		bool hasHashkey() const { return has_hashkey; }

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
		bool pointer_dereference = false;
		bool has_hashkey = false; // FIXME(@rail5): Find a better way to represent this
		// What's being represented is @{#object.reference[@]} vs @{object.reference[@]}
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
	bpp_assert(context != nullptr, "Context pointer is null");
	auto program = context->getContainingProgram().lock();
	bpp_assert(program != nullptr, "Containing program is null");
	bpp_assert(!std::ranges::empty(ids), "At least one identifier is required");

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
	bpp_assert(current_class != nullptr, "Object has no type");
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

#ifndef NDEBUG
std::string get_reference_chain_prettyprint_string(const ObjectReference::ReferenceChain& chain);
#endif // NDEBUG

} // namespace bpp::IR
