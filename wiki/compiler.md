# Using the Bash++ compiler

## Basic usage

`bpp [options] [file] ...`

```bash
$ bpp program.bpp                 # Compile & immediately run the program
$ bpp -o compiled.sh program.bpp  # Compile & save the program to compiled.sh
$ bpp -o - program.bpp            # Compile & print the program to stdout
$ cat program.bpp | bpp           # Pipe previous command output to the compiler
$ bpp -h # Display help
$ bpp -v # Display version
```

## Options

- `-o` or `--output` : Specify the output file
	- If the output file is `-`, the program will be printed to stdout
	- If the output file is not specified, the program will be executed immediately after compilation
- `-I` or `--include` : Add a directory to the include path
	- The default include path is `/usr/lib/bpp/stdlib`
	- Files in the include paths can be included with `@include <file>`
- `-D` or `--dynamic` : Enable dynamic linking
	- Compiled code from included files will be linked at runtime rather than compile time
- `-t` or `--tokens` : Display lexer output (do not compile)
- `-p` or `--parse-tree` : Display parse tree (do not compile)
- `-h` or `--help` : Display help
- `-v` or `--version` : Display version

If no input file is specified, the compiler will read from stdin.

Arguments after the input file are passed to the compiled program.

## Include Paths

When an include directive is given with angle-brackets (as in `@include <file>`), the compiler will search for the file in the include paths. By default, `/usr/lib/bpp/stdlib` is the first include path.

Multiple include paths can be added with the `-I` option. The paths are searched in the order they are given.

## Dynamic vs. Static Linking

Static linking is enabled by default. This means that the compiled code from included files will be inserted into the compiled program.

The primary advantage of static linking in Bash++ is that the compiled program is a single Bash script that can be run on any system with Bash installed. The user does not need to have Bash++ installed, nor do they need to have the included files.

Dynamic linking is useful when the included files are frequently updated or when the compiled program is very large. In this case, the compiled program will be smaller and will run faster, but it will require the included files to be present at runtime.

When an included file is dynamically linked, the compiler will place a Bash `source` directive in the compiled program. This directive will load the included file at runtime.

Dynamic linking can be enabled with the `-D` option.
