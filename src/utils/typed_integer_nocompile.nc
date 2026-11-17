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

// Test the type checks of TypedInteger in debug and release.
// Strong checks are enabled only in debug.

#include "src/utils/typed_integer.h"
#include "src/utils/underlying_type.h"

namespace dawn {

using MyI32 = TypedInteger<struct MyI32T, int32_t>;
using MyU32 = TypedInteger<struct MyU32T, uint32_t>;

using MyOtherI32 = TypedInteger<struct MyOtherI32T, int32_t>;

// Helper for getting a *runtime* value of the given type.
template <typename T>
T value() {
    volatile UnderlyingType<T> val = 3;
    return T{val};
}

// Construction of TypedInteger from other things.
void TestConstruction() {
    // Cannot construct from a distinct but otherwise identical type.
    std::ignore = MyI32(value<MyI32>());       // (control case)
    std::ignore = MyI32(value<MyOtherI32>());  // expected-error {{no matching conversion for functional-style cast from}}
    std::ignore = MyI32{value<MyOtherI32>()};  // expected-error {{no matching constructor for initialization of}}

    // Cannot convert to narrower types.
    std::ignore = MyI32(value<int32_t>());   // (control case)
    std::ignore = MyI32(value<int64_t>());   // expected-error {{implicit conversion loses integer precision}}
    std::ignore = MyI32(value<uint32_t>());  // expected-error {{implicit conversion changes signedness}}
    std::ignore = MyU32(value<uint64_t>());  // expected-error {{implicit conversion loses integer precision}}
    std::ignore = MyU32(value<int16_t>());   // expected-error {{implicit conversion changes signedness}}
    std::ignore = MyU32(value<int32_t>());   // expected-error {{implicit conversion changes signedness}}
    // Braces are stricter than parentheses (-Wc++11-narrowing).
    std::ignore = MyI32{value<int32_t>()};   // (control case)
    std::ignore = MyI32{value<int64_t>()};   // expected-error {{non-constant-expression cannot be narrowed}} expected-error {{implicit conversion loses integer precision}}
    std::ignore = MyI32{value<uint32_t>()};  // expected-error {{non-constant-expression cannot be narrowed}} expected-error {{implicit conversion changes signedness}}

    // In constexpr, only the actual value matters, not the type.
    std::ignore = MyI32(0x7fff'ffffU);
    std::ignore = MyI32(uint32_t{0x7fff'ffff});
    std::ignore = MyI32(0x8000'0000U);           // expected-error {{implicit conversion changes signedness}}
    std::ignore = MyI32(uint32_t{0x8000'0000});  // expected-error {{implicit conversion changes signedness}}
    std::ignore = MyU32(0);
    std::ignore = MyU32(-1);                     // expected-error {{implicit conversion changes signedness}}
    std::ignore = MyU32(int32_t{0});
    std::ignore = MyU32(int32_t{-1});            // expected-error {{implicit conversion changes signedness}}
    std::ignore = MyU32(uint64_t{0});
    std::ignore = MyU32(0x1'0000'0000ULL);       // expected-error {{changes value}}
    // Braces are stricter than parentheses (-Wc++11-narrowing).
    std::ignore = MyI32{0x7fff'ffffU};
    std::ignore = MyI32{0x8000'0000U};           // expected-error {{constant expression evaluates to}} expected-error {{implicit conversion changes signedness}}
    std::ignore = MyU32{0x1'0000'0000ULL};       // expected-error {{constant expression evaluates to}} expected-error {{changes value from}}
}

// Assignment to TypedInteger from other things.
void TestAssignmentToTyped() {
    // Construction by assignment does NOT allow type conversions.
    { [[maybe_unused]] MyI32 x = value<MyI32>(); }       // (control case.)
    { [[maybe_unused]] MyI32 x = value<int32_t>(); }     // expected-error {{no viable conversion from}}
    { [[maybe_unused]] MyI32 x = value<MyOtherI32>(); }  // expected-error {{no viable conversion from}}

    // Same with assignment operator.
    { [[maybe_unused]] MyI32 x; x = value<MyI32>(); }       // (control case.)
    { [[maybe_unused]] MyI32 x; x = value<int32_t>(); }     // expected-error {{no viable overloaded '='}}
    { [[maybe_unused]] MyI32 x; x = value<MyOtherI32>(); }  // expected-error {{no viable overloaded '='}}
}

// Casts from TypedInteger to primitive.
void TestCastToPrimitive() {
    // Cannot convert to narrower types.
    std::ignore = int32_t{value<MyI32>()};   // (control case.)
    std::ignore = int16_t{value<MyI32>()};   // expected-error {{no viable conversion from}}
    std::ignore = uint32_t{value<MyI32>()};  // expected-error {{no viable conversion from}}
    std::ignore = int32_t{value<MyU32>()};   // expected-error {{no viable conversion from}}
}

// Assignment to primitive from TypedInteger.
void TestAssignmentToPrimitive() {
    { [[maybe_unused]] int32_t x = value<int32_t>(); }  // (control case.)
    { [[maybe_unused]] int32_t x = value<MyI32>(); }    // expected-error {{no viable conversion from}}
}

}
