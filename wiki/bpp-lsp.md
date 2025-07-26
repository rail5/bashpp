---
layout: custom
title: bpp-lsp
---
# NAME

bpp-lsp - Bash++ language server

# SYNOPSIS

```bash
bpp-lsp [options]
```

# DESCRIPTION

The Bash++ language server (`bpp-lsp`) provides language server protocol support for Bash++ files. It includes features such as code completion, diagnostics, and more. This package is intended for use with editors that support the language server protocol, such as Visual Studio Code or Eclipse Theia.

It is not required for running Bash++ scripts, but enhances the development experience by providing advanced features for Bash++ development.

# OPTIONS

- `-h`, `--help` : Display help
- `-v`, `--version` : Display version\
- `-l`, `--log <file>` : Log debug messages to a file 
- `-I`, `--include <path>` : Add a directory to the include paths
  - The default include path is `/usr/lib/bpp/stdlib`
  - Files in the include paths can be included with `@include <file>`
- `-s`, `--no-warnings` : Suppress warnings
- `--stdio` : Use stdin/stdout for communication (default)
- `--port <port>` : Use TCP port for communication
- `--socket <path>` : Use Unix domain socket for communication

# SEE ALSO

 - [bpp(1)](compiler.md) for more information on the Bash++ compiler
