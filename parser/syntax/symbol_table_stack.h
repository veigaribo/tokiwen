#ifndef SYMBOL_TABLE_STACK_H
#define SYMBOL_TABLE_STACK_H

#include "parser/symbol_table.h"
#include <list>
#include <memory>

struct param {
  std::string name;
  yy::location loc;
  type_table_entry *type;
};

class symbol_table_stack {
private:
  std::list<std::shared_ptr<symbol_table>> tables;
  std::list<param> parameters;

public:
  symbol_table_stack();
  ~symbol_table_stack();

  std::shared_ptr<symbol_table> current();
  void push();
  std::shared_ptr<symbol_table> pop();

  // Will be added to the next symbol table created
  void push_parameter(std::string name, yy::location loc,
                      type_table_entry *type);
};

#endif /* SYMBOL_TABLE_STACK_H */
