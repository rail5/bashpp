---
layout: custom
title: Scope
brief: "Scope"
description: "Scope and Accessibility in Bash++"
---
# NAME

Scope - Scope and Accessibility in Bash++

# DESCRIPTION

In Bash++, scope is defined in terms of the structural relationships between entities in the program. An entity A is accessible from entity B if and only if there exists a path from B to A that never moves downward or forward in source order. Informally, an entity is accessible if it can be reached by moving only *left* or *up* in the program's entity tree. See [bpp-entities(3)](entities.md) for more information on entities and their relationships.

# OBJECT LIFETIME MANAGEMENT

Bash++ provides deterministic object lifetime management through its scoping rules. An object's destructor is automatically called if the object goes out of scope *before* the program exits, and the object was not created via `@new`. This means that if an object is created within a function, method, or block, its destructor will be called when that function, method, or block finishes executing.

However, if an object is created at global scope, its destructor will not be called at program termination. This design choice is made to avoid non-deterministic behavior in forked processes. If you need to ensure that destructors are called for global-scope objects, you should call them explicitly using `@delete`.

The management of the lifetimes of objects created via `@new` is the responsibility of the programmer. Destructors for these objects are not called automatically, and you must explicitly call `@delete` to trigger the destructor when you are done with the object.

## SCOPE'S IMPACT ON OBJECT LIFETIMES

If a code entity carries local scope, this means that any objects created within that code entity will have their destructors automatically called when the code entity finishes executing, as long as the objects were not created via `@new`.

Supershells, notably, do *not* carry local scope. Any objects created within a supershell will not have their destructors automatically called when the supershell finishes executing, and will continue to be accessible from the parent scope after the supershell finishes.

The supershell is *not* an isolated scope -- the parent scope of the supershell is responsible for managing the lifetimes of objects created within the supershell. For example, if a function contains a supershell, any objects created within that supershell will be destroyed when the function finishes. On the other hand, if a supershell is created at global scope, any objects created within that supershell will not be automatically destroyed, since the global scope does not have its destructors called at program termination.

Below is the list of code entities which are considered to carry local scope:

 - Methods (`@method ...`, as well as constructors and destructors)
 - Plain functions (`function_name() { ... }` / `function function_name { ... }`)
 - Curly-braced blocks (`{ ... }`)
 - Subshells (`( ... )`, `$( ... )`, or backtick-enclosed command substitutions)
 - Process substitutions (`<( ... )` or `>( ... )`)
 - Control flow constructs (`if` branches, `while` loops, `until` loops, `for` loops, `case` branches, and `select` branches)

And the following code entities are *not* considered to carry local scope:

 - Supershells (`@( ... )`)
 - The program itself (i.e., global scope)

The rule, simply put, is: an object's destructor will automatically be called when and if it goes out of scope, as long as the object was not created via `@new`.

# SEE ALSO

 - [bpp-entities(3)](entities.md) for more information on entities and their relationships
 - [bpp-objects(3)](objects.md) for more information on object lifetime management and destructors
 - [bpp-delete(3)](delete.md) for more information on the `@delete` directive and manual destructor calls
 - [bpp-new(3)](new.md) for more information on the `@new` directive and dynamic object creation
 - [bpp-classes(3)](classes.md) for more information on classes and system methods such as `__new` and `__delete`
 - [bpp-supershell(3)](supershell.md) for more information on the supershell construct and its scoping rules
