#include "parser/coercions.h"
#include "common/strings.h"
#include "parser/syntax/parser.hpp"
#include "parser/types.h"

bool can_coerce_to_boolean(type &t1) {
  return t1.kind == type_kind::INT || t1.kind == type_kind::POINTER;
}

std::shared_ptr<ast_node> to_boolean_coerce(type &from,
                                            std::shared_ptr<ast_node> node,
                                            yy::location loc) {
  if (from.kind == type_kind::INT) {
    return std::make_shared<int_to_boolean_coercion_node>(node, loc);
  }

  if (from.kind == type_kind::POINTER) {
    return std::make_shared<pointer_to_boolean_coercion_node>(node, loc);
  }

  return node;
}

bool can_integral_coerce_to(type &t1, type &t2) {
  if (t1.kind == type_kind::BOOLEAN) {
    if (t2.kind == type_kind::INT)
      return true;
  }

  return false;
}

bool can_arithmetic_coerce_to(type &t1, type &t2) {
  if (can_integral_coerce_to(t1, t2))
    return true;

  if (t1.kind == type_kind::INT) {
    if (t2.kind == type_kind::FLOAT)
      return true;
  }

  return false;
}

bool can_coerce_to(type &t1, type &t2) {
  return can_arithmetic_coerce_to(t1, t2);
}

std::shared_ptr<ast_node> integral_coerce(type &from, type &to,
                                          std::shared_ptr<ast_node> node,
                                          yy::location loc) {
  if (from.kind == type_kind::BOOLEAN) {
    if (to.kind == type_kind::INT)
      return std::make_shared<boolean_to_int_coercion_node>(node, loc);
  }

  return node;
}

std::shared_ptr<ast_node> arithmetic_coerce(type &from, type &to,
                                            std::shared_ptr<ast_node> node,
                                            yy::location loc) {
  node = integral_coerce(from, to, node, loc);

  if (from.kind == type_kind::INT) {
    if (to.kind == type_kind::FLOAT)
      return std::make_shared<int_to_float_coercion_node>(node, loc);
  }

  // shouldn't happen
  return node;
}

std::shared_ptr<ast_node>
coerce(type &from, type &to, std::shared_ptr<ast_node> node, yy::location loc) {
  return arithmetic_coerce(from, to, node, loc);
}

std::pair<std::shared_ptr<ast_node>, std::shared_ptr<ast_node>>
coerce_arithmetic_bin_op(std::shared_ptr<ast_node> left,
                         std::shared_ptr<ast_node> right, yy::location loc) {
  type &left_type = *left->typ, &right_type = *right->typ;

  if (!is_arithmetic(left_type))
    throw yy::parser::syntax_error(
        loc, "Tried to perform arithmetic binary operation with non-arithmetic "
             "argument of type " +
                 to_string(left_type));

  if (!is_arithmetic(right_type))
    throw yy::parser::syntax_error(
        loc, "Tried to perform arithmetic binary operation with non-arithmetic "
             "argument of type " +
                 to_string(right_type));

  if (left_type.matches(right_type)) {
    return std::pair{left, right};
  }

  if (can_arithmetic_coerce_to(left_type, right_type)) {
    return std::pair{arithmetic_coerce(left_type, right_type, left, loc),
                     right};
  }

  if (can_arithmetic_coerce_to(right_type, left_type)) {
    return std::pair{left,
                     arithmetic_coerce(right_type, left_type, right, loc)};
  }

  throw yy::parser::syntax_error(
      loc,
      "Can't coerce types " + to_string(left_type) + " and " +
          to_string(right_type) +
          " to match arithmetic operation. Maybe try casting them explicitly");
}

std::pair<std::shared_ptr<ast_node>, std::shared_ptr<ast_node>>
coerce_integral_bin_op(std::shared_ptr<ast_node> left,
                       std::shared_ptr<ast_node> right, yy::location loc) {
  type &left_type = *left->typ, &right_type = *right->typ;

  if (!is_integral(left_type))
    throw yy::parser::syntax_error(
        loc, "Tried to perform integral binary operation with non-integral "
             "argument of type " +
                 to_string(left_type));

  if (!is_integral(right_type))
    throw yy::parser::syntax_error(
        loc, "Tried to perform integral binary operation with non-integral "
             "argument of type " +
                 to_string(right_type));

  if (left_type.matches(right_type)) {
    return std::pair{left, right};
  }

  if (can_integral_coerce_to(left_type, right_type)) {
    return std::pair{integral_coerce(left_type, right_type, left, loc), right};
  }

  if (can_integral_coerce_to(right_type, left_type)) {
    return std::pair{left, integral_coerce(right_type, left_type, right, loc)};
  }

  throw yy::parser::syntax_error(
      loc,
      "Can't coerce types " + to_string(left_type) + " and " +
          to_string(right_type) +
          " to match integral operation. Maybe try casting them explicitly");
}

std::pair<std::shared_ptr<ast_node>, std::shared_ptr<ast_node>>
coerce_bin_op(std::shared_ptr<ast_node> left, std::shared_ptr<ast_node> right,
              yy::location loc) {
  type &left_type = *left->typ, &right_type = *right->typ;

  if (left_type.matches(right_type)) {
    return std::pair{left, right};
  }

  if (can_coerce_to(left_type, right_type)) {
    return std::pair{coerce(left_type, right_type, left, loc), right};
  }

  if (can_coerce_to(right_type, left_type)) {
    return std::pair{left, coerce(right_type, left_type, right, loc)};
  }

  throw yy::parser::syntax_error(
      loc, "Can't coerce types " + to_string(left_type) + " and " +
               to_string(right_type) +
               " to match. Maybe try casting them explicitly");
}

std::shared_ptr<ast_node> coerce_to_boolean(std::shared_ptr<ast_node> node,
                                            yy::location loc) {
  type &typ = *node->typ;
  auto b = std::make_unique<type_boolean>();

  if (typ.matches(*b)) {
    return node;
  }

  if (can_coerce_to_boolean(typ)) {
    return to_boolean_coerce(typ, node, loc);
  }

  throw yy::parser::syntax_error(loc, "Can't coerce type " + to_string(typ) +
                                          " to boolean");
}

std::shared_ptr<ast_node>
coerced_conditional(std::shared_ptr<ast_node> condition,
                    std::shared_ptr<ast_node> if_body,
                    std::shared_ptr<ast_node> else_body, yy::location loc) {
  auto condition2 = coerce_to_boolean(condition, loc);
  return std::make_shared<conditional_node>(condition2, if_body, else_body,
                                            loc);
}

std::shared_ptr<ast_node> coerced_while(std::shared_ptr<ast_node> condition,
                                        std::shared_ptr<ast_node> body,
                                        yy::location loc) {
  auto condition2 = coerce_to_boolean(condition, loc);
  return std::make_shared<while_loop_node>(condition2, body, loc);
}

std::shared_ptr<ast_node> coerced_sum(std::shared_ptr<ast_node> left,
                                      std::shared_ptr<ast_node> right,
                                      yy::location loc) {
  auto operands = coerce_arithmetic_bin_op(left, right, loc);
  return std::make_shared<sum_node>(operands.first, operands.second, loc);
}

std::shared_ptr<ast_node> coerced_subtraction(std::shared_ptr<ast_node> left,
                                              std::shared_ptr<ast_node> right,
                                              yy::location loc) {
  auto operands = coerce_arithmetic_bin_op(left, right, loc);
  return std::make_shared<subtraction_node>(operands.first, operands.second,
                                            loc);
}

std::shared_ptr<ast_node>
coerced_multiplication(std::shared_ptr<ast_node> left,
                       std::shared_ptr<ast_node> right, yy::location loc) {
  auto operands = coerce_arithmetic_bin_op(left, right, loc);
  return std::make_shared<multiplication_node>(operands.first, operands.second,
                                               loc);
}

std::shared_ptr<ast_node> coerced_division(std::shared_ptr<ast_node> left,
                                           std::shared_ptr<ast_node> right,
                                           yy::location loc) {
  auto operands = coerce_arithmetic_bin_op(left, right, loc);
  return std::make_shared<division_node>(operands.first, operands.second, loc);
}

std::shared_ptr<ast_node> coerced_modulo(std::shared_ptr<ast_node> left,
                                         std::shared_ptr<ast_node> right,
                                         yy::location loc) {
  auto operands = coerce_integral_bin_op(left, right, loc);
  return std::make_shared<modulo_node>(operands.first, operands.second, loc);
}

std::shared_ptr<ast_node> coerced_lt(std::shared_ptr<ast_node> left,
                                     std::shared_ptr<ast_node> right,
                                     yy::location loc) {
  auto operands = coerce_bin_op(left, right, loc);
  return std::make_shared<lt_node>(operands.first, operands.second, loc);
}

std::shared_ptr<ast_node> coerced_gt(std::shared_ptr<ast_node> left,
                                     std::shared_ptr<ast_node> right,
                                     yy::location loc) {
  auto operands = coerce_bin_op(left, right, loc);
  return std::make_shared<gt_node>(operands.first, operands.second, loc);
}

std::shared_ptr<ast_node> coerced_lteq(std::shared_ptr<ast_node> left,
                                       std::shared_ptr<ast_node> right,
                                       yy::location loc) {
  auto operands = coerce_bin_op(left, right, loc);
  return std::make_shared<lteq_node>(operands.first, operands.second, loc);
}

std::shared_ptr<ast_node> coerced_gteq(std::shared_ptr<ast_node> left,
                                       std::shared_ptr<ast_node> right,
                                       yy::location loc) {
  auto operands = coerce_bin_op(left, right, loc);
  return std::make_shared<gteq_node>(operands.first, operands.second, loc);
}

std::shared_ptr<ast_node> coerced_equals(std::shared_ptr<ast_node> left,
                                         std::shared_ptr<ast_node> right,
                                         yy::location loc) {
  auto operands = coerce_bin_op(left, right, loc);
  return std::make_shared<equals_node>(operands.first, operands.second, loc);
}

std::shared_ptr<ast_node> coerced_nequals(std::shared_ptr<ast_node> left,
                                          std::shared_ptr<ast_node> right,
                                          yy::location loc) {
  auto operands = coerce_bin_op(left, right, loc);
  return std::make_shared<nequals_node>(operands.first, operands.second, loc);
}

std::shared_ptr<ast_node> coerced_or(std::shared_ptr<ast_node> left,
                                     std::shared_ptr<ast_node> right,
                                     yy::location loc) {
  auto left2 = coerce_to_boolean(left, loc);
  auto right2 = coerce_to_boolean(right, loc);
  return std::make_shared<or_node>(left2, right2, loc);
}

std::shared_ptr<ast_node> coerced_and(std::shared_ptr<ast_node> left,
                                      std::shared_ptr<ast_node> right,
                                      yy::location loc) {
  auto left2 = coerce_to_boolean(left, loc);
  auto right2 = coerce_to_boolean(right, loc);
  return std::make_shared<and_node>(left2, right2, loc);
}
