#include "world.h"

#include <fstream>
#include <sstream>

#include "util.h"
#include "wizard_frame.h"

namespace {

OptionValue OptionValueForChoiceValue(const OptionDefinition& option,
                                      const YAML::Node& node) {
  OptionValue option_value;

  std::string str_val = node.as<std::string>();
  if (option.choices.HasKey(str_val)) {
    option_value.string_value = str_val;
  } else if (str_val == "random") {
    option_value.random = true;
  } else {
    option_value.error = "Unknown value.";
  }

  return option_value;
}

OptionValue OptionValueForRangeValue(const OptionDefinition& option,
                                     const YAML::Node& node) {
  OptionValue option_value;

  std::string str_val = node.as<std::string>();
  if (str_val.starts_with("random")) {
    option_value = GetRandomOptionValueFromString(str_val);
  } else {
    try {
      option_value.int_value = node.as<int>();

      if (option_value.int_value < option.min_value) {
        option_value.error = "Value is too small.";
      } else if (option_value.int_value > option.max_value) {
        option_value.error = "Value is too large.";
      }
    } catch (const std::exception&) {
      option_value.error = "Value is not numeric or random.";
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

void World::Save(const std::string& filename) {
  std::ofstream file_stream(filename);
  file_stream << yaml_ << std::endl;
}

void World::FromYaml(const std::string& text) {
  yaml_ = YAML::Load(text);

  PopulateFromYaml();
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

  if (meta_update_callback_) {
    meta_update_callback_();
  }
}

void World::UnsetGame() {
  if (game_ && yaml_[*game_]) {
    yaml_.remove(*game_);
  }

  game_ = std::nullopt;
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
          } else {
            yaml_[*game_][option_name][weight_value.int_value] =
                weight_value.weight;
          }
        }
      }
    } else {
      yaml_[*game_][option_name] = option_value.int_value;
    }
  } else if (option.type == kSetOption) {
    yaml_[*game_].remove(option_name);

    bool any_set = false;
    for (int i = 0; i < option_value.set_values.size(); i++) {
      if (option_value.set_values.at(i)) {
        any_set = true;

        if (option.set_type == kCustomSet) {
          yaml_[*game_][option_name].push_back(option.choices.GetKeyById(i));
        } else if (option.set_type == kItemSet) {
          yaml_[*game_][option_name].push_back(game.GetItems().GetValue(i));
        } else if (option.set_type == kLocationSet) {
          yaml_[*game_][option_name].push_back(game.GetLocations().GetValue(i));
        }
      }
    }

    if (!any_set) {
      yaml_[*game_][option_name] = YAML::Load("{}");
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

void World::PopulateFromYaml() {
  options_.clear();

  if (yaml_["name"]) {
    name_ = yaml_["name"].as<std::string>();
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

                try {
                  sub_option_value.weight = it->second.as<int>();
                } catch (const std::exception&) {
                  option_value.error = "Invalid weight value.";
                  break;
                }

                if (sub_option_value.weight > 0) {
                  option_value.weighting.push_back(std::move(sub_option_value));
                }
              }

              if (option_value.weighting.size() == 1) {
                OptionValue sub_option_value = option_value.weighting.front();
                sub_option_value.error = option_value.error;
                option_value = sub_option_value;
              }
            }
          } else if (option.type == kSetOption) {
            if (option.set_type == kCustomSet) {
              option_value.set_values.resize(option.choices.GetItems().size());
            } else if (option.set_type == kItemSet) {
              option_value.set_values.resize(game.GetItems().size());
            } else if (option.set_type == kLocationSet) {
              option_value.set_values.resize(game.GetLocations().size());
            }

            if (game_node[option.name].IsSequence()) {
              for (const YAML::Node& set_value : game_node[option.name]) {
                std::string str_val = set_value.as<std::string>();

                if (option.set_type == kCustomSet) {
                  option_value.set_values[option.choices.GetKeyId(str_val)] =
                      true;
                } else if (option.set_type == kItemSet) {
                  option_value.set_values[game.GetItems().GetId(str_val)] =
                      true;
                } else if (option.set_type == kLocationSet) {
                  option_value.set_values[game.GetLocations().GetId(str_val)] =
                      true;
                }
              }
            }
          }

          options_[option.name] = std::move(option_value);
        }
      }
    }
  }
}
