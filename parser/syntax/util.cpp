#include "parser/syntax/util.h"
#include "common/strings.h"
#include "parser/ast.h"
#include "parser/syntax/parser.hpp"

std::shared_ptr<ast_node> declare_var(std::shared_ptr<symbol_table> table,
                                      type_table_entry *type, std::string name,
                                      yy::location loc) {
  auto type_node = std::make_shared<type_identifier_node>(type, loc);
  auto maybe_entry = table->insert_variable(name, loc, type_node->entry);

  if (!maybe_entry.has_value()) {
    throw yy::parser::syntax_error(
        loc, "Variable `" + name + "` already declared in this context.");
  }

  auto entry = maybe_entry.value();
  auto name_node = std::make_shared<var_identifier_node>(entry, loc);
  return std::make_shared<declaration_node>(type_node, name_node, loc);
}

std::shared_ptr<ast_node> declare_var(std::shared_ptr<symbol_table> table,
                                      std::string type, std::string name,
                                      yy::location loc) {
  auto type_entry = get_type(table, type, loc);
  return declare_var(table, type_entry, name, loc);
}

std::shared_ptr<ast_node>
declare_assign_var(std::shared_ptr<symbol_table> table, std::string type,
                   std::string name, std::shared_ptr<ast_node> value,
                   yy::location loc) {
  auto type_ast_node = use_type(table, type, loc);
  auto type_node =
      std::dynamic_pointer_cast<type_identifier_node>(type_ast_node);
  auto maybe_entry = table->insert_variable(name, loc, type_node->entry);

  if (!maybe_entry.has_value()) {
    throw yy::parser::syntax_error(
        loc, "Variable `" + name + "` already declared in this context.");
  }

  auto entry = maybe_entry.value();
  auto name_node = std::make_shared<var_identifier_node>(entry, loc);
  return std::make_shared<declaration_assignment_node>(type_node, name_node,
                                                       value, loc);
}

type_table_entry *get_type(std::shared_ptr<symbol_table> table,
                           std::string name, yy::location loc) {
  auto maybe_entry = table->get_type(name);

  if (!maybe_entry.has_value()) {
    throw yy::parser::syntax_error(loc, "Type `" + name + "` not found.");
  }

  return maybe_entry.value();
}

var_table_entry *get_var(std::shared_ptr<symbol_table> table, std::string name,
                         yy::location loc) {
  auto maybe_entry = table->get_var(name);

  if (!maybe_entry.has_value()) {
    throw yy::parser::syntax_error(loc, "Variable `" + name + "` not found.");
  }

  return maybe_entry.value();
}

std::shared_ptr<ast_node> use_var(std::shared_ptr<symbol_table> table,
                                  std::string name, yy::location loc) {
  auto entry = get_var(table, name, loc);
  return std::make_shared<var_identifier_node>(entry, loc);
}

std::shared_ptr<ast_node> use_type(std::shared_ptr<symbol_table> table,
                                   std::string name, yy::location loc) {
  auto entry = get_type(table, name, loc);
  return std::make_shared<type_identifier_node>(entry, loc);
}
