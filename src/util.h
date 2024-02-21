#ifndef UTIL_H_84145E76
#define UTIL_H_84145E76

#include <filesystem>
#include <iterator>
#include <string>
#include <string_view>

#include "double_map.h"
#include "game_definition.h"

template <class OutputIterator>
void split(std::string input, std::string delimiter, OutputIterator out) {
  while (!input.empty()) {
    int divider = input.find(delimiter);
    if (divider == std::string::npos) {
      *out = input;
      out++;

      input = "";
    } else {
      *out = input.substr(0, divider);
      out++;

      input = input.substr(divider + delimiter.length());
    }
  }
}

template <class Container>
Container split(std::string input, std::string delimiter) {
  Container result;

  split(input, delimiter, std::back_inserter(result));

  return result;
}

OptionValue GetRandomOptionValueFromString(std::string descriptor);

std::string RandomOptionValueToString(const OptionValue& option_value);

const DoubleMap<std::string>& GetOptionSetElements(
    const Game& game, const std::string& option_name);

const std::filesystem::path& GetExecutableDirectory();

std::string GetAbsolutePath(std::string_view path);

#endif /* end of include guard: UTIL_H_84145E76 */
