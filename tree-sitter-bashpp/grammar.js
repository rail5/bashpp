/**
 * @file Bash++ grammar for Tree-sitter
 * @license GPL-3.0-or-later
 */

const Bash = require('tree-sitter-bash/grammar');

module.exports = grammar(Bash, {
  name: 'bashpp',
  rules: {},
});
