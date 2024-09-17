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
    %3:i32 = glsl.atomicCompSwap %v, 123i, 345i
    %4:bool = eq %3, 123i
    %5:__atomic_compare_exchange_result_i32 = construct %3, %4
    %x:__atomic_compare_exchange_result_i32 = let %5
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

TEST_F(GlslWriter_BuiltinPolyfillTest, BitcastFloatToFloat) {
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
    %3:f32 = bitcast %a
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

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, BitcastIntToFloat) {
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
    %3:f32 = bitcast %a
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

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, BitcastUintToFloat) {
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
    %3:f32 = bitcast %a
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

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, BitcastVec3UintToVec3Float) {
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
    %3:vec3<f32> = bitcast %a
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

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, BitcastFloatToInt) {
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
    %3:i32 = bitcast %a
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

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, BitcastFloatToUint) {
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
    %3:u32 = bitcast %a
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

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, BitcastUintToInt) {
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
    %3:i32 = bitcast %a
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

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, BitcastIntToUint) {
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
    %3:u32 = bitcast %a
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

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, BitcastI32ToVec2F16) {
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
    %3:vec2<f16> = bitcast %a
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
    %3:vec2<f16> = call %tint_bitcast_to_f16, %a
    %x:vec2<f16> = let %3
    ret
  }
}
%tint_bitcast_to_f16 = func(%src:i32):vec2<f16> {
  $B2: {
    %7:u32 = convert %src
    %8:vec2<f16> = glsl.unpackFloat2x16 %7
    ret %8
  }
}
)";

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, BitcastVec2F16ToI32) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Let("a", b.Construct<vec2<f16>>(1_h, 2_h));
        b.Let("x", b.Bitcast<i32>(a));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %2:vec2<f16> = construct 1.0h, 2.0h
    %a:vec2<f16> = let %2
    %4:i32 = bitcast %a
    %x:i32 = let %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %2:vec2<f16> = construct 1.0h, 2.0h
    %a:vec2<f16> = let %2
    %4:i32 = call %tint_bitcast_from_f16, %a
    %x:i32 = let %4
    ret
  }
}
%tint_bitcast_from_f16 = func(%src:vec2<f16>):i32 {
  $B2: {
    %8:u32 = glsl.packFloat2x16 %src
    %9:i32 = convert %8
    ret %9
  }
}
)";

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, BitcastU32ToVec2F16) {
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
    %3:vec2<f16> = bitcast %a
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
    %3:vec2<f16> = call %tint_bitcast_to_f16, %a
    %x:vec2<f16> = let %3
    ret
  }
}
%tint_bitcast_to_f16 = func(%src:u32):vec2<f16> {
  $B2: {
    %7:u32 = convert %src
    %8:vec2<f16> = glsl.unpackFloat2x16 %7
    ret %8
  }
}
)";

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, BitcastVec2F16ToU32) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Let("a", b.Construct<vec2<f16>>(1_h, 2_h));
        b.Let("x", b.Bitcast<u32>(a));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %2:vec2<f16> = construct 1.0h, 2.0h
    %a:vec2<f16> = let %2
    %4:u32 = bitcast %a
    %x:u32 = let %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %2:vec2<f16> = construct 1.0h, 2.0h
    %a:vec2<f16> = let %2
    %4:u32 = call %tint_bitcast_from_f16, %a
    %x:u32 = let %4
    ret
  }
}
%tint_bitcast_from_f16 = func(%src:vec2<f16>):u32 {
  $B2: {
    %8:u32 = glsl.packFloat2x16 %src
    %9:u32 = convert %8
    ret %9
  }
}
)";

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, BitcastF32ToVec2F16) {
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
    %3:vec2<f16> = bitcast %a
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
    %3:vec2<f16> = call %tint_bitcast_to_f16, %a
    %x:vec2<f16> = let %3
    ret
  }
}
%tint_bitcast_to_f16 = func(%src:f32):vec2<f16> {
  $B2: {
    %7:u32 = glsl.floatBitsToUint %src
    %8:vec2<f16> = glsl.unpackFloat2x16 %7
    ret %8
  }
}
)";

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, BitcastVec2F16ToF32) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Let("a", b.Construct<vec2<f16>>(1_h, 2_h));
        b.Let("x", b.Bitcast<f32>(a));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %2:vec2<f16> = construct 1.0h, 2.0h
    %a:vec2<f16> = let %2
    %4:f32 = bitcast %a
    %x:f32 = let %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %2:vec2<f16> = construct 1.0h, 2.0h
    %a:vec2<f16> = let %2
    %4:f32 = call %tint_bitcast_from_f16, %a
    %x:f32 = let %4
    ret
  }
}
%tint_bitcast_from_f16 = func(%src:vec2<f16>):f32 {
  $B2: {
    %8:u32 = glsl.packFloat2x16 %src
    %9:f32 = glsl.uintBitsToFloat %8
    ret %9
  }
}
)";

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, BitcastVec2I32ToVec4F16) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Let("a", b.Construct<vec2<i32>>(1_i, 2_i));
        b.Let("x", b.Bitcast<vec4<f16>>(a));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %2:vec2<i32> = construct 1i, 2i
    %a:vec2<i32> = let %2
    %4:vec4<f16> = bitcast %a
    %x:vec4<f16> = let %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %2:vec2<i32> = construct 1i, 2i
    %a:vec2<i32> = let %2
    %4:vec4<f16> = call %tint_bitcast_to_f16, %a
    %x:vec4<f16> = let %4
    ret
  }
}
%tint_bitcast_to_f16 = func(%src:vec2<i32>):vec4<f16> {
  $B2: {
    %8:vec2<u32> = convert %src
    %9:u32 = swizzle %8, x
    %10:vec2<f16> = glsl.unpackFloat2x16 %9
    %11:u32 = swizzle %8, y
    %12:vec2<f16> = glsl.unpackFloat2x16 %11
    %13:vec4<f16> = construct %10, %12
    ret %13
  }
}
)";

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, BitcastVec4F16ToVec2I32) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Let("a", b.Construct<vec4<f16>>(1_h, 2_h, 3_h, 4_h));
        b.Let("x", b.Bitcast<vec2<i32>>(a));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %2:vec4<f16> = construct 1.0h, 2.0h, 3.0h, 4.0h
    %a:vec4<f16> = let %2
    %4:vec2<i32> = bitcast %a
    %x:vec2<i32> = let %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %2:vec4<f16> = construct 1.0h, 2.0h, 3.0h, 4.0h
    %a:vec4<f16> = let %2
    %4:vec2<i32> = call %tint_bitcast_from_f16, %a
    %x:vec2<i32> = let %4
    ret
  }
}
%tint_bitcast_from_f16 = func(%src:vec4<f16>):vec2<i32> {
  $B2: {
    %8:vec2<f16> = swizzle %src, xy
    %9:u32 = glsl.packFloat2x16 %8
    %10:vec2<f16> = swizzle %src, zw
    %11:u32 = glsl.packFloat2x16 %10
    %12:vec2<u32> = construct %9, %11
    %13:vec2<i32> = convert %12
    ret %13
  }
}
)";

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, BitcastVec2U32ToVec4F16) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Let("a", b.Construct<vec2<u32>>(1_u, 2_u));
        b.Let("x", b.Bitcast<vec4<f16>>(a));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %2:vec2<u32> = construct 1u, 2u
    %a:vec2<u32> = let %2
    %4:vec4<f16> = bitcast %a
    %x:vec4<f16> = let %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %2:vec2<u32> = construct 1u, 2u
    %a:vec2<u32> = let %2
    %4:vec4<f16> = call %tint_bitcast_to_f16, %a
    %x:vec4<f16> = let %4
    ret
  }
}
%tint_bitcast_to_f16 = func(%src:vec2<u32>):vec4<f16> {
  $B2: {
    %8:vec2<u32> = convert %src
    %9:u32 = swizzle %8, x
    %10:vec2<f16> = glsl.unpackFloat2x16 %9
    %11:u32 = swizzle %8, y
    %12:vec2<f16> = glsl.unpackFloat2x16 %11
    %13:vec4<f16> = construct %10, %12
    ret %13
  }
}
)";

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, BitcastVec4F16ToVec2U32) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Let("a", b.Construct<vec4<f16>>(1_h, 2_h, 3_h, 4_h));
        b.Let("x", b.Bitcast<vec2<u32>>(a));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %2:vec4<f16> = construct 1.0h, 2.0h, 3.0h, 4.0h
    %a:vec4<f16> = let %2
    %4:vec2<u32> = bitcast %a
    %x:vec2<u32> = let %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %2:vec4<f16> = construct 1.0h, 2.0h, 3.0h, 4.0h
    %a:vec4<f16> = let %2
    %4:vec2<u32> = call %tint_bitcast_from_f16, %a
    %x:vec2<u32> = let %4
    ret
  }
}
%tint_bitcast_from_f16 = func(%src:vec4<f16>):vec2<u32> {
  $B2: {
    %8:vec2<f16> = swizzle %src, xy
    %9:u32 = glsl.packFloat2x16 %8
    %10:vec2<f16> = swizzle %src, zw
    %11:u32 = glsl.packFloat2x16 %10
    %12:vec2<u32> = construct %9, %11
    %13:vec2<u32> = convert %12
    ret %13
  }
}
)";

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, BitcastVec2F32ToVec4F16) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Let("a", b.Construct<vec2<f32>>(1_f, 2_f));
        b.Let("x", b.Bitcast<vec4<f16>>(a));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %2:vec2<f32> = construct 1.0f, 2.0f
    %a:vec2<f32> = let %2
    %4:vec4<f16> = bitcast %a
    %x:vec4<f16> = let %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %2:vec2<f32> = construct 1.0f, 2.0f
    %a:vec2<f32> = let %2
    %4:vec4<f16> = call %tint_bitcast_to_f16, %a
    %x:vec4<f16> = let %4
    ret
  }
}
%tint_bitcast_to_f16 = func(%src:vec2<f32>):vec4<f16> {
  $B2: {
    %8:vec2<u32> = glsl.floatBitsToUint %src
    %9:u32 = swizzle %8, x
    %10:vec2<f16> = glsl.unpackFloat2x16 %9
    %11:u32 = swizzle %8, y
    %12:vec2<f16> = glsl.unpackFloat2x16 %11
    %13:vec4<f16> = construct %10, %12
    ret %13
  }
}
)";

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, BitcastVec4F16ToVec2F32) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Let("a", b.Construct<vec4<f16>>(1_h, 2_h, 3_h, 4_h));
        b.Let("x", b.Bitcast<vec2<f32>>(a));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %2:vec4<f16> = construct 1.0h, 2.0h, 3.0h, 4.0h
    %a:vec4<f16> = let %2
    %4:vec2<f32> = bitcast %a
    %x:vec2<f32> = let %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %2:vec4<f16> = construct 1.0h, 2.0h, 3.0h, 4.0h
    %a:vec4<f16> = let %2
    %4:vec2<f32> = call %tint_bitcast_from_f16, %a
    %x:vec2<f32> = let %4
    ret
  }
}
%tint_bitcast_from_f16 = func(%src:vec4<f16>):vec2<f32> {
  $B2: {
    %8:vec2<f16> = swizzle %src, xy
    %9:u32 = glsl.packFloat2x16 %8
    %10:vec2<f16> = swizzle %src, zw
    %11:u32 = glsl.packFloat2x16 %10
    %12:vec2<u32> = construct %9, %11
    %13:vec2<f32> = glsl.uintBitsToFloat %12
    ret %13
  }
}
)";

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::glsl::writer::raise
