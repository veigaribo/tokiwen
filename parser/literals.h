#ifndef LITERALS_H
#define LITERALS_H

#include <cstdint>
#include <string>

// Literal sub-parser

std::int64_t parse_int(std::string lexeme);
double parse_float(std::string lexeme);
char parse_char(std::string lexeme);
std::string parse_string(std::string lexeme);

#endif /* LITERALS_H */
