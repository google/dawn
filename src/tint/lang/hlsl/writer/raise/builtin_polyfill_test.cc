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

#include <string>

#include "gtest/gtest.h"
#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/ir/transform/helper_test.h"
#include "src/tint/lang/core/number.h"
#include "src/tint/lang/core/type/builtin_structs.h"
#include "src/tint/lang/core/type/depth_multisampled_texture.h"
#include "src/tint/lang/core/type/depth_texture.h"
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

TEST_F(HlslWriter_BuiltinPolyfillTest, TextureDimensions_1d) {
    auto* t = b.FunctionParam(
        "t", ty.Get<core::type::SampledTexture>(core::type::TextureDimension::k1d, ty.f32()));
    auto* func = b.Function("foo", ty.u32());
    func->SetParams({t});
    b.Append(func->Block(), [&] {
        auto* result = b.Call<u32>(core::BuiltinFn::kTextureDimensions, t);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_1d<f32>):u32 {
  $B1: {
    %3:u32 = textureDimensions %t
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_1d<f32>):u32 {
  $B1: {
    %3:ptr<function, u32, read_write> = var
    %4:void = %t.GetDimensions %3
    %5:u32 = load %3
    ret %5
  }
}
)";

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowVectorElementPointer};
    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, TextureDimensions_2d_WithoutLod) {
    auto* t = b.FunctionParam(
        "t", ty.Get<core::type::SampledTexture>(core::type::TextureDimension::k2d, ty.f32()));
    auto* func = b.Function("foo", ty.vec2<u32>());
    func->SetParams({t});
    b.Append(func->Block(), [&] {
        auto* result = b.Call<vec2<u32>>(core::BuiltinFn::kTextureDimensions, t);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_2d<f32>):vec2<u32> {
  $B1: {
    %3:vec2<u32> = textureDimensions %t
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_2d<f32>):vec2<u32> {
  $B1: {
    %3:ptr<function, vec2<u32>, read_write> = var
    %4:ptr<function, u32, read_write> = access %3, 0u
    %5:ptr<function, u32, read_write> = access %3, 1u
    %6:void = %t.GetDimensions %4, %5
    %7:vec2<u32> = load %3
    ret %7
  }
}
)";

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowVectorElementPointer};
    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, TextureDimensions_2d_WithI32Lod) {
    auto* t = b.FunctionParam(
        "t", ty.Get<core::type::SampledTexture>(core::type::TextureDimension::k2d, ty.f32()));
    auto* func = b.Function("foo", ty.vec2<u32>());
    func->SetParams({t});
    b.Append(func->Block(), [&] {
        auto* result = b.Call<vec2<u32>>(core::BuiltinFn::kTextureDimensions, t, 3_i);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_2d<f32>):vec2<u32> {
  $B1: {
    %3:vec2<u32> = textureDimensions %t, 3i
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_2d<f32>):vec2<u32> {
  $B1: {
    %3:u32 = convert 3i
    %4:ptr<function, vec3<u32>, read_write> = var
    %5:ptr<function, u32, read_write> = access %4, 0u
    %6:ptr<function, u32, read_write> = access %4, 1u
    %7:ptr<function, u32, read_write> = access %4, 2u
    %8:void = %t.GetDimensions %3, %5, %6, %7
    %9:vec3<u32> = load %4
    %10:vec2<u32> = swizzle %9, xy
    ret %10
  }
}
)";

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowVectorElementPointer};
    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, TextureDimensions_3d) {
    auto* t = b.FunctionParam(
        "t", ty.Get<core::type::SampledTexture>(core::type::TextureDimension::k3d, ty.f32()));
    auto* func = b.Function("foo", ty.vec3<u32>());
    func->SetParams({t});
    b.Append(func->Block(), [&] {
        auto* result = b.Call<vec3<u32>>(core::BuiltinFn::kTextureDimensions, t);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_3d<f32>):vec3<u32> {
  $B1: {
    %3:vec3<u32> = textureDimensions %t
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_3d<f32>):vec3<u32> {
  $B1: {
    %3:ptr<function, vec3<u32>, read_write> = var
    %4:ptr<function, u32, read_write> = access %3, 0u
    %5:ptr<function, u32, read_write> = access %3, 1u
    %6:ptr<function, u32, read_write> = access %3, 2u
    %7:void = %t.GetDimensions %4, %5, %6
    %8:vec3<u32> = load %3
    ret %8
  }
}
)";

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowVectorElementPointer};
    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, TextureLoad_1DF32) {
    auto* t = b.FunctionParam(
        "t", ty.Get<core::type::SampledTexture>(core::type::TextureDimension::k1d, ty.f32()));
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({t});
    b.Append(func->Block(), [&] {
        auto* coords = b.Zero<i32>();
        auto* level = b.Zero<u32>();
        auto* result = b.Call<vec4<f32>>(core::BuiltinFn::kTextureLoad, t, coords, level);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_1d<f32>):vec4<f32> {
  $B1: {
    %3:vec4<f32> = textureLoad %t, 0i, 0u
    ret %3
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_1d<f32>):vec4<f32> {
  $B1: {
    %3:i32 = convert 0i
    %4:i32 = convert 0u
    %5:vec2<i32> = construct %3, %4
    %6:vec4<f32> = %t.Load %5
    %7:vec4<f32> = convert %6
    ret %7
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, TextureLoad_2DLevelI32) {
    auto* t = b.FunctionParam(
        "t", ty.Get<core::type::SampledTexture>(core::type::TextureDimension::k2d, ty.i32()));
    auto* func = b.Function("foo", ty.vec4<i32>());
    func->SetParams({t});
    b.Append(func->Block(), [&] {
        auto* coords = b.Zero<vec2<i32>>();
        auto* level = b.Zero<i32>();
        auto* result = b.Call<vec4<i32>>(core::BuiltinFn::kTextureLoad, t, coords, level);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_2d<i32>):vec4<i32> {
  $B1: {
    %3:vec4<i32> = textureLoad %t, vec2<i32>(0i), 0i
    ret %3
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_2d<i32>):vec4<i32> {
  $B1: {
    %3:vec2<i32> = convert vec2<i32>(0i)
    %4:i32 = convert 0i
    %5:vec3<i32> = construct %3, %4
    %6:vec4<i32> = %t.Load %5
    %7:vec4<i32> = convert %6
    ret %7
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, TextureLoad_3DLevelU32) {
    auto* t = b.FunctionParam(
        "t", ty.Get<core::type::SampledTexture>(core::type::TextureDimension::k3d, ty.f32()));
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({t});
    b.Append(func->Block(), [&] {
        auto* coords = b.Zero<vec3<i32>>();
        auto* level = b.Zero<u32>();
        auto* result = b.Call<vec4<f32>>(core::BuiltinFn::kTextureLoad, t, coords, level);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_3d<f32>):vec4<f32> {
  $B1: {
    %3:vec4<f32> = textureLoad %t, vec3<i32>(0i), 0u
    ret %3
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_3d<f32>):vec4<f32> {
  $B1: {
    %3:vec3<i32> = convert vec3<i32>(0i)
    %4:i32 = convert 0u
    %5:vec4<i32> = construct %3, %4
    %6:vec4<f32> = %t.Load %5
    %7:vec4<f32> = convert %6
    ret %7
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, TextureLoad_Multisampled2DI32) {
    auto* t = b.FunctionParam(
        "t", ty.Get<core::type::MultisampledTexture>(core::type::TextureDimension::k2d, ty.i32()));
    auto* func = b.Function("foo", ty.vec4<i32>());
    func->SetParams({t});
    b.Append(func->Block(), [&] {
        auto* coords = b.Zero<vec2<i32>>();
        auto* sample_idx = b.Zero<u32>();
        auto* result = b.Call<vec4<i32>>(core::BuiltinFn::kTextureLoad, t, coords, sample_idx);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_multisampled_2d<i32>):vec4<i32> {
  $B1: {
    %3:vec4<i32> = textureLoad %t, vec2<i32>(0i), 0u
    ret %3
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_multisampled_2d<i32>):vec4<i32> {
  $B1: {
    %3:vec2<i32> = convert vec2<i32>(0i)
    %4:i32 = convert 0u
    %5:vec4<i32> = %t.Load %3, %4
    %6:vec4<i32> = convert %5
    ret %6
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, TextureLoad_Depth2DLevelF32) {
    auto* t =
        b.FunctionParam("t", ty.Get<core::type::DepthTexture>(core::type::TextureDimension::k2d));
    auto* func = b.Function("foo", ty.f32());
    func->SetParams({t});
    b.Append(func->Block(), [&] {
        auto* coords = b.Zero<vec2<i32>>();
        auto* level = b.Zero<u32>();
        auto* result = b.Call<f32>(core::BuiltinFn::kTextureLoad, t, coords, level);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_depth_2d):f32 {
  $B1: {
    %3:f32 = textureLoad %t, vec2<i32>(0i), 0u
    ret %3
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_depth_2d):f32 {
  $B1: {
    %3:vec2<i32> = convert vec2<i32>(0i)
    %4:i32 = convert 0u
    %5:vec3<i32> = construct %3, %4
    %6:vec4<f32> = %t.Load %5
    %7:f32 = swizzle %6, x
    ret %7
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, TextureLoad_Depth2DArrayLevelF32) {
    auto* t = b.FunctionParam(
        "t", ty.Get<core::type::DepthTexture>(core::type::TextureDimension::k2dArray));
    auto* func = b.Function("foo", ty.f32());
    func->SetParams({t});
    b.Append(func->Block(), [&] {
        auto* coords = b.Zero<vec2<i32>>();
        auto* array_idx = b.Zero<u32>();
        auto* sample_idx = b.Zero<u32>();
        auto* result = b.Call<f32>(core::BuiltinFn::kTextureLoad, t, coords, array_idx, sample_idx);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_depth_2d_array):f32 {
  $B1: {
    %3:f32 = textureLoad %t, vec2<i32>(0i), 0u, 0u
    ret %3
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_depth_2d_array):f32 {
  $B1: {
    %3:vec2<i32> = convert vec2<i32>(0i)
    %4:i32 = convert 0u
    %5:i32 = convert 0u
    %6:vec4<i32> = construct %3, %4, %5
    %7:vec4<f32> = %t.Load %6
    %8:f32 = swizzle %7, x
    ret %8
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, TextureLoad_DepthMultisampledF32) {
    auto* t = b.FunctionParam(
        "t", ty.Get<core::type::DepthMultisampledTexture>(core::type::TextureDimension::k2d));
    auto* func = b.Function("foo", ty.f32());
    func->SetParams({t});
    b.Append(func->Block(), [&] {
        auto* coords = b.Zero<vec2<i32>>();
        auto* sample_idx = b.Zero<u32>();
        auto* result = b.Call<f32>(core::BuiltinFn::kTextureLoad, t, coords, sample_idx);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_depth_multisampled_2d):f32 {
  $B1: {
    %3:f32 = textureLoad %t, vec2<i32>(0i), 0u
    ret %3
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_depth_multisampled_2d):f32 {
  $B1: {
    %3:vec2<i32> = convert vec2<i32>(0i)
    %4:i32 = convert 0u
    %5:vec4<f32> = %t.Load %3, %4
    %6:f32 = swizzle %5, x
    ret %6
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, TextureStore1D) {
    auto* t = b.Var(ty.ptr(
        handle, ty.Get<core::type::StorageTexture>(
                    core::type::TextureDimension::k1d, core::TexelFormat::kR32Float,
                    core::Access::kReadWrite,
                    core::type::StorageTexture::SubtypeFor(core::TexelFormat::kR32Float, ty))));
    t->SetBindingPoint(0, 0);
    b.ir.root_block->Append(t);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Value(1_i);
        auto* value = b.Composite(ty.vec4<f32>(), .5_f, 0_f, 0_f, 1_f);
        b.Call(ty.void_(), core::BuiltinFn::kTextureStore, b.Load(t), coords, value);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<handle, texture_storage_1d<r32float, read_write>, read> = var @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:texture_storage_1d<r32float, read_write> = load %1
    %4:void = textureStore %3, 1i, vec4<f32>(0.5f, 0.0f, 0.0f, 1.0f)
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %1:ptr<handle, texture_storage_1d<r32float, read_write>, read> = var @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:texture_storage_1d<r32float, read_write> = load %1
    %4:void = hlsl.textureStore %3, 1i, vec4<f32>(0.5f, 0.0f, 0.0f, 1.0f)
    ret
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, TextureStore3D) {
    auto* t = b.Var(ty.ptr(
        handle, ty.Get<core::type::StorageTexture>(
                    core::type::TextureDimension::k3d, core::TexelFormat::kR32Float,
                    core::Access::kReadWrite,
                    core::type::StorageTexture::SubtypeFor(core::TexelFormat::kR32Float, ty))));
    t->SetBindingPoint(0, 0);
    b.ir.root_block->Append(t);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Composite(ty.vec3<i32>(), 1_i, 2_i, 3_i);
        auto* value = b.Composite(ty.vec4<f32>(), .5_f, 0_f, 0_f, 1_f);
        b.Call(ty.void_(), core::BuiltinFn::kTextureStore, b.Load(t), coords, value);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<handle, texture_storage_3d<r32float, read_write>, read> = var @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:texture_storage_3d<r32float, read_write> = load %1
    %4:void = textureStore %3, vec3<i32>(1i, 2i, 3i), vec4<f32>(0.5f, 0.0f, 0.0f, 1.0f)
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %1:ptr<handle, texture_storage_3d<r32float, read_write>, read> = var @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:texture_storage_3d<r32float, read_write> = load %1
    %4:void = hlsl.textureStore %3, vec3<i32>(1i, 2i, 3i), vec4<f32>(0.5f, 0.0f, 0.0f, 1.0f)
    ret
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, TextureStoreArray) {
    auto* t = b.Var(ty.ptr(
        handle, ty.Get<core::type::StorageTexture>(
                    core::type::TextureDimension::k2dArray, core::TexelFormat::kRgba32Float,
                    core::Access::kReadWrite,
                    core::type::StorageTexture::SubtypeFor(core::TexelFormat::kR32Float, ty))));
    t->SetBindingPoint(0, 0);
    b.ir.root_block->Append(t);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Composite(ty.vec2<i32>(), 1_i, 2_i);
        auto* value = b.Composite(ty.vec4<f32>(), .5_f, .4_f, .3_f, 1_f);
        b.Call(ty.void_(), core::BuiltinFn::kTextureStore, b.Load(t), coords, 3_u, value);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<handle, texture_storage_2d_array<rgba32float, read_write>, read> = var @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:texture_storage_2d_array<rgba32float, read_write> = load %1
    %4:void = textureStore %3, vec2<i32>(1i, 2i), 3u, vec4<f32>(0.5f, 0.40000000596046447754f, 0.30000001192092895508f, 1.0f)
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %1:ptr<handle, texture_storage_2d_array<rgba32float, read_write>, read> = var @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:texture_storage_2d_array<rgba32float, read_write> = load %1
    %4:i32 = convert 3u
    %5:vec3<i32> = construct vec2<i32>(1i, 2i), %4
    %6:void = hlsl.textureStore %3, %5, vec4<f32>(0.5f, 0.40000000596046447754f, 0.30000001192092895508f, 1.0f)
    ret
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, QuantizeToF16) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* v = b.Var("x", b.Zero(ty.vec2<f32>()));
        b.Let("a", b.Call(ty.vec2<f32>(), core::BuiltinFn::kQuantizeToF16, b.Load(v)));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %x:ptr<function, vec2<f32>, read_write> = var, vec2<f32>(0.0f)
    %3:vec2<f32> = load %x
    %4:vec2<f32> = quantizeToF16 %3
    %a:vec2<f32> = let %4
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %x:ptr<function, vec2<f32>, read_write> = var, vec2<f32>(0.0f)
    %3:vec2<f32> = load %x
    %4:vec2<u32> = hlsl.f32tof16 %3
    %5:vec2<f32> = hlsl.f16tof32 %4
    %a:vec2<f32> = let %5
    ret
  }
}
)";
    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

struct AtomicData {
    core::BuiltinFn fn;
    const char* atomic;
    const char* interlock;
};
[[maybe_unused]] std::ostream& operator<<(std::ostream& out, const AtomicData& data) {
    out << data.interlock;
    return out;
}
using HlslBuiltinPolyfillWorkgroupAtomic = core::ir::transform::TransformTestWithParam<AtomicData>;
TEST_P(HlslBuiltinPolyfillWorkgroupAtomic, Access) {
    auto param = GetParam();
    auto* var = b.Var("v", workgroup, ty.atomic<i32>(), core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("x", b.Call(ty.i32(), param.fn, var, 123_i));
        b.Return(func);
    });

    std::string src = R"(
$B1: {  # root
  %v:ptr<workgroup, atomic<i32>, read_write> = var @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:i32 = )" + std::string(param.atomic) +
                      R"( %v, 123i
    %x:i32 = let %3
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    std::string expect = R"(
$B1: {  # root
  %v:ptr<workgroup, atomic<i32>, read_write> = var @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<function, i32, read_write> = var, 0i
    %4:void = hlsl.)" + std::string(param.interlock) +
                         R"( %v, 123i, %3
    %5:i32 = load %3
    %x:i32 = let %5
    ret
  }
}
)";
    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

INSTANTIATE_TEST_SUITE_P(
    HlslWriter_BuiltinPolyfillTest,
    HlslBuiltinPolyfillWorkgroupAtomic,
    testing::Values(AtomicData{core::BuiltinFn::kAtomicAdd, "atomicAdd", "InterlockedAdd"},
                    AtomicData{core::BuiltinFn::kAtomicMax, "atomicMax", "InterlockedMax"},
                    AtomicData{core::BuiltinFn::kAtomicMin, "atomicMin", "InterlockedMin"},
                    AtomicData{core::BuiltinFn::kAtomicAnd, "atomicAnd", "InterlockedAnd"},
                    AtomicData{core::BuiltinFn::kAtomicOr, "atomicOr", "InterlockedOr"},
                    AtomicData{core::BuiltinFn::kAtomicXor, "atomicXor", "InterlockedXor"},
                    AtomicData{core::BuiltinFn::kAtomicExchange, "atomicExchange",
                               "InterlockedExchange"}));

TEST_F(HlslWriter_BuiltinPolyfillTest, BuiltinWorkgroupAtomicStore) {
    auto* sb = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("padding"), ty.vec4<f32>()},
                                                    {mod.symbols.New("a"), ty.atomic<i32>()},
                                                    {mod.symbols.New("b"), ty.atomic<u32>()},
                                                });

    auto* var = b.Var("v", workgroup, sb, core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Call(ty.void_(), core::BuiltinFn::kAtomicStore,
               b.Access(ty.ptr<workgroup, atomic<i32>, read_write>(), var, 1_u), 123_i);
        b.Return(func);
    });

    auto* src = R"(
SB = struct @align(16) {
  padding:vec4<f32> @offset(0)
  a:atomic<i32> @offset(16)
  b:atomic<u32> @offset(20)
}

$B1: {  # root
  %v:ptr<workgroup, SB, read_write> = var @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<workgroup, atomic<i32>, read_write> = access %v, 1u
    %4:void = atomicStore %3, 123i
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
SB = struct @align(16) {
  padding:vec4<f32> @offset(0)
  a:atomic<i32> @offset(16)
  b:atomic<u32> @offset(20)
}

$B1: {  # root
  %v:ptr<workgroup, SB, read_write> = var @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<workgroup, atomic<i32>, read_write> = access %v, 1u
    %4:ptr<function, i32, read_write> = var, 0i
    %5:void = hlsl.InterlockedExchange %3, 123i, %4
    ret
  }
}
)";
    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, BuiltinWorkgroupAtomicLoad) {
    auto* sb = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("padding"), ty.vec4<f32>()},
                                                    {mod.symbols.New("a"), ty.atomic<i32>()},
                                                    {mod.symbols.New("b"), ty.atomic<u32>()},
                                                });

    auto* var = b.Var("v", workgroup, sb, core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("x", b.Call(ty.i32(), core::BuiltinFn::kAtomicLoad,
                          b.Access(ty.ptr<workgroup, atomic<i32>, read_write>(), var, 1_u)));
        b.Return(func);
    });

    auto* src = R"(
SB = struct @align(16) {
  padding:vec4<f32> @offset(0)
  a:atomic<i32> @offset(16)
  b:atomic<u32> @offset(20)
}

$B1: {  # root
  %v:ptr<workgroup, SB, read_write> = var @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<workgroup, atomic<i32>, read_write> = access %v, 1u
    %4:i32 = atomicLoad %3
    %x:i32 = let %4
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
SB = struct @align(16) {
  padding:vec4<f32> @offset(0)
  a:atomic<i32> @offset(16)
  b:atomic<u32> @offset(20)
}

$B1: {  # root
  %v:ptr<workgroup, SB, read_write> = var @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<workgroup, atomic<i32>, read_write> = access %v, 1u
    %4:ptr<function, i32, read_write> = var, 0i
    %5:void = hlsl.InterlockedOr %3, 0i, %4
    %6:i32 = load %4
    %x:i32 = let %6
    ret
  }
}
)";
    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, BuiltinWorkgroupAtomicSub) {
    auto* sb = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("padding"), ty.vec4<f32>()},
                                                    {mod.symbols.New("a"), ty.atomic<i32>()},
                                                    {mod.symbols.New("b"), ty.atomic<u32>()},
                                                });

    auto* var = b.Var("v", workgroup, sb, core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("x", b.Call(ty.i32(), core::BuiltinFn::kAtomicSub,
                          b.Access(ty.ptr<workgroup, atomic<i32>, read_write>(), var, 1_u), 123_i));
        b.Return(func);
    });

    auto* src = R"(
SB = struct @align(16) {
  padding:vec4<f32> @offset(0)
  a:atomic<i32> @offset(16)
  b:atomic<u32> @offset(20)
}

$B1: {  # root
  %v:ptr<workgroup, SB, read_write> = var @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<workgroup, atomic<i32>, read_write> = access %v, 1u
    %4:i32 = atomicSub %3, 123i
    %x:i32 = let %4
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
SB = struct @align(16) {
  padding:vec4<f32> @offset(0)
  a:atomic<i32> @offset(16)
  b:atomic<u32> @offset(20)
}

$B1: {  # root
  %v:ptr<workgroup, SB, read_write> = var @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<workgroup, atomic<i32>, read_write> = access %v, 1u
    %4:ptr<function, i32, read_write> = var, 0i
    %5:i32 = negation 123i
    %6:void = hlsl.InterlockedAdd %3, %5, %4
    %7:i32 = load %4
    %x:i32 = let %7
    ret
  }
}
)";
    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, BuiltinWorkgroupAtomicCompareExchangeWeak) {
    auto* sb = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("padding"), ty.vec4<f32>()},
                                                    {mod.symbols.New("a"), ty.atomic<i32>()},
                                                    {mod.symbols.New("b"), ty.atomic<u32>()},
                                                });

    auto* var = b.Var("v", workgroup, sb, core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("x", b.Call(core::type::CreateAtomicCompareExchangeResult(ty, mod.symbols, ty.i32()),
                          core::BuiltinFn::kAtomicCompareExchangeWeak,
                          b.Access(ty.ptr<workgroup, atomic<i32>, read_write>(), var, 1_u), 123_i,
                          345_i));
        b.Return(func);
    });

    auto* src = R"(
SB = struct @align(16) {
  padding:vec4<f32> @offset(0)
  a:atomic<i32> @offset(16)
  b:atomic<u32> @offset(20)
}

__atomic_compare_exchange_result_i32 = struct @align(4) {
  old_value:i32 @offset(0)
  exchanged:bool @offset(4)
}

$B1: {  # root
  %v:ptr<workgroup, SB, read_write> = var @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<workgroup, atomic<i32>, read_write> = access %v, 1u
    %4:__atomic_compare_exchange_result_i32 = atomicCompareExchangeWeak %3, 123i, 345i
    %x:__atomic_compare_exchange_result_i32 = let %4
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
SB = struct @align(16) {
  padding:vec4<f32> @offset(0)
  a:atomic<i32> @offset(16)
  b:atomic<u32> @offset(20)
}

__atomic_compare_exchange_result_i32 = struct @align(4) {
  old_value:i32 @offset(0)
  exchanged:bool @offset(4)
}

$B1: {  # root
  %v:ptr<workgroup, SB, read_write> = var @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<workgroup, atomic<i32>, read_write> = access %v, 1u
    %4:ptr<function, i32, read_write> = var, 0i
    %5:void = hlsl.InterlockedCompareExchange %3, 123i, 345i, %4
    %6:i32 = load %4
    %7:bool = eq %6, 123i
    %8:__atomic_compare_exchange_result_i32 = construct %6, %7
    %x:__atomic_compare_exchange_result_i32 = let %8
    ret
  }
}
)";
    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, Unpack2x16Float) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* u = b.Var("u", 2_u);
        b.Let("a", b.Call(ty.vec2<f32>(), core::BuiltinFn::kUnpack2X16Float, b.Load(u)));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %u:ptr<function, u32, read_write> = var, 2u
    %3:u32 = load %u
    %4:vec2<f32> = unpack2x16float %3
    %a:vec2<f32> = let %4
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %u:ptr<function, u32, read_write> = var, 2u
    %3:u32 = load %u
    %4:u32 = and %3, 65535u
    %5:u32 = shr %3, 16u
    %6:vec2<u32> = construct %4, %5
    %7:vec2<f32> = hlsl.f16tof32 %6
    %a:vec2<f32> = let %7
    ret
  }
}
)";
    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, Unpack2x16snorm) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* u = b.Var("u", 2_u);
        b.Let("a", b.Call(ty.vec2<f32>(), core::BuiltinFn::kUnpack2X16Snorm, b.Load(u)));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %u:ptr<function, u32, read_write> = var, 2u
    %3:u32 = load %u
    %4:vec2<f32> = unpack2x16snorm %3
    %a:vec2<f32> = let %4
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %u:ptr<function, u32, read_write> = var, 2u
    %3:u32 = load %u
    %4:i32 = convert %3
    %5:i32 = shl %4, 16u
    %6:vec2<i32> = construct %5, %4
    %7:vec2<i32> = shr %6, vec2<u32>(16u)
    %8:vec2<f32> = convert %7
    %9:vec2<f32> = div %8, 32767.0f
    %10:vec2<f32> = clamp %9, vec2<f32>(-1.0f), vec2<f32>(1.0f)
    %a:vec2<f32> = let %10
    ret
  }
}
)";
    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, Unpack2x16unorm) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* u = b.Var("u", 2_u);
        b.Let("a", b.Call(ty.vec2<f32>(), core::BuiltinFn::kUnpack2X16Unorm, b.Load(u)));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %u:ptr<function, u32, read_write> = var, 2u
    %3:u32 = load %u
    %4:vec2<f32> = unpack2x16unorm %3
    %a:vec2<f32> = let %4
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %u:ptr<function, u32, read_write> = var, 2u
    %3:u32 = load %u
    %4:u32 = and %3, 65535u
    %5:u32 = shr %3, 16u
    %6:vec2<u32> = construct %4, %5
    %7:vec2<f32> = convert %6
    %8:vec2<f32> = div %7, 65535.0f
    %a:vec2<f32> = let %8
    ret
  }
}
)";
    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, Unpack4x8Snorm) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* u = b.Var("u", 2_u);
        b.Let("a", b.Call(ty.vec4<f32>(), core::BuiltinFn::kUnpack4X8Snorm, b.Load(u)));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %u:ptr<function, u32, read_write> = var, 2u
    %3:u32 = load %u
    %4:vec4<f32> = unpack4x8snorm %3
    %a:vec4<f32> = let %4
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %u:ptr<function, u32, read_write> = var, 2u
    %3:u32 = load %u
    %4:i32 = convert %3
    %5:i32 = shl %4, 24u
    %6:i32 = shl %4, 16u
    %7:i32 = shl %4, 8u
    %8:vec4<i32> = construct %5, %6, %7, %4
    %9:vec4<i32> = shr %8, vec4<u32>(24u)
    %10:vec4<f32> = convert %9
    %11:vec4<f32> = div %10, 127.0f
    %12:vec4<f32> = clamp %11, vec4<f32>(-1.0f), vec4<f32>(1.0f)
    %a:vec4<f32> = let %12
    ret
  }
}
)";
    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, Unpack4x8Unorm) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* u = b.Var("u", 2_u);
        b.Let("a", b.Call(ty.vec4<f32>(), core::BuiltinFn::kUnpack4X8Unorm, b.Load(u)));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %u:ptr<function, u32, read_write> = var, 2u
    %3:u32 = load %u
    %4:vec4<f32> = unpack4x8unorm %3
    %a:vec4<f32> = let %4
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %u:ptr<function, u32, read_write> = var, 2u
    %3:u32 = load %u
    %4:u32 = and %3, 255u
    %5:u32 = shr %3, 8u
    %6:u32 = and %5, 255u
    %7:u32 = shr %3, 16u
    %8:u32 = and %7, 255u
    %9:u32 = shr %3, 24u
    %10:vec4<u32> = construct %4, %6, %8, %9
    %11:vec4<f32> = convert %10
    %12:vec4<f32> = div %11, 255.0f
    %a:vec4<f32> = let %12
    ret
  }
}
)";
    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, Unpack4xI8) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* u = b.Var("u", 2_u);
        b.Let("a", b.Call(ty.vec4<i32>(), core::BuiltinFn::kUnpack4XI8, b.Load(u)));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %u:ptr<function, u32, read_write> = var, 2u
    %3:u32 = load %u
    %4:vec4<i32> = unpack4xI8 %3
    %a:vec4<i32> = let %4
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %u:ptr<function, u32, read_write> = var, 2u
    %3:u32 = load %u
    %4:hlsl.int8_t4_packed = convert %3
    %5:vec4<i32> = hlsl.unpack_s8s32 %4
    %a:vec4<i32> = let %5
    ret
  }
}
)";
    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriter_BuiltinPolyfillTest, Unpack4xU8) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* u = b.Var("u", 2_u);
        b.Let("a", b.Call(ty.vec4<u32>(), core::BuiltinFn::kUnpack4XU8, b.Load(u)));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %u:ptr<function, u32, read_write> = var, 2u
    %3:u32 = load %u
    %4:vec4<u32> = unpack4xU8 %3
    %a:vec4<u32> = let %4
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %u:ptr<function, u32, read_write> = var, 2u
    %3:u32 = load %u
    %4:hlsl.uint8_t4_packed = convert %3
    %5:vec4<u32> = hlsl.unpack_u8u32 %4
    %a:vec4<u32> = let %5
    ret
  }
}
)";
    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::hlsl::writer::raise
