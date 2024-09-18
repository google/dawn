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

#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/ir/function.h"
#include "src/tint/lang/core/number.h"
#include "src/tint/lang/core/type/builtin_structs.h"
#include "src/tint/lang/core/type/depth_multisampled_texture.h"
#include "src/tint/lang/core/type/depth_texture.h"
#include "src/tint/lang/core/type/multisampled_texture.h"
#include "src/tint/lang/core/type/sampled_texture.h"
#include "src/tint/lang/core/type/sampler.h"
#include "src/tint/lang/core/type/sampler_kind.h"
#include "src/tint/lang/core/type/storage_texture.h"
#include "src/tint/lang/core/type/texture_dimension.h"
#include "src/tint/lang/glsl/writer/helper_test.h"

#include "gtest/gtest.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::glsl::writer {
namespace {

TEST_F(GlslWriterTest, BuiltinGeneric) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    func->SetWorkgroupSize(1, 1, 1);
    b.Append(func->Block(), [&] {
        auto* x = b.Let("x", 1_i);

        auto* c = b.Call(ty.i32(), core::BuiltinFn::kAbs, x);
        b.Let("w", c);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  int x = 1;
  int w = abs(x);
}
)");
}

TEST_F(GlslWriterTest, BuiltinSelectScalar) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* x = b.Let("x", 1_i);
        auto* y = b.Let("y", 2_i);

        auto* c = b.Call(ty.i32(), core::BuiltinFn::kSelect, x, y, true);
        b.Let("w", c);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

void main() {
  int x = 1;
  int y = 2;
  int w = ((true) ? (y) : (x));
}
)");
}

TEST_F(GlslWriterTest, BuiltinSelectVector) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* x = b.Let("x", b.Construct<vec2<i32>>(1_i, 2_i));
        auto* y = b.Let("y", b.Construct<vec2<i32>>(3_i, 4_i));
        auto* cmp = b.Construct<vec2<bool>>(true, false);

        auto* c = b.Call(ty.vec2<i32>(), core::BuiltinFn::kSelect, x, y, cmp);
        b.Let("w", c);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

void main() {
  ivec2 x = ivec2(1, 2);
  ivec2 y = ivec2(3, 4);
  bvec2 v = bvec2(true, false);
  int v_1 = ((v.x) ? (y.x) : (x.x));
  ivec2 w = ivec2(v_1, ((v.y) ? (y.y) : (x.y)));
}
)");
}

TEST_F(GlslWriterTest, BuiltinStorageBarrier) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    func->SetWorkgroupSize(1, 1, 1);
    b.Append(func->Block(), [&] {
        b.Call(ty.void_(), core::BuiltinFn::kStorageBarrier);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  barrier();
  memoryBarrierBuffer();
}
)");
}

TEST_F(GlslWriterTest, BuiltinTextureBarrier) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    func->SetWorkgroupSize(1, 1, 1);
    b.Append(func->Block(), [&] {
        b.Call(ty.void_(), core::BuiltinFn::kTextureBarrier);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  barrier();
  memoryBarrierImage();
}
)");
}

TEST_F(GlslWriterTest, BuiltinWorkgroupBarrier) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    func->SetWorkgroupSize(1, 1, 1);
    b.Append(func->Block(), [&] {
        b.Call(ty.void_(), core::BuiltinFn::kWorkgroupBarrier);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  barrier();
}
)");
}

TEST_F(GlslWriterTest, BuiltinStorageAtomicCompareExchangeWeak) {
    auto* sb = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("padding"), ty.vec4<f32>()},
                                                    {mod.symbols.New("a"), ty.atomic<i32>()},
                                                    {mod.symbols.New("b"), ty.atomic<u32>()},
                                                });

    auto* var = b.Var("v", storage, sb, core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("x",
              b.Call(core::type::CreateAtomicCompareExchangeResult(ty, mod.symbols, ty.i32()),
                     core::BuiltinFn::kAtomicCompareExchangeWeak,
                     b.Access(ty.ptr<storage, atomic<i32>, read_write>(), var, 1_u), 123_i, 345_i));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;


struct SB {
  vec4 padding;
  int a;
  uint b;
};

struct atomic_compare_exchange_result_i32 {
  int old_value;
  bool exchanged;
};

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  SB tint_symbol;
} v_1;
void main() {
  int v_2 = atomicCompSwap(v_1.tint_symbol.a, 123, 345);
  atomic_compare_exchange_result_i32 x = atomic_compare_exchange_result_i32(v_2, (v_2 == 123));
}
)");
}

TEST_F(GlslWriterTest, BuiltinStorageAtomicCompareExchangeWeakDirect) {
    auto* var = b.Var("v", storage, ty.atomic<i32>(), core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("x", b.Call(core::type::CreateAtomicCompareExchangeResult(ty, mod.symbols, ty.i32()),
                          core::BuiltinFn::kAtomicCompareExchangeWeak, var, 123_i, 345_i));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;


struct atomic_compare_exchange_result_i32 {
  int old_value;
  bool exchanged;
};

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  int tint_symbol;
} v_1;
void main() {
  int v_2 = atomicCompSwap(v_1.tint_symbol, 123, 345);
  atomic_compare_exchange_result_i32 x = atomic_compare_exchange_result_i32(v_2, (v_2 == 123));
}
)");
}

TEST_F(GlslWriterTest, BuiltinWorkgroupAtomicLoad) {
    auto* sb = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("padding"), ty.vec4<f32>()},
                                                    {mod.symbols.New("a"), ty.atomic<i32>()},
                                                    {mod.symbols.New("b"), ty.atomic<u32>()},
                                                });

    auto* var = b.Var("v", workgroup, sb, core::Access::kReadWrite);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    func->SetWorkgroupSize(1, 1, 1);
    b.Append(func->Block(), [&] {
        b.Let("x", b.Call(ty.i32(), core::BuiltinFn::kAtomicLoad,
                          b.Access(ty.ptr<workgroup, atomic<i32>, read_write>(), var, 1_u)));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(

struct SB {
  vec4 padding;
  int a;
  uint b;
};

shared SB v;
void foo_inner(uint tint_local_index) {
  if ((tint_local_index == 0u)) {
    v.padding = vec4(0.0f);
    atomicExchange(v.a, 0);
    atomicExchange(v.b, 0u);
  }
  barrier();
  int x = atomicOr(v.a, 0);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  foo_inner(gl_LocalInvocationIndex);
}
)");
}

TEST_F(GlslWriterTest, BuiltinWorkgroupAtomicSub) {
    auto* sb = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("padding"), ty.vec4<f32>()},
                                                    {mod.symbols.New("a"), ty.atomic<i32>()},
                                                    {mod.symbols.New("b"), ty.atomic<u32>()},
                                                });

    auto* var = b.Var("v", workgroup, sb, core::Access::kReadWrite);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    func->SetWorkgroupSize(1, 1, 1);
    b.Append(func->Block(), [&] {
        b.Let("x", b.Call(ty.i32(), core::BuiltinFn::kAtomicSub,
                          b.Access(ty.ptr<workgroup, atomic<i32>, read_write>(), var, 1_u), 123_i));
        b.Let("y", b.Call(ty.u32(), core::BuiltinFn::kAtomicSub,
                          b.Access(ty.ptr<workgroup, atomic<u32>, read_write>(), var, 2_u), 123_u));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(

struct SB {
  vec4 padding;
  int a;
  uint b;
};

shared SB v;
void foo_inner(uint tint_local_index) {
  if ((tint_local_index == 0u)) {
    v.padding = vec4(0.0f);
    atomicExchange(v.a, 0);
    atomicExchange(v.b, 0u);
  }
  barrier();
  int x = atomicAdd(v.a, -(123));
  uint y = atomicAdd(v.b, -(123u));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  foo_inner(gl_LocalInvocationIndex);
}
)");
}

TEST_F(GlslWriterTest, BuiltinWorkgroupAtomicCompareExchangeWeak) {
    auto* sb = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("padding"), ty.vec4<f32>()},
                                                    {mod.symbols.New("a"), ty.atomic<i32>()},
                                                    {mod.symbols.New("b"), ty.atomic<u32>()},
                                                });

    auto* var = b.Var("v", workgroup, sb, core::Access::kReadWrite);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    func->SetWorkgroupSize(1, 1, 1);
    b.Append(func->Block(), [&] {
        b.Let("x", b.Call(core::type::CreateAtomicCompareExchangeResult(ty, mod.symbols, ty.i32()),
                          core::BuiltinFn::kAtomicCompareExchangeWeak,
                          b.Access(ty.ptr<workgroup, atomic<i32>, read_write>(), var, 1_u), 123_i,
                          345_i));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(

struct SB {
  vec4 padding;
  int a;
  uint b;
};

struct atomic_compare_exchange_result_i32 {
  int old_value;
  bool exchanged;
};

shared SB v;
void foo_inner(uint tint_local_index) {
  if ((tint_local_index == 0u)) {
    v.padding = vec4(0.0f);
    atomicExchange(v.a, 0);
    atomicExchange(v.b, 0u);
  }
  barrier();
  int v_1 = atomicCompSwap(v.a, 123, 345);
  atomic_compare_exchange_result_i32 x = atomic_compare_exchange_result_i32(v_1, (v_1 == 123));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  foo_inner(gl_LocalInvocationIndex);
}
)");
}

TEST_F(GlslWriterTest, BuiltinBitcast_FloatToFloat) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Let("a", 1_f);
        b.Let("x", b.Bitcast<f32>(a));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

void main() {
  float a = 1.0f;
  float x = a;
}
)");
}

TEST_F(GlslWriterTest, BuiltinBitcast_IntToFloat) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Let("a", 1_i);
        b.Let("x", b.Bitcast<f32>(a));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

void main() {
  int a = 1;
  float x = intBitsToFloat(a);
}
)");
}

TEST_F(GlslWriterTest, BuiltinBitcast_UintToFloat) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Let("a", 1_u);
        b.Let("x", b.Bitcast<f32>(a));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

void main() {
  uint a = 1u;
  float x = uintBitsToFloat(a);
}
)");
}

TEST_F(GlslWriterTest, BuiltinBitcast_Vec3UintToVec3Float) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Let("a", b.Splat<vec3<u32>>(1_u));
        b.Let("x", b.Bitcast<vec3<f32>>(a));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

void main() {
  uvec3 a = uvec3(1u);
  vec3 x = uintBitsToFloat(a);
}
)");
}

TEST_F(GlslWriterTest, BuiltinBitcast_FloatToInt) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Let("a", 1_f);
        b.Let("x", b.Bitcast<i32>(a));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

void main() {
  float a = 1.0f;
  int x = floatBitsToInt(a);
}
)");
}

TEST_F(GlslWriterTest, BuiltinBitcast_FloatToUint) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Let("a", 1_f);
        b.Let("x", b.Bitcast<u32>(a));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

void main() {
  float a = 1.0f;
  uint x = floatBitsToUint(a);
}
)");
}

TEST_F(GlslWriterTest, BuiltinBitcast_UintToInt) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Let("a", 1_u);
        b.Let("x", b.Bitcast<i32>(a));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

void main() {
  uint a = 1u;
  int x = int(a);
}
)");
}

TEST_F(GlslWriterTest, BuiltinBitcast_IntToUint) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Let("a", 1_i);
        b.Let("x", b.Bitcast<u32>(a));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

void main() {
  int a = 1;
  uint x = uint(a);
}
)");
}

TEST_F(GlslWriterTest, BuiltinBitcast_I32ToVec2F16) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Let("a", 1_i);
        b.Let("x", b.Bitcast<vec2<f16>>(a));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(#extension GL_AMD_gpu_shader_half_float: require
precision highp float;
precision highp int;

f16vec2 tint_bitcast_to_f16(int src) {
  return unpackFloat2x16(uint(src));
}
void main() {
  int a = 1;
  f16vec2 x = tint_bitcast_to_f16(a);
}
)");
}

TEST_F(GlslWriterTest, BuiltinBitcast_Vec2F16ToI32) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Let("a", b.Construct<vec2<f16>>(1_h, 2_h));
        b.Let("x", b.Bitcast<i32>(a));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(#extension GL_AMD_gpu_shader_half_float: require
precision highp float;
precision highp int;

int tint_bitcast_from_f16(f16vec2 src) {
  return int(packFloat2x16(src));
}
void main() {
  f16vec2 a = f16vec2(1.0hf, 2.0hf);
  int x = tint_bitcast_from_f16(a);
}
)");
}

TEST_F(GlslWriterTest, BuiltinBitcast_U32ToVec2F16) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Let("a", 1_u);
        b.Let("x", b.Bitcast<vec2<f16>>(a));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(#extension GL_AMD_gpu_shader_half_float: require
precision highp float;
precision highp int;

f16vec2 tint_bitcast_to_f16(uint src) {
  return unpackFloat2x16(uint(src));
}
void main() {
  uint a = 1u;
  f16vec2 x = tint_bitcast_to_f16(a);
}
)");
}

TEST_F(GlslWriterTest, BuiltinBitcast_Vec2F16ToU32) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Let("a", b.Construct<vec2<f16>>(1_h, 2_h));
        b.Let("x", b.Bitcast<u32>(a));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(#extension GL_AMD_gpu_shader_half_float: require
precision highp float;
precision highp int;

uint tint_bitcast_from_f16(f16vec2 src) {
  return uint(packFloat2x16(src));
}
void main() {
  f16vec2 a = f16vec2(1.0hf, 2.0hf);
  uint x = tint_bitcast_from_f16(a);
}
)");
}

TEST_F(GlslWriterTest, BuiltinBitcast_F32ToVec2F16) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Let("a", 1_f);
        b.Let("x", b.Bitcast<vec2<f16>>(a));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(#extension GL_AMD_gpu_shader_half_float: require
precision highp float;
precision highp int;

f16vec2 tint_bitcast_to_f16(float src) {
  return unpackFloat2x16(floatBitsToUint(src));
}
void main() {
  float a = 1.0f;
  f16vec2 x = tint_bitcast_to_f16(a);
}
)");
}

TEST_F(GlslWriterTest, BuiltinBitcast_Vec2F16ToF32) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Let("a", b.Construct<vec2<f16>>(1_h, 2_h));
        b.Let("x", b.Bitcast<f32>(a));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(#extension GL_AMD_gpu_shader_half_float: require
precision highp float;
precision highp int;

float tint_bitcast_from_f16(f16vec2 src) {
  return uintBitsToFloat(packFloat2x16(src));
}
void main() {
  f16vec2 a = f16vec2(1.0hf, 2.0hf);
  float x = tint_bitcast_from_f16(a);
}
)");
}

TEST_F(GlslWriterTest, BuiltinBitcast_Vec2I32ToVec4F16) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Let("a", b.Construct<vec2<i32>>(1_i, 2_i));
        b.Let("x", b.Bitcast<vec4<f16>>(a));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(#extension GL_AMD_gpu_shader_half_float: require
precision highp float;
precision highp int;

f16vec4 tint_bitcast_to_f16(ivec2 src) {
  uvec2 v = uvec2(src);
  f16vec2 v_1 = unpackFloat2x16(v.x);
  return f16vec4(v_1, unpackFloat2x16(v.y));
}
void main() {
  ivec2 a = ivec2(1, 2);
  f16vec4 x = tint_bitcast_to_f16(a);
}
)");
}

TEST_F(GlslWriterTest, BuiltinBitcast_Vec4F16ToVec2I32) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Let("a", b.Construct<vec4<f16>>(1_h, 2_h, 3_h, 4_h));
        b.Let("x", b.Bitcast<vec2<i32>>(a));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(#extension GL_AMD_gpu_shader_half_float: require
precision highp float;
precision highp int;

ivec2 tint_bitcast_from_f16(f16vec4 src) {
  uint v = packFloat2x16(src.xy);
  return ivec2(uvec2(v, packFloat2x16(src.zw)));
}
void main() {
  f16vec4 a = f16vec4(1.0hf, 2.0hf, 3.0hf, 4.0hf);
  ivec2 x = tint_bitcast_from_f16(a);
}
)");
}

TEST_F(GlslWriterTest, BuiltinBitcast_Vec2U32ToVec4F16) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Let("a", b.Construct<vec2<u32>>(1_u, 2_u));
        b.Let("x", b.Bitcast<vec4<f16>>(a));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(#extension GL_AMD_gpu_shader_half_float: require
precision highp float;
precision highp int;

f16vec4 tint_bitcast_to_f16(uvec2 src) {
  uvec2 v = uvec2(src);
  f16vec2 v_1 = unpackFloat2x16(v.x);
  return f16vec4(v_1, unpackFloat2x16(v.y));
}
void main() {
  uvec2 a = uvec2(1u, 2u);
  f16vec4 x = tint_bitcast_to_f16(a);
}
)");
}

TEST_F(GlslWriterTest, BuiltinBitcast_Vec4F16ToVec2U32) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Let("a", b.Construct<vec4<f16>>(1_h, 2_h, 3_h, 4_h));
        b.Let("x", b.Bitcast<vec2<u32>>(a));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(#extension GL_AMD_gpu_shader_half_float: require
precision highp float;
precision highp int;

uvec2 tint_bitcast_from_f16(f16vec4 src) {
  uint v = packFloat2x16(src.xy);
  return uvec2(uvec2(v, packFloat2x16(src.zw)));
}
void main() {
  f16vec4 a = f16vec4(1.0hf, 2.0hf, 3.0hf, 4.0hf);
  uvec2 x = tint_bitcast_from_f16(a);
}
)");
}

TEST_F(GlslWriterTest, BuiltinBitcast_Vec2F32ToVec4F16) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Let("a", b.Construct<vec2<f32>>(1_f, 2_f));
        b.Let("x", b.Bitcast<vec4<f16>>(a));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(#extension GL_AMD_gpu_shader_half_float: require
precision highp float;
precision highp int;

f16vec4 tint_bitcast_to_f16(vec2 src) {
  uvec2 v = floatBitsToUint(src);
  f16vec2 v_1 = unpackFloat2x16(v.x);
  return f16vec4(v_1, unpackFloat2x16(v.y));
}
void main() {
  vec2 a = vec2(1.0f, 2.0f);
  f16vec4 x = tint_bitcast_to_f16(a);
}
)");
}

TEST_F(GlslWriterTest, BuiltinBitcast_Vec4F16ToVec2F32) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a = b.Let("a", b.Construct<vec4<f16>>(1_h, 2_h, 3_h, 4_h));
        b.Let("x", b.Bitcast<vec2<f32>>(a));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(#extension GL_AMD_gpu_shader_half_float: require
precision highp float;
precision highp int;

vec2 tint_bitcast_from_f16(f16vec4 src) {
  uint v = packFloat2x16(src.xy);
  return uintBitsToFloat(uvec2(v, packFloat2x16(src.zw)));
}
void main() {
  f16vec4 a = f16vec4(1.0hf, 2.0hf, 3.0hf, 4.0hf);
  vec2 x = tint_bitcast_from_f16(a);
}
)");
}

TEST_F(GlslWriterTest, BuiltinTextureDimensions_1d) {
    auto* t = b.FunctionParam(
        "t", ty.Get<core::type::SampledTexture>(core::type::TextureDimension::k1d, ty.f32()));
    auto* func = b.Function("foo", ty.u32());
    func->SetParams({t});
    b.Append(func->Block(), [&] {
        auto* result = b.Call<u32>(core::BuiltinFn::kTextureDimensions, t);
        b.Return(func, result);
    });

    Options opts{};
    opts.version = Version(Version::Standard::kDesktop, 4, 6);
    ASSERT_TRUE(Generate(opts)) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, R"(#version 460

uint foo(highp sampler1D t) {
  return uint(textureSize(t, 0));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
)");
}

TEST_F(GlslWriterTest, BuiltinTextureDimensions_2d_WithoutLod) {
    auto* t = b.FunctionParam(
        "t", ty.Get<core::type::SampledTexture>(core::type::TextureDimension::k2d, ty.f32()));
    auto* func = b.Function("foo", ty.vec2<u32>());
    func->SetParams({t});
    b.Append(func->Block(), [&] {
        auto* result = b.Call<vec2<u32>>(core::BuiltinFn::kTextureDimensions, t);
        b.Return(func, result);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(
uvec2 foo(highp sampler2D t) {
  return uvec2(textureSize(t, 0));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
)");
}

// TODO(dsinclair): Requires textureNumLevels
TEST_F(GlslWriterTest, DISABLED_BuiltinTextureDimensions_2d_WithU32Lod) {
    auto* t = b.FunctionParam(
        "t", ty.Get<core::type::SampledTexture>(core::type::TextureDimension::k2d, ty.f32()));
    auto* func = b.Function("foo", ty.vec2<u32>());
    func->SetParams({t});
    b.Append(func->Block(), [&] {
        auto* result = b.Call<vec2<u32>>(core::BuiltinFn::kTextureDimensions, t, 3_u);
        b.Return(func, result);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(
)");
}

TEST_F(GlslWriterTest, BuiltinTextureDimensions_2dArray) {
    auto* t = b.FunctionParam(
        "t", ty.Get<core::type::SampledTexture>(core::type::TextureDimension::k2dArray, ty.f32()));
    auto* func = b.Function("foo", ty.vec2<u32>());
    func->SetParams({t});
    b.Append(func->Block(), [&] {
        auto* result = b.Call<vec2<u32>>(core::BuiltinFn::kTextureDimensions, t);
        b.Return(func, result);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(
uvec2 foo(highp sampler2DArray t) {
  return uvec2(textureSize(t, 0).xy);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
)");
}

TEST_F(GlslWriterTest, BuiltinTextureDimensions_Storage1D) {
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

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(
uvec2 foo(highp readonly image2D t) {
  return uvec2(imageSize(t));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
)");
}

TEST_F(GlslWriterTest, BuiltinTextureDimensions_DepthMultisampled) {
    auto* t = b.FunctionParam(
        "t", ty.Get<core::type::DepthMultisampledTexture>(core::type::TextureDimension::k2d));
    auto* func = b.Function("foo", ty.vec2<u32>());
    func->SetParams({t});
    b.Append(func->Block(), [&] {
        auto* result = b.Call<vec2<u32>>(core::BuiltinFn::kTextureDimensions, t);
        b.Return(func, result);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(
uvec2 foo(highp sampler2DMS t) {
  return uvec2(textureSize(t));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
)");
}

TEST_F(GlslWriterTest, CountOneBits) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("x", b.Call(ty.u32(), core::BuiltinFn::kCountOneBits, 1_u));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

void main() {
  uint x = uint(bitCount(1u));
}
)");
}

TEST_F(GlslWriterTest, ExtractBits) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("x", b.Call(ty.u32(), core::BuiltinFn::kExtractBits, 1_u, 2_u, 3_u));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

void main() {
  uint v = min(2u, 32u);
  uint v_1 = min(3u, (32u - v));
  int v_2 = int(v);
  uint x = bitfieldExtract(1u, v_2, int(v_1));
}
)");
}

TEST_F(GlslWriterTest, InsertBits) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("x", b.Call(ty.u32(), core::BuiltinFn::kInsertBits, 1_u, 2_u, 3_u, 4_u));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

void main() {
  uint v = min(3u, 32u);
  uint v_1 = min(4u, (32u - v));
  int v_2 = int(v);
  uint x = bitfieldInsert(1u, 2u, v_2, int(v_1));
}
)");
}

}  // namespace
}  // namespace tint::glsl::writer
