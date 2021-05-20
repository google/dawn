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

#include "src/resolver/resolver.h"

#include "gmock/gmock.h"
#include "src/ast/assignment_statement.h"
#include "src/ast/bitcast_expression.h"
#include "src/ast/break_statement.h"
#include "src/ast/call_statement.h"
#include "src/ast/continue_statement.h"
#include "src/ast/if_statement.h"
#include "src/ast/intrinsic_texture_helper_test.h"
#include "src/ast/loop_statement.h"
#include "src/ast/return_statement.h"
#include "src/ast/stage_decoration.h"
#include "src/ast/struct_block_decoration.h"
#include "src/ast/switch_statement.h"
#include "src/ast/unary_op_expression.h"
#include "src/ast/variable_decl_statement.h"
#include "src/resolver/resolver_test_helper.h"
#include "src/sem/call.h"
#include "src/sem/function.h"
#include "src/sem/member_accessor_expression.h"
#include "src/sem/sampled_texture_type.h"
#include "src/sem/statement.h"
#include "src/sem/variable.h"

using ::testing::ElementsAre;
using ::testing::HasSubstr;

namespace tint {
namespace resolver {
namespace {

using IntrinsicType = sem::IntrinsicType;

using ResolverIntrinsicTest = ResolverTest;

using ResolverIntrinsicDerivativeTest = ResolverTestWithParam<std::string>;
TEST_P(ResolverIntrinsicDerivativeTest, Scalar) {
  auto name = GetParam();

  Global("ident", ty.f32(), ast::StorageClass::kInput);

  auto* expr = Call(name, "ident");
  WrapInFunction(expr);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(expr), nullptr);
  ASSERT_TRUE(TypeOf(expr)->Is<sem::F32>());
}

TEST_P(ResolverIntrinsicDerivativeTest, Vector) {
  auto name = GetParam();
  Global("ident", ty.vec4<f32>(), ast::StorageClass::kInput);

  auto* expr = Call(name, "ident");
  WrapInFunction(expr);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(expr), nullptr);
  ASSERT_TRUE(TypeOf(expr)->Is<sem::Vector>());
  EXPECT_TRUE(TypeOf(expr)->As<sem::Vector>()->type()->Is<sem::F32>());
  EXPECT_EQ(TypeOf(expr)->As<sem::Vector>()->size(), 4u);
}

TEST_P(ResolverIntrinsicDerivativeTest, MissingParam) {
  auto name = GetParam();

  auto* expr = Call(name);
  WrapInFunction(expr);

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(), "error: no matching call to " + name +
                              "()\n\n"
                              "2 candidate functions:\n  " +
                              name + "(f32) -> f32\n  " + name +
                              "(vecN<f32>) -> vecN<f32>\n");
}

INSTANTIATE_TEST_SUITE_P(ResolverTest,
                         ResolverIntrinsicDerivativeTest,
                         testing::Values("dpdx",
                                         "dpdxCoarse",
                                         "dpdxFine",
                                         "dpdy",
                                         "dpdyCoarse",
                                         "dpdyFine",
                                         "fwidth",
                                         "fwidthCoarse",
                                         "fwidthFine"));

using ResolverIntrinsic = ResolverTestWithParam<std::string>;
TEST_P(ResolverIntrinsic, Test) {
  auto name = GetParam();

  Global("my_var", ty.vec3<bool>(), ast::StorageClass::kInput);

  auto* expr = Call(name, "my_var");
  WrapInFunction(expr);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(expr), nullptr);
  EXPECT_TRUE(TypeOf(expr)->Is<sem::Bool>());
}
INSTANTIATE_TEST_SUITE_P(ResolverTest,
                         ResolverIntrinsic,
                         testing::Values("any", "all"));

using ResolverIntrinsicTest_FloatMethod = ResolverTestWithParam<std::string>;
TEST_P(ResolverIntrinsicTest_FloatMethod, Vector) {
  auto name = GetParam();

  Global("my_var", ty.vec3<f32>(), ast::StorageClass::kInput);

  auto* expr = Call(name, "my_var");
  WrapInFunction(expr);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(expr), nullptr);
  ASSERT_TRUE(TypeOf(expr)->Is<sem::Vector>());
  EXPECT_TRUE(TypeOf(expr)->As<sem::Vector>()->type()->Is<sem::Bool>());
  EXPECT_EQ(TypeOf(expr)->As<sem::Vector>()->size(), 3u);
}

TEST_P(ResolverIntrinsicTest_FloatMethod, Scalar) {
  auto name = GetParam();

  Global("my_var", ty.f32(), ast::StorageClass::kInput);

  auto* expr = Call(name, "my_var");
  WrapInFunction(expr);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(expr), nullptr);
  EXPECT_TRUE(TypeOf(expr)->Is<sem::Bool>());
}

TEST_P(ResolverIntrinsicTest_FloatMethod, MissingParam) {
  auto name = GetParam();

  Global("my_var", ty.f32(), ast::StorageClass::kInput);

  auto* expr = Call(name);
  WrapInFunction(expr);

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(), "error: no matching call to " + name +
                              "()\n\n"
                              "2 candidate functions:\n  " +
                              name + "(f32) -> bool\n  " + name +
                              "(vecN<f32>) -> vecN<bool>\n");
}

TEST_P(ResolverIntrinsicTest_FloatMethod, TooManyParams) {
  auto name = GetParam();

  Global("my_var", ty.f32(), ast::StorageClass::kInput);

  auto* expr = Call(name, "my_var", 1.23f);
  WrapInFunction(expr);

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(), "error: no matching call to " + name +
                              "(f32, f32)\n\n"
                              "2 candidate functions:\n  " +
                              name + "(f32) -> bool\n  " + name +
                              "(vecN<f32>) -> vecN<bool>\n");
}
INSTANTIATE_TEST_SUITE_P(
    ResolverTest,
    ResolverIntrinsicTest_FloatMethod,
    testing::Values("isInf", "isNan", "isFinite", "isNormal"));

enum class Texture { kF32, kI32, kU32 };
inline std::ostream& operator<<(std::ostream& out, Texture data) {
  if (data == Texture::kF32) {
    out << "f32";
  } else if (data == Texture::kI32) {
    out << "i32";
  } else {
    out << "u32";
  }
  return out;
}

struct TextureTestParams {
  ast::TextureDimension dim;
  Texture type = Texture::kF32;
  ast::ImageFormat format = ast::ImageFormat::kR16Float;
};
inline std::ostream& operator<<(std::ostream& out, TextureTestParams data) {
  out << data.dim << "_" << data.type;
  return out;
}

class ResolverIntrinsicTest_TextureOperation
    : public ResolverTestWithParam<TextureTestParams> {
 public:
  /// Gets an appropriate type for the coords parameter depending the the
  /// dimensionality of the texture being sampled.
  /// @param dim dimensionality of the texture being sampled
  /// @param scalar the scalar type
  /// @returns a pointer to a type appropriate for the coord param
  ast::Type* GetCoordsType(ast::TextureDimension dim, ast::Type* scalar) {
    switch (dim) {
      case ast::TextureDimension::k1d:
        return scalar;
      case ast::TextureDimension::k2d:
      case ast::TextureDimension::k2dArray:
        return ty.vec(scalar, 2);
      case ast::TextureDimension::k3d:
      case ast::TextureDimension::kCube:
      case ast::TextureDimension::kCubeArray:
        return ty.vec(scalar, 3);
      default:
        [=]() { FAIL() << "Unsupported texture dimension: " << dim; }();
    }
    return nullptr;
  }

  void add_call_param(std::string name,
                      const ast::Type* type,
                      ast::ExpressionList* call_params) {
    if (type->UnwrapAll()->is_handle()) {
      Global(name, type, ast::StorageClass::kNone, nullptr,
             {
                 create<ast::BindingDecoration>(0),
                 create<ast::GroupDecoration>(0),
             });

    } else {
      Global(name, type, ast::StorageClass::kPrivate);
    }

    call_params->push_back(Expr(name));
  }
  ast::Type* subtype(Texture type) {
    if (type == Texture::kF32) {
      return ty.f32();
    }
    if (type == Texture::kI32) {
      return ty.i32();
    }
    return ty.u32();
  }
};

using ResolverIntrinsicTest_StorageTextureOperation =
    ResolverIntrinsicTest_TextureOperation;
TEST_P(ResolverIntrinsicTest_StorageTextureOperation, TextureLoadRo) {
  auto dim = GetParam().dim;
  auto type = GetParam().type;
  auto format = GetParam().format;

  auto* coords_type = GetCoordsType(dim, ty.i32());
  auto* texture_type = ty.storage_texture(dim, format);
  auto* ro_texture_type =
      ty.access(ast::AccessControl::kReadOnly, texture_type);

  ast::ExpressionList call_params;

  add_call_param("texture", ro_texture_type, &call_params);
  add_call_param("coords", coords_type, &call_params);

  if (ast::IsTextureArray(dim)) {
    add_call_param("array_index", ty.i32(), &call_params);
  }

  auto* expr = Call("textureLoad", call_params);
  WrapInFunction(expr);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(expr), nullptr);
  ASSERT_TRUE(TypeOf(expr)->Is<sem::Vector>());
  if (type == Texture::kF32) {
    EXPECT_TRUE(TypeOf(expr)->As<sem::Vector>()->type()->Is<sem::F32>());
  } else if (type == Texture::kI32) {
    EXPECT_TRUE(TypeOf(expr)->As<sem::Vector>()->type()->Is<sem::I32>());
  } else {
    EXPECT_TRUE(TypeOf(expr)->As<sem::Vector>()->type()->Is<sem::U32>());
  }
  EXPECT_EQ(TypeOf(expr)->As<sem::Vector>()->size(), 4u);
}

INSTANTIATE_TEST_SUITE_P(
    ResolverTest,
    ResolverIntrinsicTest_StorageTextureOperation,
    testing::Values(
        TextureTestParams{ast::TextureDimension::k1d, Texture::kF32,
                          ast::ImageFormat::kR32Float},
        TextureTestParams{ast::TextureDimension::k1d, Texture::kI32,
                          ast::ImageFormat::kR32Sint},
        TextureTestParams{ast::TextureDimension::k1d, Texture::kF32,
                          ast::ImageFormat::kRgba8Unorm},
        TextureTestParams{ast::TextureDimension::k2d, Texture::kF32,
                          ast::ImageFormat::kR32Float},
        TextureTestParams{ast::TextureDimension::k2d, Texture::kI32,
                          ast::ImageFormat::kR32Sint},
        TextureTestParams{ast::TextureDimension::k2d, Texture::kF32,
                          ast::ImageFormat::kRgba8Unorm},
        TextureTestParams{ast::TextureDimension::k2dArray, Texture::kF32,
                          ast::ImageFormat::kR32Float},
        TextureTestParams{ast::TextureDimension::k2dArray, Texture::kI32,
                          ast::ImageFormat::kR32Sint},
        TextureTestParams{ast::TextureDimension::k2dArray, Texture::kF32,
                          ast::ImageFormat::kRgba8Unorm},
        TextureTestParams{ast::TextureDimension::k3d, Texture::kF32,
                          ast::ImageFormat::kR32Float},
        TextureTestParams{ast::TextureDimension::k3d, Texture::kI32,
                          ast::ImageFormat::kR32Sint},
        TextureTestParams{ast::TextureDimension::k3d, Texture::kF32,
                          ast::ImageFormat::kRgba8Unorm}));

using ResolverIntrinsicTest_SampledTextureOperation =
    ResolverIntrinsicTest_TextureOperation;
TEST_P(ResolverIntrinsicTest_SampledTextureOperation, TextureLoadSampled) {
  auto dim = GetParam().dim;
  auto type = GetParam().type;

  auto* s = subtype(type);
  auto* coords_type = GetCoordsType(dim, ty.i32());
  auto* texture_type = ty.sampled_texture(dim, s);

  ast::ExpressionList call_params;

  add_call_param("texture", texture_type, &call_params);
  add_call_param("coords", coords_type, &call_params);
  if (dim == ast::TextureDimension::k2dArray) {
    add_call_param("array_index", ty.i32(), &call_params);
  }
  add_call_param("level", ty.i32(), &call_params);

  auto* expr = Call("textureLoad", call_params);
  WrapInFunction(expr);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(expr), nullptr);
  ASSERT_TRUE(TypeOf(expr)->Is<sem::Vector>());
  if (type == Texture::kF32) {
    EXPECT_TRUE(TypeOf(expr)->As<sem::Vector>()->type()->Is<sem::F32>());
  } else if (type == Texture::kI32) {
    EXPECT_TRUE(TypeOf(expr)->As<sem::Vector>()->type()->Is<sem::I32>());
  } else {
    EXPECT_TRUE(TypeOf(expr)->As<sem::Vector>()->type()->Is<sem::U32>());
  }
  EXPECT_EQ(TypeOf(expr)->As<sem::Vector>()->size(), 4u);
}

INSTANTIATE_TEST_SUITE_P(
    ResolverTest,
    ResolverIntrinsicTest_SampledTextureOperation,
    testing::Values(TextureTestParams{ast::TextureDimension::k1d},
                    TextureTestParams{ast::TextureDimension::k2d},
                    TextureTestParams{ast::TextureDimension::k2dArray},
                    TextureTestParams{ast::TextureDimension::k3d}));

TEST_F(ResolverIntrinsicTest, Dot_Vec2) {
  Global("my_var", ty.vec2<f32>(), ast::StorageClass::kInput);

  auto* expr = Call("dot", "my_var", "my_var");
  WrapInFunction(expr);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(expr), nullptr);
  EXPECT_TRUE(TypeOf(expr)->Is<sem::F32>());
}

TEST_F(ResolverIntrinsicTest, Dot_Vec3) {
  Global("my_var", ty.vec3<f32>(), ast::StorageClass::kInput);

  auto* expr = Call("dot", "my_var", "my_var");
  WrapInFunction(expr);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(expr), nullptr);
  EXPECT_TRUE(TypeOf(expr)->Is<sem::F32>());
}

TEST_F(ResolverIntrinsicTest, Dot_Vec4) {
  Global("my_var", ty.vec4<f32>(), ast::StorageClass::kInput);

  auto* expr = Call("dot", "my_var", "my_var");
  WrapInFunction(expr);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(expr), nullptr);
  EXPECT_TRUE(TypeOf(expr)->Is<sem::F32>());
}

TEST_F(ResolverIntrinsicTest, Dot_Error_Scalar) {
  auto* expr = Call("dot", 1.0f, 1.0f);
  WrapInFunction(expr);

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(),
            R"(error: no matching call to dot(f32, f32)

1 candidate function:
  dot(vecN<f32>, vecN<f32>) -> f32
)");
}

TEST_F(ResolverIntrinsicTest, Dot_Error_VectorInt) {
  Global("my_var", ty.vec4<i32>(), ast::StorageClass::kInput);

  auto* expr = Call("dot", "my_var", "my_var");
  WrapInFunction(expr);

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(),
            R"(error: no matching call to dot(vec4<i32>, vec4<i32>)

1 candidate function:
  dot(vecN<f32>, vecN<f32>) -> f32
)");
}

TEST_F(ResolverIntrinsicTest, Select) {
  Global("my_var", ty.vec3<f32>(), ast::StorageClass::kInput);

  Global("bool_var", ty.vec3<bool>(), ast::StorageClass::kInput);

  auto* expr = Call("select", "my_var", "my_var", "bool_var");
  WrapInFunction(expr);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(expr), nullptr);
  EXPECT_TRUE(TypeOf(expr)->Is<sem::Vector>());
  EXPECT_EQ(TypeOf(expr)->As<sem::Vector>()->size(), 3u);
  EXPECT_TRUE(TypeOf(expr)->As<sem::Vector>()->type()->Is<sem::F32>());
}

TEST_F(ResolverIntrinsicTest, Select_Error_NoParams) {
  auto* expr = Call("select");
  WrapInFunction(expr);

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(),
            R"(error: no matching call to select()

2 candidate functions:
  select(T, T, bool) -> T  where: T is scalar
  select(vecN<T>, vecN<T>, vecN<bool>) -> vecN<T>  where: T is scalar
)");
}

TEST_F(ResolverIntrinsicTest, Select_Error_SelectorInt) {
  auto* expr = Call("select", 1, 1, 1);
  WrapInFunction(expr);

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(),
            R"(error: no matching call to select(i32, i32, i32)

2 candidate functions:
  select(T, T, bool) -> T  where: T is scalar
  select(vecN<T>, vecN<T>, vecN<bool>) -> vecN<T>  where: T is scalar
)");
}

TEST_F(ResolverIntrinsicTest, Select_Error_Matrix) {
  auto* expr = Call(
      "select", mat2x2<f32>(vec2<f32>(1.0f, 1.0f), vec2<f32>(1.0f, 1.0f)),
      mat2x2<f32>(vec2<f32>(1.0f, 1.0f), vec2<f32>(1.0f, 1.0f)), Expr(true));
  WrapInFunction(expr);

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(),
            R"(error: no matching call to select(mat2x2<f32>, mat2x2<f32>, bool)

2 candidate functions:
  select(T, T, bool) -> T  where: T is scalar
  select(vecN<T>, vecN<T>, vecN<bool>) -> vecN<T>  where: T is scalar
)");
}

TEST_F(ResolverIntrinsicTest, Select_Error_MismatchTypes) {
  auto* expr = Call("select", 1.0f, vec2<f32>(2.0f, 3.0f), Expr(true));
  WrapInFunction(expr);

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(),
            R"(error: no matching call to select(f32, vec2<f32>, bool)

2 candidate functions:
  select(T, T, bool) -> T  where: T is scalar
  select(vecN<T>, vecN<T>, vecN<bool>) -> vecN<T>  where: T is scalar
)");
}

TEST_F(ResolverIntrinsicTest, Select_Error_MismatchVectorSize) {
  auto* expr = Call("select", vec2<f32>(1.0f, 2.0f),
                    vec3<f32>(3.0f, 4.0f, 5.0f), Expr(true));
  WrapInFunction(expr);

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(),
            R"(error: no matching call to select(vec2<f32>, vec3<f32>, bool)

2 candidate functions:
  select(T, T, bool) -> T  where: T is scalar
  select(vecN<T>, vecN<T>, vecN<bool>) -> vecN<T>  where: T is scalar
)");
}

struct IntrinsicData {
  const char* name;
  IntrinsicType intrinsic;
};

inline std::ostream& operator<<(std::ostream& out, IntrinsicData data) {
  out << data.name;
  return out;
}

using ResolverIntrinsicTest_Barrier = ResolverTestWithParam<IntrinsicData>;
TEST_P(ResolverIntrinsicTest_Barrier, InferType) {
  auto param = GetParam();

  auto* call = Call(param.name);
  WrapInFunction(call);

  EXPECT_TRUE(r()->Resolve()) << r()->error();
  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->Is<sem::Void>());
}

TEST_P(ResolverIntrinsicTest_Barrier, Error_TooManyParams) {
  auto param = GetParam();

  auto* call = Call(param.name, vec4<f32>(1.f, 2.f, 3.f, 4.f), 1.0f);
  WrapInFunction(call);

  EXPECT_FALSE(r()->Resolve());

  EXPECT_THAT(r()->error(), HasSubstr("error: no matching call to " +
                                      std::string(param.name)));
}

INSTANTIATE_TEST_SUITE_P(
    ResolverTest,
    ResolverIntrinsicTest_Barrier,
    testing::Values(
        IntrinsicData{"storageBarrier", IntrinsicType::kStorageBarrier},
        IntrinsicData{"workgroupBarrier", IntrinsicType::kWorkgroupBarrier}));

using ResolverIntrinsicTest_DataPacking = ResolverTestWithParam<IntrinsicData>;
TEST_P(ResolverIntrinsicTest_DataPacking, InferType) {
  auto param = GetParam();

  bool pack4 = param.intrinsic == IntrinsicType::kPack4x8Snorm ||
               param.intrinsic == IntrinsicType::kPack4x8Unorm;

  auto* call = pack4 ? Call(param.name, vec4<f32>(1.f, 2.f, 3.f, 4.f))
                     : Call(param.name, vec2<f32>(1.f, 2.f));
  WrapInFunction(call);

  EXPECT_TRUE(r()->Resolve()) << r()->error();
  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->Is<sem::U32>());
}

TEST_P(ResolverIntrinsicTest_DataPacking, Error_IncorrectParamType) {
  auto param = GetParam();

  bool pack4 = param.intrinsic == IntrinsicType::kPack4x8Snorm ||
               param.intrinsic == IntrinsicType::kPack4x8Unorm;

  auto* call = pack4 ? Call(param.name, vec4<i32>(1, 2, 3, 4))
                     : Call(param.name, vec2<i32>(1, 2));
  WrapInFunction(call);

  EXPECT_FALSE(r()->Resolve());

  EXPECT_THAT(r()->error(), HasSubstr("error: no matching call to " +
                                      std::string(param.name)));
}

TEST_P(ResolverIntrinsicTest_DataPacking, Error_NoParams) {
  auto param = GetParam();

  auto* call = Call(param.name);
  WrapInFunction(call);

  EXPECT_FALSE(r()->Resolve());

  EXPECT_THAT(r()->error(), HasSubstr("error: no matching call to " +
                                      std::string(param.name)));
}

TEST_P(ResolverIntrinsicTest_DataPacking, Error_TooManyParams) {
  auto param = GetParam();

  bool pack4 = param.intrinsic == IntrinsicType::kPack4x8Snorm ||
               param.intrinsic == IntrinsicType::kPack4x8Unorm;

  auto* call = pack4 ? Call(param.name, vec4<f32>(1.f, 2.f, 3.f, 4.f), 1.0f)
                     : Call(param.name, vec2<f32>(1.f, 2.f), 1.0f);
  WrapInFunction(call);

  EXPECT_FALSE(r()->Resolve());

  EXPECT_THAT(r()->error(), HasSubstr("error: no matching call to " +
                                      std::string(param.name)));
}

INSTANTIATE_TEST_SUITE_P(
    ResolverTest,
    ResolverIntrinsicTest_DataPacking,
    testing::Values(
        IntrinsicData{"pack4x8snorm", IntrinsicType::kPack4x8Snorm},
        IntrinsicData{"pack4x8unorm", IntrinsicType::kPack4x8Unorm},
        IntrinsicData{"pack2x16snorm", IntrinsicType::kPack2x16Snorm},
        IntrinsicData{"pack2x16unorm", IntrinsicType::kPack2x16Unorm},
        IntrinsicData{"pack2x16float", IntrinsicType::kPack2x16Float}));

using ResolverIntrinsicTest_DataUnpacking =
    ResolverTestWithParam<IntrinsicData>;
TEST_P(ResolverIntrinsicTest_DataUnpacking, InferType) {
  auto param = GetParam();

  bool pack4 = param.intrinsic == IntrinsicType::kUnpack4x8Snorm ||
               param.intrinsic == IntrinsicType::kUnpack4x8Unorm;

  auto* call = Call(param.name, 1u);
  WrapInFunction(call);

  EXPECT_TRUE(r()->Resolve()) << r()->error();
  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->is_float_vector());
  if (pack4) {
    EXPECT_EQ(TypeOf(call)->As<sem::Vector>()->size(), 4u);
  } else {
    EXPECT_EQ(TypeOf(call)->As<sem::Vector>()->size(), 2u);
  }
}

INSTANTIATE_TEST_SUITE_P(
    ResolverTest,
    ResolverIntrinsicTest_DataUnpacking,
    testing::Values(
        IntrinsicData{"unpack4x8snorm", IntrinsicType::kUnpack4x8Snorm},
        IntrinsicData{"unpack4x8unorm", IntrinsicType::kUnpack4x8Unorm},
        IntrinsicData{"unpack2x16snorm", IntrinsicType::kUnpack2x16Snorm},
        IntrinsicData{"unpack2x16unorm", IntrinsicType::kUnpack2x16Unorm},
        IntrinsicData{"unpack2x16float", IntrinsicType::kUnpack2x16Float}));

using ResolverIntrinsicTest_SingleParam = ResolverTestWithParam<IntrinsicData>;
TEST_P(ResolverIntrinsicTest_SingleParam, Scalar) {
  auto param = GetParam();

  auto* call = Call(param.name, 1.f);
  WrapInFunction(call);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->is_float_scalar());
}

TEST_P(ResolverIntrinsicTest_SingleParam, Vector) {
  auto param = GetParam();

  auto* call = Call(param.name, vec3<f32>(1.0f, 1.0f, 3.0f));
  WrapInFunction(call);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->is_float_vector());
  EXPECT_EQ(TypeOf(call)->As<sem::Vector>()->size(), 3u);
}

TEST_P(ResolverIntrinsicTest_SingleParam, Error_NoParams) {
  auto param = GetParam();

  auto* call = Call(param.name);
  WrapInFunction(call);

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(),
            "error: no matching call to " + std::string(param.name) +
                "()\n\n"
                "2 candidate functions:\n  " +
                std::string(param.name) + "(f32) -> f32\n  " +
                std::string(param.name) + "(vecN<f32>) -> vecN<f32>\n");
}

TEST_P(ResolverIntrinsicTest_SingleParam, Error_TooManyParams) {
  auto param = GetParam();

  auto* call = Call(param.name, 1, 2, 3);
  WrapInFunction(call);

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(),
            "error: no matching call to " + std::string(param.name) +
                "(i32, i32, i32)\n\n"
                "2 candidate functions:\n  " +
                std::string(param.name) + "(f32) -> f32\n  " +
                std::string(param.name) + "(vecN<f32>) -> vecN<f32>\n");
}

INSTANTIATE_TEST_SUITE_P(
    ResolverTest,
    ResolverIntrinsicTest_SingleParam,
    testing::Values(IntrinsicData{"acos", IntrinsicType::kAcos},
                    IntrinsicData{"asin", IntrinsicType::kAsin},
                    IntrinsicData{"atan", IntrinsicType::kAtan},
                    IntrinsicData{"ceil", IntrinsicType::kCeil},
                    IntrinsicData{"cos", IntrinsicType::kCos},
                    IntrinsicData{"cosh", IntrinsicType::kCosh},
                    IntrinsicData{"exp", IntrinsicType::kExp},
                    IntrinsicData{"exp2", IntrinsicType::kExp2},
                    IntrinsicData{"floor", IntrinsicType::kFloor},
                    IntrinsicData{"fract", IntrinsicType::kFract},
                    IntrinsicData{"inverseSqrt", IntrinsicType::kInverseSqrt},
                    IntrinsicData{"log", IntrinsicType::kLog},
                    IntrinsicData{"log2", IntrinsicType::kLog2},
                    IntrinsicData{"round", IntrinsicType::kRound},
                    IntrinsicData{"sign", IntrinsicType::kSign},
                    IntrinsicData{"sin", IntrinsicType::kSin},
                    IntrinsicData{"sinh", IntrinsicType::kSinh},
                    IntrinsicData{"sqrt", IntrinsicType::kSqrt},
                    IntrinsicData{"tan", IntrinsicType::kTan},
                    IntrinsicData{"tanh", IntrinsicType::kTanh},
                    IntrinsicData{"trunc", IntrinsicType::kTrunc}));

using ResolverIntrinsicDataTest = ResolverTest;

TEST_F(ResolverIntrinsicDataTest, ArrayLength_Vector) {
  auto* ary = ty.array<i32>();
  auto* str = Structure("S", {Member("x", ary)},
                        {create<ast::StructBlockDecoration>()});
  auto* ac = ty.access(ast::AccessControl::kReadOnly, str);
  Global("a", ac, ast::StorageClass::kStorage, nullptr,
         {
             create<ast::BindingDecoration>(0),
             create<ast::GroupDecoration>(0),
         });

  auto* call = Call("arrayLength", MemberAccessor("a", "x"));
  WrapInFunction(call);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->Is<sem::U32>());
}

TEST_F(ResolverIntrinsicDataTest, ArrayLength_Error_ArraySized) {
  Global("arr", ty.array<int, 4>(), ast::StorageClass::kInput);
  auto* call = Call("arrayLength", "arr");
  WrapInFunction(call);

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(),
            "error: no matching call to arrayLength(array<i32, 4>)\n\n"
            "1 candidate function:\n"
            "  arrayLength(array<T>) -> u32\n");
}

TEST_F(ResolverIntrinsicDataTest, Normalize_Vector) {
  auto* call = Call("normalize", vec3<f32>(1.0f, 1.0f, 3.0f));
  WrapInFunction(call);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->is_float_vector());
  EXPECT_EQ(TypeOf(call)->As<sem::Vector>()->size(), 3u);
}

TEST_F(ResolverIntrinsicDataTest, Normalize_Error_NoParams) {
  auto* call = Call("normalize");
  WrapInFunction(call);

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(),
            "error: no matching call to normalize()\n\n"
            "1 candidate function:\n"
            "  normalize(vecN<f32>) -> vecN<f32>\n");
}

TEST_F(ResolverIntrinsicDataTest, FrexpScalar) {
  Global("exp", ty.i32(), ast::StorageClass::kWorkgroup);
  auto* call = Call("frexp", 1.0f, AddressOf("exp"));
  WrapInFunction(call);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->Is<sem::F32>());
}

TEST_F(ResolverIntrinsicDataTest, FrexpVector) {
  Global("exp", ty.vec3<i32>(), ast::StorageClass::kWorkgroup);
  auto* call = Call("frexp", vec3<f32>(1.0f, 2.0f, 3.0f), AddressOf("exp"));
  WrapInFunction(call);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->Is<sem::Vector>());
  EXPECT_TRUE(TypeOf(call)->As<sem::Vector>()->type()->Is<sem::F32>());
}

TEST_F(ResolverIntrinsicDataTest, Frexp_Error_FirstParamInt) {
  Global("exp", ty.i32(), ast::StorageClass::kWorkgroup);
  auto* call = Call("frexp", 1, AddressOf("exp"));
  WrapInFunction(call);

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(),
            "error: no matching call to frexp(i32, ptr<workgroup, i32>)\n\n"
            "2 candidate functions:\n"
            "  frexp(f32, ptr<T>) -> f32  where: T is i32 or u32\n"
            "  frexp(vecN<f32>, ptr<vecN<T>>) -> vecN<f32>  "
            "where: T is i32 or u32\n");
}

TEST_F(ResolverIntrinsicDataTest, Frexp_Error_SecondParamFloatPtr) {
  Global("exp", ty.f32(), ast::StorageClass::kWorkgroup);
  auto* call = Call("frexp", 1.0f, AddressOf("exp"));
  WrapInFunction(call);

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(),
            "error: no matching call to frexp(f32, ptr<workgroup, f32>)\n\n"
            "2 candidate functions:\n"
            "  frexp(f32, ptr<T>) -> f32  where: T is i32 or u32\n"
            "  frexp(vecN<f32>, ptr<vecN<T>>) -> vecN<f32>  "
            "where: T is i32 or u32\n");
}

TEST_F(ResolverIntrinsicDataTest, Frexp_Error_SecondParamNotAPointer) {
  auto* call = Call("frexp", 1.0f, 1);
  WrapInFunction(call);

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(),
            "error: no matching call to frexp(f32, i32)\n\n"
            "2 candidate functions:\n"
            "  frexp(f32, ptr<T>) -> f32  where: T is i32 or u32\n"
            "  frexp(vecN<f32>, ptr<vecN<T>>) -> vecN<f32>  "
            "where: T is i32 or u32\n");
}

TEST_F(ResolverIntrinsicDataTest, Frexp_Error_VectorSizesDontMatch) {
  Global("exp", ty.vec4<i32>(), ast::StorageClass::kWorkgroup);
  auto* call = Call("frexp", vec2<f32>(1.0f, 2.0f), AddressOf("exp"));
  WrapInFunction(call);

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(
      r()->error(),
      R"(error: no matching call to frexp(vec2<f32>, ptr<workgroup, vec4<i32>>)

2 candidate functions:
  frexp(f32, ptr<T>) -> f32  where: T is i32 or u32
  frexp(vecN<f32>, ptr<vecN<T>>) -> vecN<f32>  where: T is i32 or u32
)");
}

TEST_F(ResolverIntrinsicDataTest, ModfScalar) {
  Global("whole", ty.f32(), ast::StorageClass::kWorkgroup);
  auto* call = Call("modf", 1.0f, AddressOf("whole"));
  WrapInFunction(call);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->Is<sem::F32>());
}

TEST_F(ResolverIntrinsicDataTest, ModfVector) {
  Global("whole", ty.vec3<f32>(), ast::StorageClass::kWorkgroup);
  auto* call = Call("modf", vec3<f32>(1.0f, 2.0f, 3.0f), AddressOf("whole"));
  WrapInFunction(call);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->Is<sem::Vector>());
  EXPECT_TRUE(TypeOf(call)->As<sem::Vector>()->type()->Is<sem::F32>());
}

TEST_F(ResolverIntrinsicDataTest, Modf_Error_FirstParamInt) {
  Global("whole", ty.f32(), ast::StorageClass::kWorkgroup);
  auto* call = Call("modf", 1, AddressOf("whole"));
  WrapInFunction(call);

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(),
            "error: no matching call to modf(i32, ptr<workgroup, f32>)\n\n"
            "2 candidate functions:\n"
            "  modf(f32, ptr<f32>) -> f32\n"
            "  modf(vecN<f32>, ptr<vecN<f32>>) -> vecN<f32>\n");
}

TEST_F(ResolverIntrinsicDataTest, Modf_Error_SecondParamIntPtr) {
  Global("whole", ty.i32(), ast::StorageClass::kWorkgroup);
  auto* call = Call("modf", 1.0f, AddressOf("whole"));
  WrapInFunction(call);

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(),
            "error: no matching call to modf(f32, ptr<workgroup, i32>)\n\n"
            "2 candidate functions:\n"
            "  modf(f32, ptr<f32>) -> f32\n"
            "  modf(vecN<f32>, ptr<vecN<f32>>) -> vecN<f32>\n");
}

TEST_F(ResolverIntrinsicDataTest, Modf_Error_SecondParamNotAPointer) {
  auto* call = Call("modf", 1.0f, 1.0f);
  WrapInFunction(call);

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(),
            "error: no matching call to modf(f32, f32)\n\n"
            "2 candidate functions:\n"
            "  modf(f32, ptr<f32>) -> f32\n"
            "  modf(vecN<f32>, ptr<vecN<f32>>) -> vecN<f32>\n");
}

TEST_F(ResolverIntrinsicDataTest, Modf_Error_VectorSizesDontMatch) {
  Global("whole", ty.vec4<f32>(), ast::StorageClass::kWorkgroup);
  auto* call = Call("modf", vec2<f32>(1.0f, 2.0f), AddressOf("whole"));
  WrapInFunction(call);

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(
      r()->error(),
      R"(error: no matching call to modf(vec2<f32>, ptr<workgroup, vec4<f32>>)

2 candidate functions:
  modf(vecN<f32>, ptr<vecN<f32>>) -> vecN<f32>
  modf(f32, ptr<f32>) -> f32
)");
}

using ResolverIntrinsicTest_SingleParam_FloatOrInt =
    ResolverTestWithParam<IntrinsicData>;
TEST_P(ResolverIntrinsicTest_SingleParam_FloatOrInt, Float_Scalar) {
  auto param = GetParam();

  auto* call = Call(param.name, 1.f);
  WrapInFunction(call);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->is_float_scalar());
}

TEST_P(ResolverIntrinsicTest_SingleParam_FloatOrInt, Float_Vector) {
  auto param = GetParam();

  auto* call = Call(param.name, vec3<f32>(1.0f, 1.0f, 3.0f));
  WrapInFunction(call);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->is_float_vector());
  EXPECT_EQ(TypeOf(call)->As<sem::Vector>()->size(), 3u);
}

TEST_P(ResolverIntrinsicTest_SingleParam_FloatOrInt, Sint_Scalar) {
  auto param = GetParam();

  auto* call = Call(param.name, -1);
  WrapInFunction(call);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->Is<sem::I32>());
}

TEST_P(ResolverIntrinsicTest_SingleParam_FloatOrInt, Sint_Vector) {
  auto param = GetParam();

  auto* call = Call(param.name, vec3<i32>(1, 1, 3));
  WrapInFunction(call);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->is_signed_integer_vector());
  EXPECT_EQ(TypeOf(call)->As<sem::Vector>()->size(), 3u);
}

TEST_P(ResolverIntrinsicTest_SingleParam_FloatOrInt, Uint_Scalar) {
  auto param = GetParam();

  auto* call = Call(param.name, 1u);
  WrapInFunction(call);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->Is<sem::U32>());
}

TEST_P(ResolverIntrinsicTest_SingleParam_FloatOrInt, Uint_Vector) {
  auto param = GetParam();

  auto* call = Call(param.name, vec3<u32>(1u, 1u, 3u));
  WrapInFunction(call);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->is_unsigned_integer_vector());
  EXPECT_EQ(TypeOf(call)->As<sem::Vector>()->size(), 3u);
}

TEST_P(ResolverIntrinsicTest_SingleParam_FloatOrInt, Error_NoParams) {
  auto param = GetParam();

  auto* call = Call(param.name);
  WrapInFunction(call);

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(),
            "error: no matching call to " + std::string(param.name) +
                "()\n\n"
                "2 candidate functions:\n  " +
                std::string(param.name) +
                "(T) -> T  where: T is f32, i32 or u32\n  " +
                std::string(param.name) +
                "(vecN<T>) -> vecN<T>  where: T is f32, i32 or u32\n");
}

INSTANTIATE_TEST_SUITE_P(ResolverTest,
                         ResolverIntrinsicTest_SingleParam_FloatOrInt,
                         testing::Values(IntrinsicData{"abs",
                                                       IntrinsicType::kAbs}));

TEST_F(ResolverIntrinsicTest, Length_Scalar) {
  auto* call = Call("length", 1.f);
  WrapInFunction(call);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->is_float_scalar());
}

TEST_F(ResolverIntrinsicTest, Length_FloatVector) {
  auto* call = Call("length", vec3<f32>(1.0f, 1.0f, 3.0f));
  WrapInFunction(call);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->is_float_scalar());
}

using ResolverIntrinsicTest_TwoParam = ResolverTestWithParam<IntrinsicData>;
TEST_P(ResolverIntrinsicTest_TwoParam, Scalar) {
  auto param = GetParam();

  auto* call = Call(param.name, 1.f, 1.f);
  WrapInFunction(call);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->is_float_scalar());
}

TEST_P(ResolverIntrinsicTest_TwoParam, Vector) {
  auto param = GetParam();

  auto* call = Call(param.name, vec3<f32>(1.0f, 1.0f, 3.0f),
                    vec3<f32>(1.0f, 1.0f, 3.0f));
  WrapInFunction(call);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->is_float_vector());
  EXPECT_EQ(TypeOf(call)->As<sem::Vector>()->size(), 3u);
}

TEST_P(ResolverIntrinsicTest_TwoParam, Error_NoTooManyParams) {
  auto param = GetParam();

  auto* call = Call(param.name, 1, 2, 3);
  WrapInFunction(call);

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(),
            "error: no matching call to " + std::string(param.name) +
                "(i32, i32, i32)\n\n"
                "2 candidate functions:\n  " +
                std::string(param.name) + "(f32, f32) -> f32\n  " +
                std::string(param.name) +
                "(vecN<f32>, vecN<f32>) -> vecN<f32>\n");
}

TEST_P(ResolverIntrinsicTest_TwoParam, Error_NoParams) {
  auto param = GetParam();

  auto* call = Call(param.name);
  WrapInFunction(call);

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(),
            "error: no matching call to " + std::string(param.name) +
                "()\n\n"
                "2 candidate functions:\n  " +
                std::string(param.name) + "(f32, f32) -> f32\n  " +
                std::string(param.name) +
                "(vecN<f32>, vecN<f32>) -> vecN<f32>\n");
}

INSTANTIATE_TEST_SUITE_P(
    ResolverTest,
    ResolverIntrinsicTest_TwoParam,
    testing::Values(IntrinsicData{"atan2", IntrinsicType::kAtan2},
                    IntrinsicData{"pow", IntrinsicType::kPow},
                    IntrinsicData{"step", IntrinsicType::kStep},
                    IntrinsicData{"reflect", IntrinsicType::kReflect}));

TEST_F(ResolverIntrinsicTest, Distance_Scalar) {
  auto* call = Call("distance", 1.f, 1.f);
  WrapInFunction(call);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->is_float_scalar());
}

TEST_F(ResolverIntrinsicTest, Distance_Vector) {
  auto* call = Call("distance", vec3<f32>(1.0f, 1.0f, 3.0f),
                    vec3<f32>(1.0f, 1.0f, 3.0f));
  WrapInFunction(call);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->Is<sem::F32>());
}

TEST_F(ResolverIntrinsicTest, Cross) {
  auto* call =
      Call("cross", vec3<f32>(1.0f, 2.0f, 3.0f), vec3<f32>(1.0f, 2.0f, 3.0f));
  WrapInFunction(call);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->is_float_vector());
  EXPECT_EQ(TypeOf(call)->As<sem::Vector>()->size(), 3u);
}

TEST_F(ResolverIntrinsicTest, Cross_Error_NoArgs) {
  auto* call = Call("cross");
  WrapInFunction(call);

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(), R"(error: no matching call to cross()

1 candidate function:
  cross(vec3<f32>, vec3<f32>) -> vec3<f32>
)");
}

TEST_F(ResolverIntrinsicTest, Cross_Error_Scalar) {
  auto* call = Call("cross", 1.0f, 1.0f);
  WrapInFunction(call);

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(), R"(error: no matching call to cross(f32, f32)

1 candidate function:
  cross(vec3<f32>, vec3<f32>) -> vec3<f32>
)");
}

TEST_F(ResolverIntrinsicTest, Cross_Error_Vec3Int) {
  auto* call = Call("cross", vec3<i32>(1, 2, 3), vec3<i32>(1, 2, 3));
  WrapInFunction(call);

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(),
            R"(error: no matching call to cross(vec3<i32>, vec3<i32>)

1 candidate function:
  cross(vec3<f32>, vec3<f32>) -> vec3<f32>
)");
}

TEST_F(ResolverIntrinsicTest, Cross_Error_Vec4) {
  auto* call = Call("cross", vec4<f32>(1.0f, 2.0f, 3.0f, 4.0f),
                    vec4<f32>(1.0f, 2.0f, 3.0f, 4.0f));

  WrapInFunction(call);

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(),
            R"(error: no matching call to cross(vec4<f32>, vec4<f32>)

1 candidate function:
  cross(vec3<f32>, vec3<f32>) -> vec3<f32>
)");
}

TEST_F(ResolverIntrinsicTest, Cross_Error_TooManyParams) {
  auto* call = Call("cross", vec3<f32>(1.0f, 2.0f, 3.0f),
                    vec3<f32>(1.0f, 2.0f, 3.0f), vec3<f32>(1.0f, 2.0f, 3.0f));

  WrapInFunction(call);

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(),
            R"(error: no matching call to cross(vec3<f32>, vec3<f32>, vec3<f32>)

1 candidate function:
  cross(vec3<f32>, vec3<f32>) -> vec3<f32>
)");
}
TEST_F(ResolverIntrinsicTest, Normalize) {
  auto* call = Call("normalize", vec3<f32>(1.0f, 1.0f, 3.0f));
  WrapInFunction(call);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->is_float_vector());
  EXPECT_EQ(TypeOf(call)->As<sem::Vector>()->size(), 3u);
}

TEST_F(ResolverIntrinsicTest, Normalize_NoArgs) {
  auto* call = Call("normalize");
  WrapInFunction(call);

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(), R"(error: no matching call to normalize()

1 candidate function:
  normalize(vecN<f32>) -> vecN<f32>
)");
}

using ResolverIntrinsicTest_ThreeParam = ResolverTestWithParam<IntrinsicData>;
TEST_P(ResolverIntrinsicTest_ThreeParam, Scalar) {
  auto param = GetParam();

  auto* call = Call(param.name, 1.f, 1.f, 1.f);
  WrapInFunction(call);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->is_float_scalar());
}

TEST_P(ResolverIntrinsicTest_ThreeParam, Vector) {
  auto param = GetParam();

  auto* call = Call(param.name, vec3<f32>(1.0f, 1.0f, 3.0f),
                    vec3<f32>(1.0f, 1.0f, 3.0f), vec3<f32>(1.0f, 1.0f, 3.0f));
  WrapInFunction(call);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->is_float_vector());
  EXPECT_EQ(TypeOf(call)->As<sem::Vector>()->size(), 3u);
}
TEST_P(ResolverIntrinsicTest_ThreeParam, Error_NoParams) {
  auto param = GetParam();

  auto* call = Call(param.name);
  WrapInFunction(call);

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(),
            "error: no matching call to " + std::string(param.name) +
                "()\n\n"
                "2 candidate functions:\n  " +
                std::string(param.name) + "(f32, f32, f32) -> f32\n  " +
                std::string(param.name) +
                "(vecN<f32>, vecN<f32>, vecN<f32>) -> vecN<f32>\n");
}

INSTANTIATE_TEST_SUITE_P(
    ResolverTest,
    ResolverIntrinsicTest_ThreeParam,
    testing::Values(IntrinsicData{"mix", IntrinsicType::kMix},
                    IntrinsicData{"smoothStep", IntrinsicType::kSmoothStep},
                    IntrinsicData{"fma", IntrinsicType::kFma},
                    IntrinsicData{"faceForward", IntrinsicType::kFaceForward}));

using ResolverIntrinsicTest_ThreeParam_FloatOrInt =
    ResolverTestWithParam<IntrinsicData>;
TEST_P(ResolverIntrinsicTest_ThreeParam_FloatOrInt, Float_Scalar) {
  auto param = GetParam();

  auto* call = Call(param.name, 1.f, 1.f, 1.f);
  WrapInFunction(call);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->is_float_scalar());
}

TEST_P(ResolverIntrinsicTest_ThreeParam_FloatOrInt, Float_Vector) {
  auto param = GetParam();

  auto* call = Call(param.name, vec3<f32>(1.0f, 1.0f, 3.0f),
                    vec3<f32>(1.0f, 1.0f, 3.0f), vec3<f32>(1.0f, 1.0f, 3.0f));
  WrapInFunction(call);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->is_float_vector());
  EXPECT_EQ(TypeOf(call)->As<sem::Vector>()->size(), 3u);
}

TEST_P(ResolverIntrinsicTest_ThreeParam_FloatOrInt, Sint_Scalar) {
  auto param = GetParam();

  auto* call = Call(param.name, 1, 1, 1);
  WrapInFunction(call);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->Is<sem::I32>());
}

TEST_P(ResolverIntrinsicTest_ThreeParam_FloatOrInt, Sint_Vector) {
  auto param = GetParam();

  auto* call = Call(param.name, vec3<i32>(1, 1, 3), vec3<i32>(1, 1, 3),
                    vec3<i32>(1, 1, 3));
  WrapInFunction(call);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->is_signed_integer_vector());
  EXPECT_EQ(TypeOf(call)->As<sem::Vector>()->size(), 3u);
}

TEST_P(ResolverIntrinsicTest_ThreeParam_FloatOrInt, Uint_Scalar) {
  auto param = GetParam();

  auto* call = Call(param.name, 1u, 1u, 1u);
  WrapInFunction(call);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->Is<sem::U32>());
}

TEST_P(ResolverIntrinsicTest_ThreeParam_FloatOrInt, Uint_Vector) {
  auto param = GetParam();

  auto* call = Call(param.name, vec3<u32>(1u, 1u, 3u), vec3<u32>(1u, 1u, 3u),
                    vec3<u32>(1u, 1u, 3u));
  WrapInFunction(call);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->is_unsigned_integer_vector());
  EXPECT_EQ(TypeOf(call)->As<sem::Vector>()->size(), 3u);
}

TEST_P(ResolverIntrinsicTest_ThreeParam_FloatOrInt, Error_NoParams) {
  auto param = GetParam();

  auto* call = Call(param.name);
  WrapInFunction(call);

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(),
            "error: no matching call to " + std::string(param.name) +
                "()\n\n"
                "2 candidate functions:\n  " +
                std::string(param.name) +
                "(T, T, T) -> T  where: T is f32, i32 or u32\n  " +
                std::string(param.name) +
                "(vecN<T>, vecN<T>, vecN<T>) -> vecN<T>  where: T is f32, i32 "
                "or u32\n");
}

INSTANTIATE_TEST_SUITE_P(ResolverTest,
                         ResolverIntrinsicTest_ThreeParam_FloatOrInt,
                         testing::Values(IntrinsicData{"clamp",
                                                       IntrinsicType::kClamp}));

using ResolverIntrinsicTest_Int_SingleParam =
    ResolverTestWithParam<IntrinsicData>;
TEST_P(ResolverIntrinsicTest_Int_SingleParam, Scalar) {
  auto param = GetParam();

  auto* call = Call(param.name, 1);
  WrapInFunction(call);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->is_integer_scalar());
}

TEST_P(ResolverIntrinsicTest_Int_SingleParam, Vector) {
  auto param = GetParam();

  auto* call = Call(param.name, vec3<i32>(1, 1, 3));
  WrapInFunction(call);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->is_signed_integer_vector());
  EXPECT_EQ(TypeOf(call)->As<sem::Vector>()->size(), 3u);
}

TEST_P(ResolverIntrinsicTest_Int_SingleParam, Error_NoParams) {
  auto param = GetParam();

  auto* call = Call(param.name);
  WrapInFunction(call);

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(), "error: no matching call to " +
                              std::string(param.name) +
                              "()\n\n"
                              "2 candidate functions:\n  " +
                              std::string(param.name) +
                              "(T) -> T  where: T is i32 or u32\n  " +
                              std::string(param.name) +
                              "(vecN<T>) -> vecN<T>  where: T is i32 or u32\n");
}

INSTANTIATE_TEST_SUITE_P(
    ResolverTest,
    ResolverIntrinsicTest_Int_SingleParam,
    testing::Values(IntrinsicData{"countOneBits", IntrinsicType::kCountOneBits},
                    IntrinsicData{"reverseBits", IntrinsicType::kReverseBits}));

using ResolverIntrinsicTest_FloatOrInt_TwoParam =
    ResolverTestWithParam<IntrinsicData>;
TEST_P(ResolverIntrinsicTest_FloatOrInt_TwoParam, Scalar_Signed) {
  auto param = GetParam();

  auto* call = Call(param.name, 1, 1);
  WrapInFunction(call);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->Is<sem::I32>());
}

TEST_P(ResolverIntrinsicTest_FloatOrInt_TwoParam, Scalar_Unsigned) {
  auto param = GetParam();

  auto* call = Call(param.name, 1u, 1u);
  WrapInFunction(call);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->Is<sem::U32>());
}

TEST_P(ResolverIntrinsicTest_FloatOrInt_TwoParam, Scalar_Float) {
  auto param = GetParam();

  auto* call = Call(param.name, 1.0f, 1.0f);
  WrapInFunction(call);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->Is<sem::F32>());
}

TEST_P(ResolverIntrinsicTest_FloatOrInt_TwoParam, Vector_Signed) {
  auto param = GetParam();

  auto* call = Call(param.name, vec3<i32>(1, 1, 3), vec3<i32>(1, 1, 3));
  WrapInFunction(call);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->is_signed_integer_vector());
  EXPECT_EQ(TypeOf(call)->As<sem::Vector>()->size(), 3u);
}

TEST_P(ResolverIntrinsicTest_FloatOrInt_TwoParam, Vector_Unsigned) {
  auto param = GetParam();

  auto* call = Call(param.name, vec3<u32>(1u, 1u, 3u), vec3<u32>(1u, 1u, 3u));
  WrapInFunction(call);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->is_unsigned_integer_vector());
  EXPECT_EQ(TypeOf(call)->As<sem::Vector>()->size(), 3u);
}

TEST_P(ResolverIntrinsicTest_FloatOrInt_TwoParam, Vector_Float) {
  auto param = GetParam();

  auto* call =
      Call(param.name, vec3<f32>(1.f, 1.f, 3.f), vec3<f32>(1.f, 1.f, 3.f));
  WrapInFunction(call);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->is_float_vector());
  EXPECT_EQ(TypeOf(call)->As<sem::Vector>()->size(), 3u);
}

TEST_P(ResolverIntrinsicTest_FloatOrInt_TwoParam, Error_NoParams) {
  auto param = GetParam();

  auto* call = Call(param.name);
  WrapInFunction(call);

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(),
            "error: no matching call to " + std::string(param.name) +
                "()\n\n"
                "2 candidate functions:\n  " +
                std::string(param.name) +
                "(T, T) -> T  where: T is f32, i32 or u32\n  " +
                std::string(param.name) +
                "(vecN<T>, vecN<T>) -> vecN<T>  where: T is f32, i32 or u32\n");
}

INSTANTIATE_TEST_SUITE_P(
    ResolverTest,
    ResolverIntrinsicTest_FloatOrInt_TwoParam,
    testing::Values(IntrinsicData{"min", IntrinsicType::kMin},
                    IntrinsicData{"max", IntrinsicType::kMax}));

TEST_F(ResolverIntrinsicTest, Determinant_2x2) {
  Global("var", ty.mat2x2<f32>(), ast::StorageClass::kPrivate);

  auto* call = Call("determinant", "var");
  WrapInFunction(call);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->Is<sem::F32>());
}

TEST_F(ResolverIntrinsicTest, Determinant_3x3) {
  Global("var", ty.mat3x3<f32>(), ast::StorageClass::kPrivate);

  auto* call = Call("determinant", "var");
  WrapInFunction(call);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->Is<sem::F32>());
}

TEST_F(ResolverIntrinsicTest, Determinant_4x4) {
  Global("var", ty.mat4x4<f32>(), ast::StorageClass::kPrivate);

  auto* call = Call("determinant", "var");
  WrapInFunction(call);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->Is<sem::F32>());
}

TEST_F(ResolverIntrinsicTest, Determinant_NotSquare) {
  Global("var", ty.mat2x3<f32>(), ast::StorageClass::kPrivate);

  auto* call = Call("determinant", "var");
  WrapInFunction(call);

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(),
            "error: no matching call to determinant(mat2x3<f32>)\n\n"
            "1 candidate function:\n"
            "  determinant(matNxN<f32>) -> f32\n");
}

TEST_F(ResolverIntrinsicTest, Determinant_NotMatrix) {
  Global("var", ty.f32(), ast::StorageClass::kPrivate);

  auto* call = Call("determinant", "var");
  WrapInFunction(call);

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(),
            "error: no matching call to determinant(f32)\n\n"
            "1 candidate function:\n"
            "  determinant(matNxN<f32>) -> f32\n");
}

using ResolverIntrinsicTest_Texture =
    ResolverTestWithParam<ast::intrinsic::test::TextureOverloadCase>;

INSTANTIATE_TEST_SUITE_P(
    ResolverTest,
    ResolverIntrinsicTest_Texture,
    testing::ValuesIn(ast::intrinsic::test::TextureOverloadCase::ValidCases()));

std::string to_str(const std::string& function,
                   const sem::ParameterList& params) {
  std::stringstream out;
  out << function << "(";
  bool first = true;
  for (auto& param : params) {
    if (!first) {
      out << ", ";
    }
    out << sem::str(param.usage);
    first = false;
  }
  out << ")";
  return out.str();
}

const char* expected_texture_overload(
    ast::intrinsic::test::ValidTextureOverload overload) {
  using ValidTextureOverload = ast::intrinsic::test::ValidTextureOverload;
  switch (overload) {
    case ValidTextureOverload::kDimensions1d:
    case ValidTextureOverload::kDimensions2d:
    case ValidTextureOverload::kDimensions2dArray:
    case ValidTextureOverload::kDimensions3d:
    case ValidTextureOverload::kDimensionsCube:
    case ValidTextureOverload::kDimensionsCubeArray:
    case ValidTextureOverload::kDimensionsMultisampled2d:
    case ValidTextureOverload::kDimensionsDepth2d:
    case ValidTextureOverload::kDimensionsDepth2dArray:
    case ValidTextureOverload::kDimensionsDepthCube:
    case ValidTextureOverload::kDimensionsDepthCubeArray:
    case ValidTextureOverload::kDimensionsStorageRO1d:
    case ValidTextureOverload::kDimensionsStorageRO2d:
    case ValidTextureOverload::kDimensionsStorageRO2dArray:
    case ValidTextureOverload::kDimensionsStorageRO3d:
    case ValidTextureOverload::kDimensionsStorageWO1d:
    case ValidTextureOverload::kDimensionsStorageWO2d:
    case ValidTextureOverload::kDimensionsStorageWO2dArray:
    case ValidTextureOverload::kDimensionsStorageWO3d:
      return R"(textureDimensions(texture))";
    case ValidTextureOverload::kNumLayers2dArray:
    case ValidTextureOverload::kNumLayersCubeArray:
    case ValidTextureOverload::kNumLayersDepth2dArray:
    case ValidTextureOverload::kNumLayersDepthCubeArray:
    case ValidTextureOverload::kNumLayersStorageWO2dArray:
      return R"(textureNumLayers(texture))";
    case ValidTextureOverload::kNumLevels2d:
    case ValidTextureOverload::kNumLevels2dArray:
    case ValidTextureOverload::kNumLevels3d:
    case ValidTextureOverload::kNumLevelsCube:
    case ValidTextureOverload::kNumLevelsCubeArray:
    case ValidTextureOverload::kNumLevelsDepth2d:
    case ValidTextureOverload::kNumLevelsDepth2dArray:
    case ValidTextureOverload::kNumLevelsDepthCube:
    case ValidTextureOverload::kNumLevelsDepthCubeArray:
      return R"(textureNumLevels(texture))";
    case ValidTextureOverload::kNumSamplesMultisampled2d:
      return R"(textureNumSamples(texture))";
    case ValidTextureOverload::kDimensions2dLevel:
    case ValidTextureOverload::kDimensions2dArrayLevel:
    case ValidTextureOverload::kDimensions3dLevel:
    case ValidTextureOverload::kDimensionsCubeLevel:
    case ValidTextureOverload::kDimensionsCubeArrayLevel:
    case ValidTextureOverload::kDimensionsDepth2dLevel:
    case ValidTextureOverload::kDimensionsDepth2dArrayLevel:
    case ValidTextureOverload::kDimensionsDepthCubeLevel:
    case ValidTextureOverload::kDimensionsDepthCubeArrayLevel:
      return R"(textureDimensions(texture, level))";
    case ValidTextureOverload::kSample1dF32:
      return R"(textureSample(texture, sampler, coords))";
    case ValidTextureOverload::kSample2dF32:
      return R"(textureSample(texture, sampler, coords))";
    case ValidTextureOverload::kSample2dOffsetF32:
      return R"(textureSample(texture, sampler, coords, offset))";
    case ValidTextureOverload::kSample2dArrayF32:
      return R"(textureSample(texture, sampler, coords, array_index))";
    case ValidTextureOverload::kSample2dArrayOffsetF32:
      return R"(textureSample(texture, sampler, coords, array_index, offset))";
    case ValidTextureOverload::kSample3dF32:
      return R"(textureSample(texture, sampler, coords))";
    case ValidTextureOverload::kSample3dOffsetF32:
      return R"(textureSample(texture, sampler, coords, offset))";
    case ValidTextureOverload::kSampleCubeF32:
      return R"(textureSample(texture, sampler, coords))";
    case ValidTextureOverload::kSampleCubeArrayF32:
      return R"(textureSample(texture, sampler, coords, array_index))";
    case ValidTextureOverload::kSampleDepth2dF32:
      return R"(textureSample(texture, sampler, coords))";
    case ValidTextureOverload::kSampleDepth2dOffsetF32:
      return R"(textureSample(texture, sampler, coords, offset))";
    case ValidTextureOverload::kSampleDepth2dArrayF32:
      return R"(textureSample(texture, sampler, coords, array_index))";
    case ValidTextureOverload::kSampleDepth2dArrayOffsetF32:
      return R"(textureSample(texture, sampler, coords, array_index, offset))";
    case ValidTextureOverload::kSampleDepthCubeF32:
      return R"(textureSample(texture, sampler, coords))";
    case ValidTextureOverload::kSampleDepthCubeArrayF32:
      return R"(textureSample(texture, sampler, coords, array_index))";
    case ValidTextureOverload::kSampleBias2dF32:
      return R"(textureSampleBias(texture, sampler, coords, bias))";
    case ValidTextureOverload::kSampleBias2dOffsetF32:
      return R"(textureSampleBias(texture, sampler, coords, bias, offset))";
    case ValidTextureOverload::kSampleBias2dArrayF32:
      return R"(textureSampleBias(texture, sampler, coords, array_index, bias))";
    case ValidTextureOverload::kSampleBias2dArrayOffsetF32:
      return R"(textureSampleBias(texture, sampler, coords, array_index, bias, offset))";
    case ValidTextureOverload::kSampleBias3dF32:
      return R"(textureSampleBias(texture, sampler, coords, bias))";
    case ValidTextureOverload::kSampleBias3dOffsetF32:
      return R"(textureSampleBias(texture, sampler, coords, bias, offset))";
    case ValidTextureOverload::kSampleBiasCubeF32:
      return R"(textureSampleBias(texture, sampler, coords, bias))";
    case ValidTextureOverload::kSampleBiasCubeArrayF32:
      return R"(textureSampleBias(texture, sampler, coords, array_index, bias))";
    case ValidTextureOverload::kSampleLevel2dF32:
      return R"(textureSampleLevel(texture, sampler, coords, level))";
    case ValidTextureOverload::kSampleLevel2dOffsetF32:
      return R"(textureSampleLevel(texture, sampler, coords, level, offset))";
    case ValidTextureOverload::kSampleLevel2dArrayF32:
      return R"(textureSampleLevel(texture, sampler, coords, array_index, level))";
    case ValidTextureOverload::kSampleLevel2dArrayOffsetF32:
      return R"(textureSampleLevel(texture, sampler, coords, array_index, level, offset))";
    case ValidTextureOverload::kSampleLevel3dF32:
      return R"(textureSampleLevel(texture, sampler, coords, level))";
    case ValidTextureOverload::kSampleLevel3dOffsetF32:
      return R"(textureSampleLevel(texture, sampler, coords, level, offset))";
    case ValidTextureOverload::kSampleLevelCubeF32:
      return R"(textureSampleLevel(texture, sampler, coords, level))";
    case ValidTextureOverload::kSampleLevelCubeArrayF32:
      return R"(textureSampleLevel(texture, sampler, coords, array_index, level))";
    case ValidTextureOverload::kSampleLevelDepth2dF32:
      return R"(textureSampleLevel(texture, sampler, coords, level))";
    case ValidTextureOverload::kSampleLevelDepth2dOffsetF32:
      return R"(textureSampleLevel(texture, sampler, coords, level, offset))";
    case ValidTextureOverload::kSampleLevelDepth2dArrayF32:
      return R"(textureSampleLevel(texture, sampler, coords, array_index, level))";
    case ValidTextureOverload::kSampleLevelDepth2dArrayOffsetF32:
      return R"(textureSampleLevel(texture, sampler, coords, array_index, level, offset))";
    case ValidTextureOverload::kSampleLevelDepthCubeF32:
      return R"(textureSampleLevel(texture, sampler, coords, level))";
    case ValidTextureOverload::kSampleLevelDepthCubeArrayF32:
      return R"(textureSampleLevel(texture, sampler, coords, array_index, level))";
    case ValidTextureOverload::kSampleGrad2dF32:
      return R"(textureSampleGrad(texture, sampler, coords, ddx, ddy))";
    case ValidTextureOverload::kSampleGrad2dOffsetF32:
      return R"(textureSampleGrad(texture, sampler, coords, ddx, ddy, offset))";
    case ValidTextureOverload::kSampleGrad2dArrayF32:
      return R"(textureSampleGrad(texture, sampler, coords, array_index, ddx, ddy))";
    case ValidTextureOverload::kSampleGrad2dArrayOffsetF32:
      return R"(textureSampleGrad(texture, sampler, coords, array_index, ddx, ddy, offset))";
    case ValidTextureOverload::kSampleGrad3dF32:
      return R"(textureSampleGrad(texture, sampler, coords, ddx, ddy))";
    case ValidTextureOverload::kSampleGrad3dOffsetF32:
      return R"(textureSampleGrad(texture, sampler, coords, ddx, ddy, offset))";
    case ValidTextureOverload::kSampleGradCubeF32:
      return R"(textureSampleGrad(texture, sampler, coords, ddx, ddy))";
    case ValidTextureOverload::kSampleGradCubeArrayF32:
      return R"(textureSampleGrad(texture, sampler, coords, array_index, ddx, ddy))";
    case ValidTextureOverload::kSampleCompareDepth2dF32:
      return R"(textureSampleCompare(texture, sampler, coords, depth_ref))";
    case ValidTextureOverload::kSampleCompareDepth2dOffsetF32:
      return R"(textureSampleCompare(texture, sampler, coords, depth_ref, offset))";
    case ValidTextureOverload::kSampleCompareDepth2dArrayF32:
      return R"(textureSampleCompare(texture, sampler, coords, array_index, depth_ref))";
    case ValidTextureOverload::kSampleCompareDepth2dArrayOffsetF32:
      return R"(textureSampleCompare(texture, sampler, coords, array_index, depth_ref, offset))";
    case ValidTextureOverload::kSampleCompareDepthCubeF32:
      return R"(textureSampleCompare(texture, sampler, coords, depth_ref))";
    case ValidTextureOverload::kSampleCompareDepthCubeArrayF32:
      return R"(textureSampleCompare(texture, sampler, coords, array_index, depth_ref))";
    case ValidTextureOverload::kLoad1dLevelF32:
      return R"(textureLoad(texture, coords, level))";
    case ValidTextureOverload::kLoad1dLevelU32:
      return R"(textureLoad(texture, coords, level))";
    case ValidTextureOverload::kLoad1dLevelI32:
      return R"(textureLoad(texture, coords, level))";
    case ValidTextureOverload::kLoad2dLevelF32:
      return R"(textureLoad(texture, coords, level))";
    case ValidTextureOverload::kLoad2dLevelU32:
      return R"(textureLoad(texture, coords, level))";
    case ValidTextureOverload::kLoad2dLevelI32:
      return R"(textureLoad(texture, coords, level))";
    case ValidTextureOverload::kLoad2dArrayLevelF32:
      return R"(textureLoad(texture, coords, array_index, level))";
    case ValidTextureOverload::kLoad2dArrayLevelU32:
      return R"(textureLoad(texture, coords, array_index, level))";
    case ValidTextureOverload::kLoad2dArrayLevelI32:
      return R"(textureLoad(texture, coords, array_index, level))";
    case ValidTextureOverload::kLoad3dLevelF32:
      return R"(textureLoad(texture, coords, level))";
    case ValidTextureOverload::kLoad3dLevelU32:
      return R"(textureLoad(texture, coords, level))";
    case ValidTextureOverload::kLoad3dLevelI32:
      return R"(textureLoad(texture, coords, level))";
    case ValidTextureOverload::kLoadMultisampled2dF32:
      return R"(textureLoad(texture, coords, sample_index))";
    case ValidTextureOverload::kLoadMultisampled2dU32:
      return R"(textureLoad(texture, coords, sample_index))";
    case ValidTextureOverload::kLoadMultisampled2dI32:
      return R"(textureLoad(texture, coords, sample_index))";
    case ValidTextureOverload::kLoadDepth2dLevelF32:
      return R"(textureLoad(texture, coords, level))";
    case ValidTextureOverload::kLoadDepth2dArrayLevelF32:
      return R"(textureLoad(texture, coords, array_index, level))";
    case ValidTextureOverload::kLoadStorageRO1dRgba32float:
      return R"(textureLoad(texture, coords))";
    case ValidTextureOverload::kLoadStorageRO2dRgba8unorm:
    case ValidTextureOverload::kLoadStorageRO2dRgba8snorm:
    case ValidTextureOverload::kLoadStorageRO2dRgba8uint:
    case ValidTextureOverload::kLoadStorageRO2dRgba8sint:
    case ValidTextureOverload::kLoadStorageRO2dRgba16uint:
    case ValidTextureOverload::kLoadStorageRO2dRgba16sint:
    case ValidTextureOverload::kLoadStorageRO2dRgba16float:
    case ValidTextureOverload::kLoadStorageRO2dR32uint:
    case ValidTextureOverload::kLoadStorageRO2dR32sint:
    case ValidTextureOverload::kLoadStorageRO2dR32float:
    case ValidTextureOverload::kLoadStorageRO2dRg32uint:
    case ValidTextureOverload::kLoadStorageRO2dRg32sint:
    case ValidTextureOverload::kLoadStorageRO2dRg32float:
    case ValidTextureOverload::kLoadStorageRO2dRgba32uint:
    case ValidTextureOverload::kLoadStorageRO2dRgba32sint:
    case ValidTextureOverload::kLoadStorageRO2dRgba32float:
      return R"(textureLoad(texture, coords))";
    case ValidTextureOverload::kLoadStorageRO2dArrayRgba32float:
      return R"(textureLoad(texture, coords, array_index))";
    case ValidTextureOverload::kLoadStorageRO3dRgba32float:
      return R"(textureLoad(texture, coords))";
    case ValidTextureOverload::kStoreWO1dRgba32float:
      return R"(textureStore(texture, coords, value))";
    case ValidTextureOverload::kStoreWO2dRgba32float:
      return R"(textureStore(texture, coords, value))";
    case ValidTextureOverload::kStoreWO2dArrayRgba32float:
      return R"(textureStore(texture, coords, array_index, value))";
    case ValidTextureOverload::kStoreWO3dRgba32float:
      return R"(textureStore(texture, coords, value))";
  }
  return "<unmatched texture overload>";
}

TEST_P(ResolverIntrinsicTest_Texture, Call) {
  auto param = GetParam();

  param.buildTextureVariable(this);
  param.buildSamplerVariable(this);

  auto* call = Call(param.function, param.args(this));
  WrapInFunction(call);

  ASSERT_TRUE(r()->Resolve()) << r()->error();

  if (std::string(param.function) == "textureDimensions") {
    switch (param.texture_dimension) {
      default:
        FAIL() << "invalid texture dimensions: " << param.texture_dimension;
      case ast::TextureDimension::k1d:
        EXPECT_TRUE(TypeOf(call)->Is<sem::I32>());
        break;
      case ast::TextureDimension::k2d:
      case ast::TextureDimension::k2dArray: {
        auto* vec = As<sem::Vector>(TypeOf(call));
        ASSERT_NE(vec, nullptr);
        EXPECT_EQ(vec->size(), 2u);
        EXPECT_TRUE(vec->type()->Is<sem::I32>());
        break;
      }
      case ast::TextureDimension::k3d:
      case ast::TextureDimension::kCube:
      case ast::TextureDimension::kCubeArray: {
        auto* vec = As<sem::Vector>(TypeOf(call));
        ASSERT_NE(vec, nullptr);
        EXPECT_EQ(vec->size(), 3u);
        EXPECT_TRUE(vec->type()->Is<sem::I32>());
        break;
      }
    }
  } else if (std::string(param.function) == "textureNumLayers") {
    EXPECT_TRUE(TypeOf(call)->Is<sem::I32>());
  } else if (std::string(param.function) == "textureNumLevels") {
    EXPECT_TRUE(TypeOf(call)->Is<sem::I32>());
  } else if (std::string(param.function) == "textureNumSamples") {
    EXPECT_TRUE(TypeOf(call)->Is<sem::I32>());
  } else if (std::string(param.function) == "textureStore") {
    EXPECT_TRUE(TypeOf(call)->Is<sem::Void>());
  } else {
    switch (param.texture_kind) {
      case ast::intrinsic::test::TextureKind::kRegular:
      case ast::intrinsic::test::TextureKind::kMultisampled:
      case ast::intrinsic::test::TextureKind::kStorage: {
        auto* vec = TypeOf(call)->As<sem::Vector>();
        ASSERT_NE(vec, nullptr);
        switch (param.texture_data_type) {
          case ast::intrinsic::test::TextureDataType::kF32:
            EXPECT_TRUE(vec->type()->Is<sem::F32>());
            break;
          case ast::intrinsic::test::TextureDataType::kU32:
            EXPECT_TRUE(vec->type()->Is<sem::U32>());
            break;
          case ast::intrinsic::test::TextureDataType::kI32:
            EXPECT_TRUE(vec->type()->Is<sem::I32>());
            break;
        }
        break;
      }
      case ast::intrinsic::test::TextureKind::kDepth: {
        EXPECT_TRUE(TypeOf(call)->Is<sem::F32>());
        break;
      }
    }
  }

  auto* call_sem = Sem().Get(call);
  ASSERT_NE(call_sem, nullptr);
  auto* target = call_sem->Target();
  ASSERT_NE(target, nullptr);

  auto got = resolver::to_str(param.function, target->Parameters());
  auto* expected = expected_texture_overload(param.overload);
  EXPECT_EQ(got, expected);
}

}  // namespace
}  // namespace resolver
}  // namespace tint
