#ifndef DOUBLE_MAP_H_545C60BC
#define DOUBLE_MAP_H_545C60BC

#include <map>
#include <vector>

template <typename T>
class DoubleMap {
 public:
  void Append(const T& value) {
    backward_[value] = forward_.size();
    forward_.push_back(value);
  }

  bool HasValue(const T& value) const { return backward_.count(value); }

  size_t GetId(const T& value) const { return backward_.at(value); }

  const T& GetValue(size_t id) const { return forward_.at(id); }

  size_t size() const { return forward_.size(); }

  const std::vector<T>& GetList() const { return forward_; }

 private:
  std::vector<T> forward_;
  std::map<T, size_t> backward_;
};

#endif /* end of include guard: DOUBLE_MAP_H_545C60BC */
