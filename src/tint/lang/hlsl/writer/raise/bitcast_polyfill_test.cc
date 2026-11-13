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

#include "src/tint/lang/hlsl/writer/raise/bitcast_polyfill.h"

#include "gtest/gtest.h"
#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/ir/transform/helper_test.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::hlsl::writer::raise {
namespace {

struct HlslWriter_BitcastPolyfillTest : public core::ir::transform::TransformTest {
  protected:
    void SetUp() override { mod.properties.Add(core::ir::Property::kAllow16BitFloats); }
};

TEST_F(HlslWriter_BitcastPolyfillTest, BitcastIdentity) {
    auto* a = b.FunctionParam<i32>("a");
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({a});
    b.Append(func->Block(), [&] { b.Return(func, b.Bitcast<i32>(a)); });

    auto* src = R"(
%foo = func(%a:i32):i32 {
  $B1: {
    %3:i32 = bitcast<i32> %a
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%a:i32):i32 {
  $B1: {
    ret %a
  }
}
)";

    Run(BitcastPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BitcastPolyfillTest, Asuint) {
    auto* a = b.FunctionParam<i32>("a");
    auto* func = b.Function("foo", ty.u32());
    func->SetParams({a});
    b.Append(func->Block(), [&] { b.Return(func, b.Bitcast<u32>(a)); });

    auto* src = R"(
%foo = func(%a:i32):u32 {
  $B1: {
    %3:u32 = bitcast<u32> %a
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%a:i32):u32 {
  $B1: {
    %3:u32 = hlsl.asuint %a
    ret %3
  }
}
)";

    Run(BitcastPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BitcastPolyfillTest, Asint) {
    auto* a = b.FunctionParam<u32>("a");
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({a});
    b.Append(func->Block(), [&] { b.Return(func, b.Bitcast<i32>(a)); });

    auto* src = R"(
%foo = func(%a:u32):i32 {
  $B1: {
    %3:i32 = bitcast<i32> %a
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%a:u32):i32 {
  $B1: {
    %3:i32 = hlsl.asint %a
    ret %3
  }
}
)";

    Run(BitcastPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BitcastPolyfillTest, Asfloat) {
    auto* a = b.FunctionParam<i32>("a");
    auto* func = b.Function("foo", ty.f32());
    func->SetParams({a});
    b.Append(func->Block(), [&] { b.Return(func, b.Bitcast<f32>(a)); });

    auto* src = R"(
%foo = func(%a:i32):f32 {
  $B1: {
    %3:f32 = bitcast<f32> %a
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%a:i32):f32 {
  $B1: {
    %3:f32 = hlsl.asfloat %a
    ret %3
  }
}
)";

    Run(BitcastPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BitcastPolyfillTest, AsfloatVec) {
    auto* a = b.FunctionParam<vec3<i32>>("a");
    auto* func = b.Function("foo", ty.vec<f32, 3>());
    func->SetParams({a});
    b.Append(func->Block(), [&] { b.Return(func, b.Bitcast(ty.vec(ty.f32(), 3), a)); });

    auto* src = R"(
%foo = func(%a:vec3<i32>):vec3<f32> {
  $B1: {
    %3:vec3<f32> = bitcast<vec3<f32>> %a
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%a:vec3<i32>):vec3<f32> {
  $B1: {
    %3:vec3<f32> = hlsl.asfloat %a
    ret %3
  }
}
)";

    Run(BitcastPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BitcastPolyfillTest, BitcastFromF16) {
    auto* a = b.FunctionParam<vec2<f16>>("a");
    auto* func = b.Function("foo", ty.f32());
    func->SetParams({a});
    b.Append(func->Block(), [&] { b.Return(func, b.Bitcast(ty.f32(), a)); });

    auto* src = R"(
%foo = func(%a:vec2<f16>):f32 {
  $B1: {
    %3:f32 = bitcast<f32> %a
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%a:vec2<f16>):f32 {
  $B1: {
    %3:f32 = call %tint_bitcast_from_f16, %a
    ret %3
  }
}
%tint_bitcast_from_f16 = func(%src:vec2<f16>):f32 {
  $B2: {
    %6:vec2<u16> = hlsl.asuint16 %src
    %7:vec2<u32> = convert %6
    %8:vec2<u32> = and %7, vec2<u32>(65535u)
    %9:vec2<u32> = shl %8, vec2<u32>(0u, 16u)
    %10:u32 = access %9, 0u
    %11:u32 = access %9, 1u
    %12:u32 = or %10, %11
    %13:f32 = hlsl.asfloat %12
    ret %13
  }
}
)";

    Run(BitcastPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BitcastPolyfillTest, BitcastToF16) {
    auto* a = b.FunctionParam<f32>("a");
    auto* func = b.Function("foo", ty.vec2h());
    func->SetParams({a});
    b.Append(func->Block(), [&] { b.Return(func, b.Bitcast(ty.vec2h(), a)); });

    auto* src = R"(
%foo = func(%a:f32):vec2<f16> {
  $B1: {
    %3:vec2<f16> = bitcast<vec2<f16>> %a
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%a:f32):vec2<f16> {
  $B1: {
    %3:vec2<f16> = call %tint_bitcast_to_f16, %a
    ret %3
  }
}
%tint_bitcast_to_f16 = func(%src:f32):vec2<f16> {
  $B2: {
    %6:u32 = hlsl.asuint %src
    %v:u32 = let %6
    %8:vec2<u32> = construct %v, %v
    %9:vec2<u32> = shr %8, vec2<u32>(0u, 16u)
    %10:vec2<u32> = and %9, vec2<u32>(65535u)
    %11:vec2<u16> = convert %10
    %v16:vec2<u16> = let %11
    %13:vec2<f16> = hlsl.asfloat16 %v16
    ret %13
  }
}
)";

    Run(BitcastPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BitcastPolyfillTest, BitcastFromVec2F16) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Var("a", b.Construct<vec2<f16>>(1_h, 2_h));
        auto* z = b.Load(a);
        b.Let("b", b.Bitcast<i32>(z));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %a:ptr<function, vec2<f16>, read_write> = var vec2<f16>(1.0h, 2.0h)
    %3:vec2<f16> = load %a
    %4:i32 = bitcast<i32> %3
    %b:i32 = let %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %a:ptr<function, vec2<f16>, read_write> = var vec2<f16>(1.0h, 2.0h)
    %3:vec2<f16> = load %a
    %4:i32 = call %tint_bitcast_from_f16, %3
    %b:i32 = let %4
    ret
  }
}
%tint_bitcast_from_f16 = func(%src:vec2<f16>):i32 {
  $B2: {
    %8:vec2<u16> = hlsl.asuint16 %src
    %9:vec2<u32> = convert %8
    %10:vec2<u32> = and %9, vec2<u32>(65535u)
    %11:vec2<u32> = shl %10, vec2<u32>(0u, 16u)
    %12:u32 = access %11, 0u
    %13:u32 = access %11, 1u
    %14:u32 = or %12, %13
    %15:i32 = hlsl.asint %14
    ret %15
  }
}
)";

    Run(BitcastPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BitcastPolyfillTest, BitcastToVec4F16) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Var("a", b.Construct<vec2<i32>>(1_i, 2_i));
        b.Let("b", b.Bitcast<vec4<f16>>(b.Load(a)));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %a:ptr<function, vec2<i32>, read_write> = var vec2<i32>(1i, 2i)
    %3:vec2<i32> = load %a
    %4:vec4<f16> = bitcast<vec4<f16>> %3
    %b:vec4<f16> = let %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %a:ptr<function, vec2<i32>, read_write> = var vec2<i32>(1i, 2i)
    %3:vec2<i32> = load %a
    %4:vec4<f16> = call %tint_bitcast_to_f16, %3
    %b:vec4<f16> = let %4
    ret
  }
}
%tint_bitcast_to_f16 = func(%src:vec2<i32>):vec4<f16> {
  $B2: {
    %8:vec2<u32> = hlsl.asuint %src
    %v:vec2<u32> = let %8
    %10:vec4<u32> = swizzle %v, xxyy
    %11:vec4<u32> = shr %10, vec4<u32>(0u, 16u, 0u, 16u)
    %12:vec4<u32> = and %11, vec4<u32>(65535u)
    %13:vec4<u16> = convert %12
    %v16:vec4<u16> = let %13
    %15:vec4<f16> = hlsl.asfloat16 %v16
    ret %15
  }
}
)";

    Run(BitcastPolyfill);
    EXPECT_EQ(expect, str());
}

// Test bitcast from f16 to u16 scalar — should use asuint16.
TEST_F(HlslWriter_BitcastPolyfillTest, BitcastF16ToU16) {
    mod.properties.Add(core::ir::Property::kAllow16BitIntegers);
    auto* a = b.FunctionParam("a", ty.f16());
    auto* func = b.Function("foo", ty.u16());
    func->SetParams({a});
    b.Append(func->Block(), [&] { b.Return(func, b.Bitcast(ty.u16(), a)); });

    auto* src = R"(
%foo = func(%a:f16):u16 {
  $B1: {
    %3:u16 = bitcast<u16> %a
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%a:f16):u16 {
  $B1: {
    %3:u16 = hlsl.asuint16 %a
    ret %3
  }
}
)";

    Run(BitcastPolyfill);
    EXPECT_EQ(expect, str());
}

// Test bitcast from u16 to f16 scalar — should use asfloat16.
TEST_F(HlslWriter_BitcastPolyfillTest, BitcastU16ToF16) {
    mod.properties.Add(core::ir::Property::kAllow16BitIntegers);
    auto* a = b.FunctionParam("a", ty.u16());
    auto* func = b.Function("foo", ty.f16());
    func->SetParams({a});
    b.Append(func->Block(), [&] { b.Return(func, b.Bitcast(ty.f16(), a)); });

    auto* src = R"(
%foo = func(%a:u16):f16 {
  $B1: {
    %3:f16 = bitcast<f16> %a
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%a:u16):f16 {
  $B1: {
    %3:f16 = hlsl.asfloat16 %a
    ret %3
  }
}
)";

    Run(BitcastPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BitcastPolyfillTest, BitcastVec2U16ToVec2F16) {
    mod.properties.Add(core::ir::Property::kAllow16BitIntegers);
    auto* a = b.FunctionParam<vec2<u16>>("a");
    auto* func = b.Function("foo", ty.vec2h());
    func->SetParams({a});
    b.Append(func->Block(), [&] { b.Return(func, b.Bitcast(ty.vec2h(), a)); });

    auto* src = R"(
%foo = func(%a:vec2<u16>):vec2<f16> {
  $B1: {
    %3:vec2<f16> = bitcast<vec2<f16>> %a
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%a:vec2<u16>):vec2<f16> {
  $B1: {
    %3:vec2<f16> = hlsl.asfloat16 %a
    ret %3
  }
}
)";

    Run(BitcastPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BitcastPolyfillTest, BitcastVec2U16ToU32) {
    mod.properties.Add(core::ir::Property::kAllow16BitIntegers);
    auto* a = b.FunctionParam<vec2<u16>>("a");
    auto* func = b.Function("foo", ty.u32());
    func->SetParams({a});
    b.Append(func->Block(), [&] { b.Return(func, b.Bitcast(ty.u32(), a)); });

    auto* src = R"(
%foo = func(%a:vec2<u16>):u32 {
  $B1: {
    %3:u32 = bitcast<u32> %a
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%a:vec2<u16>):u32 {
  $B1: {
    %3:u32 = call %tint_bitcast_from_u16, %a
    ret %3
  }
}
%tint_bitcast_from_u16 = func(%src:vec2<u16>):u32 {
  $B2: {
    %6:vec2<u32> = convert %src
    %7:vec2<u32> = and %6, vec2<u32>(65535u)
    %8:vec2<u32> = shl %7, vec2<u32>(0u, 16u)
    %9:u32 = access %8, 0u
    %10:u32 = access %8, 1u
    %11:u32 = or %9, %10
    ret %11
  }
}
)";

    Run(BitcastPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BitcastPolyfillTest, BitcastU32ToVec2U16) {
    mod.properties.Add(core::ir::Property::kAllow16BitIntegers);
    auto* a = b.FunctionParam<u32>("a");
    auto* func = b.Function("foo", ty.vec(ty.u16(), 2));
    func->SetParams({a});
    b.Append(func->Block(), [&] { b.Return(func, b.Bitcast(ty.vec(ty.u16(), 2), a)); });

    auto* src = R"(
%foo = func(%a:u32):vec2<u16> {
  $B1: {
    %3:vec2<u16> = bitcast<vec2<u16>> %a
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%a:u32):vec2<u16> {
  $B1: {
    %3:vec2<u16> = call %tint_bitcast_to_u16, %a
    ret %3
  }
}
%tint_bitcast_to_u16 = func(%src:u32):vec2<u16> {
  $B2: {
    %v:u32 = let %src
    %7:vec2<u32> = construct %v, %v
    %8:vec2<u32> = shr %7, vec2<u32>(0u, 16u)
    %9:vec2<u32> = and %8, vec2<u32>(65535u)
    %10:vec2<u16> = convert %9
    %v16:vec2<u16> = let %10
    ret %v16
  }
}
)";

    Run(BitcastPolyfill);
    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::hlsl::writer::raise
