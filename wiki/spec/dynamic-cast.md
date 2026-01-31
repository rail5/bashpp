---
layout: custom
title: Dynamic Cast
brief: "@dynamic_cast"
description: "Safely cast an object to a different type at runtime"
---
# NAME

@dynamic_cast - Safely cast an object to a different type at runtime

# SYNOPSIS

```bash
@dynamic_cast<CLASS-NAME[*]> {INPUT}
@dynamic_cast<$shell_variable> {INPUT}
@dynamic_cast<@object.reference> {INPUT}
```

# DESCRIPTION

The `@dynamic_cast` directive is used to safely cast an object to a different type at runtime. At runtime, the directive itself will be replaced with the *output* described below.

The **output** of the `@dynamic_cast` directive will be either:

 - **An exact copy of INPUT** if the input is a valid pointer to an object which can be safely cast to the specified type.
 - `@nullptr` otherwise.

# TARGETS

The *target* of the `@dynamic_cast` directive is the type to which you want to cast the object. The target is given in the `<...>` bracket pair and can be one of the following:

 - A class name, optionally followed by an asterisk (`*`)
 - A shell variable, which contains the name of the target class
 - An object reference, whose output will be the name of the target class

If a class name is given directly, the compiler will emit a warning if the class is not defined in the current context.

If a shell variable or object reference is given, the compiler will expect to resolve the class name at runtime.

If an object reference is given, it can refer to either a data member or a method. I.e., all kinds of object references are acceptable here. If the reference refers to a method, that method will be implicitly executed in a supershell, and its output will be treated as the target class name.

# EXAMPLE

The safest way to use `@dynamic_cast` is to specify the target class name directly:

<div class="highlight"><pre class="highlight"><code>
{%- include code/snippets/manual-dynamic-cast-example-1.html -%}
</code></pre></div>

However, we can also give a shell variable or an object reference as the target class. In this case, no warnings will be emitted, and the class name will be resolved at runtime:

<div class="highlight"><pre class="highlight"><code>
{%- include code/snippets/manual-dynamic-cast-example-2.html -%}
</code></pre></div>

# NOTES

The *input* to the `@dynamic_cast` directive is that which is being casted. It is typically a pointer to an object, but it may be any rvalue at all, including a call to a method, a simple string, a supershell/subshell, etc. Of course, in most cases, the input should be a pointer to an object.

The `@dynamic_cast` directive is most useful when the type of the object is not known at compile time, or when the object may be of a different type than expected.

You can safely verify that a cast was successful by checking if the result is `@nullptr`. If it is not, you can safely use the result as a pointer to the specified type.

<div class="highlight"><pre class="highlight"><code>
{%- include code/snippets/dynamic-casting-example.html -%}
</code></pre></div>

# SEE ALSO

 - [bpp-pointers(3)](pointers.md) for more information on pointers
 - [bpp-classes(3)](classes.md) for more information on classes
