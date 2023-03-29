#ifdef __EMSCRIPTEN__

#include <emscripten/bind.h>
#include <parser/facade.h>

using namespace emscripten;

EMSCRIPTEN_BINDINGS(ast) {
  class_<ast_node>("AstNode")
      .property("kind", &ast_node::kind)
      .smart_ptr<std::shared_ptr<ast_node>>("shared_ptr<AstNode>");

  class_<parse_result>("ParseResult")
      .property("success", &parse_result::success)
      .property("message", &parse_result::message)
      .property("ast", &parse_result::ast);

  enum_<keyword>("Keyword")
      .value("If", keyword::IF)
      .value("Else", keyword::ELSE)
      .value("While", keyword::WHILE)
      .value("Goto", keyword::GOTO)
      .value("Write", keyword::WRITE)
      .value("Read", keyword::READ)
      .value("True", keyword::TRUE)
      .value("False", keyword::FALSE);

  class_<parser>("Parser")
      .constructor<std::string>()
      .function("parse", &parser::parse)
      .function("setKeyword", &parser::set_keyword);
}

#endif
