---
layout: custom
title: Super
brief: "@super"
description: "Change context to a parent class"
---
# NAME

@super - Change context to a parent class

# SYNOPSIS

```bash
@super.methodName [args...]
```

# DESCRIPTION

The `@super` keyword can be used to call a parent class's version of an overridden method from within a child class. This is used when extending parent method functionality while preserving the original behavior.

The actual behavior of `@super` is **only** to change the context to the parent class of the current object before continuing member resolution. Using it is functionally equivalent to using the `@this` pointer, except that it considers the current object as an instance of its *immediate parent class*.

Key behaviors:

 - **Static Resolution**: Any referenced method is resolved at compile-time, meaning it will always call the parent class's method, even if the child class overrides it.
 
 - **Context Change**: It temporarily changes the context to the parent class of the current object before continuing member resolution.

# EXAMPLE

<div class="highlight"><pre class="highlight"><code>
{%- include code/snippets/manual-super-example-1.html -%}
</code></pre></div>

# NOTES

## ACCESS RESTRICTIONS

Since `@super` merely changes the context to the parent class, it can be used in any context where `@this` would be used. This includes method calls, data member accesses, etc. However, `@super` does not bypass access restrictions: if the parent class's method or data member is private, it cannot be accessed by a child class, even with `@super`.

Because `@super` changes context to the parent class before continuing resolution, it is not possible to use `@super` to access a method or data member that is only defined in the child class.

## TOPRIMITIVE

Calling `@super.toPrimitive` will result in the parent class's version of the `toPrimitive` method being called, even if the child class has overridden it.

This is true even when the call to `toPrimitive` is *implicit*, i.e., when a non-primitive object is referenced where a primitive is expected -- in this case, by dereferencing with `*@super` in such a context.

For example, consider `echo *@super`. Firstly, dereferencing the pointer tells the compiler to treat the pointer as it would treat a non-primitive object. Secondly, referencing a non-primitive object in that context implicitly calls `toPrimitive`. Finally, because `@super` changes the context to the parent class, the parent class's version of `toPrimitive` will be called.

## LIMITATIONS

Using `@super` only accesses the **immediate parent class**. It cannot traverse further up the class hierarchy.

# SEE ALSO

- [bpp-objects(3)](objects.md) for more information on objects
- [bpp-methods(3)](methods.md) for more information on methods
- [bpp-classes(3)](classes.md) for more information on classes
- [bpp-pointers(3)](pointers.md) for more information on pointers
