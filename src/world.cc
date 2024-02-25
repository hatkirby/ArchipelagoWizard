#include "world.h"

#include <fstream>
#include <sstream>
#include <stdexcept>

#include "util.h"
#include "wizard_frame.h"

namespace {

OptionValue OptionValueForChoiceValue(const OptionDefinition& option,
                                      const YAML::Node& node) {
  OptionValue option_value;

  std::string str_val = node.as<std::string>();
  if (option.choices.HasValue(str_val)) {
    option_value.string_value = str_val;
  } else if (option.aliases.count(str_val)) {
    option_value.string_value = option.aliases.at(str_val);
  } else if (str_val == "random") {
    option_value.random = true;
  } else {
    try {
      int int_val = std::stoi(str_val);

      if (option.choices.HasKey(int_val)) {
        option_value.int_value = int_val;
      } else {
        wxString error;
        error << "Unknown ID \"";
        error << int_val;
        error << "\".";

        option_value.error = error.ToStdString();
      }
    } catch (const std::invalid_argument&) {
      wxString error;
      error << "Unknown value \"";
      error << str_val;
      error << "\".";

      option_value.error = error.ToStdString();
    }
  }

  return option_value;
}

OptionValue OptionValueForRangeValue(const OptionDefinition& option,
                                     const YAML::Node& node) {
  OptionValue option_value;

  std::string str_val = node.as<std::string>();
  if (str_val.starts_with("random")) {
    option_value = GetRandomOptionValueFromString(str_val);
  } else if (option.value_names.HasValue(str_val)) {
    option_value.int_value = option.value_names.GetByValue(str_val);
  } else {
    try {
      option_value.int_value = node.as<int>();

      if (!option.value_names.HasKey(option_value.int_value)) {
        if (option_value.int_value < option.min_value) {
          wxString error;
          error << "Value ";
          error << str_val;
          error << " is too small.";

          option_value.error = error.ToStdString();
        } else if (option_value.int_value > option.max_value) {
          wxString error;
          error << "Value ";
          error << str_val;
          error << " is too large.";

          option_value.error = error.ToStdString();
        }
      }
    } catch (const std::exception&) {
      wxString error;
      error << "Invalid value \"";
      error << str_val;
      error << "\".";

      option_value.error = error.ToStdString();
    }
  }

  return option_value;
}

}  // namespace

void World::Load(const std::string& filename) {
  yaml_ = YAML::LoadFile(filename);
  filename_ = filename;

  PopulateFromYaml();
}

void World::SetName(std::string name) {
  name_ = name;

  yaml_["name"] = name_;

  if (meta_update_callback_) {
    meta_update_callback_();
  }
}

void World::SetDescription(const std::string& v) {
  description_ = v;

  yaml_["description"] = description_;
}

void World::Save(const std::string& filename) {
  std::ofstream file_stream(filename);
  file_stream << yaml_ << std::endl;

  SetDirty(false);
}

void World::FromYaml(const std::string& text) {
  YAML::Node old_node = Clone(yaml_);
  yaml_ = YAML::Load(text);

  try {
    PopulateFromYaml();
  } catch (const std::exception&) {
    yaml_ = old_node;

    throw;
  }
}

std::string World::ToYaml() const {
  std::ostringstream str_stream;
  str_stream << yaml_ << std::endl;
  return str_stream.str();
}

void World::SetGame(const std::string& game) {
  UnsetGame();

  game_ = game;
  yaml_["game"] = game;
  dirty_ = true;

  if (meta_update_callback_) {
    meta_update_callback_();
  }
}

void World::UnsetGame() {
  if (game_ && yaml_[*game_]) {
    yaml_.remove(*game_);
  }

  game_ = std::nullopt;
  dirty_ = true;
  options_.clear();

  if (meta_update_callback_) {
    meta_update_callback_();
  }
}

bool World::HasOption(const std::string& option_name) const {
  return options_.count(option_name);
}

const OptionValue& World::GetOption(const std::string& option_name) const {
  return options_.at(option_name);
}

void World::SetOption(const std::string& option_name,
                      OptionValue option_value) {
  const Game& game = game_definitions_->GetGame(*game_);
  const OptionDefinition& option = game.GetOption(option_name);

  if (!dirty_) {
    SetDirty(true);
  }

  yaml_[*game_].remove(option_name);
  if (option.type == kSelectOption) {
    if (option_value.random) {
      if (option_value.weighting.empty()) {
        yaml_[*game_][option_name] = "random";
      } else {
        for (const OptionValue& weight_value : option_value.weighting) {
          yaml_[*game_][option_name][weight_value.string_value] =
              weight_value.weight;
        }
      }
    } else {
      yaml_[*game_][option_name] = option_value.string_value;
    }
  } else if (option.type == kRangeOption) {
    if (option_value.random) {
      if (option_value.weighting.empty()) {
        yaml_[*game_][option_name] = RandomOptionValueToString(option_value);
      } else {
        for (const OptionValue& weight_value : option_value.weighting) {
          if (weight_value.random) {
            yaml_[*game_][option_name]
                 [RandomOptionValueToString(weight_value)] =
                     weight_value.weight;
          } else if (option.value_names.HasKey(weight_value.int_value)) {
            yaml_[*game_][option_name]
                 [option.value_names.GetByKey(weight_value.int_value)] =
                     weight_value.weight;
          } else {
            yaml_[*game_][option_name][weight_value.int_value] =
                weight_value.weight;
          }
        }
      }
    } else if (option.value_names.HasKey(option_value.int_value)) {
      yaml_[*game_][option_name] =
          option.value_names.GetByKey(option_value.int_value);
    } else {
      yaml_[*game_][option_name] = option_value.int_value;
    }
  } else if (option.type == kSetOption) {
    yaml_[*game_].remove(option_name);

    const DoubleMap<std::string>& option_set =
        GetOptionSetElements(game, option_name);
    bool any_set = false;
    for (int i = 0; i < option_value.set_values.size(); i++) {
      if (option_value.set_values.at(i)) {
        any_set = true;

        yaml_[*game_][option_name].push_back(option_set.GetValue(i));
      }
    }

    if (!any_set) {
      yaml_[*game_][option_name] = YAML::Load("[]");
    }
  } else if (option.type == kDictOption) {
    yaml_[*game_].remove(option_name);

    const DoubleMap<std::string>& option_set =
        GetOptionSetElements(game, option_name);
    if (option_value.dict_values.empty()) {
      yaml_[*game_][option_name] = YAML::Load("{}");
    } else {
      for (const auto& [id, amount] : option_value.dict_values) {
        yaml_[*game_][option_name][option_set.GetValue(id)] = amount;
      }
    }
  }

  options_[option_name] = std::move(option_value);
}

void World::UnsetOption(const std::string& option_name) {
  options_.erase(option_name);

  if (yaml_[*game_] && yaml_[*game_][option_name]) {
    yaml_[*game_].remove(option_name);
  }

  if (yaml_[*game_].IsMap() && yaml_[*game_].size() == 0) {
    yaml_.remove(*game_);
  }
}

void World::ClearOptions() {
  dirty_ = true;
  options_.clear();
}

void World::PopulateFromYaml() {
  if (yaml_["game"] &&
      !game_definitions_->HasGame(yaml_["game"].as<std::string>())) {
    wxString error;
    error << "Game \"";
    error << yaml_["game"].as<std::string>();
    error << "\" is not supported.";
    throw std::invalid_argument(error.ToStdString());
  }

  options_.clear();

  if (yaml_["name"]) {
    name_ = yaml_["name"].as<std::string>();
  }

  if (yaml_["description"]) {
    description_ = yaml_["description"].as<std::string>();
  }

  if (yaml_["game"]) {
    game_ = yaml_["game"].as<std::string>();

    if (yaml_[*game_]) {
      const YAML::Node& game_node = yaml_[*game_];
      const Game& game = game_definitions_->GetGame(*game_);

      for (const OptionDefinition& option : game.GetOptions()) {
        if (game_node[option.name]) {
          OptionValue option_value;

          if (option.type == kSelectOption || option.type == kRangeOption) {
            // Choices and ranges can both be weighted.
            if (game_node[option.name].IsScalar()) {
              if (option.type == kSelectOption) {
                option_value =
                    OptionValueForChoiceValue(option, game_node[option.name]);
              } else if (option.type == kRangeOption) {
                option_value =
                    OptionValueForRangeValue(option, game_node[option.name]);
              }
            } else if (game_node[option.name].IsMap()) {
              option_value.random = true;

              std::vector<wxString> errors;
              for (YAML::const_iterator it = game_node[option.name].begin();
                   it != game_node[option.name].end(); it++) {
                OptionValue sub_option_value;
                if (option.type == kSelectOption) {
                  sub_option_value =
                      OptionValueForChoiceValue(option, it->first);
                } else if (option.type == kRangeOption) {
                  sub_option_value =
                      OptionValueForRangeValue(option, it->first);
                }

                if (sub_option_value.error) {
                  errors.push_back(*sub_option_value.error);
                }

                try {
                  sub_option_value.weight = it->second.as<int>();
                } catch (const std::exception&) {
                  wxString error;
                  error << "Weight value \"";
                  error << it->second.as<std::string>();
                  error << "\" is not numeric.";

                  errors.push_back(error);
                }

                if (sub_option_value.weight > 0) {
                  option_value.weighting.push_back(std::move(sub_option_value));
                }
              }

              if (option_value.weighting.size() == 1) {
                OptionValue sub_option_value = option_value.weighting.front();
                option_value = sub_option_value;
              }

              if (!errors.empty()) {
                option_value.error = implode(errors).ToStdString();
              }
            }
          } else if (option.type == kSetOption) {
            const DoubleMap<std::string>& option_set =
                GetOptionSetElements(game, option.name);
            option_value.set_values.resize(option_set.size());

            if (game_node[option.name].IsSequence()) {
              std::vector<wxString> errors;

              for (const YAML::Node& set_value : game_node[option.name]) {
                std::string str_val = set_value.as<std::string>();
                if (option_set.HasValue(str_val)) {
                  option_value.set_values[option_set.GetId(str_val)] = true;
                } else {
                  wxString msg;
                  msg << "Invalid value \"";
                  msg << str_val;
                  msg << "\".";

                  errors.push_back(msg);
                }
              }

              if (!errors.empty()) {
                option_value.error = implode(errors).ToStdString();
              }
            } else {
              option_value.error = "Option value should be a list.";
            }
          } else if (option.type == kDictOption) {
            const DoubleMap<std::string>& option_set =
                GetOptionSetElements(game, option.name);

            if (game_node[option.name].IsMap()) {
              std::vector<wxString> errors;

              for (YAML::const_iterator it = game_node[option.name].begin();
                   it != game_node[option.name].end(); it++) {
                std::string str_val = it->first.as<std::string>();
                int int_val = it->second.as<int>();

                if (option_set.HasValue(str_val)) {
                  option_value.dict_values[option_set.GetId(str_val)] = int_val;
                } else {
                  wxString msg;
                  msg << "Invalid value \"";
                  msg << str_val;
                  msg << "\".";

                  errors.push_back(msg);
                }
              }

              if (!errors.empty()) {
                option_value.error = implode(errors).ToStdString();
              }
            } else {
              option_value.error = "Option value should be a map.";
            }
          }

          options_[option.name] = std::move(option_value);
        }
      }
    }
  }
}
