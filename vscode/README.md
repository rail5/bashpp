# Bash++ VSCode Extension

This extension provides language support for Bash++, including syntax highlighting and optional language server capabilities.

If the Bash++ language server is installed on your system, the extension will connect to it automatically and provide in-editor completions, error reporting, etc.

## Website

You can find more information about Bash++ at [bpp.sh](https://bpp.sh)

## Building from source

First, you need to install `npm`, `node` and `vsce`.

```bash
sudo apt install npm nodejs
sudo npm install -g vsce
```

Then, you can build the extension.

```bash
make
```

This will create a `.vsix` file that you can install in VSCode.

## Extension Information

### License: GNU GPL v3 or later

This extension (as well as the Bash++ language and compiler) are licensed under the [GNU General Public License v3 or later](https://www.gnu.org/licenses/gpl-3.0.html).

### Language Server Support

By default, the extension will connect to the Bash++ language server **if it is installed on your system.**

If the extension cannot find `/usr/bin/bpp-lsp`, it will silently stop and *only* provide syntax highlighting.

You can configure the path to the language server (as well as other options) in the extension's settings

### Syntax Highlighting

The TextMate grammar is a modified version of the [Bash](https://github.com/microsoft/vscode-textmate/blob/main/test-cases/themes/syntaxes/Shell-Unix-Bash.tmLanguage.json) grammar provided by Microsoft.

The original grammar is licensed under the [MIT License](https://choosealicense.com/licenses/mit/). In compliance with its terms, the full text of the MIT License is provided below.

#### MIT License

```plaintext
MIT License

Copyright (c) [year] [fullname]

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```
