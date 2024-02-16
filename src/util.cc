#include "util.h"

#include <sstream>

OptionValue GetRandomOptionValueFromString(std::string descriptor) {
  OptionValue result;

  auto parts = split<std::vector<std::string>>(descriptor, "-");
  int it = 0;

  if (parts[it] == "random") {
    result.random = true;
    result.range_random_type = kUniformRandom;

    it++;
  }

  if (it == parts.size()) {
    return result;
  }

  bool is_range = false;
  if (parts[it] == "range") {
    is_range = true;

    it++;

    // It's an error for there to be no parts after "range".
    if (it == parts.size()) {
      result.error = "Ranged random specifier missing min and max values.";
      return result;
    }
  }

  if (parts[it] == "low" || parts[it] == "middle" || parts[it] == "high") {
    if (parts[it] == "low") {
      result.range_random_type = kLowRandom;
    } else if (parts[it] == "middle") {
      result.range_random_type = kMiddleRandom;
    } else if (parts[it] == "high") {
      result.range_random_type = kHighRandom;
    }

    it++;
  }

  if (is_range) {
    if (it == parts.size()) {
      result.error = "Ranged random specifier missing min and max values.";
      return result;
    }

    if (it + 1 == parts.size()) {
      result.error = "Ranged random specifier missing max value.";
      return result;
    }

    int min = std::stoi(parts[it]);
    int max = std::stoi(parts[it + 1]);

    result.range_subset = std::tuple<int, int>(min, max);
  }

  if (it != parts.size()) {
    result.error = "Malformed random specifier.";
  }

  return result;
}

std::string RandomOptionValueToString(const OptionValue& option_value) {
  std::ostringstream random_str;
  random_str << "random";

  if (option_value.range_subset) {
    random_str << "-range";
  }

  if (option_value.range_random_type == kLowRandom) {
    random_str << "-low";
  } else if (option_value.range_random_type == kMiddleRandom) {
    random_str << "-middle";
  } else if (option_value.range_random_type == kHighRandom) {
    random_str << "-high";
  }

  if (option_value.range_subset) {
    auto& [min, max] = *option_value.range_subset;
    random_str << "-" << min << "-" << max;
  }

  return random_str.str();
}
