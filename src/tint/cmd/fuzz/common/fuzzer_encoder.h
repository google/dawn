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

#ifndef SRC_TINT_CMD_FUZZ_COMMON_FUZZER_ENCODER_H_
#define SRC_TINT_CMD_FUZZ_COMMON_FUZZER_ENCODER_H_

#include <algorithm>
#include <bitset>
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
#include "src/tint/utils/reflection/reflection.h"
#include "src/utils/compiler.h"

namespace tint::fuzz {

/// SafeFuzzerEncoder writes out canonical minimal representations of values.
///
/// Note: Due to clamping of values, i.e. the number of elements in a container, when decoding
///       multiple different entropy sets will map to the same state. For encoding the minimal
///       version is chosen to be canonical to make the code easier to reason about. This means that
///       in general textual roundtrip between Encode/Decode is not guaranteed, but semantics will
///       roundtrip. The canonical form should always textually roundtrip.
class SafeFuzzerEncoder {
  public:
    /// Constructor
    /// @param buffer the vector to append bytes to.
    explicit SafeFuzzerEncoder(std::vector<std::byte>& buffer) : buffer_(buffer) {}

    /// Appends the bytes of `val`.
    template <typename T>
    void EncodeIntegral(T val) {
        static_assert(std::is_integral_v<T>);
        size_t offset = buffer_.size();
        buffer_.resize(offset + sizeof(T));
        tint::Copy(std::span{buffer_}.subspan(offset, sizeof(T)), val);
    }

    /// Appends the bytes of `val`.
    template <typename T>
    void EncodeFloat(T val) {
        static_assert(std::is_floating_point_v<T>);
        size_t offset = buffer_.size();
        buffer_.resize(offset + sizeof(T));
        tint::Copy(std::span{buffer_}.subspan(offset, sizeof(T)), val);
    }

    /// Appends `1` for true, `0` for false.
    void EncodeBool(bool val) { EncodeIntegral<uint8_t>(val ? 1 : 0); }

    /// Appends the length byte, clamped to @p max_len.
    /// @param len the original collection length.
    /// @param max_len the maximum allowed length.
    /// @returns the clamped length actually encoded.
    size_t EncodeCollectionLength(size_t len, size_t max_len) {
        size_t clamped = std::min(len, max_len);
        EncodeIntegral<uint8_t>(static_cast<uint8_t>(clamped));
        return clamped;
    }

    /// Appends raw bytes.
    /// @param bytes the span of bytes to append.
    void EncodeRaw(std::span<const std::byte> bytes) {
        buffer_.insert(buffer_.end(), bytes.begin(), bytes.end());
    }

  private:
    std::vector<std::byte>& buffer_;
};

/// FuzzEncoder encodes type `T` into `SafeFuzzerEncoder`.
template <typename T, typename = void>
struct FuzzEncoder;

/// Encoder specialization for integral types.
template <typename T>
    requires(std::is_integral_v<T>)
struct FuzzEncoder<T> {
    static void Encode(SafeFuzzerEncoder& encoder, T val) { encoder.EncodeIntegral<T>(val); }
};

/// Encoder specialization for enum types.
template <typename T>
    requires(std::is_enum_v<T>)
struct FuzzEncoder<T> {
    static void Encode(SafeFuzzerEncoder& encoder, T val) {
        using UnderT = std::underlying_type_t<T>;
        encoder.EncodeIntegral<UnderT>(static_cast<UnderT>(val));
    }
};

/// Encoder specialization for floating point types.
template <typename T>
    requires(std::is_floating_point_v<T>)
struct FuzzEncoder<T> {
    static void Encode(SafeFuzzerEncoder& encoder, T val) { encoder.EncodeFloat<T>(val); }
};

/// Encoder specialization for boolean.
template <>
struct FuzzEncoder<bool, void> {
    static void Encode(SafeFuzzerEncoder& encoder, bool val) { encoder.EncodeBool(val); }
};

/// Encoder specialization for std::bitset.
template <size_t N>
struct FuzzEncoder<std::bitset<N>> {
    static void Encode(SafeFuzzerEncoder& encoder, const std::bitset<N>& val) {
        for (size_t i = 0; i < N; ++i) {
            encoder.EncodeBool(val[i]);
        }
    }
};

/// Encoder specialization for std::string.
template <>
struct FuzzEncoder<std::string, void> {
    static void Encode(SafeFuzzerEncoder& encoder, const std::string& val) {
        size_t len = encoder.EncodeCollectionLength(val.length(), kMaxStringLength);
        for (size_t i = 0; i < len; ++i) {
            encoder.EncodeIntegral<char>(val[i]);
        }
    }
};

/// Forward declarations for structural templates.
template <typename T>
    requires(HasReflection<T>)
struct FuzzEncoder<T> {
    static void Encode(SafeFuzzerEncoder& encoder, const T& val) {
        ForeachField(val, [&](const auto& field) {
            using FieldT = std::decay_t<decltype(field)>;
            FuzzEncoder<FieldT>::Encode(encoder, field);
        });
    }
};

/// Encoder specialization for std::optional.
template <typename T>
struct FuzzEncoder<std::optional<T>> {
    static void Encode(SafeFuzzerEncoder& encoder, const std::optional<T>& val) {
        encoder.EncodeBool(val.has_value());
        if (val.has_value()) {
            FuzzEncoder<T>::Encode(encoder, *val);
        }
    }
};

/// Encoder specialization for std::vector.
template <typename T>
struct FuzzEncoder<std::vector<T>> {
    static void Encode(SafeFuzzerEncoder& encoder, const std::vector<T>& val) {
        size_t len = encoder.EncodeCollectionLength(val.size(), kMaxCollectionLength);
        for (size_t i = 0; i < len; ++i) {
            FuzzEncoder<T>::Encode(encoder, val[i]);
        }
    }
};

/// Encoder specialization for std::unordered_set.
template <typename T>
struct FuzzEncoder<std::unordered_set<T>> {
    static void Encode(SafeFuzzerEncoder& encoder, const std::unordered_set<T>& val) {
        size_t len = encoder.EncodeCollectionLength(val.size(), kMaxCollectionLength);
        // Since all elements may not be written out, need to have a stable ordering for them.
        // Encoding the individual elements and sorting by the encoded bytes.
        std::vector<std::vector<std::byte>> elems;
        elems.reserve(val.size());
        for (const auto& item : val) {
            std::vector<std::byte> item_buffer;
            SafeFuzzerEncoder item_encoder(item_buffer);
            FuzzEncoder<T>::Encode(item_encoder, item);
            elems.push_back(std::move(item_buffer));
        }
        std::sort(elems.begin(), elems.end());

        for (size_t i = 0; i < len; ++i) {
            encoder.EncodeRaw(elems[i]);
        }
    }
};

/// Encoder specialization for std::unordered_map.
template <typename K, typename V>
struct FuzzEncoder<std::unordered_map<K, V>> {
    static void Encode(SafeFuzzerEncoder& encoder, const std::unordered_map<K, V>& val) {
        size_t len = encoder.EncodeCollectionLength(val.size(), kMaxCollectionLength);
        // Since all elements may not be written out, need to have a stable ordering for them.
        // Encoding the keys and values and sorting by the encoded bytes for the keys.
        struct EncodedPair {
            std::vector<std::byte> key;
            std::vector<std::byte> value;
            bool operator<(const EncodedPair& other) const { return key < other.key; }
        };
        std::vector<EncodedPair> encoded_items;
        encoded_items.reserve(val.size());
        for (const auto& [k, v] : val) {
            EncodedPair pair;
            SafeFuzzerEncoder key_encoder(pair.key);
            FuzzEncoder<K>::Encode(key_encoder, k);
            SafeFuzzerEncoder value_encoder(pair.value);
            FuzzEncoder<V>::Encode(value_encoder, v);
            encoded_items.push_back(std::move(pair));
        }
        std::sort(encoded_items.begin(), encoded_items.end());

        for (size_t i = 0; i < len; ++i) {
            encoder.EncodeRaw(encoded_items[i].key);
            encoder.EncodeRaw(encoded_items[i].value);
        }
    }
};

/// Helper templates for handling std::tuple recursively.
template <typename Tuple, size_t I = 0>
void EncodeTuple(SafeFuzzerEncoder& encoder, const Tuple& t) {
    if constexpr (I < std::tuple_size_v<Tuple>) {
        using ElementT = std::tuple_element_t<I, Tuple>;
        FuzzEncoder<ElementT>::Encode(encoder, std::get<I>(t));
        EncodeTuple<Tuple, I + 1>(encoder, t);
    }
}

/// Specialization of FuzzEncoder for std::tuple.
template <typename... Args>
struct FuzzEncoder<std::tuple<Args...>> {
    static void Encode(SafeFuzzerEncoder& encoder, const std::tuple<Args...>& val) {
        EncodeTuple(encoder, val);
    }
};

}  // namespace tint::fuzz

#endif  // SRC_TINT_CMD_FUZZ_COMMON_FUZZER_ENCODER_H_
