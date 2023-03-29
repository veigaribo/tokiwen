#ifndef TYPES_H
#define TYPES_H

#include <iostream>
#include <memory>

#define TYPE_KINDS                                                             \
  X(VOID, "void")                                                              \
  X(ERROR, "error")                                                            \
  X(INT, "int")                                                                \
  X(FLOAT, "float")                                                            \
  X(BOOLEAN, "boolean")                                                        \
  X(CHAR, "char")                                                              \
  X(FUNCTION, "function")                                                      \
  X(STRUCT, "struct")                                                          \
  X(POINTER, "pointer")

#define X(Enum, Name) Enum,
enum class type_kind { TYPE_KINDS };
#undef X

std::ostream &operator<<(std::ostream &o, const type_kind &a);

class type {
private:
  virtual std::ostream &extract(std::ostream &o) const = 0;

public:
  const type_kind kind;

  type(type_kind kind);

  virtual bool matches(type &other);
  virtual size_t size();

  friend std::ostream &operator<<(std::ostream &o, const type &a);
};

bool is_arithmetic(type &t);
bool is_integral(type &t);
bool is_pointer(type &t);

#define BASIC_TYPE(Name)                                                       \
  class type_##Name : public type {                                            \
  private:                                                                     \
    virtual std::ostream &extract(std::ostream &o) const;                      \
                                                                               \
  public:                                                                      \
    type_##Name();                                                             \
    virtual size_t size();                                                     \
  }

BASIC_TYPE(void);
BASIC_TYPE(error);
BASIC_TYPE(int);
BASIC_TYPE(float);
BASIC_TYPE(boolean);
BASIC_TYPE(char);
BASIC_TYPE(function);

class type_struct : public type {
private:
  std::ostream &extract(std::ostream &o) const;

public:
  type_struct();

  bool matches(type &other);
  virtual size_t size();
};

class type_pointer : public type {
private:
  std::ostream &extract(std::ostream &o) const;
  std::shared_ptr<type> of;

public:
  type_pointer(std::shared_ptr<type> of);

  bool matches(type &other);
  virtual size_t size();
};

#endif /* TYPES_H */
