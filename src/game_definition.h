#ifndef GAME_DEFINITION_H_10B5D32A
#define GAME_DEFINITION_H_10B5D32A

#include <map>
#include <optional>
#include <set>
#include <string>
#include <tuple>
#include <vector>

#include "ordered_bijection.h"

enum OptionType {
  kUNKNOWN_OPTION_TYPE,
  kRangeOption,
  kSelectOption,
};

struct OptionDefinition {
  OptionType type = kUNKNOWN_OPTION_TYPE;
  bool named_range = false;

  std::string name;
  std::string display_name;
  std::string description;

  int default_range_value = 0;
  int min_value = 0;
  int max_value = 0;
  OrderedBijection<int, std::string> value_names;  // value, display name

  std::string default_choice;
  OrderedBijection<std::string, std::string> choices;  // id, display name
};

class Game {
 public:
  Game(std::string name, std::vector<OptionDefinition> options)
      : name_(std::move(name)), options_(std::move(options)) {
    for (const OptionDefinition& option : options_) {
      options_by_name_[option.name] = option;
    }
  }

  const std::string& GetName() const { return name_; }

  const std::vector<OptionDefinition>& GetOptions() const { return options_; }

  const OptionDefinition& GetOption(const std::string& option_name) const {
    return options_by_name_.at(option_name);
  }

 private:
  std::string name_;
  std::vector<OptionDefinition> options_;
  std::map<std::string, OptionDefinition> options_by_name_;
};

class GameDefinitions {
 public:
  GameDefinitions();

  const Game& GetGame(const std::string& game) const { return games_.at(game); }

  const std::set<std::string>& GetAllGames() const { return all_games_; }

 private:
  std::map<std::string, Game> games_;
  std::set<std::string> all_games_;
};

#endif /* end of include guard: GAME_DEFINITION_H_10B5D32A */
