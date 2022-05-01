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

#include "src/tint/fuzzers/tint_spirv_tools_fuzzer/mutator_cache.h"

namespace tint::fuzzers::spvtools_fuzzer {

MutatorCache::MutatorCache(size_t max_size) : map_(), entries_(), max_size_(max_size) {
    assert(max_size && "`max_size` may not be 0");
}

MutatorCache::Value::pointer MutatorCache::Get(const Key& key) {
    auto it = map_.find(key);
    if (it == map_.end()) {
        return nullptr;
    }
    UpdateUsage(it);
    return entries_.front().second.get();
}

void MutatorCache::Put(const Key& key, Value value) {
    assert(value && "Mutator cache can't have nullptr unique_ptr");
    auto it = map_.find(key);
    if (it != map_.end()) {
        it->second->second = std::move(value);
        UpdateUsage(it);
    } else {
        if (map_.size() == max_size_) {
            Remove(*entries_.back().first);
        }

        entries_.emplace_front(nullptr, std::move(value));
        auto pair = map_.emplace(key, entries_.begin());
        assert(pair.second && "The key must be unique");
        entries_.front().first = &pair.first->first;
    }
}

MutatorCache::Value MutatorCache::Remove(const Key& key) {
    auto it = map_.find(key);
    if (it == map_.end()) {
        return nullptr;
    }
    auto result = std::move(it->second->second);
    entries_.erase(it->second);
    map_.erase(it);
    return result;
}

size_t MutatorCache::KeyHash::operator()(const std::vector<uint32_t>& vec) const {
    return std::hash<std::u32string>()({vec.begin(), vec.end()});
}

void MutatorCache::UpdateUsage(Map::iterator it) {
    auto entry = std::move(*it->second);
    entries_.erase(it->second);
    entries_.push_front(std::move(entry));
    it->second = entries_.begin();
}

}  // namespace tint::fuzzers::spvtools_fuzzer
