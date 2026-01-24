---
layout: custom
title: Language Interoperability
---
# NAME

Interoperability - Interoperability with Bash Features

# DESCRIPTION

Certain Bash features are incompatible with Bash++'s object model and runtime guarantees. Use of these constructs is not banned, but results in **undefined behavior**. This means that the behavior of the program may vary depending on the Bash version, the system, or even the specific execution context.

## eval

The `eval` command is a Bash built-in that executes its arguments as a Bash command. In Bash++, using `eval` can lead to unexpected behavior, especially when dealing with objects and methods, because it bypasses the Bash++ compiler altogether. It may interfere with scope tracking, scope safety, object lifetimes, and method resolution.

Because the arguments passed to `eval` are interpreted by Bash directly, the Bash++ compiler is unable to enforce any of its guarantees on code executed via `eval`.

The behavior of `eval` is therefore undefined in Bash++ programs.

## set

The `set` command in Bash is used to change shell options and positional parameters. In Bash++, using `set` to modify shell options can conflict with the assumptions made by the Bash++ runtime about the execution environment. This can lead to undefined behavior, especially if options that affect error handling or command execution are altered.

In particular, changing options like `-e` (exit on error) or `-u` (treat unset variables as an error) can lead to premature termination of the program, or unexpected errors when accessing object properties, and will almost certainly break the guarantees provided by Bash++ (e.g., object lifetimes and destructors).

The behavior of `set` is therefore undefined in Bash++ programs.

# NOTES

These features are not banned and you are free to use them as you see fit. However, their behavior is **undefined** as far as Bash++ is concerned. This means that the Bash++ compiler will not enforce any rules regarding their use, and the behavior of your program may vary depending on how you use them.
