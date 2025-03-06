# Change Log

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
