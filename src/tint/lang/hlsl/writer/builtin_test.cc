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
#include "src/tint/lang/core/type/storage_texture.h"
#include "src/tint/lang/core/type/texture_dimension.h"
#include "src/tint/lang/hlsl/writer/helper_test.h"

#include "gtest/gtest.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::hlsl::writer {
namespace {

TEST_F(HlslWriterTest, BuiltinSelectScalar) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* x = b.Let("x", 1_i);
        auto* y = b.Let("y", 2_i);

        auto* c = b.Call(ty.i32(), core::BuiltinFn::kSelect, x, y, true);
        b.Let("w", c);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
void foo() {
  int x = 1;
  int y = 2;
  int w = ((true) ? (y) : (x));
}

)");
}

TEST_F(HlslWriterTest, BuiltinSelectVector) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* x = b.Let("x", b.Construct<vec2<i32>>(1_i, 2_i));
        auto* y = b.Let("y", b.Construct<vec2<i32>>(3_i, 4_i));
        auto* cmp = b.Construct<vec2<bool>>(true, false);

        auto* c = b.Call(ty.vec2<i32>(), core::BuiltinFn::kSelect, x, y, cmp);
        b.Let("w", c);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
void foo() {
  int2 x = int2(1, 2);
  int2 y = int2(3, 4);
  int2 w = ((bool2(true, false)) ? (y) : (x));
}

)");
}

TEST_F(HlslWriterTest, BuiltinTrunc) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* val = b.Var("v", b.Zero(ty.f32()));

        auto* v = b.Load(val);
        auto* t = b.Call(ty.f32(), core::BuiltinFn::kTrunc, v);

        b.Let("val", t);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
void foo() {
  float v = 0.0f;
  float v_1 = v;
  float v_2 = floor(v_1);
  float val = (((v_1 < 0.0f)) ? (ceil(v_1)) : (v_2));
}

)");
}

TEST_F(HlslWriterTest, BuiltinTruncVec) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* val = b.Var("v", b.Splat(ty.vec3<f32>(), 2_f));

        auto* v = b.Load(val);
        auto* t = b.Call(ty.vec3<f32>(), core::BuiltinFn::kTrunc, v);

        b.Let("val", t);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
void foo() {
  float3 v = (2.0f).xxx;
  float3 v_1 = v;
  float3 v_2 = floor(v_1);
  float3 val = (((v_1 < (0.0f).xxx)) ? (ceil(v_1)) : (v_2));
}

)");
}

TEST_F(HlslWriterTest, BuiltinTruncF16) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* val = b.Var("v", b.Zero(ty.f16()));

        auto* v = b.Load(val);
        auto* t = b.Call(ty.f16(), core::BuiltinFn::kTrunc, v);

        b.Let("val", t);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
void foo() {
  float16_t v = float16_t(0.0h);
  float16_t v_1 = v;
  float16_t v_2 = floor(v_1);
  float16_t val = (((v_1 < float16_t(0.0h))) ? (ceil(v_1)) : (v_2));
}

)");
}

TEST_F(HlslWriterTest, DISABLED_BuiltinStorageAtomicStore) {
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
        b.Call(ty.void_(), core::BuiltinFn::kAtomicStore,
               b.Access(ty.ptr<storage, atomic<i32>, read_write>(), var, 1_u), 123_i);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
)");
}

TEST_F(HlslWriterTest, DISABLED_BuiltinStorageAtomicLoad) {
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
        b.Let("x", b.Call(ty.i32(), core::BuiltinFn::kAtomicLoad,
                          b.Access(ty.ptr<storage, atomic<i32>, read_write>(), var, 1_u)));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
)");
}

TEST_F(HlslWriterTest, DISABLED_BuiltinStorageAtomicAdd) {
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
        b.Let("x", b.Call(ty.i32(), core::BuiltinFn::kAtomicAdd,
                          b.Access(ty.ptr<storage, atomic<i32>, read_write>(), var, 1_u), 123_i));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
)");
}

TEST_F(HlslWriterTest, DISABLED_BuiltinStorageAtomicSub) {
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
        b.Let("x", b.Call(ty.i32(), core::BuiltinFn::kAtomicSub,
                          b.Access(ty.ptr<storage, atomic<i32>, read_write>(), var, 1_u), 123_i));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
)");
}

TEST_F(HlslWriterTest, DISABLED_BuiltinStorageAtomicMax) {
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
        b.Let("x", b.Call(ty.i32(), core::BuiltinFn::kAtomicMax,
                          b.Access(ty.ptr<storage, atomic<i32>, read_write>(), var, 1_u), 123_i));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
)");
}

TEST_F(HlslWriterTest, DISABLED_BuiltinStorageAtomicMin) {
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
        b.Let("x", b.Call(ty.i32(), core::BuiltinFn::kAtomicMin,
                          b.Access(ty.ptr<storage, atomic<i32>, read_write>(), var, 1_u), 123_i));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
)");
}

TEST_F(HlslWriterTest, DISABLED_BuiltinStorageAtomicAnd) {
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
        b.Let("x", b.Call(ty.i32(), core::BuiltinFn::kAtomicAnd,
                          b.Access(ty.ptr<storage, atomic<i32>, read_write>(), var, 1_u), 123_i));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
)");
}

TEST_F(HlslWriterTest, DISABLED_BuiltinStorageAtomicOr) {
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
        b.Let("x", b.Call(ty.i32(), core::BuiltinFn::kAtomicOr,
                          b.Access(ty.ptr<storage, atomic<i32>, read_write>(), var, 1_u), 123_i));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
)");
}

TEST_F(HlslWriterTest, DISABLED_BuiltinStorageAtomicXor) {
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
        b.Let("x", b.Call(ty.i32(), core::BuiltinFn::kAtomicXor,
                          b.Access(ty.ptr<storage, atomic<i32>, read_write>(), var, 2_u), 123_i));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
)");
}

TEST_F(HlslWriterTest, DISABLED_BuiltinStorageAtomicExchange) {
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
        b.Let("x", b.Call(ty.i32(), core::BuiltinFn::kAtomicExchange,
                          b.Access(ty.ptr<storage, atomic<i32>, read_write>(), var, 2_u), 123_i));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
)");
}

TEST_F(HlslWriterTest, DISABLED_BuiltinStorageAtomicCompareExchangeWeak) {
    auto* sb = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("padding"), ty.vec4<f32>()},
                                                    {mod.symbols.New("a"), ty.atomic<i32>()},
                                                    {mod.symbols.New("b"), ty.atomic<u32>()},
                                                });

    auto* out = ty.Struct(
        mod.symbols.New("__atomic_compare_exchange_result"),
        {{mod.symbols.New("old_value"), ty.i32()}, {mod.symbols.New("exchanged"), ty.bool_()}});

    auto* var = b.Var("v", storage, sb, core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("x",
              b.Call(out, core::BuiltinFn::kAtomicCompareExchangeWeak,
                     b.Access(ty.ptr<storage, atomic<i32>, read_write>(), var, 1_u), 123_i, 345_i));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
)");
}

struct AtomicData {
    core::BuiltinFn fn;
    const char* interlock;
};
[[maybe_unused]] std::ostream& operator<<(std::ostream& out, const AtomicData& data) {
    out << data.interlock;
    return out;
}
using HlslBuiltinWorkgroupAtomic = HlslWriterTestWithParam<AtomicData>;
TEST_P(HlslBuiltinWorkgroupAtomic, Access) {
    auto param = GetParam();
    auto* var = b.Var("v", workgroup, ty.atomic<i32>(), core::Access::kReadWrite);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    func->SetWorkgroupSize(1, 1, 1);
    b.Append(func->Block(), [&] {
        b.Let("x", b.Call(ty.i32(), param.fn, var, 123_i));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(struct foo_inputs {
  uint tint_local_index : SV_GroupIndex;
};


groupshared int v;
void foo_inner(uint tint_local_index) {
  if ((tint_local_index == 0u)) {
    int v_1 = 0;
    InterlockedExchange(v, 0, v_1);
  }
  GroupMemoryBarrierWithGroupSync();
  int v_2 = 0;
  )" + std::string(param.interlock) +
                                R"((v, 123, v_2);
  int x = v_2;
}

[numthreads(1, 1, 1)]
void foo(foo_inputs inputs) {
  foo_inner(inputs.tint_local_index);
}

)");
}

INSTANTIATE_TEST_SUITE_P(HlslWriterTest,
                         HlslBuiltinWorkgroupAtomic,
                         testing::Values(AtomicData{core::BuiltinFn::kAtomicAdd, "InterlockedAdd"},
                                         AtomicData{core::BuiltinFn::kAtomicMax, "InterlockedMax"},
                                         AtomicData{core::BuiltinFn::kAtomicMin, "InterlockedMin"},
                                         AtomicData{core::BuiltinFn::kAtomicAnd, "InterlockedAnd"},
                                         AtomicData{core::BuiltinFn::kAtomicOr, "InterlockedOr"},
                                         AtomicData{core::BuiltinFn::kAtomicXor, "InterlockedXor"},
                                         AtomicData{core::BuiltinFn::kAtomicExchange,
                                                    "InterlockedExchange"}));

TEST_F(HlslWriterTest, BuiltinWorkgroupAtomicStore) {
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
        b.Call(ty.void_(), core::BuiltinFn::kAtomicStore,
               b.Access(ty.ptr<workgroup, atomic<i32>, read_write>(), var, 1_u), 123_i);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(struct SB {
  float4 padding;
  int a;
  uint b;
};

struct foo_inputs {
  uint tint_local_index : SV_GroupIndex;
};


groupshared SB v;
void foo_inner(uint tint_local_index) {
  if ((tint_local_index == 0u)) {
    v.padding = (0.0f).xxxx;
    int v_1 = 0;
    InterlockedExchange(v.a, 0, v_1);
    uint v_2 = 0u;
    InterlockedExchange(v.b, 0u, v_2);
  }
  GroupMemoryBarrierWithGroupSync();
  int v_3 = 0;
  InterlockedExchange(v.a, 123, v_3);
}

[numthreads(1, 1, 1)]
void foo(foo_inputs inputs) {
  foo_inner(inputs.tint_local_index);
}

)");
}

TEST_F(HlslWriterTest, BuiltinWorkgroupAtomicLoad) {
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

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(struct SB {
  float4 padding;
  int a;
  uint b;
};

struct foo_inputs {
  uint tint_local_index : SV_GroupIndex;
};


groupshared SB v;
void foo_inner(uint tint_local_index) {
  if ((tint_local_index == 0u)) {
    v.padding = (0.0f).xxxx;
    int v_1 = 0;
    InterlockedExchange(v.a, 0, v_1);
    uint v_2 = 0u;
    InterlockedExchange(v.b, 0u, v_2);
  }
  GroupMemoryBarrierWithGroupSync();
  int v_3 = 0;
  InterlockedOr(v.a, 0, v_3);
  int x = v_3;
}

[numthreads(1, 1, 1)]
void foo(foo_inputs inputs) {
  foo_inner(inputs.tint_local_index);
}

)");
}

TEST_F(HlslWriterTest, BuiltinWorkgroupAtomicSub) {
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
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(struct SB {
  float4 padding;
  int a;
  uint b;
};

struct foo_inputs {
  uint tint_local_index : SV_GroupIndex;
};


groupshared SB v;
void foo_inner(uint tint_local_index) {
  if ((tint_local_index == 0u)) {
    v.padding = (0.0f).xxxx;
    int v_1 = 0;
    InterlockedExchange(v.a, 0, v_1);
    uint v_2 = 0u;
    InterlockedExchange(v.b, 0u, v_2);
  }
  GroupMemoryBarrierWithGroupSync();
  int v_3 = 0;
  InterlockedAdd(v.a, -(123), v_3);
  int x = v_3;
}

[numthreads(1, 1, 1)]
void foo(foo_inputs inputs) {
  foo_inner(inputs.tint_local_index);
}

)");
}

TEST_F(HlslWriterTest, BuiltinWorkgroupAtomicCompareExchangeWeak) {
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

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(struct SB {
  float4 padding;
  int a;
  uint b;
};

struct atomic_compare_exchange_result_i32 {
  int old_value;
  bool exchanged;
};

struct foo_inputs {
  uint tint_local_index : SV_GroupIndex;
};


groupshared SB v;
void foo_inner(uint tint_local_index) {
  if ((tint_local_index == 0u)) {
    v.padding = (0.0f).xxxx;
    int v_1 = 0;
    InterlockedExchange(v.a, 0, v_1);
    uint v_2 = 0u;
    InterlockedExchange(v.b, 0u, v_2);
  }
  GroupMemoryBarrierWithGroupSync();
  int v_3 = 0;
  InterlockedCompareExchange(v.a, 123, 345, v_3);
  int v_4 = v_3;
  atomic_compare_exchange_result_i32 x = {v_4, (v_4 == 123)};
}

[numthreads(1, 1, 1)]
void foo(foo_inputs inputs) {
  foo_inner(inputs.tint_local_index);
}

)");
}

TEST_F(HlslWriterTest, BuiltinSignScalar) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("x", b.Call(ty.f16(), core::BuiltinFn::kSign, 1_h));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
void foo() {
  float16_t x = float16_t(sign(float16_t(1.0h)));
}

)");
}

TEST_F(HlslWriterTest, BuiltinSignVector) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("x", b.Call(ty.vec3<f32>(), core::BuiltinFn::kSign,
                          b.Composite(ty.vec3<f32>(), 1_f, 2_f, 3_f)));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
void foo() {
  float3 x = float3(sign(float3(1.0f, 2.0f, 3.0f)));
}

)");
}

TEST_F(HlslWriterTest, BuiltinStorageBarrier) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    func->SetWorkgroupSize(1, 1, 1);
    b.Append(func->Block(), [&] {
        b.Call(ty.void_(), core::BuiltinFn::kStorageBarrier);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
[numthreads(1, 1, 1)]
void foo() {
  DeviceMemoryBarrierWithGroupSync();
}

)");
}

TEST_F(HlslWriterTest, BuiltinTextureBarrier) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    func->SetWorkgroupSize(1, 1, 1);
    b.Append(func->Block(), [&] {
        b.Call(ty.void_(), core::BuiltinFn::kTextureBarrier);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
[numthreads(1, 1, 1)]
void foo() {
  DeviceMemoryBarrierWithGroupSync();
}

)");
}

TEST_F(HlslWriterTest, BuiltinWorkgroupBarrier) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    func->SetWorkgroupSize(1, 1, 1);
    b.Append(func->Block(), [&] {
        b.Call(ty.void_(), core::BuiltinFn::kWorkgroupBarrier);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
[numthreads(1, 1, 1)]
void foo() {
  GroupMemoryBarrierWithGroupSync();
}

)");
}

TEST_F(HlslWriterTest, BuiltinTextureNumLevels1D) {
    auto* t = b.FunctionParam(
        "t", ty.Get<core::type::SampledTexture>(core::type::TextureDimension::k1d, ty.f32()));

    auto* func = b.Function("foo", ty.void_());
    func->SetParams({t});

    b.Append(func->Block(), [&] {
        b.Let("d", b.Call(ty.u32(), core::BuiltinFn::kTextureNumLevels, t));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
void foo(Texture1D<float4> t) {
  uint2 v = (0u).xx;
  t.GetDimensions(0u, v[0u], v[1u]);
  uint d = v.y;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

)");
}

TEST_F(HlslWriterTest, BuiltinTextureNumLevels2D) {
    auto* t = b.FunctionParam(
        "t", ty.Get<core::type::SampledTexture>(core::type::TextureDimension::k2d, ty.f32()));

    auto* func = b.Function("foo", ty.void_());
    func->SetParams({t});

    b.Append(func->Block(), [&] {
        b.Let("d", b.Call(ty.u32(), core::BuiltinFn::kTextureNumLevels, t));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
void foo(Texture2D<float4> t) {
  uint3 v = (0u).xxx;
  t.GetDimensions(0u, v[0u], v[1u], v[2u]);
  uint d = v.z;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

)");
}

TEST_F(HlslWriterTest, BuiltinTextureNumLevels3D) {
    auto* t = b.FunctionParam(
        "t", ty.Get<core::type::SampledTexture>(core::type::TextureDimension::k3d, ty.f32()));

    auto* func = b.Function("foo", ty.void_());
    func->SetParams({t});

    b.Append(func->Block(), [&] {
        b.Let("d", b.Call(ty.u32(), core::BuiltinFn::kTextureNumLevels, t));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
void foo(Texture3D<float4> t) {
  uint4 v = (0u).xxxx;
  t.GetDimensions(0u, v[0u], v[1u], v[2u], v[3u]);
  uint d = v.w;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

)");
}

TEST_F(HlslWriterTest, BuiltinTextureDimension1D) {
    auto* t = b.FunctionParam(
        "t", ty.Get<core::type::SampledTexture>(core::type::TextureDimension::k1d, ty.f32()));

    auto* func = b.Function("foo", ty.void_());
    func->SetParams({t});

    b.Append(func->Block(), [&] {
        b.Let("d", b.Call(ty.u32(), core::BuiltinFn::kTextureDimensions, t));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
void foo(Texture1D<float4> t) {
  uint v = 0u;
  t.GetDimensions(v);
  uint d = v;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

)");
}

TEST_F(HlslWriterTest, BuiltinTextureDimension2D) {
    auto* t = b.FunctionParam(
        "t", ty.Get<core::type::SampledTexture>(core::type::TextureDimension::k2d, ty.f32()));

    auto* func = b.Function("foo", ty.void_());
    func->SetParams({t});

    b.Append(func->Block(), [&] {
        b.Let("d", b.Call(ty.vec2<u32>(), core::BuiltinFn::kTextureDimensions, t));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
void foo(Texture2D<float4> t) {
  uint2 v = (0u).xx;
  t.GetDimensions(v[0u], v[1u]);
  uint2 d = v;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

)");
}

TEST_F(HlslWriterTest, BuiltinTextureDimension2dLOD) {
    auto* t = b.FunctionParam(
        "t", ty.Get<core::type::SampledTexture>(core::type::TextureDimension::k2d, ty.f32()));

    auto* func = b.Function("foo", ty.void_());
    func->SetParams({t});

    b.Append(func->Block(), [&] {
        b.Let("d", b.Call(ty.vec2<u32>(), core::BuiltinFn::kTextureDimensions, t, 1_i));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
void foo(Texture2D<float4> t) {
  uint3 v = (0u).xxx;
  t.GetDimensions(0u, v[0u], v[1u], v[2u]);
  uint3 v_1 = (0u).xxx;
  t.GetDimensions(uint(min(uint(1), (v.z - 1u))), v_1[0u], v_1[1u], v_1[2u]);
  uint2 d = v_1.xy;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

)");
}

TEST_F(HlslWriterTest, BuiltinTextureDimension3D) {
    auto* t = b.FunctionParam(
        "t", ty.Get<core::type::SampledTexture>(core::type::TextureDimension::k3d, ty.f32()));

    auto* func = b.Function("foo", ty.void_());
    func->SetParams({t});

    b.Append(func->Block(), [&] {
        b.Let("d", b.Call(ty.vec3<u32>(), core::BuiltinFn::kTextureDimensions, t));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
void foo(Texture3D<float4> t) {
  uint3 v = (0u).xxx;
  t.GetDimensions(v[0u], v[1u], v[2u]);
  uint3 d = v;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

)");
}

TEST_F(HlslWriterTest, BuiltinTextureLayers2dArray) {
    auto* t = b.FunctionParam(
        "t", ty.Get<core::type::SampledTexture>(core::type::TextureDimension::k2dArray, ty.f32()));

    auto* func = b.Function("foo", ty.void_());
    func->SetParams({t});

    b.Append(func->Block(), [&] {
        b.Let("d", b.Call(ty.u32(), core::BuiltinFn::kTextureNumLayers, t));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
void foo(Texture2DArray<float4> t) {
  uint3 v = (0u).xxx;
  t.GetDimensions(v[0u], v[1u], v[2u]);
  uint d = v.z;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

)");
}

TEST_F(HlslWriterTest, BuiltinTextureNumLayersCubeArray) {
    auto* t = b.FunctionParam("t", ty.Get<core::type::SampledTexture>(
                                       core::type::TextureDimension::kCubeArray, ty.f32()));

    auto* func = b.Function("foo", ty.void_());
    func->SetParams({t});

    b.Append(func->Block(), [&] {
        b.Let("d", b.Call(ty.u32(), core::BuiltinFn::kTextureNumLayers, t));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
void foo(TextureCubeArray<float4> t) {
  uint3 v = (0u).xxx;
  t.GetDimensions(v[0u], v[1u], v[2u]);
  uint d = v.z;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

)");
}

TEST_F(HlslWriterTest, BuiltinTextureNumSamples) {
    auto* t = b.FunctionParam(
        "t", ty.Get<core::type::DepthMultisampledTexture>(core::type::TextureDimension::k2d));

    auto* func = b.Function("foo", ty.void_());
    func->SetParams({t});

    b.Append(func->Block(), [&] {
        b.Let("d", b.Call(ty.u32(), core::BuiltinFn::kTextureNumSamples, t));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
void foo(Texture2DMS<float4> t) {
  uint3 v = (0u).xxx;
  t.GetDimensions(v[0u], v[1u], v[2u]);
  uint d = v.z;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

)");
}

TEST_F(HlslWriterTest, BuiltinTextureLoad_1DF32) {
    auto* t = b.Var(ty.ptr(
        handle, ty.Get<core::type::SampledTexture>(core::type::TextureDimension::k1d, ty.f32())));
    t->SetBindingPoint(0, 0);
    b.ir.root_block->Append(t);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Value(1_u);
        auto* level = b.Value(3_u);
        b.Let("x", b.Call<vec4<f32>>(core::BuiltinFn::kTextureLoad, b.Load(t), coords, level));
        b.Return(func);
    });

    Options opts;
    opts.disable_robustness = true;
    ASSERT_TRUE(Generate(opts)) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
Texture1D<float4> v : register(t0);
void foo() {
  Texture1D<float4> v_1 = v;
  int v_2 = int(1u);
  float4 x = float4(v_1.Load(int2(v_2, int(3u))));
}

)");
}

TEST_F(HlslWriterTest, BuiltinTextureLoad_2DLevelI32) {
    auto* t = b.Var(ty.ptr(
        handle, ty.Get<core::type::SampledTexture>(core::type::TextureDimension::k2d, ty.i32())));
    t->SetBindingPoint(0, 0);
    b.ir.root_block->Append(t);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Composite(ty.vec2<u32>(), 1_u, 2_u);
        auto* level = b.Value(3_u);
        b.Let("x", b.Call<vec4<i32>>(core::BuiltinFn::kTextureLoad, b.Load(t), coords, level));
        b.Return(func);
    });

    Options opts;
    opts.disable_robustness = true;
    ASSERT_TRUE(Generate(opts)) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
Texture2D<int4> v : register(t0);
void foo() {
  Texture2D<int4> v_1 = v;
  int2 v_2 = int2(uint2(1u, 2u));
  int4 x = int4(v_1.Load(int3(v_2, int(3u))));
}

)");
}

TEST_F(HlslWriterTest, BuiltinTextureLoad_3DLevelU32) {
    auto* t = b.Var(ty.ptr(
        handle, ty.Get<core::type::SampledTexture>(core::type::TextureDimension::k3d, ty.f32())));
    t->SetBindingPoint(0, 0);
    b.ir.root_block->Append(t);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Composite(ty.vec3<i32>(), 1_i, 2_i, 3_i);
        auto* level = b.Value(4_u);
        b.Let("x", b.Call<vec4<f32>>(core::BuiltinFn::kTextureLoad, b.Load(t), coords, level));
        b.Return(func);
    });

    Options opts;
    opts.disable_robustness = true;
    ASSERT_TRUE(Generate(opts)) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
Texture3D<float4> v : register(t0);
void foo() {
  Texture3D<float4> v_1 = v;
  int3 v_2 = int3(int3(1, 2, 3));
  float4 x = float4(v_1.Load(int4(v_2, int(4u))));
}

)");
}

TEST_F(HlslWriterTest, BuiltinTextureLoad_Multisampled2DI32) {
    auto* t = b.Var(ty.ptr(handle, ty.Get<core::type::MultisampledTexture>(
                                       core::type::TextureDimension::k2d, ty.i32())));
    t->SetBindingPoint(0, 0);
    b.ir.root_block->Append(t);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Composite(ty.vec2<i32>(), 1_i, 2_i);
        auto* sample_idx = b.Value(3_i);
        b.Let("x", b.Call<vec4<i32>>(core::BuiltinFn::kTextureLoad, b.Load(t), coords, sample_idx));
        b.Return(func);
    });

    Options opts;
    opts.disable_robustness = true;
    ASSERT_TRUE(Generate(opts)) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
Texture2DMS<int4> v : register(t0);
void foo() {
  Texture2DMS<int4> v_1 = v;
  int2 v_2 = int2(int2(1, 2));
  int4 x = int4(v_1.Load(v_2, int(3)));
}

)");
}

TEST_F(HlslWriterTest, BuiltinTextureLoad_Depth2DLevelF32) {
    auto* t =
        b.Var(ty.ptr(handle, ty.Get<core::type::DepthTexture>(core::type::TextureDimension::k2d)));
    t->SetBindingPoint(0, 0);
    b.ir.root_block->Append(t);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec2<i32>(), b.Value(1_i), b.Value(2_i));
        auto* level = b.Value(3_u);
        b.Let("x", b.Call<f32>(core::BuiltinFn::kTextureLoad, b.Load(t), coords, level));
        b.Return(func);
    });

    Options opts;
    opts.disable_robustness = true;
    ASSERT_TRUE(Generate(opts)) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
Texture2D v : register(t0);
void foo() {
  int2 v_1 = int2(1, 2);
  Texture2D v_2 = v;
  int2 v_3 = int2(v_1);
  float x = v_2.Load(int3(v_3, int(3u))).x;
}

)");
}

TEST_F(HlslWriterTest, BuiltinTextureLoad_Depth2DArrayLevelF32) {
    auto* t = b.Var(
        ty.ptr(handle, ty.Get<core::type::DepthTexture>(core::type::TextureDimension::k2dArray)));
    t->SetBindingPoint(0, 0);
    b.ir.root_block->Append(t);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Composite(ty.vec2<i32>(), 1_i, 2_i);
        auto* array_idx = b.Value(3_u);
        auto* sample_idx = b.Value(4_i);
        b.Let("x",
              b.Call<f32>(core::BuiltinFn::kTextureLoad, b.Load(t), coords, array_idx, sample_idx));
        b.Return(func);
    });

    Options opts;
    opts.disable_robustness = true;
    ASSERT_TRUE(Generate(opts)) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
Texture2DArray v : register(t0);
void foo() {
  Texture2DArray v_1 = v;
  int2 v_2 = int2(int2(1, 2));
  int v_3 = int(3u);
  float x = v_1.Load(int4(v_2, v_3, int(4))).x;
}

)");
}

TEST_F(HlslWriterTest, BuiltinTextureLoad_DepthMultisampledF32) {
    auto* t = b.Var(ty.ptr(
        handle, ty.Get<core::type::DepthMultisampledTexture>(core::type::TextureDimension::k2d)));
    t->SetBindingPoint(0, 0);
    b.ir.root_block->Append(t);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Composite(ty.vec2<i32>(), 1_i, 2_i);
        auto* sample_idx = b.Value(3_u);
        b.Let("x", b.Call<f32>(core::BuiltinFn::kTextureLoad, b.Load(t), coords, sample_idx));
        b.Return(func);
    });

    Options opts;
    opts.disable_robustness = true;
    ASSERT_TRUE(Generate(opts)) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
Texture2DMS<float4> v : register(t0);
void foo() {
  Texture2DMS<float4> v_1 = v;
  int2 v_2 = int2(int2(1, 2));
  float x = v_1.Load(v_2, int(3u)).x;
}

)");
}

TEST_F(HlslWriterTest, BuiltinTextureStore1D) {
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

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
RWTexture1D<float4> v : register(u0);
void foo() {
  v[1] = float4(0.5f, 0.0f, 0.0f, 1.0f);
}

)");
}

TEST_F(HlslWriterTest, BuiltinTextureStore3D) {
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

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
RWTexture3D<float4> v : register(u0);
void foo() {
  v[int3(1, 2, 3)] = float4(0.5f, 0.0f, 0.0f, 1.0f);
}

)");
}

TEST_F(HlslWriterTest, BuiltinTextureStoreArray) {
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

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
RWTexture2DArray<float4> v : register(u0);
void foo() {
  RWTexture2DArray<float4> v_1 = v;
  v_1[int3(int2(1, 2), int(3u))] = float4(0.5f, 0.40000000596046447754f, 0.30000001192092895508f, 1.0f);
}

)");
}

TEST_F(HlslWriterTest, BuiltinQuantizeToF16) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* v = b.Var("x", b.Zero(ty.vec2<f32>()));
        b.Let("a", b.Call(ty.vec2<f32>(), core::BuiltinFn::kQuantizeToF16, b.Load(v)));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
void foo() {
  float2 x = (0.0f).xx;
  float2 a = f16tof32(f32tof16(x));
}

)");
}

TEST_F(HlslWriterTest, BuiltinUnpack2x16Float) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* u = b.Var("u", 2_u);
        b.Let("a", b.Call(ty.vec2<f32>(), core::BuiltinFn::kUnpack2X16Float, b.Load(u)));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
void foo() {
  uint u = 2u;
  uint v = u;
  float2 a = f16tof32(uint2((v & 65535u), (v >> 16u)));
}

)");
}

TEST_F(HlslWriterTest, BuiltinUnpack2x16Snorm) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* u = b.Var("u", 2_u);
        b.Let("a", b.Call(ty.vec2<f32>(), core::BuiltinFn::kUnpack2X16Snorm, b.Load(u)));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
void foo() {
  uint u = 2u;
  int v = int(u);
  float2 a = clamp((float2((int2((v << 16u), v) >> (16u).xx)) / 32767.0f), (-1.0f).xx, (1.0f).xx);
}

)");
}

TEST_F(HlslWriterTest, BuiltinUnpack2x16Unorm) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* u = b.Var("u", 2_u);
        b.Let("a", b.Call(ty.vec2<f32>(), core::BuiltinFn::kUnpack2X16Unorm, b.Load(u)));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
void foo() {
  uint u = 2u;
  uint v = u;
  float2 a = (float2(uint2((v & 65535u), (v >> 16u))) / 65535.0f);
}

)");
}

TEST_F(HlslWriterTest, BuiltinUnpack4x8Snorm) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* u = b.Var("u", 2_u);
        b.Let("a", b.Call(ty.vec4<f32>(), core::BuiltinFn::kUnpack4X8Snorm, b.Load(u)));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
void foo() {
  uint u = 2u;
  int v = int(u);
  float4 a = clamp((float4((int4((v << 24u), (v << 16u), (v << 8u), v) >> (24u).xxxx)) / 127.0f), (-1.0f).xxxx, (1.0f).xxxx);
}

)");
}

TEST_F(HlslWriterTest, BuiltinUnpack4x8Unorm) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* u = b.Var("u", 2_u);
        b.Let("a", b.Call(ty.vec4<f32>(), core::BuiltinFn::kUnpack4X8Unorm, b.Load(u)));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
void foo() {
  uint u = 2u;
  uint v = u;
  float4 a = (float4(uint4((v & 255u), ((v >> 8u) & 255u), ((v >> 16u) & 255u), (v >> 24u))) / 255.0f);
}

)");
}

TEST_F(HlslWriterTest, BuiltinUnpack4xI8CorePolyfill) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* u = b.Var("u", 2_u);
        b.Let("a", b.Call(ty.vec4<i32>(), core::BuiltinFn::kUnpack4XI8, b.Load(u)));
        b.Return(func);
    });

    Options opts{};
    opts.polyfill_pack_unpack_4x8 = true;
    ASSERT_TRUE(Generate(opts)) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
void foo() {
  uint u = 2u;
  uint v = u;
  uint4 v_1 = uint4(24u, 16u, 8u, 0u);
  int4 v_2 = asint((uint4((v).xxxx) << v_1));
  int4 a = (v_2 >> uint4((24u).xxxx));
}

)");
}

TEST_F(HlslWriterTest, BuiltinUnpack4xI8) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* u = b.Var("u", 2_u);
        b.Let("a", b.Call(ty.vec4<i32>(), core::BuiltinFn::kUnpack4XI8, b.Load(u)));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
void foo() {
  uint u = 2u;
  int4 a = unpack_s8s32(int8_t4_packed(u));
}

)");
}

TEST_F(HlslWriterTest, BuiltinUnpack4xU8CorePolyfill) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* u = b.Var("u", 2_u);
        b.Let("a", b.Call(ty.vec4<u32>(), core::BuiltinFn::kUnpack4XU8, b.Load(u)));
        b.Return(func);
    });

    Options opts{};
    opts.polyfill_pack_unpack_4x8 = true;
    ASSERT_TRUE(Generate(opts)) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
void foo() {
  uint u = 2u;
  uint v = u;
  uint4 v_1 = uint4(0u, 8u, 16u, 24u);
  uint4 v_2 = (uint4((v).xxxx) >> v_1);
  uint4 a = (v_2 & uint4((255u).xxxx));
}

)");
}

TEST_F(HlslWriterTest, BuiltinUnpack4xU8) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* u = b.Var("u", 2_u);
        b.Let("a", b.Call(ty.vec4<u32>(), core::BuiltinFn::kUnpack4XU8, b.Load(u)));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
void foo() {
  uint u = 2u;
  uint4 a = unpack_u8u32(uint8_t4_packed(u));
}

)");
}

}  // namespace
}  // namespace tint::hlsl::writer
