// Copyright 2021 The Tint Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef SRC_TINT_FUZZERS_TINT_SPIRV_TOOLS_FUZZER_MUTATOR_CACHE_H_
#define SRC_TINT_FUZZERS_TINT_SPIRV_TOOLS_FUZZER_MUTATOR_CACHE_H_

#include <cassert>
#include <list>
#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>

#include "src/tint/fuzzers/tint_spirv_tools_fuzzer/mutator.h"

namespace tint::fuzzers::spvtools_fuzzer {

/// Implementation of a fixed size LRU cache. That is, when the number of
/// elements reaches a certain threshold, the element that wasn't used for the
/// longest period of time is removed from the cache when a new element is
/// inserted. All operations have amortized constant time complexity.
class MutatorCache {
  public:
    /// SPIR-V binary that is being mutated.
    using Key = std::vector<uint32_t>;

    /// Mutator that is used to mutate the `Key`.
    using Value = std::unique_ptr<Mutator>;

    /// Constructor.
    /// @param max_size - the maximum number of elements the cache can store. May
    ///     not be equal to 0.
    explicit MutatorCache(size_t max_size);

    /// Retrieves a pointer to a value, associated with a given `key`.
    ///
    /// If the key is present in the cache, its usage is updated and the
    /// (non-null) pointer to the value is returned. Otherwise, `nullptr` is
    /// returned.
    ///
    /// @param key - may not exist in this cache.
    /// @return non-`nullptr` pointer to a value if `key` exists in the cache.
    /// @return `nullptr` if `key` doesn't exist in this cache.
    Value::pointer Get(const Key& key);

    /// Inserts a `key`-`value` pair into the cache.
    ///
    /// If the `key` is already present, the `value` replaces the old value and
    /// the usage of `key` is updated. If the `key` is not present, then:
    /// - if the number of elements in the cache is equal to `max_size`, the
    ///   key-value pair, where the usage of the key wasn't updated for the
    ///   longest period of time, is removed from the cache.
    /// - a new `key`-`value` pair is inserted into the cache.
    ///
    /// @param key - a key.
    /// @param value - may not be a `nullptr`.
    void Put(const Key& key, Value value);

    /// Removes `key` and an associated value from the cache.
    ///
    /// @param key - a key.
    /// @return a non-`nullptr` pointer to the removed value, associated with
    ///     `key`.
    /// @return `nullptr` if `key` is not present in the cache.
    Value Remove(const Key& key);

  private:
    struct KeyHash {
        size_t operator()(const std::vector<uint32_t>& vec) const;
    };

    using Entry = std::pair<const Key*, Value>;
    using Map = std::unordered_map<Key, std::list<Entry>::iterator, KeyHash>;

    void UpdateUsage(Map::iterator it);

    Map map_;
    std::list<Entry> entries_;
    const size_t max_size_;
};

}  // namespace tint::fuzzers::spvtools_fuzzer

#endif  // SRC_TINT_FUZZERS_TINT_SPIRV_TOOLS_FUZZER_MUTATOR_CACHE_H_
