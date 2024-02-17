#ifndef GAME_DEFINITION_H_10B5D32A
#define GAME_DEFINITION_H_10B5D32A

#include <map>
#include <optional>
#include <set>
#include <string>
#include <tuple>
#include <vector>

#include "double_map.h"
#include "ordered_bijection.h"

enum OptionType {
  kUNKNOWN_OPTION_TYPE,
  kRangeOption,
  kSelectOption,
  kSetOption,
};

enum SetType {
  kUNKNOWN_SET_TYPE,
  kCustomSet,
  kItemSet,
  kLocationSet,
};

enum RandomValueType {
  kUNKNOWN_RANDOM_VALUE_TYPE,
  kUniformRandom,
  kLowRandom,
  kMiddleRandom,
  kHighRandom,
};

struct OptionValue {
  bool random = false;
  std::string string_value;
  int int_value = 0;
  std::vector<bool> set_values;

  int weight = 50;
  std::vector<OptionValue> weighting;

  RandomValueType range_random_type = kUNKNOWN_RANDOM_VALUE_TYPE;
  std::optional<std::tuple<int, int>> range_subset;  // low, high

  std::optional<std::string> error;
};

struct OptionDefinition {
  OptionType type = kUNKNOWN_OPTION_TYPE;
  bool named_range = false;

  std::string name;
  std::string display_name;
  std::string description;

  int min_value = 0;
  int max_value = 0;
  OrderedBijection<int, std::string> value_names;  // value, display name

  SetType set_type = kUNKNOWN_SET_TYPE;
  OrderedBijection<std::string, std::string> choices;  // id, display name

  OptionValue default_value;
};

class Game {
 public:
  Game(std::string name, std::vector<OptionDefinition> options,
       DoubleMap<std::string> items, DoubleMap<std::string> locations)
      : name_(std::move(name)),
        options_(std::move(options)),
        items_(std::move(items)),
        locations_(std::move(locations)) {
    for (const OptionDefinition& option : options_) {
      options_by_name_[option.name] = option;
    }
  }

  const std::string& GetName() const { return name_; }

  const std::vector<OptionDefinition>& GetOptions() const { return options_; }

  const OptionDefinition& GetOption(const std::string& option_name) const {
    return options_by_name_.at(option_name);
  }

  const DoubleMap<std::string>& GetItems() const { return items_; }

  const DoubleMap<std::string>& GetLocations() const { return locations_; }

 private:
  std::string name_;
  std::vector<OptionDefinition> options_;
  std::map<std::string, OptionDefinition> options_by_name_;
  DoubleMap<std::string> items_;
  DoubleMap<std::string> locations_;
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
