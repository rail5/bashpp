# NAME

@include - Include a file in the current script

# SYNOPSIS

```bash
@include [dynamic | static] {FILE} [as "PATH"]
@include_once [dynamic | static] {FILE} [as "PATH"]
```

# DESCRIPTION

The `@include` and `@include_once` directives are used to include files in the current script.

The `@include` directive will include the specified file every time it is called, while `@include_once` will include the file only once, even if it is called multiple times.

You can specify the type of include (dynamic or static) using the `dynamic` or `static` keyword. If you do not specify a type, the default is `static`.

A statically-included file will have its compiled code written to the current script.

A dynamically-included file will be linked at runtime, and the compiled code will be expected to be found somewhere in the system at runtime.

If you specify `dynamic`, you may like to specify *where* the compiled file will be found at runtime. This is done using the `as` keyword, followed by the path to the file. If you do not specify a path, the default is the *original* path of the file, with the extension replaced by `.sh`.

The FILE can be any Bash++ script. If you give the file in **quotes** (`"FILE"`), it will be interpreted as a relative or absolute path. If you give the file in **angle-brackets** (`<FILE>`), the compiler will search for the file in the include paths. You can add more directories to the compiler's include paths using the `-I` option.

# EXAMPLE

```bash
@include_once dynamic "IncludedClass.bpp"
@include <Stack>
@include_once dynamic "/path/to/IncludedClass.bpp" as "/path/to/IncludedClass.sh"
```

# NOTES

The default include type is `static`. If you do not specify a type, the file will be included as a static include.

The default include path (for angle-bracket includes) is `/usr/lib/bpp/stdlib/`. You can add more directories to the compiler's include paths using the `-I` option.

# SEE ALSO

 - [bpp(1)](../compiler.md) for more information on the compiler's options
