# Using the Bash++ compiler

## Basic usage

```bash
$ bpp program.bpp                 # Compile & immediately run the program
$ bpp program.bpp -o compiled.sh  # Compile & save the program to compiled.sh
$ bpp program.bpp -o -            # Compile & print the program to stdout
$ bpp -h # Display help
$ bpp -v # Display version
```

## Options

- `-o` or `--output` : Specify the output file
	- If the output file is `-`, the program will be printed to stdout
	- If the output file is not specified, the program will be executed immediately after compilation
- `-h` or `--help` : Display help
- `-v` or `--version` : Display version
