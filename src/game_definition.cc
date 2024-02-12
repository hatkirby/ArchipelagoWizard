#include "game_definition.h"

#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

GameDefinitions::GameDefinitions() {
  std::ifstream datafile("weighted-options.json");
  nlohmann::ordered_json all_games = nlohmann::ordered_json::parse(datafile);

  for (const auto& [game_name, game_data] : all_games["games"].items()) {
    std::vector<OptionDefinition> options;

    for (const auto& [option_name, option_data] :
         game_data["gameSettings"].items()) {
      OptionDefinition option;
      option.name = option_name;

      if (option_data.contains("displayName")) {
        option.display_name = option_data["displayName"];
      } else {
        option.display_name = option_name;
      }

      if (option_data.contains("description")) {
        option.description = option_data["description"];
      }

      if (option_data["type"] == "select") {
        option.type = kSelectOption;

        for (const auto& choice : option_data["options"]) {
          option.choices.emplace_back(choice["value"], choice["name"]);
        }

        option.default_choice = option_data["defaultValue"];
      } else if (option_data["type"] == "range" ||
                 option_data["type"] == "named_range") {
        option.type = kRangeOption;
        option.default_range_value = option_data["defaultValue"];
        option.min_value = option_data["min"];
        option.max_value = option_data["max"];

        if (option_data["type"] == "named_range") {
          option.named_range = true;

          for (const auto& [value_name, value_value] :
               option_data["value_names"].items()) {
            option.value_names.emplace_back(value_value, value_name);
          }
        }
      }

      options.push_back(std::move(option));
    }

    std::cout << "Read " << options.size() << " options for " << game_name
              << std::endl;
    games_.emplace(std::piecewise_construct, std::forward_as_tuple(game_name),
                   std::forward_as_tuple(game_name, std::move(options)));
    all_games_.insert(game_name);
  }
}
