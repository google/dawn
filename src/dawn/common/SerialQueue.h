// Copyright 2017 The Dawn Authors
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

#ifndef SRC_DAWN_COMMON_SERIALQUEUE_H_
#define SRC_DAWN_COMMON_SERIALQUEUE_H_

#include <utility>
#include <vector>

#include "dawn/common/SerialStorage.h"

template <typename Serial, typename Value>
class SerialQueue;

template <typename SerialT, typename ValueT>
struct SerialStorageTraits<SerialQueue<SerialT, ValueT>> {
    using Serial = SerialT;
    using Value = ValueT;
    using SerialPair = std::pair<Serial, std::vector<Value>>;
    using Storage = std::vector<SerialPair>;
    using StorageIterator = typename Storage::iterator;
    using ConstStorageIterator = typename Storage::const_iterator;
};

// SerialQueue stores an associative list mapping a Serial to Value.
// It enforces that the Serials enqueued are strictly non-decreasing.
// This makes it very efficient iterate or clear all items added up
// to some Serial value because they are stored contiguously in memory.
template <typename Serial, typename Value>
class SerialQueue : public SerialStorage<SerialQueue<Serial, Value>> {
  public:
    // The serial must be given in (not strictly) increasing order.
    void Enqueue(const Value& value, Serial serial);
    void Enqueue(Value&& value, Serial serial);
    void Enqueue(const std::vector<Value>& values, Serial serial);
    void Enqueue(std::vector<Value>&& values, Serial serial);
};

// SerialQueue

template <typename Serial, typename Value>
void SerialQueue<Serial, Value>::Enqueue(const Value& value, Serial serial) {
    DAWN_ASSERT(this->Empty() || this->mStorage.back().first <= serial);

    if (this->Empty() || this->mStorage.back().first < serial) {
        this->mStorage.emplace_back(serial, std::vector<Value>{});
    }
    this->mStorage.back().second.push_back(value);
}

template <typename Serial, typename Value>
void SerialQueue<Serial, Value>::Enqueue(Value&& value, Serial serial) {
    DAWN_ASSERT(this->Empty() || this->mStorage.back().first <= serial);

    if (this->Empty() || this->mStorage.back().first < serial) {
        this->mStorage.emplace_back(serial, std::vector<Value>{});
    }
    this->mStorage.back().second.push_back(std::move(value));
}

template <typename Serial, typename Value>
void SerialQueue<Serial, Value>::Enqueue(const std::vector<Value>& values, Serial serial) {
    DAWN_ASSERT(values.size() > 0);
    DAWN_ASSERT(this->Empty() || this->mStorage.back().first <= serial);
    this->mStorage.emplace_back(serial, values);
}

template <typename Serial, typename Value>
void SerialQueue<Serial, Value>::Enqueue(std::vector<Value>&& values, Serial serial) {
    DAWN_ASSERT(values.size() > 0);
    DAWN_ASSERT(this->Empty() || this->mStorage.back().first <= serial);
    this->mStorage.emplace_back(serial, std::move(values));
}

#endif  // SRC_DAWN_COMMON_SERIALQUEUE_H_
