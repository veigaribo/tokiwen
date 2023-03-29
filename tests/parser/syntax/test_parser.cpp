#include "constraints.h" // EqualsRef
#include "parser/ast.h"
#include "parser/lex/scanner.hpp"
#include "parser/syntax/symbol_table_stack.h"
#include <bandit/bandit.h>

using namespace snowhouse;
using namespace bandit;

#define NODE(VarName, NodeName, ...)                                           \
  std::shared_ptr<ast_node> VarName =                                          \
      std::make_shared<NodeName##_node>(__VA_ARGS__, location)

#define STMT_NODE(VarName, NodeName, ...)                                      \
  std::shared_ptr<ast_node> VarName = std::make_shared<statement_node>(        \
      std::make_shared<NodeName##_node>(__VA_ARGS__, location), location)

#define NOOP(VarName)                                                          \
  std::shared_ptr<ast_node> VarName = std::make_shared<noop_node>(location)

#define VAR(Name, TypeEntry)                                                   \
  var_table_entry var_##Name##_entry;                                          \
  var_##Name##_entry.name = #Name;                                             \
  var_##Name##_entry.type = TypeEntry;                                         \
  NODE(var_##Name##_node, var_identifier, &var_##Name##_entry)

#define MAKE_VAR(Name, TypeEntry)                                              \
  auto maybe_var_##Name##_entry =                                              \
      root_symbol_table->insert_variable(#Name, yy::location(), TypeEntry);    \
                                                                               \
  AssertThat(maybe_var_##Name##_entry.has_value(), IsTrue());                  \
  auto var_##Name##_entry = maybe_var_##Name##_entry.value();                  \
  NODE(var_##Name##_node, var_identifier, var_##Name##_entry)

#define TYPE(Name, Value)                                                      \
  type_table_entry type_##Name##_entry;                                        \
  type_##Name##_entry.name = #Name;                                            \
  type_##Name##_entry.value = Value;                                           \
  NODE(type_##Name##_node, type_identifier, &type_##Name##_entry)

#define INIT_SYMBOL_TABLE                                                      \
  std::string prelude = "prelude";                                             \
  auto root_symbol_table = stbuilder.current();                                \
  auto location = yy::location(&prelude, 0, 0);                                \
  auto test_t = root_symbol_table->insert_type("test_t", location,             \
                                               std::make_shared<type_int>());  \
  auto test_var =                                                              \
      root_symbol_table->insert_variable("test_var", location, test_t.value())

#define INIT_PARSER(Input)                                                     \
  yy::scanner scanner(Input);                                                  \
  scanner.init_default_keywords();                                             \
  symbol_table_stack stbuilder;                                                \
  std::string message_recipient;                                               \
  INIT_SYMBOL_TABLE;                                                           \
  std::shared_ptr<ast_node> result;                                            \
  yy::parser parser(scanner, stbuilder, &result, &message_recipient)

#define PARSE(Input)                                                           \
  INIT_PARSER(Input);                                                          \
  auto ok = parser.parse()

#define DEBUG_PARSE(Input)                                                     \
  INIT_PARSER(Input);                                                          \
  parser.set_debug_level(1);                                                   \
  auto ok = parser.parse()

#define PARSE_SUCCESS(Input)                                                   \
  PARSE(Input);                                                                \
  AssertThat(ok, EqualsRef(0))

#define DEBUG_PARSE_SUCCESS(Input)                                             \
  DEBUG_PARSE(Input);                                                          \
  AssertThat(ok, EqualsRef(0))

#define PARSE_FAIL(Input)                                                      \
  PARSE(Input);                                                                \
  AssertThat(ok, EqualsRef(1))

#define TEST_LITERAL_EXPR_STATEMENT(Literal, Value, LiteralType, Name)         \
  it("parses expression statement: literal " #Name, [&]() {                    \
    PARSE_SUCCESS(" " #Literal "; ");                                          \
                                                                               \
    STMT_NODE(x_literal, LiteralType##_literal, Value);                        \
    NOOP(x_noop);                                                              \
    NODE(x_seq, sequence, x_literal, x_noop);                                  \
    NODE(x_block, block, root_symbol_table, x_seq);                            \
    AssertThat(*result, EqualsRef(*x_block));                                  \
  })

#define TEST_UNARY_EXPR_STATEMENT(Op, OpName)                                  \
  it("parses expression statement: simple " #OpName, [&]() {                   \
    PARSE_SUCCESS(#Op " 333;");                                                \
                                                                               \
    NODE(x_333, int_literal, 333);                                             \
    STMT_NODE(x_##OpName, OpName, x_333);                                      \
    NOOP(x_noop);                                                              \
    NODE(x_seq, sequence, x_##OpName, x_noop);                                 \
    NODE(x_block, block, root_symbol_table, x_seq);                            \
    AssertThat(*result, EqualsRef(*x_block));                                  \
  })

#define TEST_BIN_EXPR_STATEMENT_INTS(Op, OpName)                               \
  it("parses expression statement: simple ints " #OpName, [&]() {              \
    PARSE_SUCCESS("222 " #Op " 444;");                                         \
                                                                               \
    NODE(x_222, int_literal, 222);                                             \
    NODE(x_444, int_literal, 444);                                             \
    STMT_NODE(x_##OpName, OpName, x_222, x_444);                               \
    NOOP(x_noop);                                                              \
    NODE(x_seq, sequence, x_##OpName, x_noop);                                 \
    NODE(x_block, block, root_symbol_table, x_seq);                            \
    AssertThat(*result, EqualsRef(*x_block));                                  \
  })

#define TEST_BIN_EXPR_STATEMENT_BOOLEANS(Op, OpName)                           \
  it("parses expression statement: simple booleans " #OpName, [&]() {          \
    PARSE_SUCCESS("true " #Op " false;");                                      \
                                                                               \
    NODE(x_t, boolean_literal, true);                                          \
    NODE(x_f, boolean_literal, false);                                         \
    STMT_NODE(x_##OpName, OpName, x_t, x_f);                                   \
    NOOP(x_noop);                                                              \
    NODE(x_seq, sequence, x_##OpName, x_noop);                                 \
    NODE(x_block, block, root_symbol_table, x_seq);                            \
    AssertThat(*result, EqualsRef(*x_block));                                  \
  })

#define TEST_BIN_EXPR_STATEMENT_INT_TO_BOOLEANS(Op, OpName)                    \
  it("parses expression statement: ints to booleans " #OpName, [&]() {         \
    PARSE_SUCCESS("222 " #Op " 444;");                                         \
                                                                               \
    NODE(x_222, int_literal, 222);                                             \
    NODE(x_444, int_literal, 444);                                             \
    NODE(x_itob1, int_to_boolean_coercion, x_222);                             \
    NODE(x_itob2, int_to_boolean_coercion, x_444);                             \
    STMT_NODE(x_##OpName, OpName, x_itob1, x_itob2);                           \
    NOOP(x_noop);                                                              \
    NODE(x_seq, sequence, x_##OpName, x_noop);                                 \
    NODE(x_block, block, root_symbol_table, x_seq);                            \
    AssertThat(*result, EqualsRef(*x_block));                                  \
  })

#define TEST_BIN_EXPR_STATEMENT_FLOATS(Op, OpName)                             \
  it("parses expression statement: simple floats " #OpName, [&]() {            \
    PARSE_SUCCESS("2.22 " #Op " 44.4;");                                       \
                                                                               \
    NODE(x_222, float_literal, 2.22);                                          \
    NODE(x_444, float_literal, 44.4);                                          \
    STMT_NODE(x_##OpName, OpName, x_222, x_444);                               \
    NOOP(x_noop);                                                              \
    NODE(x_seq, sequence, x_##OpName, x_noop);                                 \
    NODE(x_block, block, root_symbol_table, x_seq);                            \
    AssertThat(*result, EqualsRef(*x_block));                                  \
  })

#define TEST_BIN_EXPR_STATEMENT_INT_FLOAT(Op, OpName)                          \
  it("parses expression statement: int and float " #OpName, [&]() {            \
    PARSE_SUCCESS("222 " #Op " 44.4;");                                        \
                                                                               \
    NODE(x_222, int_literal, 222);                                             \
    NODE(x_itof, int_to_float_coercion, x_222);                                \
    NODE(x_444, float_literal, 44.4);                                          \
    STMT_NODE(x_##OpName, OpName, x_itof, x_444);                              \
    NOOP(x_noop);                                                              \
    NODE(x_seq, sequence, x_##OpName, x_noop);                                 \
    NODE(x_block, block, root_symbol_table, x_seq);                            \
    AssertThat(*result, EqualsRef(*x_block));                                  \
  })

#define TEST_BIN_EXPR_STATEMENT_FLOAT_INT(Op, OpName)                          \
  it("parses expression statement: float and int " #OpName, [&]() {            \
    PARSE_SUCCESS("2.22 " #Op " 444;");                                        \
                                                                               \
    NODE(x_222, float_literal, 2.22);                                          \
    NODE(x_444, int_literal, 444);                                             \
    NODE(x_itof, int_to_float_coercion, x_444);                                \
    STMT_NODE(x_##OpName, OpName, x_222, x_itof);                              \
    NOOP(x_noop);                                                              \
    NODE(x_seq, sequence, x_##OpName, x_noop);                                 \
    NODE(x_block, block, root_symbol_table, x_seq);                            \
    AssertThat(*result, EqualsRef(*x_block));                                  \
  })

#define TEST_BIN_EXPR_STATEMENT_INT_BOOLEAN(Op, OpName)                        \
  it("parses expression statement: int and boolean " #OpName, [&]() {          \
    PARSE_SUCCESS("222 " #Op " true;");                                        \
                                                                               \
    NODE(x_222, int_literal, 222);                                             \
    NODE(x_true, boolean_literal, true);                                       \
    NODE(x_itof, boolean_to_int_coercion, x_true);                             \
    STMT_NODE(x_##OpName, OpName, x_222, x_itof);                              \
    NOOP(x_noop);                                                              \
    NODE(x_seq, sequence, x_##OpName, x_noop);                                 \
    NODE(x_block, block, root_symbol_table, x_seq);                            \
    AssertThat(*result, EqualsRef(*x_block));                                  \
  })

#define TEST_BIN_EXPR_STATEMENT_LEFT_ASSOC_INTS(Op, OpName)                    \
  it("parses expression statement: left associate " #OpName, [&]() {           \
    PARSE_SUCCESS("222 " #Op " 444 " #Op "666;");                              \
                                                                               \
    NODE(x_222, int_literal, 222);                                             \
    NODE(x_444, int_literal, 444);                                             \
    NODE(x_666, int_literal, 666);                                             \
    NODE(x_##OpName##1, OpName, x_222, x_444);                                 \
    STMT_NODE(x_##OpName##2, OpName, x_##OpName##1, x_666);                    \
    NOOP(x_noop);                                                              \
    NODE(x_seq, sequence, x_##OpName##2, x_noop);                              \
    NODE(x_block, block, root_symbol_table, x_seq);                            \
    AssertThat(*result, EqualsRef(*x_block));                                  \
  })

#define TEST_BIN_EXPR_STATEMENT_LEFT_ASSOC_BOOLEANS(Op, OpName)                \
  it("parses expression statement: left associate " #OpName, [&]() {           \
    PARSE_SUCCESS("true " #Op " false " #Op "true;");                          \
                                                                               \
    NODE(x_true1, boolean_literal, true);                                      \
    NODE(x_false, boolean_literal, false);                                     \
    NODE(x_true2, boolean_literal, true);                                      \
    NODE(x_##OpName##1, OpName, x_true1, x_false);                             \
    STMT_NODE(x_##OpName##2, OpName, x_##OpName##1, x_true2);                  \
    NOOP(x_noop);                                                              \
    NODE(x_seq, sequence, x_##OpName##2, x_noop);                              \
    NODE(x_block, block, root_symbol_table, x_seq);                            \
    AssertThat(*result, EqualsRef(*x_block));                                  \
  })

#define TEST_ASSIGNMENT(Op, OpName)                                            \
  it("parses operation assignment: " #OpName, [&]() {                          \
    PARSE_SUCCESS("test_var " #Op " 0;");                                      \
    VAR(test_var, test_t.value());                                             \
                                                                               \
    NODE(x_0, int_literal, 0);                                                 \
    STMT_NODE(x_##OpName, OpName, var_test_var_node, x_0);                     \
    NOOP(x_noop);                                                              \
    NODE(x_seq, sequence, x_##OpName, x_noop);                                 \
    NODE(x_block, block, root_symbol_table, x_seq);                            \
    AssertThat(*result, EqualsRef(*x_block));                                  \
  });                                                                          \
                                                                               \
  it("fails parsing operation assignment: " #OpName,                           \
     [&]() { PARSE_FAIL("jest" #Op " 0;"); });

#define TEST_ASSIGNMENT_ASSOC_RIGHT(Op, OpName)                                \
  it("parses operation assignment: " #OpName, [&]() {                          \
    INIT_PARSER("test_var " #Op " test_var2 " #Op " 0;");                      \
    VAR(test_var, test_t.value());                                             \
    VAR(test_var2, test_t.value());                                            \
                                                                               \
    root_symbol_table->insert_variable("test_var2", location, test_t.value()); \
    auto ok = parser.parse();                                                  \
                                                                               \
    NODE(x_0, int_literal, 0);                                                 \
    NODE(x_##OpName##2, OpName, var_test_var2_node, x_0);                      \
    STMT_NODE(x_##OpName##1, OpName, var_test_var_node, x_##OpName##2);        \
    NOOP(x_noop);                                                              \
    NODE(x_seq, sequence, x_##OpName##1, x_noop);                              \
    NODE(x_block, block, root_symbol_table, x_seq);                            \
    AssertThat(*result, EqualsRef(*x_block));                                  \
  });

go_bandit([]() {
  describe("parser", []() {
    it("parses nothing", [&]() {
      PARSE_SUCCESS("");

      NOOP(x_noop);
      NODE(x_block, block, root_symbol_table, x_noop);
      AssertThat(*result, EqualsRef(*x_block));
    });

    it("parses empty statement", [&]() {
      PARSE_SUCCESS(";");

      NOOP(x_noop);
      NODE(x_stmt_noop, statement, x_noop);
      NODE(x_seq, sequence, x_stmt_noop, x_noop);
      NODE(x_block, block, root_symbol_table, x_seq);
      AssertThat(*result, EqualsRef(*x_block));
    });

    TEST_LITERAL_EXPR_STATEMENT(1024, 1024, int, integer);
    TEST_LITERAL_EXPR_STATEMENT(3.1415, 3.1415, float, dotted float);
    TEST_LITERAL_EXPR_STATEMENT(3e5, 3e5, float, exponent float);
    TEST_LITERAL_EXPR_STATEMENT(6.02e23, 6.02e23, float, dotted exponent float);

    TEST_UNARY_EXPR_STATEMENT(+, unary_plus);
    TEST_UNARY_EXPR_STATEMENT(-, unary_minus);
    TEST_UNARY_EXPR_STATEMENT(!, not );

    TEST_BIN_EXPR_STATEMENT_INTS(+, sum);
    TEST_BIN_EXPR_STATEMENT_INTS(-, subtraction);
    TEST_BIN_EXPR_STATEMENT_INTS(*, multiplication);
    TEST_BIN_EXPR_STATEMENT_INTS(/, division);
    TEST_BIN_EXPR_STATEMENT_INTS(%, modulo);
    TEST_BIN_EXPR_STATEMENT_INTS(<, lt);
    TEST_BIN_EXPR_STATEMENT_INTS(>, gt);
    TEST_BIN_EXPR_STATEMENT_INTS(<=, lteq);
    TEST_BIN_EXPR_STATEMENT_INTS(>=, gteq);
    TEST_BIN_EXPR_STATEMENT_INTS(==, equals);
    TEST_BIN_EXPR_STATEMENT_INTS(!=, nequals);

    TEST_BIN_EXPR_STATEMENT_BOOLEANS(&&, and);
    TEST_BIN_EXPR_STATEMENT_BOOLEANS(||, or);

    TEST_BIN_EXPR_STATEMENT_INT_TO_BOOLEANS(&&, and);
    TEST_BIN_EXPR_STATEMENT_INT_TO_BOOLEANS(||, or);

    TEST_BIN_EXPR_STATEMENT_FLOATS(+, sum);
    TEST_BIN_EXPR_STATEMENT_FLOATS(-, subtraction);
    TEST_BIN_EXPR_STATEMENT_FLOATS(*, multiplication);
    TEST_BIN_EXPR_STATEMENT_FLOATS(/, division);
    TEST_BIN_EXPR_STATEMENT_FLOATS(<, lt);
    TEST_BIN_EXPR_STATEMENT_FLOATS(>, gt);
    TEST_BIN_EXPR_STATEMENT_FLOATS(<=, lteq);
    TEST_BIN_EXPR_STATEMENT_FLOATS(>=, gteq);
    TEST_BIN_EXPR_STATEMENT_FLOATS(==, equals);
    TEST_BIN_EXPR_STATEMENT_FLOATS(!=, nequals);

    TEST_BIN_EXPR_STATEMENT_INT_FLOAT(+, sum);
    TEST_BIN_EXPR_STATEMENT_INT_FLOAT(-, subtraction);
    TEST_BIN_EXPR_STATEMENT_INT_FLOAT(*, multiplication);
    TEST_BIN_EXPR_STATEMENT_INT_FLOAT(/, division);
    TEST_BIN_EXPR_STATEMENT_INT_FLOAT(<, lt);
    TEST_BIN_EXPR_STATEMENT_INT_FLOAT(>, gt);
    TEST_BIN_EXPR_STATEMENT_INT_FLOAT(<=, lteq);
    TEST_BIN_EXPR_STATEMENT_INT_FLOAT(>=, gteq);
    TEST_BIN_EXPR_STATEMENT_INT_FLOAT(==, equals);
    TEST_BIN_EXPR_STATEMENT_INT_FLOAT(!=, nequals);

    TEST_BIN_EXPR_STATEMENT_FLOAT_INT(+, sum);
    TEST_BIN_EXPR_STATEMENT_FLOAT_INT(-, subtraction);
    TEST_BIN_EXPR_STATEMENT_FLOAT_INT(*, multiplication);
    TEST_BIN_EXPR_STATEMENT_FLOAT_INT(/, division);
    TEST_BIN_EXPR_STATEMENT_FLOAT_INT(<, lt);
    TEST_BIN_EXPR_STATEMENT_FLOAT_INT(>, gt);
    TEST_BIN_EXPR_STATEMENT_FLOAT_INT(<=, lteq);
    TEST_BIN_EXPR_STATEMENT_FLOAT_INT(>=, gteq);
    TEST_BIN_EXPR_STATEMENT_FLOAT_INT(==, equals);
    TEST_BIN_EXPR_STATEMENT_FLOAT_INT(!=, nequals);

    TEST_BIN_EXPR_STATEMENT_INT_BOOLEAN(+, sum);
    TEST_BIN_EXPR_STATEMENT_INT_BOOLEAN(-, subtraction);
    TEST_BIN_EXPR_STATEMENT_INT_BOOLEAN(*, multiplication);
    TEST_BIN_EXPR_STATEMENT_INT_BOOLEAN(/, division);
    TEST_BIN_EXPR_STATEMENT_INT_BOOLEAN(<, lt);
    TEST_BIN_EXPR_STATEMENT_INT_BOOLEAN(>, gt);
    TEST_BIN_EXPR_STATEMENT_INT_BOOLEAN(<=, lteq);
    TEST_BIN_EXPR_STATEMENT_INT_BOOLEAN(>=, gteq);
    TEST_BIN_EXPR_STATEMENT_INT_BOOLEAN(==, equals);
    TEST_BIN_EXPR_STATEMENT_INT_BOOLEAN(!=, nequals);

    TEST_BIN_EXPR_STATEMENT_LEFT_ASSOC_INTS(+, sum);
    TEST_BIN_EXPR_STATEMENT_LEFT_ASSOC_INTS(-, subtraction);
    TEST_BIN_EXPR_STATEMENT_LEFT_ASSOC_INTS(*, multiplication);
    TEST_BIN_EXPR_STATEMENT_LEFT_ASSOC_INTS(/, division);
    TEST_BIN_EXPR_STATEMENT_LEFT_ASSOC_INTS(%, modulo);
    TEST_BIN_EXPR_STATEMENT_LEFT_ASSOC_BOOLEANS(<, lt);
    TEST_BIN_EXPR_STATEMENT_LEFT_ASSOC_BOOLEANS(>, gt);
    TEST_BIN_EXPR_STATEMENT_LEFT_ASSOC_BOOLEANS(<=, lteq);
    TEST_BIN_EXPR_STATEMENT_LEFT_ASSOC_BOOLEANS(>=, gteq);
    TEST_BIN_EXPR_STATEMENT_LEFT_ASSOC_BOOLEANS(==, equals);
    TEST_BIN_EXPR_STATEMENT_LEFT_ASSOC_BOOLEANS(!=, nequals);
    TEST_BIN_EXPR_STATEMENT_LEFT_ASSOC_BOOLEANS(&&, and);
    TEST_BIN_EXPR_STATEMENT_LEFT_ASSOC_BOOLEANS(||, or);

    it("parses expression statement: parenthesis", [&]() {
      PARSE_SUCCESS("11 * (55 + 66);");

      NODE(x_11, int_literal, 11);
      NODE(x_55, int_literal, 55);
      NODE(x_66, int_literal, 66);
      NODE(x_sum, sum, x_55, x_66);
      STMT_NODE(x_mult, multiplication, x_11, x_sum);
      NOOP(x_noop);
      NODE(x_seq, sequence, x_mult, x_noop);
      NODE(x_block, block, root_symbol_table, x_seq);
      AssertThat(*result, EqualsRef(*x_block));
    });

    it("parses variable declaration", [&]() {
      PARSE_SUCCESS("test_t jest;");
      TYPE(test_t, std::make_shared<type_int>());
      VAR(jest, test_t.value());

      STMT_NODE(x_decl, declaration, type_test_t_node, var_jest_node);
      NOOP(x_noop);
      NODE(x_seq, sequence, x_decl, x_noop);
      NODE(x_block, block, root_symbol_table, x_seq);
      AssertThat(*result, EqualsRef(*x_block));

      auto block = std::dynamic_pointer_cast<block_node>(result);
      AssertThat(block->table->get_var("jest").has_value(), Is().True());
    });

    it("fails parsing variable declaration: undefined type",
       [&]() { PARSE_FAIL("idk_t jest;"); });

    it("fails parsing variable declaration: variable redeclaration", [&]() {
      INIT_PARSER("test_t x;");
      root_symbol_table->insert_variable("x", location, test_t.value());
      auto ok = parser.parse();
      AssertThat(ok, Equals(1));
    });

    it("parses variable declaration: type and name the same", [&]() {
      PARSE_SUCCESS("test_t test_t;");
      TYPE(test_t, std::make_shared<type_int>());
      VAR(test_t, test_t.value());

      STMT_NODE(x_decl, declaration, type_test_t_node, var_test_t_node);
      NOOP(x_noop);
      NODE(x_seq, sequence, x_decl, x_noop);
      NODE(x_block, block, root_symbol_table, x_seq);
      AssertThat(*result, EqualsRef(*x_block));
    });

    it("parses variable declaration & assignment", [&]() {
      PARSE_SUCCESS("test_t n1 = 0;");
      TYPE(test_t, std::make_shared<type_int>());
      VAR(n1, test_t.value());

      NODE(x_lit, int_literal, 0);
      STMT_NODE(x_decl_assign, declaration_assignment, type_test_t_node,
                var_n1_node, x_lit);
      NOOP(x_noop);
      NODE(x_seq, sequence, x_decl_assign, x_noop);
      NODE(x_block, block, root_symbol_table, x_seq);
      AssertThat(*result, EqualsRef(*x_block));
    });

    TEST_ASSIGNMENT(=, assignment);
    TEST_ASSIGNMENT(+=, sum_assignment);
    TEST_ASSIGNMENT(-=, subtraction_assignment);
    TEST_ASSIGNMENT(*=, multiplication_assignment);
    TEST_ASSIGNMENT(/=, division_assignment);
    TEST_ASSIGNMENT(%=, modulo_assignment);

    TEST_ASSIGNMENT_ASSOC_RIGHT(=, assignment);
    TEST_ASSIGNMENT_ASSOC_RIGHT(+=, sum_assignment);
    TEST_ASSIGNMENT_ASSOC_RIGHT(-=, subtraction_assignment);
    TEST_ASSIGNMENT_ASSOC_RIGHT(*=, multiplication_assignment);
    TEST_ASSIGNMENT_ASSOC_RIGHT(/=, division_assignment);
    TEST_ASSIGNMENT_ASSOC_RIGHT(%=, modulo_assignment);

    it("parses blocks", [&]() {
      PARSE_SUCCESS("{ 5; }");

      STMT_NODE(x_5, int_literal, 5);
      NOOP(x_noop);
      NODE(x_inner_seq, sequence, x_5, x_noop);
      NODE(x_inner_block, block, root_symbol_table, x_inner_seq);
      NODE(x_seq, sequence, x_inner_block, x_noop);
      NODE(x_block, block, root_symbol_table, x_seq);
      AssertThat(*result, EqualsRef(*x_block));
    });

    it("parses nested blocks", [&]() {
      PARSE_SUCCESS("{ 1; { 2; } 3; }");

      STMT_NODE(x_1, int_literal, 1);
      STMT_NODE(x_2, int_literal, 2);
      STMT_NODE(x_3, int_literal, 3);
      NOOP(x_noop);
      NODE(x_innermost_seq, sequence, x_2, x_noop);
      NODE(x_innermost_block, block, root_symbol_table, x_innermost_seq);
      NODE(x_seq3, sequence, x_3, x_noop);
      NODE(x_seq2, sequence, x_innermost_block, x_seq3);
      NODE(x_seq1, sequence, x_1, x_seq2);
      NODE(x_outerinner_block, block, root_symbol_table, x_seq1);
      NODE(x_outer_seq, sequence, x_outerinner_block, x_noop);
      NODE(x_block, block, root_symbol_table, x_outer_seq);
      AssertThat(*result, EqualsRef(*x_block));
    });

    it("parses variable declarations in blocks", [&]() {
      PARSE_SUCCESS("{ test_t plato; }");
      TYPE(test_t, std::make_shared<type_int>());
      VAR(plato, test_t.value());

      NOOP(x_noop);
      STMT_NODE(x_decl, declaration, type_test_t_node, var_plato_node);
      NODE(x_inner_seq, sequence, x_decl, x_noop);
      // Equals does not really check symbol tables so whatever
      NODE(x_inner_block, block, root_symbol_table, x_inner_seq);
      NODE(x_outer_seq, sequence, x_inner_block, x_noop);
      NODE(x_block, block, root_symbol_table, x_outer_seq);
      AssertThat(*result, EqualsRef(*x_block));

      auto outer_block_node = std::dynamic_pointer_cast<block_node>(result);
      auto outer_seq_node = std::dynamic_pointer_cast<sequence_node>(
          outer_block_node->children[0]);
      auto inner_block_node =
          std::dynamic_pointer_cast<block_node>(outer_seq_node->children[0]);
      auto definitely_not_plato = outer_block_node->table->get_var("plato");
      auto hopefully_plato = inner_block_node->table->get_var("plato");

      AssertThat(definitely_not_plato.has_value(), IsFalse());
      AssertThat(hopefully_plato.has_value(), IsTrue());
    });

    it("checks for variables in outer blocks", [&]() {
      PARSE_SUCCESS("test_t shadow; { shadow; }");
      TYPE(test_t, std::make_shared<type_int>());
      VAR(shadow, test_t.value());

      NOOP(x_noop);
      STMT_NODE(x_decl, declaration, type_test_t_node, var_shadow_node);
      STMT_NODE(x_inner_var, var_identifier, &var_shadow_entry);
      NODE(x_inner_seq, sequence, x_inner_var, x_noop);
      NODE(x_inner_block, block, root_symbol_table, x_inner_seq);
      NODE(x_outer_seq2, sequence, x_inner_block, x_noop);
      NODE(x_outer_seq1, sequence, x_decl, x_outer_seq2);
      NODE(x_block, block, root_symbol_table, x_outer_seq1);
      AssertThat(*result, EqualsRef(*x_block));
    });

    it("shadows variables from outer blocks", [&]() {
      INIT_PARSER("test_t shadow; { test_t2 shadow; }");
      TYPE(test_t, std::make_shared<type_int>());
      TYPE(test_t2, std::make_shared<type_int>());
      root_symbol_table->insert_type("test_t2", location,
                                     type_test_t2_entry.value);
      VAR(shadow, test_t.value());

      auto ok = parser.parse();
      AssertThat(ok, Equals(0));

      NOOP(x_noop);
      STMT_NODE(x_outer_decl, declaration, type_test_t_node, var_shadow_node);
      STMT_NODE(x_inner_decl, declaration, type_test_t2_node, var_shadow_node);
      NODE(x_inner_seq, sequence, x_inner_decl, x_noop);
      NODE(x_inner_block, block, root_symbol_table, x_inner_seq);
      NODE(x_outer_seq2, sequence, x_inner_block, x_noop);
      NODE(x_outer_seq1, sequence, x_outer_decl, x_outer_seq2);
      NODE(x_block, block, root_symbol_table, x_outer_seq1);
      AssertThat(*result, EqualsRef(*x_block));

      auto outer_block_node = std::dynamic_pointer_cast<block_node>(result);
      auto outer_seq1_node = std::dynamic_pointer_cast<sequence_node>(
          outer_block_node->children[0]);
      auto outer_seq2_node = std::dynamic_pointer_cast<sequence_node>(
          outer_seq1_node->children[1]);
      auto inner_block_node =
          std::dynamic_pointer_cast<block_node>(outer_seq2_node->children[0]);

      auto outer_shadow = outer_block_node->table->get_var("shadow");
      auto inner_shadow = inner_block_node->table->get_var("shadow");

      AssertThat(outer_shadow.has_value(), IsTrue());
      AssertThat(inner_shadow.has_value(), IsTrue());

      auto outer_shadow_value = outer_shadow.value();
      auto inner_shadow_value = inner_shadow.value();

      AssertThat(outer_shadow_value->type->name, Equals("test_t"));
      AssertThat(inner_shadow_value->type->name, Equals("test_t2"));
    });

    it("checks for variables in outer blocks: fail",
       [&]() { PARSE_FAIL("{ shadow; }"); });

    it("parses conditional: simple", [&]() {
      PARSE_SUCCESS("if (true) 2;");

      NODE(x_t, boolean_literal, true);
      STMT_NODE(x_2, int_literal, 2);
      NOOP(x_noop);
      STMT_NODE(x_if, conditional, x_t, x_2, x_noop);
      NODE(x_seq, sequence, x_if, x_noop);
      NODE(x_block, block, root_symbol_table, x_seq);
      AssertThat(*result, EqualsRef(*x_block));
    });

    it("parses conditional: with block", [&]() {
      PARSE_SUCCESS("if (true) { 2; }");

      NODE(x_t, boolean_literal, true);
      STMT_NODE(x_2, int_literal, 2);
      NOOP(x_noop);
      NODE(x_if_block_seq, sequence, x_2, x_noop);
      NODE(x_if_block, block, root_symbol_table, x_if_block_seq);
      STMT_NODE(x_if, conditional, x_t, x_if_block, x_noop);
      NODE(x_seq, sequence, x_if, x_noop);
      NODE(x_block, block, root_symbol_table, x_seq);
      AssertThat(*result, EqualsRef(*x_block));
    });

    it("parses conditional: with else", [&]() {
      PARSE_SUCCESS("if (true) 2; else 3;");

      NODE(x_t, boolean_literal, true);
      STMT_NODE(x_2, int_literal, 2);
      STMT_NODE(x_3, int_literal, 3);
      NOOP(x_noop);
      STMT_NODE(x_if, conditional, x_t, x_2, x_3);
      NODE(x_seq, sequence, x_if, x_noop);
      NODE(x_block, block, root_symbol_table, x_seq);
      AssertThat(*result, EqualsRef(*x_block));
    });

    it("parses conditional: dangling else", [&]() {
      PARSE_SUCCESS("if (false) if (true) 3; else 4;");

      NODE(x_f, boolean_literal, false);
      NODE(x_t, boolean_literal, true);
      STMT_NODE(x_3, int_literal, 3);
      STMT_NODE(x_4, int_literal, 4);
      NOOP(x_noop);
      STMT_NODE(x_if2, conditional, x_t, x_3, x_4);
      STMT_NODE(x_if1, conditional, x_f, x_if2, x_noop);
      NODE(x_seq, sequence, x_if1, x_noop);
      NODE(x_block, block, root_symbol_table, x_seq);
      AssertThat(*result, EqualsRef(*x_block));
    });

    it("parses conditional: coercion to boolean", [&]() {
      PARSE_SUCCESS("if (1) 2;");

      NODE(x_1, int_literal, 1);
      NODE(x_itob, int_to_boolean_coercion, x_1);
      STMT_NODE(x_2, int_literal, 2);
      NOOP(x_noop);
      STMT_NODE(x_if, conditional, x_itob, x_2, x_noop);
      NODE(x_seq, sequence, x_if, x_noop);
      NODE(x_block, block, root_symbol_table, x_seq);
      AssertThat(*result, EqualsRef(*x_block));
    });

    it("while loop: simple", [&]() {
      PARSE_SUCCESS("while(true) 2;");

      NODE(x_true, boolean_literal, true);
      STMT_NODE(x_2, int_literal, 2);
      NOOP(x_noop);
      STMT_NODE(x_while, while_loop, x_true, x_2);
      NODE(x_seq, sequence, x_while, x_noop);
      NODE(x_block, block, root_symbol_table, x_seq);
      AssertThat(*result, EqualsRef(*x_block));
    });

    it("label: simple", [&]() {
      PARSE_SUCCESS("start: 10;");

      STMT_NODE(x_10, int_literal, 10);
      NODE(x_start, label, "start");
      NOOP(x_noop);
      NODE(x_seq1, sequence, x_10, x_noop);
      NODE(x_seq2, sequence, x_start, x_seq1);
      NODE(x_block, block, root_symbol_table, x_seq2);
      AssertThat(*result, EqualsRef(*x_block));
    });

    it("goto: simple", [&]() {
      PARSE_SUCCESS("goto start;");

      STMT_NODE(x_goto, goto, "start");
      NOOP(x_noop);
      NODE(x_seq, sequence, x_goto, x_noop);
      NODE(x_block, block, root_symbol_table, x_seq);
      AssertThat(*result, EqualsRef(*x_block));
    });
  });
});
