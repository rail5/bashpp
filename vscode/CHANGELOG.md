# Change Log

## [0.5.4]

- Highlight classes written after `@new`

## [0.5.3]

- Added support for the new Bash++ language server
  - If installed on your system, the language server will be automatically started when you open a Bash++ file in VSCode.
  - The language server provides features such as:
    - Syntax checking
    - Code completion
    - Go to definition
    - Find references
  - The language server can be disabled or configured via the VSCode settings.

## [0.5.2]

- Bug fix: properly highlight `@this` and `@super` as *keywords* within double-quoted strings
 - Also properly highlight `@{this}` and `@{super}` as keywords

## [0.5.1]

- Specially-highlight the `@this` and `@super` keywords even when followed by a "dot chain"
  - I.e., `@this.innerObject.method` should show `@this` highlighted *as a keyword*, and `.innerObject.method` highlighted *as an object reference*
  - Previously, the entire string was simply highlighted as an object reference

## [0.5.0]

- Brought syntax rules up to speed with Bash++ v0.5.0 by adding support for the `@super` keyword

## [0.4.0]

- Brought syntax rules up to speed with Bash++ v0.4.0 by adding support for the `@include` directive's new syntax
  - The `@include` (or `@include_once`) directives can now optionally specify whether the file should be linked dynamically or statically.
  - If linked dynamically, you can also optionally specify where the compiled library will be found at runtime.
  - The syntax is now: `@include [dynamic|static] {PATH} [as "{PATH}"]`
  - This syntax is backwards-compatible with the old syntax, so `@include {PATH}` will still work as before.

## [0.3.1]

- Compatible with VSCode versions at least as old as 1.75.0

## [0.3.0]

- Brought syntax rules up to speed with Bash++ v0.3.0 by adding recognition of the `@dynamic_cast` keyword

## [0.2.1]

- Identify source files as Bash++ even without the `.bpp` extension by checking the shebang line for `bpp` as the interpreter.

## [0.2.0]

- Brought syntax rules up to speed with Bash++ v0.2.0 by adding support for angle-bracket includes such as @include &lt;file&gt;

## [0.0.9]

- Updated syntax rules to reflect the removal of type-casting from Bash++
  - Type-casting may be implemented in a future version of Bash++, but for now, it is not a feature of the language.

## [0.0.8]

- Properly highlight variables within heredocs

## [0.0.7]

- Patched a bug causing valid identifiers to not be recognized
  - Identifiers in Bash++ cannot contain two consecutive underscores. The regex for this in the syntax rules was mistakenly written such that, if the *line* itself contained two consecutive underscores, a valid identifier on that line would not be matched.

## [0.0.6]

- Update syntax rules: methods cannot take non-primitive arguments, but they can take pointers to non-primitive types.

## [0.0.5]

- Updated icon

## [0.0.4]

- Patched a bug introduced in the previous change that messed up highlighting of Bash++ variables within strings.

## [0.0.3]

- Moved Bash++-specific highlighting rules to a higher priority than shell keywords and builtins.
  - This way, Bash++ keywords will be highlighted correctly even if they are also valid shell keywords.
- Changed Language Mode name to `Bash++` instead of `Bash++ Script`.

## [0.0.2]

- Changed publisher name to `rail5`
- Added icon to the extension
- Published the extension to the Visual Studio Code Marketplace

## [0.0.1]

- Initial release
