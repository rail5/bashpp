# Bash++

Bash with classes

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

## Building from source

The Bash++ compiler is currently in pre-alpha, and is not yet ready for general use. It's expected to break often, and is not yet feature-complete. However, if you'd like to build it from source, you can of course do so. Please report any bugs you find to the [issue tracker](https://github.com/rail5/bashpp/issues).

### Prerequisites

 - `bash`
 - `make`
 - `g++`
 - `antlr4`
 - Antlr4 C++ runtime

Optional:
 - `pandoc` and `perl` for building the documentation
 - `debhelper` for building the Debian package and keeping version numbers up-to-date via `dpk-parsechangelog`

On Debian-based systems, you can install the prerequisites with:

```bash
sudo apt update
sudo apt install build-essential antlr4 libantlr4-runtime-dev pandoc perl debhelper
```

### Building

```bash
make # Build the Bash++ compiler, which can then be found at bin/bpp
make manual # Build the manpages, which can then be found at debian/bpp.1 and debian/bpp.5
make parser # Build 'bin/BashppParser', which will print lexer output and parse trees. Useful for debugging.
debuild -us -uc # Build the Debian .deb package for installation
```
