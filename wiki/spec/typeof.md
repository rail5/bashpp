---
layout: custom
title: Typeof
brief: "@typeof"
description: "Determine the type of an object at runtime"
---
# NAME

@typeof - Determine the type of an object at runtime

# SYNOPSIS

```bash
@typeof {INPUT}
```

# DESCRIPTION

The `@typeof` operator is used to determine the type of a pointer at runtime. The directive is expanded to the *result* described below.

The **result** of the `@typeof` directive will be either:

 - **The class name** of the object if the input is a valid pointer to an object.
 - An empty string otherwise.

# EXAMPLE

<div class="highlight"><pre class="highlight"><code>
{%- include code/snippets/manual-typeof-example-1.html -%}
</code></pre></div>

# NOTES

## EXPANSION

The `@typeof` directive expands to the result described above. This result is then interpreted according to the shell context in which the directive is used. For example, if the directive is used in a context where a command is expected, the result will be treated as a command. If it is used in a context where an argument is expected, the result will be treated as an argument.

<div class="highlight"><pre class="highlight"><code>
{%- include code/snippets/manual-typeof-example-2.html -%}
</code></pre></div>

## INPUT

The *input* to the `@typeof` directive is that which is being inspected. The input can be any rvalue at all, but *should* be a pointer to an object.

An easy mistake to make, for example, would be to pass an object directly instead of its address -- this would implicitly call the object's `toPrimitive` method, and attempt to find the "type of" that method's output.
