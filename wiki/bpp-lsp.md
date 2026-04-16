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

###### `--stdio`

Run the language server over standard input/output. This is the default mode of operation.

###### `-l <file>`, `--log <file>`

Log debug messages to the specified file.

###### `-I <path>`, `--include <path>`

Add a directory to the include paths for Bash++ libraries. This allows the language server to resolve symbols and provide completions for libraries located in these directories.

##### `-b <version>`, `--target-bash <version>`

Specify the target Bash version for compilation. This affects how the language server analyzes code and provides diagnostics. The default is 5.2.

###### `-s`, `--no-warnings`

Suppress all warnings during the language server's operation. If this option is used, diagnostics will only include errors.

###### `-h`, `--help`

Display help information.

###### `-v`, `--version`

Display the version of the Bash++ language server.

# SEE ALSO

 - [bpp(1)](compiler.md) for more information on the Bash++ compiler
