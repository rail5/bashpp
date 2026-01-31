---
layout: custom
title: Pointers
brief: "Pointer"
description: "A pointer to an object"
---
# NAME

Pointer - A pointer to an object

# SYNOPSIS

```bash
@{CLASS-NAME}* {POINTER-NAME}[={VALUE}]
```

# DESCRIPTION

A pointer is a variable that holds the address of an object. It is used to reference an object without creating a copy of it.

The syntax for declaring a pointer is similar to that used to declare an object, with the addition of an asterisk (`*`) after the class name. For example, to declare a pointer to an object of class `MyClass`, you would use the following syntax:

<div class="highlight"><pre class="highlight"><code>
{%- include code/snippets/manual-pointers-example-1.html -%}
</code></pre></div>

Any primitive value can be assigned to a pointer, however, it would generally be most useful to assign the address of an object. The following is valid syntax, but absolutely useless:

<div class="highlight"><pre class="highlight"><code>
{%- include code/snippets/manual-pointers-example-2.html -%}
</code></pre></div>

You can take the address of an object using the `&` operator. For example, to get the address of an object `myObject`, you would use the following syntax:

<div class="highlight"><pre class="highlight"><code>
{%- include code/snippets/manual-pointers-example-3.html -%}
</code></pre></div>

Accessing data members and methods of an object through a pointer is done using exactly the same syntax as with normal objects. There is no need to use a different operator, as in many other programming languages.

Pointers are implicitly dereferenced as needed in Bash++. However, if you would like to explicitly dereference a pointer, you can use the `*` operator. For example, to dereference a pointer `myPointer`, you would use the following syntax:

<div class="highlight"><pre class="highlight"><code>
{%- include code/snippets/manual-pointers-example-4.html -%}
</code></pre></div>

This will output the result of the `toPrimitive` method of the object pointed to by `myPointer`. If the pointer had not been dereferenced, it would have output the address of the object instead.

Semantically, the use of the dereference operator means that the compiler should treat the pointer in the exact same way that it would treat an object itself. I.e., that the compiler should carry on as though the object had been directly referenced, instead of the pointer. This means that a dereferenced pointer is acceptable in every context where an object is acceptable. For example:

<div class="highlight"><pre class="highlight"><code>
{%- include code/snippets/manual-pointers-example-5.html -%}
</code></pre></div>

# EXAMPLE

<div class="highlight"><pre class="highlight"><code>
{%- include code/snippets/manual-pointers-example-6.html -%}
</code></pre></div>

# NOTES

If an object was instantiated using `@new`, that object's lifetime will not be automatically managed by the compiler. In this case, it is the programmer's responsibility to ensure that the object is deleted using `@delete` when it is no longer needed.

<div class="highlight"><pre class="highlight"><code>
{%- include code/snippets/manual-pointers-example-7.html -%}
</code></pre></div>

# SEE ALSO

 - [bpp-new(3)](new.md) for creating a new instance of a class
 - [bpp-delete(3)](delete.md) for deleting an object
 - [bpp-classes(3)](classes.md) for more information on classes
 - [bpp-dynamic-cast(3)](dynamic-cast.md) for more information on dynamic casting
