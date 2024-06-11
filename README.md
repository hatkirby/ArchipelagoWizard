# ArchipelagoWizard

ArchipelagoWizard is a tool that can be used to generate YAML files compatible with the [Archipelago](https://archipelago.gg/) multiworld multi-game randomizer. It aims to replicate the experience of using the archipelago.gg website for creating YAMLs, while also allowing you to configure options for worlds that are not officially supported by Archipelago.

## Features

- Create YAMLs using a form similar to the website
- Load YAML files created in the tool or elsewhere
- Builtin YAML editor for advanced configuration
- GUI for weighted randomisation of choice and range fields
- Filterable item picker UI for option sets with many values such as `start_inventory`
- Option presets

### Not yet implemented

- Worlds with a random game
- YAML files with multiple worlds
- Schematised option types like `item_links`
- In-place validation in the YAML editor

## Screenshots

<img src="https://github.com/hatkirby/ArchipelagoWizard/assets/442990/f98de7af-ebf5-42e0-aaf1-e9647e49ae5a" alt="Screenshot of the main ArchipelagoWizard window" width="400"/> <img src="https://github.com/hatkirby/ArchipelagoWizard/assets/442990/4d2ff465-b61b-4ff3-80b3-17f005352d74" alt="Screenshot of the YAML editor" width="400"/>
<img src="https://github.com/hatkirby/ArchipelagoWizard/assets/442990/f3f18c67-0c5d-47ee-aeaa-82403c3b68ed" alt="Screenshot of the advanced randomization dialog" width="400"/> <img src="https://github.com/hatkirby/ArchipelagoWizard/assets/442990/cf75eb37-f1b9-4379-b8e3-ceb5636440fd" alt="Screenshot of the item picker dialog" width="400" />

## Getting started

The latest version can be downloaded on [the releases page](https://github.com/hatkirby/ArchipelagoWizard/releases). It contains a datafile up-to-date with Archipelago source main as of the time of release.

Only Windows builds are provided currently. However, ArchipelagoWizard is compatible with both macOS and Linux, and has been tested on both.

ArchipelagoWizard uses a datafile of options dumped from Archipelago. If you have any installed apworlds that you would like to see options for, you need to generate a new datafile. You can do this using the wizard apworld (`wizard.apworld`). Install it in your `lib/worlds` folder the way you would install any other apworld, and then open the Launcher, and click "Dump Options for Wizard". If successful, it should create a file called `dumped-options.json` in your Archipelago directory. This file then just needs to be copied into the same folder as the ap-wizard executable.

**NOTE**: There is an issue with Archipelago 0.4.7 and earlier where option aliases are ignored. There is an open PR to fix it (ArchipelagoMW/Archipelago#3512).

## Building from source

Compiling from source requires `cmake` and `ninja-build`. Run the following commands to configure and build:

```sh
git submodule update --init
cmake --preset ap-wizard
cmake --build --preset ap-wizard-debug
```

Replace "debug" with "release" for a release-optimised build.
