// Copyright 2023 The Dawn & Tint Authors
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

#ifndef SRC_TINT_UTILS_BYTES_DECODER_H_
#define SRC_TINT_UTILS_BYTES_DECODER_H_

#include <string>
#include <tuple>
#include <unordered_map>
#include <utility>

#include "src/tint/utils/bytes/reader.h"
#include "src/tint/utils/result/result.h"

namespace tint::bytes {

template <typename T, typename = void>
struct Decoder;

/// Decodes T from @p reader.
/// @param reader the byte reader
/// @returns the decoded object
template <typename T>
Result<T> Decode(Reader& reader) {
    return Decoder<T>::Decode(reader);
}

/// Decoder specialization for integer types
template <typename T>
struct Decoder<T, std::enable_if_t<std::is_integral_v<T>>> {
    /// Decode decodes the integer type from @p reader.
    /// @param reader the reader to decode from
    /// @returns the decoded integer type, or an error if the stream is too short.
    static Result<T> Decode(Reader& reader) {
        if (reader.BytesRemaining() < sizeof(T)) {
            return Failure{"EOF"};
        }
        return reader.Int<T>();
    }
};

/// Decoder specialization for floating point types
template <typename T>
struct Decoder<T, std::enable_if_t<std::is_floating_point_v<T>>> {
    /// Decode decodes the floating point type from @p reader.
    /// @param reader the reader to decode from
    /// @returns the decoded floating point type, or an error if the stream is too short.
    static Result<T> Decode(Reader& reader) {
        if (reader.BytesRemaining() < sizeof(T)) {
            return Failure{"EOF"};
        }
        return reader.Float<T>();
    }
};

/// Decoder specialization for a uint16_t length prefixed string.
template <typename T>
struct Decoder<T, std::enable_if_t<std::is_same_v<T, std::string>>> {
    /// Decode decodes the string from @p reader.
    /// @param reader the reader to decode from
    /// @returns the decoded string, or an error if the stream is too short.
    static Result<T> Decode(Reader& reader) {
        if (reader.BytesRemaining() < sizeof(uint16_t)) {
            return Failure{"EOF"};
        }
        auto len = reader.Int<uint16_t>();
        if (reader.BytesRemaining() < len) {
            return Failure{"EOF"};
        }
        return reader.String(len);
    }
};

/// Decoder specialization for bool types
template <>
struct Decoder<bool, void> {
    static Result<bool> Decode(Reader& reader) {
        /// Decode decodes the boolean from @p reader.
        /// @param reader the reader to decode from
        /// @returns the decoded boolean, or an error if the stream is too short.
        if (reader.IsEOF()) {
            return Failure{"EOF"};
        }
        return reader.Bool();
    }
};

/// Decoder specialization for types that use TINT_REFLECT
template <typename T>
struct Decoder<T, std::enable_if_t<HasReflection<T>>> {
    /// Decode decodes the reflected type from @p reader.
    /// @param reader the reader to decode from
    /// @returns the decoded reflected type, or an error if the stream is too short.
    static Result<T> Decode(Reader& reader) {
        T object{};
        diag::List errs;
        ForeachField(object, [&](auto& field) {  //
            auto value = bytes::Decode<std::decay_t<decltype(field)>>(reader);
            if (value) {
                field = value.Get();
            } else {
                errs.add(value.Failure().reason);
            }
        });
        if (errs.empty()) {
            return object;
        }
        return Failure{errs};
    }
};

/// Decoder specialization for std::unordered_map
template <typename K, typename V>
struct Decoder<std::unordered_map<K, V>, void> {
    /// Decode decodes the map from @p reader.
    /// @param reader the reader to decode from
    /// @returns the decoded map, or an error if the stream is too short.
    static Result<std::unordered_map<K, V>> Decode(Reader& reader) {
        std::unordered_map<K, V> out;

        while (true) {
            if (reader.IsEOF()) {
                return Failure{"EOF"};
            }
            if (reader.Bool()) {
                break;
            }
            auto key = bytes::Decode<K>(reader);
            if (!key) {
                return key.Failure();
            }
            auto val = bytes::Decode<V>(reader);
            if (!val) {
                return val.Failure();
            }
            out.emplace(std::move(key.Get()), std::move(val.Get()));
        }

        return out;
    }
};

/// Decoder specialization for std::tuple
template <typename FIRST, typename... OTHERS>
struct Decoder<std::tuple<FIRST, OTHERS...>, void> {
    /// Decode decodes the tuple from @p reader.
    /// @param reader the reader to decode from
    /// @returns the decoded tuple, or an error if the stream is too short.
    static Result<std::tuple<FIRST, OTHERS...>> Decode(Reader& reader) {
        auto first = bytes::Decode<FIRST>(reader);
        if (!first) {
            return first.Failure();
        }
        if constexpr (sizeof...(OTHERS) > 0) {
            auto others = bytes::Decode<std::tuple<OTHERS...>>(reader);
            if (!others) {
                return others.Failure();
            }
            return std::tuple_cat(std::tuple<FIRST>(first.Get()), others.Get());
        } else {
            return std::tuple<FIRST>(first.Get());
        }
    }
};

}  // namespace tint::bytes

#endif  // SRC_TINT_UTILS_BYTES_DECODER_H_
