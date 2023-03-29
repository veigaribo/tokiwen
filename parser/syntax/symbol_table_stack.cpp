#include "parser/syntax/symbol_table_stack.h"

symbol_table_stack::symbol_table_stack() {
  auto root_table = std::make_shared<symbol_table>();
  root_table->init_default_symbols();
  tables.push_back(root_table);
}

std::shared_ptr<symbol_table> symbol_table_stack::current() {
  return tables.back();
}

void symbol_table_stack::push() {
  auto new_table = std::make_shared<symbol_table>(current());
  tables.push_back(new_table);
}

std::shared_ptr<symbol_table> symbol_table_stack::pop() {
  auto back = tables.back();
  tables.pop_back();
  return back;
}

void symbol_table_stack::push_parameter(std::string name, yy::location loc,
                                        type_table_entry *type) {
  this->parameters.push_back(param{name, loc, type});
}

symbol_table_stack::~symbol_table_stack() {}
