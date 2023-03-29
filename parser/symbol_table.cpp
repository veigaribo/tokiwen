#include "parser/symbol_table.h"

std::ostream &operator<<(std::ostream &o, const type_table_entry &a) {
  return o << "[TYPE: " << a.name << "@" << a.declared_at << "]";
}

std::ostream &operator<<(std::ostream &o, const var_table_entry &a) {
  return o << "[VAR: " << a.name << "@" << a.declared_at << ", type " << *a.type
           << "]";
}

symbol_table::symbol_table() : parent(std::nullopt), offset_counter(0) {}
symbol_table::symbol_table(std::shared_ptr<symbol_table> parent)
    : parent(parent) {
  parent->add_child(this);
}

void symbol_table::add_child(symbol_table *child) {
  this->children.push_back(child);
}

std::vector<symbol_table *> &symbol_table::get_children() {
  return this->children;
}

size_t symbol_table::size() { return locals.size() + types.size(); }

std::shared_ptr<variable_map> symbol_table::vars() {
  auto new_map = std::make_shared<variable_map>();
  new_map->insert(this->locals.begin(), this->locals.end());
  return new_map;
}

std::optional<var_table_entry *>
symbol_table::insert_var(std::string name, yy::location loc,
                         type_table_entry *type, variable_map *map) {
  // Check for redeclaration of the same symbol
  // Local variables and parameters share the same namespace
  auto maybe_local = this->locals.find(name);

  if (maybe_local != map->end()) {
    return std::nullopt;
  }

  var_table_entry entry;
  entry.name = name;
  entry.declared_at = loc;
  entry.type = type;
  entry.offset = get_offset();

  inc_offset(type->value->size());

  (*map)[name] = entry;
  return &(*map)[name];
}

std::optional<var_table_entry *>
symbol_table::insert_variable(std::string name, yy::location loc,
                              type_table_entry *type) {
  return insert_var(name, loc, type, &this->locals);
}

std::optional<type_table_entry *>
symbol_table::insert_type(std::string name, yy::location loc,
                          std::shared_ptr<type> value) {
  // Check for redeclaration of the same symbol in the same context
  auto maybe_found = this->types.find(name);

  if (maybe_found != this->types.end()) {
    return std::nullopt;
  }

  type_table_entry entry;
  entry.name = name;
  entry.declared_at = loc;
  entry.value = value;

  this->types[name] = entry;
  return &this->types[name];
}

std::optional<var_table_entry *> symbol_table::get_var(std::string name,
                                                       variable_map *map) {
  auto maybe_found = map->find(name);

  if (maybe_found != map->end()) {
    return &maybe_found->second;
  }

  return std::nullopt;
}

std::optional<var_table_entry *> symbol_table::get_var(std::string name) {
  auto var = get_var(name, &this->locals);

  if (var.has_value()) {
    return var;
  }

  if (parent.has_value()) {
    return parent.value()->get_var(name);
  }

  return std::nullopt;
}

std::optional<type_table_entry *> symbol_table::get_type(std::string name) {
  auto maybe_found = this->types.find(name);

  if (maybe_found != this->types.end()) {
    return &maybe_found->second;
  }

  if (parent.has_value()) {
    return parent.value()->get_type(name);
  }

  return std::nullopt;
}

symbol_table *symbol_table::get_root() {
  auto current = this;
  auto next = current->parent;

  while (next.has_value()) {
    current = next.value().get();
    next = current->parent;
  }

  return current;
}

std::string default_filename = "prelude";

yy::location symbol_table::default_location() {
  return yy::location(&default_filename, 0, 0);
}

std::uint64_t symbol_table::get_offset() {
  return this->get_root()->offset_counter;
}

void symbol_table::inc_offset(std::uint64_t amount) {
  this->get_root()->offset_counter += amount;
}

std::optional<var_table_entry *>
symbol_table::insert_default_var(std::string name, yy::location loc,
                                 type_table_entry *type) {
  auto root = get_root();
  auto entry = root->insert_variable(name, loc, type);
  root->default_vars[name] = *entry.value();
  return entry;
}

std::optional<type_table_entry *>
symbol_table::insert_default_type(std::string name, yy::location loc,
                                  std::shared_ptr<type> value) {
  auto root = get_root();
  auto entry = root->insert_type(name, loc, value);
  root->default_types[name] = *entry.value();
  return entry;
}

void symbol_table::init_default_symbols() {
  auto root = get_root();
  root->insert_default_type("int", default_location(),
                            std::make_shared<type_int>());
  root->insert_default_type("float", default_location(),
                            std::make_shared<type_float>());
  root->insert_default_type("boolean", default_location(),
                            std::make_shared<type_boolean>());
  root->insert_default_type("char", default_location(),
                            std::make_shared<type_char>());
  root->insert_default_type("function", default_location(),
                            std::make_shared<type_function>());
  root->insert_default_type("void", default_location(),
                            std::make_shared<type_void>());
}

var_table_entry *symbol_table::get_default_var(std::string name) {
  auto root = get_root();
  auto maybe_found = root->default_vars.find(name);

  if (maybe_found != root->default_vars.end()) {
    return &maybe_found->second;
  }

  throw std::runtime_error("No default variable named " + name);
}

type_table_entry *symbol_table::get_default_type(std::string name) {
  auto root = get_root();
  auto maybe_found = root->default_types.find(name);

  if (maybe_found != root->default_types.end()) {
    return &maybe_found->second;
  }

  throw std::runtime_error("No default type named " + name);
}

std::ostream &operator<<(std::ostream &o, const symbol_table &a) {
  o << "{TYPES: ";

  auto types_it = a.types.begin();
  auto types_end = a.types.end();

  if (types_it != types_end) {
    auto [_, val] = *types_it;
    o << val;
    ++types_it;
  }

  for (; types_it != types_end; ++types_it) {
    auto [_, val] = *types_it;
    o << ", " << val;
  }

  o << " | LOCALS: ";

  auto locals_it = a.locals.begin();
  auto locals_end = a.locals.end();

  if (locals_it != locals_end) {
    auto [_, val] = *locals_it;
    o << val;
    ++locals_it;
  }

  for (; locals_it != locals_end; ++locals_it) {
    auto [_, val] = *locals_it;
    o << ", " << val;
  }

  return o << "}";
}
