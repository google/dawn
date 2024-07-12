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

#include "src/tint/lang/hlsl/writer/raise/builtin_polyfill.h"

#include "gtest/gtest.h"
#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/ir/transform/helper_test.h"
#include "src/tint/lang/core/number.h"
#include "src/tint/lang/core/type/builtin_structs.h"
#include "src/tint/lang/core/type/multisampled_texture.h"
#include "src/tint/lang/core/type/sampled_texture.h"
#include "src/tint/lang/core/type/storage_texture.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::hlsl::writer::raise {
namespace {

using HlslWriter_BuiltinPolyfillTest = core::ir::transform::TransformTest;

TEST_F(HlslWriter_BuiltinPolyfillTest, BitcastIdentity) {
    auto* a = b.FunctionParam<i32>("a");
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({a});
    b.Append(func->Block(), [&] { b.Return(func, b.Bitcast<i32>(a)); });

    auto* src = R"(
%foo = func(%a:i32):i32 {
  $B1: {
    %3:i32 = bitcast %a
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

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, Asuint) {
    auto* a = b.FunctionParam<i32>("a");
    auto* func = b.Function("foo", ty.u32());
    func->SetParams({a});
    b.Append(func->Block(), [&] { b.Return(func, b.Bitcast<u32>(a)); });

    auto* src = R"(
%foo = func(%a:i32):u32 {
  $B1: {
    %3:u32 = bitcast %a
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

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, Asint) {
    auto* a = b.FunctionParam<u32>("a");
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({a});
    b.Append(func->Block(), [&] { b.Return(func, b.Bitcast<i32>(a)); });

    auto* src = R"(
%foo = func(%a:u32):i32 {
  $B1: {
    %3:i32 = bitcast %a
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

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, Asfloat) {
    auto* a = b.FunctionParam<i32>("a");
    auto* func = b.Function("foo", ty.f32());
    func->SetParams({a});
    b.Append(func->Block(), [&] { b.Return(func, b.Bitcast<f32>(a)); });

    auto* src = R"(
%foo = func(%a:i32):f32 {
  $B1: {
    %3:f32 = bitcast %a
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

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, AsfloatVec) {
    auto* a = b.FunctionParam<vec3<i32>>("a");
    auto* func = b.Function("foo", ty.vec<f32, 3>());
    func->SetParams({a});
    b.Append(func->Block(), [&] { b.Return(func, b.Bitcast(ty.vec(ty.f32(), 3), a)); });

    auto* src = R"(
%foo = func(%a:vec3<i32>):vec3<f32> {
  $B1: {
    %3:vec3<f32> = bitcast %a
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

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, BitcastFromF16) {
    auto* a = b.FunctionParam<vec2<f16>>("a");
    auto* func = b.Function("foo", ty.f32());
    func->SetParams({a});
    b.Append(func->Block(), [&] { b.Return(func, b.Bitcast(ty.f32(), a)); });

    auto* src = R"(
%foo = func(%a:vec2<f16>):f32 {
  $B1: {
    %3:f32 = bitcast %a
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
    %6:vec2<f32> = convert %src
    %7:vec2<u32> = hlsl.f32tof16 %6
    %r:vec2<u32> = let %7
    %9:u32 = swizzle %r, x
    %10:u32 = and %9, 65535u
    %11:u32 = swizzle %r, y
    %12:u32 = and %11, 65535u
    %13:u32 = shl %12, 16u
    %14:u32 = or %10, %13
    %15:f32 = hlsl.asfloat %14
    ret %15
  }
}
)";

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, BitcastToF16) {
    auto* a = b.FunctionParam<f32>("a");
    auto* func = b.Function("foo", ty.vec2<f16>());
    func->SetParams({a});
    b.Append(func->Block(), [&] { b.Return(func, b.Bitcast(ty.vec2<f16>(), a)); });

    auto* src = R"(
%foo = func(%a:f32):vec2<f16> {
  $B1: {
    %3:vec2<f16> = bitcast %a
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
    %8:u32 = and %v, 65535u
    %9:f32 = hlsl.f16tof32 %8
    %t_low:f32 = let %9
    %11:u32 = shr %v, 16u
    %12:u32 = and %11, 65535u
    %13:f32 = hlsl.f16tof32 %12
    %t_high:f32 = let %13
    %15:f16 = swizzle %t_low, x
    %16:f16 = swizzle %t_high, x
    %17:vec2<f16> = construct %15, %16
    ret %17
  }
}
)";

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, BitcastFromVec2F16) {
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
    %2:vec2<f16> = construct 1.0h, 2.0h
    %a:ptr<function, vec2<f16>, read_write> = var, %2
    %4:vec2<f16> = load %a
    %5:i32 = bitcast %4
    %b:i32 = let %5
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %2:vec2<f16> = construct 1.0h, 2.0h
    %a:ptr<function, vec2<f16>, read_write> = var, %2
    %4:vec2<f16> = load %a
    %5:i32 = call %tint_bitcast_from_f16, %4
    %b:i32 = let %5
    ret
  }
}
%tint_bitcast_from_f16 = func(%src:vec2<f16>):i32 {
  $B2: {
    %9:vec2<f32> = convert %src
    %10:vec2<u32> = hlsl.f32tof16 %9
    %r:vec2<u32> = let %10
    %12:u32 = swizzle %r, x
    %13:u32 = and %12, 65535u
    %14:u32 = swizzle %r, y
    %15:u32 = and %14, 65535u
    %16:u32 = shl %15, 16u
    %17:u32 = or %13, %16
    %18:i32 = hlsl.asint %17
    ret %18
  }
}
)";

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, BitcastToVec4F16) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Var("a", b.Construct<vec2<i32>>(1_i, 2_i));
        b.Let("b", b.Bitcast<vec4<f16>>(b.Load(a)));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %2:vec2<i32> = construct 1i, 2i
    %a:ptr<function, vec2<i32>, read_write> = var, %2
    %4:vec2<i32> = load %a
    %5:vec4<f16> = bitcast %4
    %b:vec4<f16> = let %5
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %2:vec2<i32> = construct 1i, 2i
    %a:ptr<function, vec2<i32>, read_write> = var, %2
    %4:vec2<i32> = load %a
    %5:vec4<f16> = call %tint_bitcast_to_f16, %4
    %b:vec4<f16> = let %5
    ret
  }
}
%tint_bitcast_to_f16 = func(%src:vec2<i32>):vec4<f16> {
  $B2: {
    %9:vec2<u32> = hlsl.asuint %src
    %v:vec2<u32> = let %9
    %mask:vec2<u32> = let vec2<u32>(65535u)
    %shift:vec2<u32> = let vec2<u32>(16u)
    %13:vec2<u32> = and %v, %mask
    %14:vec2<f32> = hlsl.f16tof32 %13
    %t_low:vec2<f32> = let %14
    %16:vec2<u32> = shr %v, %shift
    %17:vec2<u32> = and %16, %mask
    %18:vec2<f32> = hlsl.f16tof32 %17
    %t_high:vec2<f32> = let %18
    %20:f16 = swizzle %t_low, x
    %21:f16 = swizzle %t_high, x
    %22:f16 = swizzle %t_low, y
    %23:f16 = swizzle %t_high, y
    %24:vec4<f16> = construct %20, %21, %22, %23
    ret %24
  }
}
)";

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, Sign) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("a", b.Call(ty.f32(), core::BuiltinFn::kSign, -1_f));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %2:f32 = sign -1.0f
    %a:f32 = let %2
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %2:i32 = hlsl.sign -1.0f
    %3:f32 = convert %2
    %a:f32 = let %3
    ret
  }
}
)";

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, SignVec) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("a", b.Call(ty.vec3<i32>(), core::BuiltinFn::kSign,
                          b.Composite(ty.vec3<i32>(), 1_i, 2_i, 3_i)));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %2:vec3<i32> = sign vec3<i32>(1i, 2i, 3i)
    %a:vec3<i32> = let %2
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %2:vec3<i32> = hlsl.sign vec3<i32>(1i, 2i, 3i)
    %3:vec3<i32> = convert %2
    %a:vec3<i32> = let %3
    ret
  }
}
)";

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, TextureNumLevels) {
    auto* t = b.FunctionParam(
        "t", ty.Get<core::type::SampledTexture>(core::type::TextureDimension::k1d, ty.f32()));
    auto* func = b.Function("foo", ty.u32());
    func->SetParams({t});
    b.Append(func->Block(), [&] {
        auto* result = b.Call<u32>(core::BuiltinFn::kTextureNumLevels, t);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_1d<f32>):u32 {
  $B1: {
    %3:u32 = textureNumLevels %t
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_1d<f32>):u32 {
  $B1: {
    %3:ptr<function, vec2<u32>, read_write> = var
    %4:ptr<function, u32, read_write> = access %3, 0u
    %5:ptr<function, u32, read_write> = access %3, 1u
    %6:void = %t.GetDimensions 0u, %4, %5
    %7:u32 = swizzle %3, y
    ret %7
  }
}
)";

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowVectorElementPointer};
    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, TextureNumLayers) {
    auto* t = b.FunctionParam("t", ty.Get<core::type::SampledTexture>(
                                       core::type::TextureDimension::kCubeArray, ty.f32()));
    auto* func = b.Function("foo", ty.u32());
    func->SetParams({t});
    b.Append(func->Block(), [&] {
        auto* result = b.Call<u32>(core::BuiltinFn::kTextureNumLayers, t);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_cube_array<f32>):u32 {
  $B1: {
    %3:u32 = textureNumLayers %t
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_cube_array<f32>):u32 {
  $B1: {
    %3:ptr<function, vec3<u32>, read_write> = var
    %4:ptr<function, u32, read_write> = access %3, 0u
    %5:ptr<function, u32, read_write> = access %3, 1u
    %6:ptr<function, u32, read_write> = access %3, 2u
    %7:void = %t.GetDimensions %4, %5, %6
    %8:u32 = swizzle %3, z
    ret %8
  }
}
)";

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowVectorElementPointer};
    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, TextureNumSamples) {
    auto* t = b.FunctionParam(
        "t", ty.Get<core::type::MultisampledTexture>(core::type::TextureDimension::k2d, ty.f32()));
    auto* func = b.Function("foo", ty.u32());
    func->SetParams({t});
    b.Append(func->Block(), [&] {
        auto* result = b.Call<u32>(core::BuiltinFn::kTextureNumSamples, t);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_multisampled_2d<f32>):u32 {
  $B1: {
    %3:u32 = textureNumSamples %t
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_multisampled_2d<f32>):u32 {
  $B1: {
    %3:ptr<function, vec3<u32>, read_write> = var
    %4:ptr<function, u32, read_write> = access %3, 0u
    %5:ptr<function, u32, read_write> = access %3, 1u
    %6:ptr<function, u32, read_write> = access %3, 2u
    %7:void = %t.GetDimensions %4, %5, %6
    %8:u32 = swizzle %3, z
    ret %8
  }
}
)";

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowVectorElementPointer};
    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::hlsl::writer::raise
