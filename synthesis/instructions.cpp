#include "synthesis/instructions.h"

#define X(Opcode, Enum, Operands) #Enum,
const char *op_names[] = {OPERATIONS};
#undef X

#define X(Opcode, Enum, Operands) Opcode,
const std::uint8_t opcode[] = {OPERATIONS};
#undef X

#define X(Opcode, Enum, Operands) Operands,
const std::uint8_t operand_count[] = {OPERATIONS};
#undef X

#define X(Code, Enum) Code,
const std::uint8_t syscall_code[] = {SYSCALLS};
#undef X

const char *name_of_instruction(op operation) {
  return op_names[(size_t)operation];
}

std::uint8_t opcode_of_instruction(op operation) {
  return opcode[(size_t)operation];
}

std::uint8_t operand_count_of_instruction(op operation) {
  return operand_count[(size_t)operation];
}

std::uint8_t code_of_syscall(sys_call call) {
  return syscall_code[(size_t)call];
}

op instruction_with_operands::get_operation() { return this->operation; }

std::uint64_t instruction_with_operands::get_operand(int index) {
  if (index >= 0 && index < operand_count()) {
    return this->operands[index];
  }

  return 0xBAD;
}

size_t instruction_with_operands::operand_count() {
  return operand_count_of_instruction(this->operation);
}

std::vector<std::uint8_t> instruction_with_operands::encode() {
  std::vector<std::uint8_t> vec;
  vec.push_back(opcode_of_instruction(this->operation));

  for (size_t i = 0; i < operand_count_of_instruction(this->operation); ++i) {
    vec.push_back(this->operands[i]);
  }

  return vec;
}

std::ostream &operator<<(std::ostream &o, const instruction_with_operands &a) {
  o << name_of_instruction(a.operation) << "(";

  auto opcount = operand_count_of_instruction(a.operation);

  if (opcount == 0) {
    return o << ")";
  }

  o << a.operands[0];

  for (auto i = 1; i < opcount; ++i) {
    o << ", " << a.operands[i];
  }

  o << ")";

  return o;
}
