#include "world.h"

World::World() {}

World::World(std::string_view filename) {}

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
