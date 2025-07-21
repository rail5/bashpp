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

### Position Encodings

Our compiler uses UTF-8, but VSCode demands UTF-16 position encodings. Microsoft is full of weird ideas about what's normal:

> We choose UTF-16 encoding here since most language store strings in memory in UTF-16 not UTF-8.

&mdash; [LSP Architect](https://github.com/microsoft/language-server-protocol/issues/376)

This statement is simply not true.

Adding UTF-16 position encoding support is going to be a **massive** pain, and *only* has any benefit for Microsoft products. Therefore I consider it extremely low priority.

The language server, at the moment, sends position data to the client based on UTF-8 counts. In the vast majority of cases, this will not cause a problem, but if you have emojis in your source code, there will be some discrepancies.

The trouble comes from 4-byte UTF-8 characters, which are counted as 1 character in UTF-8, but 2 "code points" in UTF-16.

All of the most-commonly used characters however (including Latin, Cyrillic, Greek, Chinese, Japanese, Korean, Arabic, and Hebrew, to name a few) **will work just fine.**

However: emojis, ancient cuneiform, and other obscure characters will not be counted correctly. If you have these in your source code, you'll see error squiggles show up in the wrong place, etc.

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
