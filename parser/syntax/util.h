#ifndef UTIL_H
#define UTIL_H

#include "parser/syntax/symbol_table_stack.h"
#include <memory>

// To keep the grammar file mostly clean

std::shared_ptr<ast_node> declare_var(std::shared_ptr<symbol_table> table,
                                      std::string type, std::string name,
                                      yy::location loc);
std::shared_ptr<ast_node>
declare_assign_var(std::shared_ptr<symbol_table> table, std::string type,
                   std::string name, std::shared_ptr<ast_node> value,
                   yy::location loc);

std::shared_ptr<ast_node> declare_struct(std::shared_ptr<symbol_table> table,
                                         std::string name, yy::location loc);
std::shared_ptr<ast_node> declare_typedef(std::shared_ptr<symbol_table> table,
                                          std::string name,
                                          std::string original,
                                          yy::location loc);

type_table_entry *get_type(std::shared_ptr<symbol_table> table,
                           std::string name, yy::location loc);
var_table_entry *get_var(std::shared_ptr<symbol_table> table, std::string name,
                         yy::location loc);

std::shared_ptr<ast_node> use_var(std::shared_ptr<symbol_table> table,
                                  std::string name, yy::location loc);
std::shared_ptr<ast_node> use_type(std::shared_ptr<symbol_table> table,
                                   std::string name, yy::location loc);

// Set the parameters on the stack so that the next block will contain them
std::shared_ptr<ast_node> set_parameters(symbol_table_stack &stack,
                                         std::shared_ptr<ast_node> list);

std::shared_ptr<ast_node>
declare_function(std::shared_ptr<symbol_table> table, std::string return_type,
                 std::string name, std::shared_ptr<ast_node> parameters,
                 std::shared_ptr<ast_node> body, yy::location loc);

std::shared_ptr<ast_node> invoke_function(std::shared_ptr<symbol_table> table,
                                          std::string name,
                                          std::shared_ptr<ast_node> arguments,
                                          yy::location loc);

#endif /* UTIL_H */
