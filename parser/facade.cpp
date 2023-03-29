#include "parser/facade.h"

parser::parser(std::string input) : input(input) {
  this->scanner = yy::scanner(this->input);
  this->scanner.init_default_keywords();
  this->stbuilder = symbol_table_stack();
  this->y = std::make_shared<yy::parser>(scanner, stbuilder, &this->ast,
                                         &this->message_recipient);

  // test
  // auto t1 = this->scanner.yylex();
  // std::cout << t1.name() << '\n';
}

void parser::set_keyword(keyword kw, std::string value) {
  this->scanner.keyword_map[value] = kw;
}

void parser::debug(int level) {
  // test
  // auto t1 = this->scanner.yylex();
  // std::cout << t1.name() << '\n';

  this->y->set_debug_level(level);
}

parse_result parser::parse() {
  // test
  // auto t1 = this->scanner.yylex();
  // std::cout << t1.name() << '\n';

  parse_result result;
  auto status = this->y->parse();

  result.success = status == 0;
  result.ast = this->ast;
  result.message = this->message_recipient;

  return result;
}
