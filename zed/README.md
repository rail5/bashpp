# Bash++ for Zed

This extension recognizes `.bpp` files and reuses Zed's built-in Bash grammar
for baseline syntax highlighting. Bash++ identifiers are classified by
`bpp-lsp` using the compiler's Flex/Bison AST, so the extension does not
maintain a second Bash++ parser.

## Development installation

1. Build and install Bash++ so that `bpp-lsp` is available on `PATH`.
2. Install Rust and add the targets used by this repository and current Zed
   releases:

   ```bash
   rustup target add wasm32-wasip1 wasm32-wasip2
   ```

3. Run `make test-zed` from the repository root.
4. Open Zed's extension view, select **Install Dev Extension**, and choose this
   `zed/` directory.
5. Enable combined semantic highlighting in Zed's settings:

   ```json
   {
     "languages": {
       "Bash++": {
         "semantic_tokens": "combined"
       }
     }
   }
   ```

Zed will use Tree-sitter Bash highlighting immediately. Once `bpp-lsp` starts,
semantic tokens add Bash++ class, method, property, variable, and parameter
highlighting.

## Marketplace publication

After the extension is ready for publication, add this repository as a
submodule in the
[`zed-industries/extensions`](https://github.com/zed-industries/extensions)
registry and set `path = "zed"` for its entry in `extensions.toml`. The
extension source remains in this repository; no separate parser or package is
required.
