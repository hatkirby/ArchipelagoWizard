#include "game_definition.h"

#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <set>

#include "util.h"

GameDefinitions::GameDefinitions() {
  std::ifstream datafile(GetAbsolutePath("dumped-options.json"));
  nlohmann::ordered_json all_games = nlohmann::ordered_json::parse(datafile);

  for (const auto& [game_name, game_data] : all_games.items()) {
    std::vector<OptionDefinition> options;

    std::set<std::string> sorted_game_items;
    for (const auto& item_name : game_data["itemGroups"]) {
      sorted_game_items.insert(item_name);
    }
    for (const auto& item_name : game_data["items"]) {
      sorted_game_items.insert(item_name);
    }

    DoubleMap<std::string> game_items;
    for (const std::string& game_item : sorted_game_items) {
      game_items.Append(game_item);
    }

    std::set<std::string> sorted_game_locations;
    for (const auto& location_name : game_data["locationGroups"]) {
      sorted_game_locations.insert(location_name);
    }
    for (const auto& location_name : game_data["locations"]) {
      sorted_game_locations.insert(location_name);
    }

    DoubleMap<std::string> game_locations;
    for (const std::string& game_location : sorted_game_locations) {
      game_locations.Append(game_location);
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
          option.custom_set.Append(choice);
          option.default_value.set_values.push_back(false);
        }

        for (const auto& default_value : option_data["defaultValue"]) {
          option.default_value
              .set_values[option.custom_set.GetId(default_value)] = true;
        }
      } else if (option_data["type"] == "items-set") {
        option.type = kSetOption;
        option.set_type = kItemSet;
        option.default_value.set_values.resize(game_items.size());

        for (const auto& default_value : option_data["defaultValue"]) {
          option.default_value.set_values[game_items.GetId(default_value)] =
              true;
        }
      } else if (option_data["type"] == "items-dict") {
        option.type = kDictOption;
        option.set_type = kItemSet;

        for (const auto& default_value : option_data["defaultValue"]) {
          option.default_value.dict_values[game_items.GetId(default_value)] = 1;
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

    for (const auto& common_option : game_data["commonOptions"]) {
      OptionDefinition option;
      option.name = common_option;
      option.common = true;

      if (option.name == "local_items") {
        option.type = kSetOption;
        option.set_type = kItemSet;
        option.display_name = "Local Items";
        option.description = "Forces these items to be in their native world.";
      } else if (option.name == "non_local_items") {
        option.type = kSetOption;
        option.set_type = kItemSet;
        option.display_name = "Non-Local Items";
        option.description =
            "Forces these items to be outside their native world.";
      } else if (option.name == "start_inventory") {
        option.type = kDictOption;
        option.set_type = kItemSet;
        option.display_name = "Start Inventory";
        option.description = "Start with these items.";
      } else if (option.name == "start_inventory_from_pool") {
        option.type = kDictOption;
        option.set_type = kItemSet;
        option.display_name = "Start Inventory from Pool";
        option.description =
            "Start with these items and don't place them in the world.\nThe "
            "game decides what the replacement items will be.";
      } else if (option.name == "start_hints") {
        option.type = kSetOption;
        option.set_type = kItemSet;
        option.display_name = "Start Hints";
        option.description =
            "Start with these item's locations prefilled into the !hint "
            "command.";
      } else if (option.name == "start_location_hints") {
        option.type = kSetOption;
        option.set_type = kLocationSet;
        option.display_name = "Start Location Hints";
        option.description =
            "Start with these locations and their item prefilled into the "
            "!hint command.";
      } else if (option.name == "exclude_locations") {
        option.type = kSetOption;
        option.set_type = kLocationSet;
        option.display_name = "Excluded Locations";
        option.description =
            "Prevent these locations from having an important item.";
      } else if (option.name == "priority_locations") {
        option.type = kSetOption;
        option.set_type = kLocationSet;
        option.display_name = "Priority Locations";
        option.description =
            "Force these locations to have an important item.";
      } else if (option.name == "item_links") {
        option.type = kUNKNOWN_OPTION_TYPE;
        option.display_name = "Item Links";
        option.description = "Share part of your item pool with other players.";
      } else {
        continue;
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
