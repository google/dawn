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

#ifndef COMMON_SERIALQUEUE_H_
#define COMMON_SERIALQUEUE_H_

#include "common/Assert.h"
#include "common/Serial.h"

#include <cstdint>
#include <vector>

template <typename T>
class SerialQueue {
  private:
    using SerialPair = std::pair<Serial, std::vector<T>>;
    using Storage = std::vector<SerialPair>;
    using StorageIterator = typename Storage::iterator;
    using ConstStorageIterator = typename Storage::const_iterator;

  public:
    class Iterator {
      public:
        Iterator(StorageIterator start);
        Iterator& operator++();

        bool operator==(const Iterator& other) const;
        bool operator!=(const Iterator& other) const;
        T& operator*() const;

      private:
        StorageIterator mStorageIterator;
        // Special case the mSerialIterator when it should be equal to mStorageIterator.begin()
        // otherwise we could ask mStorageIterator.begin() when mStorageIterator is mStorage.end()
        // which is invalid. mStorageIterator.begin() is tagged with a nullptr.
        T* mSerialIterator;
    };

    class ConstIterator {
      public:
        ConstIterator(ConstStorageIterator start);
        ConstIterator& operator++();

        bool operator==(const ConstIterator& other) const;
        bool operator!=(const ConstIterator& other) const;
        const T& operator*() const;

      private:
        ConstStorageIterator mStorageIterator;
        const T* mSerialIterator;
    };

    class BeginEnd {
      public:
        BeginEnd(StorageIterator start, StorageIterator end);

        Iterator begin() const;
        Iterator end() const;

      private:
        StorageIterator mStartIt;
        StorageIterator mEndIt;
    };

    class ConstBeginEnd {
      public:
        ConstBeginEnd(ConstStorageIterator start, ConstStorageIterator end);

        ConstIterator begin() const;
        ConstIterator end() const;

      private:
        ConstStorageIterator mStartIt;
        ConstStorageIterator mEndIt;
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
    ConstBeginEnd IterateAll() const;
    ConstBeginEnd IterateUpTo(Serial serial) const;
    BeginEnd IterateAll();
    BeginEnd IterateUpTo(Serial serial);

    void Clear();
    void ClearUpTo(Serial serial);

    Serial FirstSerial() const;

  private:
    // Returns the first StorageIterator that a serial bigger than serial.
    ConstStorageIterator FindUpTo(Serial serial) const;
    StorageIterator FindUpTo(Serial serial);
    Storage mStorage;
};

// SerialQueue

template <typename T>
void SerialQueue<T>::Enqueue(const T& value, Serial serial) {
    NXT_ASSERT(Empty() || mStorage.back().first <= serial);

    if (Empty() || mStorage.back().first < serial) {
        mStorage.emplace_back(SerialPair(serial, {}));
    }
    mStorage.back().second.emplace_back(value);
}

template <typename T>
void SerialQueue<T>::Enqueue(T&& value, Serial serial) {
    NXT_ASSERT(Empty() || mStorage.back().first <= serial);

    if (Empty() || mStorage.back().first < serial) {
        mStorage.emplace_back(SerialPair(serial, {}));
    }
    mStorage.back().second.emplace_back(value);
}

template <typename T>
void SerialQueue<T>::Enqueue(const std::vector<T>& values, Serial serial) {
    NXT_ASSERT(values.size() > 0);
    NXT_ASSERT(Empty() || mStorage.back().first <= serial);
    mStorage.emplace_back(SerialPair(serial, {values}));
}

template <typename T>
void SerialQueue<T>::Enqueue(std::vector<T>&& values, Serial serial) {
    NXT_ASSERT(values.size() > 0);
    NXT_ASSERT(Empty() || mStorage.back().first <= serial);
    mStorage.emplace_back(SerialPair(serial, {values}));
}

template <typename T>
bool SerialQueue<T>::Empty() const {
    return mStorage.empty();
}

template <typename T>
typename SerialQueue<T>::ConstBeginEnd SerialQueue<T>::IterateAll() const {
    return {mStorage.begin(), mStorage.end()};
}

template <typename T>
typename SerialQueue<T>::ConstBeginEnd SerialQueue<T>::IterateUpTo(Serial serial) const {
    return {mStorage.begin(), FindUpTo(serial)};
}

template <typename T>
typename SerialQueue<T>::BeginEnd SerialQueue<T>::IterateAll() {
    return {mStorage.begin(), mStorage.end()};
}

template <typename T>
typename SerialQueue<T>::BeginEnd SerialQueue<T>::IterateUpTo(Serial serial) {
    return {mStorage.begin(), FindUpTo(serial)};
}

template <typename T>
void SerialQueue<T>::Clear() {
    mStorage.clear();
}

template <typename T>
void SerialQueue<T>::ClearUpTo(Serial serial) {
    mStorage.erase(mStorage.begin(), FindUpTo(serial));
}

template <typename T>
Serial SerialQueue<T>::FirstSerial() const {
    NXT_ASSERT(!Empty());
    return mStorage.front().first;
}

template <typename T>
typename SerialQueue<T>::ConstStorageIterator SerialQueue<T>::FindUpTo(Serial serial) const {
    auto it = mStorage.begin();
    while (it != mStorage.end() && it->first <= serial) {
        it++;
    }
    return it;
}

template <typename T>
typename SerialQueue<T>::StorageIterator SerialQueue<T>::FindUpTo(Serial serial) {
    auto it = mStorage.begin();
    while (it != mStorage.end() && it->first <= serial) {
        it++;
    }
    return it;
}

// SerialQueue::BeginEnd

template <typename T>
SerialQueue<T>::BeginEnd::BeginEnd(typename SerialQueue<T>::StorageIterator start,
                                   typename SerialQueue<T>::StorageIterator end)
    : mStartIt(start), mEndIt(end) {
}

template <typename T>
typename SerialQueue<T>::Iterator SerialQueue<T>::BeginEnd::begin() const {
    return {mStartIt};
}

template <typename T>
typename SerialQueue<T>::Iterator SerialQueue<T>::BeginEnd::end() const {
    return {mEndIt};
}

// SerialQueue::Iterator

template <typename T>
SerialQueue<T>::Iterator::Iterator(typename SerialQueue<T>::StorageIterator start)
    : mStorageIterator(start), mSerialIterator(nullptr) {
}

template <typename T>
typename SerialQueue<T>::Iterator& SerialQueue<T>::Iterator::operator++() {
    T* vectorData = mStorageIterator->second.data();

    if (mSerialIterator == nullptr) {
        mSerialIterator = vectorData + 1;
    } else {
        mSerialIterator++;
    }

    if (mSerialIterator >= vectorData + mStorageIterator->second.size()) {
        mSerialIterator = nullptr;
        mStorageIterator++;
    }

    return *this;
}

template <typename T>
bool SerialQueue<T>::Iterator::operator==(const typename SerialQueue<T>::Iterator& other) const {
    return other.mStorageIterator == mStorageIterator && other.mSerialIterator == mSerialIterator;
}

template <typename T>
bool SerialQueue<T>::Iterator::operator!=(const typename SerialQueue<T>::Iterator& other) const {
    return !(*this == other);
}

template <typename T>
T& SerialQueue<T>::Iterator::operator*() const {
    if (mSerialIterator == nullptr) {
        return *mStorageIterator->second.begin();
    }
    return *mSerialIterator;
}

// SerialQueue::ConstBeginEnd

template <typename T>
SerialQueue<T>::ConstBeginEnd::ConstBeginEnd(typename SerialQueue<T>::ConstStorageIterator start,
                                             typename SerialQueue<T>::ConstStorageIterator end)
    : mStartIt(start), mEndIt(end) {
}

template <typename T>
typename SerialQueue<T>::ConstIterator SerialQueue<T>::ConstBeginEnd::begin() const {
    return {mStartIt};
}

template <typename T>
typename SerialQueue<T>::ConstIterator SerialQueue<T>::ConstBeginEnd::end() const {
    return {mEndIt};
}

// SerialQueue::ConstIterator

template <typename T>
SerialQueue<T>::ConstIterator::ConstIterator(typename SerialQueue<T>::ConstStorageIterator start)
    : mStorageIterator(start), mSerialIterator(nullptr) {
}

template <typename T>
typename SerialQueue<T>::ConstIterator& SerialQueue<T>::ConstIterator::operator++() {
    const T* vectorData = mStorageIterator->second.data();

    if (mSerialIterator == nullptr) {
        mSerialIterator = vectorData + 1;
    } else {
        mSerialIterator++;
    }

    if (mSerialIterator >= vectorData + mStorageIterator->second.size()) {
        mSerialIterator = nullptr;
        mStorageIterator++;
    }

    return *this;
}

template <typename T>
bool SerialQueue<T>::ConstIterator::operator==(
    const typename SerialQueue<T>::ConstIterator& other) const {
    return other.mStorageIterator == mStorageIterator && other.mSerialIterator == mSerialIterator;
}

template <typename T>
bool SerialQueue<T>::ConstIterator::operator!=(
    const typename SerialQueue<T>::ConstIterator& other) const {
    return !(*this == other);
}

template <typename T>
const T& SerialQueue<T>::ConstIterator::operator*() const {
    if (mSerialIterator == nullptr) {
        return *mStorageIterator->second.begin();
    }
    return *mSerialIterator;
}

#endif  // COMMON_SERIALQUEUE_H_
