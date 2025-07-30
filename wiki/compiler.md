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
- `-s` or `--no-warnings` : Suppress warnings
- `-I` or `--include` : Add a directory to the include path
	- The default include path is `/usr/lib/bpp/stdlib`
	- Files in the include paths can be included with `@include <file>`
- `-b` or `--target-bash` : Specify the target Bash version for the compiled program
	- The default is Bash 5.2, but you can specify a different version like `-b 5.3` or `-b 6.0`
	- This affects how the program is compiled, especially regarding features and syntax compatibility
- `-t` or `--tokens` : Display lexer output (do not compile)
- `-p` or `--parse-tree` : Display parse tree (do not compile)
- `-h` or `--help` : Display help
- `-v` or `--version` : Display version

If no input file is specified, the compiler will read from stdin.

Arguments after the input file are passed to the compiled program.

## Include Paths

When an include directive is given with angle-brackets (as in `@include <file>`), the compiler will search for the file in the include paths. By default, `/usr/lib/bpp/stdlib` is the first include path.

Multiple include paths can be added with the `-I` option. The paths are searched in the order they are given.
