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

#include "src/tint/resolver/resolver.h"

#include "gmock/gmock.h"
#include "src/tint/ast/assignment_statement.h"
#include "src/tint/ast/bitcast_expression.h"
#include "src/tint/ast/break_statement.h"
#include "src/tint/ast/builtin_texture_helper_test.h"
#include "src/tint/ast/call_statement.h"
#include "src/tint/ast/continue_statement.h"
#include "src/tint/ast/if_statement.h"
#include "src/tint/ast/loop_statement.h"
#include "src/tint/ast/return_statement.h"
#include "src/tint/ast/stage_attribute.h"
#include "src/tint/ast/switch_statement.h"
#include "src/tint/ast/unary_op_expression.h"
#include "src/tint/ast/variable_decl_statement.h"
#include "src/tint/resolver/resolver_test_helper.h"
#include "src/tint/sem/call.h"
#include "src/tint/sem/function.h"
#include "src/tint/sem/member_accessor_expression.h"
#include "src/tint/sem/sampled_texture.h"
#include "src/tint/sem/statement.h"
#include "src/tint/sem/variable.h"

using ::testing::ElementsAre;
using ::testing::HasSubstr;

using namespace tint::number_suffixes;  // NOLINT

namespace tint::resolver {
namespace {

using BuiltinType = sem::BuiltinType;

using ResolverBuiltinTest = ResolverTest;

using ResolverBuiltinDerivativeTest = ResolverTestWithParam<std::string>;
TEST_P(ResolverBuiltinDerivativeTest, Scalar) {
    auto name = GetParam();

    Global("ident", ty.f32(), ast::StorageClass::kPrivate);

    auto* expr = Call(name, "ident");
    Func("func", {}, ty.void_(), {Ignore(expr)},
         {create<ast::StageAttribute>(ast::PipelineStage::kFragment)});

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(expr), nullptr);
    ASSERT_TRUE(TypeOf(expr)->Is<sem::F32>());
}

TEST_P(ResolverBuiltinDerivativeTest, Vector) {
    auto name = GetParam();
    Global("ident", ty.vec4<f32>(), ast::StorageClass::kPrivate);

    auto* expr = Call(name, "ident");
    Func("func", {}, ty.void_(), {Ignore(expr)},
         {create<ast::StageAttribute>(ast::PipelineStage::kFragment)});

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(expr), nullptr);
    ASSERT_TRUE(TypeOf(expr)->Is<sem::Vector>());
    EXPECT_TRUE(TypeOf(expr)->As<sem::Vector>()->type()->Is<sem::F32>());
    EXPECT_EQ(TypeOf(expr)->As<sem::Vector>()->Width(), 4u);
}

TEST_P(ResolverBuiltinDerivativeTest, MissingParam) {
    auto name = GetParam();

    auto* expr = Call(name);
    WrapInFunction(expr);

    EXPECT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(), "error: no matching call to " + name +
                                "()\n\n"
                                "2 candidate functions:\n  " +
                                name + "(f32) -> f32\n  " + name + "(vecN<f32>) -> vecN<f32>\n");
}

INSTANTIATE_TEST_SUITE_P(ResolverTest,
                         ResolverBuiltinDerivativeTest,
                         testing::Values("dpdx",
                                         "dpdxCoarse",
                                         "dpdxFine",
                                         "dpdy",
                                         "dpdyCoarse",
                                         "dpdyFine",
                                         "fwidth",
                                         "fwidthCoarse",
                                         "fwidthFine"));

using ResolverBuiltinTest_BoolMethod = ResolverTestWithParam<std::string>;
TEST_P(ResolverBuiltinTest_BoolMethod, Scalar) {
    auto name = GetParam();

    Global("my_var", ty.bool_(), ast::StorageClass::kPrivate);

    auto* expr = Call(name, "my_var");
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(expr), nullptr);
    EXPECT_TRUE(TypeOf(expr)->Is<sem::Bool>());
}
TEST_P(ResolverBuiltinTest_BoolMethod, Vector) {
    auto name = GetParam();

    Global("my_var", ty.vec3<bool>(), ast::StorageClass::kPrivate);

    auto* expr = Call(name, "my_var");
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(expr), nullptr);
    EXPECT_TRUE(TypeOf(expr)->Is<sem::Bool>());
}
INSTANTIATE_TEST_SUITE_P(ResolverTest,
                         ResolverBuiltinTest_BoolMethod,
                         testing::Values("any", "all"));

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
    ast::TexelFormat format = ast::TexelFormat::kR32Float;
};
inline std::ostream& operator<<(std::ostream& out, TextureTestParams data) {
    out << data.dim << "_" << data.type;
    return out;
}

class ResolverBuiltinTest_TextureOperation : public ResolverTestWithParam<TextureTestParams> {
  public:
    /// Gets an appropriate type for the coords parameter depending the the
    /// dimensionality of the texture being sampled.
    /// @param dim dimensionality of the texture being sampled
    /// @param scalar the scalar type
    /// @returns a pointer to a type appropriate for the coord param
    const ast::Type* GetCoordsType(ast::TextureDimension dim, const ast::Type* scalar) {
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

    void add_call_param(std::string name, const ast::Type* type, ast::ExpressionList* call_params) {
        if (type->IsAnyOf<ast::Texture, ast::Sampler>()) {
            Global(name, type,
                   ast::AttributeList{
                       create<ast::BindingAttribute>(0),
                       create<ast::GroupAttribute>(0),
                   });

        } else {
            Global(name, type, ast::StorageClass::kPrivate);
        }

        call_params->push_back(Expr(name));
    }
    const ast::Type* subtype(Texture type) {
        if (type == Texture::kF32) {
            return ty.f32();
        }
        if (type == Texture::kI32) {
            return ty.i32();
        }
        return ty.u32();
    }
};

using ResolverBuiltinTest_SampledTextureOperation = ResolverBuiltinTest_TextureOperation;
TEST_P(ResolverBuiltinTest_SampledTextureOperation, TextureLoadSampled) {
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
    EXPECT_EQ(TypeOf(expr)->As<sem::Vector>()->Width(), 4u);
}

INSTANTIATE_TEST_SUITE_P(ResolverTest,
                         ResolverBuiltinTest_SampledTextureOperation,
                         testing::Values(TextureTestParams{ast::TextureDimension::k1d},
                                         TextureTestParams{ast::TextureDimension::k2d},
                                         TextureTestParams{ast::TextureDimension::k2dArray},
                                         TextureTestParams{ast::TextureDimension::k3d}));

TEST_F(ResolverBuiltinTest, Dot_Vec2) {
    Global("my_var", ty.vec2<f32>(), ast::StorageClass::kPrivate);

    auto* expr = Call("dot", "my_var", "my_var");
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(expr), nullptr);
    EXPECT_TRUE(TypeOf(expr)->Is<sem::F32>());
}

TEST_F(ResolverBuiltinTest, Dot_Vec3) {
    Global("my_var", ty.vec3<i32>(), ast::StorageClass::kPrivate);

    auto* expr = Call("dot", "my_var", "my_var");
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(expr), nullptr);
    EXPECT_TRUE(TypeOf(expr)->Is<sem::I32>());
}

TEST_F(ResolverBuiltinTest, Dot_Vec4) {
    Global("my_var", ty.vec4<u32>(), ast::StorageClass::kPrivate);

    auto* expr = Call("dot", "my_var", "my_var");
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(expr), nullptr);
    EXPECT_TRUE(TypeOf(expr)->Is<sem::U32>());
}

TEST_F(ResolverBuiltinTest, Dot_Error_Scalar) {
    auto* expr = Call("dot", 1.0f, 1.0f);
    WrapInFunction(expr);

    EXPECT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(),
              R"(error: no matching call to dot(f32, f32)

1 candidate function:
  dot(vecN<T>, vecN<T>) -> T  where: T is f32, i32 or u32
)");
}

TEST_F(ResolverBuiltinTest, Select) {
    Global("my_var", ty.vec3<f32>(), ast::StorageClass::kPrivate);

    Global("bool_var", ty.vec3<bool>(), ast::StorageClass::kPrivate);

    auto* expr = Call("select", "my_var", "my_var", "bool_var");
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(expr), nullptr);
    EXPECT_TRUE(TypeOf(expr)->Is<sem::Vector>());
    EXPECT_EQ(TypeOf(expr)->As<sem::Vector>()->Width(), 3u);
    EXPECT_TRUE(TypeOf(expr)->As<sem::Vector>()->type()->Is<sem::F32>());
}

TEST_F(ResolverBuiltinTest, Select_Error_NoParams) {
    auto* expr = Call("select");
    WrapInFunction(expr);

    EXPECT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(),
              R"(error: no matching call to select()

3 candidate functions:
  select(T, T, bool) -> T  where: T is f32, i32, u32 or bool
  select(vecN<T>, vecN<T>, bool) -> vecN<T>  where: T is f32, i32, u32 or bool
  select(vecN<T>, vecN<T>, vecN<bool>) -> vecN<T>  where: T is f32, i32, u32 or bool
)");
}

TEST_F(ResolverBuiltinTest, Select_Error_SelectorInt) {
    auto* expr = Call("select", 1_i, 1_i, 1_i);
    WrapInFunction(expr);

    EXPECT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(),
              R"(error: no matching call to select(i32, i32, i32)

3 candidate functions:
  select(T, T, bool) -> T  where: T is f32, i32, u32 or bool
  select(vecN<T>, vecN<T>, bool) -> vecN<T>  where: T is f32, i32, u32 or bool
  select(vecN<T>, vecN<T>, vecN<bool>) -> vecN<T>  where: T is f32, i32, u32 or bool
)");
}

TEST_F(ResolverBuiltinTest, Select_Error_Matrix) {
    auto* expr = Call("select", mat2x2<f32>(vec2<f32>(1.0f, 1.0f), vec2<f32>(1.0f, 1.0f)),
                      mat2x2<f32>(vec2<f32>(1.0f, 1.0f), vec2<f32>(1.0f, 1.0f)), Expr(true));
    WrapInFunction(expr);

    EXPECT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(),
              R"(error: no matching call to select(mat2x2<f32>, mat2x2<f32>, bool)

3 candidate functions:
  select(T, T, bool) -> T  where: T is f32, i32, u32 or bool
  select(vecN<T>, vecN<T>, bool) -> vecN<T>  where: T is f32, i32, u32 or bool
  select(vecN<T>, vecN<T>, vecN<bool>) -> vecN<T>  where: T is f32, i32, u32 or bool
)");
}

TEST_F(ResolverBuiltinTest, Select_Error_MismatchTypes) {
    auto* expr = Call("select", 1.0f, vec2<f32>(2.0f, 3.0f), Expr(true));
    WrapInFunction(expr);

    EXPECT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(),
              R"(error: no matching call to select(f32, vec2<f32>, bool)

3 candidate functions:
  select(T, T, bool) -> T  where: T is f32, i32, u32 or bool
  select(vecN<T>, vecN<T>, bool) -> vecN<T>  where: T is f32, i32, u32 or bool
  select(vecN<T>, vecN<T>, vecN<bool>) -> vecN<T>  where: T is f32, i32, u32 or bool
)");
}

TEST_F(ResolverBuiltinTest, Select_Error_MismatchVectorSize) {
    auto* expr = Call("select", vec2<f32>(1.0f, 2.0f), vec3<f32>(3.0f, 4.0f, 5.0f), Expr(true));
    WrapInFunction(expr);

    EXPECT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(),
              R"(error: no matching call to select(vec2<f32>, vec3<f32>, bool)

3 candidate functions:
  select(T, T, bool) -> T  where: T is f32, i32, u32 or bool
  select(vecN<T>, vecN<T>, bool) -> vecN<T>  where: T is f32, i32, u32 or bool
  select(vecN<T>, vecN<T>, vecN<bool>) -> vecN<T>  where: T is f32, i32, u32 or bool
)");
}

struct BuiltinData {
    const char* name;
    BuiltinType builtin;
};

inline std::ostream& operator<<(std::ostream& out, BuiltinData data) {
    out << data.name;
    return out;
}

using ResolverBuiltinTest_Barrier = ResolverTestWithParam<BuiltinData>;
TEST_P(ResolverBuiltinTest_Barrier, InferType) {
    auto param = GetParam();

    auto* call = Call(param.name);
    WrapInFunction(CallStmt(call));

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    ASSERT_NE(TypeOf(call), nullptr);
    EXPECT_TRUE(TypeOf(call)->Is<sem::Void>());
}

TEST_P(ResolverBuiltinTest_Barrier, Error_TooManyParams) {
    auto param = GetParam();

    auto* call = Call(param.name, vec4<f32>(1.f, 2.f, 3.f, 4.f), 1.0f);
    WrapInFunction(CallStmt(call));

    EXPECT_FALSE(r()->Resolve());

    EXPECT_THAT(r()->error(), HasSubstr("error: no matching call to " + std::string(param.name)));
}

INSTANTIATE_TEST_SUITE_P(
    ResolverTest,
    ResolverBuiltinTest_Barrier,
    testing::Values(BuiltinData{"storageBarrier", BuiltinType::kStorageBarrier},
                    BuiltinData{"workgroupBarrier", BuiltinType::kWorkgroupBarrier}));

using ResolverBuiltinTest_DataPacking = ResolverTestWithParam<BuiltinData>;
TEST_P(ResolverBuiltinTest_DataPacking, InferType) {
    auto param = GetParam();

    bool pack4 =
        param.builtin == BuiltinType::kPack4x8snorm || param.builtin == BuiltinType::kPack4x8unorm;

    auto* call = pack4 ? Call(param.name, vec4<f32>(1.f, 2.f, 3.f, 4.f))
                       : Call(param.name, vec2<f32>(1.f, 2.f));
    WrapInFunction(call);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    ASSERT_NE(TypeOf(call), nullptr);
    EXPECT_TRUE(TypeOf(call)->Is<sem::U32>());
}

TEST_P(ResolverBuiltinTest_DataPacking, Error_IncorrectParamType) {
    auto param = GetParam();

    bool pack4 =
        param.builtin == BuiltinType::kPack4x8snorm || param.builtin == BuiltinType::kPack4x8unorm;

    auto* call = pack4 ? Call(param.name, vec4<i32>(1_i, 2_i, 3_i, 4_i))
                       : Call(param.name, vec2<i32>(1_i, 2_i));
    WrapInFunction(call);

    EXPECT_FALSE(r()->Resolve());

    EXPECT_THAT(r()->error(), HasSubstr("error: no matching call to " + std::string(param.name)));
}

TEST_P(ResolverBuiltinTest_DataPacking, Error_NoParams) {
    auto param = GetParam();

    auto* call = Call(param.name);
    WrapInFunction(call);

    EXPECT_FALSE(r()->Resolve());

    EXPECT_THAT(r()->error(), HasSubstr("error: no matching call to " + std::string(param.name)));
}

TEST_P(ResolverBuiltinTest_DataPacking, Error_TooManyParams) {
    auto param = GetParam();

    bool pack4 =
        param.builtin == BuiltinType::kPack4x8snorm || param.builtin == BuiltinType::kPack4x8unorm;

    auto* call = pack4 ? Call(param.name, vec4<f32>(1.f, 2.f, 3.f, 4.f), 1.0f)
                       : Call(param.name, vec2<f32>(1.f, 2.f), 1.0f);
    WrapInFunction(call);

    EXPECT_FALSE(r()->Resolve());

    EXPECT_THAT(r()->error(), HasSubstr("error: no matching call to " + std::string(param.name)));
}

INSTANTIATE_TEST_SUITE_P(ResolverTest,
                         ResolverBuiltinTest_DataPacking,
                         testing::Values(BuiltinData{"pack4x8snorm", BuiltinType::kPack4x8snorm},
                                         BuiltinData{"pack4x8unorm", BuiltinType::kPack4x8unorm},
                                         BuiltinData{"pack2x16snorm", BuiltinType::kPack2x16snorm},
                                         BuiltinData{"pack2x16unorm", BuiltinType::kPack2x16unorm},
                                         BuiltinData{"pack2x16float",
                                                     BuiltinType::kPack2x16float}));

using ResolverBuiltinTest_DataUnpacking = ResolverTestWithParam<BuiltinData>;
TEST_P(ResolverBuiltinTest_DataUnpacking, InferType) {
    auto param = GetParam();

    bool pack4 = param.builtin == BuiltinType::kUnpack4x8snorm ||
                 param.builtin == BuiltinType::kUnpack4x8unorm;

    auto* call = Call(param.name, 1_u);
    WrapInFunction(call);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    ASSERT_NE(TypeOf(call), nullptr);
    EXPECT_TRUE(TypeOf(call)->is_float_vector());
    if (pack4) {
        EXPECT_EQ(TypeOf(call)->As<sem::Vector>()->Width(), 4u);
    } else {
        EXPECT_EQ(TypeOf(call)->As<sem::Vector>()->Width(), 2u);
    }
}

INSTANTIATE_TEST_SUITE_P(
    ResolverTest,
    ResolverBuiltinTest_DataUnpacking,
    testing::Values(BuiltinData{"unpack4x8snorm", BuiltinType::kUnpack4x8snorm},
                    BuiltinData{"unpack4x8unorm", BuiltinType::kUnpack4x8unorm},
                    BuiltinData{"unpack2x16snorm", BuiltinType::kUnpack2x16snorm},
                    BuiltinData{"unpack2x16unorm", BuiltinType::kUnpack2x16unorm},
                    BuiltinData{"unpack2x16float", BuiltinType::kUnpack2x16float}));

using ResolverBuiltinTest_SingleParam = ResolverTestWithParam<BuiltinData>;
TEST_P(ResolverBuiltinTest_SingleParam, Scalar) {
    auto param = GetParam();

    auto* call = Call(param.name, 1.f);
    WrapInFunction(call);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(call), nullptr);
    EXPECT_TRUE(TypeOf(call)->is_float_scalar());
}

TEST_P(ResolverBuiltinTest_SingleParam, Vector) {
    auto param = GetParam();

    auto* call = Call(param.name, vec3<f32>(1.0f, 1.0f, 3.0f));
    WrapInFunction(call);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(call), nullptr);
    EXPECT_TRUE(TypeOf(call)->is_float_vector());
    EXPECT_EQ(TypeOf(call)->As<sem::Vector>()->Width(), 3u);
}

TEST_P(ResolverBuiltinTest_SingleParam, Error_NoParams) {
    auto param = GetParam();

    auto* call = Call(param.name);
    WrapInFunction(call);

    EXPECT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(), "error: no matching call to " + std::string(param.name) +
                                "()\n\n"
                                "2 candidate functions:\n  " +
                                std::string(param.name) + "(f32) -> f32\n  " +
                                std::string(param.name) + "(vecN<f32>) -> vecN<f32>\n");
}

TEST_P(ResolverBuiltinTest_SingleParam, Error_TooManyParams) {
    auto param = GetParam();

    auto* call = Call(param.name, 1_i, 2_i, 3_i);
    WrapInFunction(call);

    EXPECT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(), "error: no matching call to " + std::string(param.name) +
                                "(i32, i32, i32)\n\n"
                                "2 candidate functions:\n  " +
                                std::string(param.name) + "(f32) -> f32\n  " +
                                std::string(param.name) + "(vecN<f32>) -> vecN<f32>\n");
}

INSTANTIATE_TEST_SUITE_P(ResolverTest,
                         ResolverBuiltinTest_SingleParam,
                         testing::Values(BuiltinData{"acos", BuiltinType::kAcos},
                                         BuiltinData{"asin", BuiltinType::kAsin},
                                         BuiltinData{"atan", BuiltinType::kAtan},
                                         BuiltinData{"ceil", BuiltinType::kCeil},
                                         BuiltinData{"cos", BuiltinType::kCos},
                                         BuiltinData{"cosh", BuiltinType::kCosh},
                                         BuiltinData{"exp", BuiltinType::kExp},
                                         BuiltinData{"exp2", BuiltinType::kExp2},
                                         BuiltinData{"floor", BuiltinType::kFloor},
                                         BuiltinData{"fract", BuiltinType::kFract},
                                         BuiltinData{"inverseSqrt", BuiltinType::kInverseSqrt},
                                         BuiltinData{"log", BuiltinType::kLog},
                                         BuiltinData{"log2", BuiltinType::kLog2},
                                         BuiltinData{"round", BuiltinType::kRound},
                                         BuiltinData{"sign", BuiltinType::kSign},
                                         BuiltinData{"sin", BuiltinType::kSin},
                                         BuiltinData{"sinh", BuiltinType::kSinh},
                                         BuiltinData{"sqrt", BuiltinType::kSqrt},
                                         BuiltinData{"tan", BuiltinType::kTan},
                                         BuiltinData{"tanh", BuiltinType::kTanh},
                                         BuiltinData{"trunc", BuiltinType::kTrunc}));

using ResolverBuiltinDataTest = ResolverTest;

TEST_F(ResolverBuiltinDataTest, ArrayLength_Vector) {
    auto* ary = ty.array<i32>();
    auto* str = Structure("S", {Member("x", ary)});
    Global("a", ty.Of(str), ast::StorageClass::kStorage, ast::Access::kRead,
           ast::AttributeList{
               create<ast::BindingAttribute>(0),
               create<ast::GroupAttribute>(0),
           });

    auto* call = Call("arrayLength", AddressOf(MemberAccessor("a", "x")));
    WrapInFunction(call);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(call), nullptr);
    EXPECT_TRUE(TypeOf(call)->Is<sem::U32>());
}

TEST_F(ResolverBuiltinDataTest, ArrayLength_Error_ArraySized) {
    Global("arr", ty.array<i32, 4>(), ast::StorageClass::kPrivate);
    auto* call = Call("arrayLength", AddressOf("arr"));
    WrapInFunction(call);

    EXPECT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(),
              R"(error: no matching call to arrayLength(ptr<private, array<i32, 4>, read_write>)

1 candidate function:
  arrayLength(ptr<storage, array<T>, A>) -> u32
)");
}

TEST_F(ResolverBuiltinDataTest, Normalize_Vector) {
    auto* call = Call("normalize", vec3<f32>(1.0f, 1.0f, 3.0f));
    WrapInFunction(call);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(call), nullptr);
    EXPECT_TRUE(TypeOf(call)->is_float_vector());
    EXPECT_EQ(TypeOf(call)->As<sem::Vector>()->Width(), 3u);
}

TEST_F(ResolverBuiltinDataTest, Normalize_Error_NoParams) {
    auto* call = Call("normalize");
    WrapInFunction(call);

    EXPECT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(), R"(error: no matching call to normalize()

1 candidate function:
  normalize(vecN<f32>) -> vecN<f32>
)");
}

TEST_F(ResolverBuiltinDataTest, FrexpScalar) {
    auto* call = Call("frexp", 1.0f);
    WrapInFunction(call);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(call), nullptr);
    auto* ty = TypeOf(call)->As<sem::Struct>();
    ASSERT_NE(ty, nullptr);
    ASSERT_EQ(ty->Members().size(), 2u);

    auto* sig = ty->Members()[0];
    EXPECT_TRUE(sig->Type()->Is<sem::F32>());
    EXPECT_EQ(sig->Offset(), 0u);
    EXPECT_EQ(sig->Size(), 4u);
    EXPECT_EQ(sig->Align(), 4u);
    EXPECT_EQ(sig->Name(), Sym("sig"));

    auto* exp = ty->Members()[1];
    EXPECT_TRUE(exp->Type()->Is<sem::I32>());
    EXPECT_EQ(exp->Offset(), 4u);
    EXPECT_EQ(exp->Size(), 4u);
    EXPECT_EQ(exp->Align(), 4u);
    EXPECT_EQ(exp->Name(), Sym("exp"));

    EXPECT_EQ(ty->Size(), 8u);
    EXPECT_EQ(ty->SizeNoPadding(), 8u);
}

TEST_F(ResolverBuiltinDataTest, FrexpVector) {
    auto* call = Call("frexp", vec3<f32>());
    WrapInFunction(call);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(call), nullptr);
    auto* ty = TypeOf(call)->As<sem::Struct>();
    ASSERT_NE(ty, nullptr);
    ASSERT_EQ(ty->Members().size(), 2u);

    auto* sig = ty->Members()[0];
    ASSERT_TRUE(sig->Type()->Is<sem::Vector>());
    EXPECT_EQ(sig->Type()->As<sem::Vector>()->Width(), 3u);
    EXPECT_TRUE(sig->Type()->As<sem::Vector>()->type()->Is<sem::F32>());
    EXPECT_EQ(sig->Offset(), 0u);
    EXPECT_EQ(sig->Size(), 12u);
    EXPECT_EQ(sig->Align(), 16u);
    EXPECT_EQ(sig->Name(), Sym("sig"));

    auto* exp = ty->Members()[1];
    ASSERT_TRUE(exp->Type()->Is<sem::Vector>());
    EXPECT_EQ(exp->Type()->As<sem::Vector>()->Width(), 3u);
    EXPECT_TRUE(exp->Type()->As<sem::Vector>()->type()->Is<sem::I32>());
    EXPECT_EQ(exp->Offset(), 16u);
    EXPECT_EQ(exp->Size(), 12u);
    EXPECT_EQ(exp->Align(), 16u);
    EXPECT_EQ(exp->Name(), Sym("exp"));

    EXPECT_EQ(ty->Size(), 32u);
    EXPECT_EQ(ty->SizeNoPadding(), 28u);
}

TEST_F(ResolverBuiltinDataTest, Frexp_Error_FirstParamInt) {
    Global("v", ty.i32(), ast::StorageClass::kWorkgroup);
    auto* call = Call("frexp", 1_i, AddressOf("v"));
    WrapInFunction(call);

    EXPECT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(),
              R"(error: no matching call to frexp(i32, ptr<workgroup, i32, read_write>)

2 candidate functions:
  frexp(f32) -> __frexp_result
  frexp(vecN<f32>) -> __frexp_result_vecN
)");
}

TEST_F(ResolverBuiltinDataTest, Frexp_Error_SecondParamFloatPtr) {
    Global("v", ty.f32(), ast::StorageClass::kWorkgroup);
    auto* call = Call("frexp", 1.0f, AddressOf("v"));
    WrapInFunction(call);

    EXPECT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(),
              R"(error: no matching call to frexp(f32, ptr<workgroup, f32, read_write>)

2 candidate functions:
  frexp(f32) -> __frexp_result
  frexp(vecN<f32>) -> __frexp_result_vecN
)");
}

TEST_F(ResolverBuiltinDataTest, Frexp_Error_SecondParamNotAPointer) {
    auto* call = Call("frexp", 1.0f, 1_i);
    WrapInFunction(call);

    EXPECT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(), R"(error: no matching call to frexp(f32, i32)

2 candidate functions:
  frexp(f32) -> __frexp_result
  frexp(vecN<f32>) -> __frexp_result_vecN
)");
}

TEST_F(ResolverBuiltinDataTest, Frexp_Error_VectorSizesDontMatch) {
    Global("v", ty.vec4<i32>(), ast::StorageClass::kWorkgroup);
    auto* call = Call("frexp", vec2<f32>(1.0f, 2.0f), AddressOf("v"));
    WrapInFunction(call);

    EXPECT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(),
              R"(error: no matching call to frexp(vec2<f32>, ptr<workgroup, vec4<i32>, read_write>)

2 candidate functions:
  frexp(vecN<f32>) -> __frexp_result_vecN
  frexp(f32) -> __frexp_result
)");
}

TEST_F(ResolverBuiltinDataTest, ModfScalar) {
    auto* call = Call("modf", 1.0f);
    WrapInFunction(call);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(call), nullptr);
    auto* ty = TypeOf(call)->As<sem::Struct>();
    ASSERT_NE(ty, nullptr);
    ASSERT_EQ(ty->Members().size(), 2u);

    auto* fract = ty->Members()[0];
    EXPECT_TRUE(fract->Type()->Is<sem::F32>());
    EXPECT_EQ(fract->Offset(), 0u);
    EXPECT_EQ(fract->Size(), 4u);
    EXPECT_EQ(fract->Align(), 4u);
    EXPECT_EQ(fract->Name(), Sym("fract"));

    auto* whole = ty->Members()[1];
    EXPECT_TRUE(whole->Type()->Is<sem::F32>());
    EXPECT_EQ(whole->Offset(), 4u);
    EXPECT_EQ(whole->Size(), 4u);
    EXPECT_EQ(whole->Align(), 4u);
    EXPECT_EQ(whole->Name(), Sym("whole"));

    EXPECT_EQ(ty->Size(), 8u);
    EXPECT_EQ(ty->SizeNoPadding(), 8u);
}

TEST_F(ResolverBuiltinDataTest, ModfVector) {
    auto* call = Call("modf", vec3<f32>());
    WrapInFunction(call);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(call), nullptr);
    auto* ty = TypeOf(call)->As<sem::Struct>();
    ASSERT_NE(ty, nullptr);
    ASSERT_EQ(ty->Members().size(), 2u);

    auto* fract = ty->Members()[0];
    ASSERT_TRUE(fract->Type()->Is<sem::Vector>());
    EXPECT_EQ(fract->Type()->As<sem::Vector>()->Width(), 3u);
    EXPECT_TRUE(fract->Type()->As<sem::Vector>()->type()->Is<sem::F32>());
    EXPECT_EQ(fract->Offset(), 0u);
    EXPECT_EQ(fract->Size(), 12u);
    EXPECT_EQ(fract->Align(), 16u);
    EXPECT_EQ(fract->Name(), Sym("fract"));

    auto* whole = ty->Members()[1];
    ASSERT_TRUE(whole->Type()->Is<sem::Vector>());
    EXPECT_EQ(whole->Type()->As<sem::Vector>()->Width(), 3u);
    EXPECT_TRUE(whole->Type()->As<sem::Vector>()->type()->Is<sem::F32>());
    EXPECT_EQ(whole->Offset(), 16u);
    EXPECT_EQ(whole->Size(), 12u);
    EXPECT_EQ(whole->Align(), 16u);
    EXPECT_EQ(whole->Name(), Sym("whole"));

    EXPECT_EQ(ty->Size(), 32u);
    EXPECT_EQ(ty->SizeNoPadding(), 28u);
}

TEST_F(ResolverBuiltinDataTest, Modf_Error_FirstParamInt) {
    Global("whole", ty.f32(), ast::StorageClass::kWorkgroup);
    auto* call = Call("modf", 1_i, AddressOf("whole"));
    WrapInFunction(call);

    EXPECT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(),
              R"(error: no matching call to modf(i32, ptr<workgroup, f32, read_write>)

2 candidate functions:
  modf(f32) -> __modf_result
  modf(vecN<f32>) -> __modf_result_vecN
)");
}

TEST_F(ResolverBuiltinDataTest, Modf_Error_SecondParamIntPtr) {
    Global("whole", ty.i32(), ast::StorageClass::kWorkgroup);
    auto* call = Call("modf", 1.0f, AddressOf("whole"));
    WrapInFunction(call);

    EXPECT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(),
              R"(error: no matching call to modf(f32, ptr<workgroup, i32, read_write>)

2 candidate functions:
  modf(f32) -> __modf_result
  modf(vecN<f32>) -> __modf_result_vecN
)");
}

TEST_F(ResolverBuiltinDataTest, Modf_Error_SecondParamNotAPointer) {
    auto* call = Call("modf", 1.0f, 1.0f);
    WrapInFunction(call);

    EXPECT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(), R"(error: no matching call to modf(f32, f32)

2 candidate functions:
  modf(f32) -> __modf_result
  modf(vecN<f32>) -> __modf_result_vecN
)");
}

TEST_F(ResolverBuiltinDataTest, Modf_Error_VectorSizesDontMatch) {
    Global("whole", ty.vec4<f32>(), ast::StorageClass::kWorkgroup);
    auto* call = Call("modf", vec2<f32>(1.0f, 2.0f), AddressOf("whole"));
    WrapInFunction(call);

    EXPECT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(),
              R"(error: no matching call to modf(vec2<f32>, ptr<workgroup, vec4<f32>, read_write>)

2 candidate functions:
  modf(vecN<f32>) -> __modf_result_vecN
  modf(f32) -> __modf_result
)");
}

using ResolverBuiltinTest_SingleParam_FloatOrInt = ResolverTestWithParam<BuiltinData>;
TEST_P(ResolverBuiltinTest_SingleParam_FloatOrInt, Float_Scalar) {
    auto param = GetParam();

    auto* call = Call(param.name, 1.f);
    WrapInFunction(call);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(call), nullptr);
    EXPECT_TRUE(TypeOf(call)->is_float_scalar());
}

TEST_P(ResolverBuiltinTest_SingleParam_FloatOrInt, Float_Vector) {
    auto param = GetParam();

    auto* call = Call(param.name, vec3<f32>(1.0f, 1.0f, 3.0f));
    WrapInFunction(call);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(call), nullptr);
    EXPECT_TRUE(TypeOf(call)->is_float_vector());
    EXPECT_EQ(TypeOf(call)->As<sem::Vector>()->Width(), 3u);
}

TEST_P(ResolverBuiltinTest_SingleParam_FloatOrInt, Sint_Scalar) {
    auto param = GetParam();

    auto* call = Call(param.name, i32(-1));
    WrapInFunction(call);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(call), nullptr);
    EXPECT_TRUE(TypeOf(call)->Is<sem::I32>());
}

TEST_P(ResolverBuiltinTest_SingleParam_FloatOrInt, Sint_Vector) {
    auto param = GetParam();

    auto* call = Call(param.name, vec3<i32>(1_i, 1_i, 3_i));
    WrapInFunction(call);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(call), nullptr);
    EXPECT_TRUE(TypeOf(call)->is_signed_integer_vector());
    EXPECT_EQ(TypeOf(call)->As<sem::Vector>()->Width(), 3u);
}

TEST_P(ResolverBuiltinTest_SingleParam_FloatOrInt, Uint_Scalar) {
    auto param = GetParam();

    auto* call = Call(param.name, 1_u);
    WrapInFunction(call);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(call), nullptr);
    EXPECT_TRUE(TypeOf(call)->Is<sem::U32>());
}

TEST_P(ResolverBuiltinTest_SingleParam_FloatOrInt, Uint_Vector) {
    auto param = GetParam();

    auto* call = Call(param.name, vec3<u32>(1_u, 1_u, 3_u));
    WrapInFunction(call);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(call), nullptr);
    EXPECT_TRUE(TypeOf(call)->is_unsigned_integer_vector());
    EXPECT_EQ(TypeOf(call)->As<sem::Vector>()->Width(), 3u);
}

TEST_P(ResolverBuiltinTest_SingleParam_FloatOrInt, Error_NoParams) {
    auto param = GetParam();

    auto* call = Call(param.name);
    WrapInFunction(call);

    EXPECT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(),
              "error: no matching call to " + std::string(param.name) +
                  "()\n\n"
                  "2 candidate functions:\n  " +
                  std::string(param.name) + "(T) -> T  where: T is f32, i32 or u32\n  " +
                  std::string(param.name) + "(vecN<T>) -> vecN<T>  where: T is f32, i32 or u32\n");
}

INSTANTIATE_TEST_SUITE_P(ResolverTest,
                         ResolverBuiltinTest_SingleParam_FloatOrInt,
                         testing::Values(BuiltinData{"abs", BuiltinType::kAbs}));

TEST_F(ResolverBuiltinTest, Length_Scalar) {
    auto* call = Call("length", 1.f);
    WrapInFunction(call);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(call), nullptr);
    EXPECT_TRUE(TypeOf(call)->is_float_scalar());
}

TEST_F(ResolverBuiltinTest, Length_FloatVector) {
    auto* call = Call("length", vec3<f32>(1.0f, 1.0f, 3.0f));
    WrapInFunction(call);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(call), nullptr);
    EXPECT_TRUE(TypeOf(call)->is_float_scalar());
}

using ResolverBuiltinTest_TwoParam = ResolverTestWithParam<BuiltinData>;
TEST_P(ResolverBuiltinTest_TwoParam, Scalar) {
    auto param = GetParam();

    auto* call = Call(param.name, 1.f, 1.f);
    WrapInFunction(call);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(call), nullptr);
    EXPECT_TRUE(TypeOf(call)->is_float_scalar());
}

TEST_P(ResolverBuiltinTest_TwoParam, Vector) {
    auto param = GetParam();

    auto* call = Call(param.name, vec3<f32>(1.0f, 1.0f, 3.0f), vec3<f32>(1.0f, 1.0f, 3.0f));
    WrapInFunction(call);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(call), nullptr);
    EXPECT_TRUE(TypeOf(call)->is_float_vector());
    EXPECT_EQ(TypeOf(call)->As<sem::Vector>()->Width(), 3u);
}

TEST_P(ResolverBuiltinTest_TwoParam, Error_NoTooManyParams) {
    auto param = GetParam();

    auto* call = Call(param.name, 1_i, 2_i, 3_i);
    WrapInFunction(call);

    EXPECT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(), "error: no matching call to " + std::string(param.name) +
                                "(i32, i32, i32)\n\n"
                                "2 candidate functions:\n  " +
                                std::string(param.name) + "(f32, f32) -> f32\n  " +
                                std::string(param.name) + "(vecN<f32>, vecN<f32>) -> vecN<f32>\n");
}

TEST_P(ResolverBuiltinTest_TwoParam, Error_NoParams) {
    auto param = GetParam();

    auto* call = Call(param.name);
    WrapInFunction(call);

    EXPECT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(), "error: no matching call to " + std::string(param.name) +
                                "()\n\n"
                                "2 candidate functions:\n  " +
                                std::string(param.name) + "(f32, f32) -> f32\n  " +
                                std::string(param.name) + "(vecN<f32>, vecN<f32>) -> vecN<f32>\n");
}

INSTANTIATE_TEST_SUITE_P(ResolverTest,
                         ResolverBuiltinTest_TwoParam,
                         testing::Values(BuiltinData{"atan2", BuiltinType::kAtan2},
                                         BuiltinData{"pow", BuiltinType::kPow},
                                         BuiltinData{"step", BuiltinType::kStep}));

TEST_F(ResolverBuiltinTest, Distance_Scalar) {
    auto* call = Call("distance", 1.f, 1.f);
    WrapInFunction(call);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(call), nullptr);
    EXPECT_TRUE(TypeOf(call)->is_float_scalar());
}

TEST_F(ResolverBuiltinTest, Distance_Vector) {
    auto* call = Call("distance", vec3<f32>(1.0f, 1.0f, 3.0f), vec3<f32>(1.0f, 1.0f, 3.0f));
    WrapInFunction(call);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(call), nullptr);
    EXPECT_TRUE(TypeOf(call)->Is<sem::F32>());
}

TEST_F(ResolverBuiltinTest, Cross) {
    auto* call = Call("cross", vec3<f32>(1.0f, 2.0f, 3.0f), vec3<f32>(1.0f, 2.0f, 3.0f));
    WrapInFunction(call);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(call), nullptr);
    EXPECT_TRUE(TypeOf(call)->is_float_vector());
    EXPECT_EQ(TypeOf(call)->As<sem::Vector>()->Width(), 3u);
}

TEST_F(ResolverBuiltinTest, Cross_Error_NoArgs) {
    auto* call = Call("cross");
    WrapInFunction(call);

    EXPECT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(), R"(error: no matching call to cross()

1 candidate function:
  cross(vec3<f32>, vec3<f32>) -> vec3<f32>
)");
}

TEST_F(ResolverBuiltinTest, Cross_Error_Scalar) {
    auto* call = Call("cross", 1.0f, 1.0f);
    WrapInFunction(call);

    EXPECT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(), R"(error: no matching call to cross(f32, f32)

1 candidate function:
  cross(vec3<f32>, vec3<f32>) -> vec3<f32>
)");
}

TEST_F(ResolverBuiltinTest, Cross_Error_Vec3Int) {
    auto* call = Call("cross", vec3<i32>(1_i, 2_i, 3_i), vec3<i32>(1_i, 2_i, 3_i));
    WrapInFunction(call);

    EXPECT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(),
              R"(error: no matching call to cross(vec3<i32>, vec3<i32>)

1 candidate function:
  cross(vec3<f32>, vec3<f32>) -> vec3<f32>
)");
}

TEST_F(ResolverBuiltinTest, Cross_Error_Vec4) {
    auto* call =
        Call("cross", vec4<f32>(1.0f, 2.0f, 3.0f, 4.0f), vec4<f32>(1.0f, 2.0f, 3.0f, 4.0f));

    WrapInFunction(call);

    EXPECT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(),
              R"(error: no matching call to cross(vec4<f32>, vec4<f32>)

1 candidate function:
  cross(vec3<f32>, vec3<f32>) -> vec3<f32>
)");
}

TEST_F(ResolverBuiltinTest, Cross_Error_TooManyParams) {
    auto* call = Call("cross", vec3<f32>(1.0f, 2.0f, 3.0f), vec3<f32>(1.0f, 2.0f, 3.0f),
                      vec3<f32>(1.0f, 2.0f, 3.0f));

    WrapInFunction(call);

    EXPECT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(),
              R"(error: no matching call to cross(vec3<f32>, vec3<f32>, vec3<f32>)

1 candidate function:
  cross(vec3<f32>, vec3<f32>) -> vec3<f32>
)");
}
TEST_F(ResolverBuiltinTest, Normalize) {
    auto* call = Call("normalize", vec3<f32>(1.0f, 1.0f, 3.0f));
    WrapInFunction(call);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(call), nullptr);
    EXPECT_TRUE(TypeOf(call)->is_float_vector());
    EXPECT_EQ(TypeOf(call)->As<sem::Vector>()->Width(), 3u);
}

TEST_F(ResolverBuiltinTest, Normalize_NoArgs) {
    auto* call = Call("normalize");
    WrapInFunction(call);

    EXPECT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(), R"(error: no matching call to normalize()

1 candidate function:
  normalize(vecN<f32>) -> vecN<f32>
)");
}

using ResolverBuiltinTest_ThreeParam = ResolverTestWithParam<BuiltinData>;
TEST_P(ResolverBuiltinTest_ThreeParam, Scalar) {
    auto param = GetParam();

    auto* call = Call(param.name, 1.f, 1.f, 1.f);
    WrapInFunction(call);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(call), nullptr);
    EXPECT_TRUE(TypeOf(call)->is_float_scalar());
}

TEST_P(ResolverBuiltinTest_ThreeParam, Vector) {
    auto param = GetParam();

    auto* call = Call(param.name, vec3<f32>(1.0f, 1.0f, 3.0f), vec3<f32>(1.0f, 1.0f, 3.0f),
                      vec3<f32>(1.0f, 1.0f, 3.0f));
    WrapInFunction(call);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(call), nullptr);
    EXPECT_TRUE(TypeOf(call)->is_float_vector());
    EXPECT_EQ(TypeOf(call)->As<sem::Vector>()->Width(), 3u);
}
TEST_P(ResolverBuiltinTest_ThreeParam, Error_NoParams) {
    auto param = GetParam();

    auto* call = Call(param.name);
    WrapInFunction(call);

    EXPECT_FALSE(r()->Resolve());

    EXPECT_THAT(r()->error(),
                HasSubstr("error: no matching call to " + std::string(param.name) + "()"));
}

INSTANTIATE_TEST_SUITE_P(ResolverTest,
                         ResolverBuiltinTest_ThreeParam,
                         testing::Values(BuiltinData{"mix", BuiltinType::kMix},
                                         BuiltinData{"smoothstep", BuiltinType::kSmoothstep},
                                         BuiltinData{"smoothStep", BuiltinType::kSmoothStep},
                                         BuiltinData{"fma", BuiltinType::kFma}));

using ResolverBuiltinTest_ThreeParam_FloatOrInt = ResolverTestWithParam<BuiltinData>;
TEST_P(ResolverBuiltinTest_ThreeParam_FloatOrInt, Float_Scalar) {
    auto param = GetParam();

    auto* call = Call(param.name, 1.f, 1.f, 1.f);
    WrapInFunction(call);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(call), nullptr);
    EXPECT_TRUE(TypeOf(call)->is_float_scalar());
}

TEST_P(ResolverBuiltinTest_ThreeParam_FloatOrInt, Float_Vector) {
    auto param = GetParam();

    auto* call = Call(param.name, vec3<f32>(1.0f, 1.0f, 3.0f), vec3<f32>(1.0f, 1.0f, 3.0f),
                      vec3<f32>(1.0f, 1.0f, 3.0f));
    WrapInFunction(call);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(call), nullptr);
    EXPECT_TRUE(TypeOf(call)->is_float_vector());
    EXPECT_EQ(TypeOf(call)->As<sem::Vector>()->Width(), 3u);
}

TEST_P(ResolverBuiltinTest_ThreeParam_FloatOrInt, Sint_Scalar) {
    auto param = GetParam();

    auto* call = Call(param.name, 1_i, 1_i, 1_i);
    WrapInFunction(call);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(call), nullptr);
    EXPECT_TRUE(TypeOf(call)->Is<sem::I32>());
}

TEST_P(ResolverBuiltinTest_ThreeParam_FloatOrInt, Sint_Vector) {
    auto param = GetParam();

    auto* call = Call(param.name, vec3<i32>(1_i, 1_i, 3_i), vec3<i32>(1_i, 1_i, 3_i),
                      vec3<i32>(1_i, 1_i, 3_i));
    WrapInFunction(call);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(call), nullptr);
    EXPECT_TRUE(TypeOf(call)->is_signed_integer_vector());
    EXPECT_EQ(TypeOf(call)->As<sem::Vector>()->Width(), 3u);
}

TEST_P(ResolverBuiltinTest_ThreeParam_FloatOrInt, Uint_Scalar) {
    auto param = GetParam();

    auto* call = Call(param.name, 1_u, 1_u, 1_u);
    WrapInFunction(call);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(call), nullptr);
    EXPECT_TRUE(TypeOf(call)->Is<sem::U32>());
}

TEST_P(ResolverBuiltinTest_ThreeParam_FloatOrInt, Uint_Vector) {
    auto param = GetParam();

    auto* call = Call(param.name, vec3<u32>(1_u, 1_u, 3_u), vec3<u32>(1_u, 1_u, 3_u),
                      vec3<u32>(1_u, 1_u, 3_u));
    WrapInFunction(call);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(call), nullptr);
    EXPECT_TRUE(TypeOf(call)->is_unsigned_integer_vector());
    EXPECT_EQ(TypeOf(call)->As<sem::Vector>()->Width(), 3u);
}

TEST_P(ResolverBuiltinTest_ThreeParam_FloatOrInt, Error_NoParams) {
    auto param = GetParam();

    auto* call = Call(param.name);
    WrapInFunction(call);

    EXPECT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(), "error: no matching call to " + std::string(param.name) +
                                "()\n\n"
                                "2 candidate functions:\n  " +
                                std::string(param.name) +
                                "(T, T, T) -> T  where: T is f32, i32 or u32\n  " +
                                std::string(param.name) +
                                "(vecN<T>, vecN<T>, vecN<T>) -> vecN<T>  where: T is f32, i32 "
                                "or u32\n");
}

INSTANTIATE_TEST_SUITE_P(ResolverTest,
                         ResolverBuiltinTest_ThreeParam_FloatOrInt,
                         testing::Values(BuiltinData{"clamp", BuiltinType::kClamp}));

using ResolverBuiltinTest_Int_SingleParam = ResolverTestWithParam<BuiltinData>;
TEST_P(ResolverBuiltinTest_Int_SingleParam, Scalar) {
    auto param = GetParam();

    auto* call = Call(param.name, 1_i);
    WrapInFunction(call);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(call), nullptr);
    EXPECT_TRUE(TypeOf(call)->is_integer_scalar());
}

TEST_P(ResolverBuiltinTest_Int_SingleParam, Vector) {
    auto param = GetParam();

    auto* call = Call(param.name, vec3<i32>(1_i, 1_i, 3_i));
    WrapInFunction(call);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(call), nullptr);
    EXPECT_TRUE(TypeOf(call)->is_signed_integer_vector());
    EXPECT_EQ(TypeOf(call)->As<sem::Vector>()->Width(), 3u);
}

TEST_P(ResolverBuiltinTest_Int_SingleParam, Error_NoParams) {
    auto param = GetParam();

    auto* call = Call(param.name);
    WrapInFunction(call);

    EXPECT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(), "error: no matching call to " + std::string(param.name) +
                                "()\n\n"
                                "2 candidate functions:\n  " +
                                std::string(param.name) + "(T) -> T  where: T is i32 or u32\n  " +
                                std::string(param.name) +
                                "(vecN<T>) -> vecN<T>  where: T is i32 or u32\n");
}

INSTANTIATE_TEST_SUITE_P(ResolverTest,
                         ResolverBuiltinTest_Int_SingleParam,
                         testing::Values(BuiltinData{"countOneBits", BuiltinType::kCountOneBits},
                                         BuiltinData{"reverseBits", BuiltinType::kReverseBits}));

using ResolverBuiltinTest_FloatOrInt_TwoParam = ResolverTestWithParam<BuiltinData>;
TEST_P(ResolverBuiltinTest_FloatOrInt_TwoParam, Scalar_Signed) {
    auto param = GetParam();

    auto* call = Call(param.name, 1_i, 1_i);
    WrapInFunction(call);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(call), nullptr);
    EXPECT_TRUE(TypeOf(call)->Is<sem::I32>());
}

TEST_P(ResolverBuiltinTest_FloatOrInt_TwoParam, Scalar_Unsigned) {
    auto param = GetParam();

    auto* call = Call(param.name, 1_u, 1_u);
    WrapInFunction(call);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(call), nullptr);
    EXPECT_TRUE(TypeOf(call)->Is<sem::U32>());
}

TEST_P(ResolverBuiltinTest_FloatOrInt_TwoParam, Scalar_Float) {
    auto param = GetParam();

    auto* call = Call(param.name, 1.0f, 1.0f);
    WrapInFunction(call);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(call), nullptr);
    EXPECT_TRUE(TypeOf(call)->Is<sem::F32>());
}

TEST_P(ResolverBuiltinTest_FloatOrInt_TwoParam, Vector_Signed) {
    auto param = GetParam();

    auto* call = Call(param.name, vec3<i32>(1_i, 1_i, 3_i), vec3<i32>(1_i, 1_i, 3_i));
    WrapInFunction(call);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(call), nullptr);
    EXPECT_TRUE(TypeOf(call)->is_signed_integer_vector());
    EXPECT_EQ(TypeOf(call)->As<sem::Vector>()->Width(), 3u);
}

TEST_P(ResolverBuiltinTest_FloatOrInt_TwoParam, Vector_Unsigned) {
    auto param = GetParam();

    auto* call = Call(param.name, vec3<u32>(1_u, 1_u, 3_u), vec3<u32>(1_u, 1_u, 3_u));
    WrapInFunction(call);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(call), nullptr);
    EXPECT_TRUE(TypeOf(call)->is_unsigned_integer_vector());
    EXPECT_EQ(TypeOf(call)->As<sem::Vector>()->Width(), 3u);
}

TEST_P(ResolverBuiltinTest_FloatOrInt_TwoParam, Vector_Float) {
    auto param = GetParam();

    auto* call = Call(param.name, vec3<f32>(1.f, 1.f, 3.f), vec3<f32>(1.f, 1.f, 3.f));
    WrapInFunction(call);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(call), nullptr);
    EXPECT_TRUE(TypeOf(call)->is_float_vector());
    EXPECT_EQ(TypeOf(call)->As<sem::Vector>()->Width(), 3u);
}

TEST_P(ResolverBuiltinTest_FloatOrInt_TwoParam, Error_NoParams) {
    auto param = GetParam();

    auto* call = Call(param.name);
    WrapInFunction(call);

    EXPECT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(), "error: no matching call to " + std::string(param.name) +
                                "()\n\n"
                                "2 candidate functions:\n  " +
                                std::string(param.name) +
                                "(T, T) -> T  where: T is f32, i32 or u32\n  " +
                                std::string(param.name) +
                                "(vecN<T>, vecN<T>) -> vecN<T>  where: T is f32, i32 or u32\n");
}

INSTANTIATE_TEST_SUITE_P(ResolverTest,
                         ResolverBuiltinTest_FloatOrInt_TwoParam,
                         testing::Values(BuiltinData{"min", BuiltinType::kMin},
                                         BuiltinData{"max", BuiltinType::kMax}));

TEST_F(ResolverBuiltinTest, Determinant_2x2) {
    Global("var", ty.mat2x2<f32>(), ast::StorageClass::kPrivate);

    auto* call = Call("determinant", "var");
    WrapInFunction(call);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(call), nullptr);
    EXPECT_TRUE(TypeOf(call)->Is<sem::F32>());
}

TEST_F(ResolverBuiltinTest, Determinant_3x3) {
    Global("var", ty.mat3x3<f32>(), ast::StorageClass::kPrivate);

    auto* call = Call("determinant", "var");
    WrapInFunction(call);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(call), nullptr);
    EXPECT_TRUE(TypeOf(call)->Is<sem::F32>());
}

TEST_F(ResolverBuiltinTest, Determinant_4x4) {
    Global("var", ty.mat4x4<f32>(), ast::StorageClass::kPrivate);

    auto* call = Call("determinant", "var");
    WrapInFunction(call);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(call), nullptr);
    EXPECT_TRUE(TypeOf(call)->Is<sem::F32>());
}

TEST_F(ResolverBuiltinTest, Determinant_NotSquare) {
    Global("var", ty.mat2x3<f32>(), ast::StorageClass::kPrivate);

    auto* call = Call("determinant", "var");
    WrapInFunction(call);

    EXPECT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(), R"(error: no matching call to determinant(mat2x3<f32>)

1 candidate function:
  determinant(matNxN<f32>) -> f32
)");
}

TEST_F(ResolverBuiltinTest, Determinant_NotMatrix) {
    Global("var", ty.f32(), ast::StorageClass::kPrivate);

    auto* call = Call("determinant", "var");
    WrapInFunction(call);

    EXPECT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(), R"(error: no matching call to determinant(f32)

1 candidate function:
  determinant(matNxN<f32>) -> f32
)");
}

using ResolverBuiltinTest_Texture = ResolverTestWithParam<ast::builtin::test::TextureOverloadCase>;

INSTANTIATE_TEST_SUITE_P(ResolverTest,
                         ResolverBuiltinTest_Texture,
                         testing::ValuesIn(ast::builtin::test::TextureOverloadCase::ValidCases()));

std::string to_str(const std::string& function, const sem::ParameterList& params) {
    std::stringstream out;
    out << function << "(";
    bool first = true;
    for (auto* param : params) {
        if (!first) {
            out << ", ";
        }
        out << sem::str(param->Usage());
        first = false;
    }
    out << ")";
    return out.str();
}

const char* expected_texture_overload(ast::builtin::test::ValidTextureOverload overload) {
    using ValidTextureOverload = ast::builtin::test::ValidTextureOverload;
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
        case ValidTextureOverload::kDimensionsDepthMultisampled2d:
        case ValidTextureOverload::kDimensionsStorageWO1d:
        case ValidTextureOverload::kDimensionsStorageWO2d:
        case ValidTextureOverload::kDimensionsStorageWO2dArray:
        case ValidTextureOverload::kDimensionsStorageWO3d:
            return R"(textureDimensions(texture))";
        case ValidTextureOverload::kGather2dF32:
            return R"(textureGather(component, texture, sampler, coords))";
        case ValidTextureOverload::kGather2dOffsetF32:
            return R"(textureGather(component, texture, sampler, coords, offset))";
        case ValidTextureOverload::kGather2dArrayF32:
            return R"(textureGather(component, texture, sampler, coords, array_index))";
        case ValidTextureOverload::kGather2dArrayOffsetF32:
            return R"(textureGather(component, texture, sampler, coords, array_index, offset))";
        case ValidTextureOverload::kGatherCubeF32:
            return R"(textureGather(component, texture, sampler, coords))";
        case ValidTextureOverload::kGatherCubeArrayF32:
            return R"(textureGather(component, texture, sampler, coords, array_index))";
        case ValidTextureOverload::kGatherDepth2dF32:
            return R"(textureGather(texture, sampler, coords))";
        case ValidTextureOverload::kGatherDepth2dOffsetF32:
            return R"(textureGather(texture, sampler, coords, offset))";
        case ValidTextureOverload::kGatherDepth2dArrayF32:
            return R"(textureGather(texture, sampler, coords, array_index))";
        case ValidTextureOverload::kGatherDepth2dArrayOffsetF32:
            return R"(textureGather(texture, sampler, coords, array_index, offset))";
        case ValidTextureOverload::kGatherDepthCubeF32:
            return R"(textureGather(texture, sampler, coords))";
        case ValidTextureOverload::kGatherDepthCubeArrayF32:
            return R"(textureGather(texture, sampler, coords, array_index))";
        case ValidTextureOverload::kGatherCompareDepth2dF32:
            return R"(textureGatherCompare(texture, sampler, coords, depth_ref))";
        case ValidTextureOverload::kGatherCompareDepth2dOffsetF32:
            return R"(textureGatherCompare(texture, sampler, coords, depth_ref, offset))";
        case ValidTextureOverload::kGatherCompareDepth2dArrayF32:
            return R"(textureGatherCompare(texture, sampler, coords, array_index, depth_ref))";
        case ValidTextureOverload::kGatherCompareDepth2dArrayOffsetF32:
            return R"(textureGatherCompare(texture, sampler, coords, array_index, depth_ref, offset))";
        case ValidTextureOverload::kGatherCompareDepthCubeF32:
            return R"(textureGatherCompare(texture, sampler, coords, depth_ref))";
        case ValidTextureOverload::kGatherCompareDepthCubeArrayF32:
            return R"(textureGatherCompare(texture, sampler, coords, array_index, depth_ref))";
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
        case ValidTextureOverload::kNumSamplesDepthMultisampled2d:
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
        case ValidTextureOverload::kSampleCompareLevelDepth2dF32:
            return R"(textureSampleCompare(texture, sampler, coords, depth_ref))";
        case ValidTextureOverload::kSampleCompareLevelDepth2dOffsetF32:
            return R"(textureSampleCompare(texture, sampler, coords, depth_ref, offset))";
        case ValidTextureOverload::kSampleCompareLevelDepth2dArrayF32:
            return R"(textureSampleCompare(texture, sampler, coords, array_index, depth_ref))";
        case ValidTextureOverload::kSampleCompareLevelDepth2dArrayOffsetF32:
            return R"(textureSampleCompare(texture, sampler, coords, array_index, depth_ref, offset))";
        case ValidTextureOverload::kSampleCompareLevelDepthCubeF32:
            return R"(textureSampleCompare(texture, sampler, coords, depth_ref))";
        case ValidTextureOverload::kSampleCompareLevelDepthCubeArrayF32:
            return R"(textureSampleCompare(texture, sampler, coords, array_index, depth_ref))";
        case ValidTextureOverload::kLoad1dLevelF32:
        case ValidTextureOverload::kLoad1dLevelU32:
        case ValidTextureOverload::kLoad1dLevelI32:
        case ValidTextureOverload::kLoad2dLevelF32:
        case ValidTextureOverload::kLoad2dLevelU32:
        case ValidTextureOverload::kLoad2dLevelI32:
            return R"(textureLoad(texture, coords, level))";
        case ValidTextureOverload::kLoad2dArrayLevelF32:
        case ValidTextureOverload::kLoad2dArrayLevelU32:
        case ValidTextureOverload::kLoad2dArrayLevelI32:
            return R"(textureLoad(texture, coords, array_index, level))";
        case ValidTextureOverload::kLoad3dLevelF32:
        case ValidTextureOverload::kLoad3dLevelU32:
        case ValidTextureOverload::kLoad3dLevelI32:
        case ValidTextureOverload::kLoadDepth2dLevelF32:
            return R"(textureLoad(texture, coords, level))";
        case ValidTextureOverload::kLoadDepthMultisampled2dF32:
        case ValidTextureOverload::kLoadMultisampled2dF32:
        case ValidTextureOverload::kLoadMultisampled2dU32:
        case ValidTextureOverload::kLoadMultisampled2dI32:
            return R"(textureLoad(texture, coords, sample_index))";
        case ValidTextureOverload::kLoadDepth2dArrayLevelF32:
            return R"(textureLoad(texture, coords, array_index, level))";
        case ValidTextureOverload::kStoreWO1dRgba32float:
        case ValidTextureOverload::kStoreWO2dRgba32float:
        case ValidTextureOverload::kStoreWO3dRgba32float:
            return R"(textureStore(texture, coords, value))";
        case ValidTextureOverload::kStoreWO2dArrayRgba32float:
            return R"(textureStore(texture, coords, array_index, value))";
    }
    return "<unmatched texture overload>";
}

TEST_P(ResolverBuiltinTest_Texture, Call) {
    auto param = GetParam();

    param.BuildTextureVariable(this);
    param.BuildSamplerVariable(this);

    auto* call = Call(param.function, param.args(this));
    auto* stmt = CallStmt(call);
    Func("func", {}, ty.void_(), {stmt}, {Stage(ast::PipelineStage::kFragment)});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    if (std::string(param.function) == "textureDimensions") {
        switch (param.texture_dimension) {
            default:
                FAIL() << "invalid texture dimensions: " << param.texture_dimension;
            case ast::TextureDimension::k1d:
                EXPECT_TRUE(TypeOf(call)->Is<sem::I32>());
                break;
            case ast::TextureDimension::k2d:
            case ast::TextureDimension::k2dArray:
            case ast::TextureDimension::kCube:
            case ast::TextureDimension::kCubeArray: {
                auto* vec = As<sem::Vector>(TypeOf(call));
                ASSERT_NE(vec, nullptr);
                EXPECT_EQ(vec->Width(), 2u);
                EXPECT_TRUE(vec->type()->Is<sem::I32>());
                break;
            }
            case ast::TextureDimension::k3d: {
                auto* vec = As<sem::Vector>(TypeOf(call));
                ASSERT_NE(vec, nullptr);
                EXPECT_EQ(vec->Width(), 3u);
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
    } else if (std::string(param.function) == "textureGather") {
        auto* vec = As<sem::Vector>(TypeOf(call));
        ASSERT_NE(vec, nullptr);
        EXPECT_EQ(vec->Width(), 4u);
        switch (param.texture_data_type) {
            case ast::builtin::test::TextureDataType::kF32:
                EXPECT_TRUE(vec->type()->Is<sem::F32>());
                break;
            case ast::builtin::test::TextureDataType::kU32:
                EXPECT_TRUE(vec->type()->Is<sem::U32>());
                break;
            case ast::builtin::test::TextureDataType::kI32:
                EXPECT_TRUE(vec->type()->Is<sem::I32>());
                break;
        }
    } else if (std::string(param.function) == "textureGatherCompare") {
        auto* vec = As<sem::Vector>(TypeOf(call));
        ASSERT_NE(vec, nullptr);
        EXPECT_EQ(vec->Width(), 4u);
        EXPECT_TRUE(vec->type()->Is<sem::F32>());
    } else {
        switch (param.texture_kind) {
            case ast::builtin::test::TextureKind::kRegular:
            case ast::builtin::test::TextureKind::kMultisampled:
            case ast::builtin::test::TextureKind::kStorage: {
                auto* vec = TypeOf(call)->As<sem::Vector>();
                ASSERT_NE(vec, nullptr);
                switch (param.texture_data_type) {
                    case ast::builtin::test::TextureDataType::kF32:
                        EXPECT_TRUE(vec->type()->Is<sem::F32>());
                        break;
                    case ast::builtin::test::TextureDataType::kU32:
                        EXPECT_TRUE(vec->type()->Is<sem::U32>());
                        break;
                    case ast::builtin::test::TextureDataType::kI32:
                        EXPECT_TRUE(vec->type()->Is<sem::I32>());
                        break;
                }
                break;
            }
            case ast::builtin::test::TextureKind::kDepth:
            case ast::builtin::test::TextureKind::kDepthMultisampled: {
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
}  // namespace tint::resolver
