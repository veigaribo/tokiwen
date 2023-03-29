#ifndef PROGRAM_H
#define PROGRAM_H

#include "parser/syntax/location.hpp"
#include "synthesis/instructions.h"
#include <cstdint>
#include <map>
#include <vector>

// Could be a reference to the symbol table but I think it's better to keep
// their lifetimes separate
struct variable_data {
  std::string name;
  std::uint64_t size;
  std::uint64_t address;
  yy::location declared_at;
};

struct program_metadata {
  std::map<std::uint64_t, variable_data> variables;
  std::vector<std::uint64_t> statement_boundaries;
  std::vector<std::uint64_t>
      source_line_map; // What line of code generated each instruction
};

class program {
public:
  std::vector<std::uint8_t> data;
  std::vector<instruction_with_operands> code;
  program_metadata metadata;
};

#endif /* PROGRAM_H */
