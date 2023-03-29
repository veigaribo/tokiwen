#include "parser/lex/scanner.hpp"
#include <bandit/bandit.h>

using namespace snowhouse;
using namespace bandit;

go_bandit([]() {
  describe("scanner", []() {
    it("ignores whitespace", [&]() {
      yy::scanner scanner(" \n ");
      auto token = scanner.yylex();

      AssertThat(token.type_get(),
                 Equals(yy::parser::symbol_kind_type::S_YYEOF));
    });

    it("scans 1 char tokens", [&]() {
      yy::scanner scanner("= + - * / % { } ! ;");

      AssertThat(scanner.yylex().type_get(),
                 Equals(yy::parser::symbol_kind_type::S_ASSIGN));
      AssertThat(scanner.yylex().type_get(),
                 Equals(yy::parser::symbol_kind_type::S_PLUS));
      AssertThat(scanner.yylex().type_get(),
                 Equals(yy::parser::symbol_kind_type::S_MINUS));
      AssertThat(scanner.yylex().type_get(),
                 Equals(yy::parser::symbol_kind_type::S_STAR));
      AssertThat(scanner.yylex().type_get(),
                 Equals(yy::parser::symbol_kind_type::S_SLASH));
      AssertThat(scanner.yylex().type_get(),
                 Equals(yy::parser::symbol_kind_type::S_PERCENT));
      AssertThat(scanner.yylex().type_get(),
                 Equals(yy::parser::symbol_kind_type::S_LCURLY));
      AssertThat(scanner.yylex().type_get(),
                 Equals(yy::parser::symbol_kind_type::S_RCURLY));
      AssertThat(scanner.yylex().type_get(),
                 Equals(yy::parser::symbol_kind_type::S_NOT));
      AssertThat(scanner.yylex().type_get(),
                 Equals(yy::parser::symbol_kind_type::S_SEMI));
      AssertThat(scanner.yylex().type_get(),
                 Equals(yy::parser::symbol_kind_type::S_YYEOF));
    });

    it("scans 2 char tokens", [&]() {
      yy::scanner scanner("+= -= *= /= %= >= <= == != && ||");

      AssertThat(scanner.yylex().type_get(),
                 Equals(yy::parser::symbol_kind_type::S_PLUS_ASSIGN));
      AssertThat(scanner.yylex().type_get(),
                 Equals(yy::parser::symbol_kind_type::S_MINUS_ASSIGN));
      AssertThat(scanner.yylex().type_get(),
                 Equals(yy::parser::symbol_kind_type::S_STAR_ASSIGN));
      AssertThat(scanner.yylex().type_get(),
                 Equals(yy::parser::symbol_kind_type::S_SLASH_ASSIGN));
      AssertThat(scanner.yylex().type_get(),
                 Equals(yy::parser::symbol_kind_type::S_PERCENT_ASSIGN));
      AssertThat(scanner.yylex().type_get(),
                 Equals(yy::parser::symbol_kind_type::S_GTEQ));
      AssertThat(scanner.yylex().type_get(),
                 Equals(yy::parser::symbol_kind_type::S_LTEQ));
      AssertThat(scanner.yylex().type_get(),
                 Equals(yy::parser::symbol_kind_type::S_EQUALS));
      AssertThat(scanner.yylex().type_get(),
                 Equals(yy::parser::symbol_kind_type::S_NEQUALS));
      AssertThat(scanner.yylex().type_get(),
                 Equals(yy::parser::symbol_kind_type::S_AND));
      AssertThat(scanner.yylex().type_get(),
                 Equals(yy::parser::symbol_kind_type::S_OR));
      AssertThat(scanner.yylex().type_get(),
                 Equals(yy::parser::symbol_kind_type::S_YYEOF));
    });

    it("scans identifiers", [&]() {
      yy::scanner scanner("int x1 a_b _p");

      AssertThat(scanner.yylex().type_get(),
                 Equals(yy::parser::symbol_kind_type::S_IDENTIFIER));
      AssertThat(scanner.yylex().type_get(),
                 Equals(yy::parser::symbol_kind_type::S_IDENTIFIER));
      AssertThat(scanner.yylex().type_get(),
                 Equals(yy::parser::symbol_kind_type::S_IDENTIFIER));
      AssertThat(scanner.yylex().type_get(),
                 Equals(yy::parser::symbol_kind_type::S_IDENTIFIER));
      AssertThat(scanner.yylex().type_get(),
                 Equals(yy::parser::symbol_kind_type::S_YYEOF));
    });

    it("scans keywords", [&]() {
      yy::scanner scanner("if else");
      scanner.init_default_keywords();

      AssertThat(scanner.yylex().type_get(),
                 Equals(yy::parser::symbol_kind_type::S_IF));
      AssertThat(scanner.yylex().type_get(),
                 Equals(yy::parser::symbol_kind_type::S_ELSE));
      AssertThat(scanner.yylex().type_get(),
                 Equals(yy::parser::symbol_kind_type::S_YYEOF));
    });

    // Token values are not tested here because there may be a non-terminal
    // symbol with the same name and make_TOKEN will return the non-terminal
    // instead of the terminal, so the value type may be different and that
    // would be a mess.
    it("scans literal tokens: integer", [&]() {
      yy::scanner scanner("12321 98789");

      AssertThat(scanner.yylex().type_get(),
                 Equals(yy::parser::symbol_kind_type::S_INT_LITERAL));
      AssertThat(scanner.yylex().type_get(),
                 Equals(yy::parser::symbol_kind_type::S_INT_LITERAL));
      AssertThat(scanner.yylex().type_get(),
                 Equals(yy::parser::symbol_kind_type::S_YYEOF));
    });

    it("scans literal tokens: float", [&]() {
      yy::scanner scanner("2.71828 7.3e-3 1e1");

      AssertThat(scanner.yylex().type_get(),
                 Equals(yy::parser::symbol_kind_type::S_FLOAT_LITERAL));
      AssertThat(scanner.yylex().type_get(),
                 Equals(yy::parser::symbol_kind_type::S_FLOAT_LITERAL));
      AssertThat(scanner.yylex().type_get(),
                 Equals(yy::parser::symbol_kind_type::S_FLOAT_LITERAL));
      AssertThat(scanner.yylex().type_get(),
                 Equals(yy::parser::symbol_kind_type::S_YYEOF));
    });

    it("scans literal tokens: boolean", [&]() {
      yy::scanner scanner("false true");
      scanner.init_default_keywords();

      AssertThat(scanner.yylex().type_get(),
                 Equals(yy::parser::symbol_kind_type::S_BOOLEAN_LITERAL));
      AssertThat(scanner.yylex().type_get(),
                 Equals(yy::parser::symbol_kind_type::S_BOOLEAN_LITERAL));
      AssertThat(scanner.yylex().type_get(),
                 Equals(yy::parser::symbol_kind_type::S_YYEOF));
    });

    it("scans literal tokens: char", [&]() {
      yy::scanner scanner("'a' '0' '#'");

      AssertThat(scanner.yylex().type_get(),
                 Equals(yy::parser::symbol_kind_type::S_CHAR_LITERAL));
      AssertThat(scanner.yylex().type_get(),
                 Equals(yy::parser::symbol_kind_type::S_CHAR_LITERAL));
      AssertThat(scanner.yylex().type_get(),
                 Equals(yy::parser::symbol_kind_type::S_CHAR_LITERAL));
      AssertThat(scanner.yylex().type_get(),
                 Equals(yy::parser::symbol_kind_type::S_YYEOF));
    });

    it("scans literal tokens: string", [&]() {
      yy::scanner scanner("\"pumpkin\" \"!\\n@\\t#\"");

      AssertThat(scanner.yylex().type_get(),
                 Equals(yy::parser::symbol_kind_type::S_STRING_LITERAL));
      AssertThat(scanner.yylex().type_get(),
                 Equals(yy::parser::symbol_kind_type::S_STRING_LITERAL));
      AssertThat(scanner.yylex().type_get(),
                 Equals(yy::parser::symbol_kind_type::S_YYEOF));
    });
  });
});
