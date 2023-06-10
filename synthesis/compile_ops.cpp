#include "synthesis/compiler.h"
#include <vector>

std::shared_ptr<address_placeholder> absolute(std::uint64_t index) {
  return std::make_shared<absolute_address>(index);
}

std::shared_ptr<address_placeholder> intermediate_value(std::uint64_t index) {
  return std::make_shared<intermediate_value_address>(index);
}

std::shared_ptr<address_placeholder> label(std::uint64_t index) {
  return std::make_shared<hidden_label_reference>(index);
}

std::shared_ptr<address_placeholder> label(std::string name) {
  return std::make_shared<user_label_reference>(name);
}

// TODO: Improve this
void compiler::compile_select(std::shared_ptr<ast_node> node) {
  switch (node->kind) {
  case ast_node_kind::SUM:
  case ast_node_kind::SUBTRACTION:
  case ast_node_kind::MULTIPLICATION:
  case ast_node_kind::DIVISION:
  case ast_node_kind::MODULO:
  case ast_node_kind::ASSIGNMENT:
  case ast_node_kind::SUM_ASSIGNMENT:
  case ast_node_kind::SUBTRACTION_ASSIGNMENT:
  case ast_node_kind::MULTIPLICATION_ASSIGNMENT:
  case ast_node_kind::DIVISION_ASSIGNMENT:
  case ast_node_kind::MODULO_ASSIGNMENT:
  case ast_node_kind::LT:
  case ast_node_kind::GT:
  case ast_node_kind::LTEQ:
  case ast_node_kind::GTEQ:
  case ast_node_kind::EQUALS:
  case ast_node_kind::NEQUALS:
  case ast_node_kind::AND:
  case ast_node_kind::OR:

  case ast_node_kind::INT_TO_FLOAT_COERCION:
  case ast_node_kind::INT_TO_BOOLEAN_COERCION:
  case ast_node_kind::BOOLEAN_TO_INT_COERCION:
  case ast_node_kind::POINTER_TO_BOOLEAN_COERCION:
  case ast_node_kind::UNARY_MINUS:
  case ast_node_kind::UNARY_PLUS:
  case ast_node_kind::NOT:

  case ast_node_kind::VAR_IDENTIFIER:

  case ast_node_kind::INT_LITERAL:
  case ast_node_kind::FLOAT_LITERAL:
  case ast_node_kind::BOOLEAN_LITERAL:
  case ast_node_kind::CHAR_LITERAL:
  case ast_node_kind::STRING_LITERAL: // TODO
    return compile_expr(node);

  case ast_node_kind::STATEMENT:
    return compile_statement(node);

  case ast_node_kind::BLOCK:
    return compile_block(node);
  case ast_node_kind::SEQUENCE:
    return compile_sequence(node);

  case ast_node_kind::DECLARATION_ASSIGNMENT:
    return compile_decl_assignment(node);

  case ast_node_kind::CONDITIONAL:
    return compile_conditional(node);

  case ast_node_kind::WHILE:
    return compile_while(node);

  case ast_node_kind::LABEL:
    return compile_label(node);

  case ast_node_kind::GOTO:
    return compile_goto(node);

  case ast_node_kind::WRITE:
    return compile_write(node);

  case ast_node_kind::READ:
    return compile_read(node);

  case ast_node_kind::DECLARATION:
  case ast_node_kind::NOOP:
    return;
  default:
    std::cout << "Node not implemented " << name_of_ast_node_kind(node->kind)
              << '\n';
    return;
  }
}

// In the loose sense that 2 operands becomes 1 value
bool is_bin_operation(std::shared_ptr<ast_node> node) {
  switch (node->kind) {
  case ast_node_kind::SUM:
  case ast_node_kind::SUBTRACTION:
  case ast_node_kind::MULTIPLICATION:
  case ast_node_kind::DIVISION:
  case ast_node_kind::MODULO:
  case ast_node_kind::SUM_ASSIGNMENT:
  case ast_node_kind::SUBTRACTION_ASSIGNMENT:
  case ast_node_kind::MULTIPLICATION_ASSIGNMENT:
  case ast_node_kind::DIVISION_ASSIGNMENT:
  case ast_node_kind::MODULO_ASSIGNMENT:
  case ast_node_kind::LT:
  case ast_node_kind::GT:
  case ast_node_kind::LTEQ:
  case ast_node_kind::GTEQ:
  case ast_node_kind::EQUALS:
  case ast_node_kind::NEQUALS:
  case ast_node_kind::AND:
  case ast_node_kind::OR:
    return true;
  default:
    return false;
  }
}

bool is_unary_operation(std::shared_ptr<ast_node> node) {
  switch (node->kind) {
  case ast_node_kind::INT_TO_FLOAT_COERCION:
  case ast_node_kind::INT_TO_BOOLEAN_COERCION:
  case ast_node_kind::BOOLEAN_TO_INT_COERCION:
  case ast_node_kind::POINTER_TO_BOOLEAN_COERCION:
  case ast_node_kind::UNARY_MINUS:
  case ast_node_kind::UNARY_PLUS:
  case ast_node_kind::NOT:
    return true;
  default:
    return false;
  }
}

bool is_simple_assignment(std::shared_ptr<ast_node> node) {
  switch (node->kind) {
  case ast_node_kind::ASSIGNMENT:
    return true;
  default:
    return false;
  }
}

void add_expr_nodes(std::vector<std::shared_ptr<expr_component>> *vec,
                    std::shared_ptr<ast_node> tree) {
  if (is_bin_operation(tree)) {
    add_expr_nodes(vec, tree->children[0]);
    add_expr_nodes(vec, tree->children[1]);

    auto ooperator = std::make_shared<expr_bin_operator>(tree.get());
    vec->push_back(ooperator);
    return;
  }

  if (is_unary_operation(tree)) {
    add_expr_nodes(vec, tree->children[0]);

    auto ooperator = std::make_shared<expr_unary_operator>(tree.get());
    vec->push_back(ooperator);
    return;
  }

  if (is_simple_assignment(tree)) {
    add_expr_nodes(vec, tree->children[1]);

    auto ooperator = std::make_shared<expr_bin_operator>(tree.get());
    vec->push_back(ooperator);
    return;
  }

  auto operand = std::make_shared<expr_operand>(tree.get());
  vec->push_back(operand);
}

void compiler::compile_expr(std::shared_ptr<ast_node> tree) {
  std::vector<std::shared_ptr<expr_component>> components;
  add_expr_nodes(&components, tree);

  unsigned long stack_size = 0;

  for (auto &component : components) {
    // Reserve the data necessary to evaluate the expression
    // (We only have one register to work with)
    stack_size += component->get_stack_size_contribution();
    this->data.ensure_intermediate_values(stack_size);

    compile_expr_select(component.get());
  }

  auto result = this->data.pop_intermediate();
}

void compiler::compile_expr_select(expr_component *expr) {
  switch (expr->node->kind) {
  case ast_node_kind::VAR_IDENTIFIER:
    return compile_expr_var(expr);
  case ast_node_kind::INT_LITERAL:
  case ast_node_kind::FLOAT_LITERAL:
  case ast_node_kind::BOOLEAN_LITERAL:
  case ast_node_kind::CHAR_LITERAL:
    return compile_expr_literal(expr);
  case ast_node_kind::UNARY_MINUS:
    return compile_expr_unary_minus(expr);
  case ast_node_kind::UNARY_PLUS:
    return compile_expr_unary_plus(expr);
  case ast_node_kind::SUM:
    return compile_expr_sum(expr);
  case ast_node_kind::SUBTRACTION:
    return compile_expr_sub(expr);
  case ast_node_kind::MULTIPLICATION:
    return compile_expr_mul(expr);
  case ast_node_kind::DIVISION:
    return compile_expr_div(expr);
  case ast_node_kind::MODULO:
    return compile_expr_mod(expr);
  case ast_node_kind::ASSIGNMENT:
    return compile_expr_assignment(expr);
  case ast_node_kind::SUM_ASSIGNMENT:
    return compile_expr_sum_assignment(expr);
  case ast_node_kind::SUBTRACTION_ASSIGNMENT:
    return compile_expr_subtraction_assignment(expr);
  case ast_node_kind::MULTIPLICATION_ASSIGNMENT:
    return compile_expr_multiplication_assignment(expr);
  case ast_node_kind::DIVISION_ASSIGNMENT:
    return compile_expr_division_assignment(expr);
  case ast_node_kind::MODULO_ASSIGNMENT:
    return compile_expr_modulo_assignment(expr);
  case ast_node_kind::GT:
    return compile_expr_gt(expr);
  case ast_node_kind::LT:
    return compile_expr_lt(expr);
  case ast_node_kind::GTEQ:
    return compile_expr_gteq(expr);
  case ast_node_kind::LTEQ:
    return compile_expr_lteq(expr);
  case ast_node_kind::EQUALS:
    return compile_expr_equals(expr);
  case ast_node_kind::NEQUALS:
    return compile_expr_nequals(expr);
  case ast_node_kind::NOT:
    return compile_expr_not(expr);
  case ast_node_kind::AND:
    return compile_expr_and(expr);
  case ast_node_kind::OR:
    return compile_expr_or(expr);
  default: {
    std::cout << "Expression node not implemented "
              << name_of_ast_node_kind(expr->node->kind) << '\n';
    return;
  }
  }
}

void compiler::compile_expr_unary_minus(expr_component *expr) {
  auto operand1 = this->data.pop_intermediate();

  // push_instruction(instruction_with_operand_placeholders(op::LOAD, operand1),
  //                  expr->node->location);
  push_instruction(instruction_with_operand_placeholders(op::NEGATE),
                   expr->node->location);

  auto result = this->data.push_intermediate(expr);
}

void compiler::compile_expr_unary_plus(expr_component *expr) {}

void compiler::compile_expr_sum(expr_component *expr) {
  auto operand1 = this->data.pop_intermediate();
  auto operand2 = this->data.pop_intermediate();

  // push_instruction(instruction_with_operand_placeholders(op::LOAD, operand1),
  //                  expr->node->location);
  push_instruction(instruction_with_operand_placeholders(op::ADD, operand2),
                   expr->node->location);

  auto result = this->data.push_intermediate(expr);
}

void compiler::compile_expr_sub(expr_component *expr) {
  auto operand1 = this->data.pop_intermediate();
  auto operand2 = this->data.pop_intermediate();

  // push_instruction(instruction_with_operand_placeholders(op::LOAD, operand1),
  //                  expr->node->location);
  push_instruction(
      instruction_with_operand_placeholders(op::SUBTRACT, operand2),
      expr->node->location);

  auto result = this->data.push_intermediate(expr);
}

void compiler::compile_expr_mul(expr_component *expr) {
  auto operand1 = this->data.pop_intermediate();
  auto operand2 = this->data.pop_intermediate();

  // push_instruction(instruction_with_operand_placeholders(op::LOAD, operand1),
  //                  expr->node->location);
  push_instruction(
      instruction_with_operand_placeholders(op::MULTIPLY, operand2),
      expr->node->location);

  auto result = this->data.push_intermediate(expr);
}

void compiler::compile_expr_div(expr_component *expr) {
  auto operand1 = this->data.pop_intermediate();
  auto operand2 = this->data.pop_intermediate();

  // push_instruction(instruction_with_operand_placeholders(op::LOAD, operand1),
  //                  expr->node->location);
  push_instruction(instruction_with_operand_placeholders(op::DIVIDE, operand2),
                   expr->node->location);

  auto result = this->data.push_intermediate(expr);
}

void compiler::compile_expr_mod(expr_component *expr) {
  auto operand1 = this->data.pop_intermediate();
  auto operand2 = this->data.pop_intermediate();

  // push_instruction(instruction_with_operand_placeholders(op::LOAD, operand1),
  //                  expr->node->location);
  push_instruction(
      instruction_with_operand_placeholders(op::REMAINDER, operand2),
      expr->node->location);

  auto result = this->data.push_intermediate(expr);
}

void compiler::compile_expr_var(expr_component *expr) {
  auto var_node = (var_identifier_node *)(expr->node);
  auto var_offset = var_node->entry->offset;

  if (this->data.intermediate_stack_size() != 0) {
    auto last = this->data.peek_intermediate();
    push_instruction(instruction_with_operand_placeholders(op::SET, last),
                     expr->node->location);
  }

  push_instruction(
      instruction_with_operand_placeholders(op::LOAD, absolute(var_offset)),
      expr->node->location);

  auto bitmask = 0;
  for (auto i = 0; i < var_node->typ->size() * 8; ++i) {
    bitmask = (bitmask << 1) | 1;
  }

  push_instruction(
      instruction_with_operand_placeholders(op::AND_I, absolute(bitmask)),
      expr->node->location);

  auto result = this->data.push_intermediate(expr);
}

void compiler::compile_expr_literal(expr_component *expr) {
  std::uint64_t value;

  switch (expr->node->typ->kind) {
  case type_kind::INT: {
    auto node = (int_literal_node *)(expr->node);
    value = node->value;
    break;
  }
  case type_kind::FLOAT: {
    auto node = (float_literal_node *)(expr->node);
    value = *(std::int64_t *)&(node->value);
    break;
  }
  case type_kind::BOOLEAN: {
    auto node = (boolean_literal_node *)(expr->node);
    value = node->value ? 1 : 0;
    break;
  }
  case type_kind::CHAR: {
    auto node = (char_literal_node *)(expr->node);
    value = node->value;
    break;
  }
  default:
    std::cout << "Invalid type in expression " << expr->node->typ->kind << '\n';
    value = 0xBAD;
    break;
  }

  if (this->data.intermediate_stack_size() != 0) {
    auto last = this->data.peek_intermediate();
    push_instruction(instruction_with_operand_placeholders(op::SET, last),
                     expr->node->location);
  }

  push_instruction(
      instruction_with_operand_placeholders(op::LOAD_I, absolute(value)),
      expr->node->location);

  auto result = this->data.push_intermediate(expr);
}

void compiler::compile_expr_assignment(expr_component *expr) {
  auto var_node =
      std::dynamic_pointer_cast<var_identifier_node>(expr->node->children[0]);
  auto var_offset = var_node->entry->offset;

  push_instruction(
      instruction_with_operand_placeholders(op::SET, absolute(var_offset)),
      expr->node->location);

  // The assigned value is kept in the register
}

void compiler::compile_expr_sum_assignment(expr_component *expr) {
  auto var_node =
      std::dynamic_pointer_cast<var_identifier_node>(expr->node->children[0]);
  auto var_offset = var_node->entry->offset;

  auto previous = this->data.pop_intermediate();
  auto modifier = this->data.pop_intermediate();

  // push_instruction(instruction_with_operand_placeholders(op::LOAD, previous),
  //                  expr->node->location);

  push_instruction(instruction_with_operand_placeholders(op::ADD, modifier),
                   expr->node->location);

  push_instruction(
      instruction_with_operand_placeholders(op::SET, absolute(var_offset)),
      expr->node->location);

  auto result = this->data.push_intermediate(expr);
}

void compiler::compile_expr_subtraction_assignment(expr_component *expr) {
  auto var_node =
      std::dynamic_pointer_cast<var_identifier_node>(expr->node->children[0]);
  auto var_offset = var_node->entry->offset;

  auto previous = this->data.pop_intermediate();
  auto modifier = this->data.pop_intermediate();

  // push_instruction(instruction_with_operand_placeholders(op::LOAD, previous),
  //                  expr->node->location);

  push_instruction(
      instruction_with_operand_placeholders(op::SUBTRACT, modifier),
      expr->node->location);

  push_instruction(
      instruction_with_operand_placeholders(op::SET, absolute(var_offset)),
      expr->node->location);

  auto result = this->data.push_intermediate(expr);
}

void compiler::compile_expr_multiplication_assignment(expr_component *expr) {
  auto var_node =
      std::dynamic_pointer_cast<var_identifier_node>(expr->node->children[0]);
  auto var_offset = var_node->entry->offset;

  auto previous = this->data.pop_intermediate();
  auto modifier = this->data.pop_intermediate();

  // push_instruction(instruction_with_operand_placeholders(op::LOAD, previous),
  //                  expr->node->location);

  push_instruction(
      instruction_with_operand_placeholders(op::MULTIPLY, modifier),
      expr->node->location);

  push_instruction(
      instruction_with_operand_placeholders(op::SET, absolute(var_offset)),
      expr->node->location);

  auto result = this->data.push_intermediate(expr);
}

void compiler::compile_expr_division_assignment(expr_component *expr) {
  auto var_node =
      std::dynamic_pointer_cast<var_identifier_node>(expr->node->children[0]);
  auto var_offset = var_node->entry->offset;

  auto previous = this->data.pop_intermediate();
  auto modifier = this->data.pop_intermediate();

  // push_instruction(instruction_with_operand_placeholders(op::LOAD, previous),
  //                  expr->node->location);

  push_instruction(instruction_with_operand_placeholders(op::DIVIDE, modifier),
                   expr->node->location);

  push_instruction(
      instruction_with_operand_placeholders(op::SET, absolute(var_offset)),
      expr->node->location);

  auto result = this->data.push_intermediate(expr);
}

void compiler::compile_expr_modulo_assignment(expr_component *expr) {
  auto var_node =
      std::dynamic_pointer_cast<var_identifier_node>(expr->node->children[0]);
  auto var_offset = var_node->entry->offset;

  auto previous = this->data.pop_intermediate();
  auto modifier = this->data.pop_intermediate();

  // push_instruction(instruction_with_operand_placeholders(op::LOAD, previous),
  //                  expr->node->location);

  push_instruction(
      instruction_with_operand_placeholders(op::REMAINDER, modifier),
      expr->node->location);

  push_instruction(
      instruction_with_operand_placeholders(op::SET, absolute(var_offset)),
      expr->node->location);

  auto result = this->data.push_intermediate(expr);
}

void compiler::compile_expr_gt(expr_component *expr) {
  auto operand1 = this->data.pop_intermediate();
  auto operand2 = this->data.pop_intermediate();

  // push_instruction(instruction_with_operand_placeholders(op::LOAD, operand1),
  //                  expr->node->location);

  push_instruction(instruction_with_operand_placeholders(op::GT, operand2),
                   expr->node->location);

  auto result = this->data.push_intermediate(expr);
}

void compiler::compile_expr_lt(expr_component *expr) {
  auto operand1 = this->data.pop_intermediate();
  auto operand2 = this->data.pop_intermediate();

  // push_instruction(instruction_with_operand_placeholders(op::LOAD, operand1),
  //                  expr->node->location);

  push_instruction(instruction_with_operand_placeholders(op::LT, operand2),
                   expr->node->location);

  auto result = this->data.push_intermediate(expr);
}

void compiler::compile_expr_gteq(expr_component *expr) {
  auto operand1 = this->data.pop_intermediate();
  auto operand2 = this->data.pop_intermediate();

  // push_instruction(instruction_with_operand_placeholders(op::LOAD, operand1),
  //                  expr->node->location);

  push_instruction(instruction_with_operand_placeholders(op::GTEQ, operand2),
                   expr->node->location);

  auto result = this->data.push_intermediate(expr);
}

void compiler::compile_expr_lteq(expr_component *expr) {
  auto operand1 = this->data.pop_intermediate();
  auto operand2 = this->data.pop_intermediate();

  // push_instruction(instruction_with_operand_placeholders(op::LOAD, operand1),
  //                  expr->node->location);

  push_instruction(instruction_with_operand_placeholders(op::LTEQ, operand2),
                   expr->node->location);

  auto result = this->data.push_intermediate(expr);
}

void compiler::compile_expr_equals(expr_component *expr) {
  auto operand1 = this->data.pop_intermediate();
  auto operand2 = this->data.pop_intermediate();

  // push_instruction(instruction_with_operand_placeholders(op::LOAD, operand1),
  //                  expr->node->location);

  push_instruction(instruction_with_operand_placeholders(op::EQUALS, operand2),
                   expr->node->location);

  auto result = this->data.push_intermediate(expr);
}

void compiler::compile_expr_nequals(expr_component *expr) {
  auto operand1 = this->data.pop_intermediate();
  auto operand2 = this->data.pop_intermediate();

  // push_instruction(instruction_with_operand_placeholders(op::LOAD, operand1),
  //                  expr->node->location);

  push_instruction(instruction_with_operand_placeholders(op::EQUALS, operand2),
                   expr->node->location);

  push_instruction(instruction_with_operand_placeholders(op::NOT),
                   expr->node->location);

  auto result = this->data.push_intermediate(expr);
}

void compiler::compile_expr_not(expr_component *expr) {
  auto operand1 = this->data.pop_intermediate();

  // push_instruction(instruction_with_operand_placeholders(op::LOAD, operand1),
  //                  expr->node->location);

  push_instruction(instruction_with_operand_placeholders(op::NOT),
                   expr->node->location);

  auto result = this->data.push_intermediate(expr);
  push_instruction(instruction_with_operand_placeholders(op::SET, result),
                   expr->node->location);
}

void compiler::compile_expr_and(expr_component *expr) {
  auto operand1 = this->data.pop_intermediate();
  auto operand2 = this->data.pop_intermediate();

  // push_instruction(instruction_with_operand_placeholders(op::LOAD, operand1),
  //                  expr->node->location);

  push_instruction(instruction_with_operand_placeholders(op::AND, operand2),
                   expr->node->location);

  auto result = this->data.push_intermediate(expr);
}

void compiler::compile_expr_or(expr_component *expr) {
  auto operand1 = this->data.pop_intermediate();
  auto operand2 = this->data.pop_intermediate();

  // push_instruction(instruction_with_operand_placeholders(op::LOAD, operand1),
  //                  expr->node->location);

  push_instruction(instruction_with_operand_placeholders(op::OR, operand2),
                   expr->node->location);

  auto result = this->data.push_intermediate(expr);
}

void compiler::compile_statement(std::shared_ptr<ast_node> node) {
  push_statement_boundary();
  compile_select(node->children[0]);
}

void compiler::compile_block(std::shared_ptr<ast_node> node) {
  compile_select(node->children[0]);
}

void compiler::compile_sequence(std::shared_ptr<ast_node> node) {
  compile_select(node->children[0]);
  compile_select(node->children[1]);
}

void compiler::compile_decl_assignment(std::shared_ptr<ast_node> node) {
  compile_select(node->children[2]);

  auto var = std::dynamic_pointer_cast<var_identifier_node>(node->children[1]);
  auto var_offset = var->entry->offset;

  push_instruction(
      instruction_with_operand_placeholders(op::SET, absolute(var_offset)),
      node->location);
}

void compiler::compile_conditional(std::shared_ptr<ast_node> node) {
  compile_select(node->children[0]);
  push_statement_boundary();

  auto else_body_label = this->make_label();
  auto end_label = this->make_label();

  push_instruction(instruction_with_operand_placeholders(
                       op::BRANCH_IF_ZERO, label(else_body_label)),
                   node->location);

  compile_select(node->children[1]);

  push_instruction(
      instruction_with_operand_placeholders(op::JUMP, label(end_label)),
      node->location);

  auto else_body_index = current_instruction_index();
  this->hidden_labels[else_body_label] = else_body_index;

  compile_select(node->children[2]);

  auto end_index = current_instruction_index();
  this->hidden_labels[end_label] = end_index;
}

void compiler::compile_while(std::shared_ptr<ast_node> node) {
  auto start_label = this->make_label();
  auto start_index = current_instruction_index();
  this->hidden_labels[start_label] = start_index;

  compile_select(node->children[0]);
  push_statement_boundary();

  auto end_label = this->make_label();

  push_instruction(instruction_with_operand_placeholders(op::BRANCH_IF_ZERO,
                                                         label(end_label)),
                   node->location);

  compile_select(node->children[1]);

  push_instruction(
      instruction_with_operand_placeholders(op::JUMP, label(start_label)),
      node->location);

  auto end_index = current_instruction_index();
  this->hidden_labels[end_label] = end_index;
}

void compiler::compile_label(std::shared_ptr<ast_node> node) {
  auto cast = std::dynamic_pointer_cast<label_node>(node);
  this->user_labels[cast->value] = current_instruction_index();
}

void compiler::compile_goto(std::shared_ptr<ast_node> node) {
  auto cast = std::dynamic_pointer_cast<goto_node>(node);

  push_instruction(
      instruction_with_operand_placeholders(op::JUMP, label(cast->value)),
      node->location);
}

void compiler::compile_write(std::shared_ptr<ast_node> node) {
  compile_select(node->children[0]);

  auto syscall_code = code_of_syscall(sys_call::WRITE);
  push_instruction(instruction_with_operand_placeholders(
                       op::INTERRUPT, absolute(syscall_code)),
                   node->location);
}

void compiler::compile_read(std::shared_ptr<ast_node> node) {
  auto var_node =
      std::dynamic_pointer_cast<var_identifier_node>(node->children[0]);
  auto var_offset = var_node->entry->offset;

  push_instruction(
      instruction_with_operand_placeholders(op::LOAD_I, absolute(var_offset)),
      node->location);

  auto syscall_code = code_of_syscall(sys_call::READ);
  push_instruction(instruction_with_operand_placeholders(
                       op::INTERRUPT, absolute(syscall_code)),
                   node->location);
}
