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

#ifndef SRC_TINT_UTILS_ENUM_SET_H_
#define SRC_TINT_UTILS_ENUM_SET_H_

#include <cstdint>
#include <functional>
#include <type_traits>
#include <utility>

#include "src/tint/utils/string_stream.h"

namespace tint::utils {

/// EnumSet is a set of enum values.
/// @note As the EnumSet is backed by a single uint64_t value, it can only hold
/// enum values in the range [0 .. 63].
template <typename ENUM>
struct EnumSet {
  public:
    /// Enum is the enum type this EnumSet wraps
    using Enum = ENUM;

    /// Constructor. Initializes the EnumSet with zero.
    constexpr EnumSet() = default;

    /// Copy constructor.
    /// @param s the set to copy
    constexpr EnumSet(const EnumSet& s) = default;

    /// Constructor. Initializes the EnumSet with the given values.
    /// @param values the enumerator values to construct the set with
    template <typename... VALUES>
    explicit constexpr EnumSet(VALUES... values) : set(Union(values...)) {}

    /// Copy assignment operator.
    /// @param set the set to assign to this set
    /// @return this set so calls can be chained
    inline EnumSet& operator=(const EnumSet& set) = default;

    /// Copy assignment operator.
    /// @param e the enum value
    /// @return this set so calls can be chained
    inline EnumSet& operator=(Enum e) { return *this = EnumSet{e}; }

    /// Adds all the given values to this set
    /// @param values the values to add
    /// @return this set so calls can be chained
    template <typename... VALUES>
    inline EnumSet& Add(VALUES... values) {
        return Add(EnumSet(std::forward<VALUES>(values)...));
    }

    /// Removes all the given values from this set
    /// @param values the values to remove
    /// @return this set so calls can be chained
    template <typename... VALUES>
    inline EnumSet& Remove(VALUES... values) {
        return Remove(EnumSet(std::forward<VALUES>(values)...));
    }

    /// Adds all of s to this set
    /// @param s the enum value
    /// @return this set so calls can be chained
    inline EnumSet& Add(EnumSet s) { return (*this = *this + s); }

    /// Removes all of s from this set
    /// @param s the enum value
    /// @return this set so calls can be chained
    inline EnumSet& Remove(EnumSet s) { return (*this = *this - s); }

    /// @param e the enum value
    /// @returns a copy of this set with e added
    inline EnumSet operator+(Enum e) const {
        EnumSet out;
        out.set = set | Bit(e);
        return out;
    }

    /// @param e the enum value
    /// @returns a copy of this set with e removed
    inline EnumSet operator-(Enum e) const {
        EnumSet out;
        out.set = set & ~Bit(e);
        return out;
    }

    /// @param s the other set
    /// @returns the union of this set with s (this ∪ rhs)
    inline EnumSet operator+(EnumSet s) const {
        EnumSet out;
        out.set = set | s.set;
        return out;
    }

    /// @param s the other set
    /// @returns the set of entries found in this but not in s (this \ s)
    inline EnumSet operator-(EnumSet s) const {
        EnumSet out;
        out.set = set & ~s.set;
        return out;
    }

    /// @param s the other set
    /// @returns the intersection of this set with s (this ∩ rhs)
    inline EnumSet operator&(EnumSet s) const {
        EnumSet out;
        out.set = set & s.set;
        return out;
    }

    /// @param e the enum value
    /// @return true if the set contains `e`
    inline bool Contains(Enum e) const { return (set & Bit(e)) != 0; }

    /// @return true if the set is empty
    inline bool Empty() const { return set == 0; }

    /// Equality operator
    /// @param rhs the other EnumSet to compare this to
    /// @return true if this EnumSet is equal to rhs
    inline bool operator==(EnumSet rhs) const { return set == rhs.set; }

    /// Inequality operator
    /// @param rhs the other EnumSet to compare this to
    /// @return true if this EnumSet is not equal to rhs
    inline bool operator!=(EnumSet rhs) const { return set != rhs.set; }

    /// Equality operator
    /// @param rhs the enum to compare this to
    /// @return true if this EnumSet only contains `rhs`
    inline bool operator==(Enum rhs) const { return set == Bit(rhs); }

    /// Inequality operator
    /// @param rhs the enum to compare this to
    /// @return false if this EnumSet only contains `rhs`
    inline bool operator!=(Enum rhs) const { return set != Bit(rhs); }

    /// @return the underlying value for the EnumSet
    inline uint64_t Value() const { return set; }

    /// Iterator provides read-only, unidirectional iterator over the enums of an
    /// EnumSet.
    class Iterator {
        static constexpr int8_t kEnd = 63;

        Iterator(uint64_t s, int8_t b) : set(s), pos(b) {}

        /// Make the constructor accessible to the EnumSet.
        friend struct EnumSet;

      public:
        /// @return the Enum value at this point in the iterator
        Enum operator*() const { return static_cast<Enum>(pos); }

        /// Increments the iterator
        /// @returns this iterator
        Iterator& operator++() {
            while (pos < kEnd) {
                pos++;
                if (set & (static_cast<uint64_t>(1) << static_cast<uint64_t>(pos))) {
                    break;
                }
            }
            return *this;
        }

        /// Equality operator
        /// @param rhs the Iterator to compare this to
        /// @return true if the two iterators are equal
        bool operator==(const Iterator& rhs) const { return set == rhs.set && pos == rhs.pos; }

        /// Inequality operator
        /// @param rhs the Iterator to compare this to
        /// @return true if the two iterators are different
        bool operator!=(const Iterator& rhs) const { return !(*this == rhs); }

      private:
        const uint64_t set;
        int8_t pos;
    };

    /// @returns an read-only iterator to the beginning of the set
    Iterator begin() const {
        auto it = Iterator{set, -1};
        ++it;  // Move to first set bit
        return it;
    }

    /// @returns an iterator to the beginning of the set
    Iterator end() const { return Iterator{set, Iterator::kEnd}; }

  private:
    static constexpr uint64_t Bit(Enum value) {
        return static_cast<uint64_t>(1) << static_cast<uint64_t>(value);
    }

    static constexpr uint64_t Union() { return 0; }

    template <typename FIRST, typename... VALUES>
    static constexpr uint64_t Union(FIRST first, VALUES... values) {
        return Bit(first) | Union(values...);
    }

    uint64_t set = 0;
};

/// Writes the EnumSet to the stream.
/// @param out the stream to write to
/// @param set the EnumSet to write
/// @returns out so calls can be chained
template <typename ENUM>
inline utils::StringStream& operator<<(utils::StringStream& out, EnumSet<ENUM> set) {
    out << "{";
    bool first = true;
    for (auto e : set) {
        if (!first) {
            out << ", ";
        }
        first = false;
        out << e;
    }
    return out << "}";
}

}  // namespace tint::utils

namespace std {

/// Custom std::hash specialization for tint::utils::EnumSet<T>
template <typename T>
class hash<tint::utils::EnumSet<T>> {
  public:
    /// @param e the EnumSet to create a hash for
    /// @return the hash value
    inline std::size_t operator()(const tint::utils::EnumSet<T>& e) const {
        return std::hash<uint64_t>()(e.Value());
    }
};

}  // namespace std

#endif  // SRC_TINT_UTILS_ENUM_SET_H_
