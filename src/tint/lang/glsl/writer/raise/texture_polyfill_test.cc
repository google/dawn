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

#include "src/tint/lang/glsl/writer/raise/texture_polyfill.h"

#include <vector>

#include "gtest/gtest.h"
#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/ir/transform/helper_test.h"
#include "src/tint/lang/core/number.h"
#include "src/tint/lang/core/type/depth_multisampled_texture.h"
#include "src/tint/lang/core/type/depth_texture.h"
#include "src/tint/lang/core/type/multisampled_texture.h"
#include "src/tint/lang/core/type/sampled_texture.h"
#include "src/tint/lang/core/type/storage_texture.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::glsl::writer::raise {
namespace {

using GlslWriter_TexturePolyfillTest = core::ir::transform::TransformTest;

TEST_F(GlslWriter_TexturePolyfillTest, TextureDimensions_1d) {
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
%foo = func(%t:texture_2d<f32>):u32 {
  $B1: {
    %3:vec2<i32> = glsl.textureSize %t, 0i
    %4:vec2<u32> = bitcast %3
    %5:u32 = swizzle %4, x
    ret %5
  }
}
)";

    TexturePolyfillConfig cfg;
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureDimensions_2d_WithoutLod) {
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
    %3:vec2<i32> = glsl.textureSize %t, 0i
    %4:vec2<u32> = bitcast %3
    ret %4
  }
}
)";

    TexturePolyfillConfig cfg;
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureDimensions_2d_WithU32Lod) {
    auto* t = b.FunctionParam(
        "t", ty.Get<core::type::SampledTexture>(core::type::TextureDimension::k2d, ty.f32()));
    auto* func = b.Function("foo", ty.vec2<u32>());
    func->SetParams({t});
    b.Append(func->Block(), [&] {
        auto* result = b.Call<vec2<u32>>(core::BuiltinFn::kTextureDimensions, t, 3_u);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_2d<f32>):vec2<u32> {
  $B1: {
    %3:vec2<u32> = textureDimensions %t, 3u
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_2d<f32>):vec2<u32> {
  $B1: {
    %3:i32 = bitcast 3u
    %4:vec2<i32> = glsl.textureSize %t, %3
    %5:vec2<u32> = bitcast %4
    ret %5
  }
}
)";

    TexturePolyfillConfig cfg;
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureDimensions_2dArray) {
    auto* t = b.FunctionParam(
        "t", ty.Get<core::type::SampledTexture>(core::type::TextureDimension::k2dArray, ty.f32()));
    auto* func = b.Function("foo", ty.vec2<u32>());
    func->SetParams({t});
    b.Append(func->Block(), [&] {
        auto* result = b.Call<vec2<u32>>(core::BuiltinFn::kTextureDimensions, t);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_2d_array<f32>):vec2<u32> {
  $B1: {
    %3:vec2<u32> = textureDimensions %t
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_2d_array<f32>):vec2<u32> {
  $B1: {
    %3:vec3<i32> = glsl.textureSize %t, 0i
    %4:vec2<i32> = swizzle %3, xy
    %5:vec2<u32> = bitcast %4
    ret %5
  }
}
)";

    TexturePolyfillConfig cfg;
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureDimensions_Storage2D) {
    auto* t = b.FunctionParam(
        "t",
        ty.Get<core::type::StorageTexture>(
            core::type::TextureDimension::k2d, core::TexelFormat::kRg32Float, core::Access::kRead,
            core::type::StorageTexture::SubtypeFor(core::TexelFormat::kRg32Float, ty)));
    auto* func = b.Function("foo", ty.vec2<u32>());
    func->SetParams({t});
    b.Append(func->Block(), [&] {
        auto* result = b.Call<vec2<u32>>(core::BuiltinFn::kTextureDimensions, t);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_storage_2d<rg32float, read>):vec2<u32> {
  $B1: {
    %3:vec2<u32> = textureDimensions %t
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_storage_2d<rg32float, read>):vec2<u32> {
  $B1: {
    %3:vec2<i32> = glsl.imageSize %t
    %4:vec2<u32> = bitcast %3
    ret %4
  }
}
)";

    TexturePolyfillConfig cfg;
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureDimensions_DepthMultisampled) {
    auto* t = b.FunctionParam(
        "t", ty.Get<core::type::DepthMultisampledTexture>(core::type::TextureDimension::k2d));
    auto* func = b.Function("foo", ty.vec2<u32>());
    func->SetParams({t});
    b.Append(func->Block(), [&] {
        auto* result = b.Call<vec2<u32>>(core::BuiltinFn::kTextureDimensions, t);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_depth_multisampled_2d):vec2<u32> {
  $B1: {
    %3:vec2<u32> = textureDimensions %t
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_depth_multisampled_2d):vec2<u32> {
  $B1: {
    %3:vec2<i32> = glsl.textureSize %t
    %4:vec2<u32> = bitcast %3
    ret %4
  }
}
)";

    TexturePolyfillConfig cfg;
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureNumLayers_2DArray) {
    auto* var =
        b.Var("v", handle,
              ty.Get<core::type::SampledTexture>(core::type::TextureDimension::k2dArray, ty.f32()),
              core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("x", b.Call(ty.u32(), core::BuiltinFn::kTextureNumLayers, b.Load(var)));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<handle, texture_2d_array<f32>, read_write> = var @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:texture_2d_array<f32> = load %v
    %4:u32 = textureNumLayers %3
    %x:u32 = let %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:ptr<handle, texture_2d_array<f32>, read_write> = var @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:texture_2d_array<f32> = load %v
    %4:vec3<i32> = glsl.textureSize %3, 0i
    %5:i32 = swizzle %4, z
    %6:u32 = bitcast %5
    %x:u32 = let %6
    ret
  }
}
)";

    TexturePolyfillConfig cfg;
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureNumLayers_Depth2DArray) {
    auto* var =
        b.Var("v", handle, ty.Get<core::type::DepthTexture>(core::type::TextureDimension::k2dArray),
              core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("x", b.Call(ty.u32(), core::BuiltinFn::kTextureNumLayers, b.Load(var)));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<handle, texture_depth_2d_array, read_write> = var @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:texture_depth_2d_array = load %v
    %4:u32 = textureNumLayers %3
    %x:u32 = let %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:ptr<handle, texture_depth_2d_array, read_write> = var @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:texture_depth_2d_array = load %v
    %4:vec3<i32> = glsl.textureSize %3, 0i
    %5:i32 = swizzle %4, z
    %6:u32 = bitcast %5
    %x:u32 = let %6
    ret
  }
}
)";

    TexturePolyfillConfig cfg;
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureNumLayers_CubeArray) {
    auto* var = b.Var(
        "v", handle,
        ty.Get<core::type::SampledTexture>(core::type::TextureDimension::kCubeArray, ty.f32()),
        core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("x", b.Call(ty.u32(), core::BuiltinFn::kTextureNumLayers, b.Load(var)));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<handle, texture_cube_array<f32>, read_write> = var @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:texture_cube_array<f32> = load %v
    %4:u32 = textureNumLayers %3
    %x:u32 = let %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:ptr<handle, texture_cube_array<f32>, read_write> = var @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:texture_cube_array<f32> = load %v
    %4:vec3<i32> = glsl.textureSize %3, 0i
    %5:i32 = swizzle %4, z
    %6:u32 = bitcast %5
    %x:u32 = let %6
    ret
  }
}
)";

    TexturePolyfillConfig cfg;
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureNumLayers_DepthCubeArray) {
    auto* var = b.Var("v", handle,
                      ty.Get<core::type::DepthTexture>(core::type::TextureDimension::kCubeArray),
                      core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("x", b.Call(ty.u32(), core::BuiltinFn::kTextureNumLayers, b.Load(var)));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<handle, texture_depth_cube_array, read_write> = var @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:texture_depth_cube_array = load %v
    %4:u32 = textureNumLayers %3
    %x:u32 = let %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:ptr<handle, texture_depth_cube_array, read_write> = var @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:texture_depth_cube_array = load %v
    %4:vec3<i32> = glsl.textureSize %3, 0i
    %5:i32 = swizzle %4, z
    %6:u32 = bitcast %5
    %x:u32 = let %6
    ret
  }
}
)";

    TexturePolyfillConfig cfg;
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureNumLayers_Storage2DArray) {
    auto* storage_ty = ty.Get<core::type::StorageTexture>(
        core::type::TextureDimension::k2dArray, core::TexelFormat::kRg32Float, core::Access::kRead,
        core::type::StorageTexture::SubtypeFor(core::TexelFormat::kRg32Float, ty));

    auto* var = b.Var("v", handle, storage_ty, core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("x", b.Call(ty.u32(), core::BuiltinFn::kTextureNumLayers, b.Load(var)));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<handle, texture_storage_2d_array<rg32float, read>, read_write> = var @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:texture_storage_2d_array<rg32float, read> = load %v
    %4:u32 = textureNumLayers %3
    %x:u32 = let %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:ptr<handle, texture_storage_2d_array<rg32float, read>, read_write> = var @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:texture_storage_2d_array<rg32float, read> = load %v
    %4:vec3<i32> = glsl.imageSize %3
    %5:i32 = swizzle %4, z
    %6:u32 = bitcast %5
    %x:u32 = let %6
    ret
  }
}
)";

    TexturePolyfillConfig cfg;
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureLoad_1DF32) {
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
%foo = func(%t:texture_2d<f32>):vec4<f32> {
  $B1: {
    %3:vec2<i32> = construct 0i, 0i
    %4:vec2<i32> = convert %3
    %5:i32 = convert 0u
    %6:vec4<f32> = glsl.texelFetch %t, %4, %5
    ret %6
  }
}
)";

    TexturePolyfillConfig cfg;
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureLoad_2DLevelI32) {
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
    %5:vec4<i32> = glsl.texelFetch %t, %3, %4
    ret %5
  }
}
)";

    TexturePolyfillConfig cfg;
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureLoad_3DLevelU32) {
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
    %5:vec4<f32> = glsl.texelFetch %t, %3, %4
    ret %5
  }
}
)";

    TexturePolyfillConfig cfg;
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureLoad_Multisampled2DI32) {
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
    %5:vec4<i32> = glsl.texelFetch %t, %3, %4
    ret %5
  }
}
)";

    TexturePolyfillConfig cfg;
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureLoad_Storage2D) {
    auto* t = b.FunctionParam(
        "t",
        ty.Get<core::type::StorageTexture>(
            core::type::TextureDimension::k2d, core::TexelFormat::kRg32Float, core::Access::kRead,
            core::type::StorageTexture::SubtypeFor(core::TexelFormat::kRg32Float, ty)));
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({t});
    b.Append(func->Block(), [&] {
        auto* coords = b.Zero<vec2<i32>>();
        auto* result = b.Call<vec4<f32>>(core::BuiltinFn::kTextureLoad, t, coords);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%t:texture_storage_2d<rg32float, read>):vec4<f32> {
  $B1: {
    %3:vec4<f32> = textureLoad %t, vec2<i32>(0i)
    ret %3
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
%foo = func(%t:texture_storage_2d<rg32float, read>):vec4<f32> {
  $B1: {
    %3:vec2<i32> = convert vec2<i32>(0i)
    %4:vec4<f32> = glsl.imageLoad %t, %3
    ret %4
  }
}
)";

    TexturePolyfillConfig cfg;
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureStore1D) {
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
  %1:ptr<handle, texture_storage_2d<r32float, read_write>, read> = var @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:texture_storage_2d<r32float, read_write> = load %1
    %4:vec2<i32> = construct 1i, 0i
    %5:void = glsl.imageStore %3, %4, vec4<f32>(0.5f, 0.0f, 0.0f, 1.0f)
    ret
  }
}
)";

    TexturePolyfillConfig cfg;
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureStore3D) {
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
    %4:void = glsl.imageStore %3, vec3<i32>(1i, 2i, 3i), vec4<f32>(0.5f, 0.0f, 0.0f, 1.0f)
    ret
  }
}
)";

    TexturePolyfillConfig cfg;
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureStoreArray) {
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
    %6:void = glsl.imageStore %3, %5, vec4<f32>(0.5f, 0.40000000596046447754f, 0.30000001192092895508f, 1.0f)
    ret
  }
}
)";

    TexturePolyfillConfig cfg;
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureNumLevels) {
    auto* t = b.Var(ty.ptr(
        handle, ty.Get<core::type::SampledTexture>(core::type::TextureDimension::k2d, ty.f32()),
        read_write));
    t->SetBindingPoint(0, 0);
    b.ir.root_block->Append(t);

    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    func->SetWorkgroupSize(1, 1, 1);
    b.Append(func->Block(), [&] {
        auto* tex = b.Load(t);
        b.Let("len", b.Call(ty.u32(), core::BuiltinFn::kTextureNumLevels, tex));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<handle, texture_2d<f32>, read_write> = var @binding_point(0, 0)
}

%main = @compute @workgroup_size(1, 1, 1) func():void {
  $B2: {
    %3:texture_2d<f32> = load %1
    %4:u32 = textureNumLevels %3
    %len:u32 = let %4
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    auto* expect = R"(
TintTextureUniformData = struct @align(4) {
  tint_builtin_value_0:u32 @offset(0)
}

$B1: {  # root
  %1:ptr<handle, texture_2d<f32>, read_write> = var @binding_point(0, 0)
  %2:ptr<uniform, TintTextureUniformData, read> = var @binding_point(0, 30)
}

%main = @compute @workgroup_size(1, 1, 1) func():void {
  $B2: {
    %4:texture_2d<f32> = load %1
    %5:ptr<uniform, u32, read> = access %2, 0u
    %6:u32 = load %5
    %len:u32 = let %6
    ret
  }
}
)";

    TexturePolyfillConfig cfg;
    cfg.texture_builtins_from_uniform = {{0, 30u}, std::vector<BindingPoint>{{0, 0}}};
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureNumSamples) {
    auto* t = b.Var(ty.ptr(
        handle, ty.Get<core::type::DepthMultisampledTexture>(core::type::TextureDimension::k2d),
        read_write));
    t->SetBindingPoint(0, 0);
    b.ir.root_block->Append(t);

    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    func->SetWorkgroupSize(1, 1, 1);
    b.Append(func->Block(), [&] {
        auto* tex = b.Load(t);
        b.Let("len", b.Call(ty.u32(), core::BuiltinFn::kTextureNumSamples, tex));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<handle, texture_depth_multisampled_2d, read_write> = var @binding_point(0, 0)
}

%main = @compute @workgroup_size(1, 1, 1) func():void {
  $B2: {
    %3:texture_depth_multisampled_2d = load %1
    %4:u32 = textureNumSamples %3
    %len:u32 = let %4
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    auto* expect = R"(
TintTextureUniformData = struct @align(4) {
  tint_builtin_value_0:u32 @offset(0)
}

$B1: {  # root
  %1:ptr<handle, texture_depth_multisampled_2d, read_write> = var @binding_point(0, 0)
  %2:ptr<uniform, TintTextureUniformData, read> = var @binding_point(0, 30)
}

%main = @compute @workgroup_size(1, 1, 1) func():void {
  $B2: {
    %4:texture_depth_multisampled_2d = load %1
    %5:ptr<uniform, u32, read> = access %2, 0u
    %6:u32 = load %5
    %len:u32 = let %6
    ret
  }
}
)";

    TexturePolyfillConfig cfg;
    cfg.texture_builtins_from_uniform = {{0, 30u}, std::vector<BindingPoint>{{0, 0}}};
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, SameBuiltinCalledMultipleTimesTextureNumLevels) {
    auto* t = b.Var(ty.ptr(
        handle, ty.Get<core::type::SampledTexture>(core::type::TextureDimension::k2d, ty.f32()),
        read_write));
    t->SetBindingPoint(0, 0);
    b.ir.root_block->Append(t);

    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    func->SetWorkgroupSize(1, 1, 1);
    b.Append(func->Block(), [&] {
        auto* tex = b.Load(t);
        b.Let("len", b.Call(ty.u32(), core::BuiltinFn::kTextureNumLevels, tex));
        b.Let("len2", b.Call(ty.u32(), core::BuiltinFn::kTextureNumLevels, tex));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<handle, texture_2d<f32>, read_write> = var @binding_point(0, 0)
}

%main = @compute @workgroup_size(1, 1, 1) func():void {
  $B2: {
    %3:texture_2d<f32> = load %1
    %4:u32 = textureNumLevels %3
    %len:u32 = let %4
    %6:u32 = textureNumLevels %3
    %len2:u32 = let %6
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    auto* expect = R"(
TintTextureUniformData = struct @align(4) {
  tint_builtin_value_0:u32 @offset(0)
}

$B1: {  # root
  %1:ptr<handle, texture_2d<f32>, read_write> = var @binding_point(0, 0)
  %2:ptr<uniform, TintTextureUniformData, read> = var @binding_point(0, 30)
}

%main = @compute @workgroup_size(1, 1, 1) func():void {
  $B2: {
    %4:texture_2d<f32> = load %1
    %5:ptr<uniform, u32, read> = access %2, 0u
    %6:u32 = load %5
    %len:u32 = let %6
    %8:ptr<uniform, u32, read> = access %2, 0u
    %9:u32 = load %8
    %len2:u32 = let %9
    ret
  }
}
)";

    TexturePolyfillConfig cfg;
    cfg.texture_builtins_from_uniform = {{0, 30u}, std::vector<BindingPoint>{{0, 0}}};
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, SameBuiltinCalledMultipleTimesTextureNumSamples) {
    auto* t = b.Var(ty.ptr(
        handle, ty.Get<core::type::DepthMultisampledTexture>(core::type::TextureDimension::k2d),
        read_write));
    t->SetBindingPoint(0, 0);
    b.ir.root_block->Append(t);

    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    func->SetWorkgroupSize(1, 1, 1);
    b.Append(func->Block(), [&] {
        auto* tex = b.Load(t);
        b.Let("len", b.Call(ty.u32(), core::BuiltinFn::kTextureNumSamples, tex));
        b.Let("len2", b.Call(ty.u32(), core::BuiltinFn::kTextureNumSamples, tex));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<handle, texture_depth_multisampled_2d, read_write> = var @binding_point(0, 0)
}

%main = @compute @workgroup_size(1, 1, 1) func():void {
  $B2: {
    %3:texture_depth_multisampled_2d = load %1
    %4:u32 = textureNumSamples %3
    %len:u32 = let %4
    %6:u32 = textureNumSamples %3
    %len2:u32 = let %6
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    auto* expect = R"(
TintTextureUniformData = struct @align(4) {
  tint_builtin_value_0:u32 @offset(0)
}

$B1: {  # root
  %1:ptr<handle, texture_depth_multisampled_2d, read_write> = var @binding_point(0, 0)
  %2:ptr<uniform, TintTextureUniformData, read> = var @binding_point(0, 30)
}

%main = @compute @workgroup_size(1, 1, 1) func():void {
  $B2: {
    %4:texture_depth_multisampled_2d = load %1
    %5:ptr<uniform, u32, read> = access %2, 0u
    %6:u32 = load %5
    %len:u32 = let %6
    %8:ptr<uniform, u32, read> = access %2, 0u
    %9:u32 = load %8
    %len2:u32 = let %9
    ret
  }
}
)";

    TexturePolyfillConfig cfg;
    cfg.texture_builtins_from_uniform = {{0, 30u}, std::vector<BindingPoint>{{0, 0}}};
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureAsFunctionParameter) {
    auto* t = b.Var(ty.ptr(
        handle, ty.Get<core::type::SampledTexture>(core::type::TextureDimension::k2d, ty.f32()),
        read_write));
    t->SetBindingPoint(0, 0);
    b.ir.root_block->Append(t);

    auto* p = b.FunctionParam(
        "t", ty.Get<core::type::SampledTexture>(core::type::TextureDimension::k2d, ty.f32()));
    auto* f = b.Function("f", ty.u32());
    f->SetParams({p});
    b.Append(f->Block(),
             [&] { b.Return(f, b.Call(ty.u32(), core::BuiltinFn::kTextureNumLevels, p)); });

    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    func->SetWorkgroupSize(1, 1, 1);
    b.Append(func->Block(), [&] {
        auto* tex = b.Load(t);
        b.Let("len", b.Call(ty.u32(), f, tex));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<handle, texture_2d<f32>, read_write> = var @binding_point(0, 0)
}

%f = func(%t:texture_2d<f32>):u32 {
  $B2: {
    %4:u32 = textureNumLevels %t
    ret %4
  }
}
%main = @compute @workgroup_size(1, 1, 1) func():void {
  $B3: {
    %6:texture_2d<f32> = load %1
    %7:u32 = call %f, %6
    %len:u32 = let %7
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    auto* expect = R"(
TintTextureUniformData = struct @align(4) {
  tint_builtin_value_0:u32 @offset(0)
}

$B1: {  # root
  %1:ptr<handle, texture_2d<f32>, read_write> = var @binding_point(0, 0)
  %2:ptr<uniform, TintTextureUniformData, read> = var @binding_point(0, 30)
}

%f = func(%t:texture_2d<f32>, %tint_tex_value:u32):u32 {
  $B2: {
    ret %tint_tex_value
  }
}
%main = @compute @workgroup_size(1, 1, 1) func():void {
  $B3: {
    %7:texture_2d<f32> = load %1
    %8:ptr<uniform, u32, read> = access %2, 0u
    %9:u32 = load %8
    %10:u32 = call %f, %7, %9
    %len:u32 = let %10
    ret
  }
}
)";

    TexturePolyfillConfig cfg;
    cfg.texture_builtins_from_uniform = {{0, 30u}, std::vector<BindingPoint>{{0, 0}}};
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureAsFunctionParameterUsedTwice) {
    auto* t = b.Var(ty.ptr(
        handle, ty.Get<core::type::SampledTexture>(core::type::TextureDimension::k2d, ty.f32()),
        read_write));
    t->SetBindingPoint(0, 0);
    b.ir.root_block->Append(t);

    auto* p = b.FunctionParam(
        "t", ty.Get<core::type::SampledTexture>(core::type::TextureDimension::k2d, ty.f32()));
    auto* f = b.Function("f", ty.u32());
    f->SetParams({p});
    b.Append(f->Block(), [&] {
        auto* val = b.Call(ty.u32(), core::BuiltinFn::kTextureNumLevels, p)->Result(0);
        val = b.Add(ty.u32(), val, b.Call(ty.u32(), core::BuiltinFn::kTextureNumLevels, p))
                  ->Result(0);
        b.Return(f, val);
    });

    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    func->SetWorkgroupSize(1, 1, 1);
    b.Append(func->Block(), [&] {
        auto* tex = b.Load(t);
        b.Let("len", b.Call(ty.u32(), f, tex));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<handle, texture_2d<f32>, read_write> = var @binding_point(0, 0)
}

%f = func(%t:texture_2d<f32>):u32 {
  $B2: {
    %4:u32 = textureNumLevels %t
    %5:u32 = textureNumLevels %t
    %6:u32 = add %4, %5
    ret %6
  }
}
%main = @compute @workgroup_size(1, 1, 1) func():void {
  $B3: {
    %8:texture_2d<f32> = load %1
    %9:u32 = call %f, %8
    %len:u32 = let %9
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    auto* expect = R"(
TintTextureUniformData = struct @align(4) {
  tint_builtin_value_0:u32 @offset(0)
}

$B1: {  # root
  %1:ptr<handle, texture_2d<f32>, read_write> = var @binding_point(0, 0)
  %2:ptr<uniform, TintTextureUniformData, read> = var @binding_point(0, 30)
}

%f = func(%t:texture_2d<f32>, %tint_tex_value:u32):u32 {
  $B2: {
    %6:u32 = add %tint_tex_value, %tint_tex_value
    ret %6
  }
}
%main = @compute @workgroup_size(1, 1, 1) func():void {
  $B3: {
    %8:texture_2d<f32> = load %1
    %9:ptr<uniform, u32, read> = access %2, 0u
    %10:u32 = load %9
    %11:u32 = call %f, %8, %10
    %len:u32 = let %11
    ret
  }
}
)";

    TexturePolyfillConfig cfg;
    cfg.texture_builtins_from_uniform = {{0, 30u}, std::vector<BindingPoint>{{0, 0}}};
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureAsFunctionParameterMultipleParameters) {
    auto* t1 = b.Var(ty.ptr(
        handle, ty.Get<core::type::SampledTexture>(core::type::TextureDimension::k2d, ty.f32()),
        read_write));
    t1->SetBindingPoint(0, 0);
    b.ir.root_block->Append(t1);

    auto* t2 = b.Var(ty.ptr(
        handle, ty.Get<core::type::SampledTexture>(core::type::TextureDimension::k2d, ty.f32()),
        read_write));
    t2->SetBindingPoint(0, 1);
    b.ir.root_block->Append(t2);

    auto* t3 = b.Var(ty.ptr(
        handle, ty.Get<core::type::SampledTexture>(core::type::TextureDimension::k2d, ty.f32()),
        read_write));
    t3->SetBindingPoint(0, 2);
    b.ir.root_block->Append(t3);

    auto* p1 = b.FunctionParam(
        "t1", ty.Get<core::type::SampledTexture>(core::type::TextureDimension::k2d, ty.f32()));
    auto* p2 = b.FunctionParam(
        "t2", ty.Get<core::type::SampledTexture>(core::type::TextureDimension::k2d, ty.f32()));
    auto* p3 = b.FunctionParam(
        "t3", ty.Get<core::type::SampledTexture>(core::type::TextureDimension::k2d, ty.f32()));
    auto* f = b.Function("f", ty.u32());
    f->SetParams({p1, p2, p3});
    b.Append(f->Block(), [&] {
        auto* v1 = b.Call(ty.u32(), core::BuiltinFn::kTextureNumLevels, p1)->Result(0);
        auto* v2 = b.Call(ty.u32(), core::BuiltinFn::kTextureNumLevels, p2)->Result(0);
        auto* v3 = b.Call(ty.u32(), core::BuiltinFn::kTextureNumLevels, p3)->Result(0);

        auto* val = b.Add(ty.u32(), v1, v2);
        val = b.Add(ty.u32(), val, v3);
        b.Return(f, val);
    });

    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    func->SetWorkgroupSize(1, 1, 1);
    b.Append(func->Block(), [&] {
        auto* tex1 = b.Load(t1);
        auto* tex2 = b.Load(t2);
        auto* tex3 = b.Load(t3);
        b.Let("len", b.Call(ty.u32(), f, tex1, tex2, tex3));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<handle, texture_2d<f32>, read_write> = var @binding_point(0, 0)
  %2:ptr<handle, texture_2d<f32>, read_write> = var @binding_point(0, 1)
  %3:ptr<handle, texture_2d<f32>, read_write> = var @binding_point(0, 2)
}

%f = func(%t1:texture_2d<f32>, %t2:texture_2d<f32>, %t3:texture_2d<f32>):u32 {
  $B2: {
    %8:u32 = textureNumLevels %t1
    %9:u32 = textureNumLevels %t2
    %10:u32 = textureNumLevels %t3
    %11:u32 = add %8, %9
    %12:u32 = add %11, %10
    ret %12
  }
}
%main = @compute @workgroup_size(1, 1, 1) func():void {
  $B3: {
    %14:texture_2d<f32> = load %1
    %15:texture_2d<f32> = load %2
    %16:texture_2d<f32> = load %3
    %17:u32 = call %f, %14, %15, %16
    %len:u32 = let %17
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    auto* expect = R"(
TintTextureUniformData = struct @align(4) {
  tint_builtin_value_0:u32 @offset(0)
  tint_builtin_value_1:u32 @offset(4)
  tint_builtin_value_2:u32 @offset(8)
}

$B1: {  # root
  %1:ptr<handle, texture_2d<f32>, read_write> = var @binding_point(0, 0)
  %2:ptr<handle, texture_2d<f32>, read_write> = var @binding_point(0, 1)
  %3:ptr<handle, texture_2d<f32>, read_write> = var @binding_point(0, 2)
  %4:ptr<uniform, TintTextureUniformData, read> = var @binding_point(0, 30)
}

%f = func(%t1:texture_2d<f32>, %t2:texture_2d<f32>, %t3:texture_2d<f32>, %tint_tex_value:u32, %tint_tex_value_1:u32, %tint_tex_value_2:u32):u32 {  # %tint_tex_value_1: 'tint_tex_value', %tint_tex_value_2: 'tint_tex_value'
  $B2: {
    %12:u32 = add %tint_tex_value, %tint_tex_value_1
    %13:u32 = add %12, %tint_tex_value_2
    ret %13
  }
}
%main = @compute @workgroup_size(1, 1, 1) func():void {
  $B3: {
    %15:texture_2d<f32> = load %1
    %16:texture_2d<f32> = load %2
    %17:texture_2d<f32> = load %3
    %18:ptr<uniform, u32, read> = access %4, 0u
    %19:u32 = load %18
    %20:ptr<uniform, u32, read> = access %4, 1u
    %21:u32 = load %20
    %22:ptr<uniform, u32, read> = access %4, 2u
    %23:u32 = load %22
    %24:u32 = call %f, %15, %16, %17, %19, %21, %23
    %len:u32 = let %24
    ret
  }
}
)";

    TexturePolyfillConfig cfg;
    cfg.texture_builtins_from_uniform = {{0, 30u},
                                         std::vector<BindingPoint>{{0, 0}, {0, 1}, {0, 2}}};
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureAsFunctionParameterNested) {
    auto* t = b.Var(ty.ptr(
        handle, ty.Get<core::type::SampledTexture>(core::type::TextureDimension::k2d, ty.f32()),
        read_write));
    t->SetBindingPoint(0, 0);
    b.ir.root_block->Append(t);

    auto* p2 = b.FunctionParam(
        "t2", ty.Get<core::type::SampledTexture>(core::type::TextureDimension::k2d, ty.f32()));
    auto* f2 = b.Function("f2", ty.u32());
    f2->SetParams({p2});
    b.Append(f2->Block(),
             [&] { b.Return(f2, b.Call(ty.u32(), core::BuiltinFn::kTextureNumLevels, p2)); });

    auto* p1 = b.FunctionParam(
        "t1", ty.Get<core::type::SampledTexture>(core::type::TextureDimension::k2d, ty.f32()));
    auto* f1 = b.Function("f1", ty.u32());
    f1->SetParams({p1});
    b.Append(f1->Block(), [&] { b.Return(f1, b.Call(ty.u32(), f2, p1)); });

    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    func->SetWorkgroupSize(1, 1, 1);
    b.Append(func->Block(), [&] {
        auto* tex = b.Load(t);
        b.Let("len", b.Call(ty.u32(), f1, tex));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<handle, texture_2d<f32>, read_write> = var @binding_point(0, 0)
}

%f2 = func(%t2:texture_2d<f32>):u32 {
  $B2: {
    %4:u32 = textureNumLevels %t2
    ret %4
  }
}
%f1 = func(%t1:texture_2d<f32>):u32 {
  $B3: {
    %7:u32 = call %f2, %t1
    ret %7
  }
}
%main = @compute @workgroup_size(1, 1, 1) func():void {
  $B4: {
    %9:texture_2d<f32> = load %1
    %10:u32 = call %f1, %9
    %len:u32 = let %10
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    auto* expect = R"(
TintTextureUniformData = struct @align(4) {
  tint_builtin_value_0:u32 @offset(0)
}

$B1: {  # root
  %1:ptr<handle, texture_2d<f32>, read_write> = var @binding_point(0, 0)
  %2:ptr<uniform, TintTextureUniformData, read> = var @binding_point(0, 30)
}

%f2 = func(%t2:texture_2d<f32>, %tint_tex_value:u32):u32 {
  $B2: {
    ret %tint_tex_value
  }
}
%f1 = func(%t1:texture_2d<f32>, %tint_tex_value_1:u32):u32 {  # %tint_tex_value_1: 'tint_tex_value'
  $B3: {
    %9:u32 = call %f2, %t1, %tint_tex_value_1
    ret %9
  }
}
%main = @compute @workgroup_size(1, 1, 1) func():void {
  $B4: {
    %11:texture_2d<f32> = load %1
    %12:ptr<uniform, u32, read> = access %2, 0u
    %13:u32 = load %12
    %14:u32 = call %f1, %11, %13
    %len:u32 = let %14
    ret
  }
}
)";

    TexturePolyfillConfig cfg;
    cfg.texture_builtins_from_uniform = {{0, 30u}, std::vector<BindingPoint>{{0, 0}}};
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_TexturePolyfillTest, TextureAsFunctionParameterMixed) {
    auto* t1 = b.Var(ty.ptr(
        handle, ty.Get<core::type::SampledTexture>(core::type::TextureDimension::k2d, ty.f32()),
        read_write));
    t1->SetBindingPoint(0, 0);
    b.ir.root_block->Append(t1);
    auto* t2 = b.Var(ty.ptr(
        handle, ty.Get<core::type::SampledTexture>(core::type::TextureDimension::k2d, ty.f32()),
        read_write));
    t2->SetBindingPoint(0, 1);
    b.ir.root_block->Append(t2);
    auto* t3 = b.Var(ty.ptr(
        handle, ty.Get<core::type::SampledTexture>(core::type::TextureDimension::k2d, ty.f32()),
        read_write));
    t3->SetBindingPoint(0, 2);
    b.ir.root_block->Append(t3);
    auto* t4 = b.Var(ty.ptr(
        handle, ty.Get<core::type::SampledTexture>(core::type::TextureDimension::k2d, ty.f32()),
        read_write));
    t4->SetBindingPoint(0, 3);
    b.ir.root_block->Append(t4);
    auto* t5 = b.Var(
        ty.ptr(handle,
               ty.Get<core::type::SampledTexture>(core::type::TextureDimension::k2dArray, ty.f32()),
               read_write));
    t5->SetBindingPoint(0, 4);
    b.ir.root_block->Append(t5);

    auto* p_nested1 = b.FunctionParam(
        "t1", ty.Get<core::type::SampledTexture>(core::type::TextureDimension::k2d, ty.f32()));
    auto* p_nested2 = b.FunctionParam(
        "t2", ty.Get<core::type::SampledTexture>(core::type::TextureDimension::k2d, ty.f32()));
    auto* f_nested = b.Function("f_nested", ty.u32());
    f_nested->SetParams({p_nested1, p_nested2});
    b.Append(f_nested->Block(), [&] {
        auto* v1 = b.Call(ty.u32(), core::BuiltinFn::kTextureNumLevels, p_nested1);
        auto* v2 = b.Call(ty.u32(), core::BuiltinFn::kTextureNumLevels, p_nested2);
        b.Return(f_nested, b.Add(ty.u32(), v1, v2));
    });

    auto* a = b.FunctionParam("a", ty.u32());
    auto* t = b.FunctionParam(
        "t", ty.Get<core::type::SampledTexture>(core::type::TextureDimension::k2d, ty.f32()));
    auto* f1 = b.Function("f1", ty.u32());
    f1->SetParams({a, t});
    b.Append(f1->Block(), [&] {
        auto* val1 = b.Call(ty.u32(), f_nested, t, b.Load(t1));
        auto* val2 = b.Call(ty.u32(), core::BuiltinFn::kTextureNumLevels, b.Load(t3));
        auto* add = b.Add(ty.u32(), a, val1);
        b.Return(f1, b.Add(ty.u32(), add, val2));
    });

    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    func->SetWorkgroupSize(1, 1, 1);
    b.Append(func->Block(), [&] {
        auto* tex5 = b.Load(t5);
        b.Let("m", b.Call(ty.u32(), core::BuiltinFn::kTextureNumLayers, tex5));

        auto* tex1 = b.Load(t1);
        b.Let("n", b.Call(ty.u32(), f1, 9_u, tex1));

        auto* tex2 = b.Load(t2);
        b.Let("o", b.Call(ty.u32(), f_nested, tex2, tex2));

        auto* tex4 = b.Load(t4);
        b.Let("p", b.Call(ty.u32(), f_nested, tex1, tex4));

        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<handle, texture_2d<f32>, read_write> = var @binding_point(0, 0)
  %2:ptr<handle, texture_2d<f32>, read_write> = var @binding_point(0, 1)
  %3:ptr<handle, texture_2d<f32>, read_write> = var @binding_point(0, 2)
  %4:ptr<handle, texture_2d<f32>, read_write> = var @binding_point(0, 3)
  %5:ptr<handle, texture_2d_array<f32>, read_write> = var @binding_point(0, 4)
}

%f_nested = func(%t1:texture_2d<f32>, %t2:texture_2d<f32>):u32 {
  $B2: {
    %9:u32 = textureNumLevels %t1
    %10:u32 = textureNumLevels %t2
    %11:u32 = add %9, %10
    ret %11
  }
}
%f1 = func(%a:u32, %t:texture_2d<f32>):u32 {
  $B3: {
    %15:texture_2d<f32> = load %1
    %16:u32 = call %f_nested, %t, %15
    %17:texture_2d<f32> = load %3
    %18:u32 = textureNumLevels %17
    %19:u32 = add %a, %16
    %20:u32 = add %19, %18
    ret %20
  }
}
%main = @compute @workgroup_size(1, 1, 1) func():void {
  $B4: {
    %22:texture_2d_array<f32> = load %5
    %23:u32 = textureNumLayers %22
    %m:u32 = let %23
    %25:texture_2d<f32> = load %1
    %26:u32 = call %f1, 9u, %25
    %n:u32 = let %26
    %28:texture_2d<f32> = load %2
    %29:u32 = call %f_nested, %28, %28
    %o:u32 = let %29
    %31:texture_2d<f32> = load %4
    %32:u32 = call %f_nested, %25, %31
    %p:u32 = let %32
    ret
  }
}
)";

    ASSERT_EQ(src, str());

    auto* expect = R"(
TintTextureUniformData = struct @align(4) {
  tint_builtin_value_0:u32 @offset(0)
  tint_builtin_value_1:u32 @offset(4)
  tint_builtin_value_2:u32 @offset(8)
  tint_builtin_value_3:u32 @offset(12)
}

$B1: {  # root
  %1:ptr<handle, texture_2d<f32>, read_write> = var @binding_point(0, 0)
  %2:ptr<handle, texture_2d<f32>, read_write> = var @binding_point(0, 1)
  %3:ptr<handle, texture_2d<f32>, read_write> = var @binding_point(0, 2)
  %4:ptr<handle, texture_2d<f32>, read_write> = var @binding_point(0, 3)
  %5:ptr<handle, texture_2d_array<f32>, read_write> = var @binding_point(0, 4)
  %6:ptr<uniform, TintTextureUniformData, read> = var @binding_point(0, 30)
}

%f_nested = func(%t1:texture_2d<f32>, %t2:texture_2d<f32>, %tint_tex_value:u32, %tint_tex_value_1:u32):u32 {  # %tint_tex_value_1: 'tint_tex_value'
  $B2: {
    %12:u32 = add %tint_tex_value, %tint_tex_value_1
    ret %12
  }
}
%f1 = func(%a:u32, %t:texture_2d<f32>, %tint_tex_value_2:u32):u32 {  # %tint_tex_value_2: 'tint_tex_value'
  $B3: {
    %17:texture_2d<f32> = load %1
    %18:ptr<uniform, u32, read> = access %6, 2u
    %19:u32 = load %18
    %20:u32 = call %f_nested, %t, %17, %tint_tex_value_2, %19
    %21:texture_2d<f32> = load %3
    %22:ptr<uniform, u32, read> = access %6, 3u
    %23:u32 = load %22
    %24:u32 = add %a, %20
    %25:u32 = add %24, %23
    ret %25
  }
}
%main = @compute @workgroup_size(1, 1, 1) func():void {
  $B4: {
    %27:texture_2d_array<f32> = load %5
    %28:vec3<i32> = glsl.textureSize %27, 0i
    %29:i32 = swizzle %28, z
    %30:u32 = bitcast %29
    %m:u32 = let %30
    %32:texture_2d<f32> = load %1
    %33:ptr<uniform, u32, read> = access %6, 2u
    %34:u32 = load %33
    %35:u32 = call %f1, 9u, %32, %34
    %n:u32 = let %35
    %37:texture_2d<f32> = load %2
    %38:ptr<uniform, u32, read> = access %6, 0u
    %39:u32 = load %38
    %40:ptr<uniform, u32, read> = access %6, 0u
    %41:u32 = load %40
    %42:u32 = call %f_nested, %37, %37, %39, %41
    %o:u32 = let %42
    %44:texture_2d<f32> = load %4
    %45:ptr<uniform, u32, read> = access %6, 2u
    %46:u32 = load %45
    %47:ptr<uniform, u32, read> = access %6, 1u
    %48:u32 = load %47
    %49:u32 = call %f_nested, %32, %44, %46, %48
    %p:u32 = let %49
    ret
  }
}
)";

    TexturePolyfillConfig cfg;
    cfg.texture_builtins_from_uniform = {{0, 30u},
                                         std::vector<BindingPoint>{{0, 1}, {0, 3}, {0, 0}, {0, 2}}};
    Run(TexturePolyfill, cfg);
    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::glsl::writer::raise
