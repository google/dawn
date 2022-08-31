// Copyright 2022 The Tint Authors.
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

#ifndef SRC_TINT_UTILS_HASHSET_H_
#define SRC_TINT_UTILS_HASHSET_H_

#include <stddef.h>
#include <algorithm>
#include <functional>
#include <optional>
#include <tuple>
#include <utility>

#include "src/tint/debug.h"
#include "src/tint/utils/hash.h"
#include "src/tint/utils/vector.h"

namespace tint::utils {

/// Action taken by Hashset::Insert()
enum class AddAction {
    /// Insert() added a new entry to the Hashset
    kAdded,
    /// Insert() replaced an existing entry in the Hashset
    kReplaced,
    /// Insert() found an existing entry, which was not replaced.
    kKeptExisting,
};

/// An unordered set that uses a robin-hood hashing algorithm.
/// @see the fantastic tutorial: https://programming.guide/robin-hood-hashing.html
template <typename T, size_t N, typename HASH = Hasher<T>, typename EQUAL = std::equal_to<T>>
class Hashset {
    /// A slot is a single entry in the underlying vector.
    /// A slot can either be empty or filled with a value. If the slot is empty, #hash and #distance
    /// will be zero.
    struct Slot {
        template <typename V>
        bool Equals(size_t value_hash, const V& val) const {
            return value_hash == hash && EQUAL()(val, value.value());
        }

        /// The slot value. If this does not contain a value, then the slot is vacant.
        std::optional<T> value;
        /// The precomputed hash of value.
        size_t hash = 0;
        size_t distance = 0;
    };

    /// The target length of the underlying vector length in relation to the number of entries in
    /// the set, expressed as a percentage. For example a value of `150` would mean there would be
    /// at least 50% more slots than the number of set entries.
    static constexpr size_t kRehashFactor = 150;

    /// @returns the target slot vector size to hold `n` set entries.
    static constexpr size_t NumSlots(size_t count) { return (count * kRehashFactor) / 100; }

    /// The fixed-size slot vector length, based on N and kRehashFactor.
    static constexpr size_t kNumFixedSlots = NumSlots(N);

    /// The minimum number of slots for the set.
    static constexpr size_t kMinSlots = std::max<size_t>(kNumFixedSlots, 4);

  public:
    /// Iterator for entries in the set.
    /// Iterators are invalidated if the set is modified.
    class Iterator {
      public:
        /// @returns the value pointed to by this iterator
        const T* operator->() const { return &current->value.value(); }

        /// Increments the iterator
        /// @returns this iterator
        Iterator& operator++() {
            if (current == end) {
                return *this;
            }
            current++;
            SkipToNextValue();
            return *this;
        }

        /// Equality operator
        /// @param other the other iterator to compare this iterator to
        /// @returns true if this iterator is equal to other
        bool operator==(const Iterator& other) const { return current == other.current; }

        /// Inequality operator
        /// @param other the other iterator to compare this iterator to
        /// @returns true if this iterator is not equal to other
        bool operator!=(const Iterator& other) const { return current != other.current; }

        /// @returns a reference to the value at the iterator
        const T& operator*() const { return current->value.value(); }

      private:
        /// Friend class
        friend class Hashset;

        Iterator(const Slot* c, const Slot* e) : current(c), end(e) { SkipToNextValue(); }

        /// Moves the iterator forward, stopping at the next slot that is not empty.
        void SkipToNextValue() {
            while (current != end && !current->value.has_value()) {
                current++;
            }
        }

        const Slot* current;  /// The slot the iterator is pointing to
        const Slot* end;      /// One past the last slot in the set
    };

    /// Type of `T`.
    using value_type = T;

    /// Constructor
    Hashset() { slots_.Resize(kMinSlots); }

    /// Copy constructor
    /// @param other the other Hashset to copy
    Hashset(const Hashset& other) = default;

    /// Move constructor
    /// @param other the other Hashset to move
    Hashset(Hashset&& other) = default;

    /// Destructor
    ~Hashset() { Clear(); }

    /// Copy-assignment operator
    /// @param other the other Hashset to copy
    /// @returns this so calls can be chained
    Hashset& operator=(const Hashset& other) = default;

    /// Move-assignment operator
    /// @param other the other Hashset to move
    /// @returns this so calls can be chained
    Hashset& operator=(Hashset&& other) = default;

    /// Removes all entries from the set.
    void Clear() {
        slots_.Clear();  // Destructs all entries
        slots_.Resize(kMinSlots);
        count_ = 0;
        generation_++;
    }

    /// Result of Add()
    struct AddResult {
        /// Whether the insert replaced or added a new entry to the set.
        AddAction action = AddAction::kAdded;
        /// A pointer to the inserted entry.
        /// @warning do not modify this pointer in a way that would cause the equality or hash of
        ///          the entry to change. Doing this will corrupt the Hashset.
        T* entry = nullptr;

        /// @returns true if the entry was added to the set, or an existing entry was replaced.
        operator bool() const { return action != AddAction::kKeptExisting; }
    };

    /// Adds a value to the set, if the set does not already contain an entry equal to `value`.
    /// @param value the value to add to the set.
    /// @returns A AddResult describing the result of the add
    /// @warning do not modify the inserted entry in a way that would cause the equality of hash of
    ///          the entry to change. Doing this will corrupt the Hashset.
    template <typename V>
    AddResult Add(V&& value) {
        return Put<PutMode::kAdd>(std::forward<V>(value));
    }

    /// Adds a value to the set, replacing any entry equal to `value`.
    /// @param value the value to add to the set.
    /// @returns A AddResult describing the result of the replace
    template <typename V>
    AddResult Replace(V&& value) {
        return Put<PutMode::kReplace>(std::forward<V>(value));
    }

    /// Removes an entry from the set.
    /// @param value the value to remove from the set.
    /// @returns true if an entry was removed.
    template <typename V>
    bool Remove(const V& value) {
        const auto [found, start] = IndexOf(value);
        if (!found) {
            return false;
        }

        // Shuffle the entries backwards until we either find a free slot, or a slot that has zero
        // distance.
        Slot* prev = nullptr;
        Scan(start, [&](size_t, size_t index) {
            auto& slot = slots_[index];
            if (prev) {
                // note: `distance == 0` also includes empty slots.
                if (slot.distance == 0) {
                    // Clear the previous slot, and stop shuffling.
                    *prev = {};
                    return Action::kStop;
                } else {
                    // Shuffle the slot backwards.
                    prev->value = std::move(slot.value);
                    prev->hash = slot.hash;
                    prev->distance = slot.distance - 1;
                }
            }
            prev = &slot;
            return Action::kContinue;
        });

        // Entry was removed.
        count_--;
        generation_++;

        return true;
    }

    /// @param value the value to search for.
    /// @returns the value of the entry that is equal to `value`, or no value if the entry was not
    ///          found.
    template <typename V>
    std::optional<T> Get(const V& value) const {
        if (const auto [found, index] = IndexOf(value); found) {
            return slots_[index].value.value();
        }
        return std::nullopt;
    }

    /// @param value the value to search for.
    /// @returns a pointer to the entry that is equal to the given value, or nullptr if the set does
    ///          not contain the given value.
    template <typename V>
    const T* Find(const V& value) const {
        const auto [found, index] = IndexOf(value);
        return found ? &slots_[index].value.value() : nullptr;
    }

    /// @param value the value to search for.
    /// @returns a pointer to the entry that is equal to the given value, or nullptr if the set does
    ///          not contain the given value.
    /// @warning do not modify the inserted entry in a way that would cause the equality of hash of
    ///          the entry to change. Doing this will corrupt the Hashset.
    template <typename V>
    T* Find(const V& value) {
        const auto [found, index] = IndexOf(value);
        return found ? &slots_[index].value.value() : nullptr;
    }

    /// Checks whether an entry exists in the set
    /// @param value the value to search for.
    /// @returns true if the set contains an entry with the given value.
    template <typename V>
    bool Contains(const V& value) const {
        const auto [found, _] = IndexOf(value);
        return found;
    }

    /// Pre-allocates memory so that the set can hold at least `capacity` entries.
    /// @param capacity the new capacity of the set.
    void Reserve(size_t capacity) {
        // Calculate the number of slots required to hold `capacity` entries.
        const size_t num_slots = std::max(NumSlots(capacity), kMinSlots);
        if (slots_.Length() >= num_slots) {
            // Already have enough slots.
            return;
        }

        // Move all the values out of the set and into a vector.
        Vector<T, N> values;
        values.Reserve(count_);
        for (auto& slot : slots_) {
            if (slot.value.has_value()) {
                values.Push(std::move(slot.value.value()));
            }
        }

        // Clear the set, grow the number of slots.
        Clear();
        slots_.Resize(num_slots);

        // As the number of slots has grown, the slot indices will have changed from before, so
        // re-add all the values back into the set.
        for (auto& value : values) {
            Add(std::move(value));
        }
    }

    /// @returns the number of entries in the set.
    size_t Count() const { return count_; }

    /// @returns true if the set contains no entries.
    bool IsEmpty() const { return count_ == 0; }

    /// @returns a monotonic counter which is incremented whenever the set is mutated.
    size_t Generation() const { return generation_; }

    /// @returns an iterator to the start of the set.
    Iterator begin() const { return Iterator{slots_.begin(), slots_.end()}; }

    /// @returns an iterator to the end of the set.
    Iterator end() const { return Iterator{slots_.end(), slots_.end()}; }

    /// A debug function for checking that the set is in good health.
    /// Asserts if the set is corrupted.
    void ValidateIntegrity() const {
        size_t num_alive = 0;
        for (size_t slot_idx = 0; slot_idx < slots_.Length(); slot_idx++) {
            const auto& slot = slots_[slot_idx];
            if (slot.value.has_value()) {
                num_alive++;
                auto const [index, hash] = Hash(slot.value.value());
                TINT_ASSERT(Utils, hash == slot.hash);
                TINT_ASSERT(Utils, slot_idx == Wrap(index + slot.distance));
            }
        }
        TINT_ASSERT(Utils, num_alive == count_);
    }

  private:
    /// The behaviour of Put() when an entry already exists with the given key.
    enum class PutMode {
        /// Do not replace existing entries with the new value.
        kAdd,
        /// Replace existing entries with the new value.
        kReplace,
    };
    /// The common implementation for Add() and Replace()
    /// @param value the value to add to the set.
    /// @returns A AddResult describing the result of the insertion
    template <PutMode MODE, typename V>
    AddResult Put(V&& value) {
        // Ensure the set can fit a new entry
        if (ShouldRehash(count_ + 1)) {
            Reserve((count_ + 1) * 2);
        }

        const auto hash = Hash(value);

        AddResult result{};
        Scan(hash.scan_start, [&](size_t distance, size_t index) {
            auto& slot = slots_[index];
            if (!slot.value.has_value()) {
                // Found an empty slot.
                // Place value directly into the slot, and we're done.
                slot.value.emplace(std::forward<V>(value));
                slot.hash = hash.value;
                slot.distance = distance;
                count_++;
                generation_++;
                result = AddResult{AddAction::kAdded, &slot.value.value()};
                return Action::kStop;
            }

            // Slot has an entry

            if (slot.Equals(hash.value, value)) {
                // Slot is equal to value. Replace or preserve?
                if constexpr (MODE == PutMode::kReplace) {
                    slot.value = std::forward<V>(value);
                    generation_++;
                    result = AddResult{AddAction::kReplaced, &slot.value.value()};
                } else {
                    result = AddResult{AddAction::kKeptExisting, &slot.value.value()};
                }
                return Action::kStop;
            }

            if (slot.distance < distance) {
                // Existing slot has a closer distance than the value we're attempting to insert.
                // Steal from the rich!
                // Move the current slot to a temporary (evicted), and put the value into the slot.
                Slot evicted{std::forward<V>(value), hash.value, distance};
                std::swap(evicted, slot);

                // Find a new home for the evicted slot.
                evicted.distance++;  // We've already swapped at index.
                InsertShuffle(Wrap(index + 1), std::move(evicted));

                count_++;
                generation_++;
                result = AddResult{AddAction::kAdded, &slot.value.value()};

                return Action::kStop;
            }
            return Action::kContinue;
        });

        return result;
    }

    /// Return type of the Scan() callback.
    enum class Action {
        /// Continue scanning for a slot
        kContinue,
        /// Immediately stop scanning for a slot
        kStop,
    };

    /// Sequentially visits each of the slots starting with the slot with the index `start`, calling
    /// the callback function `f` for each slot until `f` returns Action::kStop.
    /// `f` must be a function with the signature `Action(size_t distance, size_t index)`.
    /// `f` must return Action::kStop within one whole cycle of the slots.
    template <typename F>
    void Scan(size_t start, F&& f) const {
        size_t index = start;
        for (size_t distance = 0; distance < slots_.Length(); distance++) {
            if (f(distance, index) == Action::kStop) {
                return;
            }
            index = Wrap(index + 1);
        }
        tint::diag::List diags;
        TINT_ICE(Utils, diags) << "Hashset::Scan() looped entire set without finding a slot";
    }

    /// HashResult is the return value of Hash()
    struct HashResult {
        /// The target (zero-distance) slot index for the value.
        size_t scan_start;
        /// The calculated hash of the value.
        size_t value;
    };

    /// @returns a tuple holding the target slot index for the given value, and the hash of the
    ///          value, respectively.
    template <typename V>
    HashResult Hash(const V& value) const {
        size_t hash = HASH()(value);
        size_t index = Wrap(hash);
        return {index, hash};
    }

    /// Looks for the value in the set.
    /// @returns a tuple holding a boolean representing whether the value was found in the set, and
    ///          if found, the index of the slot that holds the value.
    template <typename V>
    std::tuple<bool, size_t> IndexOf(const V& value) const {
        const auto hash = Hash(value);

        bool found = false;
        size_t idx = 0;

        Scan(hash.scan_start, [&](size_t distance, size_t index) {
            auto& slot = slots_[index];
            if (!slot.value.has_value()) {
                return Action::kStop;
            }
            if (slot.Equals(hash.value, value)) {
                found = true;
                idx = index;
                return Action::kStop;
            }
            if (slot.distance < distance) {
                // If the slot distance is less than the current probe distance, then the slot must
                // be for entry that has an index that comes after value. In this situation, we know
                // that the set does not contain the value, as it would have been found before this
                // slot. The "Lookup" section of https://programming.guide/robin-hood-hashing.html
                // suggests that the condition should inverted, but this is wrong.
                return Action::kStop;
            }
            return Action::kContinue;
        });

        return {found, idx};
    }

    /// Shuffles slots for an insertion that has been placed one slot before `start`.
    /// @param evicted the slot content that was evicted for the insertion.
    void InsertShuffle(size_t start, Slot evicted) {
        Scan(start, [&](size_t, size_t index) {
            auto& slot = slots_[index];

            if (!slot.value.has_value()) {
                // Empty slot found for evicted.
                slot = std::move(evicted);
                return Action::kStop;  //  We're done.
            }

            if (slot.distance < evicted.distance) {
                // Occupied slot has shorter distance to evicted.
                // Swap slot and evicted.
                std::swap(slot, evicted);
            }

            // evicted moves further from the target slot...
            evicted.distance++;

            return Action::kContinue;
        });
    }

    /// @returns true if the set should grow the slot vector, and rehash the items.
    bool ShouldRehash(size_t count) const { return NumSlots(count) > slots_.Length(); }

    /// Wrap returns the index value modulo the number of slots.
    size_t Wrap(size_t index) const { return index % slots_.Length(); }

    /// The vector of slots. The vector length is equal to its capacity.
    Vector<Slot, kNumFixedSlots> slots_;

    /// The number of entries in the set.
    size_t count_ = 0;

    /// Counter that's incremented with each modification to the set.
    size_t generation_ = 0;
};

}  // namespace tint::utils

#endif  // SRC_TINT_UTILS_HASHSET_H_
