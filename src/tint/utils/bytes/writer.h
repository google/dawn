// Copyright 2024 The Dawn & Tint Authors
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

#ifndef SRC_TINT_UTILS_BYTES_WRITER_H_
#define SRC_TINT_UTILS_BYTES_WRITER_H_

#include <algorithm>
#include <cstdint>
#include <string>

#include "src/tint/utils/bytes/endianness.h"
#include "src/tint/utils/bytes/swap.h"
#include "src/tint/utils/containers/slice.h"
#include "src/tint/utils/result/result.h"

namespace tint::bytes {

/// A binary stream writer interface
class Writer {
  public:
    /// Destructor
    virtual ~Writer();

    /// Write writes bytes to the stream, blocking until the write has finished.
    /// @param in the byte data to write to the stream
    /// @param count the number of bytes to write. Must be greater than 0
    /// @returns the result of the write
    virtual Result<SuccessType> Write(const std::byte* in, size_t count) = 0;

    /// Writes an integer to the stream, performing byte swapping if the stream's endianness
    /// differs from the native endianness.
    /// @param value the integer value to write
    /// @param endianness the endianness of the integer in the stream
    /// @returns the result of the write
    template <typename T>
    Result<SuccessType> Int(T value, Endianness endianness = Endianness::kLittle) {
        static_assert(std::is_integral_v<T>);
        if (NativeEndianness() != endianness) {
            value = Swap(value);
        }
        return Write(reinterpret_cast<const std::byte*>(&value), sizeof(T));
    }

    /// Writes a float to the stream.
    /// @param value the float value to write
    /// @returns the result of the write
    template <typename T>
    Result<SuccessType> Float(T value) {
        static_assert(std::is_floating_point_v<T>);
        return Write(reinterpret_cast<const std::byte*>(&value), sizeof(T));
    }

    /// Writes a boolean to the stream
    /// @param value the boolean value to write
    /// @returns the result of the write
    Result<SuccessType> Bool(bool value) {
        auto b = value ? std::byte{1} : std::byte{0};
        return Write(&b, 1);
    }

    /// Writes a string of @p len bytes to the stream.
    /// @param value the string to write
    /// @returns the result of the write
    Result<SuccessType> String(std::string_view value) {
        static_assert(sizeof(std::byte) == sizeof(char), "length needs calculation");
        return Write(reinterpret_cast<const std::byte*>(value.data()), value.length());
    }
};

}  // namespace tint::bytes

#endif  // SRC_TINT_UTILS_BYTES_WRITER_H_
