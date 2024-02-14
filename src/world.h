#ifndef WORLD_H_3EAD88F6
#define WORLD_H_3EAD88F6

#include <yaml-cpp/yaml.h>

#include <functional>
#include <map>
#include <optional>
#include <string>
#include <vector>

class GameDefinitions;

struct OptionValue {
  bool random = false;
  std::string string_value;
  int int_value = 0;
  std::vector<bool> list_values;
};

class World {
 public:
  explicit World(const GameDefinitions* game_definitions)
      : game_definitions_(game_definitions) {}

  void Load(const std::string& filename);

  void Save(const std::string& filename);

  bool HasFilename() const { return filename_.has_value(); }

  const std::string& GetFilename() const { return *filename_; }

  void SetFilename(const std::string& val) { filename_ = val; }

  void FromYaml(const std::string& text);

  std::string ToYaml() const;

  const std::string& GetName() const { return name_; }

  void SetName(std::string name);

  bool HasGame() const { return game_.has_value(); }

  const std::string& GetGame() const { return *game_; }

  void SetGame(const std::string& game);

  void UnsetGame();

  bool HasOption(const std::string& option_name) const;

  const OptionValue& GetOption(const std::string& option_name) const;

  void SetOption(const std::string& option_name, OptionValue option_value);

  void UnsetOption(const std::string& option_name);

  bool HasSetOptions() const { return !options_.empty(); }

  void SetMetaUpdateCallback(std::function<void()> callback) {
    meta_update_callback_ = callback;
  }

 private:
  void PopulateFromYaml();

  const GameDefinitions* game_definitions_;

  std::string name_;
  std::optional<std::string> game_;
  std::map<std::string, OptionValue> options_;

  std::optional<std::string> filename_;

  YAML::Node yaml_;

  std::function<void()> meta_update_callback_;
};

#endif /* end of include guard: WORLD_H_3EAD88F6 */
