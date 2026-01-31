---
layout: custom
title: Supershells
brief: "Supershell"
description: "Capture the output of a command while allowing it to modify the environment"
---
# NAME

Supershell - Capture the output of a command while allowing it to modify the environment

# SYNOPSIS

```bash
@({COMMAND SEQUENCE})
```

# DESCRIPTION

A supershell is used to capture the output of a command (or sequence of commands) while also allowing those commands to modify the environment of the current shell.

The output of the supershell is the output of the command sequence. The commands in the supershell are executed in the same environment as the surrounding code, so any changes made to the environment (such as setting variables or changing directories) will affect the current shell. Their output however is redirected to the output of the supershell rather than the terminal.

# EXAMPLE

<div class="highlight"><pre class="highlight"><code>
{%- include code/snippets/manual-supershell-example-1.html -%}
</code></pre></div>

# NOTES

If an object's method is referenced in a place where a primitive is expected, the method will be executed in a supershell, and its output will be substituted in place of the method call.

Directly referencing a non-primitive object in a place where a primitive is expected will implicitly call the object's `toPrimitive` method, which will be executed in a supershell. However, referencing a *pointer* to an object in a place where a primitive is expected will simply return the pointer, since pointers are already primitives.

# SEE ALSO

 - [bpp-methods(3)](methods.md) for more information on object methods
 - [bpp-toprimitive(3)](toprimitive.md) for more information on the `toPrimitive` method
