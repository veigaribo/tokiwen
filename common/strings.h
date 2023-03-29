#ifndef STRINGS_H
#define STRINGS_H

#include <sstream>
#include <string>

// https://stackoverflow.com/a/33357612
template <typename T> std::string to_string(const T &value) {
  std::ostringstream ss;
  ss << value;
  return ss.str();
}

#endif /* STRINGS_H */
