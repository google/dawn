// Copyright 2018 The Dawn Authors
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

#ifndef SRC_DAWN_COMMON_SERIALMAP_H_
#define SRC_DAWN_COMMON_SERIALMAP_H_

#include <map>
#include <utility>
#include <vector>

#include "dawn/common/SerialStorage.h"

template <typename Serial, typename Value>
class SerialMap;

template <typename SerialT, typename ValueT>
struct SerialStorageTraits<SerialMap<SerialT, ValueT>> {
    using Serial = SerialT;
    using Value = ValueT;
    using Storage = std::map<Serial, std::vector<Value>>;
    using StorageIterator = typename Storage::iterator;
    using ConstStorageIterator = typename Storage::const_iterator;
};

// SerialMap stores a map from Serial to Value.
// Unlike SerialQueue, items may be enqueued with Serials in any
// arbitrary order. SerialMap provides useful iterators for iterating
// through Value items in order of increasing Serial.
template <typename Serial, typename Value>
class SerialMap : public SerialStorage<SerialMap<Serial, Value>> {
  public:
    void Enqueue(const Value& value, Serial serial);
    void Enqueue(Value&& value, Serial serial);
    void Enqueue(const std::vector<Value>& values, Serial serial);
    void Enqueue(std::vector<Value>&& values, Serial serial);
};

// SerialMap

template <typename Serial, typename Value>
void SerialMap<Serial, Value>::Enqueue(const Value& value, Serial serial) {
    this->mStorage[serial].emplace_back(value);
}

template <typename Serial, typename Value>
void SerialMap<Serial, Value>::Enqueue(Value&& value, Serial serial) {
    this->mStorage[serial].emplace_back(std::move(value));
}

template <typename Serial, typename Value>
void SerialMap<Serial, Value>::Enqueue(const std::vector<Value>& values, Serial serial) {
    DAWN_ASSERT(values.size() > 0);
    for (const Value& value : values) {
        Enqueue(value, serial);
    }
}

template <typename Serial, typename Value>
void SerialMap<Serial, Value>::Enqueue(std::vector<Value>&& values, Serial serial) {
    DAWN_ASSERT(values.size() > 0);
    for (Value& value : values) {
        Enqueue(std::move(value), serial);
    }
}

#endif  // SRC_DAWN_COMMON_SERIALMAP_H_
