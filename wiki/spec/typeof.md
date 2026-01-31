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

The `@typeof` directive is used to determine the type of a pointer at runtime.

At runtime, the directive itself will be replaced with the *output* described below.

The *output* of the `@typeof` directive will be either:

 - **The class name** of the object if the input is a valid pointer to an object.
 - An empty string otherwise.

# EXAMPLE

<div class="highlight"><pre class="highlight"><code>
{%- include code/snippets/manual-typeof-example-1.html -%}
</code></pre></div>

# NOTES

The *input* to the `@typeof` directive is that which is being inspected. The input can be any rvalue at all, but *should* be a pointer to an object.

An easy mistake to make, for example, would be to pass an object directly instead of its address -- this would implicitly call the object's `toPrimitive` method, and attempt to find the "type of" that method's output.
