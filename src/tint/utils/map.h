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

#ifndef SRC_TINT_UTILS_MAP_H_
#define SRC_TINT_UTILS_MAP_H_

#include <unordered_map>

namespace tint::utils {

/// Lookup is a utility function for fetching a value from an unordered map if
/// it exists, otherwise returning the `if_missing` argument.
/// @param map the unordered_map
/// @param key the map key of the item to query
/// @param if_missing the value to return if the map does not contain the given
/// key. Defaults to the zero-initializer for the value type.
/// @return the map item value, or `if_missing` if the map does not contain the
/// given key
template <typename K, typename V, typename H, typename C, typename KV = K>
V Lookup(const std::unordered_map<K, V, H, C>& map, const KV& key, const V& if_missing = {}) {
    auto it = map.find(key);
    return it != map.end() ? it->second : if_missing;
}

/// GetOrCreate is a utility function for lazily adding to an unordered map.
/// If the map already contains the key `key` then this is returned, otherwise
/// `create()` is called and the result is added to the map and is returned.
/// @param map the unordered_map
/// @param key the map key of the item to query or add
/// @param create a callable function-like object with the signature `V()`
/// @return the value of the item with the given key, or the newly created item
template <typename K, typename V, typename H, typename C, typename CREATE>
V GetOrCreate(std::unordered_map<K, V, H, C>& map, const K& key, CREATE&& create) {
    auto it = map.find(key);
    if (it != map.end()) {
        return it->second;
    }
    V value = create();
    map.emplace(key, value);
    return value;
}

}  // namespace tint::utils

#endif  // SRC_TINT_UTILS_MAP_H_
