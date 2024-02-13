#ifndef WORLD_H_3EAD88F6
#define WORLD_H_3EAD88F6

#include <functional>
#include <map>
#include <optional>
#include <string>
#include <vector>

struct OptionValue {
  bool random = false;
  std::string string_value;
  int int_value = 0;
  std::vector<bool> list_values;
};

class World {
 public:
  World();

  World(std::string_view filename);

  void Save();

  const std::string& GetName() const { return name_; }

  void SetName(std::string name);

  bool HasGame() const { return game_.has_value(); }

  const std::string& GetGame() const { return *game_; }

  void SetGame(const std::string& game);

  void UnsetGame() { game_ = std::nullopt; }

  bool HasOption(const std::string& option_name) const;

  const OptionValue& GetOption(const std::string& option_name) const;

  void SetOption(const std::string& option_name, OptionValue option_value);

  void UnsetOption(const std::string& option_name);

  void SetMetaUpdateCallback(std::function<void()> callback) {
    meta_update_callback_ = callback;
  }

 private:
  std::string name_;
  std::optional<std::string> game_;
  std::map<std::string, OptionValue> options_;

  std::function<void()> meta_update_callback_;
};

#endif /* end of include guard: WORLD_H_3EAD88F6 */
