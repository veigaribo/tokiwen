#include "parser/lex/scanner.hpp"
#include "parser/syntax/parser.hpp"

struct parse_result {
  bool success;
  std::string message;
  std::shared_ptr<ast_node> ast;
};

class parser {
private:
  std::string input;
  std::shared_ptr<yy::parser> y;
  yy::scanner scanner;
  symbol_table_stack stbuilder;
  std::shared_ptr<ast_node> ast;
  std::string message_recipient;

public:
  parser(std::string input);

  void set_keyword(keyword kw, std::string value);
  void debug(int level);
  parse_result parse();
};
