---
layout: custom
title: Value Categories
brief: "Value Categories"
description: "Lvalues and rvalues in Bash++"
---
# NAME

value-categories - Lvalues and rvalues in Bash++

# DESCRIPTION

All symbols in Bash++ are categorized as either *lvalues* or *rvalues*.

Aside: A "symbol" is an identifier-like string: variable names, command names, object references, etc. Symbols can be composed of many tokens, for example `@object.member` is a single symbol composed of four tokens: `@`, `object`, `.`, and `member`.

A symbol is an *lvalue* if and only if the *first* token of the symbol:

 - is the first non-whitespace token in a simple command, OR
 - is preceded *only* by assignments and/or redirections

All other symbols are *rvalues*.

```bash
var=value var2=value2 command arg1 arg2
# In the above command:
# - "var" is an lvalue
# - "var2" is an lvalue
# - "command" is an lvalue
# All other symbols are rvalues
```

## Considerations

 - The term "simple command" has the same meaning as in Bash and POSIX. See either the Bash reference manual or the POSIX specification for a full definition. Intuitively, it can be understood as a single pipeline element. For example, `command1 arg1 | command2 arg2 && command3 arg3` contains three simple commands: `command1 arg1`, `command2 arg2`, and `command3 arg3`. The first token of each of these simple commands is an lvalue, and all other tokens are rvalues.
 - Likewise, "simple commands" are the predicates in constructs such as `if`, `while`, and `until`. For example, in `if command1 arg1; then ...`, the simple command is `command1 arg1`, and `command1` is an lvalue.

```bash
command1 arg1 arg2 | command2 arg3 arg4
# In the above command:
# - "command1" is an lvalue
# - "command2" is an lvalue
# A second "simple command" begins after the pipe character

command1 arg1 $(command2 arg2)
# In the above command:
# - "command1" is an lvalue
# - "command2" is an lvalue
# Command substitutions such as subshells and supershells begin their own "simple commands"
```

 - Quoting a symbol transforms it into an rvalue, even if it would otherwise be an lvalue. This is a subtle point implied by the above rule. If the symbol is quoted, then its first token is preceded by a quote character.

```bash
"var"=value
# This is not a valid assignment, because "var" is an rvalue (it is quoted)

"@object.method" argument
# This method call is an rvalue method call.
# See below for more information on lvalue and rvalue method calls.
# The result is that the call itself will be expanded to the output of the method,
# and the shell will attempt to execute that output as a command.
# The same result can be achieved via an explicit supershell:
@(@object.method) argument
```

## Explanation of terms

In traditional language terminology, an *lvalue* is an expression that refers to a memory location, while an *rvalue* is one which does not.

The easiest way to understand this is to consider an assignment statement:

```bash
variable=value
```

In this statement, `variable` must resolve to a *place* to put data in, and `value` must resolve to the *data* to be put in that place. The former is an lvalue (`l` as in "left-hand side"), and the latter is an rvalue (`r` as in "right-hand side").

Bash++ extends these traditional definitions slightly. Consider the following:

```bash
function shellFunction() {
	# ...
}

# Case 1:
shellFunction argument

# Case 2:
echo shellFunction
```

In Case 1, `shellFunction` is an lvalue, and `argument` is an rvalue. In Case 2, `echo` is an lvalue, and `shellFunction` is an rvalue.

Note that when `shellFunction` is an lvalue, it refers to a function which can be executed. When it's an rvalue, it's a simple string -- Case 2 will simply echo the word "shellFunction."

## Lvalue and rvalue method calls

If an object's method is referenced in an *lvalue* position, the method is simply executed as any other command or shell function.

If an object's method is referenced in an *rvalue* position, the method will be executed in a supershell, and its output will be substituted in place of the method call.

```bash
# Case 1:
@object.method argument # Lvalue method call:
                        # method is executed normally,
                        # "argument" is passed as an argument to the method

# Case 2:
echo @object.method argument2 # Rvalue method call:
                              # method is executed in a supershell,
                              # and its output is passed as an argument to echo
                              # "argument2" is passed as a second argument to echo,
                              # not as an argument to the method

# Case 3:
echo @(@object.method argument) # Lvalue method call inside a supershell:
                                # method is executed normally,
                                # "argument" is passed as an argument to the method,
                                # the method's output is captured by the supershell,
                                # and then passed as an argument to echo
```

## Non-primitive assignments

These work exactly as you would expect.

```
@object.member=@otherObject.otherMember
```

`@object.member` is an lvalue referring to a place to put data in, and `@otherObject.otherMember` is an rvalue referring to the data to be put in that place. The assignment will copy the value from `@otherObject.otherMember` into `@object.member`.

# NOTES

Bash does not exactly share our value categories. Do not make the mistake of thinking that an lvalue symbol will always be executed as a command. For example:

```bash
var="" # Empty string

$var echo hi
```

In the above case, we consider `$var` to be an lvalue, because it is the first non-whitespace symbol in the "simple command."

However, because it expands to the empty string, the "command word" is actually `echo`, and the above code will output "hi" rather than producing an error about an invalid command.

The compiler, however, cannot predict how variables will expand at runtime (this is computationally undecidable). The categorization of symbols into "lvalues" and "rvalues" therefore is based solely on source position, and only incidentally corresponds (in the vast majority of cases) to whether that symbol can be executed as a command.

# SEE ALSO

 - [bpp-methods(3)](methods.md) for more information on object methods
 - [bpp-supershell(3)](supershell.md) for more information on supershells
