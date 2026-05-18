---
layout: custom
title: New
brief: "@new"
description: "Create a new instance of a class"
---
# NAME

@new - Create a new instance of a class

# SYNOPSIS

```bash
@new {CLASS-NAME}
```

# DESCRIPTION

The `@new` operator is used to create a new instance of a class.

It initializes the object, and calls its constructor (if it exists). The directive expands to the address of the newly created object.

# EXAMPLE

<div class="highlight"><pre class="highlight"><code>
{%- include code/snippets/manual-new-example-1.html -%}
</code></pre></div>

# NOTES

## EXPANSION

The `@new` directive expands to the address of the newly created object. This address is then interpreted according to the shell context in which the directive is used. For example, if the directive is used in a context where a command is expected, the result will be treated as a command. If it is used in a context where an argument is expected, the result will be treated as an argument.

Most commonly, you'll want to assign the result of `@new` to a pointer.

<div class="highlight"><pre class="highlight"><code>
{%- include code/snippets/manual-new-example-2.html -%}
</code></pre></div>

## LIFETIME

Objects created with `@new` are not automatically deleted when they go out of scope. Unlike objects which are instantiated by `@ClassName objectName`, objects created with `@new` must be explicitly deleted using the `@delete` directive.

The object's constructor will be called *before* the pointer is returned. This means that the object will be fully initialized before anything can use it.

# SEE ALSO

- [bpp-delete(3)](delete.md) for deleting an object
- [bpp-classes(3)](classes.md) for more information on classes
