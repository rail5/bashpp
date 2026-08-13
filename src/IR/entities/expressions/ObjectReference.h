/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <IR/bpp.h>
#include <IR/entities/Entity.h>
#include <IR/entities/Program.h>
#include <IR/entities/Object.h>
#include <IR/entities/DataMember.h>
#include <IR/entities/Method.h>
#include <AST/Token.h>

#include <error/InternalError.h>

#include <memory>
#include <filesystem>
#include <deque>

/**
 * @brief A reference to a non-primitive object in the IR
 *
 * This is a pure virtual base class.
 */
namespace bpp::IR {

class ObjectReference : public Entity {
	public:
		/**
		 * @brief A chain starting from a root object, following to an inner (data member) object, etc, ultimately ending at the referenced object.
		 */
		struct ReferenceChain {
			std::weak_ptr<Object> root;
			std::vector<std::weak_ptr<DataMember>> chain;

			explicit ReferenceChain(std::shared_ptr<Object> root) : root(root) {}
			void append(std::shared_ptr<DataMember> datamember) { chain.push_back(datamember); }
		};

		explicit ObjectReference(std::shared_ptr<Object> object) : reference(object) {}
		explicit ObjectReference(ReferenceChain&& chain) : reference(std::move(chain)) {}

		const ReferenceChain& get_reference_chain() const { return reference; }

		bpp::CodeGen::CodeSegment generate_code(bpp::CodeGen::CodeGenState* state) const override;
		PRETTYPRINT_OVERRIDE() = 0;

		virtual ~ObjectReference() = 0; // Pure virtual destructor to prevent direct instantiation

		ObjectReference(const ObjectReference&) = default;
		ObjectReference(ObjectReference&&) = default;
		ObjectReference& operator=(const ObjectReference&) = default;
		ObjectReference& operator=(ObjectReference&&) = default;
	private:
		ReferenceChain reference;
};

class MethodCall : public ObjectReference {
	private:
		std::weak_ptr<Method> method;
	public:
		MethodCall() = delete;
		MethodCall(std::shared_ptr<Object> object, std::shared_ptr<Method> method) : ObjectReference(object), method(method) {}
		MethodCall(ReferenceChain&& chain, std::shared_ptr<Method> method) : ObjectReference(std::move(chain)), method(method) {}
		void set_method(std::shared_ptr<Method> method) { this->method = method; }
		std::shared_ptr<Method> get_method() const { return method.lock(); }

		bpp::CodeGen::CodeSegment generate_code(bpp::CodeGen::CodeGenState* state) const override;
		PRETTYPRINT_OVERRIDE();

		~MethodCall() override = default;
		MethodCall(const MethodCall&) = default;
		MethodCall(MethodCall&&) = default;
		MethodCall& operator=(const MethodCall&) = default;
		MethodCall& operator=(MethodCall&&) = default;
};

class DataMemberAccess : public ObjectReference {
	public:
		bpp::CodeGen::CodeSegment generate_code(bpp::CodeGen::CodeGenState* state) const override;
		PRETTYPRINT_OVERRIDE();

		DataMemberAccess() = delete;
		explicit DataMemberAccess(std::shared_ptr<Object> object) : ObjectReference(object) {}
		explicit DataMemberAccess(ReferenceChain&& chain) : ObjectReference(std::move(chain)) {}
		~DataMemberAccess() override = default;
		DataMemberAccess(const DataMemberAccess&) = default;
		DataMemberAccess(DataMemberAccess&&) = default;
		DataMemberAccess& operator=(const DataMemberAccess&) = default;
		DataMemberAccess& operator=(DataMemberAccess&&) = default;
};


struct EntityResolution {
	std::shared_ptr<Object> object = nullptr;
	std::shared_ptr<DataMemberAccess> data_member_access = nullptr;
	std::shared_ptr<MethodCall> method_call = nullptr;

	std::optional<std::string> error_message = std::nullopt;
	std::optional<AST::Token<std::string>> error_token = std::nullopt;

	bool is_object() const { return object != nullptr; }
	bool is_data_member_access() const { return data_member_access != nullptr; }
	bool is_method_call() const { return method_call != nullptr; }
};

template <typename T>
concept DequeOfStringsOrStringViews =
	std::is_same_v<std::remove_cvref_t<T>, std::deque<std::string>>
	|| std::is_same_v<std::remove_cvref_t<T>, std::deque<std::string_view>>;

template <typename T>
concept DequeOfASTTokens = std::is_same_v<std::remove_cvref_t<T>, std::deque<AST::Token<std::string>>>;

template <typename T>
concept EitherStringsOrASTTokens = DequeOfStringsOrStringViews<T> || DequeOfASTTokens<T>;


EntityResolution resolve_entity(
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

	EntityResolution result;

	std::string first_id(ids.front());
	if (super) first_id = "this"; // For the purpose of looking up the object, treat @super as @this

	result.object = context->get_object(first_id);

	if (result.object == nullptr) {
		if (self_reference) {
			result.error_message = "Cannot use @this or @super outside of a class context";
		} else {
			result.error_message = "Object not found: " + std::string(ids.front());
		}
		if constexpr (provide_diagnostics) {
			result.error_token = ids.front();
			program->add_diagnostic({
				{file},
				ids.front().getLine(), ids.front().getCharPositionInLine(), ids.front().getValue().size(),
				result.error_message.value(),
			});
		}
		return result;
	}

	if constexpr (provide_diagnostics) {
		result.object->add_reference_position({file, ids.front().getLine(), ids.front().getCharPositionInLine()});
	}

	// Special-case: if @super, create a "faux" object as a copy of the 'this' pointer,
	// but with the type of the parent class.
	if (super) {
		const auto this_class = result.object->get_type().lock();
		bpp_assert(this_class != nullptr, "Object has no type in resolve_entity()");
		const auto parent_class = this_class->get_parent_class();
		if (parent_class == nullptr) {
			result.object = nullptr;
			result.error_message = this_class->get_name() + " has no parent class to reference with @super";
			if constexpr (provide_diagnostics) {
				result.error_token = ids.front();
				program->add_diagnostic({
					{file},
					ids.front().getLine(), ids.front().getCharPositionInLine(), ids.front().getValue().size(),
					result.error_message.value(),
				});
			}
			return result;
		}
		auto faux_object = std::make_shared<Object>(*result.object);
		faux_object->set_name("super");
		faux_object->set_type(parent_class);
		result.object = std::move(faux_object);
	}

	auto current_class = result.object->get_type().lock();
	bpp_assert(context != nullptr, "Object has no type in resolve_entity()");
	ids.pop_front();

	DataMemberAccess::ReferenceChain chain(result.object);

	while (!ids.empty()) {
		const auto current_token = ids.front();
		const std::string id(current_token);
		ids.pop_front();

		if (id.contains("__")) {
			result.object = nullptr;
			result.error_message = "Invalid identifier: " + id + " (Bash++ identifiers cannot contain double underscores)";
			if constexpr (provide_diagnostics) {
				result.error_token = current_token;
				program->add_diagnostic({
					{file},
					current_token.getLine(), current_token.getCharPositionInLine(), current_token.getValue().size(),
					result.error_message.value(),
				});
			}
			return result;
		}

		auto data_member = current_class->get_datamember(id, context);
		auto method = current_class->get_method(id, context);

		if (data_member) {
			chain.append(data_member.value());
			current_class = data_member.value()->get_type().lock();
			if (current_class == nullptr && !ids.empty()) {
				result.object = nullptr;
				result.error_message = "Unexpected identifier after primitive object reference";
				if constexpr (provide_diagnostics) {
					result.error_token = current_token;
					program->add_diagnostic({
						{file},
						current_token.getLine(), current_token.getCharPositionInLine(), current_token.getValue().size(),
						result.error_message.value(),
					});
				}
				return result;
			}
		} else if (method) {
			result.method_call = std::make_shared<MethodCall>(std::move(chain), method.value());
			if (!ids.empty()) {
				result.method_call = nullptr;
				result.object = nullptr;
				result.error_message = "Unexpected identifier after method reference";
				if constexpr (provide_diagnostics) {
					result.error_token = current_token;
					program->add_diagnostic({
						{file},
						current_token.getLine(), current_token.getCharPositionInLine(), current_token.getValue().size(),
						result.error_message.value(),
					});
				}
				return result;
			}
			return result;
		} else if (data_member.error() == LookupError::INACCESSIBLE || method.error() == LookupError::INACCESSIBLE) {
			result.object = nullptr;
			result.error_message = id + " is inaccessible in this context";
			if constexpr (provide_diagnostics) {
				result.error_token = current_token;
				program->add_diagnostic({
					{file},
					current_token.getLine(), current_token.getCharPositionInLine(), current_token.getValue().size(),
					result.error_message.value(),
				});
			}
			return result;
		} else {
			auto latest_entity = result.object;
			if (!chain.chain.empty()) latest_entity = chain.chain.back().lock();
			result.object = nullptr;
			result.error_message = latest_entity->get_name() + " has no member named " + id;
			if constexpr (provide_diagnostics) {
				result.error_token = current_token;
				program->add_diagnostic({
					{file},
					current_token.getLine(), current_token.getCharPositionInLine(), current_token.getValue().size(),
					result.error_message.value(),
				});
			}
			return result;
		}
	}

	// If we're here, it's a data member access, not a method call
	// (Method call would've returned earlier)
	result.data_member_access = std::make_shared<DataMemberAccess>(std::move(chain));
	result.object = nullptr;
	return result;
}

} // namespace bpp::IR
