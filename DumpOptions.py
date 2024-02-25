import argparse
import json
import logging
import typing

import Options

common_option_names = ["start_inventory", "local_items", "non_local_items", "start_hints", "start_location_hints",
                       "exclude_locations", "priority_locations", "start_inventory_from_pool", "item_links"]


def get_html_doc(option_type: type(Options.Option)) -> str:
    if not option_type.__doc__:
        return "Please document me!"
    return "\n".join(line.strip() for line in option_type.__doc__.split("\n")).strip()


def dump(output_path: str):
    options_output = {}

    from worlds.AutoWorld import AutoWorldRegister

    for game_name, world in AutoWorldRegister.world_types.items():
        if game_name == "Archipelago":
            continue

        all_options: typing.Dict[str, Options.AssembleOptions] = world.options_dataclass.type_hints

        game_options = {}
        common_options = []
        for option_name, option in all_options.items():
            if option_name in common_option_names:
                common_options.append(option_name)

            elif issubclass(option, Options.Choice) or issubclass(option, Options.Toggle):
                game_options[option_name] = this_option = {
                    "type": "text-select" if issubclass(option, Options.TextChoice) else "select",
                    "displayName": option.display_name if hasattr(option, "display_name") else option_name,
                    "description": get_html_doc(option),
                    "defaultValue": None,
                    "options": []
                }

                for sub_option_id, sub_option_name in option.name_lookup.items():
                    if sub_option_name != "Random":
                        this_option["options"].append({
                            "id": sub_option_id,
                            "name": option.get_option_name(sub_option_id),
                            "value": sub_option_name,
                        })
                    if sub_option_id == option.default:
                        this_option["defaultValue"] = sub_option_name

                for alias_name, sub_option_id in option.aliases.items():
                    this_option.setdefault("aliases", []).append({
                        "name": alias_name,
                        "value": option.name_lookup[sub_option_id]
                    })

                if not this_option["defaultValue"]:
                    this_option["defaultValue"] = "random"

            elif issubclass(option, Options.Range):
                game_options[option_name] = {
                    "type": "range",
                    "displayName": option.display_name if hasattr(option, "display_name") else option_name,
                    "description": get_html_doc(option),
                    "defaultValue": option.default if hasattr(
                        option, "default") and option.default != "random" else option.range_start,
                    "min": option.range_start,
                    "max": option.range_end,
                }

                if issubclass(option, Options.NamedRange):
                    game_options[option_name]["type"] = 'named_range'
                    game_options[option_name]["value_names"] = {}
                    for key, val in option.special_range_names.items():
                        game_options[option_name]["value_names"][key] = val

            elif issubclass(option, Options.ItemSet):
                game_options[option_name] = {
                    "type": "items-set",
                    "displayName": option.display_name if hasattr(option, "display_name") else option_name,
                    "description": get_html_doc(option),
                    "defaultValue": list(option.default)
                }

            elif issubclass(option, Options.ItemDict):
                game_options[option_name] = {
                    "type": "items-dict",
                    "displayName": option.display_name if hasattr(option, "display_name") else option_name,
                    "description": get_html_doc(option),
                    "defaultValue": list(option.default)
                }

            elif issubclass(option, Options.LocationSet):
                game_options[option_name] = {
                    "type": "locations-set",
                    "displayName": option.display_name if hasattr(option, "display_name") else option_name,
                    "description": get_html_doc(option),
                    "defaultValue": list(option.default)
                }

            elif issubclass(option, Options.OptionDict):
                game_options[option_name] = {
                    "type": "options-dict",
                    "displayName": option.display_name if hasattr(option, "display_name") else option_name,
                    "description": get_html_doc(option),
                    "options": list(option.valid_keys),
                    "defaultValue": list(option.default) if hasattr(option, "default") else []
                }

            elif issubclass(option, Options.OptionList) or issubclass(option, Options.OptionSet):
                if option.valid_keys:
                    game_options[option_name] = {
                        "type": "options-set",
                        "displayName": option.display_name if hasattr(option, "display_name") else option_name,
                        "description": get_html_doc(option),
                        "options": list(option.valid_keys),
                        "defaultValue": list(option.default) if hasattr(option, "default") else []
                    }
                else:
                    game_options[option_name] = {
                        "type": "free-set",
                        "displayName": option.display_name if hasattr(option, "display_name") else option_name,
                        "description": get_html_doc(option),
                        "defaultValue": list(option.default) if hasattr(option, "default") else []
                    }

            else:
                logging.debug(f"{option} not exported.")

        options_output[game_name] = {
            "options": game_options,
            "commonOptions": common_options,
            "items": tuple(world.item_names),
            "itemGroups": [
                group for group in world.item_name_groups.keys() if group != "Everything"
            ],
            "itemDescriptions": world.item_descriptions,
            "locations": tuple(world.location_names),
            "locationGroups": [
                group for group in world.location_name_groups.keys() if group != "Everywhere"
            ],
            "locationDescriptions": world.location_descriptions,
            "presets": world.web.options_presets,
        }

    with open(output_path, "w") as f:
        json.dump(options_output, f, indent=2, separators=(',', ': '))


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="World options dumper")
    parser.add_argument('--output_path', default="dumped-options.json",
                        help='Path to the file where the dumped options should be written')
    args = parser.parse_args()

    dump(args.output_path)
