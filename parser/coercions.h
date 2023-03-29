#ifndef COERCIONS_H
#define COERCIONS_H

#include "parser/ast.h"

bool can_coerce_to(type &t1, type &t2);

std::shared_ptr<ast_node>
coerced_conditional(std::shared_ptr<ast_node> condition,
                    std::shared_ptr<ast_node> if_body,
                    std::shared_ptr<ast_node> else_body, yy::location loc);

std::shared_ptr<ast_node> coerced_while(std::shared_ptr<ast_node> condition,
                                        std::shared_ptr<ast_node> body,
                                        yy::location loc);

std::shared_ptr<ast_node> coerced_sum(std::shared_ptr<ast_node> left,
                                      std::shared_ptr<ast_node> right,
                                      yy::location loc);
std::shared_ptr<ast_node> coerced_subtraction(std::shared_ptr<ast_node> left,
                                              std::shared_ptr<ast_node> right,
                                              yy::location loc);
std::shared_ptr<ast_node>
coerced_multiplication(std::shared_ptr<ast_node> left,
                       std::shared_ptr<ast_node> right, yy::location loc);

std::shared_ptr<ast_node> coerced_division(std::shared_ptr<ast_node> left,
                                           std::shared_ptr<ast_node> right,
                                           yy::location loc);
std::shared_ptr<ast_node> coerced_modulo(std::shared_ptr<ast_node> left,
                                         std::shared_ptr<ast_node> right,
                                         yy::location loc);
std::shared_ptr<ast_node> coerced_lt(std::shared_ptr<ast_node> left,
                                     std::shared_ptr<ast_node> right,
                                     yy::location loc);
std::shared_ptr<ast_node> coerced_gt(std::shared_ptr<ast_node> left,
                                     std::shared_ptr<ast_node> right,
                                     yy::location loc);
std::shared_ptr<ast_node> coerced_lteq(std::shared_ptr<ast_node> left,
                                       std::shared_ptr<ast_node> right,
                                       yy::location loc);
std::shared_ptr<ast_node> coerced_gteq(std::shared_ptr<ast_node> left,
                                       std::shared_ptr<ast_node> right,
                                       yy::location loc);
std::shared_ptr<ast_node> coerced_equals(std::shared_ptr<ast_node> left,
                                         std::shared_ptr<ast_node> right,
                                         yy::location loc);
std::shared_ptr<ast_node> coerced_nequals(std::shared_ptr<ast_node> left,
                                          std::shared_ptr<ast_node> right,
                                          yy::location loc);
std::shared_ptr<ast_node> coerced_or(std::shared_ptr<ast_node> left,
                                     std::shared_ptr<ast_node> right,
                                     yy::location loc);
std::shared_ptr<ast_node> coerced_and(std::shared_ptr<ast_node> left,
                                      std::shared_ptr<ast_node> right,
                                      yy::location loc);

#endif /* COERCIONS_H */
