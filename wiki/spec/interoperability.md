---
layout: custom
title: Language Interoperability
---
# NAME

Bash++ Interoperability Notes

# DESCRIPTION

Certain Bash features are incompatible with Bash++'s object model and runtime guarantees. Use of these constructs is not banned, but results in **undefined behavior**. This means that the behavior of the program may vary depending on the Bash version, the system, or even the specific execution context.

## eval

The `eval` command is a Bash built-in that executes its arguments as a Bash command. In Bash++, using `eval` can lead to unexpected behavior, especially when dealing with objects and methods, because it bypasses the Bash++ compiler altogether. It may interfere with scope tracking, scope safety, object lifetimes, and method resolution.

## set -e

The `set -e` command in Bash causes the shell to exit immediately if any command exits with a non-zero status. In Bash++, this can lead to premature termination of the program, especially if an object method fails or if an error occurs in a constructor or destructor. This can result in objects not being properly cleaned up, as the program may terminate before destructors are called. It is recommended to avoid using `set -e` in Bash++ programs, or to use it with caution, ensuring that error handling is properly implemented.

## set -u

The `set -u` command in Bash treats unset variables as an error when substituting. In Bash++, this can lead to issues when accessing object properties or methods that may not be initialized or set. If an object property is accessed before it is initialized, it may cause the program to exit unexpectedly. It is advisable to ensure that all object properties are properly initialized before use, or to avoid using `set -u` in Bash++ programs.

# NOTES

These features are not banned and you are free to use them as you see fit. However, their behavior is **undefined** as far as Bash++ is concerned. This means that the Bash++ compiler will not enforce any rules regarding their use, and the behavior of your program may vary depending on how you use them.
