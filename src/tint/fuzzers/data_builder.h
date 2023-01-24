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

#ifndef SRC_TINT_FUZZERS_DATA_BUILDER_H_
#define SRC_TINT_FUZZERS_DATA_BUILDER_H_

#include <cassert>
#include <functional>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "src/tint/fuzzers/random_generator.h"

namespace tint::fuzzers {

/// Builder for generic pseudo-random data
class DataBuilder {
  public:
    /// @brief Initializes the internal engine using a seed value
    /// @param seed - seed value passed to engine
    explicit DataBuilder(uint64_t seed) : generator_(seed) {}

    /// @brief Initializes the internal engine using seed data
    /// @param data - data fuzzer to calculate seed from
    /// @param size - size of data buffer
    explicit DataBuilder(const uint8_t* data, size_t size)
        : generator_(RandomGenerator::CalculateSeed(data, size)) {
        assert(data != nullptr && "|data| must be !nullptr");
    }

    /// Destructor
    ~DataBuilder() = default;

    /// Move Constructor
    DataBuilder(DataBuilder&&) = default;

    /// Generate pseudo-random data of a specific type
    /// @tparam T - type of data to produce
    /// @returns pseudo-random data of type T
    template <typename T>
    T build() {
        return BuildImpl<T>::impl(this);
    }

    /// Generate pseudo-random data of a specific type in a vector
    /// @tparam T - data type held vector
    /// @returns pseudo-random data of type std::vector<T>
    template <typename T>
    std::vector<T> vector() {
        auto count = build<uint8_t>();
        std::vector<T> out(count);
        for (uint8_t i = 0; i < count; i++) {
            out[i] = build<T>();
        }
        return out;
    }

    /// Generate complex pseudo-random data of a specific type in a vector
    /// @tparam T - data type held vector
    /// @tparam Callback - callback that takes in a DataBuilder* and returns a T
    /// @param generate - callback for generating each instance of T
    /// @returns pseudo-random data of type std::vector<T>
    template <typename T, typename Callback>
    std::vector<T> vector(Callback generate) {
        auto count = build<uint8_t>();
        std::vector<T> out(count);
        for (size_t i = 0; i < count; i++) {
            out[i] = generate(this);
        }
        return out;
    }

    /// Generate an pseudo-random entry to a enum class.
    /// Assumes enum is tightly packed starting at 0.
    /// @tparam T - type of enum class
    /// @param count - number of entries in enum class
    /// @returns a random enum class entry
    template <typename T>
    T enum_class(uint32_t count) {
        return static_cast<T>(generator_.Get4Bytes() % count);
    }

  private:
    RandomGenerator generator_;

    // Disallow copy & assign
    DataBuilder(const DataBuilder&) = delete;
    DataBuilder& operator=(const DataBuilder&) = delete;

    /// Get N bytes of pseudo-random data
    /// @param out - pointer to location to save data
    /// @param n - number of bytes to get
    void build(void* out, size_t n) {
        assert(out != nullptr && "|out| cannot be nullptr");
        assert(n > 0 && "|n| must be > 0");

        generator_.GetNBytes(reinterpret_cast<uint8_t*>(out), n);
    }

    /// Generate pseudo-random data of a specific type into an output var
    /// @tparam T - type of data to produce
    /// @param out - output var to generate into
    template <typename T>
    void build(T& out) {
        out = build<T>();
    }

    /// Implementation of ::build<T>()
    /// @tparam T - type of data to produce
    template <typename T>
    struct BuildImpl {
        /// Generate a pseudo-random variable of type T
        /// @param b - data builder to use
        /// @returns a variable of type T filled with pseudo-random data
        static T impl(DataBuilder* b) {
            T out{};
            if constexpr (tint::HasReflection<T>) {
                ForeachField(out, [&](auto& field) { b->build(field); });
            } else if constexpr (std::is_pod_v<T>) {
                b->build(&out, sizeof(T));
            } else {
                static_assert(sizeof(T) == 0, "cannot build type");
            }
            return out;
        }
    };

    /// Specialization for bool
    template <>
    struct BuildImpl<bool> {
        /// Generate a pseudo-random bool
        /// @param b - data builder to use
        /// @returns a boolean with even odds of being true or false
        static bool impl(DataBuilder* b) { return b->generator_.GetBool(); }
    };

    /// Specialization for std::string
    template <>
    struct BuildImpl<std::string> {
        /// Generate a pseudo-random string
        /// @param b - data builder to use
        /// @returns a string filled with pseudo-random data
        static std::string impl(DataBuilder* b) {
            auto count = b->build<uint8_t>();
            if (count == 0) {
                return "";
            }
            std::vector<uint8_t> source(count);
            b->build(source.data(), count);
            return {source.begin(), source.end()};
        }
    };

    /// Specialization for std::optional
    template <typename T>
    struct BuildImpl<std::optional<T>> {
        /// Generate a pseudo-random optional<T>
        /// @param b - data builder to use
        /// @returns a either a nullopt, or a randomly filled T
        static std::optional<T> impl(DataBuilder* b) {
            if (b->build<bool>()) {
                return b->build<T>();
            }
            return std::nullopt;
        }
    };

    /// Specialization for std::unordered_map<K, V>
    template <typename K, typename V>
    struct BuildImpl<std::unordered_map<K, V>> {
        /// Generate a pseudo-random std::unordered_map<K, V>
        /// @param b - data builder to use
        /// @returns std::unordered_map<K, V> filled with
        /// pseudo-random data
        static std::unordered_map<K, V> impl(DataBuilder* b) {
            std::unordered_map<K, V> out;
            uint8_t count = b->build<uint8_t>();
            for (uint8_t i = 0; i < count; ++i) {
                out.emplace(b->build<K>(), b->build<V>());
            }
            return out;
        }
    };
};

}  // namespace tint::fuzzers

#endif  // SRC_TINT_FUZZERS_DATA_BUILDER_H_
