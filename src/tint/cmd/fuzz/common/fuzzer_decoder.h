// Copyright 2026 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef SRC_TINT_CMD_FUZZ_COMMON_FUZZER_DECODER_H_
#define SRC_TINT_CMD_FUZZ_COMMON_FUZZER_DECODER_H_

#include <algorithm>
#include <bitset>
#include <cmath>
#include <cstddef>
#include <cstring>
#include <optional>
#include <span>
#include <string>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "src/tint/cmd/fuzz/common/fuzzer_limits.h"
#include "src/tint/utils/memory/copy.h"
#include "src/tint/utils/reflection/reflection.h"
#include "src/utils/compiler.h"

namespace tint::fuzz {

/// SafeFuzzerDecoder parses an entropy byte buffer in sequential order, generating structured data
/// from it, and is guaranteed to never fail. If it runs out of input bytes, it gracefully returns
/// default/zero values.
///
/// @note Inspiration and Differences from FuzzedDataProvider:
/// This class is directly inspired and influenced by LLVM's `FuzzedDataProvider` (FDP), which is
/// a standard tool for converting raw, unstructured fuzzer entropy into structured types.
/// Salient features of`SafeFuzzerDecoder` design and how it deviates from FDP:
///
/// 1. **Sequential Consumption:**
///    `FuzzedDataProvider` generates results from raw data in two distinct ways: by consuming
///    dynamic structures (strings, vectors) from the front, and fixed-size integers/booleans from
///    the back. This avoids many situations where a small changes in the format or input causes
///    large impact or randomization on the result, but requires careful structuring of calling code
///    to perform all of one type of generation and then all of the other in sequence.
///    To avoid the complexity re-ordering member reads in the decoder or enforcing a strict struct
///    layout on transform options, a strict sequential decoder is implemented.
///    This means that if a field is removed/added from a struct or an input bit is flipped it will
///    only impact the interpretation of the following elements in the struct. This makes the
///    decoder more flexible in the inputs order, but at the cost of theoretically being more prone
///    to degeneration of the genetic algorithm. This is with it making the decoder more easy to
///    reason about and thus more obvious when a change impacts other parts of the result.
///    A 'perfect' solution here would involve changes to one part of the structured data never
///    affecting any of the other parts of the data, but that requires effectively having separate
///    sources of entropy for each member of the struct.
///
/// 2. **Deep Integration with `TINT_REFLECT`:**
///    Rather than requiring manual range checking or length constraints at every single call-site
///    (as FDP does via `ConsumeIntegralInRange` or `ConsumeBytes`), `SafeFuzzerDecoder` is wrapped
///    inside a reflection based dispatcher (`FuzzDecoder`). Complex nested types (like optionals,
///    vector lists, and hash maps) automatically derive safe maximum capacities and clamp
///    automatically, ensuring no out-of-memory (OOM) or massive allocation panics can occur.
///
/// 3. **Graceful Fallback:**
///    Like FDP, `SafeFuzzerDecoder` never fails. If the buffer is depleted or empty, it returns `0`
///    for numerical types, `false` for booleans, and terminates dynamic collections cleanly. This
///    allows for test cases with no sidecar data to run using safe defaults instead of being
///    outright rejected. It specifically also never returns a 'partial' read, if there is not
///    enough data to satisfy the internal tracking is updated to consume the rest of the data, but
///    the zero/default value is returned.
class SafeFuzzerDecoder {
  public:
    /// Constructor
    /// @param data the fuzzer's sidecar data bytes to consume
    explicit SafeFuzzerDecoder(std::span<const std::byte> data) : data_(data) {}

    /// @returns true if the decoder has no more bytes to read.
    bool IsEOF() const { return data_.empty(); }

    /// Safely consumes up to `sizeof(T)` bytes left-to-right to construct an integer.
    /// @returns the decoded integer, or 0 if no more bytes are available.
    template <typename T>
    T ConsumeIntegral() {
        static_assert(std::is_integral_v<T>);
        if (data_.size() < sizeof(T)) {
            data_ = data_.subspan(data_.size());  // Consume whatever is left to prevent reuse
            return 0;
        }
        T result = 0;
        tint::Copy(&result, 1, data_.template subspan<0, sizeof(T)>());
        data_ = data_.subspan(sizeof(T));
        return result;
    }

    /// Safely consumes up to `sizeof(T)` bytes left-to-right to construct a float.
    /// @returns the decoded float, or 0.0 if no more bytes are available.
    template <typename T>
    T ConsumeFloat() {
        static_assert(std::is_floating_point_v<T>);
        if (data_.size() < sizeof(T)) {
            data_ = data_.subspan(data_.size());  // Consume whatever is left to prevent reuse
            return 0;
        }
        T result = 0;
        tint::Copy(&result, 1, data_.template subspan<0, sizeof(T)>());
        data_ = data_.subspan(sizeof(T));
        // Normalize NaNs and infinities if necessary to avoid non-determinism, or just return.
        if (std::isnan(result) || std::isinf(result)) {
            result = 0;
        }
        return result;
    }

    /// Safely consumes 1 byte to determine a boolean value.
    /// @returns true if the byte is odd, otherwise false.
    bool ConsumeBool() { return (ConsumeIntegral<uint8_t>() % 2) == 1; }

    /// Safely consumes 1 byte to determine a collection's length.
    /// @param max_len the maximum permitted collection length.
    /// @returns the collection length bounded to [0, max_len] inclusive.
    size_t ConsumeCollectionLength(size_t max_len = 16) {
        if (data_.empty()) {
            return 0;
        }
        return static_cast<size_t>(ConsumeIntegral<uint8_t>()) % (max_len + 1);
    }

  private:
    std::span<const std::byte> data_;
};

/// FuzzDecoder decodes type `T` from `SafeFuzzerDecoder`.
template <typename T, typename = void>
struct FuzzDecoder;

/// Decoder specialization for integral types.
template <typename T>
    requires(std::is_integral_v<T>)
struct FuzzDecoder<T> {
    static T Decode(SafeFuzzerDecoder& decoder) { return decoder.ConsumeIntegral<T>(); }
};

template <typename T, typename = void>
struct HasEnumRange : std::false_type {};

template <typename T>
struct HasEnumRange<T, std::void_t<decltype(tint::EnumRange<T>::kMin)>> : std::true_type {};

/// Decoder specialization for enum types.
template <typename T>
    requires(std::is_enum_v<T>)
struct FuzzDecoder<T> {
    static T Decode(SafeFuzzerDecoder& decoder) {
        using UnderT = std::underlying_type_t<T>;
        auto val = decoder.ConsumeIntegral<UnderT>();
        if constexpr (HasEnumRange<T>::value) {
            UnderT min = static_cast<UnderT>(tint::EnumRange<T>::kMin);
            UnderT max = static_cast<UnderT>(tint::EnumRange<T>::kMax);
            if (min <= max) {
                uint64_t uval = static_cast<uint64_t>(val);
                uint64_t umin = static_cast<uint64_t>(min);
                uint64_t umax = static_cast<uint64_t>(max);
                uint64_t range = umax - umin + 1;
                val = static_cast<UnderT>(umin + (uval % range));
            }
        }
        return static_cast<T>(val);
    }
};

/// Decoder specialization for floating point types.
template <typename T>
    requires(std::is_floating_point_v<T>)
struct FuzzDecoder<T> {
    static T Decode(SafeFuzzerDecoder& decoder) { return decoder.ConsumeFloat<T>(); }
};

/// Decoder specialization for boolean.
template <>
struct FuzzDecoder<bool, void> {
    static bool Decode(SafeFuzzerDecoder& decoder) { return decoder.ConsumeBool(); }
};

/// Decoder specialization for std::bitset.
template <size_t N>
struct FuzzDecoder<std::bitset<N>> {
    static std::bitset<N> Decode(SafeFuzzerDecoder& decoder) {
        std::bitset<N> bits;
        for (size_t i = 0; i < N; ++i) {
            bits[i] = decoder.ConsumeBool();
        }
        return bits;
    }
};

/// Decoder specialization for std::string.
template <>
struct FuzzDecoder<std::string, void> {
    static std::string Decode(SafeFuzzerDecoder& decoder) {
        const size_t len = decoder.ConsumeCollectionLength(kMaxStringLength);
        std::string s;
        s.reserve(len);
        for (size_t i = 0; i < len; ++i) {
            if (const char c = decoder.ConsumeIntegral<char>(); c != '\0') {
                s.push_back(c);
            }
        }
        return s;
    }
};

/// Forward declarations for structural templates.
template <typename T>
    requires(HasReflection<T>)
struct FuzzDecoder<T> {
    static T Decode(SafeFuzzerDecoder& decoder) {
        T object{};
        ForeachField(object, [&]<typename F>(F& field) {
            using FieldT = std::decay_t<F>;
            field = FuzzDecoder<FieldT>::Decode(decoder);
        });
        return object;
    }
};

/// Decoder specialization for std::optional.
template <typename T>
struct FuzzDecoder<std::optional<T>> {
    static std::optional<T> Decode(SafeFuzzerDecoder& decoder) {
        if (decoder.ConsumeBool()) {
            return FuzzDecoder<T>::Decode(decoder);
        }
        return std::nullopt;
    }
};

/// Decoder specialization for std::vector.
template <typename T>
struct FuzzDecoder<std::vector<T>> {
    static std::vector<T> Decode(SafeFuzzerDecoder& decoder) {
        size_t len = decoder.ConsumeCollectionLength(kMaxCollectionLength);
        std::vector<T> vec;
        vec.reserve(len);
        for (size_t i = 0; i < len; ++i) {
            vec.push_back(FuzzDecoder<T>::Decode(decoder));
        }
        return vec;
    }
};

/// Decoder specialization for std::unordered_set.
template <typename T>
struct FuzzDecoder<std::unordered_set<T>> {
    static std::unordered_set<T> Decode(SafeFuzzerDecoder& decoder) {
        size_t len = decoder.ConsumeCollectionLength(kMaxCollectionLength);
        std::unordered_set<T> set;
        for (size_t i = 0; i < len; ++i) {
            set.insert(FuzzDecoder<T>::Decode(decoder));
        }
        return set;
    }
};

/// Decoder specialization for std::unordered_map.
template <typename K, typename V>
struct FuzzDecoder<std::unordered_map<K, V>> {
    static std::unordered_map<K, V> Decode(SafeFuzzerDecoder& decoder) {
        const size_t len = decoder.ConsumeCollectionLength(kMaxCollectionLength);
        std::unordered_map<K, V> map;
        for (size_t i = 0; i < len; ++i) {
            auto key = FuzzDecoder<K>::Decode(decoder);
            auto val = FuzzDecoder<V>::Decode(decoder);
            map[key] = val;
        }
        return map;
    }
};

/// Helper templates for handling std::tuple recursively.
template <typename Tuple, size_t I = 0>
void DecodeTuple(SafeFuzzerDecoder& decoder, Tuple& t) {
    if constexpr (I < std::tuple_size_v<Tuple>) {
        using ElementT = std::tuple_element_t<I, Tuple>;
        std::get<I>(t) = FuzzDecoder<ElementT>::Decode(decoder);
        DecodeTuple<Tuple, I + 1>(decoder, t);
    }
}

/// Specialization of FuzzDecoder for std::tuple.
template <typename... Args>
struct FuzzDecoder<std::tuple<Args...>> {
    static std::tuple<Args...> Decode(SafeFuzzerDecoder& decoder) {
        std::tuple<Args...> t{};
        DecodeTuple(decoder, t);
        return t;
    }
};

}  // namespace tint::fuzz

#endif  // SRC_TINT_CMD_FUZZ_COMMON_FUZZER_DECODER_H_
