---
layout: custom
title: Entities
---
# NAME

Entities - Structural Units of Program Identity and Accessibility

# DESCRIPTION

The "entity" is the fundamental unit of program representation in Bash++. Every language construct in a Bash++ program is represented as an entity with its own identity and well-defined position in the program's hierarchical structure.

All entities form a tree rooted at the program entity. Each entity may contain child entities, and each entity (except the root) has exactly one parent entity. Parent-child relationships reflect structural containment in the source program, and sibling entities are ordered according to their order of appearance in the source program.

All Bash++ language constructs are represented as entities, including but not limited to:

 - The program itself
 - Classes
 - Methods
 - Objects
 - Statements
 - Expressions
 - Curly-braced blocks
 - Control flow constructs (if, while, for, etc.)
 - Literals (strings, numbers, etc.)

For those familiar with compiler design: entities can be thought of as "AST nodes with identity."

## ACCESSIBILITY

Unlike traditional symbol-based scoping models, Bash++ defines accessibility in terms of structural relationships between entities.

An entity A is accessible from entity B if and only if there exists a path from B to A that never moves downward or forward in source order. Informally, an entity is accessible if it can be reached by moving only *left* or *up* in the program's entity tree.

Consider the following structure:

<div class="highlight"><pre class="highlight"><code>
{%- include code/snippets/manual-entities-1.html -%}
</code></pre></div>

From within `methodB`, the following accessibility rules apply:

 - `dataMember` *is accessible* -- reaching it only requires moving *left* (it's an earlier sibling)
 - `methodA` *is accessible* -- reaching it only requires moving *left* (it's an earlier sibling)
 - `globalObject` *is accessible* -- reaching it only requires moving *up* to the program entity
 - `localObject` is *not accessible* -- reaching it requires moving *left* to `methodA`, then *down* into its children, which is not allowed
 - `methodC` is *not accessible* -- reaching it requires moving *right*, which is not allowed

User-defined visibility modifiers (`@public`, `@private`, `@protected`) impose additional constraints on accessibility, but do not override these fundamental structural rules.

## ENTITIES AND CODE ENTITIES

A subset of entities are *code entities* -- entities capable of containing executable instructions. Some examples of code entities are:

 - Methods
 - Statements
 - Control flow constructs
 - The program itself

Classes and objects are *not* code entities and cannot directly contain executable instructions. Attempting to place executable code directly inside a non-code entity results in a compilation error. For example:

<div class="highlight"><pre class="highlight"><code>
{%- include code/snippets/manual-entities-2.html -%}
</code></pre></div>

The compiler validates that executable logic appears only within appropriate contexts by checking whether the containing entity is a code entity.

## INCLUDED FILES

When a file is included using the `@include` or `@include_once` directives, the included file's entities become part of the including file's entity tree at the point of inclusion. This means that accessibility rules apply seamlessly across included files. In other words, includes do not affect entity accessibility, and included files are treated as if their contents were written directly in place of the include directive.

# SEE ALSO

 - [bpp-classes(3)](classes.md) for more information on classes
 - [bpp-objects(3)](objects.md) for more information on objects
 - [bpp-methods(3)](methods.md) for more information on methods
