#ifndef COMPILER_H
#define COMPILER_H

#include "parser/ast.h"
#include "synthesis/program.h"
#include <cstdint>
#include <memory>
#include <unordered_set>
#include <vector>

class data_manager;

struct address_placeholder_resolve_data {
  data_manager &data;
  std::map<std::uint64_t, std::uint64_t> &hidden_labels;
  std::map<std::string, std::uint64_t> &user_labels;
};

class address_placeholder {
public:
  virtual std::uint64_t resolve(address_placeholder_resolve_data data) = 0;
};

class absolute_address : public address_placeholder {
private:
  std::uint64_t index;

public:
  absolute_address(std::uint64_t index);
  virtual std::uint64_t resolve(address_placeholder_resolve_data data);
};

class intermediate_value_address : public address_placeholder {
private:
  std::uint64_t index;

public:
  intermediate_value_address(std::uint64_t index);
  virtual std::uint64_t resolve(address_placeholder_resolve_data data);
};

class hidden_label_reference : public address_placeholder {
private:
  std::uint64_t index;

public:
  hidden_label_reference(std::uint64_t index);
  virtual std::uint64_t resolve(address_placeholder_resolve_data data);
};

class user_label_reference : public address_placeholder {
private:
  std::string name;

public:
  user_label_reference(std::string name);
  virtual std::uint64_t resolve(address_placeholder_resolve_data data);
};

struct instruction_with_operand_placeholders {
  op operation;
  std::vector<std::shared_ptr<address_placeholder>> operands;

public:
  instruction_with_operand_placeholders(op operation);
  instruction_with_operand_placeholders(
      op operation, std::shared_ptr<address_placeholder> operand1);
  instruction_with_operand_placeholders(
      op operation, std::shared_ptr<address_placeholder> operand1,
      std::shared_ptr<address_placeholder> operand2);
  instruction_with_operand_placeholders(
      op operation, std::shared_ptr<address_placeholder> operand1,
      std::shared_ptr<address_placeholder> operand2,
      std::shared_ptr<address_placeholder> operand3);
  instruction_with_operand_placeholders(
      op operation, std::shared_ptr<address_placeholder> operand1,
      std::shared_ptr<address_placeholder> operand2,
      std::shared_ptr<address_placeholder> operand3,
      std::shared_ptr<address_placeholder> operand4);

  instruction_with_operands resolve(address_placeholder_resolve_data data);
};

// We transform an expression into a vector of expr_component effectively
// expressing it in postfix form
class expr_component {
public:
  ast_node *node;

  expr_component(ast_node *node);
  virtual long get_stack_size_contribution() = 0;
};

class expr_operand : public expr_component {
public:
  expr_operand(ast_node *node);
  virtual long get_stack_size_contribution();
};

class expr_bin_operator : public expr_component {
public:
  expr_bin_operator(ast_node *node);
  virtual long get_stack_size_contribution();
};

class expr_unary_operator : public expr_component {
public:
  expr_unary_operator(ast_node *node);
  virtual long get_stack_size_contribution();
};

class data_manager {
private:
  std::vector<expr_component *> intermediate_value_stack;
  std::uint64_t intermediate_value_stack_tip;

public:
  int variable_data_size;
  int intermediate_value_data_size;

  std::map<std::uint64_t, variable_data> variables;

  data_manager();

  std::uint64_t add_variable(var_table_entry *entry);
  void ensure_intermediate_values(unsigned int count);
  std::uint64_t get_current_intermediate_values_start();

  std::shared_ptr<address_placeholder>
  push_intermediate(expr_component *component);
  std::shared_ptr<address_placeholder> pop_intermediate();
  std::shared_ptr<address_placeholder> peek_intermediate();
  int intermediate_stack_size();

  int data_size();
};

typedef std::vector<instruction_with_operand_placeholders> compilation_unit;

class code_manager {
private:
  std::vector<compilation_unit> compilation_units;

public:
  std::uint64_t get_current_procedure_start();
  std::uint64_t get_remote_procedure_start(std::string name);
};

// Implemented in `compiler.cpp` and `compile_ops.cpp` to keep file size
// a bit smaller
class compiler {
private:
  data_manager data;
  std::vector<instruction_with_operand_placeholders> instructions;

  std::map<std::uint64_t, std::uint64_t> hidden_labels;
  std::uint64_t hidden_label_counter;

  std::map<std::string, std::uint64_t> user_labels;

  // Metadata
  std::unordered_set<std::uint64_t> statement_boundaries;
  std::vector<std::uint64_t> source_line_map;

  void push_statement_boundary();
  void push_instruction(instruction_with_operand_placeholders instruction,
                        yy::location from);
  std::uint64_t current_instruction_index();

  void setup_variables(std::shared_ptr<ast_node> root);
  std::uint64_t make_label();

  // compile_ops.cpp
  void compile_select(std::shared_ptr<ast_node> node);

  void compile_expr(std::shared_ptr<ast_node> tree);
  void compile_expr_select(expr_component *expr);
  void compile_expr_unary_minus(expr_component *expr);
  void compile_expr_unary_plus(expr_component *expr);
  void compile_expr_sum(expr_component *expr);
  void compile_expr_sub(expr_component *expr);
  void compile_expr_mul(expr_component *expr);
  void compile_expr_div(expr_component *expr);
  void compile_expr_mod(expr_component *expr);
  void compile_expr_var(expr_component *expr);
  void compile_expr_literal(expr_component *expr);
  void compile_expr_assignment(expr_component *expr);
  void compile_expr_sum_assignment(expr_component *expr);
  void compile_expr_subtraction_assignment(expr_component *expr);
  void compile_expr_multiplication_assignment(expr_component *expr);
  void compile_expr_division_assignment(expr_component *expr);
  void compile_expr_modulo_assignment(expr_component *expr);
  void compile_expr_gt(expr_component *expr);
  void compile_expr_lt(expr_component *expr);
  void compile_expr_gteq(expr_component *expr);
  void compile_expr_lteq(expr_component *expr);
  void compile_expr_equals(expr_component *expr);
  void compile_expr_nequals(expr_component *expr);
  void compile_expr_not(expr_component *expr);
  void compile_expr_and(expr_component *expr);
  void compile_expr_or(expr_component *expr);

  void compile_statement(std::shared_ptr<ast_node> node);
  void compile_block(std::shared_ptr<ast_node> node);
  void compile_sequence(std::shared_ptr<ast_node> node);
  void compile_decl_assignment(std::shared_ptr<ast_node> node);
  void compile_conditional(std::shared_ptr<ast_node> node);
  void compile_while(std::shared_ptr<ast_node> node);
  void compile_label(std::shared_ptr<ast_node> node);
  void compile_goto(std::shared_ptr<ast_node> node);
  void compile_write(std::shared_ptr<ast_node> node);
  void compile_read(std::shared_ptr<ast_node> node);
  // /compile_ops.cpp

public:
  compiler();
  program compile(std::shared_ptr<ast_node> ast);
};

#endif /* COMPILER_H */
