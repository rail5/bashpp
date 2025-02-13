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
- `-t` or `--tokens` : Display lexer output (do not compile)
- `-p` or `--parse-tree` : Display parse tree (do not compile)
- `-h` or `--help` : Display help
- `-v` or `--version` : Display version

If no input file is specified, the compiler will read from stdin.

Arguments after the input file are passed to the compiled program.
