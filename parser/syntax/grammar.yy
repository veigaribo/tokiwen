%skeleton "lalr1.cc" // -*- C++ -*-

%header

%define api.token.raw
%define api.token.constructor

%define api.parser.class {parser}
%define api.value.type variant
%define parse.assert

%code requires {
  #include "parser/ast.h"
  #include "parser/syntax/symbol_table_stack.h"

  namespace yy {
    class scanner;
  };
}

%code {
  #include "parser/coercions.h"
  #include "parser/lex/scanner.hpp"
  #include "parser/syntax/util.h"
  #include <iostream>

  #undef yylex
  #define yylex lexer.yylex

  #define NEW(...) \
    std::shared_ptr<ast_node>(new __VA_ARGS__)

  #define GET_TYPE(Identifier, Location) \
    get_type(stbuilder.current(), Identifier, Location)

  #define USE_VAR(Identifier, Location) \
    use_var(stbuilder.current(), Identifier, Location)
  #define USE_TYPE(Identifier, Location) \
    use_type(stbuilder.current(), Identifier, Location)

  #define DECLARE_VAR(Type, Name, Location) \
    declare_var(stbuilder.current(), Type, Name, Location)
  #define DECLARE_TYPE(Identifier, Location) \
    declare_type(stbuilder.current(), Identifier, Location)
  #define DECLARE_FUNCTION(Type, Name, Parameters, Body, Location) \
    declare_function(stbuilder.current(), Type, Name, Parameters, Body, Location)

  #define SET_PARAMS(Params) \
    set_parameters(stbuilder, Params)

  #define DECLARE_ASSIGN_VAR(Type, Name, Value, Location) \
    declare_assign_var(stbuilder.current(), Type, Name, Value, Location)

  #define INVOKE_FUNCTION(Name, Arguments, Location) \
    invoke_function(stbuilder.current(), Name, Arguments, Location)

  std::uint64_t function_definition_level = 0;
}

%locations
%define api.location.file "location.hpp"

%define parse.trace
%define parse.error detailed
%define parse.lac full

%define api.token.prefix {TOK_}
%token
  PLUS_ASSIGN    "+="
  MINUS_ASSIGN   "-="
  STAR_ASSIGN    "*="
  SLASH_ASSIGN   "/="
  PERCENT_ASSIGN "%="
  GTEQ           ">="
  LTEQ           "<="
  EQUALS         "=="
  NEQUALS        "!="
  AND            "&&"
  OR             "||"
  NOT            "!"
  ASSIGN         "="
  PLUS           "+"
  MINUS          "-"
  STAR           "*"
  SLASH          "/"
  PERCENT        "%"
  LPARENS        "("
  RPARENS        ")"
  GT             ">"
  LT             "<"
  LCURLY         "{"
  RCURLY         "}"
  SEMI           ";"
  COLON          ":"
  COMMA          ","
  IF             "if"
  ELSE           "else"
  WHILE          "while"
  RETURN         "return"
  GOTO           "goto"
  WRITE          "write"
  READ           "read"
;

%parse-param { yy::scanner &lexer }
%parse-param { symbol_table_stack &stbuilder }
%parse-param { std::shared_ptr<ast_node> *result }
%parse-param { std::string *message_recipient }

%token <std::string> IDENTIFIER "identifier"

%token <std::int64_t> INT_LITERAL   "int"
%token <double> FLOAT_LITERAL       "float"
%token <char> CHAR_LITERAL          "char"
%token <bool> BOOLEAN_LITERAL       "boolean"
%token <std::string> STRING_LITERAL "string"

%nterm <std::shared_ptr<ast_node>> unit
%nterm <std::shared_ptr<ast_node>> write
%nterm <std::shared_ptr<ast_node>> read
%nterm <std::shared_ptr<ast_node>> block
%nterm <std::shared_ptr<ast_node>> goto
%nterm <std::shared_ptr<ast_node>> label
%nterm <std::shared_ptr<ast_node>> conditional
%nterm <std::shared_ptr<ast_node>> while_loop
%nterm <std::shared_ptr<ast_node>> statements
%nterm <std::shared_ptr<ast_node>> statement
%nterm <std::shared_ptr<ast_node>> var_declaration
%nterm <std::shared_ptr<ast_node>> expr_statement
%nterm <std::shared_ptr<ast_node>> expr
%nterm <std::shared_ptr<ast_node>> logical_or_expr
%nterm <std::shared_ptr<ast_node>> logical_and_expr
%nterm <std::shared_ptr<ast_node>> equality_expr
%nterm <std::shared_ptr<ast_node>> relational_expr
%nterm <std::shared_ptr<ast_node>> assignment_expr
%nterm <std::shared_ptr<ast_node>> additive_expr
%nterm <std::shared_ptr<ast_node>> multiplicative_expr
%nterm <std::shared_ptr<ast_node>> unary_expr
%nterm <std::shared_ptr<ast_node>> invocation_expr
%nterm <std::shared_ptr<ast_node>> basic_expr
%nterm <std::shared_ptr<ast_node>> literal

%printer { yyo << $$; } <*>;

%%
%start unit;

unit: statements {
  // Pop the root table, hopefully
  auto table = stbuilder.pop();
  *result = NEW(block_node(table, $1, @$));
  $$ = *result; // Formality
 }

write:
  "write" expr ";" { $$ = NEW(write_node($2, @$)); }

read:
  "read" "identifier" ";" { $$ = NEW(read_node(USE_VAR($2, @2), @$)); }

label:
  "identifier" ":" { $$ = NEW(label_node($1, @$)); }

goto:
  "goto" "identifier" ";" { $$ = NEW(goto_node($2, @$)); }

%right ")" "else";
conditional:
  "if" "(" expr ")" statement { $$ = coerced_conditional($3, $5, NEW(noop_node(@$)), @$); }
| "if" "(" expr ")" statement "else" statement  { $$ = coerced_conditional($3, $5, $7, @$); }

while_loop:
  "while" "(" expr ")" statement { $$ = coerced_while($3, $5, @$); }

block:
  "{"        { stbuilder.push(); }
  statements <std::shared_ptr<ast_node>>{ $$ = $3; }
  "}"        {
    auto table = stbuilder.pop();
    $$ = NEW(block_node(table, $4, @$));
  }

statements:
  %empty               { $$ = NEW(noop_node(@$)); }
| statement statements { $$ = NEW(sequence_node($1, $2, @$)); }

statement:
  var_declaration      { $$ = NEW(statement_node($1, @$)); }
| expr_statement       { $$ = NEW(statement_node($1, @$)); }
| block                { $$ = $1; }
| conditional          { $$ = NEW(statement_node($1, @$)); }
| while_loop           { $$ = NEW(statement_node($1, @$)); }
| label                { $$ = $1; }
| goto                 { $$ = NEW(statement_node($1, @$)); }
| write                { $$ = NEW(statement_node($1, @$)); }
| read                 { $$ = NEW(statement_node($1, @$)); }

var_declaration:
  "identifier" "identifier" ";"          { $$ = DECLARE_VAR($1, $2, @2); }
| "identifier" "identifier" "=" expr ";" { $$ = DECLARE_ASSIGN_VAR($1, $2, $4, @2); }

expr_statement:
  ";"       { $$ = NEW(noop_node(@$)); }
| expr ";"  { $$ = $1; }

expr: logical_or_expr { $$ = $1; }

logical_or_expr:
  logical_or_expr "||" logical_and_expr { $$ = coerced_or($1, $3, @$); }
| logical_and_expr                      { $$ = $1; }

logical_and_expr:
  logical_and_expr "&&" equality_expr { $$ = coerced_and($1, $3, @$); }
| equality_expr                       { $$ = $1; }

equality_expr:
  equality_expr "==" relational_expr { $$ = coerced_equals($1, $3, @$); }
| equality_expr "!=" relational_expr { $$ = coerced_nequals($1, $3, @$); }
| relational_expr                    { $$ = $1; }

relational_expr:
  relational_expr "<" assignment_expr  { $$ = coerced_lt($1, $3, @$); }
| relational_expr ">" assignment_expr  { $$ = coerced_gt($1, $3, @$); }
| relational_expr "<=" assignment_expr { $$ = coerced_lteq($1, $3, @$); }
| relational_expr ">=" assignment_expr { $$ = coerced_gteq($1, $3, @$); }
| assignment_expr                      { $$ = $1; }

assignment_expr:
  "identifier" "=" assignment_expr  { $$ = NEW(assignment_node(USE_VAR($1, @1), $3, @$)); }
| "identifier" "+=" assignment_expr { $$ = NEW(sum_assignment_node(USE_VAR($1, @1), $3, @$)); }
| "identifier" "-=" assignment_expr { $$ = NEW(subtraction_assignment_node(USE_VAR($1, @1), $3, @$)); }
| "identifier" "*=" assignment_expr { $$ = NEW(multiplication_assignment_node(USE_VAR($1, @1), $3, @$)); }
| "identifier" "/=" assignment_expr { $$ = NEW(division_assignment_node(USE_VAR($1, @1), $3, @$)); }
| "identifier" "%=" assignment_expr { $$ = NEW(modulo_assignment_node(USE_VAR($1, @1), $3, @$)); }
| additive_expr                     { $$ = $1; }

additive_expr:
  additive_expr "+" multiplicative_expr { $$ = coerced_sum($1, $3, @$); }
| additive_expr "-" multiplicative_expr { $$ = coerced_subtraction($1, $3, @$); }
| multiplicative_expr                   { $$ = $1; }

multiplicative_expr:
  multiplicative_expr "*" unary_expr { $$ = coerced_multiplication($1, $3, @$); }
| multiplicative_expr "/" unary_expr { $$ = coerced_division($1, $3, @$); }
| multiplicative_expr "%" unary_expr { $$ = coerced_modulo($1, $3, @$); }
| unary_expr                         { $$ = $1; }

unary_expr:
  "-" invocation_expr { $$ = NEW(unary_minus_node($2, @$)); }
| "+" invocation_expr { $$ = NEW(unary_plus_node($2, @$)); }
| "!" invocation_expr { $$ = NEW(not_node($2, @$)); }
| invocation_expr     { $$ = $1; }

// Not implemented
invocation_expr:
  basic_expr          { $$ = $1; }

basic_expr:
  "identifier" { $$ = USE_VAR($1, @1); }
| literal      { $$ = $1; }
| "(" expr ")" { $$ = $2; }

literal:
  "int"     { $$ = NEW(int_literal_node($1, @$)); }
| "float"   { $$ = NEW(float_literal_node($1, @$)); }
| "boolean" { $$ = NEW(boolean_literal_node($1, @$)); }
| "char"    { $$ = NEW(char_literal_node($1, @$)); }
| "string"  { $$ = NEW(string_literal_node($1, @$)); }
%%

void yy::parser::error (const location_type& l, const std::string& m) {
  std::ostringstream ss;
  ss << l << ": " << m << '\n';
  message_recipient->assign(ss.str());
}
