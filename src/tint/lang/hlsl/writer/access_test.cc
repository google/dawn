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

#include "src/tint/lang/hlsl/writer/helper_test.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::hlsl::writer {
namespace {

TEST_F(HlslWriterTest, AccessArray) {
    auto* func = b.Function("a", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    func->SetWorkgroupSize(1, 1, 1);

    b.Append(func->Block(), [&] {
        auto* v = b.Var("v", b.Zero<array<f32, 3>>());
        b.Let("x", b.Load(b.Access(ty.ptr<function, f32>(), v, 1_u)));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
[numthreads(1, 1, 1)]
void a() {
  float v[3] = (float[3])0;
  float x = v[1u];
}

)");
}

TEST_F(HlslWriterTest, AccessStruct) {
    Vector members{
        ty.Get<core::type::StructMember>(b.ir.symbols.New("a"), ty.i32(), 0u, 0u, 4u, 4u,
                                         core::type::StructMemberAttributes{}),
        ty.Get<core::type::StructMember>(b.ir.symbols.New("b"), ty.f32(), 1u, 4u, 4u, 4u,
                                         core::type::StructMemberAttributes{}),
    };
    auto* strct = ty.Struct(b.ir.symbols.New("S"), std::move(members));

    auto* f = b.Function("a", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    f->SetWorkgroupSize(1, 1, 1);

    b.Append(f->Block(), [&] {
        auto* v = b.Var("v", b.Zero(strct));
        b.Let("x", b.Load(b.Access(ty.ptr<function, f32>(), v, 1_u)));
        b.Return(f);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(struct S {
  int a;
  float b;
};


[numthreads(1, 1, 1)]
void a() {
  S v = (S)0;
  float x = v.b;
}

)");
}

TEST_F(HlslWriterTest, AccessVector) {
    auto* func = b.Function("a", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    func->SetWorkgroupSize(1, 1, 1);

    b.Append(func->Block(), [&] {
        auto* v = b.Var("v", b.Zero<vec3<f32>>());
        b.Let("x", b.LoadVectorElement(v, 1_u));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
[numthreads(1, 1, 1)]
void a() {
  float3 v = (0.0f).xxx;
  float x = v[1u];
}

)");
}

TEST_F(HlslWriterTest, AccessMatrix) {
    auto* func = b.Function("a", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    func->SetWorkgroupSize(1, 1, 1);

    b.Append(func->Block(), [&] {
        auto* v = b.Var("v", b.Zero<mat4x4<f32>>());
        auto* v1 = b.Access(ty.ptr<function, vec4<f32>>(), v, 1_u);
        b.Let("x", b.LoadVectorElement(v1, 2_u));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
[numthreads(1, 1, 1)]
void a() {
  float4x4 v = float4x4((0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx);
  float x = v[1u][2u];
}

)");
}

TEST_F(HlslWriterTest, AccessStoreVectorElementConstantIndex) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* vec_var = b.Var("vec", ty.ptr<function, vec4<i32>>());
        b.StoreVectorElement(vec_var, 1_u, b.Constant(42_i));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
void foo() {
  int4 vec = (0).xxxx;
  vec[1u] = 42;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

)");
}

TEST_F(HlslWriterTest, AccessStoreVectorElementDynamicIndex) {
    auto* idx = b.FunctionParam("idx", ty.i32());
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({idx});
    b.Append(func->Block(), [&] {
        auto* vec_var = b.Var("vec", ty.ptr<function, vec4<i32>>());
        b.StoreVectorElement(vec_var, idx, b.Constant(42_i));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
void foo(int idx) {
  int4 vec = (0).xxxx;
  vec[min(uint(idx), 3u)] = 42;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

)");
}

TEST_F(HlslWriterTest, AccessNested) {
    Vector members_a{
        ty.Get<core::type::StructMember>(b.ir.symbols.New("d"), ty.i32(), 0u, 0u, 4u, 4u,
                                         core::type::StructMemberAttributes{}),
        ty.Get<core::type::StructMember>(b.ir.symbols.New("e"), ty.array<f32, 3>(), 1u, 4u, 4u, 4u,
                                         core::type::StructMemberAttributes{}),
    };
    auto* a_strct = ty.Struct(b.ir.symbols.New("A"), std::move(members_a));

    Vector members_s{
        ty.Get<core::type::StructMember>(b.ir.symbols.New("a"), ty.i32(), 0u, 0u, 4u, 4u,
                                         core::type::StructMemberAttributes{}),
        ty.Get<core::type::StructMember>(b.ir.symbols.New("b"), ty.f32(), 1u, 4u, 4u, 4u,
                                         core::type::StructMemberAttributes{}),
        ty.Get<core::type::StructMember>(b.ir.symbols.New("c"), a_strct, 2u, 8u, 8u, 8u,
                                         core::type::StructMemberAttributes{}),
    };
    auto* s_strct = ty.Struct(b.ir.symbols.New("S"), std::move(members_s));

    auto* f = b.Function("a", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    f->SetWorkgroupSize(1, 1, 1);

    b.Append(f->Block(), [&] {
        auto* v = b.Var("v", b.Zero(s_strct));
        b.Let("x", b.Load(b.Access(ty.ptr<function, f32>(), v, 2_u, 1_u, 1_i)));
        b.Return(f);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(struct A {
  int d;
  float e[3];
};

struct S {
  int a;
  float b;
  A c;
};


[numthreads(1, 1, 1)]
void a() {
  S v = (S)0;
  float x = v.c.e[1u];
}

)");
}

TEST_F(HlslWriterTest, AccessSwizzle) {
    auto* f = b.Function("a", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    f->SetWorkgroupSize(1, 1, 1);

    b.Append(f->Block(), [&] {
        auto* v = b.Var("v", b.Zero<vec3<f32>>());
        b.Let("b", b.Swizzle(ty.f32(), v, {1u}));
        b.Return(f);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
[numthreads(1, 1, 1)]
void a() {
  float3 v = (0.0f).xxx;
  float b = v.y;
}

)");
}

TEST_F(HlslWriterTest, AccessSwizzleMulti) {
    auto* f = b.Function("a", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    f->SetWorkgroupSize(1, 1, 1);

    b.Append(f->Block(), [&] {
        auto* v = b.Var("v", b.Zero<vec4<f32>>());
        b.Let("b", b.Swizzle(ty.vec4<f32>(), v, {3u, 2u, 1u, 0u}));
        b.Return(f);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
[numthreads(1, 1, 1)]
void a() {
  float4 v = (0.0f).xxxx;
  float4 b = v.wzyx;
}

)");
}

TEST_F(HlslWriterTest, AccessStorageVector) {
    auto* var = b.Var<storage, vec4<f32>, core::Access::kRead>("v");
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("a", b.Load(var));
        b.Let("b", b.LoadVectorElement(var, 0_u));
        b.Let("c", b.LoadVectorElement(var, 1_u));
        b.Let("d", b.LoadVectorElement(var, 2_u));
        b.Let("e", b.LoadVectorElement(var, 3_u));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
ByteAddressBuffer v : register(t0);
void foo() {
  float4 a = asfloat(v.Load4(0u));
  float b = asfloat(v.Load(0u));
  float c = asfloat(v.Load(4u));
  float d = asfloat(v.Load(8u));
  float e = asfloat(v.Load(12u));
}

)");
}

TEST_F(HlslWriterTest, AccessStorageVectorF16) {
    auto* var = b.Var<storage, vec4<f16>, core::Access::kRead>("v");
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("a", b.Load(var));
        b.Let("b", b.LoadVectorElement(var, 0_u));
        b.Let("c", b.LoadVectorElement(var, 1_u));
        b.Let("d", b.LoadVectorElement(var, 2_u));
        b.Let("e", b.LoadVectorElement(var, 3_u));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
ByteAddressBuffer v : register(t0);
void foo() {
  vector<float16_t, 4> a = v.Load4<vector<float16_t, 4>>(0u);
  float16_t b = v.Load<float16_t>(0u);
  float16_t c = v.Load<float16_t>(2u);
  float16_t d = v.Load<float16_t>(4u);
  float16_t e = v.Load<float16_t>(6u);
}

)");
}

TEST_F(HlslWriterTest, AccessStorageMatrix) {
    auto* var = b.Var<storage, mat4x4<f32>, core::Access::kRead>("v");
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("a", b.Load(var));
        b.Let("b", b.Load(b.Access(ty.ptr<storage, vec4<f32>, core::Access::kRead>(), var, 3_u)));
        b.Let("c", b.LoadVectorElement(
                       b.Access(ty.ptr<storage, vec4<f32>, core::Access::kRead>(), var, 1_u), 2_u));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
ByteAddressBuffer v : register(t0);
float4x4 v_1(uint offset) {
  float4 v_2 = asfloat(v.Load4((offset + 0u)));
  float4 v_3 = asfloat(v.Load4((offset + 16u)));
  float4 v_4 = asfloat(v.Load4((offset + 32u)));
  return float4x4(v_2, v_3, v_4, asfloat(v.Load4((offset + 48u))));
}

void foo() {
  float4x4 a = v_1(0u);
  float4 b = asfloat(v.Load4(48u));
  float c = asfloat(v.Load(24u));
}

)");
}

TEST_F(HlslWriterTest, AccessStorageArray) {
    auto* var = b.Var<storage, array<vec3<f32>, 5>, core::Access::kRead>("v");
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("a", b.Load(var));
        b.Let("b", b.Load(b.Access(ty.ptr<storage, vec3<f32>, core::Access::kRead>(), var, 3_u)));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
ByteAddressBuffer v : register(t0);
typedef float3 ary_ret[5];
ary_ret v_1(uint offset) {
  float3 a[5] = (float3[5])0;
  {
    uint v_2 = 0u;
    v_2 = 0u;
    while(true) {
      uint v_3 = v_2;
      if ((v_3 >= 5u)) {
        break;
      }
      a[v_3] = asfloat(v.Load3((offset + (v_3 * 16u))));
      {
        v_2 = (v_3 + 1u);
      }
      continue;
    }
  }
  float3 v_4[5] = a;
  return v_4;
}

void foo() {
  float3 a[5] = v_1(0u);
  float3 b = asfloat(v.Load3(48u));
}

)");
}

TEST_F(HlslWriterTest, AccessStorageStruct) {
    auto* SB = ty.Struct(mod.symbols.New("SB"),
                         {
                             {mod.symbols.New("a"), ty.i32(), core::type::StructMemberAttributes{}},
                             {mod.symbols.New("b"), ty.f32(), core::type::StructMemberAttributes{}},
                         });

    auto* var = b.Var("v", storage, SB, core::Access::kRead);
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("a", b.Load(var));
        b.Let("b", b.Load(b.Access(ty.ptr<storage, f32, core::Access::kRead>(), var, 1_u)));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(struct SB {
  int a;
  float b;
};


ByteAddressBuffer v : register(t0);
SB v_1(uint offset) {
  int v_2 = asint(v.Load((offset + 0u)));
  SB v_3 = {v_2, asfloat(v.Load((offset + 4u)))};
  return v_3;
}

void foo() {
  SB a = v_1(0u);
  float b = asfloat(v.Load(4u));
}

)");
}

TEST_F(HlslWriterTest, AccessStorageNested) {
    auto* Inner = ty.Struct(
        mod.symbols.New("Inner"),
        {
            {mod.symbols.New("s"), ty.mat3x3<f32>(), core::type::StructMemberAttributes{}},
            {mod.symbols.New("t"), ty.array<vec3<f32>, 5>(), core::type::StructMemberAttributes{}},
        });
    auto* Outer =
        ty.Struct(mod.symbols.New("Outer"),
                  {
                      {mod.symbols.New("x"), ty.f32(), core::type::StructMemberAttributes{}},
                      {mod.symbols.New("y"), Inner, core::type::StructMemberAttributes{}},
                  });

    auto* SB = ty.Struct(mod.symbols.New("SB"),
                         {
                             {mod.symbols.New("a"), ty.i32(), core::type::StructMemberAttributes{}},
                             {mod.symbols.New("b"), Outer, core::type::StructMemberAttributes{}},
                         });

    auto* var = b.Var("v", storage, SB, core::Access::kRead);
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("a", b.Load(var));
        b.Let("b", b.LoadVectorElement(b.Access(ty.ptr<storage, vec3<f32>, core::Access::kRead>(),
                                                var, 1_u, 1_u, 1_u, 3_u),
                                       2_u));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(struct Inner {
  float3x3 s;
  float3 t[5];
};

struct Outer {
  float x;
  Inner y;
};

struct SB {
  int a;
  Outer b;
};


ByteAddressBuffer v : register(t0);
typedef float3 ary_ret[5];
ary_ret v_1(uint offset) {
  float3 a[5] = (float3[5])0;
  {
    uint v_2 = 0u;
    v_2 = 0u;
    while(true) {
      uint v_3 = v_2;
      if ((v_3 >= 5u)) {
        break;
      }
      a[v_3] = asfloat(v.Load3((offset + (v_3 * 16u))));
      {
        v_2 = (v_3 + 1u);
      }
      continue;
    }
  }
  float3 v_4[5] = a;
  return v_4;
}

float3x3 v_5(uint offset) {
  float3 v_6 = asfloat(v.Load3((offset + 0u)));
  float3 v_7 = asfloat(v.Load3((offset + 16u)));
  return float3x3(v_6, v_7, asfloat(v.Load3((offset + 32u))));
}

Inner v_8(uint offset) {
  float3x3 v_9 = v_5((offset + 0u));
  float3 v_10[5] = v_1((offset + 48u));
  Inner v_11 = {v_9, v_10};
  return v_11;
}

Outer v_12(uint offset) {
  float v_13 = asfloat(v.Load((offset + 0u)));
  Inner v_14 = v_8((offset + 16u));
  Outer v_15 = {v_13, v_14};
  return v_15;
}

SB v_16(uint offset) {
  int v_17 = asint(v.Load((offset + 0u)));
  Outer v_18 = v_12((offset + 16u));
  SB v_19 = {v_17, v_18};
  return v_19;
}

void foo() {
  SB a = v_16(0u);
  float b = asfloat(v.Load(136u));
}

)");
}

TEST_F(HlslWriterTest, AccessStorageStoreVector) {
    auto* var = b.Var<storage, vec4<f32>, core::Access::kReadWrite>("v");
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.StoreVectorElement(var, 0_u, 2_f);
        b.StoreVectorElement(var, 1_u, 4_f);
        b.StoreVectorElement(var, 2_u, 8_f);
        b.StoreVectorElement(var, 3_u, 16_f);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
RWByteAddressBuffer v : register(u0);
void foo() {
  v.Store(0u, asuint(2.0f));
  v.Store(4u, asuint(4.0f));
  v.Store(8u, asuint(8.0f));
  v.Store(12u, asuint(16.0f));
}

)");
}

TEST_F(HlslWriterTest, AccessDirectVariable) {
    auto* var1 = b.Var<uniform, vec4<f32>, core::Access::kRead>("v1");
    var1->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var1);

    auto* var2 = b.Var<uniform, vec4<f32>, core::Access::kRead>("v2");
    var2->SetBindingPoint(0, 1);
    b.ir.root_block->Append(var2);

    auto* p = b.FunctionParam("x", ty.ptr<uniform, vec4<f32>, core::Access::kRead>());
    auto* bar = b.Function("bar", ty.void_());
    bar->SetParams({p});
    b.Append(bar->Block(), [&] {
        b.Let("a", b.LoadVectorElement(p, 1_u));
        b.Return(bar);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Call(bar, var1);
        b.Call(bar, var2);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
cbuffer cbuffer_v1 : register(b0) {
  uint4 v1[1];
};
cbuffer cbuffer_v2 : register(b1) {
  uint4 v2[1];
};
void bar() {
  float a = v1[1u];
}

void bar_1() {
  float a = v2[1u];
}

void foo() {
  bar();
  bar_1();
}

)");
}

TEST_F(HlslWriterTest, AccessChainFromUnnamedAccessChain) {
    auto* Inner =
        ty.Struct(mod.symbols.New("Inner"),
                  {
                      {mod.symbols.New("c"), ty.f32(), core::type::StructMemberAttributes{}},
                      {mod.symbols.New("d"), ty.u32(), core::type::StructMemberAttributes{}},
                  });
    auto* sb = ty.Struct(mod.symbols.New("SB"),
                         {
                             {mod.symbols.New("a"), ty.i32(), core::type::StructMemberAttributes{}},
                             {mod.symbols.New("b"), Inner, core::type::StructMemberAttributes{}},
                         });

    auto* var = b.Var("v", storage, sb, core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* x = b.Access(ty.ptr(storage, sb, core::Access::kReadWrite), var);
        auto* y = b.Access(ty.ptr(storage, Inner, core::Access::kReadWrite), x->Result(0), 1_u);
        b.Let("b", b.Load(b.Access(ty.ptr(storage, ty.u32(), core::Access::kReadWrite),
                                   y->Result(0), 1_u)));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
RWByteAddressBuffer v : register(u0);
void foo() {
  uint b = v.Load(8u);
}

)");
}

TEST_F(HlslWriterTest, AccessChainFromLetAccessChain) {
    auto* Inner =
        ty.Struct(mod.symbols.New("Inner"),
                  {
                      {mod.symbols.New("c"), ty.f32(), core::type::StructMemberAttributes{}},
                  });
    auto* sb = ty.Struct(mod.symbols.New("SB"),
                         {
                             {mod.symbols.New("a"), ty.i32(), core::type::StructMemberAttributes{}},
                             {mod.symbols.New("b"), Inner, core::type::StructMemberAttributes{}},
                         });

    auto* var = b.Var("v", storage, sb, core::Access::kReadWrite);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* x = b.Let("x", var);
        auto* y = b.Let(
            "y", b.Access(ty.ptr(storage, Inner, core::Access::kReadWrite), x->Result(0), 1_u));
        auto* z = b.Let(
            "z", b.Access(ty.ptr(storage, ty.f32(), core::Access::kReadWrite), y->Result(0), 0_u));
        b.Let("a", b.Load(z));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
RWByteAddressBuffer v : register(u0);
void foo() {
  float a = asfloat(v.Load(4u));
}

)");
}

}  // namespace
}  // namespace tint::hlsl::writer
