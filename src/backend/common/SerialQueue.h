// Copyright 2017 The NXT Authors
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

#ifndef BACKEND_COMMON_SERIALQUEUE_H_
#define BACKEND_COMMON_SERIALQUEUE_H_

#include "Forward.h"

#include <cstdint>
#include <vector>

namespace backend {

    using Serial = uint64_t;

    template<typename T>
    class SerialQueue {
        private:
            using SerialPair = std::pair<Serial, std::vector<T>>;
            using Storage = std::vector<SerialPair>;
            using StorageIterator = typename Storage::const_iterator;

        public:
            class Iterator {
                public:
                    Iterator(StorageIterator start);
                    Iterator& operator++();

                    bool operator==(const Iterator& other) const;
                    bool operator!=(const Iterator& other) const;
                    const T& operator*() const;

                private:
                    StorageIterator storageIterator;
                    // Special case the serialIterator when it should be equal to storageIterator.begin()
                    // otherwise we could ask storageIterator.begin() when storageIterator is storage.end()
                    // which is invalid. storageIterator.begin() is tagged with a nullptr.
                    const T* serialIterator;
            };

            class BeginEnd {
                public:
                    BeginEnd(StorageIterator start, StorageIterator end);

                    Iterator begin() const;
                    Iterator end() const;

                private:
                    StorageIterator startIt;
                    StorageIterator endIt;
            };

            // The serial must be given in (not strictly) increasing order.
            void Enqueue(const T& value, Serial serial);
            void Enqueue(T&& value, Serial serial);
            void Enqueue(const std::vector<T>& values, Serial serial);
            void Enqueue(std::vector<T>&& values, Serial serial);

            bool Empty() const;

            // The UpTo variants of Iterate and Clear affect all values associated to a serial
            // that is smaller OR EQUAL to the given serial. Iterating is done like so:
            //     for (const T& value : queue.IterateAll()) { stuff(T); }
            BeginEnd IterateAll() const;
            BeginEnd IterateUpTo(Serial serial) const;

            void Clear();
            void ClearUpTo(Serial serial);

        private:
            // Returns the first StorageIterator that a serial bigger than serial.
            StorageIterator FindUpTo(Serial serial) const;
            Storage storage;
    };

    // SerialQueue

    template<typename T>
    void SerialQueue<T>::Enqueue(const T& value, Serial serial) {
        ASSERT(Empty() || storage.back().first <= serial);

        if (Empty() || storage.back().first < serial) {
            storage.emplace_back(SerialPair(serial, {}));
        }
        storage.back().second.push_back(value);
    }

    template<typename T>
    void SerialQueue<T>::Enqueue(T&& value, Serial serial) {
        ASSERT(Empty() || storage.back().first <= serial);

        if (Empty() || storage.back().first < serial) {
            storage.emplace_back(SerialPair(serial, {}));
        }
        storage.back().second.push_back(value);
    }

    template<typename T>
    void SerialQueue<T>::Enqueue(const std::vector<T>& values, Serial serial) {
        ASSERT(Empty() || storage.back().first <= serial);
        storage.emplace_back(SerialPair(serial, {values}));
    }

    template<typename T>
    void SerialQueue<T>::Enqueue(std::vector<T>&& values, Serial serial) {
        ASSERT(Empty() || storage.back().first <= serial);
        storage.emplace_back(SerialPair(serial, {values}));
    }

    template<typename T>
    bool SerialQueue<T>::Empty() const {
        return storage.empty();
    }

    template<typename T>
    typename SerialQueue<T>::BeginEnd SerialQueue<T>::IterateAll() const {
        return {storage.begin(), storage.end()};
    }

    template<typename T>
    typename SerialQueue<T>::BeginEnd SerialQueue<T>::IterateUpTo(Serial serial) const {
        return {storage.begin(), FindUpTo(serial)};
    }

    template<typename T>
    void SerialQueue<T>::Clear() {
        storage.clear();
    }

    template<typename T>
    void SerialQueue<T>::ClearUpTo(Serial serial) {
        storage.erase(storage.begin(), FindUpTo(serial));
    }

    template<typename T>
    typename SerialQueue<T>::StorageIterator SerialQueue<T>::FindUpTo(Serial serial) const {
        auto it = storage.begin();
        while (it != storage.end() && it->first <= serial) {
            it ++;
        }
        return it;
    }

    // SerialQueue::BeginEnd

    template<typename T>
    SerialQueue<T>::BeginEnd::BeginEnd(typename SerialQueue<T>::StorageIterator start, typename SerialQueue<T>::StorageIterator end)
        : startIt(start), endIt(end) {
    }

    template<typename T>
    typename SerialQueue<T>::Iterator SerialQueue<T>::BeginEnd::begin() const {
        return {startIt};
    }

    template<typename T>
    typename SerialQueue<T>::Iterator SerialQueue<T>::BeginEnd::end() const {
        return {endIt};
    }

    // SerialQueue::Iterator

    template<typename T>
    SerialQueue<T>::Iterator::Iterator(typename SerialQueue<T>::StorageIterator start)
        : storageIterator(start), serialIterator(nullptr) {
    }

    template<typename T>
    typename SerialQueue<T>::Iterator& SerialQueue<T>::Iterator::operator++() {
        const T* vectorData = storageIterator->second.data();

        if (serialIterator == nullptr) {
            serialIterator = vectorData + 1;
        } else {
            serialIterator ++;
        }

        if (serialIterator >= vectorData + storageIterator->second.size()) {
            serialIterator = nullptr;
            storageIterator ++;
        }

        return *this;
    }

    template<typename T>
    bool SerialQueue<T>::Iterator::operator==(const typename SerialQueue<T>::Iterator& other) const {
        return other.storageIterator == storageIterator && other.serialIterator == serialIterator;
    }

    template<typename T>
    bool SerialQueue<T>::Iterator::operator!=(const typename SerialQueue<T>::Iterator& other) const {
        return !(*this == other);
    }

    template<typename T>
    const T& SerialQueue<T>::Iterator::operator*() const {
        if (serialIterator == nullptr) {
            return *storageIterator->second.begin();
        }
        return *serialIterator;
    }

}

#endif // BACKEND_COMMON_SERIALQUEUE_H_
