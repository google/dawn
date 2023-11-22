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

#ifndef SRC_TINT_UTILS_BYTES_READER_H_
#define SRC_TINT_UTILS_BYTES_READER_H_

#include <algorithm>
#include <cstdint>
#include <string>

#include "src/tint/utils/bytes/endianness.h"
#include "src/tint/utils/bytes/swap.h"
#include "src/tint/utils/containers/slice.h"
#include "src/tint/utils/reflection/reflection.h"

namespace tint::bytes {

/// A binary stream reader.
struct Reader {
    /// @returns true if there are no more bytes remaining
    bool IsEOF() const { return offset >= bytes.len; }

    /// @returns the number of bytes remaining in the stream
    size_t BytesRemaining() const { return IsEOF() ? 0 : bytes.len - offset; }

    /// Reads an integer from the stream, performing byte swapping if the stream's endianness
    /// differs from the native endianness. If there are too few bytes remaining in the stream, then
    /// the missing data will be substituted with zeros.
    /// @return the deserialized integer
    template <typename T>
    T Int() {
        static_assert(std::is_integral_v<T>);
        T out = 0;
        if (!IsEOF()) {
            size_t n = std::min(sizeof(T), BytesRemaining());
            memcpy(&out, &bytes[offset], n);
            offset += n;
            if (NativeEndianness() != endianness) {
                out = Swap(out);
            }
        }
        return out;
    }

    /// Reads a float from the stream. If there are too few bytes remaining in the stream, then
    /// the missing data will be substituted with zeros.
    /// @return the deserialized floating point number
    template <typename T>
    T Float() {
        static_assert(std::is_floating_point_v<T>);
        T out = 0;
        if (!IsEOF()) {
            size_t n = std::min(sizeof(T), BytesRemaining());
            memcpy(&out, &bytes[offset], n);
            offset += n;
        }
        return out;
    }

    /// Reads a boolean from the stream
    /// @returns true if the next byte is non-zero
    bool Bool() {
        if (IsEOF()) {
            return false;
        }
        return bytes[offset++] != std::byte{0};
    }

    /// Reads a string of @p len bytes from the stream. If there are too few bytes remaining in the
    /// stream, then the returned string will be truncated.
    /// @param len the length of the returned string in bytes
    /// @return the deserialized string
    std::string String(size_t len) {
        if (IsEOF()) {
            return "";
        }
        size_t n = std::min(len, BytesRemaining());
        std::string out(reinterpret_cast<const char*>(&bytes[offset]), n);
        offset += n;
        return out;
    }

    /// The data to read from
    Slice<const std::byte> bytes;

    /// The current byte offset
    size_t offset = 0;

    /// The endianness of integers serialized in the stream
    Endianness endianness = Endianness::kLittle;
};

/// Reads the templated type from the reader and assigns it to @p out
/// @note This function does not
template <typename T>
Reader& operator>>(Reader& reader, T& out) {
    constexpr bool is_numeric = std::is_integral_v<T> || std::is_floating_point_v<T>;
    static_assert(is_numeric);

    if constexpr (std::is_integral_v<T>) {
        out = reader.Int<T>();
        return reader;
    }

    if constexpr (std::is_floating_point_v<T>) {
        out = reader.Float<T>();
        return reader;
    }

    // Unreachable
    return reader;
}

}  // namespace tint::bytes

#endif  // SRC_TINT_UTILS_BYTES_READER_H_
