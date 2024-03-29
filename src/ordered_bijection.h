#ifndef ORDERED_BIJECTION_H_0A722EC4
#define ORDERED_BIJECTION_H_0A722EC4

#include <map>
#include <tuple>
#include <vector>

template <typename K, typename V>
class OrderedBijection {
 public:
  void Append(K key, V value) {
    keys_[key] = ordering_.size();
    values_[value] = ordering_.size();
    ordering_.emplace_back(key, value);
    forward_[key] = value;
    backward_[value] = key;
  }

  const std::vector<std::tuple<K, V>>& GetItems() const { return ordering_; }

  bool HasKey(const K& key) const { return forward_.count(key); }

  const V& GetByKey(const K& key) const { return forward_.at(key); }

  std::optional<V> GetByKeyOptional(const K& key) const {
    if (forward_.count(key)) {
      return forward_.at(key);
    } else {
      return std::nullopt;
    }
  }

  bool HasValue(const V& value) const { return backward_.count(value); }

  const K& GetByValue(const V& value) const { return backward_.at(value); }

  std::optional<K> GetByValueOptional(const V& value) const {
    if (backward_.count(value)) {
      return backward_.at(value);
    } else {
      return std::nullopt;
    }
  }

  size_t GetKeyId(const K& key) const { return keys_.at(key); }

  size_t GetValueId(const V& value) const { return values_.at(value); }

  const K& GetKeyById(size_t id) const { return std::get<0>(ordering_.at(id)); }

  const V& GetValueById(size_t id) const {
    return std::get<1>(ordering_.at(id));
  }

  bool HasId(size_t id) const { return id >= 0 && id < ordering_.size(); }

 private:
  std::vector<std::tuple<K, V>> ordering_;
  std::map<K, V> forward_;
  std::map<V, K> backward_;
  std::map<K, size_t> keys_;
  std::map<V, size_t> values_;
};

#endif /* end of include guard: ORDERED_BIJECTION_H_0A722EC4 */
