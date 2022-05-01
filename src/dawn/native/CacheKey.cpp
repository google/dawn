// Copyright 2022 The Dawn Authors
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

#include "dawn/native/CacheKey.h"

#include <iomanip>

namespace dawn::native {

std::ostream& operator<<(std::ostream& os, const CacheKey& key) {
    os << std::hex;
    for (const int b : key) {
        os << std::setfill('0') << std::setw(2) << b << " ";
    }
    os << std::dec;
    return os;
}

template <>
void CacheKeySerializer<std::string>::Serialize(CacheKey* key, const std::string& t) {
    key->Record(static_cast<size_t>(t.length()));
    key->insert(key->end(), t.begin(), t.end());
}

template <>
void CacheKeySerializer<CacheKey>::Serialize(CacheKey* key, const CacheKey& t) {
    // For nested cache keys, we do not record the length, and just copy the key so that it
    // appears we just flatten the keys into a single key.
    key->insert(key->end(), t.begin(), t.end());
}

}  // namespace dawn::native
