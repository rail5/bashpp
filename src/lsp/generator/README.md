# LSP C++ Classes Generator

The files in this directory are responsible reading the LSP spec from `metaModel.json` and automatically generating C++ classes for each of the LSP types (Messages, Parameters, Objects, etc.) defined in the spec.

The generated classes are then used to handle LSP requests and responses in the Bash++ language server.
