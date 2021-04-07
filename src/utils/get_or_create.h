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

#ifndef SRC_UTILS_GET_OR_CREATE_H_
#define SRC_UTILS_GET_OR_CREATE_H_

#include <unordered_map>

namespace tint {
namespace utils {

/// GetOrCreate is a utility function for lazily adding to an unordered map.
/// If the map already contains the key `key` then this is returned, otherwise
/// `create()` is called and the result is added to the map and is returned.
/// @param map the unordered_map
/// @param key the map key of the item to query or add
/// @param create a callable function-like object with the signature `V()`
/// @return the value of the item with the given key, or the newly created item
template <typename K, typename V, typename CREATE, typename H>
V GetOrCreate(std::unordered_map<K, V, H>& map, K key, CREATE&& create) {
  auto it = map.find(key);
  if (it != map.end()) {
    return it->second;
  }
  V value = create();
  map.emplace(key, value);
  return value;
}

}  // namespace utils
}  // namespace tint

#endif  //  SRC_UTILS_GET_OR_CREATE_H_
