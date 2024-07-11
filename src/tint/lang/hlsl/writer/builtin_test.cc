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

TEST_F(HlslWriterTest, DISABLED_BuiltinWorkgroupAtomicStore) {
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

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
)");
}

TEST_F(HlslWriterTest, DISABLED_BuiltinWorkgroupAtomicLoad) {
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

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
)");
}

TEST_F(HlslWriterTest, DISABLED_BuiltinWorkgroupAtomicAdd) {
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
        b.Let("x", b.Call(ty.i32(), core::BuiltinFn::kAtomicAdd,
                          b.Access(ty.ptr<workgroup, atomic<i32>, read_write>(), var, 1_u), 123_i));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
)");
}

TEST_F(HlslWriterTest, DISABLED_BuiltinWorkgroupAtomicSub) {
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

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
)");
}

TEST_F(HlslWriterTest, DISABLED_BuiltinWorkgroupAtomicMax) {
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
        b.Let("x", b.Call(ty.i32(), core::BuiltinFn::kAtomicMax,
                          b.Access(ty.ptr<workgroup, atomic<i32>, read_write>(), var, 1_u), 123_i));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
)");
}

TEST_F(HlslWriterTest, DISABLED_BuiltinWorkgroupAtomicMin) {
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
        b.Let("x", b.Call(ty.i32(), core::BuiltinFn::kAtomicMin,
                          b.Access(ty.ptr<workgroup, atomic<i32>, read_write>(), var, 1_u), 123_i));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
)");
}

TEST_F(HlslWriterTest, DISABLED_BuiltinWorkgroupAtomicAnd) {
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
        b.Let("x", b.Call(ty.i32(), core::BuiltinFn::kAtomicAnd,
                          b.Access(ty.ptr<workgroup, atomic<i32>, read_write>(), var, 1_u), 123_i));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
)");
}

TEST_F(HlslWriterTest, DISABLED_BuiltinWorkgroupAtomicOr) {
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
        b.Let("x", b.Call(ty.i32(), core::BuiltinFn::kAtomicOr,
                          b.Access(ty.ptr<workgroup, atomic<i32>, read_write>(), var, 1_u), 123_i));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
)");
}

TEST_F(HlslWriterTest, DISABLED_BuiltinWorkgroupAtomicXor) {
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
        b.Let("x", b.Call(ty.i32(), core::BuiltinFn::kAtomicXor,
                          b.Access(ty.ptr<workgroup, atomic<i32>, read_write>(), var, 2_u), 123_i));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
)");
}

TEST_F(HlslWriterTest, DISABLED_BuiltinWorkgroupAtomicExchange) {
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
        b.Let("x", b.Call(ty.i32(), core::BuiltinFn::kAtomicExchange,
                          b.Access(ty.ptr<workgroup, atomic<i32>, read_write>(), var, 2_u), 123_i));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
)");
}

TEST_F(HlslWriterTest, DISABLED_BuiltinWorkgroupAtomicCompareExchangeWeak) {
    auto* sb = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("padding"), ty.vec4<f32>()},
                                                    {mod.symbols.New("a"), ty.atomic<i32>()},
                                                    {mod.symbols.New("b"), ty.atomic<u32>()},
                                                });

    auto* out = ty.Struct(
        mod.symbols.New("__atomic_compare_exchange_result"),
        {{mod.symbols.New("old_value"), ty.i32()}, {mod.symbols.New("exchanged"), ty.bool_()}});

    auto* var = b.Var("v", workgroup, sb, core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("x", b.Call(out, core::BuiltinFn::kAtomicCompareExchangeWeak,
                          b.Access(ty.ptr<workgroup, atomic<i32>, read_write>(), var, 1_u), 123_i,
                          345_i));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
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

}  // namespace
}  // namespace tint::hlsl::writer
