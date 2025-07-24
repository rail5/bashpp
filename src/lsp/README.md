# Bash++ Language Server Protocol (LSP) Implementation

## Overview

This is a work-in-progress implementation of a Bash++ language server.

It is designed to provide language server protocol (LSP) features such as code completion, go-to definition, and hover information for Bash++ scripts.

## Feature Checklist

- [&#10003;] Diagnostics (syntax errors, etc.)
- [&#10003;] Code completions
- [&#10003;] Go to definition
- [&#10003;] Hover information
- [ &nbsp; ] Document symbols (in progress)
- [ &nbsp; ] Workspace renaming (in progress)

## Copyright and License

Note that, as is also the case for the rest of the Bash++ project, the LSP implementation is licensed under the [GNU GPL v3](https://www.gnu.org/licenses/gpl-3.0.en.html).

The files `metaModel.json` and `metaModel.schema.json` which define the LSP model are pulled from Microsoft's [Language server protocol implementation for VSCode](https://github.com/microsoft/vscode-languageserver-node) and are licensed under the MIT License.

Full contents of the original license in compliance with its terms of reuse:

```
Copyright (c) Microsoft Corporation

All rights reserved.

MIT License

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy,
modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software
is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED *AS IS*, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT
OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
```
