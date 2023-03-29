#include "parser/types.h"

#define X(Enum, Name) Name,
char const *type_kind_names[] = {TYPE_KINDS};
#undef X

std::ostream &operator<<(std::ostream &o, const type_kind &a) {
  return o << type_kind_names[(size_t)a];
}

type::type(type_kind kind) : kind(kind) {}

bool type::matches(type &other) { return this->kind == other.kind; }
size_t type::size() { return 0; }

type_void::type_void() : type(type_kind::VOID) {}
size_t type_void::size() { return 0; }

std::ostream &operator<<(std::ostream &o, const type &a) {
  return a.extract(o);
}

std::ostream &type_void::extract(std::ostream &o) const { return o << "void"; }

type_error::type_error() : type(type_kind::VOID) {}
size_t type_error::size() { return 0; }

std::ostream &type_error::extract(std::ostream &o) const {
  return o << "error!";
}

type_int::type_int() : type(type_kind::INT) {}
size_t type_int::size() { return 8; }

std::ostream &type_int::extract(std::ostream &o) const { return o << "int"; }

type_float::type_float() : type(type_kind::FLOAT) {}
size_t type_float::size() { return 8; }

std::ostream &type_float::extract(std::ostream &o) const {
  return o << "float";
}

type_boolean::type_boolean() : type(type_kind::BOOLEAN) {}
size_t type_boolean::size() { return 4; }

std::ostream &type_boolean::extract(std::ostream &o) const {
  return o << "boolean";
}

type_char::type_char() : type(type_kind::CHAR) {}
size_t type_char::size() { return 1; }

std::ostream &type_char::extract(std::ostream &o) const { return o << "char"; }

type_function::type_function() : type(type_kind::FUNCTION) {}
size_t type_function::size() { return 8; }

std::ostream &type_function::extract(std::ostream &o) const {
  return o << "function";
}

type_struct::type_struct() : type(type_kind::CHAR) {}
size_t type_struct::size() {
  // TODO
  return 0;
}

bool type_struct::matches(type &other) {
  // TODO
  return true;
}

std::ostream &type_struct::extract(std::ostream &o) const {
  return o << "struct";
}

type_pointer::type_pointer(std::shared_ptr<type> of)
    : type(type_kind::POINTER), of(of) {}

size_t type_pointer::size() { return 8; }

bool type_pointer::matches(type &other) {
  if (this->kind != other.kind) {
    return false;
  }

  auto other_ptr = (type_pointer &)other;
  return of->matches(*other_ptr.of);
}

std::ostream &type_pointer::extract(std::ostream &o) const {
  return o << "ptr<" << of << ">";
}

bool is_arithmetic(type &t) {
  return is_integral(t) || t.kind == type_kind::FLOAT;
}

bool is_integral(type &t) {
  return t.kind == type_kind::INT || t.kind == type_kind::BOOLEAN;
}

bool is_pointer(type &t) { return t.kind == type_kind::POINTER; }
