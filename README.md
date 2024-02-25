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

Compiling from source requires `cmake` and `ninja-build`. Run the following commands to configure and build:

```sh
git submodule update --init
cmake --preset ap-wizard
cmake --build --preset ap-wizard-debug
```

Replace "debug" with "release" for a release-optimised build.

ArchipelagoWizard uses a datafile of options dumped from Archipelago. A script (`DumpOptions.py`) is included in this repository which can be copied into an Archipelago source tree and used to generate this datafile. A minor change to `Options.py` is also needed ([source](https://github.com/hatkirby/Archipelago/commit/123524a7c31a08813c4b6ce3a03c8afbbb3a990c#diff-0f5a189559e017401b555bcac1815941d9c9cbe91169c88ce50818038ab3e44e)). There is not currently a way to generate this file with a frozen Archipelago release, but ideally there will be one going forward. The datafile is called `dumped-options.json` and must be put in the same folder as the executable.
