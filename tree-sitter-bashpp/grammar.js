/**
 * @file Bash++ grammar for Tree-sitter
 * @license GPL-3.0-or-later
 */

const Bash = require('tree-sitter-bash/grammar');

module.exports = grammar(Bash, {
  name: 'bashpp',

  rules: {
    _statement_not_subshell: ($, previous) => choice(
      previous,
      $.class_definition,
      $.data_member_declaration,
      $.method_definition,
      $.constructor_definition,
      $.destructor_definition,
      $.include_statement,
      $.object_declaration,
      $.pointer_declaration,
    ),

    _statement_not_pipeline: ($, previous) => choice(
      previous,
      $.class_definition,
      $.data_member_declaration,
      $.method_definition,
      $.constructor_definition,
      $.destructor_definition,
      $.include_statement,
      $.object_declaration,
      $.pointer_declaration,
    ),

    class_definition: $ => prec.right(3, seq(
      bashppKeyword('@class'),
      field('name', alias($._bashpp_identifier, $.type_identifier)),
      optional(seq(
        ':',
        field('parent', alias($._bashpp_identifier, $.type_identifier)),
      )),
      field('body', $.compound_statement),
    )),

    data_member_declaration: $ => prec.right(4, seq(
      field('modifier', $.access_modifier),
      choice(
        seq(
          field('name', alias($._bashpp_identifier, $.member_name)),
          optional(field('value', $.default_value)),
        ),
        field('declaration', choice(
          $.object_declaration,
          $.pointer_declaration,
        )),
      ),
    )),

    method_definition: $ => prec.right(4, seq(
      optional(field('virtual', $.virtual_modifier)),
      field('modifier', $.access_modifier),
      bashppKeyword('@method'),
      field('name', alias($._bashpp_identifier, $.method_name)),
      repeat(field('parameter', $.method_parameter)),
      field('body', $.compound_statement),
    )),

    method_parameter: $ => choice(
      field('name', alias($._bashpp_identifier, $.parameter_name)),
      seq(
        field('type', alias($._bashpp_type, $.type_identifier)),
        optional(field('pointer', '*')),
        field('name', alias($._bashpp_identifier, $.parameter_name)),
      ),
    ),

    constructor_definition: $ => prec.right(4, seq(
      bashppKeyword('@constructor'),
      field('body', $.compound_statement),
    )),

    destructor_definition: $ => prec.right(4, seq(
      bashppKeyword('@destructor'),
      field('body', $.compound_statement),
    )),

    include_statement: $ => prec.right(4, seq(
      field('keyword', choice(
        bashppKeyword('@include'),
        bashppKeyword('@include_once'),
      )),
      optional(field('type', $.include_type)),
      field('path', $.include_path),
      optional(seq(
        'as',
        field('alias', $.include_path),
      )),
    )),

    include_type: _ => choice('dynamic', 'static'),

    include_path: _ => token(choice(
      /"([^"\\\r\n]|\\.)+"/,
      /<[^>\r\n]+>/,
    )),

    object_declaration: $ => prec.right(3, seq(
      field('type', alias($._bashpp_type, $.type_identifier)),
      field('name', alias($._bashpp_identifier, $.object_name)),
      optional(field('value', $.default_value)),
    )),

    pointer_declaration: $ => prec.right(3, seq(
      field('type', alias($._bashpp_type, $.type_identifier)),
      field('pointer', '*'),
      field('name', alias($._bashpp_identifier, $.object_name)),
      optional(field('value', $.default_value)),
    )),

    default_value: $ => seq(
      field('operator', choice('=', '+=')),
      field('value', $._bashpp_value),
    ),

    access_modifier: _ => choice(
      bashppKeyword('@public'),
      bashppKeyword('@private'),
      bashppKeyword('@protected'),
    ),

    virtual_modifier: _ => bashppKeyword('@virtual'),

    _bashpp_value: $ => choice(
      $._literal,
      $.array,
    ),

    _bashpp_identifier: _ => /[a-zA-Z_][a-zA-Z0-9_]*/,

    _bashpp_type: _ => token(prec(2, /@[a-zA-Z_][a-zA-Z0-9_]*/)),
  },
});

/**
 * Gives reserved Bash++ words priority over the general `@Type` token.
 *
 * @param {string} value
 * @returns {TokenRule}
 */
function bashppKeyword(value) {
  return token(prec(3, value));
}
