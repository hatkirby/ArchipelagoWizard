#ifndef WORLD_H_3EAD88F6
#define WORLD_H_3EAD88F6

#include <map>
#include <optional>
#include <string>

struct OptionValue {
  bool random = false;
  std::string string_value;
  int int_value = 0;
};

class World {
 public:
  World();

  World(std::string_view filename);

  void Save();

  const std::string& GetName() const { return name_; }

  void SetName(std::string name) { name_ = name; }

  bool HasGame() const { return game_.has_value(); }

  const std::string& GetGame() const { return *game_; }

  void SetGame(const std::string& game) { game_ = game; }

  void UnsetGame() { game_ = std::nullopt; }

  bool HasOption(const std::string& option_name) const;

  const OptionValue& GetOption(const std::string& option_name) const;

  void SetOption(const std::string& option_name, OptionValue option_value);

  void UnsetOption(const std::string& option_name);

 private:
  std::string name_;
  std::optional<std::string> game_;
  std::map<std::string, OptionValue> options_;
};

#endif /* end of include guard: WORLD_H_3EAD88F6 */
