# Bash++

<img title="" src="./wiki/banner.png" alt="Bash++: Bash with classes" width="319" data-align="center">

```javascript
@class Bashpp {
  @public author="Andrew S. Rightenburg"
  @public source="https://github.com/rail5/bashpp"
  @public license="GNU GPL v3"

  @public @method printInfo {
	echo "Bash++ is a superset of Bash that adds support for classes and objects."
	echo "It's designed to be a simple way to add object-orientation to Bash scripts."
	echo "Author: @this.author"
	echo "Source: @this.source"
	echo "License: @this.license"
  }
}

@Bashpp myBashpp
@myBashpp.printInfo
```

More documentation is available on the [website](https://bpp.sh).

![GPLv3](https://www.gnu.org/graphics/gplv3-with-text-136x68.png)


The Bash++ language and compiler are licensed under the GNU General Public License v3.

This does not affect any code you write in Bash++ – only the Bash++ language and compiler themselves. You are perfectly free to use Bash++ to write software under a different license.

## Installation

> [!IMPORTANT]
> The Bash++ compiler is currently in *beta*, and is expected to have bugs. Please report any bugs you find to the [issue tracker](https://github.com/rail5/bashpp/issues).
>

### Debian GNU/Linux

Users of Debian-based distributions can install the compiler from the [deb.rail5.org](https://deb.rail5.org) repository.

```shell
sudo curl -s -o /etc/apt/trusted.gpg.d/rail5-signing-key.gpg "https://deb.rail5.org/rail5-signing-key.gpg"
sudo curl -s -o /etc/apt/sources.list.d/rail5.list "https://deb.rail5.org/rail5.list"
sudo apt update
sudo apt install bpp
```

Pre-built packages are available for **amd64**, **i386**, and **arm64** architectures. The .debs can also be downloaded directly from the [GitHub releases page](https://github.com/rail5/bashpp/releases/latest)

### Building from source

#### Prerequisites

 - `bash`
 - `make`
 - `g++`
 - `antlr4`
 - Antlr4 C++ runtime

Optional:
 - `pandoc` and `perl` for building the documentation
 - `debhelper` for building the Debian package and keeping version numbers up-to-date via `dpkg-parsechangelog`

On Debian-based systems, you can install the prerequisites with:

```bash
$ sudo apt update
$ sudo apt install build-essential antlr4 libantlr4-runtime-dev pandoc perl debhelper
```

#### Building

```bash
$ make        # Build the Bash++ compiler, which can then be found at bin/bpp
$ make manual # Build the manpages, which can then be found at debian/bpp.1 and debian/bpp.7
$ make test   # Run the test suite. The test suite is itself written in Bash++.
$ debuild -us -uc # Build the Debian .deb package for installation
```

## Using the compiler

```bash
$ bpp program.bpp                 # Compile & immediately run the program
$ bpp -o compiled.sh program.bpp  # Compile & save the program to compiled.sh
$ bpp -o - program.bpp            # Compile & print the program to stdout
$ cat program.bpp | bpp           # Pipe previous command output to the compiler
$ bpp -h # Display help
$ bpp -v # Display version
```

It's strongly recommended to use the `-o` flag to specify an output file. If you don't, you'll be re-compiling your programs every time you run them, which could take some time, especially for larger programs. Using `-o` and running the compiled program is much faster for subsequent runs.

## Debugging

If you encounter a bug, please report it to the [issue tracker](https://github.com/rail5/bashpp/issues).

If you'd like to debug the compiler yourself, you can use the compiler with the `-tp` flags. Using `-t` will print lexer output, and using `-p` will print the parse tree for an input file. This can be useful for debugging syntax errors and compiler issues.

```bash
$ bpp -tp program.bpp
```

[Shellwatch](https://github.com/rail5/shellwatch) is a tool designed specifically to help debug Bash++ programs and compiler issues. Shellwatch will step a Bash script line-by-line, showing the current state of variables and their values, as well as the current line of the script being executed. Shellwatch will also allow you to modify the values of stored variables before continuing script execution. This tool can be very helpful for debugging complex (compiled) Bash++ programs which are hard to analyze with your eyes alone.

```bash
$ shellwatch compiled-program.sh
```

## VSCode extension

The [Bash++ extension for Visual Studio Code](https://marketplace.visualstudio.com/items?itemName=rail5.bashpp) provides syntax highlighting for Bash++.
