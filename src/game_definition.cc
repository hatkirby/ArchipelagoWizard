#include "game_definition.h"

#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

#include "util.h"

GameDefinitions::GameDefinitions() {
  std::ifstream datafile("dumped-options.json");
  nlohmann::ordered_json all_games = nlohmann::ordered_json::parse(datafile);

  for (const auto& [game_name, game_data] : all_games.items()) {
    std::vector<OptionDefinition> options;

    DoubleMap<std::string> game_items;
    for (const auto& item_name : game_data["itemGroups"]) {
      game_items.Append(item_name);
    }
    for (const auto& item_name : game_data["items"]) {
      game_items.Append(item_name);
    }

    DoubleMap<std::string> game_locations;
    for (const auto& location_name : game_data["locationGroups"]) {
      game_locations.Append(location_name);
    }
    for (const auto& location_name : game_data["locations"]) {
      game_locations.Append(location_name);
    }

    for (const auto& [option_name, option_data] :
         game_data["options"].items()) {
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
          option.choices.Append(choice["value"], choice["name"]);
        }

        if (option_data["defaultValue"] == "random") {
          option.default_value.random = true;
        } else {
          option.default_value.string_value = option_data["defaultValue"];
        }
      } else if (option_data["type"] == "options-set") {
        option.type = kSetOption;
        option.set_type = kCustomSet;

        for (const auto& choice : option_data["options"]) {
          option.choices.Append(choice, choice);
          option.default_value.set_values.push_back(false);
        }

        for (const auto& default_value : option_data["defaultValue"]) {
          option.default_value
              .set_values[option.choices.GetKeyId(default_value)] = true;
        }
      } else if (option_data["type"] == "items-set") {
        option.type = kSetOption;
        option.set_type = kItemSet;
        option.default_value.set_values.resize(game_items.size());

        for (const auto& default_value : option_data["defaultValue"]) {
          option.default_value.set_values[game_items.GetId(default_value)] =
              true;
        }
      } else if (option_data["type"] == "locations-set") {
        option.type = kSetOption;
        option.set_type = kLocationSet;
        option.default_value.set_values.resize(game_locations.size());

        for (const auto& default_value : option_data["defaultValue"]) {
          option.default_value.set_values[game_locations.GetId(default_value)] =
              true;
        }
      } else if (option_data["type"] == "range" ||
                 option_data["type"] == "named_range") {
        option.type = kRangeOption;
        option.min_value = option_data["min"];
        option.max_value = option_data["max"];

        if (option_data["type"] == "named_range") {
          option.named_range = true;

          for (const auto& [value_name, value_value] :
               option_data["value_names"].items()) {
            option.value_names.Append(value_value, value_name);
          }
        }

        if (option_data["defaultValue"].is_string()) {
          std::string default_value = option_data["defaultValue"];
          if (default_value.starts_with("random")) {
            option.default_value =
                GetRandomOptionValueFromString(default_value);
          } else if (option.value_names.HasValue(default_value)) {
            option.default_value.int_value =
                option.value_names.GetByValue(default_value);
          }
        } else if (option_data["defaultValue"].is_number()) {
          option.default_value.int_value = option_data["defaultValue"];
        }
      }

      options.push_back(std::move(option));
    }

    std::cout << "Read " << options.size() << " options for " << game_name
              << std::endl;
    games_.emplace(std::piecewise_construct, std::forward_as_tuple(game_name),
                   std::forward_as_tuple(game_name, std::move(options),
                                         std::move(game_items),
                                         std::move(game_locations)));
    all_games_.insert(game_name);
  }
}
