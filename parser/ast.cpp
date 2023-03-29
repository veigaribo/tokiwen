#include "ast.h"

#define X(Enum, Name) Name,
char const *ast_kind_names[] = {AST_KINDS};
#undef X

std::ostream &operator<<(std::ostream &o, const ast_node_kind &a) {
  return o << static_cast<std::underlying_type_t<ast_node_kind>>(a);
}

std::string name_of_ast_node_kind(ast_node_kind kind) {
  return ast_kind_names[(size_t)kind];
}

ast_node::ast_node(ast_node_kind kind, std::shared_ptr<type> typ,
                   yy::location location)
    : kind(kind), typ(typ), location(location) {}

ast_node::ast_node(ast_node_kind kind, yy::location location)
    : kind(kind), typ(std::make_shared<type_void>()), location(location) {}

std::ostream &ast_node::extract(std::ostream &o) const {
  o << ast_kind_names[(size_t)this->kind];

  if (!std::make_shared<type_void>()->matches(*this->typ)) {
    o << "[" << *this->typ << "]";
  }

  o << "(";

  if (this->children.size() == 0) {
    o << ")";
    return o;
  }

  auto first = this->children[0];
  o << *first;

  for (auto child = this->children.begin() + 1; child < this->children.end();
       ++child) {
    o << ", " << **child;
  }

  o << ")";
  return o;
}

bool ast_node::equals(const ast_node &other) const {
  if (this->kind != other.kind) {
    return false;
  }

  if (this->typ->kind != other.typ->kind) {
    return false;
  }

  if (this->children.size() != other.children.size()) {
    return false;
  }

  for (size_t i = 0; i < this->children.size(); ++i) {
    auto this_child = *this->children[i];
    auto that_child = *other.children[i];

    if (this_child != that_child) {
      return false;
    }
  }

  return true;
}

bool ast_node::operator==(const ast_node &other) const { return equals(other); }

// `extract` is virtual so the correct implementatian will be chosen at runtime
std::ostream &operator<<(std::ostream &o, const ast_node &a) {
  return a.extract(o);
}

#define AST_NODE_IMPL_EXPR_LEAF(Name, Kind, Type, Member1Type, Member1Name)    \
  Name::Name(Member1Type Member1Name, yy::location location)                   \
      : Member1Name(Member1Name),                                              \
        ast_node(ast_node_kind::Kind, Type, location) {}

#define AST_NODE_IMPL_STMT_LEAF(Name, Kind, Member1Type, Member1Name)          \
  Name::Name(Member1Type Member1Name, yy::location location)                   \
      : Member1Name(Member1Name), ast_node(ast_node_kind::Kind, location) {}

#define AST_NODE_IMPL_EXTRACT(Name, Extract)                                   \
  std::ostream &Name::extract(std::ostream &o) const { return o << Extract; }

#define AST_NODE_IMPL_EQUALS(Name, Comparison)                                 \
  bool Name::equals(const ast_node &other1) const {                            \
    if (other1.kind != this->kind) {                                           \
      return false;                                                            \
    }                                                                          \
    auto other = (Name &)other1;                                               \
    return Comparison;                                                         \
  }

#define AST_NODE_IMPL_EXPR_1(Name, Kind, Type)                                 \
  Name::Name(std::shared_ptr<ast_node> child1, yy::location location)          \
      : ast_node(ast_node_kind::Kind, location) {                              \
    this->children.push_back(child1);                                          \
    this->typ = Type;                                                          \
  }

#define AST_NODE_IMPL_STMT_1(Name, Kind)                                       \
  Name::Name(std::shared_ptr<ast_node> child1, yy::location location)          \
      : ast_node(ast_node_kind::Kind, location) {                              \
    this->children.push_back(child1);                                          \
  }

#define AST_NODE_IMPL_EXPR_2(Name, Kind, Type)                                 \
  Name::Name(std::shared_ptr<ast_node> child1,                                 \
             std::shared_ptr<ast_node> child2, yy::location location)          \
      : ast_node(ast_node_kind::Kind, location) {                              \
    this->children.push_back(child1);                                          \
    this->children.push_back(child2);                                          \
    this->typ = Type;                                                          \
  }

#define AST_NODE_IMPL_STMT_2(Name, Kind)                                       \
  Name::Name(std::shared_ptr<ast_node> child1,                                 \
             std::shared_ptr<ast_node> child2, yy::location location)          \
      : ast_node(ast_node_kind::Kind, location) {                              \
    this->children.push_back(child1);                                          \
    this->children.push_back(child2);                                          \
  }

#define AST_NODE_IMPL_STMT_3(Name, Kind)                                       \
  Name::Name(std::shared_ptr<ast_node> child1,                                 \
             std::shared_ptr<ast_node> child2,                                 \
             std::shared_ptr<ast_node> child3, yy::location location)          \
      : ast_node(ast_node_kind::Kind, location) {                              \
    this->children.push_back(child1);                                          \
    this->children.push_back(child2);                                          \
    this->children.push_back(child3);                                          \
  }

noop_node::noop_node(yy::location location)
    : ast_node(ast_node_kind::NOOP, std::make_shared<type_void>(), location) {}

AST_NODE_IMPL_EXPR_LEAF(var_identifier_node, VAR_IDENTIFIER, entry->type->value,
                        var_table_entry *, entry);

AST_NODE_IMPL_EXTRACT(var_identifier_node, "VAR_IDENTIFIER["
                                               << *typ << "]("
                                               << "var: " << entry->name << "@"
                                               << entry->declared_at << ")");

// Checks only name & kind, which could be innacurate, but is
// convenient for tests, which is mostly what this is for.
AST_NODE_IMPL_EQUALS(var_identifier_node, entry->name == other.entry->name);

AST_NODE_IMPL_STMT_LEAF(type_identifier_node, TYPE_IDENTIFIER,
                        type_table_entry *, entry);

AST_NODE_IMPL_EXTRACT(type_identifier_node, "TYPE_IDENTIFIER("
                                                << "type: " << entry->name
                                                << "@" << entry->declared_at
                                                << ")");

AST_NODE_IMPL_EQUALS(type_identifier_node, entry->name == other.entry->name);

AST_NODE_IMPL_EXPR_LEAF(int_literal_node, INT_LITERAL,
                        std::make_shared<type_int>(), std::int64_t, value);
AST_NODE_IMPL_EXTRACT(int_literal_node,
                      "INT_LITERAL[" << *typ << "](" << value << ")");
AST_NODE_IMPL_EQUALS(int_literal_node, value == other.value);

AST_NODE_IMPL_EXPR_LEAF(float_literal_node, FLOAT_LITERAL,
                        std::make_shared<type_float>(), double, value);
AST_NODE_IMPL_EXTRACT(float_literal_node,
                      "FLOAT_LITERAL[" << *typ << "](" << value << ")");
AST_NODE_IMPL_EQUALS(float_literal_node, value == other.value);

AST_NODE_IMPL_EXPR_LEAF(boolean_literal_node, BOOLEAN_LITERAL,
                        std::make_shared<type_boolean>(), bool, value);
AST_NODE_IMPL_EXTRACT(boolean_literal_node,
                      "BOOLEAN_LITERAL[" << *typ << "](" << value << ")");
AST_NODE_IMPL_EQUALS(boolean_literal_node, value == other.value);

AST_NODE_IMPL_EXPR_LEAF(char_literal_node, CHAR_LITERAL,
                        std::make_shared<type_char>(), char, value);
AST_NODE_IMPL_EXTRACT(char_literal_node,
                      "CHAR_LITERAL[" << *typ << "](" << value << ")");
AST_NODE_IMPL_EQUALS(char_literal_node, value == other.value);

AST_NODE_IMPL_EXPR_LEAF(
    string_literal_node, STRING_LITERAL,
    std::make_shared<type_pointer>(std::make_shared<type_char>()), std::string,
    value);
AST_NODE_IMPL_EXTRACT(string_literal_node,
                      "CHAR_LITERAL[" << *typ << "](" << value << ")");
AST_NODE_IMPL_EQUALS(string_literal_node, value == other.value);

AST_NODE_IMPL_EXPR_1(int_to_float_coercion_node, INT_TO_FLOAT_COERCION,
                     std::make_shared<type_float>());

AST_NODE_IMPL_EXPR_1(int_to_boolean_coercion_node, INT_TO_BOOLEAN_COERCION,
                     std::make_shared<type_boolean>());

AST_NODE_IMPL_EXPR_1(boolean_to_int_coercion_node, BOOLEAN_TO_INT_COERCION,
                     std::make_shared<type_int>());

AST_NODE_IMPL_EXPR_1(pointer_to_boolean_coercion_node,
                     POINTER_TO_BOOLEAN_COERCION,
                     std::make_shared<type_boolean>());

std::shared_ptr<type>
determine_unary_op_type_arithmetic(std::shared_ptr<ast_node> operand) {
  return operand->typ;
}

std::shared_ptr<type> determine_unary_op_type_integral_then_boolean(
    std::shared_ptr<ast_node> operand) {
  return std::make_shared<type_boolean>();
}

AST_NODE_IMPL_EXPR_1(unary_minus_node, UNARY_MINUS,
                     determine_unary_op_type_arithmetic(child1));
AST_NODE_IMPL_EXPR_1(unary_plus_node, UNARY_PLUS,
                     determine_unary_op_type_arithmetic(child1));

AST_NODE_IMPL_EXPR_1(not_node, NOT,
                     determine_unary_op_type_integral_then_boolean(child1));

block_node::block_node(std::shared_ptr<symbol_table> table,
                       std::shared_ptr<ast_node> body, yy::location location)
    : table(table), ast_node(ast_node_kind::BLOCK, location) {
  this->children.push_back(body);
}

std::ostream &block_node::extract(std::ostream &o) const {
  return o << "BLOCK("
           << "<" << table->size() << " SYMBOLS>"
           << ", " << *this->children[0] << ")";
}

bool block_node::equals(const ast_node &other) const {
  // Does not check the symbol table, again for convenience.
  return *this->children[0] == *other.children[0];
}

std::shared_ptr<type>
determine_bin_op_type_arithmetic(std::shared_ptr<ast_node> left,
                                 std::shared_ptr<ast_node> right) {
  return left->typ;
}

std::shared_ptr<type>
determine_bin_op_type_integral(std::shared_ptr<ast_node> left,
                               std::shared_ptr<ast_node> right) {
  return left->typ;
}

AST_NODE_IMPL_EXPR_2(sum_node, SUM,
                     determine_bin_op_type_arithmetic(child1, child2));
AST_NODE_IMPL_EXPR_2(subtraction_node, SUBTRACTION,
                     determine_bin_op_type_arithmetic(child1, child2));
AST_NODE_IMPL_EXPR_2(multiplication_node, MULTIPLICATION,
                     determine_bin_op_type_arithmetic(child1, child2));
AST_NODE_IMPL_EXPR_2(division_node, DIVISION,
                     determine_bin_op_type_arithmetic(child1, child2));
AST_NODE_IMPL_EXPR_2(modulo_node, MODULO,
                     determine_bin_op_type_integral(child1, child2));

AST_NODE_IMPL_EXPR_2(lt_node, LT, std::make_shared<type_boolean>());
AST_NODE_IMPL_EXPR_2(gt_node, GT, std::make_shared<type_boolean>());
AST_NODE_IMPL_EXPR_2(lteq_node, LTEQ, std::make_shared<type_boolean>());
AST_NODE_IMPL_EXPR_2(gteq_node, GTEQ, std::make_shared<type_boolean>());
AST_NODE_IMPL_EXPR_2(equals_node, EQUALS, std::make_shared<type_boolean>());
AST_NODE_IMPL_EXPR_2(nequals_node, NEQUALS, std::make_shared<type_boolean>());
AST_NODE_IMPL_EXPR_2(and_node, AND, std::make_shared<type_boolean>());
AST_NODE_IMPL_EXPR_2(or_node, OR, std::make_shared<type_boolean>());

AST_NODE_IMPL_EXPR_2(assignment_node, ASSIGNMENT, child2->typ);
AST_NODE_IMPL_EXPR_2(sum_assignment_node, SUM_ASSIGNMENT, child2->typ);
AST_NODE_IMPL_EXPR_2(subtraction_assignment_node, SUBTRACTION_ASSIGNMENT,
                     child2->typ);
AST_NODE_IMPL_EXPR_2(multiplication_assignment_node, MULTIPLICATION_ASSIGNMENT,
                     child2->typ);
AST_NODE_IMPL_EXPR_2(division_assignment_node, DIVISION_ASSIGNMENT,
                     child2->typ);
AST_NODE_IMPL_EXPR_2(modulo_assignment_node, MODULO_ASSIGNMENT, child2->typ);

AST_NODE_IMPL_STMT_2(sequence_node, SEQUENCE);

AST_NODE_IMPL_STMT_2(declaration_node, DECLARATION);
AST_NODE_IMPL_STMT_3(declaration_assignment_node, DECLARATION_ASSIGNMENT);

AST_NODE_IMPL_STMT_1(statement_node, STATEMENT);

AST_NODE_IMPL_STMT_3(conditional_node, CONDITIONAL);
AST_NODE_IMPL_STMT_2(while_loop_node, WHILE);

AST_NODE_IMPL_STMT_LEAF(label_node, LABEL, std::string, value);
AST_NODE_IMPL_EXTRACT(label_node, "LABEL(" << this->value << ")");
AST_NODE_IMPL_EQUALS(label_node, this->value == other.value);

AST_NODE_IMPL_STMT_LEAF(goto_node, GOTO, std::string, value);
AST_NODE_IMPL_EXTRACT(goto_node, "GOTO(" << this->value << ")");
AST_NODE_IMPL_EQUALS(goto_node, this->value == other.value);

AST_NODE_IMPL_STMT_1(write_node, WRITE);
AST_NODE_IMPL_STMT_1(read_node, READ);
