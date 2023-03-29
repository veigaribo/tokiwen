#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include "cinttypes"
#include "parser/syntax/location.hpp"
#include "parser/types.h"
#include <map>
#include <memory>
#include <optional>
#include <vector>

class ast_node;

struct type_table_entry {
  std::string name;
  yy::location declared_at;
  std::shared_ptr<type> value;

  friend std::ostream &operator<<(std::ostream &o, const type_table_entry &a);
};

struct var_table_entry;

struct function_data {
  std::shared_ptr<type> return_type;
  std::shared_ptr<ast_node> body;
  std::vector<var_table_entry *> parameters;
};

struct var_table_entry {
  std::string name;
  yy::location declared_at;
  type_table_entry *type;
  std::shared_ptr<function_data> fn_data;
  std::uint64_t offset;

  friend std::ostream &operator<<(std::ostream &o, const var_table_entry &a);
};

typedef std::map<std::string, var_table_entry> variable_map;
typedef std::map<std::string, type_table_entry> type_map;

class symbol_table {
private:
  std::uint64_t offset_counter;

  variable_map locals;
  type_map types;

  std::optional<std::shared_ptr<symbol_table>> parent;
  std::vector<symbol_table *> children;

  variable_map default_vars;
  type_map default_types;

  symbol_table *get_root();
  yy::location default_location();

  std::uint64_t get_offset();
  void inc_offset(std::uint64_t amount);

  std::optional<var_table_entry *> insert_default_var(std::string name,
                                                      yy::location loc,
                                                      type_table_entry *type);

  std::optional<type_table_entry *>
  insert_default_type(std::string name, yy::location loc,
                      std::shared_ptr<type> value);

  std::optional<var_table_entry *> insert_var(std::string name,
                                              yy::location loc,
                                              type_table_entry *type,
                                              variable_map *map);

  std::optional<var_table_entry *> get_var(std::string name, variable_map *map);

public:
  symbol_table();
  symbol_table(std::shared_ptr<symbol_table> parent);

  void add_child(symbol_table *child);
  std::vector<symbol_table *> &get_children();

  void init_default_symbols();
  size_t size();

  var_table_entry *get_default_var(std::string name);
  type_table_entry *get_default_type(std::string name);

  std::shared_ptr<variable_map> vars();

  // Empty if already exists
  std::optional<var_table_entry *>
  insert_variable(std::string name, yy::location loc, type_table_entry *type);

  std::optional<type_table_entry *>
  insert_type(std::string name, yy::location loc, std::shared_ptr<type> value);

  // Empty if does not exist
  std::optional<var_table_entry *> get_var(std::string name);
  std::optional<type_table_entry *> get_type(std::string name);

  friend std::ostream &operator<<(std::ostream &o, const symbol_table &a);
};

#endif /* SYMBOL_TABLE_H */
