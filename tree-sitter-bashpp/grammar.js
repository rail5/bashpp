/**
 * @file Bash++ grammar for Tree-sitter
 * @license GPL-3.0-or-later
 */

const Bash = require('tree-sitter-bash/grammar');

const BASH_SPECIAL_CHARACTERS = [
  '\'',
  '"',
  '<',
  '>',
  '{',
  '}',
  '\\[',
  '\\]',
  '(',
  ')',
  '`',
  '$',
  '=',
  '|',
  '&',
  ';',
  '\\',
  '\\s',
  '@',
];

module.exports = grammar(Bash, {
  name: 'bashpp',

  externals: ($, previous) => previous.concat([
    $._bashpp_object_type,
    $._bashpp_pointer_type,
    $._bashpp_assignment_reference,
    $._bashpp_assignment_operator,
    $._bashpp_reference_end,
  ]),

  rules: {
    _statement_not_subshell: ($, previous) => choice(
      $.variable_assignment,
      $.class_definition,
      $.data_member_declaration,
      $.method_definition,
      $.constructor_definition,
      $.destructor_definition,
      $.include_statement,
      $.object_declaration,
      $.pointer_declaration,
      $.object_assignment,
      $.delete_statement,
      previous,
    ),

    _statement_not_pipeline: ($, previous) => choice(
      $.variable_assignment,
      $.class_definition,
      $.data_member_declaration,
      $.method_definition,
      $.constructor_definition,
      $.destructor_definition,
      $.include_statement,
      $.object_declaration,
      $.pointer_declaration,
      $.object_assignment,
      $.delete_statement,
      previous,
    ),

    variable_assignment: $ => seq(
      field('name', choice(
        $.variable_name,
        $.subscript,
      )),
      field('operator', choice('=', '+=')),
      field('value', choice(
        $._bashpp_value,
        $._empty_value,
        alias($._comment_word, $.word),
      )),
    ),

    subscript: $ => seq(
      field('name', $.variable_name),
      '[',
      field('index', choice(
        alias('@', $.word),
        $._literal,
        $.binary_expression,
        $.unary_expression,
        $.compound_statement,
        $.subshell,
      )),
      optional($._concat),
      ']',
      optional($._concat),
    ),

    command: ($, previous) => choice(
      prec.dynamic(10, seq(
        field('name', alias(
          $._bashpp_braced_command_assignment,
          $.command_name,
        )),
      )),
      previous,
    ),

    command_name: ($, previous) => choice(
      $.object_reference,
      $.self_reference,
      $.braced_reference,
      $.pointer_dereference,
      $.supershell,
      previous,
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
        field('type', choice(
          alias($._bashpp_object_type, $.type_identifier),
          alias($._bashpp_pointer_type, $.type_identifier),
        )),
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
      field('type', alias($._bashpp_object_type, $.type_identifier)),
      field('name', alias($._bashpp_identifier, $.object_name)),
      optional(field('value', $.default_value)),
    )),

    pointer_declaration: $ => prec.right(3, seq(
      field('type', alias($._bashpp_pointer_type, $.type_identifier)),
      field('name', alias($._bashpp_identifier, $.object_name)),
      optional(field('value', $.default_value)),
    )),

    default_value: $ => seq(
      field('operator', $.assignment_operator),
      field('value', $._bashpp_value),
    ),

    assignment_operator: $ => $._bashpp_assignment_operator,

    access_modifier: _ => choice(
      bashppKeyword('@public'),
      bashppKeyword('@private'),
      bashppKeyword('@protected'),
    ),

    virtual_modifier: _ => bashppKeyword('@virtual'),

    object_assignment: $ => prec.right(10, seq(
      field('left', $.assignment_reference),
      optional(field('index', $.reference_index)),
      field('operator', $.assignment_operator),
      field('right', $._bashpp_value),
    )),

    assignment_reference: $ => field(
      'name',
      alias($._bashpp_assignment_reference, $.reference_name),
    ),

    object_reference: $ => prec.dynamic(10, seq(
      field('name', alias($._bashpp_reference_name, $.reference_name)),
      optional(field('index', $.reference_index)),
      $._bashpp_reference_end,
    )),

    self_reference: $ => prec.dynamic(10, seq(
      field('name', choice(
        alias($._bashpp_this_reference, $.self),
        alias($._bashpp_super_reference, $.super),
      )),
      optional(field('index', $.reference_index)),
      $._bashpp_reference_end,
    )),

    braced_reference: $ => prec.dynamic(10, seq(
      bashppKeyword('@{'),
      field('name', alias($._bashpp_braced_name, $.reference_name)),
      optional(field('index', $.reference_index)),
      '}',
    )),

    reference_index: $ => seq(
      '[',
      field('value', choice(
        '@',
        $._expression,
      )),
      ']',
    ),

    address_expression: $ => prec.dynamic(10, seq(
      field('name', alias($._bashpp_address_name, $.reference_name)),
      optional(field('index', $.reference_index)),
      $._bashpp_reference_end,
    )),

    pointer_dereference: $ => prec.dynamic(10, seq(
      field('name', alias(
        $._bashpp_dereference_name,
        $.reference_name,
      )),
      optional(field('index', $.reference_index)),
      $._bashpp_reference_end,
    )),

    new_expression: $ => prec.right(4, seq(
      bashppKeyword('@new'),
      field('type', alias($._bashpp_identifier, $.type_identifier)),
    )),

    delete_statement: $ => prec.right(4, seq(
      bashppKeyword('@delete'),
      field('argument', choice(
        $.object_reference,
        $.self_reference,
      )),
    )),

    nullptr_literal: _ => bashppKeyword('@nullptr'),

    typeof_expression: $ => prec.right(4, seq(
      bashppKeyword('@typeof'),
      field('argument', $._bashpp_value),
    )),

    dynamic_cast_expression: $ => prec.right(4, seq(
      bashppKeyword('@dynamic_cast'),
      '<',
      field('target', $.dynamic_cast_target),
      '>',
      field('value', $._bashpp_value),
    )),

    dynamic_cast_target: $ => choice(
      alias($._bashpp_cast_type, $.type_identifier),
      $.simple_expansion,
      $.expansion,
      $.object_reference,
      $.self_reference,
      $.braced_reference,
    ),

    supershell: $ => prec.right(4, seq(
      bashppKeyword('@('),
      optional($._statements),
      ')',
    )),

    _primary_expression: ($, previous) => choice(
      previous,
      $.object_reference,
      $.self_reference,
      $.braced_reference,
      $.address_expression,
      $.pointer_dereference,
      $.new_expression,
      $.nullptr_literal,
      $.typeof_expression,
      $.dynamic_cast_expression,
      $.supershell,
    ),

    _bashpp_braced_command_assignment: $ => prec.dynamic(10, seq(
      $.braced_reference,
      $.assignment_operator,
      $._primary_expression,
    )),

    _c_expression_not_assignment: ($, previous) => choice(
      previous,
      $.object_reference,
      $.self_reference,
      $.braced_reference,
      $.pointer_dereference,
      $.nullptr_literal,
      $.typeof_expression,
      $.supershell,
    ),

    _arithmetic_literal: ($, previous) => choice(
      previous,
      $.object_reference,
      $.self_reference,
      $.braced_reference,
      $.pointer_dereference,
      $.nullptr_literal,
      $.typeof_expression,
      $.supershell,
    ),

    string: $ => seq(
      '"',
      repeat(seq(
        choice(
          seq(optional('$'), $.string_content),
          $.expansion,
          $.simple_expansion,
          $.command_substitution,
          $.arithmetic_expansion,
          $._bashpp_string_interpolation,
        ),
        optional($._concat),
      )),
      optional('$'),
      '"',
    ),

    string_content: _ => token(prec(-1, /([^"`$@&*\\\r\n]|\\(.|\r?\n))+/)),

    _bashpp_string_interpolation: $ => choice(
      $.object_reference,
      $.self_reference,
      $.braced_reference,
      $.address_expression,
      $.pointer_dereference,
      $.supershell,
      alias(/[&*]/, $.string_content),
    ),

    heredoc_body: $ => seq(
      $._heredoc_body_beginning,
      repeat(choice(
        $.expansion,
        $.simple_expansion,
        $.command_substitution,
        $.object_reference,
        $.self_reference,
        $.braced_reference,
        $.address_expression,
        $.pointer_dereference,
        $.supershell,
        $.heredoc_content,
      )),
    ),

    _bashpp_value: $ => choice(
      $._literal,
      $.array,
      $.new_expression,
      $.nullptr_literal,
      $.typeof_expression,
      $.dynamic_cast_expression,
    ),

    word: _ => token(prec(-1, seq(
      choice(
        noneOf('#', ...BASH_SPECIAL_CHARACTERS),
        seq('\\', noneOf('\\s')),
      ),
      repeat(choice(
        noneOf(...BASH_SPECIAL_CHARACTERS),
        seq('\\', noneOf('\\s')),
        '\\ ',
      )),
    ))),

    _comment_word: _ => token(prec(-8, seq(
      choice(
        noneOf(...BASH_SPECIAL_CHARACTERS),
        seq('\\', noneOf('\\s')),
      ),
      repeat(choice(
        noneOf(...BASH_SPECIAL_CHARACTERS),
        seq('\\', noneOf('\\s')),
        '\\ ',
      )),
    ))),

    _bashpp_identifier: _ => /[a-zA-Z_][a-zA-Z0-9_]*/,

    _bashpp_cast_type: _ => /[a-zA-Z_][a-zA-Z0-9_]*\*?/,

    _bashpp_reference_name: _ => token(prec(
      3,
      /@[a-zA-Z_][a-zA-Z0-9_]*(\.[a-zA-Z_][a-zA-Z0-9_]*)*/,
    )),

    _bashpp_this_reference: _ => token(prec(
      5,
      /@this(\.[a-zA-Z_][a-zA-Z0-9_]*)*/,
    )),

    _bashpp_super_reference: _ => token(prec(
      5,
      /@super(\.[a-zA-Z_][a-zA-Z0-9_]*)*/,
    )),

    _bashpp_braced_name: _ => token(prec(
      2,
      /#?[a-zA-Z_][a-zA-Z0-9_]*(\.[a-zA-Z_][a-zA-Z0-9_]*)*/,
    )),

    _bashpp_address_name: _ => token(prec(
      3,
      /&@[a-zA-Z_][a-zA-Z0-9_]*(\.[a-zA-Z_][a-zA-Z0-9_]*)*/,
    )),

    _bashpp_dereference_name: _ => token(prec(
      3,
      /\*@[a-zA-Z_][a-zA-Z0-9_]*(\.[a-zA-Z_][a-zA-Z0-9_]*)*/,
    )),

  },
});

/**
 * Gives reserved Bash++ words priority over the general `@Type` token.
 *
 * @param {string} value
 * @returns {TokenRule}
 */
function bashppKeyword(value) {
  return token(prec(5, value));
}

/**
 * Returns a regular expression that excludes the provided characters.
 *
 * @param {...string} characters
 * @returns {RegExp}
 */
function noneOf(...characters) {
  const negatedCharacters = characters
    .map(character => character === '\\' ? '\\\\' : character)
    .join('');
  return new RegExp('[^' + negatedCharacters + ']');
}
