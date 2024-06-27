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

#include <utility>

#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/hlsl/writer/helper_test.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::hlsl::writer {
namespace {

TEST_F(HlslWriterTest, ConstantBoolFalse) {
    auto* f = b.Function("a", ty.bool_());
    f->Block()->Append(b.Return(f, false));

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
bool a() {
  return false;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

)");
}

TEST_F(HlslWriterTest, ConstantBoolTrue) {
    auto* f = b.Function("a", ty.bool_());
    f->Block()->Append(b.Return(f, true));

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
bool a() {
  return true;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

)");
}

TEST_F(HlslWriterTest, ConstantInt) {
    auto* f = b.Function("a", ty.i32());
    f->Block()->Append(b.Return(f, -12345_i));

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
int a() {
  return -12345;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

)");
}

TEST_F(HlslWriterTest, ConstantUInt) {
    auto* f = b.Function("a", ty.u32());
    f->Block()->Append(b.Return(f, 56779_u));

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
uint a() {
  return 56779u;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

)");
}

TEST_F(HlslWriterTest, ConstantFloat) {
    auto* f = b.Function("a", ty.f32());
    // Use a number close to 1<<30 but whose decimal representation ends in 0.
    f->Block()->Append(b.Return(f, f32((1 << 30) - 4)));

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
float a() {
  return 1073741824.0f;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

)");
}

TEST_F(HlslWriterTest, ConstantF16) {
    auto* f = b.Function("a", ty.f16());
    // Use a number close to 1<<16 but whose decimal representation ends in 0.
    f->Block()->Append(b.Return(f, f16((1 << 15) - 8)));

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
float16_t a() {
  return float16_t(32752.0h);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

)");
}

TEST_F(HlslWriterTest, ConstantTypeVecF32) {
    auto* f = b.Function("a", ty.vec3<f32>());
    b.Append(f->Block(), [&] { b.Return(f, b.Composite(ty.vec3<f32>(), 1_f, 2_f, 3_f)); });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
float3 a() {
  return float3(1.0f, 2.0f, 3.0f);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

)");
}

TEST_F(HlslWriterTest, ConstantTypeVecF16) {
    auto* f = b.Function("a", ty.vec3<f16>());
    b.Append(f->Block(), [&] { b.Return(f, b.Composite(ty.vec3<f16>(), 1_h, 2_h, 3_h)); });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
vector<float16_t, 3> a() {
  return vector<float16_t, 3>(float16_t(1.0h), float16_t(2.0h), float16_t(3.0h));
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

)");
}

TEST_F(HlslWriterTest, ConstantTypeVecEmptyF32) {
    auto* f = b.Function("a", ty.vec3<f32>());
    b.Append(f->Block(), [&] { b.Return(f, b.Zero<vec3<f32>>()); });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
float3 a() {
  return (0.0f).xxx;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

)");
}

TEST_F(HlslWriterTest, ConstantTypeVecEmptyF16) {
    auto* f = b.Function("a", ty.vec3<f16>());
    b.Append(f->Block(), [&] { b.Return(f, b.Zero<vec3<f16>>()); });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
vector<float16_t, 3> a() {
  return (float16_t(0.0h)).xxx;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

)");
}

TEST_F(HlslWriterTest, ConstantTypeVecSingleScalarF32Literal) {
    auto* f = b.Function("a", ty.vec3<f32>());
    b.Append(f->Block(), [&] { b.Return(f, b.Splat(ty.vec3<f32>(), 2_f)); });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
float3 a() {
  return (2.0f).xxx;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

)");
}

TEST_F(HlslWriterTest, ConstantTypeVecSingleScalarF16Literal) {
    auto* f = b.Function("a", ty.vec3<f16>());
    b.Append(f->Block(), [&] { b.Return(f, b.Splat(ty.vec3<f16>(), 2_h)); });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
vector<float16_t, 3> a() {
  return (float16_t(2.0h)).xxx;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

)");
}

TEST_F(HlslWriterTest, ConstantTypeVecSingleScalarBoolLiteral) {
    auto* f = b.Function("a", ty.vec3<bool>());
    b.Append(f->Block(), [&] { b.Return(f, b.Splat(ty.vec3<bool>(), true)); });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
bool3 a() {
  return (true).xxx;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

)");
}

TEST_F(HlslWriterTest, ConstantTypeVecSingleScalarInt) {
    auto* f = b.Function("a", ty.vec3<i32>());
    b.Append(f->Block(), [&] { b.Return(f, b.Splat(ty.vec3<i32>(), 2_i)); });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
int3 a() {
  return (2).xxx;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

)");
}

TEST_F(HlslWriterTest, ConstantTypeVecSingleScalarUInt) {
    auto* f = b.Function("a", ty.vec3<u32>());
    b.Append(f->Block(), [&] { b.Return(f, b.Splat(ty.vec3<u32>(), 2_u)); });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
uint3 a() {
  return (2u).xxx;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

)");
}

TEST_F(HlslWriterTest, ConstantTypeMatF32) {
    auto* f = b.Function("a", ty.mat2x3<f32>());
    b.Append(f->Block(), [&] {
        b.Return(f, b.Composite(ty.mat2x3<f32>(), b.Composite(ty.vec3<f32>(), 1_f, 2_f, 3_f),
                                b.Composite(ty.vec3<f32>(), 3_f, 4_f, 5_f)));
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
float2x3 a() {
  return float2x3(float3(1.0f, 2.0f, 3.0f), float3(3.0f, 4.0f, 5.0f));
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

)");
}

TEST_F(HlslWriterTest, ConstantTypeMatF16) {
    auto* f = b.Function("a", ty.mat2x3<f16>());
    b.Append(f->Block(), [&] {
        b.Return(f, b.Composite(ty.mat2x3<f16>(), b.Composite(ty.vec3<f16>(), 1_h, 2_h, 3_h),
                                b.Composite(ty.vec3<f16>(), 3_h, 4_h, 5_h)));
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
matrix<float16_t, 2, 3> a() {
  return matrix<float16_t, 2, 3>(vector<float16_t, 3>(float16_t(1.0h), float16_t(2.0h), float16_t(3.0h)), vector<float16_t, 3>(float16_t(3.0h), float16_t(4.0h), float16_t(5.0h)));
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

)");
}

TEST_F(HlslWriterTest, ConstantTypeMatComplexF32) {
    // mat4x4<f32>(
    //     vec4<f32>(2.0f, 3.0f, 4.0f, 8.0f),
    //     vec4<f32>(),
    //     vec4<f32>(7.0f),
    //     vec4<f32>(42.0f, 21.0f, 6.0f, -5.0f),
    //   );
    auto* f = b.Function("a", ty.mat4x4<f32>());
    b.Append(f->Block(), [&] {
        b.Return(f, b.Composite(ty.mat4x4<f32>(), b.Composite(ty.vec4<f32>(), 2_f, 3_f, 4_f, 8_f),
                                b.Zero<vec4<f32>>(), b.Splat(ty.vec4<f32>(), 7_f),
                                b.Composite(ty.vec4<f32>(), 42_f, 21_f, 6_f, -5_f)));
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
float4x4 a() {
  return float4x4(float4(2.0f, 3.0f, 4.0f, 8.0f), (0.0f).xxxx, (7.0f).xxxx, float4(42.0f, 21.0f, 6.0f, -5.0f));
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

)");
}

TEST_F(HlslWriterTest, ConstantTypeMatComplexF16) {
    // mat4x4<f16>(
    //     vec4<f16>(2.h, 3.h, 4.h, 8.h),
    //     vec4<f16>(),
    //     vec4<f16>(7.0h),
    //     vec4<f16>(42.0h, 21.0h, 6.0h, -5.0h),
    //   );
    auto* f = b.Function("a", ty.mat4x4<f16>());
    b.Append(f->Block(), [&] {
        b.Return(f, b.Composite(ty.mat4x4<f16>(), b.Composite(ty.vec4<f16>(), 2_h, 3_h, 4_h, 8_h),
                                b.Zero<vec4<f16>>(), b.Splat(ty.vec4<f16>(), 7_h),
                                b.Composite(ty.vec4<f16>(), 42_h, 21_h, 6_h, -5_h)));
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
matrix<float16_t, 4, 4> a() {
  return matrix<float16_t, 4, 4>(vector<float16_t, 4>(float16_t(2.0h), float16_t(3.0h), float16_t(4.0h), float16_t(8.0h)), (float16_t(0.0h)).xxxx, (float16_t(7.0h)).xxxx, vector<float16_t, 4>(float16_t(42.0h), float16_t(21.0h), float16_t(6.0h), float16_t(-5.0h)));
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

)");
}

TEST_F(HlslWriterTest, ConstantTypeMatEmptyF32) {
    auto* f = b.Function("a", ty.mat2x3<f32>());
    b.Append(f->Block(), [&] { b.Return(f, b.Zero<mat2x3<f32>>()); });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
float2x3 a() {
  return float2x3((0.0f).xxx, (0.0f).xxx);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

)");
}

TEST_F(HlslWriterTest, ConstantTypeMatEmptyF16) {
    auto* f = b.Function("a", ty.mat2x3<f16>());
    b.Append(f->Block(), [&] { b.Return(f, b.Zero<mat2x3<f16>>()); });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
matrix<float16_t, 2, 3> a() {
  return matrix<float16_t, 2, 3>((float16_t(0.0h)).xxx, (float16_t(0.0h)).xxx);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

)");
}

TEST_F(HlslWriterTest, ConstantTypeMatIdentityF32) {
    // fn f() {
    //  var m_1: mat4x4<f32> = mat4x4<f32>();
    //  var m_2: mat4x4<f32> = mat4x4<f32>(m_1);
    // }

    auto* f = b.Function("a", ty.void_());
    b.Append(f->Block(), [&] {
        auto* m1 = b.Var("m_1", b.Zero<mat4x4<f32>>());
        b.Var("m_2", b.Load(m1));
        b.Return(f);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
void a() {
  float4x4 m_1 = float4x4((0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx);
  float4x4 m_2 = m_1;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

)");
}

TEST_F(HlslWriterTest, ConstantTypeMatIdentityF16) {
    // fn f() {
    //   var m_1: mat4x4<f16> = mat4x4<f16>();
    //   var m_2: mat4x4<f16> = mat4x4<f16>(m_1);
    // }

    auto* f = b.Function("a", ty.void_());
    b.Append(f->Block(), [&] {
        auto* m1 = b.Var("m_1", b.Zero<mat4x4<f16>>());
        b.Var("m_2", b.Load(m1));
        b.Return(f);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
void a() {
  matrix<float16_t, 4, 4> m_1 = matrix<float16_t, 4, 4>((float16_t(0.0h)).xxxx, (float16_t(0.0h)).xxxx, (float16_t(0.0h)).xxxx, (float16_t(0.0h)).xxxx);
  matrix<float16_t, 4, 4> m_2 = m_1;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

)");
}

TEST_F(HlslWriterTest, ConstantTypeArrayFunctionReturn) {
    auto* f = b.Function("a", ty.array<vec3<f32>, 3>());
    b.Append(f->Block(), [&] {
        b.Return(f,
                 b.Composite(ty.array<vec3<f32>, 3>(), b.Composite(ty.vec3<f32>(), 1_f, 2_f, 3_f),
                             b.Composite(ty.vec3<f32>(), 4_f, 5_f, 6_f),
                             b.Composite(ty.vec3<f32>(), 7_f, 8_f, 9_f)));
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
typedef float3 ary_ret[3];
ary_ret a() {
  float3 v[3] = {float3(1.0f, 2.0f, 3.0f), float3(4.0f, 5.0f, 6.0f), float3(7.0f, 8.0f, 9.0f)};
  return v;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

)");
}

TEST_F(HlslWriterTest, ConstantTypeArrayEmptyFunctionReturn) {
    auto* f = b.Function("a", ty.array<vec3<f32>, 3>());
    b.Append(f->Block(), [&] { b.Return(f, b.Zero<array<vec3<f32>, 3>>()); });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
typedef float3 ary_ret[3];
ary_ret a() {
  float3 v[3] = (float3[3])0;
  return v;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

)");
}

TEST_F(HlslWriterTest, ConstantTypeArray) {
    auto* f = b.Function("a", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    f->SetWorkgroupSize(1, 1, 1);

    b.Append(f->Block(), [&] {
        b.Var("v", b.Composite(ty.array<vec3<f32>, 3>(), b.Composite(ty.vec3<f32>(), 1_f, 2_f, 3_f),
                               b.Composite(ty.vec3<f32>(), 4_f, 5_f, 6_f),
                               b.Composite(ty.vec3<f32>(), 7_f, 8_f, 9_f)));
        b.Return(f);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
[numthreads(1, 1, 1)]
void a() {
  float3 v[3] = {float3(1.0f, 2.0f, 3.0f), float3(4.0f, 5.0f, 6.0f), float3(7.0f, 8.0f, 9.0f)};
}

)");
}

TEST_F(HlslWriterTest, ConstantTypeArrayEmpty) {
    auto* f = b.Function("a", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    f->SetWorkgroupSize(1, 1, 1);

    b.Append(f->Block(), [&] {
        b.Var("v", b.Zero<array<vec3<f32>, 3>>());
        b.Return(f);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
[numthreads(1, 1, 1)]
void a() {
  float3 v[3] = (float3[3])0;
}

)");
}

TEST_F(HlslWriterTest, ConstantTypeStructNestedEmpty) {
    Vector members_a{
        ty.Get<core::type::StructMember>(b.ir.symbols.New("d"), ty.i32(), 0u, 0u, 4u, 4u,
                                         core::type::StructMemberAttributes{}),
        ty.Get<core::type::StructMember>(b.ir.symbols.New("e"), ty.f32(), 1u, 4u, 4u, 4u,
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

    auto* f = b.Function("a", s_strct);
    b.Append(f->Block(), [&] { b.Return(f, b.Zero(s_strct)); });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(struct A {
  int d;
  float e;
};

struct S {
  int a;
  float b;
  A c;
};


S a() {
  S v = (S)0;
  return v;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

)");
}

TEST_F(HlslWriterTest, ConstantTypeStructNested) {
    Vector members_a{
        ty.Get<core::type::StructMember>(b.ir.symbols.New("e"), ty.vec4<f32>(), 0u, 0u, 16u, 16u,
                                         core::type::StructMemberAttributes{}),
    };
    auto* a_strct = ty.Struct(b.ir.symbols.New("A"), std::move(members_a));

    Vector members_s{
        ty.Get<core::type::StructMember>(b.ir.symbols.New("c"), a_strct, 0u, 0u, 16u, 16u,
                                         core::type::StructMemberAttributes{}),
    };
    auto* s_strct = ty.Struct(b.ir.symbols.New("S"), std::move(members_s));

    auto* f = b.Function("a", s_strct);
    b.Append(f->Block(), [&] {
        b.Return(f, b.Construct(s_strct, b.Construct(a_strct, b.Splat(ty.vec4<f32>(), 1_f))));
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(struct A {
  float4 e;
};

struct S {
  A c;
};


S a() {
  A v = {(1.0f).xxxx};
  S v_1 = {v};
  return v_1;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

)");
}

TEST_F(HlslWriterTest, ConstantTypeLetStructComposite) {
    Vector members_a{
        ty.Get<core::type::StructMember>(b.ir.symbols.New("e"), ty.vec4<f32>(), 0u, 0u, 16u, 16u,
                                         core::type::StructMemberAttributes{}),
    };
    auto* a_strct = ty.Struct(b.ir.symbols.New("A"), std::move(members_a));

    Vector members_s{
        ty.Get<core::type::StructMember>(b.ir.symbols.New("c"), a_strct, 0u, 0u, 16u, 16u,
                                         core::type::StructMemberAttributes{}),
    };
    auto* s_strct = ty.Struct(b.ir.symbols.New("S"), std::move(members_s));

    auto* f = b.Function("a", ty.f32());
    b.Append(f->Block(), [&] {
        b.Let("z", b.Composite(s_strct, b.Composite(a_strct, b.Splat(ty.vec4<f32>(), 1_f))));
        b.Return(f, 1_f);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(struct A {
  float4 e;
};

struct S {
  A c;
};


float a() {
  S z = {{(1.0f).xxxx}};
  return 1.0f;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

)");
}

// TODO(dsinclair): Need support for `static const` variables
TEST_F(HlslWriterTest, DISABLED_ConstantTypeLetStructCompositeModuleScoped) {
    Vector members_a{
        ty.Get<core::type::StructMember>(b.ir.symbols.New("e"), ty.vec4<f32>(), 0u, 0u, 16u, 16u,
                                         core::type::StructMemberAttributes{}),
    };
    auto* a_strct = ty.Struct(b.ir.symbols.New("A"), std::move(members_a));

    Vector members_s{
        ty.Get<core::type::StructMember>(b.ir.symbols.New("c"), a_strct, 0u, 0u, 16u, 16u,
                                         core::type::StructMemberAttributes{}),
    };
    auto* s_strct = ty.Struct(b.ir.symbols.New("S"), std::move(members_s));

    b.ir.root_block->Append(b.Var<private_>(
        "z", b.Composite(s_strct, b.Composite(a_strct, b.Splat(ty.vec4<f32>(), 1_f)))));

    auto* f = b.Function("a", ty.f32());
    b.Append(f->Block(), [&] {
        b.Var<function>("t",
                        b.Composite(s_strct, b.Composite(a_strct, b.Splat(ty.vec4<f32>(), 1_f))));
        b.Return(f, 1_f);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(struct A {
  float4 e;
};

struct S {
  A c;
};

static const A c_1 = {(1.f).xxxx};
static const S c_2 = {c_1};
static S z = c_2;
float a() {
  S t = {{(1.0f).xxxx}};
  return 1.0f;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

)");
}

TEST_F(HlslWriterTest, ConstantTypeStructEmpty) {
    Vector members{
        ty.Get<core::type::StructMember>(b.ir.symbols.New("a"), ty.i32(), 0u, 0u, 4u, 4u,
                                         core::type::StructMemberAttributes{}),
        ty.Get<core::type::StructMember>(b.ir.symbols.New("b"), ty.f32(), 1u, 4u, 4u, 4u,
                                         core::type::StructMemberAttributes{}),
        ty.Get<core::type::StructMember>(b.ir.symbols.New("c"), ty.vec3<i32>(), 2u, 8u, 16u, 16u,
                                         core::type::StructMemberAttributes{}),
        ty.Get<core::type::StructMember>(b.ir.symbols.New("d"), ty.array<f32, 3>(), 2u, 8u, 16u,
                                         16u, core::type::StructMemberAttributes{}),
    };
    auto* strct = ty.Struct(b.ir.symbols.New("S"), std::move(members));

    auto* f = b.Function("a", strct);
    b.Append(f->Block(), [&] { b.Return(f, b.Zero(strct)); });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(struct S {
  int a;
  float b;
  int3 c;
  float d[3];
};


S a() {
  S v = (S)0;
  return v;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

)");
}

TEST_F(HlslWriterTest, ConstantTypeStruct) {
    Vector members{
        ty.Get<core::type::StructMember>(b.ir.symbols.New("a"), ty.i32(), 0u, 0u, 4u, 4u,
                                         core::type::StructMemberAttributes{}),
        ty.Get<core::type::StructMember>(b.ir.symbols.New("b"), ty.f32(), 1u, 4u, 4u, 4u,
                                         core::type::StructMemberAttributes{}),
        ty.Get<core::type::StructMember>(b.ir.symbols.New("c"), ty.vec3<i32>(), 2u, 16u, 16u, 16u,
                                         core::type::StructMemberAttributes{}),
        ty.Get<core::type::StructMember>(b.ir.symbols.New("d"), ty.vec4<f32>(), 2u, 32u, 16u, 16u,
                                         core::type::StructMemberAttributes{}),
    };
    auto* strct = ty.Struct(b.ir.symbols.New("S"), std::move(members));

    auto* f = b.Function("a", strct);
    b.Append(f->Block(), [&] {
        b.Return(f, b.Construct(strct, 1_i, 1_f, b.Splat(ty.vec3<i32>(), 2_i),
                                b.Splat(ty.vec4<f32>(), 3_f)));
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(struct S {
  int a;
  float b;
  int3 c;
  float4 d;
};


S a() {
  S v = {1, 1.0f, (2).xxx, (3.0f).xxxx};
  return v;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

)");
}

TEST_F(HlslWriterTest, ConstantTypeLetStruct) {
    Vector members{
        ty.Get<core::type::StructMember>(b.ir.symbols.New("a"), ty.i32(), 0u, 0u, 4u, 4u,
                                         core::type::StructMemberAttributes{}),
        ty.Get<core::type::StructMember>(b.ir.symbols.New("b"), ty.f32(), 1u, 4u, 4u, 4u,
                                         core::type::StructMemberAttributes{}),
        ty.Get<core::type::StructMember>(b.ir.symbols.New("c"), ty.vec3<i32>(), 2u, 16u, 16u, 16u,
                                         core::type::StructMemberAttributes{}),
        ty.Get<core::type::StructMember>(b.ir.symbols.New("d"), ty.vec4<f32>(), 2u, 32u, 16u, 16u,
                                         core::type::StructMemberAttributes{}),
    };
    auto* strct = ty.Struct(b.ir.symbols.New("S"), std::move(members));

    auto* f = b.Function("a", ty.f32());
    b.Append(f->Block(), [&] {
        b.Let("z", b.Construct(strct, 1_i, 1_f, b.Splat(ty.vec3<i32>(), 2_i),
                               b.Splat(ty.vec4<f32>(), 3_f)));
        b.Return(f, 1_f);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(struct S {
  int a;
  float b;
  int3 c;
  float4 d;
};


float a() {
  S z = {1, 1.0f, (2).xxx, (3.0f).xxxx};
  return 1.0f;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

)");
}

TEST_F(HlslWriterTest, ConstantTypeStructStaticEmpty) {
    Vector members{
        ty.Get<core::type::StructMember>(b.ir.symbols.New("a"), ty.i32(), 0u, 0u, 4u, 4u,
                                         core::type::StructMemberAttributes{}),
    };
    auto* strct = ty.Struct(b.ir.symbols.New("S"), std::move(members));

    b.Append(b.ir.root_block, [&] { b.Var<private_>("p", b.Zero(strct)); });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(struct S {
  int a;
};


static
S p = (S)0;
[numthreads(1, 1, 1)]
void unused_entry_point() {
}

)");
}

// TODO(dsinclair): Need suppport for `static const` variables
TEST_F(HlslWriterTest, DISABLED_ConstantTypeStructStatic) {
    Vector members{
        ty.Get<core::type::StructMember>(b.ir.symbols.New("a"), ty.i32(), 0u, 0u, 4u, 4u,
                                         core::type::StructMemberAttributes{}),
    };
    auto* strct = ty.Struct(b.ir.symbols.New("S"), std::move(members));

    b.Append(b.ir.root_block, [&] { b.Var<private_>("p", b.Composite(strct, 3_i)); });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(struct S {
  int a;
};


static const
S p = {3};
[numthreads(1, 1, 1)]
void unused_entry_point() {
}

)");
}

}  // namespace
}  // namespace tint::hlsl::writer
