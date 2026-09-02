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

#include "src/tint/lang/glsl/writer/raise/bitcast_polyfill.h"

#include <string>

#include "gtest/gtest.h"
#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/ir/transform/helper_test.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::glsl::writer::raise {
namespace {

class GlslWriter_BitcastPolyfillTest : public core::ir::transform::TransformTest {
  protected:
    void SetUp() override {
        core::ir::transform::TransformTest::SetUp();
        mod.properties.Add(core::ir::Property::kAllow16BitFloats,
                           core::ir::Property::kAllow16BitIntegers);
    }
};

TEST_F(GlslWriter_BitcastPolyfillTest, FloatToFloat) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Let("a", 1_f);
        b.Let("x", b.Bitcast<f32>(a));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %a:f32 = let 1.0f
    %3:f32 = bitcast<f32> %a
    %x:f32 = let %3
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %a:f32 = let 1.0f
    %x:f32 = let %a
    ret
  }
}
)";

    Run(BitcastPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BitcastPolyfillTest, IntToFloat) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Let("a", 1_i);
        b.Let("x", b.Bitcast<f32>(a));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %a:i32 = let 1i
    %3:f32 = bitcast<f32> %a
    %x:f32 = let %3
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %a:i32 = let 1i
    %3:f32 = glsl.intBitsToFloat %a
    %x:f32 = let %3
    ret
  }
}
)";

    Run(BitcastPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BitcastPolyfillTest, UintToFloat) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Let("a", 1_u);
        b.Let("x", b.Bitcast<f32>(a));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %a:u32 = let 1u
    %3:f32 = bitcast<f32> %a
    %x:f32 = let %3
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %a:u32 = let 1u
    %3:f32 = glsl.uintBitsToFloat %a
    %x:f32 = let %3
    ret
  }
}
)";

    Run(BitcastPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BitcastPolyfillTest, Vec3UintToVec3Float) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Let("a", b.Splat<vec3<u32>>(1_u));
        b.Let("x", b.Bitcast<vec3<f32>>(a));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %a:vec3<u32> = let vec3<u32>(1u)
    %3:vec3<f32> = bitcast<vec3<f32>> %a
    %x:vec3<f32> = let %3
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %a:vec3<u32> = let vec3<u32>(1u)
    %3:vec3<f32> = glsl.uintBitsToFloat %a
    %x:vec3<f32> = let %3
    ret
  }
}
)";

    Run(BitcastPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BitcastPolyfillTest, FloatToInt) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Let("a", 1_f);
        b.Let("x", b.Bitcast<i32>(a));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %a:f32 = let 1.0f
    %3:i32 = bitcast<i32> %a
    %x:i32 = let %3
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %a:f32 = let 1.0f
    %3:i32 = glsl.floatBitsToInt %a
    %x:i32 = let %3
    ret
  }
}
)";

    Run(BitcastPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BitcastPolyfillTest, FloatToUint) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Let("a", 1_f);
        b.Let("x", b.Bitcast<u32>(a));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %a:f32 = let 1.0f
    %3:u32 = bitcast<u32> %a
    %x:u32 = let %3
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %a:f32 = let 1.0f
    %3:u32 = glsl.floatBitsToUint %a
    %x:u32 = let %3
    ret
  }
}
)";

    Run(BitcastPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BitcastPolyfillTest, UintToInt) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Let("a", 1_u);
        b.Let("x", b.Bitcast<i32>(a));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %a:u32 = let 1u
    %3:i32 = bitcast<i32> %a
    %x:i32 = let %3
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %a:u32 = let 1u
    %3:i32 = convert %a
    %x:i32 = let %3
    ret
  }
}
)";

    Run(BitcastPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BitcastPolyfillTest, IntToUint) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Let("a", 1_i);
        b.Let("x", b.Bitcast<u32>(a));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %a:i32 = let 1i
    %3:u32 = bitcast<u32> %a
    %x:u32 = let %3
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %a:i32 = let 1i
    %3:u32 = convert %a
    %x:u32 = let %3
    ret
  }
}
)";

    Run(BitcastPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BitcastPolyfillTest, I32ToVec2F16) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Let("a", 1_i);
        b.Let("x", b.Bitcast<vec2<f16>>(a));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %a:i32 = let 1i
    %3:vec2<f16> = bitcast<vec2<f16>> %a
    %x:vec2<f16> = let %3
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %a:i32 = let 1i
    %3:vec2<f16> = call %tint_bitcast_to_16bit, %a
    %x:vec2<f16> = let %3
    ret
  }
}
%tint_bitcast_to_16bit = func(%src:i32):vec2<f16> {
  $B2: {
    %7:u32 = convert %src
    %8:vec2<f16> = glsl.unpackFloat2x16 %7
    ret %8
  }
}
)";

    Run(BitcastPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BitcastPolyfillTest, Vec2F16ToI32) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Let("a", b.Construct<vec2<f16>>(1_h, 2_h));
        b.Let("x", b.Bitcast<i32>(a));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %a:vec2<f16> = let vec2<f16>(1.0h, 2.0h)
    %3:i32 = bitcast<i32> %a
    %x:i32 = let %3
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %a:vec2<f16> = let vec2<f16>(1.0h, 2.0h)
    %3:i32 = call %tint_bitcast_from_16bit, %a
    %x:i32 = let %3
    ret
  }
}
%tint_bitcast_from_16bit = func(%src:vec2<f16>):i32 {
  $B2: {
    %7:u32 = glsl.packFloat2x16 %src
    %8:i32 = convert %7
    ret %8
  }
}
)";

    Run(BitcastPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BitcastPolyfillTest, U32ToVec2F16) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Let("a", 1_u);
        b.Let("x", b.Bitcast<vec2<f16>>(a));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %a:u32 = let 1u
    %3:vec2<f16> = bitcast<vec2<f16>> %a
    %x:vec2<f16> = let %3
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %a:u32 = let 1u
    %3:vec2<f16> = call %tint_bitcast_to_16bit, %a
    %x:vec2<f16> = let %3
    ret
  }
}
%tint_bitcast_to_16bit = func(%src:u32):vec2<f16> {
  $B2: {
    %7:vec2<f16> = glsl.unpackFloat2x16 %src
    ret %7
  }
}
)";

    Run(BitcastPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BitcastPolyfillTest, Vec2F16ToU32) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Let("a", b.Construct<vec2<f16>>(1_h, 2_h));
        b.Let("x", b.Bitcast<u32>(a));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %a:vec2<f16> = let vec2<f16>(1.0h, 2.0h)
    %3:u32 = bitcast<u32> %a
    %x:u32 = let %3
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %a:vec2<f16> = let vec2<f16>(1.0h, 2.0h)
    %3:u32 = call %tint_bitcast_from_16bit, %a
    %x:u32 = let %3
    ret
  }
}
%tint_bitcast_from_16bit = func(%src:vec2<f16>):u32 {
  $B2: {
    %7:u32 = glsl.packFloat2x16 %src
    ret %7
  }
}
)";

    Run(BitcastPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BitcastPolyfillTest, F32ToVec2F16) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Let("a", 1_f);
        b.Let("x", b.Bitcast<vec2<f16>>(a));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %a:f32 = let 1.0f
    %3:vec2<f16> = bitcast<vec2<f16>> %a
    %x:vec2<f16> = let %3
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %a:f32 = let 1.0f
    %3:vec2<f16> = call %tint_bitcast_to_16bit, %a
    %x:vec2<f16> = let %3
    ret
  }
}
%tint_bitcast_to_16bit = func(%src:f32):vec2<f16> {
  $B2: {
    %7:u32 = glsl.floatBitsToUint %src
    %8:vec2<f16> = glsl.unpackFloat2x16 %7
    ret %8
  }
}
)";

    Run(BitcastPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BitcastPolyfillTest, Vec2F16ToF32) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Let("a", b.Construct<vec2<f16>>(1_h, 2_h));
        b.Let("x", b.Bitcast<f32>(a));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %a:vec2<f16> = let vec2<f16>(1.0h, 2.0h)
    %3:f32 = bitcast<f32> %a
    %x:f32 = let %3
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %a:vec2<f16> = let vec2<f16>(1.0h, 2.0h)
    %3:f32 = call %tint_bitcast_from_16bit, %a
    %x:f32 = let %3
    ret
  }
}
%tint_bitcast_from_16bit = func(%src:vec2<f16>):f32 {
  $B2: {
    %7:u32 = glsl.packFloat2x16 %src
    %8:f32 = glsl.uintBitsToFloat %7
    ret %8
  }
}
)";

    Run(BitcastPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BitcastPolyfillTest, Vec2I32ToVec4F16) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Let("a", b.Construct<vec2<i32>>(1_i, 2_i));
        b.Let("x", b.Bitcast<vec4<f16>>(a));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %a:vec2<i32> = let vec2<i32>(1i, 2i)
    %3:vec4<f16> = bitcast<vec4<f16>> %a
    %x:vec4<f16> = let %3
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %a:vec2<i32> = let vec2<i32>(1i, 2i)
    %3:vec4<f16> = call %tint_bitcast_to_16bit, %a
    %x:vec4<f16> = let %3
    ret
  }
}
%tint_bitcast_to_16bit = func(%src:vec2<i32>):vec4<f16> {
  $B2: {
    %7:vec2<u32> = convert %src
    %8:u32 = swizzle %7, x
    %9:vec2<f16> = glsl.unpackFloat2x16 %8
    %10:u32 = swizzle %7, y
    %11:vec2<f16> = glsl.unpackFloat2x16 %10
    %12:vec4<f16> = construct %9, %11
    ret %12
  }
}
)";

    Run(BitcastPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BitcastPolyfillTest, Vec4F16ToVec2I32) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Let("a", b.Construct<vec4<f16>>(1_h, 2_h, 3_h, 4_h));
        b.Let("x", b.Bitcast<vec2<i32>>(a));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %a:vec4<f16> = let vec4<f16>(1.0h, 2.0h, 3.0h, 4.0h)
    %3:vec2<i32> = bitcast<vec2<i32>> %a
    %x:vec2<i32> = let %3
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %a:vec4<f16> = let vec4<f16>(1.0h, 2.0h, 3.0h, 4.0h)
    %3:vec2<i32> = call %tint_bitcast_from_16bit, %a
    %x:vec2<i32> = let %3
    ret
  }
}
%tint_bitcast_from_16bit = func(%src:vec4<f16>):vec2<i32> {
  $B2: {
    %7:vec2<f16> = swizzle %src, xy
    %8:u32 = glsl.packFloat2x16 %7
    %9:vec2<f16> = swizzle %src, zw
    %10:u32 = glsl.packFloat2x16 %9
    %11:vec2<u32> = construct %8, %10
    %12:vec2<i32> = convert %11
    ret %12
  }
}
)";

    Run(BitcastPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BitcastPolyfillTest, Vec2U32ToVec4F16) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Let("a", b.Construct<vec2<u32>>(1_u, 2_u));
        b.Let("x", b.Bitcast<vec4<f16>>(a));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %a:vec2<u32> = let vec2<u32>(1u, 2u)
    %3:vec4<f16> = bitcast<vec4<f16>> %a
    %x:vec4<f16> = let %3
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %a:vec2<u32> = let vec2<u32>(1u, 2u)
    %3:vec4<f16> = call %tint_bitcast_to_16bit, %a
    %x:vec4<f16> = let %3
    ret
  }
}
%tint_bitcast_to_16bit = func(%src:vec2<u32>):vec4<f16> {
  $B2: {
    %7:u32 = swizzle %src, x
    %8:vec2<f16> = glsl.unpackFloat2x16 %7
    %9:u32 = swizzle %src, y
    %10:vec2<f16> = glsl.unpackFloat2x16 %9
    %11:vec4<f16> = construct %8, %10
    ret %11
  }
}
)";

    Run(BitcastPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BitcastPolyfillTest, Vec4F16ToVec2U32) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Let("a", b.Construct<vec4<f16>>(1_h, 2_h, 3_h, 4_h));
        b.Let("x", b.Bitcast<vec2<u32>>(a));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %a:vec4<f16> = let vec4<f16>(1.0h, 2.0h, 3.0h, 4.0h)
    %3:vec2<u32> = bitcast<vec2<u32>> %a
    %x:vec2<u32> = let %3
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %a:vec4<f16> = let vec4<f16>(1.0h, 2.0h, 3.0h, 4.0h)
    %3:vec2<u32> = call %tint_bitcast_from_16bit, %a
    %x:vec2<u32> = let %3
    ret
  }
}
%tint_bitcast_from_16bit = func(%src:vec4<f16>):vec2<u32> {
  $B2: {
    %7:vec2<f16> = swizzle %src, xy
    %8:u32 = glsl.packFloat2x16 %7
    %9:vec2<f16> = swizzle %src, zw
    %10:u32 = glsl.packFloat2x16 %9
    %11:vec2<u32> = construct %8, %10
    ret %11
  }
}
)";

    Run(BitcastPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BitcastPolyfillTest, Vec2F32ToVec4F16) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Let("a", b.Construct<vec2<f32>>(1_f, 2_f));
        b.Let("x", b.Bitcast<vec4<f16>>(a));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %a:vec2<f32> = let vec2<f32>(1.0f, 2.0f)
    %3:vec4<f16> = bitcast<vec4<f16>> %a
    %x:vec4<f16> = let %3
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %a:vec2<f32> = let vec2<f32>(1.0f, 2.0f)
    %3:vec4<f16> = call %tint_bitcast_to_16bit, %a
    %x:vec4<f16> = let %3
    ret
  }
}
%tint_bitcast_to_16bit = func(%src:vec2<f32>):vec4<f16> {
  $B2: {
    %7:vec2<u32> = glsl.floatBitsToUint %src
    %8:u32 = swizzle %7, x
    %9:vec2<f16> = glsl.unpackFloat2x16 %8
    %10:u32 = swizzle %7, y
    %11:vec2<f16> = glsl.unpackFloat2x16 %10
    %12:vec4<f16> = construct %9, %11
    ret %12
  }
}
)";

    Run(BitcastPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BitcastPolyfillTest, Vec4F16ToVec2F32) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Let("a", b.Construct<vec4<f16>>(1_h, 2_h, 3_h, 4_h));
        b.Let("x", b.Bitcast<vec2<f32>>(a));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %a:vec4<f16> = let vec4<f16>(1.0h, 2.0h, 3.0h, 4.0h)
    %3:vec2<f32> = bitcast<vec2<f32>> %a
    %x:vec2<f32> = let %3
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %a:vec4<f16> = let vec4<f16>(1.0h, 2.0h, 3.0h, 4.0h)
    %3:vec2<f32> = call %tint_bitcast_from_16bit, %a
    %x:vec2<f32> = let %3
    ret
  }
}
%tint_bitcast_from_16bit = func(%src:vec4<f16>):vec2<f32> {
  $B2: {
    %7:vec2<f16> = swizzle %src, xy
    %8:u32 = glsl.packFloat2x16 %7
    %9:vec2<f16> = swizzle %src, zw
    %10:u32 = glsl.packFloat2x16 %9
    %11:vec2<u32> = construct %8, %10
    %12:vec2<f32> = glsl.uintBitsToFloat %11
    ret %12
  }
}
)";

    Run(BitcastPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BitcastPolyfillTest, U16_To_F16) {
    auto* a = b.FunctionParam("a", ty.u16());
    auto* b_param = b.FunctionParam("b", ty.vec2<u16>());
    auto* c = b.FunctionParam("c", ty.vec3<u16>());
    auto* d = b.FunctionParam("d", ty.vec4<u16>());
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({a, b_param, c, d});
    b.Append(func->Block(), [&] {
        b.Bitcast<f16>(a);
        b.Bitcast<vec2<f16>>(b_param);
        b.Bitcast<vec3<f16>>(c);
        b.Bitcast<vec4<f16>>(d);
        b.Return(func);
    });

    auto* src = R"(
%foo = func(%a:u16, %b:vec2<u16>, %c:vec3<u16>, %d:vec4<u16>):void {
  $B1: {
    %6:f16 = bitcast<f16> %a
    %7:vec2<f16> = bitcast<vec2<f16>> %b
    %8:vec3<f16> = bitcast<vec3<f16>> %c
    %9:vec4<f16> = bitcast<vec4<f16>> %d
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%a:u16, %b:vec2<u16>, %c:vec3<u16>, %d:vec4<u16>):void {
  $B1: {
    %6:f16 = glsl.uint16BitsToFloat16 %a
    %7:vec2<f16> = glsl.uint16BitsToFloat16 %b
    %8:vec3<f16> = glsl.uint16BitsToFloat16 %c
    %9:vec4<f16> = glsl.uint16BitsToFloat16 %d
    ret
  }
}
)";

    Run(BitcastPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BitcastPolyfillTest, F16_To_U16) {
    auto* a = b.FunctionParam("a", ty.f16());
    auto* b_param = b.FunctionParam("b", ty.vec2<f16>());
    auto* c = b.FunctionParam("c", ty.vec3<f16>());
    auto* d = b.FunctionParam("d", ty.vec4<f16>());
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({a, b_param, c, d});
    b.Append(func->Block(), [&] {
        b.Bitcast<u16>(a);
        b.Bitcast<vec2<u16>>(b_param);
        b.Bitcast<vec3<u16>>(c);
        b.Bitcast<vec4<u16>>(d);
        b.Return(func);
    });

    auto* src = R"(
%foo = func(%a:f16, %b:vec2<f16>, %c:vec3<f16>, %d:vec4<f16>):void {
  $B1: {
    %6:u16 = bitcast<u16> %a
    %7:vec2<u16> = bitcast<vec2<u16>> %b
    %8:vec3<u16> = bitcast<vec3<u16>> %c
    %9:vec4<u16> = bitcast<vec4<u16>> %d
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%a:f16, %b:vec2<f16>, %c:vec3<f16>, %d:vec4<f16>):void {
  $B1: {
    %6:u16 = glsl.float16BitsToUint16 %a
    %7:vec2<u16> = glsl.float16BitsToUint16 %b
    %8:vec3<u16> = glsl.float16BitsToUint16 %c
    %9:vec4<u16> = glsl.float16BitsToUint16 %d
    ret
  }
}
)";

    Run(BitcastPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BitcastPolyfillTest, U16_To_U32) {
    auto* a = b.FunctionParam("a", ty.vec2<u16>());
    auto* b_param = b.FunctionParam("b", ty.vec4<u16>());
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({a, b_param});
    b.Append(func->Block(), [&] {
        b.Bitcast<u32>(a);
        b.Bitcast<vec2<u32>>(b_param);
        b.Return(func);
    });

    auto* src = R"(
%foo = func(%a:vec2<u16>, %b:vec4<u16>):void {
  $B1: {
    %4:u32 = bitcast<u32> %a
    %5:vec2<u32> = bitcast<vec2<u32>> %b
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%a:vec2<u16>, %b:vec4<u16>):void {
  $B1: {
    %4:u32 = call %tint_bitcast_from_16bit, %a
    %6:vec2<u32> = call %tint_bitcast_from_16bit_1, %b
    ret
  }
}
%tint_bitcast_from_16bit = func(%src:vec2<u16>):u32 {
  $B2: {
    %9:vec2<f16> = glsl.uint16BitsToFloat16 %src
    %10:u32 = glsl.packFloat2x16 %9
    ret %10
  }
}
%tint_bitcast_from_16bit_1 = func(%src_1:vec4<u16>):vec2<u32> {  # %src_1: 'src'
  $B3: {
    %12:vec4<f16> = glsl.uint16BitsToFloat16 %src_1
    %13:vec2<f16> = swizzle %12, xy
    %14:u32 = glsl.packFloat2x16 %13
    %15:vec2<f16> = swizzle %12, zw
    %16:u32 = glsl.packFloat2x16 %15
    %17:vec2<u32> = construct %14, %16
    ret %17
  }
}
)";

    Run(BitcastPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BitcastPolyfillTest, U32_To_U16) {
    auto* a = b.FunctionParam("a", ty.u32());
    auto* b_param = b.FunctionParam("b", ty.vec2<u32>());
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({a, b_param});
    b.Append(func->Block(), [&] {
        b.Bitcast<vec2<u16>>(a);
        b.Bitcast<vec4<u16>>(b_param);
        b.Return(func);
    });

    auto* src = R"(
%foo = func(%a:u32, %b:vec2<u32>):void {
  $B1: {
    %4:vec2<u16> = bitcast<vec2<u16>> %a
    %5:vec4<u16> = bitcast<vec4<u16>> %b
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%a:u32, %b:vec2<u32>):void {
  $B1: {
    %4:vec2<u16> = call %tint_bitcast_to_16bit, %a
    %6:vec4<u16> = call %tint_bitcast_to_16bit_1, %b
    ret
  }
}
%tint_bitcast_to_16bit = func(%src:u32):vec2<u16> {
  $B2: {
    %9:vec2<f16> = glsl.unpackFloat2x16 %src
    %10:vec2<u16> = glsl.float16BitsToUint16 %9
    ret %10
  }
}
%tint_bitcast_to_16bit_1 = func(%src_1:vec2<u32>):vec4<u16> {  # %src_1: 'src'
  $B3: {
    %12:u32 = swizzle %src_1, x
    %13:vec2<f16> = glsl.unpackFloat2x16 %12
    %14:u32 = swizzle %src_1, y
    %15:vec2<f16> = glsl.unpackFloat2x16 %14
    %16:vec4<f16> = construct %13, %15
    %17:vec4<u16> = glsl.float16BitsToUint16 %16
    ret %17
  }
}
)";

    Run(BitcastPolyfill);
    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::glsl::writer::raise
