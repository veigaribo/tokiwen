#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

#include "cinttypes"
#include <iostream>
#include <vector>

/*
 * Registers:
 * IP/PC: Instruction pointer / program counter
 * SP: Stack pointer
 * BP: Base pointer
 * X: General use
 */

/*
 * Instructions typically operate on the X register and also store the result
 * there.
 *
 * Prefix `F`: Float operation
 * Suffix `I`: Use immediate operand
 *
 * (Opcode, Enum name, Operands)
 *
 * Keep in sync with `editor/src/ts/vm/instructions.ts`
 */
#define OPERATIONS                                                             \
  X(0, NOOP, 0)                                                                \
                                                                               \
  X(1, LOAD, 1)                                                                \
  X(2, SET, 1)                                                                 \
  X(3, LOAD_BP, 0)                                                             \
  X(4, LOAD_I, 1)                                                              \
                                                                               \
  X(10, PUSH, 0)                                                               \
  X(11, POP, 0)                                                                \
  X(12, CALL, 0)                                                               \
  X(13, RET, 0)                                                                \
                                                                               \
  X(100, NEGATE, 0)                                                            \
  X(101, ADD, 1)                                                               \
  X(102, SUBTRACT, 1)                                                          \
  X(103, MULTIPLY, 1)                                                          \
  X(104, DIVIDE, 1)                                                            \
  X(105, REMAINDER, 1)                                                         \
                                                                               \
  X(106, ADD_I, 1)                                                             \
  X(107, SUBTRACT_I, 1)                                                        \
  X(108, MULTIPLY_I, 1)                                                        \
  X(109, DIVIDE_I, 1)                                                          \
  X(110, REMAINDER_I, 1)                                                       \
                                                                               \
  X(111, F_NEGATE, 1)                                                          \
  X(112, F_ADD, 1)                                                             \
  X(113, F_SUBTRACT, 1)                                                        \
  X(114, F_MULTIPLY, 1)                                                        \
  X(115, F_DIVIDE, 1)                                                          \
                                                                               \
  X(116, F_ADD_I, 1)                                                           \
  X(117, F_SUBTRACT_I, 1)                                                      \
  X(118, F_MULTIPLY_I, 1)                                                      \
  X(119, F_DIVIDE_I, 1)                                                        \
                                                                               \
  X(160, OR, 1)                                                                \
  X(161, AND, 1)                                                               \
  X(162, XOR, 1)                                                               \
  X(163, INVERT, 0)                                                            \
                                                                               \
  X(164, GT, 1)                                                                \
  X(165, LT, 1)                                                                \
  X(166, GTEQ, 1)                                                              \
  X(167, LTEQ, 1)                                                              \
  X(168, EQUALS, 1)                                                            \
  X(169, NOT, 0)                                                               \
                                                                               \
  X(170, OR_I, 1)                                                              \
  X(171, AND_I, 1)                                                             \
  X(172, XOR_I, 1)                                                             \
                                                                               \
  X(200, JUMP, 1)                                                              \
  X(201, BRANCH_IF_ZERO, 1)                                                    \
  X(202, BRANCH_IF_NOT_ZERO, 1)                                                \
                                                                               \
  X(255, INTERRUPT, 1)

// There is only one supported system:
#define SYSCALLS                                                               \
  X(0, READ)                                                                   \
  X(1, WRITE)

#define X(Opcode, Enum, Operands) Enum,
enum class op { OPERATIONS };
#undef X

#define X(Code, Enum) Enum,
enum class sys_call { SYSCALLS };
#undef X

const char *name_of_instruction(op operation);
std::uint8_t opcode_of_instruction(op operation);
std::uint8_t operand_count_of_instruction(op operation);

std::uint8_t code_of_syscall(sys_call call);

struct instruction_with_operands {
  op operation;
  std::uint64_t operands[4];

public:
  op get_operation();
  std::uint64_t get_operand(int index);

  size_t operand_count();
  std::vector<std::uint8_t> encode();

  friend std::ostream &operator<<(std::ostream &o,
                                  const instruction_with_operands &a);
};

#endif /* INSTRUCTIONS_H */
