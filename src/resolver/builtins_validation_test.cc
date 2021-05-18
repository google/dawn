// Copyright 2021 The Tint Authors.
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

#include "src/resolver/resolver_test_helper.h"

namespace tint {
namespace {
class ResolverBuiltinsValidationTest : public resolver::TestHelper,
                                       public testing::Test {};

TEST_F(ResolverBuiltinsValidationTest, Length_Float_Scalar) {
  auto* builtin = Call("length", 1.0f);
  WrapInFunction(builtin);

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBuiltinsValidationTest, Length_Float_Vec2) {
  auto* builtin = Call("length", vec2<f32>(1.0f, 1.0f));
  WrapInFunction(builtin);

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBuiltinsValidationTest, Length_Float_Vec3) {
  auto* builtin = Call("length", vec3<f32>(1.0f, 1.0f, 1.0f));
  WrapInFunction(builtin);

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBuiltinsValidationTest, Length_Float_Vec4) {
  auto* builtin = Call("length", vec4<f32>(1.0f, 1.0f, 1.0f, 1.0f));
  WrapInFunction(builtin);

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBuiltinsValidationTest, Distance_Float_Scalar) {
  auto* builtin = Call("distance", 1.0f, 1.0f);
  WrapInFunction(builtin);

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBuiltinsValidationTest, Distance_Float_Vec2) {
  auto* builtin =
      Call("distance", vec2<f32>(1.0f, 1.0f), vec2<f32>(1.0f, 1.0f));
  WrapInFunction(builtin);

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBuiltinsValidationTest, Distance_Float_Vec3) {
  auto* builtin = Call("distance", vec3<f32>(1.0f, 1.0f, 1.0f),
                       vec3<f32>(1.0f, 1.0f, 1.0f));
  WrapInFunction(builtin);

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBuiltinsValidationTest, Distance_Float_Vec4) {
  auto* builtin = Call("distance", vec4<f32>(1.0f, 1.0f, 1.0f, 1.0f),
                       vec4<f32>(1.0f, 1.0f, 1.0f, 1.0f));
  WrapInFunction(builtin);

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBuiltinsValidationTest, Determinant_Mat2x2) {
  auto* builtin = Call(
      "determinant", mat2x2<f32>(vec2<f32>(1.0f, 1.0f), vec2<f32>(1.0f, 1.0f)));
  WrapInFunction(builtin);

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBuiltinsValidationTest, Determinant_Mat3x3) {
  auto* builtin = Call("determinant", mat3x3<f32>(vec3<f32>(1.0f, 1.0f, 1.0f),
                                                  vec3<f32>(1.0f, 1.0f, 1.0f),
                                                  vec3<f32>(1.0f, 1.0f, 1.0f)));
  WrapInFunction(builtin);

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBuiltinsValidationTest, Determinant_Mat4x4) {
  auto* builtin =
      Call("determinant", mat4x4<f32>(vec4<f32>(1.0f, 1.0f, 1.0f, 1.0f),
                                      vec4<f32>(1.0f, 1.0f, 1.0f, 1.0f),
                                      vec4<f32>(1.0f, 1.0f, 1.0f, 1.0f),
                                      vec4<f32>(1.0f, 1.0f, 1.0f, 1.0f)));
  WrapInFunction(builtin);

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBuiltinsValidationTest, Frexp_Scalar) {
  auto* a = Var("a", ty.i32());
  auto* builtin = Call("frexp", 1.0f, AddressOf(Expr("a")));
  WrapInFunction(Decl(a), builtin);

  EXPECT_TRUE(r()->Resolve()) << r()->error();
  EXPECT_TRUE(TypeOf(builtin)->Is<sem::F32>());
  EXPECT_TRUE(TypeOf(builtin->params()[1])->Is<sem::Pointer>());
}

TEST_F(ResolverBuiltinsValidationTest, Frexp_Vec2) {
  auto* a = Var("a", ty.vec2<int>());
  auto* builtin = Call("frexp", vec2<f32>(1.0f, 1.0f), AddressOf(Expr("a")));
  WrapInFunction(Decl(a), builtin);

  EXPECT_TRUE(r()->Resolve()) << r()->error();
  EXPECT_TRUE(TypeOf(builtin)->is_float_vector());
  EXPECT_TRUE(TypeOf(builtin->params()[1])->Is<sem::Pointer>());
}

TEST_F(ResolverBuiltinsValidationTest, Frexp_Vec3) {
  auto* a = Var("a", ty.vec3<int>());
  auto* builtin =
      Call("frexp", vec3<f32>(1.0f, 1.0f, 1.0f), AddressOf(Expr("a")));
  WrapInFunction(Decl(a), builtin);

  EXPECT_TRUE(r()->Resolve()) << r()->error();
  EXPECT_TRUE(TypeOf(builtin)->is_float_vector());
  EXPECT_TRUE(TypeOf(builtin->params()[1])->Is<sem::Pointer>());
}

TEST_F(ResolverBuiltinsValidationTest, Frexp_Vec4) {
  auto* a = Var("a", ty.vec4<int>());
  auto* builtin =
      Call("frexp", vec4<f32>(1.0f, 1.0f, 1.0f, 1.0f), AddressOf(Expr("a")));
  WrapInFunction(Decl(a), builtin);

  EXPECT_TRUE(r()->Resolve()) << r()->error();
  EXPECT_TRUE(TypeOf(builtin)->is_float_vector());
  EXPECT_TRUE(TypeOf(builtin->params()[1])->Is<sem::Pointer>());
}

TEST_F(ResolverBuiltinsValidationTest, Modf_Scalar) {
  auto* a = Var("a", ty.f32());
  auto* builtin = Call("modf", 1.0f, AddressOf(Expr("a")));
  WrapInFunction(Decl(a), builtin);

  EXPECT_TRUE(r()->Resolve()) << r()->error();
  EXPECT_TRUE(TypeOf(builtin)->Is<sem::F32>());
  EXPECT_TRUE(TypeOf(builtin->params()[1])->Is<sem::Pointer>());
}

TEST_F(ResolverBuiltinsValidationTest, Modf_Vec2) {
  auto* a = Var("a", ty.vec2<f32>());
  auto* builtin = Call("modf", vec2<f32>(1.0f, 1.0f), AddressOf(Expr("a")));
  WrapInFunction(Decl(a), builtin);

  EXPECT_TRUE(r()->Resolve()) << r()->error();
  EXPECT_TRUE(TypeOf(builtin)->is_float_vector());
  EXPECT_TRUE(TypeOf(builtin->params()[1])->Is<sem::Pointer>());
}

TEST_F(ResolverBuiltinsValidationTest, Modf_Vec3) {
  auto* a = Var("a", ty.vec3<f32>());
  auto* builtin =
      Call("modf", vec3<f32>(1.0f, 1.0f, 1.0f), AddressOf(Expr("a")));
  WrapInFunction(Decl(a), builtin);

  EXPECT_TRUE(r()->Resolve()) << r()->error();
  EXPECT_TRUE(TypeOf(builtin)->is_float_vector());
  EXPECT_TRUE(TypeOf(builtin->params()[1])->Is<sem::Pointer>());
}

TEST_F(ResolverBuiltinsValidationTest, Modf_Vec4) {
  auto* a = Var("a", ty.vec4<f32>());
  auto* builtin =
      Call("modf", vec4<f32>(1.0f, 1.0f, 1.0f, 1.0f), AddressOf(Expr("a")));
  WrapInFunction(Decl(a), builtin);

  EXPECT_TRUE(r()->Resolve()) << r()->error();
  EXPECT_TRUE(TypeOf(builtin)->is_float_vector());
  EXPECT_TRUE(TypeOf(builtin->params()[1])->Is<sem::Pointer>());
}

TEST_F(ResolverBuiltinsValidationTest, Cross_Float_Vec3) {
  auto* builtin =
      Call("cross", vec3<f32>(1.0f, 1.0f, 1.0f), vec3<f32>(1.0f, 1.0f, 1.0f));
  WrapInFunction(builtin);

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBuiltinsValidationTest, Dot_Float_Vec2) {
  auto* builtin = Call("dot", vec2<f32>(1.0f, 1.0f), vec2<f32>(1.0f, 1.0f));
  WrapInFunction(builtin);

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBuiltinsValidationTest, Dot_Float_Vec3) {
  auto* builtin =
      Call("dot", vec3<f32>(1.0f, 1.0f, 1.0f), vec3<f32>(1.0f, 1.0f, 1.0f));
  WrapInFunction(builtin);

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBuiltinsValidationTest, Dot_Float_Vec4) {
  auto* builtin = Call("dot", vec4<f32>(1.0f, 1.0f, 1.0f, 1.0f),
                       vec4<f32>(1.0f, 1.0f, 1.0f, 1.0f));
  WrapInFunction(builtin);

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBuiltinsValidationTest, Select_Float_Scalar) {
  auto* builtin = Call("select", Expr(1.0f), Expr(1.0f), Expr(true));
  WrapInFunction(builtin);

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBuiltinsValidationTest, Select_Integer_Scalar) {
  auto* builtin = Call("select", Expr(1), Expr(1), Expr(true));
  WrapInFunction(builtin);

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBuiltinsValidationTest, Select_Boolean_Scalar) {
  auto* builtin = Call("select", Expr(true), Expr(true), Expr(true));
  WrapInFunction(builtin);

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBuiltinsValidationTest, Select_Float_Vec2) {
  auto* builtin = Call("select", vec2<f32>(1.0f, 1.0f), vec2<f32>(1.0f, 1.0f),
                       vec2<bool>(true, true));
  WrapInFunction(builtin);

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBuiltinsValidationTest, Select_Integer_Vec2) {
  auto* builtin =
      Call("select", vec2<int>(1, 1), vec2<int>(1, 1), vec2<bool>(true, true));
  WrapInFunction(builtin);

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBuiltinsValidationTest, Select_Boolean_Vec2) {
  auto* builtin = Call("select", vec2<bool>(true, true), vec2<bool>(true, true),
                       vec2<bool>(true, true));
  WrapInFunction(builtin);

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

template <typename T>
class ResolverBuiltinsValidationTestWithParams
    : public resolver::TestHelper,
      public testing::TestWithParam<T> {};

using FloatAllMatching =
    ResolverBuiltinsValidationTestWithParams<std::tuple<std::string, uint32_t>>;

TEST_P(FloatAllMatching, Scalar) {
  std::string name = std::get<0>(GetParam());
  uint32_t num_params = std::get<1>(GetParam());

  ast::ExpressionList params;
  for (uint32_t i = 0; i < num_params; ++i) {
    params.push_back(Expr(1.0f));
  }
  auto* builtin = Call(name, params);
  WrapInFunction(builtin);

  EXPECT_TRUE(r()->Resolve()) << r()->error();
  EXPECT_TRUE(TypeOf(builtin)->Is<sem::F32>());
}

TEST_P(FloatAllMatching, Vec2) {
  std::string name = std::get<0>(GetParam());
  uint32_t num_params = std::get<1>(GetParam());

  ast::ExpressionList params;
  for (uint32_t i = 0; i < num_params; ++i) {
    params.push_back(vec2<f32>(1.0f, 1.0f));
  }
  auto* builtin = Call(name, params);
  WrapInFunction(builtin);

  EXPECT_TRUE(r()->Resolve()) << r()->error();
  EXPECT_TRUE(TypeOf(builtin)->is_float_vector());
}

TEST_P(FloatAllMatching, Vec3) {
  std::string name = std::get<0>(GetParam());
  uint32_t num_params = std::get<1>(GetParam());

  ast::ExpressionList params;
  for (uint32_t i = 0; i < num_params; ++i) {
    params.push_back(vec3<f32>(1.0f, 1.0f, 1.0f));
  }
  auto* builtin = Call(name, params);
  WrapInFunction(builtin);

  EXPECT_TRUE(r()->Resolve()) << r()->error();
  EXPECT_TRUE(TypeOf(builtin)->is_float_vector());
}

TEST_P(FloatAllMatching, Vec4) {
  std::string name = std::get<0>(GetParam());
  uint32_t num_params = std::get<1>(GetParam());

  ast::ExpressionList params;
  for (uint32_t i = 0; i < num_params; ++i) {
    params.push_back(vec4<f32>(1.0f, 1.0f, 1.0f, 1.0f));
  }
  auto* builtin = Call(name, params);
  WrapInFunction(builtin);

  EXPECT_TRUE(r()->Resolve()) << r()->error();
  EXPECT_TRUE(TypeOf(builtin)->is_float_vector());
}

INSTANTIATE_TEST_SUITE_P(ResolverBuiltinsValidationTest,
                         FloatAllMatching,
                         ::testing::Values(std::make_tuple("abs", 1),
                                           std::make_tuple("acos", 1),
                                           std::make_tuple("asin", 1),
                                           std::make_tuple("atan", 1),
                                           std::make_tuple("atan2", 2),
                                           std::make_tuple("ceil", 1),
                                           std::make_tuple("clamp", 3),
                                           std::make_tuple("cos", 1),
                                           std::make_tuple("cosh", 1),
                                           std::make_tuple("dpdx", 1),
                                           std::make_tuple("dpdxCoarse", 1),
                                           std::make_tuple("dpdxFine", 1),
                                           std::make_tuple("dpdy", 1),
                                           std::make_tuple("dpdyCoarse", 1),
                                           std::make_tuple("dpdyFine", 1),
                                           std::make_tuple("exp", 1),
                                           std::make_tuple("exp2", 1),
                                           std::make_tuple("faceForward", 3),
                                           std::make_tuple("floor", 1),
                                           std::make_tuple("fma", 3),
                                           std::make_tuple("fract", 1),
                                           std::make_tuple("fwidth", 1),
                                           std::make_tuple("fwidthCoarse", 1),
                                           std::make_tuple("fwidthFine", 1),
                                           std::make_tuple("inverseSqrt", 1),
                                           std::make_tuple("log", 1),
                                           std::make_tuple("log2", 1),
                                           std::make_tuple("max", 2),
                                           std::make_tuple("min", 2),
                                           std::make_tuple("mix", 3),
                                           std::make_tuple("pow", 2),
                                           std::make_tuple("reflect", 2),
                                           std::make_tuple("round", 1),
                                           std::make_tuple("sign", 1),
                                           std::make_tuple("sin", 1),
                                           std::make_tuple("sinh", 1),
                                           std::make_tuple("smoothStep", 3),
                                           std::make_tuple("sqrt", 1),
                                           std::make_tuple("step", 2),
                                           std::make_tuple("tan", 1),
                                           std::make_tuple("tanh", 1),
                                           std::make_tuple("trunc", 1)));

using IntegerAllMatching =
    ResolverBuiltinsValidationTestWithParams<std::tuple<std::string, uint32_t>>;

TEST_P(IntegerAllMatching, ScalarUnsigned) {
  std::string name = std::get<0>(GetParam());
  uint32_t num_params = std::get<1>(GetParam());

  ast::ExpressionList params;
  for (uint32_t i = 0; i < num_params; ++i) {
    params.push_back(Construct<uint32_t>(1));
  }
  auto* builtin = Call(name, params);
  WrapInFunction(builtin);

  EXPECT_TRUE(r()->Resolve()) << r()->error();
  EXPECT_TRUE(TypeOf(builtin)->Is<sem::U32>());
}

TEST_P(IntegerAllMatching, Vec2Unsigned) {
  std::string name = std::get<0>(GetParam());
  uint32_t num_params = std::get<1>(GetParam());

  ast::ExpressionList params;
  for (uint32_t i = 0; i < num_params; ++i) {
    params.push_back(vec2<uint32_t>(1u, 1u));
  }
  auto* builtin = Call(name, params);
  WrapInFunction(builtin);

  EXPECT_TRUE(r()->Resolve()) << r()->error();
  EXPECT_TRUE(TypeOf(builtin)->is_unsigned_integer_vector());
}

TEST_P(IntegerAllMatching, Vec3Unsigned) {
  std::string name = std::get<0>(GetParam());
  uint32_t num_params = std::get<1>(GetParam());

  ast::ExpressionList params;
  for (uint32_t i = 0; i < num_params; ++i) {
    params.push_back(vec3<uint32_t>(1u, 1u, 1u));
  }
  auto* builtin = Call(name, params);
  WrapInFunction(builtin);

  EXPECT_TRUE(r()->Resolve()) << r()->error();
  EXPECT_TRUE(TypeOf(builtin)->is_unsigned_integer_vector());
}

TEST_P(IntegerAllMatching, Vec4Unsigned) {
  std::string name = std::get<0>(GetParam());
  uint32_t num_params = std::get<1>(GetParam());

  ast::ExpressionList params;
  for (uint32_t i = 0; i < num_params; ++i) {
    params.push_back(vec4<uint32_t>(1u, 1u, 1u, 1u));
  }
  auto* builtin = Call(name, params);
  WrapInFunction(builtin);

  EXPECT_TRUE(r()->Resolve()) << r()->error();
  EXPECT_TRUE(TypeOf(builtin)->is_unsigned_integer_vector());
}

TEST_P(IntegerAllMatching, ScalarSigned) {
  std::string name = std::get<0>(GetParam());
  uint32_t num_params = std::get<1>(GetParam());

  ast::ExpressionList params;
  for (uint32_t i = 0; i < num_params; ++i) {
    params.push_back(Construct<int32_t>(1));
  }
  auto* builtin = Call(name, params);
  WrapInFunction(builtin);

  EXPECT_TRUE(r()->Resolve()) << r()->error();
  EXPECT_TRUE(TypeOf(builtin)->Is<sem::I32>());
}

TEST_P(IntegerAllMatching, Vec2Signed) {
  std::string name = std::get<0>(GetParam());
  uint32_t num_params = std::get<1>(GetParam());

  ast::ExpressionList params;
  for (uint32_t i = 0; i < num_params; ++i) {
    params.push_back(vec2<int32_t>(1, 1));
  }
  auto* builtin = Call(name, params);
  WrapInFunction(builtin);

  EXPECT_TRUE(r()->Resolve()) << r()->error();
  EXPECT_TRUE(TypeOf(builtin)->is_signed_integer_vector());
}

TEST_P(IntegerAllMatching, Vec3Signed) {
  std::string name = std::get<0>(GetParam());
  uint32_t num_params = std::get<1>(GetParam());

  ast::ExpressionList params;
  for (uint32_t i = 0; i < num_params; ++i) {
    params.push_back(vec3<int32_t>(1, 1, 1));
  }
  auto* builtin = Call(name, params);
  WrapInFunction(builtin);

  EXPECT_TRUE(r()->Resolve()) << r()->error();
  EXPECT_TRUE(TypeOf(builtin)->is_signed_integer_vector());
}

TEST_P(IntegerAllMatching, Vec4Signed) {
  std::string name = std::get<0>(GetParam());
  uint32_t num_params = std::get<1>(GetParam());

  ast::ExpressionList params;
  for (uint32_t i = 0; i < num_params; ++i) {
    params.push_back(vec4<int32_t>(1, 1, 1, 1));
  }
  auto* builtin = Call(name, params);
  WrapInFunction(builtin);

  EXPECT_TRUE(r()->Resolve()) << r()->error();
  EXPECT_TRUE(TypeOf(builtin)->is_signed_integer_vector());
}

INSTANTIATE_TEST_SUITE_P(ResolverBuiltinsValidationTest,
                         IntegerAllMatching,
                         ::testing::Values(std::make_tuple("abs", 1),
                                           std::make_tuple("clamp", 3),
                                           std::make_tuple("countOneBits", 1),
                                           std::make_tuple("max", 2),
                                           std::make_tuple("min", 2),
                                           std::make_tuple("reverseBits", 1)));

using BooleanVectorInput =
    ResolverBuiltinsValidationTestWithParams<std::tuple<std::string, uint32_t>>;

TEST_P(BooleanVectorInput, Vec2) {
  std::string name = std::get<0>(GetParam());
  uint32_t num_params = std::get<1>(GetParam());

  ast::ExpressionList params;
  for (uint32_t i = 0; i < num_params; ++i) {
    params.push_back(vec2<bool>(true, true));
  }
  auto* builtin = Call(name, params);
  WrapInFunction(builtin);

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_P(BooleanVectorInput, Vec3) {
  std::string name = std::get<0>(GetParam());
  uint32_t num_params = std::get<1>(GetParam());

  ast::ExpressionList params;
  for (uint32_t i = 0; i < num_params; ++i) {
    params.push_back(vec3<bool>(true, true, true));
  }
  auto* builtin = Call(name, params);
  WrapInFunction(builtin);

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_P(BooleanVectorInput, Vec4) {
  std::string name = std::get<0>(GetParam());
  uint32_t num_params = std::get<1>(GetParam());

  ast::ExpressionList params;
  for (uint32_t i = 0; i < num_params; ++i) {
    params.push_back(vec4<bool>(true, true, true, true));
  }
  auto* builtin = Call(name, params);
  WrapInFunction(builtin);

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

INSTANTIATE_TEST_SUITE_P(ResolverBuiltinsValidationTest,
                         BooleanVectorInput,
                         ::testing::Values(std::make_tuple("all", 1),
                                           std::make_tuple("any", 1)));

using DataPacking4x8 = ResolverBuiltinsValidationTestWithParams<std::string>;

TEST_P(DataPacking4x8, Float_Vec4) {
  auto name = GetParam();
  auto* builtin = Call(name, vec4<f32>(1.0f, 1.0f, 1.0f, 1.0f));
  WrapInFunction(builtin);
  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

INSTANTIATE_TEST_SUITE_P(ResolverBuiltinsValidationTest,
                         DataPacking4x8,
                         ::testing::Values("pack4x8snorm", "pack4x8unorm"));

using DataPacking2x16 = ResolverBuiltinsValidationTestWithParams<std::string>;

TEST_P(DataPacking2x16, Float_Vec2) {
  auto name = GetParam();
  auto* builtin = Call(name, vec2<f32>(1.0f, 1.0f));
  WrapInFunction(builtin);
  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

INSTANTIATE_TEST_SUITE_P(ResolverBuiltinsValidationTest,
                         DataPacking2x16,
                         ::testing::Values("pack2x16snorm",
                                           "pack2x16unorm",
                                           "pack2x16float"));

}  // namespace
}  // namespace tint
