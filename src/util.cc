#include "util.h"

#include <whereami.h>

#include <sstream>
#include <vector>

OptionValue GetRandomOptionValueFromString(wxString descriptor) {
  OptionValue result;

  auto parts = split<std::vector<wxString>>(descriptor, "-");
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
      wxString error;
      error << "Ranged random specifier \"";
      error << descriptor;
      error << "\" missing min and max values.";

      result.error = error.ToStdString();
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
      wxString error;
      error << "Ranged random specifier \"";
      error << descriptor;
      error << "\" missing min and max values.";

      result.error = error.ToStdString();
      return result;
    }

    if (it + 1 == parts.size()) {
      wxString error;
      error << "Ranged random specifier \"";
      error << descriptor;
      error << "\" missing max value.";

      result.error = error.ToStdString();
      return result;
    }

    long min = 0;
    if (!parts[it].ToLong(&min)) {
      wxString error;
      error << "Ranged random specifier \"";
      error << descriptor;
      error << "\" min value is not numeric.";

      result.error = error.ToStdString();
      return result;
    }

    long max = 0;
    if (!parts[it + 1].ToLong(&max)) {
      wxString error;
      error << "Ranged random specifier \"";
      error << descriptor;
      error << "\" max value is not numeric.";

      result.error = error.ToStdString();
      return result;
    }

    result.range_subset = std::tuple<int, int>(min, max);

    it++;
    it++;
  }

  if (it != parts.size()) {
    wxString error;
    error << "Malformed random specifier \"";
    error << descriptor;
    error << "\".";

    result.error = error.ToStdString();
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

const DoubleMap<std::string>& GetOptionSetElements(
    const Game& game, const std::string& option_name) {
  const OptionDefinition& game_option = game.GetOption(option_name);

  if (game_option.set_type == kCustomSet) {
    return game_option.custom_set;
  } else if (game_option.set_type == kItemSet) {
    return game.GetItems();
  } else if (game_option.set_type == kLocationSet) {
    return game.GetLocations();
  }

  throw std::invalid_argument("Invalid option set type.");
}

const std::filesystem::path& GetExecutableDirectory() {
  static const std::filesystem::path* executable_directory = []() {
    int length = wai_getExecutablePath(NULL, 0, NULL);
    std::string buf(length, 0);
    wai_getExecutablePath(buf.data(), length, NULL);

    std::filesystem::path exec_path(buf);
    return new std::filesystem::path(exec_path.parent_path());
  }();

  return *executable_directory;
}

std::string GetAbsolutePath(std::string_view path) {
  return (GetExecutableDirectory() / path).string();
}

wxString ConvertToTitleCase(wxString input) {
  auto words = split<std::vector<wxString>>(input, " ");
  for (wxString& word : words) {
    word.MakeCapitalized();
  }

  return implode(words, " ");
}
