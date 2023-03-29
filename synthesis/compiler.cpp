#include "synthesis/compiler.h"
#include <exception>

data_manager::data_manager()
    : variable_data_size(0), intermediate_value_data_size(0),
      intermediate_value_stack_tip(0) {}

std::uint64_t data_manager::add_variable(var_table_entry *entry) {
  auto current_size = this->variable_data_size;
  auto var_size = entry->type->value->size();
  auto final_size = current_size + var_size;

  this->variable_data_size = final_size;

  // This assumes global variables will be the first things in memory
  variable_data *metadata = &this->variables[entry->offset];
  metadata->name = entry->name;
  metadata->size = entry->type->value->size();
  metadata->address = entry->offset;
  metadata->declared_at = entry->declared_at;

  return current_size;
}

void data_manager::ensure_intermediate_values(unsigned int count) {
  if (this->intermediate_value_data_size < count) {
    this->intermediate_value_data_size = count;
  }
}

std::uint64_t data_manager::get_current_intermediate_values_start() {
  // Intermediate values start after global variables
  return this->variable_data_size;
}

std::shared_ptr<address_placeholder>
data_manager::push_intermediate(expr_component *component) {
  auto tip = this->intermediate_value_stack_tip;
  this->intermediate_value_stack.push_back(component);
  this->intermediate_value_stack_tip += component->node->typ->size();

  return std::make_shared<intermediate_value_address>(tip);
}

std::shared_ptr<address_placeholder> data_manager::pop_intermediate() {
  auto last = this->intermediate_value_stack.back();
  this->intermediate_value_stack.pop_back();
  this->intermediate_value_stack_tip -= last->node->typ->size();

  return std::make_shared<intermediate_value_address>(
      this->intermediate_value_stack_tip);
}

int data_manager::data_size() {
  return this->variable_data_size + this->intermediate_value_data_size;
}

absolute_address::absolute_address(std::uint64_t index) : index(index) {}
std::uint64_t absolute_address::resolve(address_placeholder_resolve_data data) {
  return this->index;
}

intermediate_value_address::intermediate_value_address(std::uint64_t index)
    : index(index) {}
std::uint64_t
intermediate_value_address::resolve(address_placeholder_resolve_data data) {
  return data.data.get_current_intermediate_values_start() + this->index;
}

hidden_label_reference::hidden_label_reference(std::uint64_t index)
    : index(index) {}
std::uint64_t
hidden_label_reference::resolve(address_placeholder_resolve_data data) {
  return data.hidden_labels[this->index];
}

user_label_reference::user_label_reference(std::string name) : name(name) {}
std::uint64_t
user_label_reference::resolve(address_placeholder_resolve_data data) {
  auto maybe_found = data.user_labels.find(this->name);

  if (maybe_found == data.user_labels.end()) {
    throw std::runtime_error("Label " + this->name +
                             " referenced but not defined.");
  }

  return data.user_labels[this->name];
}

instruction_with_operand_placeholders::instruction_with_operand_placeholders(
    op operation)
    : operation(operation) {}

instruction_with_operand_placeholders::instruction_with_operand_placeholders(
    op operation, std::shared_ptr<address_placeholder> operand1)
    : operation(operation) {
  this->operands.push_back(operand1);
}

instruction_with_operand_placeholders::instruction_with_operand_placeholders(
    op operation, std::shared_ptr<address_placeholder> operand1,
    std::shared_ptr<address_placeholder> operand2)
    : operation(operation) {
  this->operands.push_back(operand1);
  this->operands.push_back(operand2);
}

instruction_with_operand_placeholders::instruction_with_operand_placeholders(
    op operation, std::shared_ptr<address_placeholder> operand1,
    std::shared_ptr<address_placeholder> operand2,
    std::shared_ptr<address_placeholder> operand3)
    : operation(operation) {
  this->operands.push_back(operand1);
  this->operands.push_back(operand2);
  this->operands.push_back(operand3);
}

instruction_with_operand_placeholders::instruction_with_operand_placeholders(
    op operation, std::shared_ptr<address_placeholder> operand1,
    std::shared_ptr<address_placeholder> operand2,
    std::shared_ptr<address_placeholder> operand3,
    std::shared_ptr<address_placeholder> operand4)
    : operation(operation) {
  this->operands.push_back(operand1);
  this->operands.push_back(operand2);
  this->operands.push_back(operand3);
  this->operands.push_back(operand4);
}

instruction_with_operands instruction_with_operand_placeholders::resolve(
    address_placeholder_resolve_data data) {
  instruction_with_operands result;
  result.operation = this->operation;

  auto operand_count = operand_count_of_instruction(this->operation);
  for (auto i = 0; i < operand_count; ++i) {
    result.operands[i] = this->operands[i]->resolve(data);
  }

  return result;
}

expr_component::expr_component(ast_node *node) : node(node) {}

expr_operand::expr_operand(ast_node *node) : expr_component(node) {}
long expr_operand::get_stack_size_contribution() {
  return +this->node->typ->size();
}

expr_bin_operator::expr_bin_operator(ast_node *node) : expr_component(node) {}
long expr_bin_operator::get_stack_size_contribution() {
  auto operand1 = this->node->children[0];
  auto operand2 = this->node->children[1];

  return this->node->typ->size() - operand1->typ->size() -
         operand2->typ->size();
}

expr_unary_operator::expr_unary_operator(ast_node *node)
    : expr_component(node) {}
long expr_unary_operator::get_stack_size_contribution() {
  auto operand = this->node->children[0];
  return this->node->typ->size() - operand->typ->size();
}

compiler::compiler() : hidden_label_counter(0) {}

void compiler::push_statement_boundary() {
  this->statement_boundaries.insert(instructions.size());
}

void compiler::push_instruction(
    instruction_with_operand_placeholders instruction, yy::location from) {
  this->instructions.push_back(instruction);
  this->source_line_map.push_back(from.begin.line);
}

std::uint64_t compiler::current_instruction_index() {
  return this->instructions.size();
}

void setup_variables_from_table(data_manager *data, symbol_table *table) {
  auto vars = table->vars();

  for (auto const &pair : *vars) {
    auto [key, value] = pair;
    data->add_variable(&value);
  }

  for (auto const &child : table->get_children()) {
    setup_variables_from_table(data, child);
  }
}

void compiler::setup_variables(std::shared_ptr<ast_node> root) {
  auto block = std::dynamic_pointer_cast<block_node>(root);
  setup_variables_from_table(&this->data, block->table.get());
}

std::uint64_t compiler::make_label() {
  auto index = this->hidden_label_counter++;

  // Should be set to something different afterwards
  this->hidden_labels[index] = 0;

  return index;
}

program compiler::compile(std::shared_ptr<ast_node> ast) {
  program prog;

  setup_variables(ast);
  compile_select(ast);

  prog.code.reserve(instructions.size());
  prog.data.insert(prog.data.begin(), this->data.data_size(), 0);

  address_placeholder_resolve_data address_data{this->data, this->hidden_labels,
                                                this->user_labels};

  for (auto &placeholder : instructions) {
    auto resolved = placeholder.resolve(address_data);
    prog.code.push_back(resolved);
  }

  prog.metadata.statement_boundaries.insert(
      prog.metadata.statement_boundaries.end(),
      this->statement_boundaries.begin(), this->statement_boundaries.end());

  prog.metadata.source_line_map.insert(prog.metadata.source_line_map.end(),
                                       this->source_line_map.begin(),
                                       this->source_line_map.end());

  prog.metadata.variables = this->data.variables;
  return prog;
}
