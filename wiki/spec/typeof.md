---
layout: custom
title: Typeof
---
# NAME

@typeof - Determine the type of an object at runtime

# SYNOPSIS

```bash
echo @typeof {INPUT}
var=@typeof {INPUT}
if [[ @typeof {INPUT} == "expected_type" ]]; then
	# Do something
fi
```

# DESCRIPTION

The `@typeof` directive is used to determine the type of a pointer at runtime.

The **output** of the `@typeof` directive will be either:

 - **The class name** of the object if the input is a valid pointer to an object.
 - An empty string otherwise.

# EXAMPLE

<div class="highlight"><pre class="highlight"><code>
{%- include code/snippets/manual-typeof-example-1.html -%}
</code></pre></div>

# NOTES

The input can be any rvalue at all, but *should* be a pointer to an object.

An easy mistake to make, for example, would be to pass an object directly instead of its address -- this would implicitly call the object's `toPrimitive` method, and attempt to find the "type of" that method's output.
