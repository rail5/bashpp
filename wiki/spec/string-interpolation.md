---
layout: custom
title: String Interpolation
---
# NAME

Interpolation - Embedding expressions within string literals

# SYNOPSIS

<div class="highlight"><pre class="highlight"><code>
{%- include code/snippets/string-interpolation-example.html -%}
</code></pre></div>

# DESCRIPTION

Like in Bash, certain expressions can be embedded within double-quoted strings, such as variable references and command substitutions. Double-quoted strings in Bash++ can also contain interpolations of object references and supershells.

Object references can be embedded using either the `@object.member` syntax or the `@{object.member}` syntax. If the first form is used, the object reference parse ends at the first character that is not valid in an ordinary object reference (that is, either a character which is not a valid part of an identifier, or a dot which is not followed by an identifier). If the second form is used, the object reference parse ends at the closing `}` character.

If an object's method is referenced in either of the above forms, the method will be implicitly executed in a supershell, and its output will be substituted in place of the interpolation in the string.

Supershells can be embedded using the `@(...)` syntax. The supershell parse ends at the matching closing `)` character. Nested supershells are supported.

Other Bash++ expressions are not supported within double-quoted strings. The standard Bash interpolations however are all available, including variable references, command substitutions, arithmetic expansions, etc.

# SEE ALSO
- [bpp-objects(3)](objects.md) for more information on objects
- [bpp-methods(3)](methods.md) for more information on methods
- [bpp-supershell(3)](supershell.md) for more information on supershells
