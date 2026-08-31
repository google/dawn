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

struct GlslWriter_BuiltinPolyfillTest : public core::ir::transform::TransformTest {
  protected:
    void SetUp() override { mod.properties.Add(core::ir::Property::kAllow16BitFloats); }
};

TEST_F(GlslWriter_BuiltinPolyfillTest, SelectScalar) {
    auto* a = b.FunctionParam("a", ty.f32());
    auto* b_param = b.FunctionParam("b", ty.f32());
    auto* c = b.FunctionParam("c", ty.bool_());
    auto* func = b.Function("foo", ty.f32());
    func->SetParams({a, b_param, c});
    b.Append(func->Block(),
             [&] { b.Return(func, b.Call<f32>(core::BuiltinFn::kSelect, a, b_param, c)); });

    auto* src = R"(
%foo = func(%a:f32, %b:f32, %c:bool):f32 {
  $B1: {
    %5:f32 = select %a, %b, %c
    ret %5
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%a:f32, %b:f32, %c:bool):f32 {
  $B1: {
    %5:f32 = glsl.mix %a, %b, %c
    ret %5
  }
}
)";

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, SelectVector) {
    auto* a = b.FunctionParam("a", ty.vec3f());
    auto* b_param = b.FunctionParam("b", ty.vec3f());
    auto* c = b.FunctionParam("c", ty.vec3<bool>());
    auto* func = b.Function("foo", ty.vec3f());
    func->SetParams({a, b_param, c});
    b.Append(func->Block(),
             [&] { b.Return(func, b.Call<vec3<f32>>(core::BuiltinFn::kSelect, a, b_param, c)); });

    auto* src = R"(
%foo = func(%a:vec3<f32>, %b:vec3<f32>, %c:vec3<bool>):vec3<f32> {
  $B1: {
    %5:vec3<f32> = select %a, %b, %c
    ret %5
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%a:vec3<f32>, %b:vec3<f32>, %c:vec3<bool>):vec3<f32> {
  $B1: {
    %5:vec3<f32> = glsl.mix %a, %b, %c
    ret %5
  }
}
)";

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, StorageBarrier) {
    auto* func = b.ComputeFunction("foo");
    b.Append(func->Block(), [&] {
        b.Call(ty.void_(), core::BuiltinFn::kStorageBarrier);
        b.Return(func);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:void = storageBarrier
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:void = glsl.memoryBarrierBuffer
    %3:void = glsl.barrier
    ret
  }
}
)";

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, TextureBarrier) {
    auto* func = b.ComputeFunction("foo");
    b.Append(func->Block(), [&] {
        b.Call(ty.void_(), core::BuiltinFn::kTextureBarrier);
        b.Return(func);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:void = textureBarrier
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:void = glsl.memoryBarrierImage
    %3:void = glsl.barrier
    ret
  }
}
)";

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, WorkgroupBarrier) {
    auto* func = b.ComputeFunction("foo");
    b.Append(func->Block(), [&] {
        b.Call(ty.void_(), core::BuiltinFn::kWorkgroupBarrier);
        b.Return(func);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:void = workgroupBarrier
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
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
    b.ir.root_block->Append(var);

    auto* cmp = b.FunctionParam("cmp", ty.i32());
    auto* val = b.FunctionParam("val", ty.i32());
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({cmp, val});
    b.Append(func->Block(), [&] {
        b.Let("x", b.Call(core::type::CreateAtomicCompareExchangeResult(ty, mod.symbols, ty.i32()),
                          core::BuiltinFn::kAtomicCompareExchangeWeak, var, cmp, val));
        b.Return(func);
    });

    auto* src = R"(
__atomic_compare_exchange_result_i32 = struct @align(4) {
  old_value:i32 @offset(0)
  exchanged:bool @offset(4)
}

$B1: {  # root
  %v:ptr<workgroup, atomic<i32>, read_write> = var undef
}

%foo = func(%cmp:i32, %val:i32):void {
  $B2: {
    %5:__atomic_compare_exchange_result_i32 = atomicCompareExchangeWeak %v, %cmp, %val
    %x:__atomic_compare_exchange_result_i32 = let %5
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
  %v:ptr<workgroup, atomic<i32>, read_write> = var undef
}

%foo = func(%cmp:i32, %val:i32):void {
  $B2: {
    %5:i32 = bitcast<i32> %cmp
    %6:i32 = bitcast<i32> %val
    %7:i32 = glsl.atomicCompSwap %v, %5, %6
    %8:bool = eq %7, %cmp
    %9:__atomic_compare_exchange_result_i32 = construct %7, %8
    %x:__atomic_compare_exchange_result_i32 = let %9
    ret
  }
}
)";

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, AtomicSub) {
    auto* var = b.Var("v", workgroup, ty.atomic<i32>(), core::Access::kReadWrite);
    b.ir.root_block->Append(var);

    auto* func = b.ComputeFunction("foo");
    b.Append(func->Block(), [&] {
        b.Let("x", b.Call(ty.i32(), core::BuiltinFn::kAtomicSub, var, 123_i));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<workgroup, atomic<i32>, read_write> = var undef
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
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
  %v:ptr<workgroup, atomic<i32>, read_write> = var undef
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:i32 = atomicAdd %v, -123i
    %x:i32 = let %3
    ret
  }
}
)";

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, AtomicSub_u32) {
    auto* var = b.Var("v", workgroup, ty.atomic<u32>(), core::Access::kReadWrite);
    b.ir.root_block->Append(var);

    auto* func = b.ComputeFunction("foo");
    b.Append(func->Block(), [&] {
        b.Let("x", b.Call(ty.u32(), core::BuiltinFn::kAtomicSub, var, 123_u));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<workgroup, atomic<u32>, read_write> = var undef
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
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
  %v:ptr<workgroup, atomic<u32>, read_write> = var undef
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
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
    b.ir.root_block->Append(var);

    auto* func = b.ComputeFunction("foo");
    b.Append(func->Block(), [&] {
        b.Let("x", b.Call(ty.i32(), core::BuiltinFn::kAtomicLoad, var));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<workgroup, atomic<i32>, read_write> = var undef
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
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
  %v:ptr<workgroup, atomic<i32>, read_write> = var undef
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
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

TEST_F(GlslWriter_BuiltinPolyfillTest, CountOneBits) {
    auto* a = b.FunctionParam("a", ty.u32());
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({a});
    b.Append(func->Block(), [&] {
        b.Let("x", b.Call(ty.u32(), core::BuiltinFn::kCountOneBits, a));
        b.Return(func);
    });

    auto* src = R"(
%foo = func(%a:u32):void {
  $B1: {
    %3:u32 = countOneBits %a
    %x:u32 = let %3
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%a:u32):void {
  $B1: {
    %3:i32 = glsl.bitCount %a
    %4:u32 = convert %3
    %x:u32 = let %4
    ret
  }
}
)";

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, ExtractBits) {
    auto* v = b.FunctionParam("v", ty.u32());
    auto* offset = b.FunctionParam("offset", ty.u32());
    auto* count = b.FunctionParam("count", ty.u32());
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({v, offset, count});
    b.Append(func->Block(), [&] {
        b.Let("x", b.Call(ty.u32(), core::BuiltinFn::kExtractBits, v, offset, count));
        b.Return(func);
    });

    auto* src = R"(
%foo = func(%v:u32, %offset:u32, %count:u32):void {
  $B1: {
    %5:u32 = extractBits %v, %offset, %count
    %x:u32 = let %5
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%v:u32, %offset:u32, %count:u32):void {
  $B1: {
    %5:i32 = convert %offset
    %6:i32 = convert %count
    %7:u32 = glsl.bitfieldExtract %v, %5, %6
    %x:u32 = let %7
    ret
  }
}
)";

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, InsertBits) {
    auto* v = b.FunctionParam("v", ty.u32());
    auto* n = b.FunctionParam("n", ty.u32());
    auto* offset = b.FunctionParam("offset", ty.u32());
    auto* count = b.FunctionParam("count", ty.u32());
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({v, n, offset, count});
    b.Append(func->Block(), [&] {
        b.Let("x", b.Call(ty.u32(), core::BuiltinFn::kInsertBits, v, n, offset, count));
        b.Return(func);
    });

    auto* src = R"(
%foo = func(%v:u32, %n:u32, %offset:u32, %count:u32):void {
  $B1: {
    %6:u32 = insertBits %v, %n, %offset, %count
    %x:u32 = let %6
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%v:u32, %n:u32, %offset:u32, %count:u32):void {
  $B1: {
    %6:i32 = convert %offset
    %7:i32 = convert %count
    %8:u32 = glsl.bitfieldInsert %v, %n, %6, %7
    %x:u32 = let %8
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
        auto* x = b.Let("x", b.Splat(ty.vec3f(), 1_f));
        auto* y = b.Let("y", b.Splat(ty.vec3f(), 2_f));
        auto* z = b.Let("z", b.Splat(ty.vec3f(), 3_f));

        b.Let("res", b.Call(ty.vec3f(), core::BuiltinFn::kFma, x, y, z));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %x:vec3<f32> = let vec3<f32>(1.0f)
    %y:vec3<f32> = let vec3<f32>(2.0f)
    %z:vec3<f32> = let vec3<f32>(3.0f)
    %5:vec3<f32> = fma %x, %y, %z
    %res:vec3<f32> = let %5
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    %x:vec3<f32> = let vec3<f32>(1.0f)
    %y:vec3<f32> = let vec3<f32>(2.0f)
    %z:vec3<f32> = let vec3<f32>(3.0f)
    %5:vec3<f32> = mul %x, %y
    %6:vec3<f32> = add %5, %z
    %res:vec3<f32> = let %6
    ret
  }
}
)";

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, FMA_f16) {
    auto* x = b.FunctionParam("x", ty.vec3h());
    auto* y = b.FunctionParam("y", ty.vec3h());
    auto* z = b.FunctionParam("z", ty.vec3h());
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({x, y, z});
    b.Append(func->Block(), [&] {
        b.Let("x", b.Call(ty.vec3h(), core::BuiltinFn::kFma, x, y, z));
        b.Return(func);
    });

    auto* src = R"(
%foo = func(%x:vec3<f16>, %y:vec3<f16>, %z:vec3<f16>):void {
  $B1: {
    %5:vec3<f16> = fma %x, %y, %z
    %x_1:vec3<f16> = let %5  # %x_1: 'x'
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
%foo = func(%x:vec3<f16>, %y:vec3<f16>, %z:vec3<f16>):void {
  $B1: {
    %5:vec3<f16> = mul %x, %y
    %6:vec3<f16> = add %5, %z
    %x_1:vec3<f16> = let %6  # %x_1: 'x'
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
  %v:ptr<storage, SB, read_write> = var undef @binding_point(0, 0)
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
  %v:ptr<storage, SB, read_write> = var undef @binding_point(0, 0)
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
    auto* a = b.FunctionParam("a", ty.bool_());
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({a});
    b.Append(func->Block(), [&] {
        b.Let("x", b.Call(ty.bool_(), core::BuiltinFn::kAny, a));
        b.Return(func);
    });

    auto* src = R"(
%foo = func(%a:bool):void {
  $B1: {
    %3:bool = any %a
    %x:bool = let %3
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
%foo = func(%a:bool):void {
  $B1: {
    %x:bool = let %a
    ret
  }
}
)";

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, AllScalar) {
    auto* a = b.FunctionParam("a", ty.bool_());
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({a});
    b.Append(func->Block(), [&] {
        b.Let("x", b.Call(ty.bool_(), core::BuiltinFn::kAll, a));
        b.Return(func);
    });

    auto* src = R"(
%foo = func(%a:bool):void {
  $B1: {
    %3:bool = all %a
    %x:bool = let %3
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
%foo = func(%a:bool):void {
  $B1: {
    %x:bool = let %a
    ret
  }
}
)";

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, DotF32) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* x = b.Let("x", b.Splat(ty.vec3f(), 2_f));
        auto* y = b.Let("y", b.Splat(ty.vec3f(), 3_f));
        b.Let("z", b.Call(ty.f32(), core::BuiltinFn::kDot, x, y));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %x:vec3<f32> = let vec3<f32>(2.0f)
    %y:vec3<f32> = let vec3<f32>(3.0f)
    %4:f32 = dot %x, %y
    %z:f32 = let %4
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expected = R"(
%foo = @fragment func():void {
  $B1: {
    %x:vec3<f32> = let vec3<f32>(2.0f)
    %y:vec3<f32> = let vec3<f32>(3.0f)
    %4:f32 = glsl.dot %x, %y
    %z:f32 = let %4
    ret
  }
}
)";
    Run(BuiltinPolyfill);
    EXPECT_EQ(expected, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, DotF16) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* x = b.Let("x", b.Splat(ty.vec4h(), 2_h));
        auto* y = b.Let("y", b.Splat(ty.vec4h(), 3_h));
        b.Let("z", b.Call(ty.f16(), core::BuiltinFn::kDot, x, y));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %x:vec4<f16> = let vec4<f16>(2.0h)
    %y:vec4<f16> = let vec4<f16>(3.0h)
    %4:f16 = dot %x, %y
    %z:f16 = let %4
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expected = R"(
%foo = @fragment func():void {
  $B1: {
    %x:vec4<f16> = let vec4<f16>(2.0h)
    %y:vec4<f16> = let vec4<f16>(3.0h)
    %4:f16 = glsl.dot %x, %y
    %z:f16 = let %4
    ret
  }
}
)";

    Run(BuiltinPolyfill);
    EXPECT_EQ(expected, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, DotI32) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* x = b.Let("x", b.Splat(ty.vec4i(), 2_i));
        auto* y = b.Let("y", b.Splat(ty.vec4i(), 3_i));
        b.Let("z", b.Call(ty.i32(), core::BuiltinFn::kDot, x, y));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %x:vec4<i32> = let vec4<i32>(2i)
    %y:vec4<i32> = let vec4<i32>(3i)
    %4:i32 = dot %x, %y
    %z:i32 = let %4
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expected = R"(
%foo = @fragment func():void {
  $B1: {
    %x:vec4<i32> = let vec4<i32>(2i)
    %y:vec4<i32> = let vec4<i32>(3i)
    %4:i32 = call %tint_int_dot, %x, %y
    %z:i32 = let %4
    ret
  }
}
%tint_int_dot = func(%x_1:vec4<i32>, %y_1:vec4<i32>):i32 {  # %x_1: 'x', %y_1: 'y'
  $B2: {
    %9:i32 = swizzle %x_1, x
    %10:i32 = swizzle %y_1, x
    %11:i32 = mul %9, %10
    %12:i32 = swizzle %x_1, y
    %13:i32 = swizzle %y_1, y
    %14:i32 = mul %12, %13
    %15:i32 = add %11, %14
    %16:i32 = swizzle %x_1, z
    %17:i32 = swizzle %y_1, z
    %18:i32 = mul %16, %17
    %19:i32 = add %15, %18
    %20:i32 = swizzle %x_1, w
    %21:i32 = swizzle %y_1, w
    %22:i32 = mul %20, %21
    %23:i32 = add %19, %22
    ret %23
  }
}
)";

    Run(BuiltinPolyfill);
    EXPECT_EQ(expected, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, DotU32) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* x = b.Let("x", b.Splat(ty.vec2u(), 2_u));
        auto* y = b.Let("y", b.Splat(ty.vec2u(), 3_u));
        b.Let("z", b.Call(ty.u32(), core::BuiltinFn::kDot, x, y));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %x:vec2<u32> = let vec2<u32>(2u)
    %y:vec2<u32> = let vec2<u32>(3u)
    %4:u32 = dot %x, %y
    %z:u32 = let %4
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expected = R"(
%foo = @fragment func():void {
  $B1: {
    %x:vec2<u32> = let vec2<u32>(2u)
    %y:vec2<u32> = let vec2<u32>(3u)
    %4:u32 = call %tint_int_dot, %x, %y
    %z:u32 = let %4
    ret
  }
}
%tint_int_dot = func(%x_1:vec2<u32>, %y_1:vec2<u32>):u32 {  # %x_1: 'x', %y_1: 'y'
  $B2: {
    %9:u32 = swizzle %x_1, x
    %10:u32 = swizzle %y_1, x
    %11:u32 = mul %9, %10
    %12:u32 = swizzle %x_1, y
    %13:u32 = swizzle %y_1, y
    %14:u32 = mul %12, %13
    %15:u32 = add %11, %14
    ret %15
  }
}
)";

    Run(BuiltinPolyfill);
    EXPECT_EQ(expected, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, Modf_Scalar) {
    auto* value = b.FunctionParam<f32>("value");
    auto* func = b.Function("foo", ty.f32());
    func->SetParams({value});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(core::type::CreateModfResult(ty, mod.symbols, ty.f32()),
                              core::BuiltinFn::kModf, value);
        auto* fract = b.Access<f32>(result, 0_u);
        auto* whole = b.Access<f32>(result, 1_u);
        b.Return(func, b.Add(fract, whole));
    });

    auto* src = R"(
__modf_result_f32 = struct @align(4) {
  fract:f32 @offset(0)
  whole:f32 @offset(4)
}

%foo = func(%value:f32):f32 {
  $B1: {
    %3:__modf_result_f32 = modf %value
    %4:f32 = access %3, 0u
    %5:f32 = access %3, 1u
    %6:f32 = add %4, %5
    ret %6
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
__modf_result_f32 = struct @align(4) {
  fract:f32 @offset(0)
  whole:f32 @offset(4)
}

%foo = func(%value:f32):f32 {
  $B1: {
    %3:ptr<function, __modf_result_f32, read_write> = var undef
    %4:ptr<function, f32, read_write> = access %3, 1u
    %5:f32 = glsl.modf %value, %4
    %6:ptr<function, f32, read_write> = access %3, 0u
    store %6, %5
    %7:__modf_result_f32 = load %3
    %8:f32 = access %7, 0u
    %9:f32 = access %7, 1u
    %10:f32 = add %8, %9
    ret %10
  }
}
)";

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, Modf_Vector) {
    auto* value = b.FunctionParam<vec4<f32>>("value");
    auto* func = b.Function("foo", ty.vec4f());
    func->SetParams({value});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(core::type::CreateModfResult(ty, mod.symbols, ty.vec4f()),
                              core::BuiltinFn::kModf, value);
        auto* fract = b.Access<vec4<f32>>(result, 0_u);
        auto* whole = b.Access<vec4<f32>>(result, 1_u);
        b.Return(func, b.Add(fract, whole));
    });

    auto* src = R"(
__modf_result_vec4_f32 = struct @align(16) {
  fract:vec4<f32> @offset(0)
  whole:vec4<f32> @offset(16)
}

%foo = func(%value:vec4<f32>):vec4<f32> {
  $B1: {
    %3:__modf_result_vec4_f32 = modf %value
    %4:vec4<f32> = access %3, 0u
    %5:vec4<f32> = access %3, 1u
    %6:vec4<f32> = add %4, %5
    ret %6
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
__modf_result_vec4_f32 = struct @align(16) {
  fract:vec4<f32> @offset(0)
  whole:vec4<f32> @offset(16)
}

%foo = func(%value:vec4<f32>):vec4<f32> {
  $B1: {
    %3:ptr<function, __modf_result_vec4_f32, read_write> = var undef
    %4:ptr<function, vec4<f32>, read_write> = access %3, 1u
    %5:vec4<f32> = glsl.modf %value, %4
    %6:ptr<function, vec4<f32>, read_write> = access %3, 0u
    store %6, %5
    %7:__modf_result_vec4_f32 = load %3
    %8:vec4<f32> = access %7, 0u
    %9:vec4<f32> = access %7, 1u
    %10:vec4<f32> = add %8, %9
    ret %10
  }
}
)";

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, Frexp_Scalar) {
    auto* value = b.FunctionParam<f32>("value");
    auto* func = b.Function("foo", ty.f32());
    func->SetParams({value});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(core::type::CreateFrexpResult(ty, mod.symbols, ty.f32()),
                              core::BuiltinFn::kFrexp, value);
        auto* fract = b.Access<f32>(result, 0_u);
        auto* exp = b.Access<i32>(result, 1_u);
        b.Return(func, b.Add(fract, b.Convert<f32>(exp)));
    });

    auto* src = R"(
__frexp_result_f32 = struct @align(4) {
  fract:f32 @offset(0)
  exp:i32 @offset(4)
}

%foo = func(%value:f32):f32 {
  $B1: {
    %3:__frexp_result_f32 = frexp %value
    %4:f32 = access %3, 0u
    %5:i32 = access %3, 1u
    %6:f32 = convert %5
    %7:f32 = add %4, %6
    ret %7
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
__frexp_result_f32 = struct @align(4) {
  fract:f32 @offset(0)
  exp:i32 @offset(4)
}

%foo = func(%value:f32):f32 {
  $B1: {
    %3:ptr<function, __frexp_result_f32, read_write> = var undef
    %4:ptr<function, i32, read_write> = access %3, 1u
    %5:f32 = glsl.frexp %value, %4
    %6:ptr<function, f32, read_write> = access %3, 0u
    store %6, %5
    %7:__frexp_result_f32 = load %3
    %8:f32 = access %7, 0u
    %9:i32 = access %7, 1u
    %10:f32 = convert %9
    %11:f32 = add %8, %10
    ret %11
  }
}
)";

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, Frexp_Vector) {
    auto* value = b.FunctionParam<vec4<f32>>("value");
    auto* func = b.Function("foo", ty.vec4f());
    func->SetParams({value});
    b.Append(func->Block(), [&] {
        auto* result = b.Call(core::type::CreateFrexpResult(ty, mod.symbols, ty.vec4f()),
                              core::BuiltinFn::kFrexp, value);
        auto* fract = b.Access<vec4<f32>>(result, 0_u);
        auto* exp = b.Access<vec4<i32>>(result, 1_u);
        b.Return(func, b.Add(fract, b.Convert<vec4<f32>>(exp)));
    });

    auto* src = R"(
__frexp_result_vec4_f32 = struct @align(16) {
  fract:vec4<f32> @offset(0)
  exp:vec4<i32> @offset(16)
}

%foo = func(%value:vec4<f32>):vec4<f32> {
  $B1: {
    %3:__frexp_result_vec4_f32 = frexp %value
    %4:vec4<f32> = access %3, 0u
    %5:vec4<i32> = access %3, 1u
    %6:vec4<f32> = convert %5
    %7:vec4<f32> = add %4, %6
    ret %7
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
__frexp_result_vec4_f32 = struct @align(16) {
  fract:vec4<f32> @offset(0)
  exp:vec4<i32> @offset(16)
}

%foo = func(%value:vec4<f32>):vec4<f32> {
  $B1: {
    %3:ptr<function, __frexp_result_vec4_f32, read_write> = var undef
    %4:ptr<function, vec4<i32>, read_write> = access %3, 1u
    %5:vec4<f32> = glsl.frexp %value, %4
    %6:ptr<function, vec4<f32>, read_write> = access %3, 0u
    store %6, %5
    %7:__frexp_result_vec4_f32 = load %3
    %8:vec4<f32> = access %7, 0u
    %9:vec4<i32> = access %7, 1u
    %10:vec4<f32> = convert %9
    %11:vec4<f32> = add %8, %10
    ret %11
  }
}
)";

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, AbsScalar) {
    auto* a = b.FunctionParam("a", ty.u32());
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({a});
    b.Append(func->Block(), [&] {
        b.Let("x", b.Call(ty.u32(), core::BuiltinFn::kAbs, a));
        b.Return(func);
    });

    auto* src = R"(
%foo = func(%a:u32):void {
  $B1: {
    %3:u32 = abs %a
    %x:u32 = let %3
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
%foo = func(%a:u32):void {
  $B1: {
    %x:u32 = let %a
    ret
  }
}
)";

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, AbsVector) {
    auto* a = b.FunctionParam("a", ty.vec2u());
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({a});
    b.Append(func->Block(), [&] {
        b.Let("x", b.Call(ty.vec2u(), core::BuiltinFn::kAbs, a));
        b.Return(func);
    });

    auto* src = R"(
%foo = func(%a:vec2<u32>):void {
  $B1: {
    %3:vec2<u32> = abs %a
    %x:vec2<u32> = let %3
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
%foo = func(%a:vec2<u32>):void {
  $B1: {
    %x:vec2<u32> = let %a
    ret
  }
}
)";

    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, QuantizeToF16) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* v = b.Var("x", b.Zero(ty.vec2f()));
        b.Let("a", b.Call(ty.vec2f(), core::BuiltinFn::kQuantizeToF16, b.Load(v)));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %x:ptr<function, vec2<f32>, read_write> = var vec2<f32>(0.0f)
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
    %x:ptr<function, vec2<f32>, read_write> = var vec2<f32>(0.0f)
    %3:vec2<f32> = load %x
    %4:vec2<f32> = call %tint_quantize_to_f16, %3
    %a:vec2<f32> = let %4
    ret
  }
}
%tint_quantize_to_f16 = func(%val:vec2<f32>):vec2<f32> {
  $B2: {
    %8:u32 = pack2x16float %val
    %9:vec2<f32> = unpack2x16float %8
    ret %9
  }
}
)";
    Run(BuiltinPolyfill);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, AddSat_Scalar) {
    auto* foo = b.Function("foo", ty.void_());
    auto* lhs = b.FunctionParam("a", ty.u32());
    auto* rhs = b.FunctionParam("b", ty.u32());
    foo->SetParams({lhs, rhs});
    b.Append(foo->Block(), [&] {
        auto* call = b.Call(ty.u32(), core::BuiltinFn::kAddSat, lhs, rhs);
        b.Let("res", call);
        b.Return(foo);
    });

    auto* src = R"(
%foo = func(%a:u32, %b:u32):void {
  $B1: {
    %4:u32 = addSat %a, %b
    %res:u32 = let %4
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%a:u32, %b:u32):void {
  $B1: {
    %4:ptr<function, u32, read_write> = var undef
    %5:u32 = glsl.uaddCarry %a, %b, %4
    %6:u32 = load %4
    %7:bool = eq %6, 0u
    %8:u32 = glsl.mix 4294967295u, %5, %7
    %res:u32 = let %8
    ret
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, AddSat_Vector) {
    auto* vec_ty = ty.vec2u();
    auto* foo = b.Function("foo", ty.void_());
    auto* lhs = b.FunctionParam("a", vec_ty);
    auto* rhs = b.FunctionParam("b", vec_ty);
    foo->SetParams({lhs, rhs});
    b.Append(foo->Block(), [&] {
        auto* call = b.Call(vec_ty, core::BuiltinFn::kAddSat, lhs, rhs);
        b.Let("res", call);
        b.Return(foo);
    });

    auto* src = R"(
%foo = func(%a:vec2<u32>, %b:vec2<u32>):void {
  $B1: {
    %4:vec2<u32> = addSat %a, %b
    %res:vec2<u32> = let %4
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%a:vec2<u32>, %b:vec2<u32>):void {
  $B1: {
    %4:ptr<function, vec2<u32>, read_write> = var undef
    %5:vec2<u32> = glsl.uaddCarry %a, %b, %4
    %6:vec2<u32> = load %4
    %7:vec2<bool> = eq %6, vec2<u32>(0u)
    %8:vec2<u32> = glsl.mix vec2<u32>(4294967295u), %5, %7
    %res:vec2<u32> = let %8
    ret
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, MulSat_Scalar) {
    auto* foo = b.Function("foo", ty.void_());
    auto* lhs = b.FunctionParam("a", ty.u32());
    auto* rhs = b.FunctionParam("b", ty.u32());
    foo->SetParams({lhs, rhs});
    b.Append(foo->Block(), [&] {
        auto* call = b.Call(ty.u32(), core::BuiltinFn::kMulSat, lhs, rhs);
        b.Let("res", call);
        b.Return(foo);
    });

    auto* src = R"(
%foo = func(%a:u32, %b:u32):void {
  $B1: {
    %4:u32 = mulSat %a, %b
    %res:u32 = let %4
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%a:u32, %b:u32):void {
  $B1: {
    %4:ptr<function, u32, read_write> = var undef
    %5:ptr<function, u32, read_write> = var undef
    %6:void = glsl.umulExtended %a, %b, %4, %5
    %7:u32 = load %4
    %8:bool = eq %7, 0u
    %9:u32 = load %5
    %10:u32 = glsl.mix 4294967295u, %9, %8
    %res:u32 = let %10
    ret
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_BuiltinPolyfillTest, MulSat_Vector) {
    auto* vec_ty = ty.vec2u();
    auto* foo = b.Function("foo", ty.void_());
    auto* lhs = b.FunctionParam("a", vec_ty);
    auto* rhs = b.FunctionParam("b", vec_ty);
    foo->SetParams({lhs, rhs});
    b.Append(foo->Block(), [&] {
        auto* call = b.Call(vec_ty, core::BuiltinFn::kMulSat, lhs, rhs);
        b.Let("res", call);
        b.Return(foo);
    });

    auto* src = R"(
%foo = func(%a:vec2<u32>, %b:vec2<u32>):void {
  $B1: {
    %4:vec2<u32> = mulSat %a, %b
    %res:vec2<u32> = let %4
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%a:vec2<u32>, %b:vec2<u32>):void {
  $B1: {
    %4:ptr<function, vec2<u32>, read_write> = var undef
    %5:ptr<function, vec2<u32>, read_write> = var undef
    %6:void = glsl.umulExtended %a, %b, %4, %5
    %7:vec2<u32> = load %4
    %8:vec2<bool> = eq %7, vec2<u32>(0u)
    %9:vec2<u32> = load %5
    %10:vec2<u32> = glsl.mix vec2<u32>(4294967295u), %9, %8
    %res:vec2<u32> = let %10
    ret
  }
}
)";

    Run(BuiltinPolyfill);

    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::glsl::writer::raise
