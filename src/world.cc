#include "world.h"

#include <fstream>
#include <sstream>

#include "wizard_frame.h"

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
  if (yaml_[*game_]) {
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
    yaml_[*game_][option_name] = option_value.int_value;
  } else if (option.type == kListOption) {
    // TODO: Handle list options.
  }

  options_[option_name] = std::move(option_value);
}

void World::UnsetOption(const std::string& option_name) {
  options_.erase(option_name);

  if (yaml_[*game_] && yaml_[*game_][option_name]) {
    yaml_[*game_].remove(option_name);
  }
}

void World::PopulateFromYaml() {
  options_.clear();

  if (yaml_["name"]) {
    name_ = yaml_["name"].as<std::string>();
  }

  // TODO: Handle weighting.
  if (yaml_["game"]) {
    game_ = yaml_["game"].as<std::string>();

    if (yaml_[*game_]) {
      const YAML::Node& game_node = yaml_[*game_];
      const Game& game = game_definitions_->GetGame(*game_);

      for (const OptionDefinition& option : game.GetOptions()) {
        if (game_node[option.name]) {
          if (option.type == kSelectOption) {
            if (game_node[option.name].IsScalar()) {
              std::string str_val = game_node[option.name].as<std::string>();
              if (option.choices.HasKey(str_val)) {
                OptionValue option_value;
                option_value.string_value = str_val;

                options_[option.name] = std::move(option_value);
              } else if (str_val == "random") {
                OptionValue option_value;
                option_value.random = true;

                options_[option.name] = std::move(option_value);
              }
            } else if (game_node[option.name].IsMap()) {
              OptionValue option_value;
              option_value.random = true;

              for (YAML::const_iterator it = game_node[option.name].begin();
                   it != game_node[option.name].end(); it++) {
                std::string str_val = it->first.as<std::string>();
                int weight_val = it->second.as<int>();
                if (option.choices.HasKey(str_val)) {
                  OptionValue sub_option_value;
                  sub_option_value.string_value = str_val;
                  sub_option_value.weight = weight_val;

                  option_value.weighting.push_back(std::move(sub_option_value));
                }
              }

              options_[option.name] = std::move(option_value);
            }
          } else if (option.type == kRangeOption) {
            int int_val = game_node[option.name].as<int>();
            OptionValue option_value;
            option_value.int_value = int_val;

            options_[option.name] = std::move(option_value);
          } else if (option.type == kListOption) {
            // TODO: Read list options
          }
        }
      }
    }
  }
}
