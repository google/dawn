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

#include "src/tint/lang/glsl/writer/raise/builtin_polyfill.h"

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

namespace tint::glsl::writer::raise {
namespace {

using GlslWriter_BuiltinPolyfillTest = core::ir::transform::TransformTest;

TEST_F(GlslWriter_BuiltinPolyfillTest, SelectScalar) {
    auto* func = b.Function("foo", ty.f32());
    b.Append(func->Block(),
             [&] { b.Return(func, b.Call<f32>(core::BuiltinFn::kSelect, 2_f, 1_f, false)); });

    auto* src = R"(
%foo = func():f32 {
  $B1: {
    %2:f32 = select 2.0f, 1.0f, false
    ret %2
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func():f32 {
  $B1: {
    %2:f32 = glsl.ternary 2.0f, 1.0f, false
    ret %2
  }
}
)";

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, SelectVector) {
    auto* func = b.Function("foo", ty.vec3<f32>());
    b.Append(func->Block(), [&] {
        auto* false_ = b.Splat(ty.vec3<f32>(), 2_f);
        auto* true_ = b.Splat(ty.vec3<f32>(), 1_f);
        auto* cond = b.Splat(ty.vec3<bool>(), false);
        b.Return(func, b.Call<vec3<f32>>(core::BuiltinFn::kSelect, false_, true_, cond));
    });

    auto* src = R"(
%foo = func():vec3<f32> {
  $B1: {
    %2:vec3<f32> = select vec3<f32>(2.0f), vec3<f32>(1.0f), vec3<bool>(false)
    ret %2
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func():vec3<f32> {
  $B1: {
    %2:f32 = swizzle vec3<f32>(2.0f), x
    %3:f32 = swizzle vec3<f32>(1.0f), x
    %4:f32 = swizzle vec3<bool>(false), x
    %5:f32 = glsl.ternary %2, %3, %4
    %6:f32 = swizzle vec3<f32>(2.0f), y
    %7:f32 = swizzle vec3<f32>(1.0f), y
    %8:f32 = swizzle vec3<bool>(false), y
    %9:f32 = glsl.ternary %6, %7, %8
    %10:f32 = swizzle vec3<f32>(2.0f), z
    %11:f32 = swizzle vec3<f32>(1.0f), z
    %12:f32 = swizzle vec3<bool>(false), z
    %13:f32 = glsl.ternary %10, %11, %12
    %14:vec3<f32> = construct %5, %9, %13
    ret %14
  }
}
)";

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, StorageBarrier) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    func->SetWorkgroupSize(1, 1, 1);
    b.Append(func->Block(), [&] {
        b.Call(ty.void_(), core::BuiltinFn::kStorageBarrier);
        b.Return(func);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(1, 1, 1) func():void {
  $B1: {
    %2:void = storageBarrier
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @compute @workgroup_size(1, 1, 1) func():void {
  $B1: {
    %2:void = glsl.barrier
    %3:void = glsl.memoryBarrierBuffer
    ret
  }
}
)";

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, TextureBarrier) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    func->SetWorkgroupSize(1, 1, 1);
    b.Append(func->Block(), [&] {
        b.Call(ty.void_(), core::BuiltinFn::kTextureBarrier);
        b.Return(func);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(1, 1, 1) func():void {
  $B1: {
    %2:void = textureBarrier
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @compute @workgroup_size(1, 1, 1) func():void {
  $B1: {
    %2:void = glsl.barrier
    %3:void = glsl.memoryBarrierImage
    ret
  }
}
)";

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, WorkgroupBarrier) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    func->SetWorkgroupSize(1, 1, 1);
    b.Append(func->Block(), [&] {
        b.Call(ty.void_(), core::BuiltinFn::kWorkgroupBarrier);
        b.Return(func);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(1, 1, 1) func():void {
  $B1: {
    %2:void = workgroupBarrier
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @compute @workgroup_size(1, 1, 1) func():void {
  $B1: {
    %2:void = glsl.barrier
    ret
  }
}
)";

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, AtomicCompareExchangeWeak) {
    auto* var = b.Var("v", workgroup, ty.atomic<i32>(), core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    func->SetWorkgroupSize(1, 1, 1);
    b.Append(func->Block(), [&] {
        b.Let("x", b.Call(core::type::CreateAtomicCompareExchangeResult(ty, mod.symbols, ty.i32()),
                          core::BuiltinFn::kAtomicCompareExchangeWeak, var, 123_i, 345_i));
        b.Return(func);
    });

    auto* src = R"(
__atomic_compare_exchange_result_i32 = struct @align(4) {
  old_value:i32 @offset(0)
  exchanged:bool @offset(4)
}

$B1: {  # root
  %v:ptr<workgroup, atomic<i32>, read_write> = var @binding_point(0, 0)
}

%foo = @compute @workgroup_size(1, 1, 1) func():void {
  $B2: {
    %3:__atomic_compare_exchange_result_i32 = atomicCompareExchangeWeak %v, 123i, 345i
    %x:__atomic_compare_exchange_result_i32 = let %3
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
__atomic_compare_exchange_result_i32 = struct @align(4) {
  old_value:i32 @offset(0)
  exchanged:bool @offset(4)
}

$B1: {  # root
  %v:ptr<workgroup, atomic<i32>, read_write> = var @binding_point(0, 0)
}

%foo = @compute @workgroup_size(1, 1, 1) func():void {
  $B2: {
    %3:i32 = bitcast 123i
    %4:i32 = bitcast 345i
    %5:i32 = glsl.atomicCompSwap %v, %3, %4
    %6:bool = eq %5, 123i
    %7:__atomic_compare_exchange_result_i32 = construct %5, %6
    %x:__atomic_compare_exchange_result_i32 = let %7
    ret
  }
}
)";

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, AtomicSub) {
    auto* var = b.Var("v", workgroup, ty.atomic<i32>(), core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    func->SetWorkgroupSize(1, 1, 1);
    b.Append(func->Block(), [&] {
        b.Let("x", b.Call(ty.i32(), core::BuiltinFn::kAtomicSub, var, 123_i));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<workgroup, atomic<i32>, read_write> = var @binding_point(0, 0)
}

%foo = @compute @workgroup_size(1, 1, 1) func():void {
  $B2: {
    %3:i32 = atomicSub %v, 123i
    %x:i32 = let %3
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:ptr<workgroup, atomic<i32>, read_write> = var @binding_point(0, 0)
}

%foo = @compute @workgroup_size(1, 1, 1) func():void {
  $B2: {
    %3:i32 = negation 123i
    %4:i32 = atomicAdd %v, %3
    %x:i32 = let %4
    ret
  }
}
)";

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, AtomicSub_u32) {
    auto* var = b.Var("v", workgroup, ty.atomic<u32>(), core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    func->SetWorkgroupSize(1, 1, 1);
    b.Append(func->Block(), [&] {
        b.Let("x", b.Call(ty.u32(), core::BuiltinFn::kAtomicSub, var, 123_u));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<workgroup, atomic<u32>, read_write> = var @binding_point(0, 0)
}

%foo = @compute @workgroup_size(1, 1, 1) func():void {
  $B2: {
    %3:u32 = atomicSub %v, 123u
    %x:u32 = let %3
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:ptr<workgroup, atomic<u32>, read_write> = var @binding_point(0, 0)
}

%foo = @compute @workgroup_size(1, 1, 1) func():void {
  $B2: {
    %3:u32 = glsl.atomicSub %v, 123u
    %x:u32 = let %3
    ret
  }
}
)";

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, AtomicLoad) {
    auto* var = b.Var("v", workgroup, ty.atomic<i32>(), core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    func->SetWorkgroupSize(1, 1, 1);
    b.Append(func->Block(), [&] {
        b.Let("x", b.Call(ty.i32(), core::BuiltinFn::kAtomicLoad, var));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<workgroup, atomic<i32>, read_write> = var @binding_point(0, 0)
}

%foo = @compute @workgroup_size(1, 1, 1) func():void {
  $B2: {
    %3:i32 = atomicLoad %v
    %x:i32 = let %3
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:ptr<workgroup, atomic<i32>, read_write> = var @binding_point(0, 0)
}

%foo = @compute @workgroup_size(1, 1, 1) func():void {
  $B2: {
    %3:i32 = atomicOr %v, 0i
    %x:i32 = let %3
    ret
  }
}
)";

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, TextureDimensions_1d) {
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
    %3:i32 = glsl.textureSize %t, 0i
    %4:u32 = bitcast %3
    ret %4
  }
}
)";

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, TextureDimensions_2d_WithoutLod) {
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

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, TextureDimensions_2d_WithU32Lod) {
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

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, TextureDimensions_2dArray) {
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

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, TextureDimensions_Storage2D) {
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

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, TextureDimensions_DepthMultisampled) {
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

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, CountOneBits) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("x", b.Call(ty.u32(), core::BuiltinFn::kCountOneBits, 1_u));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %2:u32 = countOneBits 1u
    %x:u32 = let %2
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %2:i32 = glsl.bitCount 1u
    %3:u32 = convert %2
    %x:u32 = let %3
    ret
  }
}
)";

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, ExtractBits) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("x", b.Call(ty.u32(), core::BuiltinFn::kExtractBits, 1_u, 2_u, 3_u));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %2:u32 = extractBits 1u, 2u, 3u
    %x:u32 = let %2
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %2:i32 = convert 2u
    %3:i32 = convert 3u
    %4:u32 = glsl.bitfieldExtract 1u, %2, %3
    %x:u32 = let %4
    ret
  }
}
)";

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, InsertBits) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("x", b.Call(ty.u32(), core::BuiltinFn::kInsertBits, 1_u, 2_u, 3_u, 4_u));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %2:u32 = insertBits 1u, 2u, 3u, 4u
    %x:u32 = let %2
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %2:i32 = convert 3u
    %3:i32 = convert 4u
    %4:u32 = glsl.bitfieldInsert 1u, 2u, %2, %3
    %x:u32 = let %4
    ret
  }
}
)";

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, TextureNumLayers_2DArray) {
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

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, TextureNumLayers_Depth2DArray) {
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

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, TextureNumLayers_CubeArray) {
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

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, TextureNumLayers_DepthCubeArray) {
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

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, TextureNumLayers_Storage2DArray) {
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

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, TextureLoad_1DF32) {
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
    %5:vec4<f32> = glsl.texelFetch %t, %3, %4
    ret %5
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, TextureLoad_2DLevelI32) {
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

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, TextureLoad_3DLevelU32) {
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

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, TextureLoad_Multisampled2DI32) {
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

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, TextureLoad_Storage2D) {
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

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, TextureStore1D) {
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
    %4:void = glsl.imageStore %3, 1i, vec4<f32>(0.5f, 0.0f, 0.0f, 1.0f)
    ret
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, TextureStore3D) {
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

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, TextureStoreArray) {
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

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, FMA_f32) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* x = b.Splat(ty.vec3<f32>(), 1_f);
        auto* y = b.Splat(ty.vec3<f32>(), 2_f);
        auto* z = b.Splat(ty.vec3<f32>(), 3_f);

        b.Let("x", b.Call(ty.vec3<f32>(), core::BuiltinFn::kFma, x, y, z));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %2:vec3<f32> = fma vec3<f32>(1.0f), vec3<f32>(2.0f), vec3<f32>(3.0f)
    %x:vec3<f32> = let %2
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %2:vec3<f32> = mul vec3<f32>(1.0f), vec3<f32>(2.0f)
    %3:vec3<f32> = add %2, vec3<f32>(3.0f)
    %x:vec3<f32> = let %3
    ret
  }
}
)";

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, FMA_f16) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* x = b.Splat(ty.vec3<f16>(), 1_h);
        auto* y = b.Splat(ty.vec3<f16>(), 2_h);
        auto* z = b.Splat(ty.vec3<f16>(), 3_h);

        b.Let("x", b.Call(ty.vec3<f16>(), core::BuiltinFn::kFma, x, y, z));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %2:vec3<f16> = fma vec3<f16>(1.0h), vec3<f16>(2.0h), vec3<f16>(3.0h)
    %x:vec3<f16> = let %2
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %2:vec3<f16> = mul vec3<f16>(1.0h), vec3<f16>(2.0h)
    %3:vec3<f16> = add %2, vec3<f16>(3.0h)
    %x:vec3<f16> = let %3
    ret
  }
}
)";

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, ArrayLength) {
    auto* sb = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("b"), ty.array<u32>()},
                                                });

    auto* var = b.Var("v", storage, sb, core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* ary = b.Access(ty.ptr<storage, array<u32>, read_write>(), var, 0_u);
        b.Let("x", b.Call(ty.u32(), core::BuiltinFn::kArrayLength, ary));
        b.Return(func);
    });

    auto* src = R"(
SB = struct @align(4) {
  b:array<u32> @offset(0)
}

$B1: {  # root
  %v:ptr<storage, SB, read_write> = var @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<storage, array<u32>, read_write> = access %v, 0u
    %4:u32 = arrayLength %3
    %x:u32 = let %4
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
SB = struct @align(4) {
  b:array<u32> @offset(0)
}

$B1: {  # root
  %v:ptr<storage, SB, read_write> = var @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<storage, array<u32>, read_write> = access %v, 0u
    %4:i32 = %3.length
    %5:u32 = convert %4
    %x:u32 = let %5
    ret
  }
}
)";

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, AnyScalar) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("x", b.Call(ty.bool_(), core::BuiltinFn::kAny, true));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %2:bool = any true
    %x:bool = let %2
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %x:bool = let true
    ret
  }
}
)";

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, AllScalar) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("x", b.Call(ty.bool_(), core::BuiltinFn::kAll, false));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %2:bool = all false
    %x:bool = let %2
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %x:bool = let false
    ret
  }
}
)";

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::glsl::writer::raise
