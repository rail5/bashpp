/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <IR/bpp.h>
#include <IR/entities/Entity.h>
#include <IR/entities/NamedEntity.h>
#include <IR/entities/Method.h>
#include <IR/entities/DataMember.h>
#include <IR/entities/MethodParameter.h>

#include <error/InternalError.h>

#include <expected>

namespace bpp::IR {

template <typename T>
concept ClassMember = std::is_same_v<T, Method> || std::is_same_v<T, DataMember>;

class Class : public Entity, public NamedEntity {
	private:
		const Class* parent_class = nullptr;

		mutable std::unique_ptr<ThisPtr> this_ptr;
		mutable std::unique_ptr<ThisPtr> super_ptr;
		mutable bool special_pointers_initialized = false;

		/**
		 * @brief Initializes the class's special "@this" and "@super" pointers.
		 *
		 * Although the *bits* of these pointers are not const,
		 * the pointers themselves are logically const,
		 * in the sense that they never change from the perspective of the public API
		 * from the first request for them to the end of the class's lifetime.
		 */
		void initSpecialPointers() const;

		std::vector<std::unique_ptr<Method>> methods;
		std::vector<std::unique_ptr<DataMember>> datamembers;

		/**
		 * A non-owning pointer to a method that has been preregistered with this class.
		 * When a method's definition is being processed, it is preregistered with the class so that it can be referenced before its definition is complete,
		 * i.e., for recursive method calls.
		 * Once the method's definition is complete, it is added to the class and removed from this preregistered pointer.
		 *
		 * This is a single pointer because we are guaranteed to only ever process ONE method definition at a time
		 * (Classes/methods cannot be nested in Bash++)
		 */
		Method* preregistered_method = nullptr;

		template <ClassMember T>
		std::expected<T*, LookupError> getMember(const std::string& name, const Entity* context) const;
	public:
		Class() = delete;
		explicit Class(const std::string& name) { setName(name); }

		const Class* getContainingClass() const override { return this; }

		/**
		 * @brief Add a method to this class, and return the actual method object that was added
		 * (which may be different from the one passed in, if it was overridden).
		 *
		 * If this method shares a name with an existing method, we will attempt to override the existing method.
		 * If the existing method is not overridable, this will fail and return an error.
		 *
		 * If the method name conflicts with the name of a data member, this will fail and return an error.
		 * 
		 * @param method The method to add
		 */
		[[ nodiscard ]] std::expected<void, NameConflictError> addMethod(std::unique_ptr<Method> method);

		/**
		 * @brief Preregister a method with this class.
		 *
		 * This is used to allow methods to be referenced before they are fully defined, such as in the case of recursive method calls.
		 * Once the method is fully defined, it will be added to the class and removed from this list.
		 *
		 * @param method The method to preregister
		 */
		void preregisterMethod(Method* method);

		/**
		 * @brief Cancel the preregistration of a method with this class.
		 *
		 * If we fail to process the method's definition, this is called to unset the 'preregistered_method' pointer,
		 * so that the method is no longer considered to be preregistered with this class.
		 */
		void unPreregisterMethod() { preregistered_method = nullptr; }

		/**
		 * @brief Add a data member to this class
		 *
		 * If the data member name conflicts with the name of an existing data member or method, this will fail and return an error.
		 * 
		 * @param datamember The data member to add
		 */
		[[ nodiscard ]] std::expected<void, NameConflictError> addDatamember(std::unique_ptr<DataMember> datamember);

		/**
		 * @brief Get a method by name
		 *
		 * This returns a method by name, taking into account the visibility restrictions of the method
		 *
		 * If the method is public, it is always returned
		 *
		 * If the method is private, it can only be returned if the context is the same as the owning class
		 *
		 * If the method is protected, it can only be returned if the context is the same as the owning class, or is a class derived from the owning class
		 * 
		 * @param name The name of the method to get
		 * @param context The context from which the method is being requested
		 * @return std::expected<Method*, LookupError> The method, or a LookupError if it doesn't exist or is inaccessible
		 */
		std::expected<Method*, LookupError> getMethod(const std::string& name, const Entity* context) const;

		/**
		 * @brief Get a data member by name
		 *
		 * This returns a data member by name, taking into account the visibility restrictions of the data member
		 *
		 * If the data member is public, it is always returned
		 *
		 * If the data member is private, it can only be returned if the context is the same as the owning class
		 *
		 * If the data member is protected, it can only be returned if the context is the same as the owning class, or is a class derived from the owning class
		 * 
		 * @param name The name of the data member to get
		 * @param context The context from which the data member is being requested
		 * @return std::expected<DataMember*, LookupError> The data member, or a LookupError if it doesn't exist or is inaccessible
		 */
		std::expected<DataMember*, LookupError> getDatamember(const std::string& name, const Entity* context) const;

		/**
		 * @brief Get a method by name without checking the context against visibility rules
		 *
		 * This function is UNSAFE and should only be used when the context is known to be correct or the consequences of an incorrect context are acceptable.
		 * 
		 * @param name The name of the method to get
		 * @return Method* The method, or nullptr if not found
		 */
		Method* getMethod_UNSAFE(const std::string& name) const;

		/**
		 * @brief Get a data member by name without checking the context against visibility rules
		 *
		 * This function is UNSAFE and should only be used when the context is known to be correct or the consequences of an incorrect context are acceptable.
		 * 
		 * @param name The name of the data member to get
		 * @return DataMember* The data member, or nullptr if not found
		 */
		DataMember* getDatamember_UNSAFE(const std::string& name) const;

		const std::vector<std::unique_ptr<Method>>& getAllMethods() const { return methods; }
		const std::vector<std::unique_ptr<DataMember>>& getAllDatamembers() const { return datamembers; }

		bool containsNonprimitiveDatamembers() const;

		using Entity::inherit;

		/**
		 * @brief Inherit methods and data members from a parent class.
		 *
		 * This function copies all methods and data members from the parent class into this class, except for toPrimitive and system methods.
		 *
		 * If a method or data member is private, it will be marked as inaccessible in the child class.
		 * 
		 * @param parent The parent class from which to inherit methods and data members.
		 */
		void inherit(const Class* parent);

		// Guide for the C++ compiler:
		// This is a convenience overload that allows the caller to pass a non-const Class pointer,
		// and have it automatically converted to a const Class pointer for the inherit() function above.
		// Without this, the call to inherit() is ambiguous,
		// because the compiler can't decide whether to convert it to a const Class pointer or a const Entity pointer.
		void inherit(Class* parent) { inherit(const_cast<const Class*>(parent)); }

		/**
		 * @brief Get the parent class of this class, if it has one.
		 * 
		 * @return const Class* The parent class, or nullptr if this class has no parent.
		 */
		const Class* getParentClass() const { return parent_class; }

		/**
		 * @brief Check if this class is derived from some other particular class
		 * 
		 * @param other The possible ancestor of this class
		 * @return true If `other` is an ancestor (or immediate parent) of this class
		 * @return false Otherwise
		 */
		bool isDerivedFrom(const Class* other) const;

		/**
		 * @brief Get the "@this" pointer for this class,
		 * which is a special method parameter that points to the current instance of the class.
		 * 
		 * @return ThisPtr* The "this" pointer for this class
		 */
		ThisPtr* getThisPtr() const {
			initSpecialPointers();
			return this_ptr.get();
		}

		/**
		 * @brief Get the "@super" pointer for this class,
		 * which is a special method parameter that points to the current instance of the class,
		 * but considers it to be an instance of the parent class.
		 * 
		 * @return ThisPtr* The "super" pointer for this class, or nullptr if this class has no parent
		 */
		ThisPtr* getSuperPtr() const {
			initSpecialPointers();
			return super_ptr.get();
		}

		bpp::CodeGen::CodeSegment generateCode(bpp::CodeGen::CodeGenState* state) const override;

		PRETTYPRINT_OVERRIDE();
};

} // namespace bpp::IR
