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

#ifndef FUZZERS_DATA_BUILDER_H_
#define FUZZERS_DATA_BUILDER_H_

#include <cassert>
#include <functional>
#include <string>
#include <vector>

#include "fuzzers/random_generator.h"

namespace tint {
namespace fuzzers {

/// Builder for generic pseudo-random data using a data buffer as seed
class DataBuilder {
 public:
  /// @brief Initialize random number generations
  /// @param data - pointer to a data buffer to use as a seed
  /// @param size - size of data buffer
  explicit DataBuilder(const uint8_t* data, size_t size)
      : generator_(data, size) {}

  ~DataBuilder() {}

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

  /// Get N bytes of pseudo-random data
  /// @param out - pointer to location to save data
  /// @param n - number of bytes to get
  void build(void* out, size_t n) {
    assert(out != nullptr && "|out| cannot be nullptr");
    assert(n > 0 && "|n| must be > 0");

    generator_.GetNBytes(reinterpret_cast<uint8_t*>(out), n);
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
      b->build(&out, sizeof(T));
      return out;
    }
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
      return std::string(source.begin(), source.end());
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
};

}  // namespace fuzzers
}  // namespace tint

#endif  // FUZZERS_DATA_BUILDER_H_
