#include <parser/literals.h>

std::int64_t parse_int(std::string lexeme) { return std::stoll(lexeme); }
double parse_float(std::string lexeme) { return std::stod(lexeme); }

// TODO: Char = Grapheme
char parse_char(std::string lexeme) { return lexeme.at(1); }

#define ESCAPE_CASE(Char, Escaped)                                             \
  case Char:                                                                   \
    building.push_back(Escaped);                                               \
    continue

// Translate escape sequences
std::string parse_string(std::string lexeme) {
  std::string building;
  bool escaped = false;

  for (size_t i = 1; i < lexeme.size() - 1; ++i) {
    auto &c = lexeme.at(i);

    if (escaped) {
      escaped = false;

      switch (c) {
        ESCAPE_CASE('n', '\n');
        ESCAPE_CASE('r', '\r');
        ESCAPE_CASE('t', '\t');
        ESCAPE_CASE('v', '\v');
        ESCAPE_CASE('f', '\f');
        ESCAPE_CASE('\\', '\\');
      }
    }

    if (c == '\\') {
      escaped = true;
      continue;
    }

    building.push_back(c);
  }

  return building;
}
