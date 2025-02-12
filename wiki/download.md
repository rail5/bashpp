---
link-title: Download
---

# Download Bash++

## Compiler

The compiler is currently in *beta* and is expected to have bugs.

Please report any bugs you encounter on the [issue tracker](https://github.com/rail5/bashpp/issues).

### Debian GNU/Linux

Users of Debian-based distributions can install the compiler from the [deb.rail5.org](https://deb.rail5.org) repository.

```shell
sudo curl -s -o /etc/apt/trusted.gpg.d/rail5-signing-key.gpg "https://deb.rail5.org/rail5-signing-key.gpg"
sudo curl -s -o /etc/apt/sources.list.d/rail5.list "https://deb.rail5.org/rail5.list"
sudo apt update

sudo apt install bpp
```

### Source

The source for the Bash++ compiler is available on [GitHub](https://github.com/rail5/bashpp).

## VSCode Extension

The Bash++ extension for Visual Studio Code provides basic syntax highlighting.

You can download it from [GitHub](https://github.com/rail5/bashpp/releases/) or install it from the [VSCode Marketplace](https://marketplace.visualstudio.com/items?itemName=rail5.bashpp).
