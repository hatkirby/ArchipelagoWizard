#include "world.h"

#include "wizard_frame.h"

void World::Load(const std::string& filename,
                 const GameDefinitions* game_definitions) {
  yaml_ = YAML::LoadFile(filename);

  if (yaml_["name"]) {
    name_ = yaml_["name"].as<std::string>();
  }

  // TODO: Handle weighting.
  if (yaml_["game"]) {
    game_ = yaml_["game"].as<std::string>();

    if (yaml_[*game_]) {
      const YAML::Node& game_node = yaml_[*game_];
      const Game& game = game_definitions->GetGame(*game_);

      for (const OptionDefinition& option : game.GetOptions()) {
        if (game_node[option.name]) {
          if (option.type == kSelectOption) {
            std::string str_val = game_node[option.name].as<std::string>();
            if (option.choices.HasKey(str_val)) {
              OptionValue option_value;
              option_value.string_value = str_val;

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

void World::SetName(std::string name) {
  name_ = name;

  if (meta_update_callback_) {
    meta_update_callback_();
  }
}

void World::SetGame(const std::string& game) {
  game_ = game;

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
  options_[option_name] = std::move(option_value);
}

void World::UnsetOption(const std::string& option_name) {
  options_.erase(option_name);
}
