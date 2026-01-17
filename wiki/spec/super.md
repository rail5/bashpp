---
layout: custom
title: Super
---
# NAME

@super - Change context to a parent class

# SYNOPSIS

```bash
@super.methodName [args...]
```

# DESCRIPTION

The `@super` keyword can be used to call a parent class's version of an overridden method from within a child class. This is used when extending parent method functionality while preserving the original behavior.

The actual behavior of `@super` is **only** to change the context to the parent class of the current object before descending the object hierarchy. This means that any method or property accessed via `@super` will be resolved in the context of the parent class.

Key behaviors:

 - **Static Resolution**: Any referenced method is resolved at compile-time, meaning it will always call the parent class's method, even if the child class overrides it.
 
 - **Context Change**: It temporarily changes the context to the parent class of the current object before descending the object hierarchy.

# EXAMPLE

<div class="highlight"><pre class="highlight"><code>
{%- include code/snippets/manual-super-example-1.html -%}
</code></pre></div>

# NOTES

Since `@super` merely changes the context to the parent class, it can be used in any context where `@this` would be used. This includes method calls, property accesses, etc.

Calling `@super.toPrimitive` or dereferencing via `*@super` will result in the parent class's `toPrimitive` method being called.

Using `@super` only accesses the **immediate parent class**. It cannot traverse further up the class hierarchy.

# SEE ALSO

- [bpp-objects(3)](objects.md) for more information on objects
- [bpp-methods(3)](methods.md) for more information on methods
- [bpp-classes(3)](classes.md) for more information on classes
- [bpp-pointers(3)](pointers.md) for more information on pointers
