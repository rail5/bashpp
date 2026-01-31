---
layout: custom
title: Methods
brief: "@method"
description: "Define a method for a class"
---
# NAME

@method - Define a method for a class

# SYNOPSIS

```bash
[@virtual] {@private | @protected | @public} @method {METHOD-NAME} [{ARGUMENTS}] {
	[COMMANDS]
}
```

# DESCRIPTION

The `@method` directive is used to define a method for a class. A method is a function that is associated with a class and is owned by particular instances of that class. It can be declared as `@private`, `@protected`, or `@public`. Methods can also be declared as `@virtual`, which means that they can be overridden in derived classes and will be dynamically dispatched.

The method name must be a valid Bash++ identifier. This means that it can only contain letters, numbers, and underscores, cannot start with a number, cannot be a reserved word, and cannot contain two consecutive underscores. The method name is case-sensitive. The method name must be unique within the class. If the method name is not unique, a compile-time error will be generated.

A method can be declared to expect arguments. The arguments are specified after the method name as a space-separated list containing primitives and/or pointers to objects. For example:

<div class="highlight"><pre class="highlight"><code>
{%- include code/snippets/manual-method-example-0.html -%}
</code></pre></div>

All arguments to methods are optional. Although a method can be declared to expect a specific number of arguments, it can still be called with any number of arguments, including none at all.

If no arguments are specified, any arguments which are passed can still be accessed using `$1`, `$2`, etc, as in any other Bash function.

Methods cannot accept non-primitive arguments. They can, however, accept *pointers* to objects as arguments. For example:

<div class="highlight"><pre class="highlight"><code>
{%- include code/snippets/manual-method-example-1.html -%}
</code></pre></div>

Declaring a pointer as an argument will run an implicit `@dynamic_cast` on the pointer to the expected type at the start of the method. This means that if the pointer is invalid, it will be set to `@nullptr` within the method.

Primitives can be declared as arguments as follows:

<div class="highlight"><pre class="highlight"><code>
{%- include code/snippets/manual-method-example-2.html -%}
</code></pre></div>

The arguments can be accessed using `$1`, `$2`, etc, as in any other Bash function.

Declaring a method to be `@virtual` means that it can be overridden in a derived class and will be dynamically dispatched. This means that if a derived class has a method with the same name, the derived class's method will be called instead of the base class's method. The correct method to call is determined at runtime.

# EXAMPLE

<div class="highlight"><pre class="highlight"><code>
{%- include code/snippets/manual-method-example-3.html -%}
</code></pre></div>

# NOTES

Methods can only be defined within a class. They cannot be defined outside of a class.

Methods can be called using the `@object.method` syntax. For example, if you have an object called `myObject` and a method called `myMethod`, you can call the method using `@myObject.myMethod`.

You can pass arguments to methods using the same syntax as for ordinary commands. For example, to pass the arguments `arg1` and `arg2` to the method `myMethod`, you can call the method using `@myObject.myMethod arg1 arg2`.

Calling an object's method in a place where a primitive is expected will run the method in a supershell and substitute its output. For example:

<div class="highlight"><pre class="highlight"><code>
{%- include code/snippets/manual-method-example-4.html -%}
</code></pre></div>

Will run the method in a supershell, and then pass its output to the `echo` command.

This is equivalent to:

<div class="highlight"><pre class="highlight"><code>
{%- include code/snippets/manual-method-example-5.html -%}
</code></pre></div>

Referencing a non-primitive object directly in a place where a primitive is expected will run the `toPrimitive` method of the object. This means that the following two lines are equivalent:

<div class="highlight"><pre class="highlight"><code>
{%- include code/snippets/manual-method-example-6.html -%}
</code></pre></div>

The toPrimitive method however will *not* be called if the reference is a **pointer** to an object, because pointers are already primitives. In that case, the pointer will be passed directly to the command.

# SEE ALSO

 - [bpp-classes(3)](classes.md) for more information on classes
 - [bpp-toprimitive(3)](toprimitive.md) for more information on the `toPrimitive` method
 - [bpp-supershell(3)](supershell.md) for more information on supershells
 - [bpp-dynamic-cast(3)](dynamic-cast.md) for more information on dynamic casting
 - [bpp-pointers(3)](pointers.md) for more information on pointers
