// Copyright 2021 The Dawn & Tint Authors
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

#ifndef SRC_TINT_LANG_SPIRV_WRITER_AST_PRINTER_SCALAR_CONSTANT_H_
#define SRC_TINT_LANG_SPIRV_WRITER_AST_PRINTER_SCALAR_CONSTANT_H_

#include <stdint.h>

#include <cstring>
#include <functional>

#include "src/tint/lang/core/number.h"
#include "src/tint/utils/math/hash.h"

// Forward declarations
namespace tint::sem {
class Call;
}  // namespace tint::sem

namespace tint::spirv::writer {

/// ScalarConstant represents a scalar constant value
struct ScalarConstant {
    /// The struct type to hold the bits representation of f16 in the Value union
    struct F16 {
        /// The 16 bits representation of the f16, stored as uint16_t
        uint16_t bits_representation;
    };

    /// The constant value
    union Value {
        /// The value as a bool
        bool b;
        /// The value as a uint32_t
        uint32_t u32;
        /// The value as a int32_t
        int32_t i32;
        /// The value as a float
        float f32;
        /// The value as bits representation of a f16
        F16 f16;

        /// The value that is wide enough to encompass all other types (including
        /// future 64-bit data types).
        uint64_t u64;
    };

    /// The kind of constant
    enum class Kind { kBool, kU32, kI32, kF32, kF16 };

    /// Constructor
    inline ScalarConstant() { value.u64 = 0; }

    /// @param value the value of the constant
    /// @returns a new ScalarConstant with the provided value and kind Kind::kU32
    static inline ScalarConstant U32(uint32_t value) {
        ScalarConstant c;
        c.value.u32 = value;
        c.kind = Kind::kU32;
        return c;
    }

    /// @param value the value of the constant
    /// @returns a new ScalarConstant with the provided value and kind Kind::kI32
    static inline ScalarConstant I32(int32_t value) {
        ScalarConstant c;
        c.value.i32 = value;
        c.kind = Kind::kI32;
        return c;
    }

    /// @param value the value of the constant
    /// @returns a new ScalarConstant with the provided value and kind Kind::kF32
    static inline ScalarConstant F32(float value) {
        ScalarConstant c;
        c.value.f32 = value;
        c.kind = Kind::kF32;
        return c;
    }

    /// @param value the value of the constant
    /// @returns a new ScalarConstant with the provided value and kind Kind::kF16
    static inline ScalarConstant F16(core::f16::type value) {
        ScalarConstant c;
        c.value.f16 = {core::f16(value).BitsRepresentation()};
        c.kind = Kind::kF16;
        return c;
    }

    /// @param value the value of the constant
    /// @returns a new ScalarConstant with the provided value and kind Kind::kBool
    static inline ScalarConstant Bool(bool value) {
        ScalarConstant c;
        c.value.b = value;
        c.kind = Kind::kBool;
        return c;
    }

    /// Equality operator
    /// @param rhs the ScalarConstant to compare against
    /// @returns true if this ScalarConstant is equal to `rhs`
    inline bool operator==(const ScalarConstant& rhs) const {
        return value.u64 == rhs.value.u64 && kind == rhs.kind;
    }

    /// Inequality operator
    /// @param rhs the ScalarConstant to compare against
    /// @returns true if this ScalarConstant is not equal to `rhs`
    inline bool operator!=(const ScalarConstant& rhs) const { return !(*this == rhs); }

    /// The constant value
    Value value;
    /// The constant value kind
    Kind kind = Kind::kBool;
};

}  // namespace tint::spirv::writer

namespace std {

/// Custom std::hash specialization for tint::Symbol so symbols can be used as
/// keys for std::unordered_map and std::unordered_set.
template <>
class hash<tint::spirv::writer::ScalarConstant> {
  public:
    /// @param c the ScalarConstant
    /// @return the Symbol internal value
    inline std::size_t operator()(const tint::spirv::writer::ScalarConstant& c) const {
        uint32_t value = 0;
        std::memcpy(&value, &c.value, sizeof(value));
        return tint::Hash(value, c.kind);
    }
};

}  // namespace std

#endif  // SRC_TINT_LANG_SPIRV_WRITER_AST_PRINTER_SCALAR_CONSTANT_H_
