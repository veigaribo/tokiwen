#ifdef __EMSCRIPTEN__

#include <emscripten/bind.h>
#include <parser/facade.h>
#include <synthesis/compiler.h>

using namespace emscripten;

std::string name_of_instruction_wrapper(op operation) {
  return name_of_instruction(operation);
}

EMSCRIPTEN_BINDINGS(synthesis) {
  auto op_b = enum_<op>("Op");

  for (size_t i = (size_t)op::NOOP; i <= (size_t)op::INTERRUPT; ++i) {
    auto enum_value = (op)i;
    op_b.value(name_of_instruction(enum_value), enum_value);
  }

  function("nameOfInstruction", &name_of_instruction_wrapper);
  function("opcodeOfInstruction", &opcode_of_instruction);
  function("operandCountOfInstruction", &operand_count_of_instruction);

  class_<instruction_with_operands>("Instruction")
      .function("getOperation", &instruction_with_operands::get_operation)
      .function("getOperand", &instruction_with_operands::get_operand)
      .function("encode", &instruction_with_operands::encode);

  register_vector<std::uint8_t>("Vector<UInt8>");
  register_vector<std::uint64_t>("Vector<UInt64>");
  register_vector<instruction_with_operands>("Vector<Instruction>");
  register_map<std::uint64_t, variable_data>("Map<UInt64, Variable>");

  class_<yy::position>("Position")
      .property("column", &yy::position::column)
      .property("line", &yy::position::line);

  class_<yy::location>("Location")
      .property("begin", &yy::location::begin)
      .property("end", &yy::location::end);

  class_<variable_data>("Variable")
      .property("name", &variable_data::name)
      .property("size", &variable_data::size)
      .property("address", &variable_data::address)
      .property("declaredAt", &variable_data::declared_at);

  class_<program_metadata>("ProgramMetadata")
      .property("variables", &program_metadata::variables)
      .property("statementBoundaries", &program_metadata::statement_boundaries)
      .property("sourceLineMap", &program_metadata::source_line_map);

  class_<program>("Program")
      .property("data", &program::data)
      .property("code", &program::code)
      .property("metadata", &program::metadata);

  class_<compiler>("Compiler")
      .constructor<>()
      .function("compile", &compiler::compile);
}

#endif
