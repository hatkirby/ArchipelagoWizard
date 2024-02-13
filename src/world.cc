#include "world.h"

#include "wizard_frame.h"

World::World() {}

World::World(std::string_view filename) {}

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
