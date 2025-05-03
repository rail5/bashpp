---
layout: custom
---
# NAME

Pointer - A pointer to an object

# SYNOPSIS

```bash
@{CLASS-NAME}* {POINTER-NAME}[={VALUE}]
```

# DESCRIPTION

A pointer is a variable that holds the address of an object. It is used to reference an object without creating a copy of it. Pointers are useful for passing objects to functions, returning objects from functions, and for dynamic memory allocation.

The syntax for declaring a pointer is similar to that of a variable, but with an asterisk (*) before the variable name. For example, to declare a pointer to an object of class `MyClass`, you would use the following syntax:

<div class="highlight"><pre class="highlight"><code>
{%- include code/snippets/manual-pointers-example-1.html -%}
</code></pre></div>

Any primitive value can be assigned to a pointer, however, it would generally be more useful to assign a pointer to an object. The following is valid syntax, but absolutely useless:

<div class="highlight"><pre class="highlight"><code>
{%- include code/snippets/manual-pointers-example-2.html -%}
</code></pre></div>

You can take the address of an object using the `&` operator. For example, to get the address of an object `myObject`, you would use the following syntax:

<div class="highlight"><pre class="highlight"><code>
{%- include code/snippets/manual-pointers-example-3.html -%}
</code></pre></div>

Accessing data members and methods of an object through a pointer is done using exactly the same syntax as with normal objects. There is no need to use the `->` operator as in C/C++.

Pointers are implicitly dereferenced as needed in Bash++. However, if you would like to explicitly dereference a pointer, you can use the `*` operator. For example, to dereference a pointer `myPointer`, you would use the following syntax:

<div class="highlight"><pre class="highlight"><code>
{%- include code/snippets/manual-pointers-example-4.html -%}
</code></pre></div>

This will output the result of the `toPrimitive` method of the object pointed to by `myPointer`. If the pointer had not been dereferenced, it would have output the address of the object instead.

# EXAMPLE

<div class="highlight"><pre class="highlight"><code>
{%- include code/snippets/manual-pointers-example-5.html -%}
</code></pre></div>

# NOTES

Pointers are not automatically deleted when they go out of scope. Unlike objects which are instantiated by `@ClassName objectName`, pointers must be explicitly deleted using the `@delete` directive.

<div class="highlight"><pre class="highlight"><code>
{%- include code/snippets/manual-pointers-example-6.html -%}
</code></pre></div>

# SEE ALSO

 - [bpp-new(3)](new.md) for creating a new instance of a class
 - [bpp-delete(3)](delete.md) for deleting an object
 - [bpp-classes(3)](classes.md) for more information on classes
 - [bpp-dynamic-cast(3)](dynamic-cast.md) for more information on dynamic casting
