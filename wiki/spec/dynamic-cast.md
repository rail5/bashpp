---
layout: custom
---
# NAME

@dynamic_cast - Safely cast an object to a different type at runtime

# SYNOPSIS

```bash
@dynamic_cast<CLASS-NAME[*]> {INPUT}
```

# DESCRIPTION

The `@dynamic_cast` directive is used to safely cast an object to a different type at runtime.

The **output** of the `@dynamic_cast` directive will be either:

 - **An exact copy of INPUT** if the input is a valid pointer to an object which can be safely cast to the specified type.
 - `@nullptr` otherwise.

# EXAMPLE

<div class="highlight"><pre class="highlight"><code>
{%- include code/snippets/manual-dynamic-cast-example-1.html -%}
</code></pre></div>

# NOTES

The input may be any rvalue at all, including a pointer to an object, a call to a method, a simple string, a supershell/subshell, etc. Of course, in most cases, the input should be a pointer to an object.

The `@dynamic_cast` directive is most useful when the type of the object is not known at compile time, or when the object may be of a different type than expected.

You can safely verify that a cast was successful by checking if the result is `@nullptr`. If it is not, you can safely use the result as a pointer to the specified type.

<div class="highlight"><pre class="highlight"><code>
{%- include code/snippets/dynamic-casting-example.html -%}
</code></pre></div>

# SEE ALSO

 - [bpp-pointers(3)](pointers.md) for more information on pointers
 - [bpp-classes(3)](classes.md) for more information on classes
