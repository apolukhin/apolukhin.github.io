#include <list>
#include <memory>
#include <unordered_map>

#include <boost/intrusive/link_mode.hpp>
#include <boost/intrusive/list.hpp>
#include <boost/intrusive/list_hook.hpp>

#include <boost/intrusive/unordered_set.hpp>
#include <boost/intrusive/unordered_set_hook.hpp>

#include <benchmark/benchmark.h>

namespace v1 {

template <typename T, typename Hash = std::hash<T>,
          typename Equal = std::equal_to<T>>
class LruSet final {
 public:
  explicit LruSet(size_t max_size) : map_(max_size), max_size_(max_size) {}
  ~LruSet() {}

  LruSet(LruSet&& lru) = default;
  LruSet(const LruSet& lru) = delete;
  LruSet& operator=(LruSet&& lru) = default;
  LruSet& operator=(const LruSet& lru) = delete;

  bool Put(const T& key) {
    auto it = map_.find(key);
    if (it != map_.end()) {
      list_.erase(it->second);
      it->second = list_.insert(list_.end(), key);
      return false;
    }

    if (list_.size() == max_size_) {
      T last = list_.front();
      list_.pop_front();
      map_.erase(last);

      map_[key] = list_.insert(list_.end(), key);
    } else {
      map_[key] = list_.insert(list_.end(), key);
    }

    return true;
  }

  bool Has(const T& key) {
    auto it = map_.find(key);
    if (it == map_.end()) return false;

    auto list_it = it->second;
    list_.erase(list_it);
    it->second = list_.insert(list_.end(), key);
    return true;
  }

 private:
  std::list<T> list_;
  std::unordered_map<T, typename std::list<T>::iterator, Hash, Equal> map_;
  std::size_t max_size_;
};

}  // namespace v1

namespace v2 {
template <typename T, typename Hash = std::hash<T>,
          typename Equal = std::equal_to<T>>
class LruSet final {
 public:
  explicit LruSet(size_t max_size) : map_(max_size), max_size_(max_size) {}
  ~LruSet() {}

  LruSet(LruSet&& lru) = default;
  LruSet(const LruSet& lru) = delete;
  LruSet& operator=(LruSet&& lru) = default;
  LruSet& operator=(const LruSet& lru) = delete;

  bool Put(const T& key) {
    auto it = map_.find(key);
    if (it != map_.end()) {
      auto list_it = it->second;
      list_.splice(list_.end(), list_, list_it);
      return false;
    }

    if (list_.size() == max_size_) {
      T last = list_.front();
      map_.erase(last);

      list_.front() = key;
      list_.splice(list_.end(), list_, list_.begin());

      map_[key] = --list_.end();
    } else {
      map_[key] = list_.insert(list_.end(), key);
    }

    return true;
  }

  bool Has(const T& key) {
    auto it = map_.find(key);
    if (it == map_.end()) return false;

    auto list_it = it->second;
    list_.splice(list_.end(), list_, list_it);
    return true;
  }

 private:
  std::list<T> list_;
  std::unordered_map<T, typename std::list<T>::iterator, Hash, Equal> map_;
  std::size_t max_size_;
};

}  // namespace v2

namespace v3 {
template <typename T, typename Hash = std::hash<T>,
          typename Equal = std::equal_to<T>>
class LruSet final {
 public:
  explicit LruSet(size_t max_size) : map_(max_size), max_size_(max_size) {}
  ~LruSet() {}

  LruSet(LruSet&& lru) = default;
  LruSet(const LruSet& lru) = delete;
  LruSet& operator=(LruSet&& lru) = default;
  LruSet& operator=(const LruSet& lru) = delete;

  bool Put(const T& key) {
    auto it = map_.find(key);
    if (it != map_.end()) {
      auto list_it = it->second;
      list_.splice(list_.end(), list_, list_it);
      return false;
    }

    if (list_.size() == max_size_) {
      T last = list_.front();
      auto node = map_.extract(last);

      list_.front() = key;
      list_.splice(list_.end(), list_, list_.begin());

      node.key() = key;
      node.mapped() = --list_.end();
      map_.insert(std::move(node));
    } else {
      map_[key] = list_.insert(list_.end(), key);
    }

    return true;
  }

  bool Has(const T& key) {
    auto it = map_.find(key);
    if (it == map_.end()) return false;

    auto list_it = it->second;
    list_.splice(list_.end(), list_, list_it);
    return true;
  }

 private:
  std::list<T> list_;
  std::unordered_map<T, typename std::list<T>::iterator, Hash, Equal> map_;
  std::size_t max_size_;
};

}  // namespace v3

namespace v4 {

using LinkMode = boost::intrusive::link_mode<
#ifdef NDEBUG
    boost::intrusive::normal_link
#else
    boost::intrusive::safe_link
#endif
    >;

using LruHook = boost::intrusive::list_base_hook<LinkMode>;

class LruNode final : public LruHook {};

template <typename T, typename Hash = std::hash<T>,
          typename Equal = std::equal_to<T>>
class LruSet final {
 public:
  explicit LruSet(size_t max_size) : map_(max_size), max_size_(max_size) {}
  ~LruSet() {}

  LruSet(LruSet&& lru) = default;
  LruSet(const LruSet& lru) = delete;
  LruSet& operator=(LruSet&& lru) = default;
  LruSet& operator=(const LruSet& lru) = delete;

  bool Put(const T& key) {
    auto it = map_.find(key);
    if (it != map_.end()) {
      list_.splice(list_.end(), list_, list_.iterator_to(it->second));
      return false;
    }

    if (list_.size() == max_size_) {
      auto node = map_.extract(GetLeastRecentKey());

      list_.splice(list_.end(), list_, list_.begin());

      node.key() = key;
      map_.insert(std::move(node));
    } else {
      auto [it, ok] = map_.emplace(key, LruNode{});
      assert(ok);
      list_.insert(list_.end(), it->second);
    }

    return true;
  }

  bool Has(const T& key) {
    auto it = map_.find(key);
    if (it == map_.end()) return false;

    list_.splice(list_.end(), list_, list_.iterator_to(it->second));
    return true;
  }

 private:
  using List = boost::intrusive::list<
      LruNode,
      boost::intrusive::constant_time_size<false>  // Beware!
      >;

  using Map = std::unordered_map<T, LruNode, Hash, Equal>;

  Map map_;
  List list_;
  std::size_t max_size_;

  const T& GetLeastRecentKey() {
    using Pair = typename Map::value_type;

    // Map stores `value_type` and `second` of that `value_type` is an `LruNode`
    // in `list_`.
    constexpr auto offset = offsetof(Pair, second) - offsetof(Pair, first);
    return *reinterpret_cast<const T*>(
        reinterpret_cast<const char*>(&list_.front()) - offset);
  }
};

}  // namespace v4

namespace v5 {

using LinkMode = boost::intrusive::link_mode<
#ifdef NDEBUG
    boost::intrusive::normal_link
#else
    boost::intrusive::safe_link
#endif
    >;

using LruListHook = boost::intrusive::list_base_hook<LinkMode>;
using LruHashSetHook = boost::intrusive::unordered_set_base_hook<LinkMode>;

template <class Key>
class LruNode final : public LruListHook, public LruHashSetHook {
 public:
  explicit LruNode(Key&& key) : key_(std::move(key)) {}

  const Key& GetKey() const noexcept { return key_; }
  void SetKey(Key key) { key_ = std::move(key); }

 private:
  Key key_;
};

template <class Key>
const Key& GetKey(const LruNode<Key>& node) noexcept {
  return node.GetKey();
}

template <class T>
const T& GetKey(const T& key) noexcept {
  return key;
}

template <typename T, typename Hash = std::hash<T>,
          typename Equal = std::equal_to<T>>
class LruSet final {
 public:
  explicit LruSet(size_t max_size)
      : buckets_(max_size ? max_size : 1),
        map_(BucketTraits(buckets_.data(), buckets_.size())) {}

  ~LruSet() {
    while (!list_.empty()) {
      ExtractNode(list_.begin());
    }
  }

  LruSet(LruSet&& lru) = default;
  LruSet(const LruSet& lru) = delete;
  LruSet& operator=(LruSet&& lru) = default;
  LruSet& operator=(const LruSet& lru) = delete;

  bool Put(const T& key) {
    auto it = map_.find(key, map_.hash_function(), map_.key_eq());
    if (it != map_.end()) {
      list_.splice(list_.end(), list_, list_.iterator_to(*it));
      return false;
    }

    if (map_.size() == buckets_.size()) {
      auto node = ExtractNode(list_.begin());
      node->SetKey(key);
      InsertNode(std::move(node));
    } else {
      auto node = std::make_unique<LruNode>(T(key));
      InsertNode(std::move(node));
    }

    return true;
  }

  bool Has(const T& key) {
    auto it = map_.find(key, map_.hash_function(), map_.key_eq());
    if (it == map_.end()) return false;

    list_.splice(list_.end(), list_, list_.iterator_to(*it));
    return true;
  }

 private:
  using LruNode = v5::LruNode<T>;
  using List = boost::intrusive::list<
      LruNode,
      boost::intrusive::constant_time_size<false>  // Beware!
      >;

  struct LruNodeHash : Hash {
    template <class NodeOrKey>
    auto operator()(const NodeOrKey& x) const {
      return Hash::operator()(v5::GetKey(x));
    }
  };

  struct LruNodeEqual : Equal {
    template <class NodeOrKey1, class NodeOrKey2>
    auto operator()(const NodeOrKey1& x, const NodeOrKey2& y) const {
      return Equal::operator()(v5::GetKey(x), v5::GetKey(y));
    }
  };

  using Map = boost::intrusive::unordered_set<
      LruNode, boost::intrusive::constant_time_size<true>,
      boost::intrusive::hash<LruNodeHash>,
      boost::intrusive::equal<LruNodeEqual>>;

  using BucketTraits = typename Map::bucket_traits;
  using BucketType = typename Map::bucket_type;

  std::unique_ptr<LruNode> ExtractNode(typename List::iterator it) noexcept {
    std::unique_ptr<LruNode> ret(&*it);
    map_.erase(map_.iterator_to(*it));
    list_.erase(it);
    return ret;
  }

  void InsertNode(std::unique_ptr<LruNode> node) noexcept {
    if (!node) return;

    map_.insert(*node);                // noexcept
    list_.insert(list_.end(), *node);  // noexcept

    [[maybe_unused]] auto ignore = node.release();
  }

  std::vector<BucketType> buckets_;
  Map map_;
  List list_;
};

}  // namespace v5

namespace {
constexpr unsigned kElementsCount = 1000;

template <class Lru>
Lru FillLru(unsigned elements_count) {
  Lru lru(kElementsCount);
  for (unsigned i = 0; i < elements_count; ++i) {
    lru.Put(i);
  }

  return lru;
}

template <class Lru>
void ValidateSanity() {
  Lru cache(2);
  cache.Put(1);
  cache.Put(2);
  cache.Put(3);
  if (cache.Has(1)) throw 1;
  if (!cache.Has(2)) throw 2u;
  if (!cache.Has(3)) throw 3ull;

  cache.Put(1);
  cache.Put(2);
  cache.Put(1);
  cache.Put(3);
  if (!cache.Has(1)) throw 1;
  if (cache.Has(2)) throw 2u;
  if (!cache.Has(3)) throw 3ull;
}

}  // namespace

template <class Lru>
void LruPut(benchmark::State& state) {
  ValidateSanity<Lru>();
  for (auto _ : state) {
    auto lru = FillLru<Lru>(kElementsCount);
    benchmark::DoNotOptimize(lru);
  }
}
BENCHMARK_TEMPLATE(LruPut, v1::LruSet<unsigned>);
BENCHMARK_TEMPLATE(LruPut, v2::LruSet<unsigned>);
BENCHMARK_TEMPLATE(LruPut, v3::LruSet<unsigned>);
BENCHMARK_TEMPLATE(LruPut, v4::LruSet<unsigned>);
BENCHMARK_TEMPLATE(LruPut, v5::LruSet<unsigned>);

template <class Lru>
void LruHas(benchmark::State& state) {
  auto lru = FillLru<Lru>(kElementsCount);
  for (auto _ : state) {
    for (unsigned i = 0; i < kElementsCount; ++i) {
      benchmark::DoNotOptimize(lru.Has(i));
    }
  }
}
BENCHMARK_TEMPLATE(LruHas, v1::LruSet<unsigned>);
BENCHMARK_TEMPLATE(LruHas, v2::LruSet<unsigned>);
BENCHMARK_TEMPLATE(LruHas, v3::LruSet<unsigned>);
BENCHMARK_TEMPLATE(LruHas, v4::LruSet<unsigned>);
BENCHMARK_TEMPLATE(LruHas, v5::LruSet<unsigned>);

template <class Lru>
void LruPutOverflow(benchmark::State& state) {
  auto lru = FillLru<Lru>(kElementsCount);
  unsigned i = kElementsCount;
  for (auto _ : state) {
    for (unsigned j = 0; j < kElementsCount; ++j) {
      lru.Put(++i);
    }
    benchmark::DoNotOptimize(lru);
  }
}
BENCHMARK_TEMPLATE(LruPutOverflow, v1::LruSet<unsigned>);
BENCHMARK_TEMPLATE(LruPutOverflow, v2::LruSet<unsigned>);
BENCHMARK_TEMPLATE(LruPutOverflow, v3::LruSet<unsigned>);
BENCHMARK_TEMPLATE(LruPutOverflow, v4::LruSet<unsigned>);
BENCHMARK_TEMPLATE(LruPutOverflow, v5::LruSet<unsigned>);

BENCHMARK_MAIN();
