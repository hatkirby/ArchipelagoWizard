#ifndef GAME_DEFINITION_H_10B5D32A
#define GAME_DEFINITION_H_10B5D32A

#include <map>
#include <set>
#include <string>
#include <tuple>
#include <vector>

enum OptionType {
  kUNKNOWN_OPTION_TYPE,
  kRangeOption,
  kSelectOption,
};

struct OptionDefinition {
  OptionType type;
  bool named_range = false;

  std::string name;
  std::string display_name;
  std::string description;

  int default_range_value = 0;
  int min_value = 0;
  int max_value = 0;
  std::vector<std::tuple<int, std::string>> value_names;  // value, display name

  std::string default_choice;
  std::vector<std::tuple<std::string, std::string>>
      choices;  // id, display name
};

class GameDefinitions {
 public:
  GameDefinitions();

  const std::vector<OptionDefinition>& GetGameOptions(
      const std::string& game) const {
    return options_by_game_.at(game);
  }

  const std::set<std::string>& GetAllGames() const { return all_games_; }

 private:
  std::map<std::string, std::vector<OptionDefinition>> options_by_game_;
  std::set<std::string> all_games_;
};

#endif /* end of include guard: GAME_DEFINITION_H_10B5D32A */
