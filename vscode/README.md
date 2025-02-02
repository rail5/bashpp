# Bash++ VSCode Extension

This extension provides basic syntax highlighting for Bash++ files.

## Building

First, you need to install `npm`, `node` and `vsce`.

```bash
sudo apt install npm nodejs
sudo npm install -g vsce
```

Then, you can build the extension.

```bash
make # Or: vsce package
```

This will create a `.vsix` file that you can install in VSCode.

## Extension Information

The TextMate grammar is a modified version of the [Bash](https://github.com/microsoft/vscode-textmate/blob/main/test-cases/themes/syntaxes/Shell-Unix-Bash.tmLanguage.json) grammar provided by Microsoft.

The original grammar is licensed under the [MIT License](https://choosealicense.com/licenses/mit/)

## MIT License

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