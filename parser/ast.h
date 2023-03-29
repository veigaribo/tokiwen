#ifndef AST_H
#define AST_H

#include "parser/symbol_table.h"
#include "parser/types.h"
#include <cstdint>
#include <iostream>
#include <memory>

#define AST_KINDS                                                              \
  X(VAR_IDENTIFIER, "VAR_IDENTIFIER")                                          \
  X(TYPE_IDENTIFIER, "TYPE_IDENTIFIER")                                        \
                                                                               \
  X(INT_LITERAL, "INT_LITERAL")                                                \
  X(FLOAT_LITERAL, "FLOAT_LITERAL")                                            \
  X(BOOLEAN_LITERAL, "BOOLEAN_LITERAL")                                        \
  X(CHAR_LITERAL, "CHAR_LITERAL")                                              \
  X(STRING_LITERAL, "STRING_LITERAL")                                          \
                                                                               \
  X(INT_TO_FLOAT_COERCION, "INT_TO_FLOAT_COERCION")                            \
  X(INT_TO_BOOLEAN_COERCION, "INT_TO_BOOLEAN_COERCION")                        \
  X(BOOLEAN_TO_INT_COERCION, "BOOLEAN_TO_INT_COERCION")                        \
  X(POINTER_TO_BOOLEAN_COERCION, "POINTER_TO_BOOLEAN_COERCION")                \
                                                                               \
  X(UNARY_MINUS, "UNARY_MINUS")                                                \
  X(UNARY_PLUS, "UNARY_PLUS")                                                  \
                                                                               \
  X(SUM, "SUM")                                                                \
  X(SUBTRACTION, "SUBTRACTION")                                                \
  X(MULTIPLICATION, "MULTIPLICATION")                                          \
  X(DIVISION, "DIVISION")                                                      \
  X(MODULO, "MODULO")                                                          \
                                                                               \
  X(DECLARATION, "DECLARATION")                                                \
  X(ASSIGNMENT, "ASSIGNMENT")                                                  \
  X(DECLARATION_ASSIGNMENT, "DECLARATION_ASSIGNMENT")                          \
                                                                               \
  X(SUM_ASSIGNMENT, "SUM_ASSIGNMENT")                                          \
  X(SUBTRACTION_ASSIGNMENT, "SUBTRACTION_ASSIGNMENT")                          \
  X(MULTIPLICATION_ASSIGNMENT, "MULTIPLICATION_ASSIGNMENT")                    \
  X(DIVISION_ASSIGNMENT, "DIVISION_ASSIGNMENT")                                \
  X(MODULO_ASSIGNMENT, "MODULO_ASSIGNMENT")                                    \
                                                                               \
  X(LT, "LT")                                                                  \
  X(GT, "GT")                                                                  \
  X(LTEQ, "LTEQ")                                                              \
  X(GTEQ, "GTEQ")                                                              \
  X(EQUALS, "EQUALS")                                                          \
  X(NEQUALS, "NEQUALS")                                                        \
                                                                               \
  X(AND, "AND")                                                                \
  X(OR, "OR")                                                                  \
  X(NOT, "NOT")                                                                \
                                                                               \
  X(BLOCK, "BLOCK")                                                            \
  X(SEQUENCE, "SEQUENCE")                                                      \
  X(NOOP, "NOOP")                                                              \
                                                                               \
  X(STATEMENT, "STATEMENT")                                                    \
  X(CONDITIONAL, "CONDITIONAL")                                                \
  X(WHILE, "WHILE")                                                            \
  X(RETURN, "RETURN")                                                          \
  X(LABEL, "LABEL")                                                            \
  X(GOTO, "GOTO")                                                              \
  X(WRITE, "WRITE")                                                            \
  X(READ, "READ")

#define X(Enum, Name) Enum,
enum class ast_node_kind { AST_KINDS };
#undef X

std::string name_of_ast_node_kind(ast_node_kind kind);

std::ostream &operator<<(std::ostream &o, const ast_node_kind &a);

class ast_node {
private:
  virtual std::ostream &extract(std::ostream &o) const;
  virtual bool equals(const ast_node &other) const;

public:
  const ast_node_kind kind;
  std::shared_ptr<type> typ;
  std::vector<std::shared_ptr<ast_node>> children;
  yy::location location;

  ast_node(ast_node_kind kind, std::shared_ptr<type> typ,
           yy::location location);
  ast_node(ast_node_kind kind, yy::location location);

  bool operator==(const ast_node &other) const;
  friend std::ostream &operator<<(std::ostream &o, const ast_node &a);
};

#define AST_NODE_LEAF(Name, Member)                                            \
  class Name : public ast_node {                                               \
  private:                                                                     \
    virtual std::ostream &extract(std::ostream &o) const;                      \
    virtual bool equals(const ast_node &other) const;                          \
                                                                               \
  public:                                                                      \
    Member;                                                                    \
    Name(Member, yy::location location);                                       \
  }

#define AST_NODE_0(Name)                                                       \
  class Name : public ast_node {                                               \
  public:                                                                      \
    Name(yy::location location);                                               \
  }

#define AST_NODE_1(Name)                                                       \
  class Name : public ast_node {                                               \
  public:                                                                      \
    Name(std::shared_ptr<ast_node> child1, yy::location location);             \
  }

#define AST_NODE_2(Name)                                                       \
  class Name : public ast_node {                                               \
  public:                                                                      \
    Name(std::shared_ptr<ast_node> child1, std::shared_ptr<ast_node> child2,   \
         yy::location location);                                               \
  }

#define AST_NODE_3(Name)                                                       \
  class Name : public ast_node {                                               \
  public:                                                                      \
    Name(std::shared_ptr<ast_node> child1, std::shared_ptr<ast_node> child2,   \
         std::shared_ptr<ast_node> child3, yy::location location);             \
  }

AST_NODE_0(noop_node);

AST_NODE_LEAF(var_identifier_node, var_table_entry *entry);
AST_NODE_LEAF(type_identifier_node, type_table_entry *entry);

AST_NODE_LEAF(int_literal_node, std::int64_t value);
AST_NODE_LEAF(float_literal_node, double value);
AST_NODE_LEAF(boolean_literal_node, bool value);
AST_NODE_LEAF(char_literal_node, char value); // TODO
AST_NODE_LEAF(string_literal_node, std::string value);

AST_NODE_1(int_to_float_coercion_node);
AST_NODE_1(int_to_boolean_coercion_node);
AST_NODE_1(boolean_to_int_coercion_node);
AST_NODE_1(char_to_int_coercion_node);
AST_NODE_1(pointer_to_boolean_coercion_node);

AST_NODE_1(unary_minus_node);
AST_NODE_1(unary_plus_node);
AST_NODE_1(not_node);

class block_node : public ast_node {
private:
  virtual std::ostream &extract(std::ostream &o) const;
  virtual bool equals(const ast_node &other) const;

public:
  std::shared_ptr<symbol_table> table;

  block_node(std::shared_ptr<symbol_table> table,
             std::shared_ptr<ast_node> body, yy::location location);
};

AST_NODE_2(sum_node);
AST_NODE_2(subtraction_node);
AST_NODE_2(multiplication_node);
AST_NODE_2(division_node);
AST_NODE_2(modulo_node);
AST_NODE_2(lt_node);
AST_NODE_2(gt_node);
AST_NODE_2(lteq_node);
AST_NODE_2(gteq_node);
AST_NODE_2(equals_node);
AST_NODE_2(nequals_node);
AST_NODE_2(and_node);
AST_NODE_2(or_node);

AST_NODE_2(declaration_node);
AST_NODE_3(declaration_assignment_node);

AST_NODE_2(assignment_node);
AST_NODE_2(sum_assignment_node);
AST_NODE_2(subtraction_assignment_node);
AST_NODE_2(multiplication_assignment_node);
AST_NODE_2(division_assignment_node);
AST_NODE_2(modulo_assignment_node);
AST_NODE_2(sequence_node);

// Wraps a statement just to stablish statement breakpoints
AST_NODE_1(statement_node);

AST_NODE_2(while_loop_node);
AST_NODE_3(conditional_node);

AST_NODE_LEAF(label_node, std::string value);
AST_NODE_LEAF(goto_node, std::string value);

AST_NODE_1(write_node);
AST_NODE_1(read_node);

#endif /* AST_H */
