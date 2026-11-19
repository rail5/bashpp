/*
 * Copyright (C) 2026 Andrew S. Rightenburg
 * Bash++: Bash with classes
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <string>
#include <vector>
#include <variant>

#include <IR/bpp.h>
#include <IR/OwnedEntityList.h>
#include <IR/entities/Entity.h>
#include <IR/entities/Object.h>

namespace bpp::IR {

using RawCode = std::string;
using RawCodeOrOwnedEntity = std::variant<RawCode, std::unique_ptr<Entity>>;
using RawCodeOrUnownedEntity = std::variant<RawCode, const Entity*>;

/**
 * @brief Any entity which can contain executable code.
 *
 * This includes the Program, all Methods, etc.
 * 
 * Notably, classes and objects cannot contain executable code, and are not therefore code entities.
 */
class CodeEntity : public Entity {
	protected:
		/// Objects owned by this entity (i.e., this entity is responsible for their lifetime)
		OwnedEntityList<Object> local_objects;
		/// The children of this node in the entity tree
		std::vector<RawCodeOrOwnedEntity> children;
	public:
		const std::vector<RawCodeOrOwnedEntity>& getChildren() const { return children; }

		/**
		 * @brief Add raw code to the entity tree as a child of this code entity
		 * 
		 * @param child The raw code to add
		 */
		void add(const RawCode& child);

		/**
		 * @brief Add another entity to the entity tree as a child of this code entity
		 * 
		 * @param child The entity to add
		 */
		void add(std::unique_ptr<Entity> child);

		/**
		 * @brief Add an object to this code entity, making it responsible for the object's lifetime.
		 *
		 * If the object name conflicts with an existing object or class known to this code entity, this function will return false and not add the object.
		 *
		 * Note that this does not guarantee instantiation.
		 * The Listener (or other caller) is responsible for ensuring that the Method call necessary
		 * to instantiate the object is added to the entity tree at the appropriate location.
		 * 
		 * @param object The object to add
		 */
		bool addObject(std::unique_ptr<Object> object);
		const OwnedEntityList<Object>& getLocalObjects() const { return local_objects; }
		OwnedEntityList<Object> releaseLocalObjects();

		/**
		 * @brief Get an object by name, searching local objects first, then parent entities
		 * 
		 * @param name The name of the object to get
		 * @param max_visible_index The maximum visible index of the object to get (for scoping purposes)
		 * @return Object* The object, or nullptr if not found
		 */
		Object* getObject(const std::string& name, std::size_t max_visible_index = SIZE_MAX) const override;

		/**
		 * @brief Get a list of all objects known to this code entity, whether owned by this code entity or merely visible to it
		 * 
		 * @return std::vector<Object*> A vector of all objects known to this code entity
		 */
		std::vector<Object*> getAllKnownObjects() const override;
		std::size_t getNumberOfKnownObjects() const override;

		/**
		 * @brief Adopt all local objects from another CodeEntity into this one.
		 *
		 * This code entity becomes responsible for the lifetimes (& ownership) of the adopted objects.
		 * 
		 * @param other The other CodeEntity from which to adopt local objects
		 */
		void adoptObjectsOf(CodeEntity* other);

		bool managesObjectLifetimes() const;

		/**
		 * @brief Create a new scope frame for this code entity, which will be destroyed when the code entity is exited.
		 *
		 * A scope frame is only created if this code entity has local objects, or if `force` is true.
		 * If `isolate_scope` is true, the scope frame will be created in an isolated (local) scope-frame stack,
		 * separate to the global scope-frame stack used by the program. This totally eliminates the possibility
		 * of accidental interference between the scope frames of this code entity and those of enclosing code entities.
		 *
		 *
		 * Why would we want to "force" a scope frame to be created, even if there are no objects to manage?
		 * Unfortunately, this is a necessary accommodation of the Bash & POSIX shell specifications.
		 *
		 * Let's consider what has to happen when we break out of a loop. The POSIX spec says:
		 *
		 *  > break [n]: exit from for, while, or until loop
		 *  >
		 *  > If n is specified, the break utility shall exit from the nth enclosing for, while, or until loop
		 *  > [...]
		 *
		 * So, firstly, this means that (if we consider loops to be local scopes, which we do),
		 * then if `break n` is given, we must destroy the scope frames belonging to the last `n` enclosing loops.
		 * (E.g., we're going to break out of 3 nested loops, we have to destroy all the objects owned by those 3 loops.)
		 *
		 * Practically speaking, Bash caps `n` at the number of LEXICALLY enclosing loops.
		 * Suppose you have a function which runs a loop, and inside that loop, it calls another looping function:
		 *   foo() {
		 *     while ... ; do bar ; done
		 *   }
		 *   bar() {
		 *     while ... ; do if ... ; then break 2 ; fi ; done
		 *   }
		 *
		 * The `break 2` in `bar()` will only break out of the loop in `bar()`, not the loop in `foo()`,
		 * because although the loop in `foo()` is "enclosing" in the sense that it's on the call stack,
		 * it is not LEXICALLY enclosing, because the loop in `bar()` is not textually inside the loop in `foo()`.
		 *
		 * "OK, great," you might be thinking, "this means we can know everything we need to know at compile-time,
		 * We just check how many loops are lexically enclosing the `break`, and then check how many of those
		 * actually have scope frames to destroy, and then we can just destroy that many scope frames."
		 *
		 * Well ... unfortunately, this behavior of Bash could reasonably be considered a bug. And therefore we can't rely on it.
		 * Bash's manual says absolutely nothing about this kind of case,
		 * and here's the rest of the quote from the POSIX spec that I skipped over above:
		 *
		 * > If n is greater than the number of lexically enclosing loops and
		 * > there is a non-lexically enclosing loop in progress in the same
		 * > execution environment as the break or continue command, it is
		 * > unspecified whether that loop encloses the command.
		 *
		 * "It is unspecified." Well, shit.
		 * In other words, we're forced to assume that `break n` can break out of ANY enclosing loop, whether lexically or not.
		 * This means that it's impossible to know at compile-time how many scope frames to destroy,
		 * because we can't possibly know what the call stack will look like at runtime.
		 *
		 * The ONLY safe approach therefore is to force EVERY loop to have a scope frame,
		 * regardless of whether it has any objects to manage,
		 * and then at runtime simply destroy the last `n` loop frames.
		 *
		 * This kind of baggage is pretty annoying. These cases are the only ones where we need comments to be this long.
		 *
		 * @param state 
		 * @param isolate_scope Whether to create the scope frame in an isolated (local) scope-frame stack, separate to the global scope-frame stack used by the program
		 * @param force Whether to force the creation of a scope frame even if this code entity has no local objects
		 * @return bpp::CodeGen::CodeSegment The code segment which creates the scope frame
		 */
		bpp::CodeGen::CodeSegment createScopeFrame(bpp::CodeGen::CodeGenState* state, bool isolate_scope, bool force) const;

		/**
		 * @brief Destroy the scope frame for this code entity, if it has one.
		 * If this code entity did not create a scope frame, this function does nothing.
		 *
		 * @param state 
		 * @return bpp::CodeGen::CodeSegment The code segment which destroys the scope frame, or an empty code segment if this code entity did not create a scope frame
		 */
		bpp::CodeGen::CodeSegment destroyScopeFrame(bpp::CodeGen::CodeGenState* state) const;

		/**
		 * @brief Mark the scope frame for this code entity as belonging to a loop, if it has one.
		 * If this code entity did not create a scope frame, this function does nothing.
		 *
		 * @param state 
		 * @return bpp::CodeGen::CodeSegment The code segment which marks the scope frame as belonging to a loop, or an empty code segment if this code entity did not create a scope frame
		 */
		bpp::CodeGen::CodeSegment markLoopFrame(bpp::CodeGen::CodeGenState* state) const;

		/**
		 * @brief Unmark the scope frame for this code entity as belonging to a loop, if it has one.
		 * This needs to be done when exiting a loop.
		 * If this code entity did not create a scope frame, this function does nothing.
		 *
		 * @param state 
		 * @return bpp::CodeGen::CodeSegment The code segment which unmarks the scope frame as belonging to a loop, or an empty code segment if this code entity did not create a scope frame
		 */
		bpp::CodeGen::CodeSegment unmarkLoopFrame(bpp::CodeGen::CodeGenState* state) const;

		bpp::CodeGen::CodeSegment generateCode(bpp::CodeGen::CodeGenState* state) const override;

		PRETTYPRINT_OVERRIDE();
};

} // namespace bpp::IR
