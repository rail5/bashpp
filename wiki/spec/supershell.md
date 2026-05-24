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

The supershell is expanded to the output of the command sequence. The commands in the supershell are executed in the same environment as the surrounding code, so any changes made to the environment (such as setting variables or changing directories) will affect the current shell. Their output however is redirected to the result of the supershell rather than the terminal.

# EXAMPLE

<div class="highlight"><pre class="highlight"><code>
{%- include code/snippets/manual-supershell-example-1.html -%}
</code></pre></div>

# NOTES

If an object's method is referenced in an rvalue position, the method will be executed in a supershell, and its output will be substituted in place of the method call. See [bpp-value-categories(3)](value-categories.md) for more information on rvalues and lvalues.

# SEE ALSO

 - [bpp-methods(3)](methods.md) for more information on object methods
 - [bpp-toprimitive(3)](toprimitive.md) for more information on the `toPrimitive` method
 - [bpp-value-categories(3)](value-categories.md) for more information on lvalue and rvalue method calls
