#ifndef CONSTRAINTS_H
#define CONSTRAINTS_H

#include "bandit/assertion_frameworks/snowhouse/constraints/expressions/expression.h"

// Same as EqualsConstraint except it stores only a reference to the compared
// value, thus allowing it to preserve its vtable and do its virtual things

template <typename ExpectedType>
struct EqualsRefConstraint
    : snowhouse::Expression<EqualsRefConstraint<ExpectedType>> {
  EqualsRefConstraint(const ExpectedType &expected) : m_expected(expected) {}

  template <typename ActualType>
  bool operator()(const ActualType &actual) const {
    return (m_expected == actual);
  }

  const ExpectedType &m_expected;
};

template <typename ExpectedType>
inline EqualsRefConstraint<ExpectedType>
EqualsRef(const ExpectedType &expected) {
  return EqualsRefConstraint<ExpectedType>(expected);
}

template <typename ExpectedType>
struct snowhouse::Stringizer<EqualsRefConstraint<ExpectedType>> {
  static std::string
  ToString(const EqualsRefConstraint<ExpectedType> &constraint) {
    std::ostringstream builder;
    builder << "equal to " << snowhouse::Stringize(constraint.m_expected);

    return builder.str();
  }
};

#endif /* CONSTRAINTS_H */
