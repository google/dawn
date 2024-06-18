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

// TODO(dsinclair): Needs `LoadVectorElement`
TEST_F(HlslWriterTest, DISABLED_AccessVector) {
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
  float3 v = (float3)0;
  float x = v[1u];
}

)");
}

// TODO(dsinclair): Needs `LoadVectorElement`
TEST_F(HlslWriterTest, DISABLED_AccessMatrix) {
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
  float3 v = (float3)0;
  float x = v[1u][2u];
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
  float x = v.c.e[1];
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

}  // namespace
}  // namespace tint::hlsl::writer
