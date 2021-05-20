// Copyright 2020 The Tint Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "gmock/gmock.h"
#include "src/ast/stage_decoration.h"
#include "src/ast/struct_block_decoration.h"
#include "src/writer/hlsl/test_helper.h"

namespace tint {
namespace writer {
namespace hlsl {
namespace {

using ::testing::HasSubstr;

using create_type_func_ptr =
    ast::Type* (*)(const ProgramBuilder::TypesBuilder& ty);

inline ast::Type* ty_i32(const ProgramBuilder::TypesBuilder& ty) {
  return ty.i32();
}
inline ast::Type* ty_u32(const ProgramBuilder::TypesBuilder& ty) {
  return ty.u32();
}
inline ast::Type* ty_f32(const ProgramBuilder::TypesBuilder& ty) {
  return ty.f32();
}
template <typename T>
inline ast::Type* ty_vec2(const ProgramBuilder::TypesBuilder& ty) {
  return ty.vec2<T>();
}
template <typename T>
inline ast::Type* ty_vec3(const ProgramBuilder::TypesBuilder& ty) {
  return ty.vec3<T>();
}
template <typename T>
inline ast::Type* ty_vec4(const ProgramBuilder::TypesBuilder& ty) {
  return ty.vec4<T>();
}
template <typename T>
inline ast::Type* ty_mat2x2(const ProgramBuilder::TypesBuilder& ty) {
  return ty.mat2x2<T>();
}
template <typename T>
inline ast::Type* ty_mat2x3(const ProgramBuilder::TypesBuilder& ty) {
  return ty.mat2x3<T>();
}
template <typename T>
inline ast::Type* ty_mat2x4(const ProgramBuilder::TypesBuilder& ty) {
  return ty.mat2x4<T>();
}
template <typename T>
inline ast::Type* ty_mat3x2(const ProgramBuilder::TypesBuilder& ty) {
  return ty.mat3x2<T>();
}
template <typename T>
inline ast::Type* ty_mat3x3(const ProgramBuilder::TypesBuilder& ty) {
  return ty.mat3x3<T>();
}
template <typename T>
inline ast::Type* ty_mat3x4(const ProgramBuilder::TypesBuilder& ty) {
  return ty.mat3x4<T>();
}
template <typename T>
inline ast::Type* ty_mat4x2(const ProgramBuilder::TypesBuilder& ty) {
  return ty.mat4x2<T>();
}
template <typename T>
inline ast::Type* ty_mat4x3(const ProgramBuilder::TypesBuilder& ty) {
  return ty.mat4x3<T>();
}
template <typename T>
inline ast::Type* ty_mat4x4(const ProgramBuilder::TypesBuilder& ty) {
  return ty.mat4x4<T>();
}

using i32 = ProgramBuilder::i32;
using u32 = ProgramBuilder::u32;
using f32 = ProgramBuilder::f32;

template <typename BASE>
class HlslGeneratorImplTest_MemberAccessorBase : public BASE {
 public:
  void SetupStorageBuffer(ast::StructMemberList members) {
    ProgramBuilder& b = *this;

    auto* s =
        b.Structure("Data", members, {b.create<ast::StructBlockDecoration>()});

    auto* ac_ty = b.ty.access(ast::AccessControl::kReadWrite, s);

    b.Global("data", ac_ty, ast::StorageClass::kStorage, nullptr,
             ast::DecorationList{
                 b.create<ast::BindingDecoration>(0),
                 b.create<ast::GroupDecoration>(1),
             });
  }

  void SetupFunction(ast::StatementList statements) {
    ProgramBuilder& b = *this;
    b.Func("main", ast::VariableList{}, b.ty.void_(), statements,
           ast::DecorationList{
               b.Stage(ast::PipelineStage::kFragment),
           });
  }
};

using HlslGeneratorImplTest_MemberAccessor =
    HlslGeneratorImplTest_MemberAccessorBase<TestHelper>;

template <typename T>
using HlslGeneratorImplTest_MemberAccessorWithParam =
    HlslGeneratorImplTest_MemberAccessorBase<TestParamHelper<T>>;

TEST_F(HlslGeneratorImplTest_MemberAccessor, EmitExpression_MemberAccessor) {
  auto* s = Structure("Data", {Member("mem", ty.f32())});
  Global("str", s, ast::StorageClass::kPrivate);

  auto* expr = MemberAccessor("str", "mem");
  WrapInFunction(Var("expr", ty.f32(), ast::StorageClass::kNone, expr));

  GeneratorImpl& gen = SanitizeAndBuild();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_EQ(result(), R"(struct Data {
  float mem;
};

static Data str;

[numthreads(1, 1, 1)]
void test_function() {
  float expr = str.mem;
  return;
}

)");
}

struct TypeCase {
  create_type_func_ptr member_type;
  std::string expected;
};
inline std::ostream& operator<<(std::ostream& out, TypeCase c) {
  ProgramBuilder b;
  auto* ty = c.member_type(b.ty);
  out << ty->FriendlyName(b.Symbols());
  return out;
}

using HlslGeneratorImplTest_MemberAccessor_StorageBufferLoad =
    HlslGeneratorImplTest_MemberAccessorWithParam<TypeCase>;
TEST_P(HlslGeneratorImplTest_MemberAccessor_StorageBufferLoad, Test) {
  // struct Data {
  //   a : i32;
  //   b : <type>;
  // };
  // var<storage> data : Data;
  // data.b;

  auto p = GetParam();

  SetupStorageBuffer({
      Member("a", ty.i32()),
      Member("b", p.member_type(ty)),
  });

  SetupFunction({
      Decl(Var("x", nullptr, ast::StorageClass::kNone,
               MemberAccessor("data", "b"))),
  });

  GeneratorImpl& gen = SanitizeAndBuild();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_THAT(result(), HasSubstr(p.expected));

  Validate();
}

INSTANTIATE_TEST_SUITE_P(
    HlslGeneratorImplTest_MemberAccessor,
    HlslGeneratorImplTest_MemberAccessor_StorageBufferLoad,
    testing::Values(
        TypeCase{ty_u32, "data.Load(4u)"},
        TypeCase{ty_f32, "asfloat(data.Load(4u))"},
        TypeCase{ty_i32, "asint(data.Load(4u))"},
        TypeCase{ty_vec2<u32>, "data.Load2(8u)"},
        TypeCase{ty_vec2<f32>, "asfloat(data.Load2(8u))"},
        TypeCase{ty_vec2<i32>, "asint(data.Load2(8u))"},
        TypeCase{ty_vec3<u32>, "data.Load3(16u)"},
        TypeCase{ty_vec3<f32>, "asfloat(data.Load3(16u))"},
        TypeCase{ty_vec3<i32>, "asint(data.Load3(16u))"},
        TypeCase{ty_vec4<u32>, "data.Load4(16u)"},
        TypeCase{ty_vec4<f32>, "asfloat(data.Load4(16u))"},
        TypeCase{ty_vec4<i32>, "asint(data.Load4(16u))"},
        TypeCase{
            ty_mat2x2<u32>,
            R"(return uint2x2(buffer.Load2((offset + 0u)), buffer.Load2((offset + 8u)));)"},
        TypeCase{
            ty_mat2x3<f32>,
            R"(return float2x3(asfloat(buffer.Load3((offset + 0u))), asfloat(buffer.Load3((offset + 16u))));)"},
        TypeCase{
            ty_mat2x4<i32>,
            R"(return int2x4(asint(buffer.Load4((offset + 0u))), asint(buffer.Load4((offset + 16u))));)"},
        TypeCase{
            ty_mat3x2<u32>,
            R"(return uint3x2(buffer.Load2((offset + 0u)), buffer.Load2((offset + 8u)), buffer.Load2((offset + 16u)));)"},
        TypeCase{
            ty_mat3x3<f32>,
            R"(return float3x3(asfloat(buffer.Load3((offset + 0u))), asfloat(buffer.Load3((offset + 16u))), asfloat(buffer.Load3((offset + 32u))));)"},
        TypeCase{
            ty_mat3x4<i32>,
            R"(return int3x4(asint(buffer.Load4((offset + 0u))), asint(buffer.Load4((offset + 16u))), asint(buffer.Load4((offset + 32u))));)"},
        TypeCase{
            ty_mat4x2<u32>,
            R"(return uint4x2(buffer.Load2((offset + 0u)), buffer.Load2((offset + 8u)), buffer.Load2((offset + 16u)), buffer.Load2((offset + 24u)));)"},
        TypeCase{
            ty_mat4x3<f32>,
            R"(return float4x3(asfloat(buffer.Load3((offset + 0u))), asfloat(buffer.Load3((offset + 16u))), asfloat(buffer.Load3((offset + 32u))), asfloat(buffer.Load3((offset + 48u))));)"},
        TypeCase{
            ty_mat4x4<i32>,
            R"(return int4x4(asint(buffer.Load4((offset + 0u))), asint(buffer.Load4((offset + 16u))), asint(buffer.Load4((offset + 32u))), asint(buffer.Load4((offset + 48u))));)"}));

using HlslGeneratorImplTest_MemberAccessor_StorageBufferStore =
    HlslGeneratorImplTest_MemberAccessorWithParam<TypeCase>;
TEST_P(HlslGeneratorImplTest_MemberAccessor_StorageBufferStore, Test) {
  // struct Data {
  //   a : i32;
  //   b : <type>;
  // };
  // var<storage> data : Data;
  // data.b = <type>();

  auto p = GetParam();

  SetupStorageBuffer({
      Member("a", ty.i32()),
      Member("b", p.member_type(ty)),
  });

  SetupFunction({
      Decl(Var("value", p.member_type(ty), ast::StorageClass::kNone,
               Construct(p.member_type(ty)))),
      Assign(MemberAccessor("data", "b"), Expr("value")),
  });

  GeneratorImpl& gen = SanitizeAndBuild();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_THAT(result(), HasSubstr(p.expected));

  Validate();
}

INSTANTIATE_TEST_SUITE_P(
    HlslGeneratorImplTest_MemberAccessor,
    HlslGeneratorImplTest_MemberAccessor_StorageBufferStore,
    testing::Values(TypeCase{ty_u32, "data.Store(4u, asuint(value))"},
                    TypeCase{ty_f32, "data.Store(4u, asuint(value))"},
                    TypeCase{ty_i32, "data.Store(4u, asuint(value))"},
                    TypeCase{ty_vec2<u32>, "data.Store2(8u, asuint(value))"},
                    TypeCase{ty_vec2<f32>, "data.Store2(8u, asuint(value))"},
                    TypeCase{ty_vec2<i32>, "data.Store2(8u, asuint(value))"},
                    TypeCase{ty_vec3<u32>, "data.Store3(16u, asuint(value))"},
                    TypeCase{ty_vec3<f32>, "data.Store3(16u, asuint(value))"},
                    TypeCase{ty_vec3<i32>, "data.Store3(16u, asuint(value))"},
                    TypeCase{ty_vec4<u32>, "data.Store4(16u, asuint(value))"},
                    TypeCase{ty_vec4<f32>, "data.Store4(16u, asuint(value))"},
                    TypeCase{ty_vec4<i32>, "data.Store4(16u, asuint(value))"},
                    TypeCase{ty_mat2x2<u32>, R"({
  buffer.Store2((offset + 0u), asuint(value[0u]));
  buffer.Store2((offset + 8u), asuint(value[1u]));
})"},
                    TypeCase{ty_mat2x3<f32>, R"({
  buffer.Store3((offset + 0u), asuint(value[0u]));
  buffer.Store3((offset + 16u), asuint(value[1u]));
})"},
                    TypeCase{ty_mat2x4<i32>, R"({
  buffer.Store4((offset + 0u), asuint(value[0u]));
  buffer.Store4((offset + 16u), asuint(value[1u]));
})"},
                    TypeCase{ty_mat3x2<u32>, R"({
  buffer.Store2((offset + 0u), asuint(value[0u]));
  buffer.Store2((offset + 8u), asuint(value[1u]));
  buffer.Store2((offset + 16u), asuint(value[2u]));
})"},
                    TypeCase{ty_mat3x3<f32>, R"({
  buffer.Store3((offset + 0u), asuint(value[0u]));
  buffer.Store3((offset + 16u), asuint(value[1u]));
  buffer.Store3((offset + 32u), asuint(value[2u]));
})"},
                    TypeCase{ty_mat3x4<i32>, R"({
  buffer.Store4((offset + 0u), asuint(value[0u]));
  buffer.Store4((offset + 16u), asuint(value[1u]));
  buffer.Store4((offset + 32u), asuint(value[2u]));
})"},
                    TypeCase{ty_mat4x2<u32>, R"({
  buffer.Store2((offset + 0u), asuint(value[0u]));
  buffer.Store2((offset + 8u), asuint(value[1u]));
  buffer.Store2((offset + 16u), asuint(value[2u]));
  buffer.Store2((offset + 24u), asuint(value[3u]));
})"},
                    TypeCase{ty_mat4x3<f32>, R"({
  buffer.Store3((offset + 0u), asuint(value[0u]));
  buffer.Store3((offset + 16u), asuint(value[1u]));
  buffer.Store3((offset + 32u), asuint(value[2u]));
  buffer.Store3((offset + 48u), asuint(value[3u]));
})"},
                    TypeCase{ty_mat4x4<i32>, R"({
  buffer.Store4((offset + 0u), asuint(value[0u]));
  buffer.Store4((offset + 16u), asuint(value[1u]));
  buffer.Store4((offset + 32u), asuint(value[2u]));
  buffer.Store4((offset + 48u), asuint(value[3u]));
})"}));

TEST_F(HlslGeneratorImplTest_MemberAccessor, StorageBuffer_Store_Matrix_Empty) {
  // struct Data {
  //   z : f32;
  //   a : mat2x3<f32>;
  // };
  // var<storage> data : Data;
  // data.a = mat2x3<f32>();

  SetupStorageBuffer({
      Member("a", ty.i32()),
      Member("b", ty.mat2x3<f32>()),
  });

  SetupFunction({
      Assign(MemberAccessor("data", "b"),
             Construct(ty.mat2x3<f32>(), ast::ExpressionList{})),
  });

  GeneratorImpl& gen = SanitizeAndBuild();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  auto* expected =
      R"(
RWByteAddressBuffer data : register(u0, space1);

void tint_symbol_1(RWByteAddressBuffer buffer, uint offset, float2x3 value) {
  buffer.Store3((offset + 0u), asuint(value[0u]));
  buffer.Store3((offset + 16u), asuint(value[1u]));
}

void main() {
  tint_symbol_1(data, 16u, float2x3(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f));
  return;
}

)";
  EXPECT_EQ(result(), expected);

  Validate();
}

TEST_F(HlslGeneratorImplTest_MemberAccessor,
       StorageBuffer_Load_Matrix_Single_Element) {
  // struct Data {
  //   z : f32;
  //   a : mat4x3<f32>;
  // };
  // var<storage> data : Data;
  // data.a[2][1];

  SetupStorageBuffer({
      Member("z", ty.f32()),
      Member("a", ty.mat4x3<f32>()),
  });

  SetupFunction({
      Decl(
          Var("x", nullptr, ast::StorageClass::kNone,
              IndexAccessor(IndexAccessor(MemberAccessor("data", "a"), 2), 1))),
  });

  GeneratorImpl& gen = SanitizeAndBuild();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  auto* expected =
      R"(
RWByteAddressBuffer data : register(u0, space1);

void main() {
  float x = asfloat(data.Load(52u));
  return;
}

)";
  EXPECT_EQ(result(), expected);

  Validate();
}

TEST_F(HlslGeneratorImplTest_MemberAccessor,
       EmitExpression_ArrayAccessor_StorageBuffer_Load_Int_FromArray) {
  // struct Data {
  //   a : [[stride(4)]] array<i32, 5>;
  // };
  // var<storage> data : Data;
  // data.a[2];

  SetupStorageBuffer({
      Member("z", ty.f32()),
      Member("a", ty.array<i32, 5>(4)),
  });

  SetupFunction({
      Decl(Var("x", nullptr, ast::StorageClass::kNone,
               IndexAccessor(MemberAccessor("data", "a"), 2))),
  });

  GeneratorImpl& gen = SanitizeAndBuild();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  auto* expected =
      R"(
RWByteAddressBuffer data : register(u0, space1);

void main() {
  int x = asint(data.Load(12u));
  return;
}

)";
  EXPECT_EQ(result(), expected);
}

TEST_F(HlslGeneratorImplTest_MemberAccessor,
       EmitExpression_ArrayAccessor_StorageBuffer_Load_Int_FromArray_ExprIdx) {
  // struct Data {
  //   a : [[stride(4)]] array<i32, 5>;
  // };
  // var<storage> data : Data;
  // data.a[(2 + 4) - 3];

  SetupStorageBuffer({
      Member("z", ty.f32()),
      Member("a", ty.array<i32, 5>(4)),
  });

  SetupFunction({
      Decl(Var("x", nullptr, ast::StorageClass::kNone,
               IndexAccessor(MemberAccessor("data", "a"),
                             Sub(Add(2, Expr(4)), Expr(3))))),
  });

  GeneratorImpl& gen = SanitizeAndBuild();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  auto* expected =
      R"(
RWByteAddressBuffer data : register(u0, space1);

void main() {
  int x = asint(data.Load((4u + (4u * uint(((2 + 4) - 3))))));
  return;
}

)";
  EXPECT_EQ(result(), expected);
}

TEST_F(HlslGeneratorImplTest_MemberAccessor, StorageBuffer_Store_ToArray) {
  // struct Data {
  //   a : [[stride(4)]] array<i32, 5>;
  // };
  // var<storage> data : Data;
  // data.a[2] = 2;

  SetupStorageBuffer({
      Member("z", ty.f32()),
      Member("a", ty.array<i32, 5>(4)),
  });

  SetupFunction({
      Assign(IndexAccessor(MemberAccessor("data", "a"), 2), 2),
  });

  GeneratorImpl& gen = SanitizeAndBuild();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  auto* expected =
      R"(
RWByteAddressBuffer data : register(u0, space1);

void main() {
  data.Store(12u, asuint(2));
  return;
}

)";
  EXPECT_EQ(result(), expected);
}

TEST_F(HlslGeneratorImplTest_MemberAccessor, StorageBuffer_Load_MultiLevel) {
  // struct Inner {
  //   a : vec3<i32>;
  //   b : vec3<f32>;
  // };
  // struct Data {
  //   var c : [[stride(32)]] array<Inner, 4>;
  // };
  //
  // var<storage> data : Pre;
  // data.c[2].b

  auto* inner = Structure("Inner", {
                                       Member("a", ty.vec3<f32>()),
                                       Member("b", ty.vec3<f32>()),
                                   });

  SetupStorageBuffer({
      Member("c", ty.array(inner, 4, 32)),
  });

  SetupFunction({
      Decl(Var(
          "x", nullptr, ast::StorageClass::kNone,
          MemberAccessor(IndexAccessor(MemberAccessor("data", "c"), 2), "b"))),
  });

  GeneratorImpl& gen = SanitizeAndBuild();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  auto* expected =
      R"(
RWByteAddressBuffer data : register(u0, space1);

void main() {
  float3 x = asfloat(data.Load3(80u));
  return;
}

)";
  EXPECT_EQ(result(), expected);

  Validate();
}

TEST_F(HlslGeneratorImplTest_MemberAccessor,
       StorageBuffer_Load_MultiLevel_Swizzle) {
  // struct Inner {
  //   a : vec3<i32>;
  //   b : vec3<f32>;
  // };
  // struct Data {
  //   var c : [[stride(32)]] array<Inner, 4>;
  // };
  //
  // var<storage> data : Pre;
  // data.c[2].b.xy

  auto* inner = Structure("Inner", {
                                       Member("a", ty.vec3<f32>()),
                                       Member("b", ty.vec3<f32>()),
                                   });

  SetupStorageBuffer({
      Member("c", ty.array(inner, 4, 32)),
  });

  SetupFunction({
      Decl(Var("x", nullptr, ast::StorageClass::kNone,
               MemberAccessor(
                   MemberAccessor(IndexAccessor(MemberAccessor("data", "c"), 2),
                                  "b"),
                   "xy"))),
  });

  GeneratorImpl& gen = SanitizeAndBuild();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  auto* expected =
      R"(
RWByteAddressBuffer data : register(u0, space1);

void main() {
  float2 x = asfloat(data.Load3(80u)).xy;
  return;
}

)";
  EXPECT_EQ(result(), expected);

  Validate();
}

TEST_F(HlslGeneratorImplTest_MemberAccessor,
       StorageBuffer_Load_MultiLevel_Swizzle_SingleLetter) {  // NOLINT
  // struct Inner {
  //   a : vec3<i32>;
  //   b : vec3<f32>;
  // };
  // struct Data {
  //   var c : [[stride(32)]] array<Inner, 4>;
  // };
  //
  // var<storage> data : Pre;
  // data.c[2].b.g

  auto* inner = Structure("Inner", {
                                       Member("a", ty.vec3<f32>()),
                                       Member("b", ty.vec3<f32>()),
                                   });

  SetupStorageBuffer({
      Member("c", ty.array(inner, 4, 32)),
  });

  SetupFunction({
      Decl(Var("x", nullptr, ast::StorageClass::kNone,
               MemberAccessor(
                   MemberAccessor(IndexAccessor(MemberAccessor("data", "c"), 2),
                                  "b"),
                   "g"))),
  });

  GeneratorImpl& gen = SanitizeAndBuild();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  auto* expected =
      R"(
RWByteAddressBuffer data : register(u0, space1);

void main() {
  float x = asfloat(data.Load(84u));
  return;
}

)";
  EXPECT_EQ(result(), expected);

  Validate();
}

TEST_F(HlslGeneratorImplTest_MemberAccessor,
       StorageBuffer_Load_MultiLevel_Index) {
  // struct Inner {
  //   a : vec3<i32>;
  //   b : vec3<f32>;
  // };
  // struct Data {
  //   var c : [[stride(32)]] array<Inner, 4>;
  // };
  //
  // var<storage> data : Pre;
  // data.c[2].b[1]

  auto* inner = Structure("Inner", {
                                       Member("a", ty.vec3<f32>()),
                                       Member("b", ty.vec3<f32>()),
                                   });

  SetupStorageBuffer({
      Member("c", ty.array(inner, 4, 32)),
  });

  SetupFunction({
      Decl(Var(
          "x", nullptr, ast::StorageClass::kNone,
          IndexAccessor(MemberAccessor(
                            IndexAccessor(MemberAccessor("data", "c"), 2), "b"),
                        1))),
  });

  GeneratorImpl& gen = SanitizeAndBuild();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  auto* expected =
      R"(
RWByteAddressBuffer data : register(u0, space1);

void main() {
  float x = asfloat(data.Load(84u));
  return;
}

)";
  EXPECT_EQ(result(), expected);

  Validate();
}

TEST_F(HlslGeneratorImplTest_MemberAccessor, StorageBuffer_Store_MultiLevel) {
  // struct Inner {
  //   a : vec3<i32>;
  //   b : vec3<f32>;
  // };
  // struct Data {
  //   var c : [[stride(32)]] array<Inner, 4>;
  // };
  //
  // var<storage> data : Pre;
  // data.c[2].b = vec3<f32>(1.f, 2.f, 3.f);

  auto* inner = Structure("Inner", {
                                       Member("a", ty.vec3<f32>()),
                                       Member("b", ty.vec3<f32>()),
                                   });

  SetupStorageBuffer({
      Member("c", ty.array(inner, 4, 32)),
  });

  SetupFunction({
      Assign(MemberAccessor(IndexAccessor(MemberAccessor("data", "c"), 2), "b"),
             vec3<f32>(1.f, 2.f, 3.f)),
  });

  GeneratorImpl& gen = SanitizeAndBuild();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  auto* expected =
      R"(
RWByteAddressBuffer data : register(u0, space1);

void main() {
  data.Store3(80u, asuint(float3(1.0f, 2.0f, 3.0f)));
  return;
}

)";
  EXPECT_EQ(result(), expected);

  Validate();
}

TEST_F(HlslGeneratorImplTest_MemberAccessor,
       StorageBuffer_Store_Swizzle_SingleLetter) {
  // struct Inner {
  //   a : vec3<i32>;
  //   b : vec3<f32>;
  // };
  // struct Data {
  //   var c : [[stride(32)]] array<Inner, 4>;
  // };
  //
  // var<storage> data : Pre;
  // data.c[2].b.y = 1.f;

  auto* inner = Structure("Inner", {
                                       Member("a", ty.vec3<i32>()),
                                       Member("b", ty.vec3<f32>()),
                                   });

  SetupStorageBuffer({
      Member("c", ty.array(inner, 4, 32)),
  });

  SetupFunction({
      Assign(MemberAccessor(
                 MemberAccessor(IndexAccessor(MemberAccessor("data", "c"), 2),
                                "b"),
                 "y"),
             Expr(1.f)),
  });

  GeneratorImpl& gen = SanitizeAndBuild();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  auto* expected =
      R"(
RWByteAddressBuffer data : register(u0, space1);

void main() {
  data.Store(84u, asuint(1.0f));
  return;
}

)";
  EXPECT_EQ(result(), expected);

  Validate();
}

TEST_F(HlslGeneratorImplTest_MemberAccessor, Swizzle_xyz) {
  auto* var = Var("my_vec", ty.vec4<f32>(), ast::StorageClass::kNone,
                  vec4<f32>(1.f, 2.f, 3.f, 4.f));
  auto* expr = MemberAccessor("my_vec", "xyz");
  WrapInFunction(var, expr);

  GeneratorImpl& gen = SanitizeAndBuild();
  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_THAT(result(), HasSubstr("my_vec.xyz"));

  Validate();
}

TEST_F(HlslGeneratorImplTest_MemberAccessor, Swizzle_gbr) {
  auto* var = Var("my_vec", ty.vec4<f32>(), ast::StorageClass::kNone,
                  vec4<f32>(1.f, 2.f, 3.f, 4.f));
  auto* expr = MemberAccessor("my_vec", "gbr");
  WrapInFunction(var, expr);

  GeneratorImpl& gen = SanitizeAndBuild();
  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_THAT(result(), HasSubstr("my_vec.gbr"));

  Validate();
}

}  // namespace
}  // namespace hlsl
}  // namespace writer
}  // namespace tint
