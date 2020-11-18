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

#include <memory>

#include "gtest/gtest.h"
#include "src/ast/builder.h"
#include "src/ast/call_expression.h"
#include "src/ast/float_literal.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/intrinsic_texture_helper_test.h"
#include "src/ast/member_accessor_expression.h"
#include "src/ast/scalar_constructor_expression.h"
#include "src/ast/sint_literal.h"
#include "src/ast/struct.h"
#include "src/ast/struct_member.h"
#include "src/ast/type/array_type.h"
#include "src/ast/type/bool_type.h"
#include "src/ast/type/depth_texture_type.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/i32_type.h"
#include "src/ast/type/matrix_type.h"
#include "src/ast/type/multisampled_texture_type.h"
#include "src/ast/type/pointer_type.h"
#include "src/ast/type/sampled_texture_type.h"
#include "src/ast/type/sampler_type.h"
#include "src/ast/type/struct_type.h"
#include "src/ast/type/u32_type.h"
#include "src/ast/type/vector_type.h"
#include "src/ast/type/void_type.h"
#include "src/ast/type_constructor_expression.h"
#include "src/ast/uint_literal.h"
#include "src/ast/variable.h"
#include "src/context.h"
#include "src/type_determiner.h"
#include "src/writer/spirv/builder.h"
#include "src/writer/spirv/spv_dump.h"

namespace tint {
namespace writer {
namespace spirv {
namespace {

class IntrinsicBuilderTest : public ast::BuilderWithContextAndModule,
                             public testing::Test {
 protected:
  void OnVariableBuilt(ast::Variable* var) override {
    td.RegisterVariableForTesting(var);
  }

  TypeDeterminer td{ctx, mod};
  spirv::Builder b{ctx, mod};
};

template <typename T>
class IntrinsicBuilderTestWithParam : public IntrinsicBuilderTest,
                                      public testing::WithParamInterface<T> {};

struct IntrinsicData {
  std::string name;
  std::string op;
};
inline std::ostream& operator<<(std::ostream& out, IntrinsicData data) {
  out << data.name;
  return out;
}

using IntrinsicBoolTest = IntrinsicBuilderTestWithParam<IntrinsicData>;
TEST_P(IntrinsicBoolTest, Call_Bool) {
  auto param = GetParam();

  auto* var = Var("v", ast::StorageClass::kPrivate, ty.vec3<bool>());
  auto expr = Call(param.name, "v");

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(&expr), 6u) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeBool
%3 = OpTypeVector %4 3
%2 = OpTypePointer Private %3
%5 = OpConstantNull %3
%1 = OpVariable %2 Private %5
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%7 = OpLoad %3 %1
%6 = )" + param.op +
                " %4 %7\n");
}
INSTANTIATE_TEST_SUITE_P(IntrinsicBuilderTest,
                         IntrinsicBoolTest,
                         testing::Values(IntrinsicData{"any", "OpAny"},
                                         IntrinsicData{"all", "OpAll"}));

using IntrinsicFloatTest = IntrinsicBuilderTestWithParam<IntrinsicData>;
TEST_P(IntrinsicFloatTest, Call_Float_Scalar) {
  auto param = GetParam();

  auto* var = Var("v", ast::StorageClass::kPrivate, ty.f32);
  auto expr = Call(param.name, "v");

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(&expr), 5u) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeFloat 32
%2 = OpTypePointer Private %3
%4 = OpConstantNull %3
%1 = OpVariable %2 Private %4
%6 = OpTypeBool
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%7 = OpLoad %3 %1
%5 = )" + param.op +
                " %6 %7\n");
}

TEST_P(IntrinsicFloatTest, Call_Float_Vector) {
  auto param = GetParam();

  auto* var = Var("v", ast::StorageClass::kPrivate, ty.vec3<f32>());
  auto expr = Call(param.name, "v");

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(&expr), 6u) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeFloat 32
%3 = OpTypeVector %4 3
%2 = OpTypePointer Private %3
%5 = OpConstantNull %3
%1 = OpVariable %2 Private %5
%8 = OpTypeBool
%7 = OpTypeVector %8 3
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%9 = OpLoad %3 %1
%6 = )" + param.op +
                " %7 %9\n");
}
INSTANTIATE_TEST_SUITE_P(IntrinsicBuilderTest,
                         IntrinsicFloatTest,
                         testing::Values(IntrinsicData{"isNan", "OpIsNan"},
                                         IntrinsicData{"isInf", "OpIsInf"}));

using IntrinsicIntTest = IntrinsicBuilderTestWithParam<IntrinsicData>;
TEST_P(IntrinsicIntTest, Call_SInt_Scalar) {
  auto param = GetParam();

  auto* var = Var("v", ast::StorageClass::kPrivate, ty.i32);
  auto expr = Call(param.name, "v");

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(&expr), 5u) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeInt 32 1
%2 = OpTypePointer Private %3
%4 = OpConstantNull %3
%1 = OpVariable %2 Private %4
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%6 = OpLoad %3 %1
%5 = )" + param.op +
                " %3 %6\n");
}

TEST_P(IntrinsicIntTest, Call_SInt_Vector) {
  auto param = GetParam();

  auto* var = Var("v", ast::StorageClass::kPrivate, ty.vec3<i32>());
  auto expr = Call(param.name, "v");

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(&expr), 6u) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeInt 32 1
%3 = OpTypeVector %4 3
%2 = OpTypePointer Private %3
%5 = OpConstantNull %3
%1 = OpVariable %2 Private %5
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%7 = OpLoad %3 %1
%6 = )" + param.op +
                " %3 %7\n");
}

TEST_P(IntrinsicIntTest, Call_UInt_Scalar) {
  auto param = GetParam();

  auto* var = Var("v", ast::StorageClass::kPrivate, ty.u32);
  auto expr = Call(param.name, "v");

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(&expr), 5u) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeInt 32 0
%2 = OpTypePointer Private %3
%4 = OpConstantNull %3
%1 = OpVariable %2 Private %4
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%6 = OpLoad %3 %1
%5 = )" + param.op +
                " %3 %6\n");
}

TEST_P(IntrinsicIntTest, Call_UInt_Vector) {
  auto param = GetParam();

  auto* var = Var("v", ast::StorageClass::kPrivate, ty.vec3<u32>());
  auto expr = Call(param.name, "v");

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(&expr), 6u) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeInt 32 0
%3 = OpTypeVector %4 3
%2 = OpTypePointer Private %3
%5 = OpConstantNull %3
%1 = OpVariable %2 Private %5
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%7 = OpLoad %3 %1
%6 = )" + param.op +
                " %3 %7\n");
}
INSTANTIATE_TEST_SUITE_P(
    IntrinsicBuilderTest,
    IntrinsicIntTest,
    testing::Values(IntrinsicData{"countOneBits", "OpBitCount"},
                    IntrinsicData{"reverseBits", "OpBitReverse"}));

TEST_F(IntrinsicBuilderTest, Call_Dot) {
  auto* var = Var("v", ast::StorageClass::kPrivate, ty.vec3<f32>());
  auto expr = Call("dot", "v", "v");

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(&expr), 6u) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeFloat 32
%3 = OpTypeVector %4 3
%2 = OpTypePointer Private %3
%5 = OpConstantNull %3
%1 = OpVariable %2 Private %5
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%7 = OpLoad %3 %1
%8 = OpLoad %3 %1
%6 = OpDot %4 %7 %8
)");
}

using IntrinsicDeriveTest = IntrinsicBuilderTestWithParam<IntrinsicData>;
TEST_P(IntrinsicDeriveTest, Call_Derivative_Scalar) {
  auto param = GetParam();

  auto* var = Var("v", ast::StorageClass::kPrivate, ty.f32);
  auto expr = Call(param.name, "v");

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(&expr), 5u) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeFloat 32
%2 = OpTypePointer Private %3
%4 = OpConstantNull %3
%1 = OpVariable %2 Private %4
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%6 = OpLoad %3 %1
%5 = )" + param.op +
                " %3 %6\n");
}

TEST_P(IntrinsicDeriveTest, Call_Derivative_Vector) {
  auto param = GetParam();

  auto* var = Var("v", ast::StorageClass::kPrivate, ty.vec3<f32>());
  auto expr = Call(param.name, "v");

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(&expr), 6u) << b.error();

  if (param.name != "dpdx" && param.name != "dpdy" && param.name != "fwidth") {
    EXPECT_EQ(DumpInstructions(b.capabilities()),
              R"(OpCapability DerivativeControl
)");
  }

  EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeFloat 32
%3 = OpTypeVector %4 3
%2 = OpTypePointer Private %3
%5 = OpConstantNull %3
%1 = OpVariable %2 Private %5
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%7 = OpLoad %3 %1
%6 = )" + param.op +
                " %3 %7\n");
}
INSTANTIATE_TEST_SUITE_P(
    IntrinsicBuilderTest,
    IntrinsicDeriveTest,
    testing::Values(IntrinsicData{"dpdx", "OpDPdx"},
                    IntrinsicData{"dpdxFine", "OpDPdxFine"},
                    IntrinsicData{"dpdxCoarse", "OpDPdxCoarse"},
                    IntrinsicData{"dpdy", "OpDPdy"},
                    IntrinsicData{"dpdyFine", "OpDPdyFine"},
                    IntrinsicData{"dpdyCoarse", "OpDPdyCoarse"},
                    IntrinsicData{"fwidth", "OpFwidth"},
                    IntrinsicData{"fwidthFine", "OpFwidthFine"},
                    IntrinsicData{"fwidthCoarse", "OpFwidthCoarse"}));

TEST_F(IntrinsicBuilderTest, Call_OuterProduct) {
  auto* v2 = Var("v2", ast::StorageClass::kPrivate, ty.vec2<f32>());
  auto* v3 = Var("v3", ast::StorageClass::kPrivate, ty.vec3<f32>());

  auto expr = Call("outerProduct", "v2", "v3");

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateGlobalVariable(v2)) << b.error();
  ASSERT_TRUE(b.GenerateGlobalVariable(v3)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(&expr), 10u) << b.error();

  EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeFloat 32
%3 = OpTypeVector %4 2
%2 = OpTypePointer Private %3
%5 = OpConstantNull %3
%1 = OpVariable %2 Private %5
%8 = OpTypeVector %4 3
%7 = OpTypePointer Private %8
%9 = OpConstantNull %8
%6 = OpVariable %7 Private %9
%11 = OpTypeMatrix %3 3
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%12 = OpLoad %3 %1
%13 = OpLoad %8 %6
%10 = OpOuterProduct %11 %12 %13
)");
}

TEST_F(IntrinsicBuilderTest, Call_Select) {
  auto* v3 = Var("v3", ast::StorageClass::kPrivate, ty.vec3<f32>());
  auto* bool_v3 = Var("bool_v3", ast::StorageClass::kPrivate, ty.vec3<bool>());
  auto expr = Call("select", "v3", "v3", "bool_v3");

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateGlobalVariable(v3)) << b.error();
  ASSERT_TRUE(b.GenerateGlobalVariable(bool_v3)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(&expr), 11u) << b.error();

  EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeFloat 32
%3 = OpTypeVector %4 3
%2 = OpTypePointer Private %3
%5 = OpConstantNull %3
%1 = OpVariable %2 Private %5
%9 = OpTypeBool
%8 = OpTypeVector %9 3
%7 = OpTypePointer Private %8
%10 = OpConstantNull %8
%6 = OpVariable %7 Private %10
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%12 = OpLoad %3 %1
%13 = OpLoad %3 %1
%14 = OpLoad %8 %6
%11 = OpSelect %3 %12 %13 %14
)");
}

TEST_F(IntrinsicBuilderTest, Call_TextureLoad_Storage_RO_1d) {
  ast::type::StorageTextureType s(ast::type::TextureDimension::k1d,
                                  ast::AccessControl::kReadOnly,
                                  ast::type::ImageFormat::kR16Float);

  ASSERT_TRUE(td.DetermineStorageTextureSubtype(&s)) << td.error();

  b.push_function(Function{});

  auto* tex = Var("texture", ast::StorageClass::kNone, &s);
  ASSERT_TRUE(b.GenerateGlobalVariable(tex)) << b.error();

  auto expr = Call("textureLoad", "texture", 1.0f, 2);

  EXPECT_TRUE(td.DetermineResultType(&expr)) << td.error();
  EXPECT_EQ(b.GenerateExpression(&expr), 5u) << b.error();

  EXPECT_EQ(DumpInstructions(b.types()),
            R"(%4 = OpTypeFloat 32
%3 = OpTypeImage %4 1D 0 0 0 2 R16f
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%6 = OpTypeVector %4 4
%8 = OpConstant %4 1
%9 = OpTypeInt 32 1
%10 = OpConstant %9 2
)");

  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%7 = OpLoad %3 %1
%5 = OpImageRead %6 %7 %8 Lod %10
)");
}

TEST_F(IntrinsicBuilderTest, Call_TextureLoad_Storage_RO_2d) {
  ast::type::StorageTextureType s(ast::type::TextureDimension::k2d,
                                  ast::AccessControl::kReadOnly,
                                  ast::type::ImageFormat::kR16Float);

  ASSERT_TRUE(td.DetermineStorageTextureSubtype(&s)) << td.error();

  b.push_function(Function{});

  auto* tex = Var("texture", ast::StorageClass::kNone, &s);
  ASSERT_TRUE(b.GenerateGlobalVariable(tex)) << b.error();

  auto expr = Call("textureLoad", "texture", vec2<f32>(1.0f, 2.0f), 2);

  EXPECT_TRUE(td.DetermineResultType(&expr)) << td.error();
  EXPECT_EQ(b.GenerateExpression(&expr), 5u) << b.error();

  EXPECT_EQ(DumpInstructions(b.types()),
            R"(%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 0 0 0 2 R16f
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%6 = OpTypeVector %4 4
%8 = OpTypeVector %4 2
%9 = OpConstant %4 1
%10 = OpConstant %4 2
%11 = OpConstantComposite %8 %9 %10
%12 = OpTypeInt 32 1
%13 = OpConstant %12 2
)");

  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%7 = OpLoad %3 %1
%5 = OpImageRead %6 %7 %11 Lod %13
)");
}

TEST_F(IntrinsicBuilderTest, Call_TextureLoad_Sampled_1d) {
  ast::type::SampledTextureType s(ast::type::TextureDimension::k1d, ty.f32);

  b.push_function(Function{});

  auto* tex = Var("texture", ast::StorageClass::kNone, &s);
  ASSERT_TRUE(b.GenerateGlobalVariable(tex)) << b.error();

  auto expr = Call("textureLoad", "texture", 1.0f, 2);

  EXPECT_TRUE(td.DetermineResultType(&expr)) << td.error();
  EXPECT_EQ(b.GenerateExpression(&expr), 5u) << b.error();

  EXPECT_EQ(DumpInstructions(b.types()),
            R"(%4 = OpTypeFloat 32
%3 = OpTypeImage %4 1D 0 0 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%6 = OpTypeVector %4 4
%8 = OpConstant %4 1
%9 = OpTypeInt 32 1
%10 = OpConstant %9 2
)");

  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%7 = OpLoad %3 %1
%5 = OpImageFetch %6 %7 %8 Lod %10
)");
}

TEST_F(IntrinsicBuilderTest, Call_TextureLoad_Sampled_2d) {
  ast::type::SampledTextureType s(ast::type::TextureDimension::k2d, ty.f32);

  b.push_function(Function{});

  auto* tex = Var("texture", ast::StorageClass::kNone, &s);
  ASSERT_TRUE(b.GenerateGlobalVariable(tex)) << b.error();

  auto expr = Call("textureLoad", "texture", vec2<f32>(1.0f, 2.0f), 2);

  EXPECT_TRUE(td.DetermineResultType(&expr)) << td.error();
  EXPECT_EQ(b.GenerateExpression(&expr), 5u) << b.error();

  EXPECT_EQ(DumpInstructions(b.types()),
            R"(%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 0 0 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%6 = OpTypeVector %4 4
%8 = OpTypeVector %4 2
%9 = OpConstant %4 1
%10 = OpConstant %4 2
%11 = OpConstantComposite %8 %9 %10
%12 = OpTypeInt 32 1
%13 = OpConstant %12 2
)");

  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%7 = OpLoad %3 %1
%5 = OpImageFetch %6 %7 %11 Lod %13
)");
}

TEST_F(IntrinsicBuilderTest, Call_TextureLoad_Multisampled_2d) {
  ast::type::MultisampledTextureType s(ast::type::TextureDimension::k2d,
                                       ty.f32);

  b.push_function(Function{});

  auto* tex = Var("texture", ast::StorageClass::kNone, &s);
  ASSERT_TRUE(b.GenerateGlobalVariable(tex)) << b.error();

  auto expr = Call("textureLoad", "texture", vec2<f32>(1.0f, 2.0f), 2);

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();
  EXPECT_EQ(b.GenerateExpression(&expr), 5u) << b.error();

  EXPECT_EQ(DumpInstructions(b.types()),
            R"(%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 0 0 1 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%6 = OpTypeVector %4 4
%8 = OpTypeVector %4 2
%9 = OpConstant %4 1
%10 = OpConstant %4 2
%11 = OpConstantComposite %8 %9 %10
%12 = OpTypeInt 32 1
%13 = OpConstant %12 2
)");

  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%7 = OpLoad %3 %1
%5 = OpImageFetch %6 %7 %11 Sample %13
)");
}

using IntrinsicTextureTest =
    IntrinsicBuilderTestWithParam<ast::intrinsic::test::TextureOverloadCase>;

INSTANTIATE_TEST_SUITE_P(
    IntrinsicBuilderTest,
    IntrinsicTextureTest,
    testing::ValuesIn(ast::intrinsic::test::TextureOverloadCase::ValidCases()));

struct expected_texture_overload_spirv {
  std::string types;
  std::string instructions;
};

expected_texture_overload_spirv expected_texture_overload(
    ast::intrinsic::test::ValidTextureOverload overload) {
  using ValidTextureOverload = ast::intrinsic::test::ValidTextureOverload;
  switch (overload) {
    case ValidTextureOverload::kSample1dF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 1D 0 0 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
%9 = OpTypeVector %4 4
%12 = OpTypeSampledImage %3
%14 = OpConstant %4 1
)",
          R"(
%10 = OpLoad %7 %5
%11 = OpLoad %3 %1
%13 = OpSampledImage %12 %11 %10
%8 = OpImageSampleImplicitLod %9 %13 %14
)"};
    case ValidTextureOverload::kSample1dArrayF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 1D 0 1 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
%9 = OpTypeVector %4 4
%12 = OpTypeSampledImage %3
%14 = OpTypeVector %4 2
%15 = OpConstant %4 1
%17 = OpTypeInt 32 0
%18 = OpConstant %17 2
)",
          R"(
%10 = OpLoad %7 %5
%11 = OpLoad %3 %1
%13 = OpSampledImage %12 %11 %10
%16 = OpConvertUToF %4 %18
%19 = OpCompositeConstruct %14 %15 %16
%8 = OpImageSampleImplicitLod %9 %13 %19
)"};
    case ValidTextureOverload::kSample2dF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 0 0 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
%9 = OpTypeVector %4 4
%12 = OpTypeSampledImage %3
%14 = OpTypeVector %4 2
%15 = OpConstant %4 1
%16 = OpConstant %4 2
%17 = OpConstantComposite %14 %15 %16
)",
          R"(
%10 = OpLoad %7 %5
%11 = OpLoad %3 %1
%13 = OpSampledImage %12 %11 %10
%8 = OpImageSampleImplicitLod %9 %13 %17
)"};
    case ValidTextureOverload::kSample2dOffsetF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 0 0 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
%9 = OpTypeVector %4 4
%12 = OpTypeSampledImage %3
%14 = OpTypeVector %4 2
%15 = OpConstant %4 1
%16 = OpConstant %4 2
%17 = OpConstantComposite %14 %15 %16
%19 = OpTypeInt 32 1
%18 = OpTypeVector %19 2
%20 = OpConstant %19 3
%21 = OpConstant %19 4
%22 = OpConstantComposite %18 %20 %21
)",
          R"(
%10 = OpLoad %7 %5
%11 = OpLoad %3 %1
%13 = OpSampledImage %12 %11 %10
%8 = OpImageSampleImplicitLod %9 %13 %17 Offset %22
)"};
    case ValidTextureOverload::kSample2dArrayF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 0 1 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
%9 = OpTypeVector %4 4
%12 = OpTypeSampledImage %3
%14 = OpTypeVector %4 3
%15 = OpTypeVector %4 2
%16 = OpConstant %4 1
%17 = OpConstant %4 2
%18 = OpConstantComposite %15 %16 %17
%22 = OpTypeInt 32 0
%23 = OpConstant %22 3
)",
          R"(
%10 = OpLoad %7 %5
%11 = OpLoad %3 %1
%13 = OpSampledImage %12 %11 %10
%19 = OpCompositeExtract %4 %18 0
%20 = OpCompositeExtract %4 %18 1
%21 = OpConvertUToF %4 %23
%24 = OpCompositeConstruct %14 %19 %20 %21
%8 = OpImageSampleImplicitLod %9 %13 %24
)"};
    case ValidTextureOverload::kSample2dArrayOffsetF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 0 1 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
%9 = OpTypeVector %4 4
%12 = OpTypeSampledImage %3
%14 = OpTypeVector %4 3
%15 = OpTypeVector %4 2
%16 = OpConstant %4 1
%17 = OpConstant %4 2
%18 = OpConstantComposite %15 %16 %17
%22 = OpTypeInt 32 0
%23 = OpConstant %22 3
%26 = OpTypeInt 32 1
%25 = OpTypeVector %26 2
%27 = OpConstant %26 4
%28 = OpConstant %26 5
%29 = OpConstantComposite %25 %27 %28
)",
          R"(
%10 = OpLoad %7 %5
%11 = OpLoad %3 %1
%13 = OpSampledImage %12 %11 %10
%19 = OpCompositeExtract %4 %18 0
%20 = OpCompositeExtract %4 %18 1
%21 = OpConvertUToF %4 %23
%24 = OpCompositeConstruct %14 %19 %20 %21
%8 = OpImageSampleImplicitLod %9 %13 %24 Offset %29
)"};
    case ValidTextureOverload::kSample3dF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 3D 0 0 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
%9 = OpTypeVector %4 4
%12 = OpTypeSampledImage %3
%14 = OpTypeVector %4 3
%15 = OpConstant %4 1
%16 = OpConstant %4 2
%17 = OpConstant %4 3
%18 = OpConstantComposite %14 %15 %16 %17
)",
          R"(
%10 = OpLoad %7 %5
%11 = OpLoad %3 %1
%13 = OpSampledImage %12 %11 %10
%8 = OpImageSampleImplicitLod %9 %13 %18
)"};
    case ValidTextureOverload::kSample3dOffsetF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 3D 0 0 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
%9 = OpTypeVector %4 4
%12 = OpTypeSampledImage %3
%14 = OpTypeVector %4 3
%15 = OpConstant %4 1
%16 = OpConstant %4 2
%17 = OpConstant %4 3
%18 = OpConstantComposite %14 %15 %16 %17
%20 = OpTypeInt 32 1
%19 = OpTypeVector %20 3
%21 = OpConstant %20 4
%22 = OpConstant %20 5
%23 = OpConstant %20 6
%24 = OpConstantComposite %19 %21 %22 %23
)",
          R"(
%10 = OpLoad %7 %5
%11 = OpLoad %3 %1
%13 = OpSampledImage %12 %11 %10
%8 = OpImageSampleImplicitLod %9 %13 %18 Offset %24
)"};
    case ValidTextureOverload::kSampleCubeF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 Cube 0 0 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
%9 = OpTypeVector %4 4
%12 = OpTypeSampledImage %3
%14 = OpTypeVector %4 3
%15 = OpConstant %4 1
%16 = OpConstant %4 2
%17 = OpConstant %4 3
%18 = OpConstantComposite %14 %15 %16 %17
)",
          R"(
%10 = OpLoad %7 %5
%11 = OpLoad %3 %1
%13 = OpSampledImage %12 %11 %10
%8 = OpImageSampleImplicitLod %9 %13 %18
)"};
    case ValidTextureOverload::kSampleCubeArrayF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 Cube 0 1 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
%9 = OpTypeVector %4 4
%12 = OpTypeSampledImage %3
%14 = OpTypeVector %4 3
%15 = OpConstant %4 1
%16 = OpConstant %4 2
%17 = OpConstant %4 3
%18 = OpConstantComposite %14 %15 %16 %17
%23 = OpTypeInt 32 0
%24 = OpConstant %23 4
)",
          R"(
%10 = OpLoad %7 %5
%11 = OpLoad %3 %1
%13 = OpSampledImage %12 %11 %10
%19 = OpCompositeExtract %4 %18 0
%20 = OpCompositeExtract %4 %18 1
%21 = OpCompositeExtract %4 %18 2
%22 = OpConvertUToF %4 %24
%25 = OpCompositeConstruct %9 %19 %20 %21 %22
%8 = OpImageSampleImplicitLod %9 %13 %25
)"};
    case ValidTextureOverload::kSampleDepth2dF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 1 0 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
%11 = OpTypeSampledImage %3
%13 = OpTypeVector %4 2
%14 = OpConstant %4 1
%15 = OpConstant %4 2
%16 = OpConstantComposite %13 %14 %15
)",
          R"(
%9 = OpLoad %7 %5
%10 = OpLoad %3 %1
%12 = OpSampledImage %11 %10 %9
%8 = OpImageSampleImplicitLod %4 %12 %16
)"};
    case ValidTextureOverload::kSampleDepth2dOffsetF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 1 0 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
%11 = OpTypeSampledImage %3
%13 = OpTypeVector %4 2
%14 = OpConstant %4 1
%15 = OpConstant %4 2
%16 = OpConstantComposite %13 %14 %15
%18 = OpTypeInt 32 1
%17 = OpTypeVector %18 2
%19 = OpConstant %18 3
%20 = OpConstant %18 4
%21 = OpConstantComposite %17 %19 %20
)",
          R"(
%9 = OpLoad %7 %5
%10 = OpLoad %3 %1
%12 = OpSampledImage %11 %10 %9
%8 = OpImageSampleImplicitLod %4 %12 %16 Offset %21
)"};
    case ValidTextureOverload::kSampleDepth2dArrayF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 1 1 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
%11 = OpTypeSampledImage %3
%13 = OpTypeVector %4 3
%14 = OpTypeVector %4 2
%15 = OpConstant %4 1
%16 = OpConstant %4 2
%17 = OpConstantComposite %14 %15 %16
%21 = OpTypeInt 32 0
%22 = OpConstant %21 3
)",
          R"(
%9 = OpLoad %7 %5
%10 = OpLoad %3 %1
%12 = OpSampledImage %11 %10 %9
%18 = OpCompositeExtract %4 %17 0
%19 = OpCompositeExtract %4 %17 1
%20 = OpConvertUToF %4 %22
%23 = OpCompositeConstruct %13 %18 %19 %20
%8 = OpImageSampleImplicitLod %4 %12 %23
)"};
    case ValidTextureOverload::kSampleDepth2dArrayOffsetF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 1 1 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
%11 = OpTypeSampledImage %3
%13 = OpTypeVector %4 3
%14 = OpTypeVector %4 2
%15 = OpConstant %4 1
%16 = OpConstant %4 2
%17 = OpConstantComposite %14 %15 %16
%21 = OpTypeInt 32 0
%22 = OpConstant %21 3
%25 = OpTypeInt 32 1
%24 = OpTypeVector %25 2
%26 = OpConstant %25 4
%27 = OpConstant %25 5
%28 = OpConstantComposite %24 %26 %27
)",
          R"(
%9 = OpLoad %7 %5
%10 = OpLoad %3 %1
%12 = OpSampledImage %11 %10 %9
%18 = OpCompositeExtract %4 %17 0
%19 = OpCompositeExtract %4 %17 1
%20 = OpConvertUToF %4 %22
%23 = OpCompositeConstruct %13 %18 %19 %20
%8 = OpImageSampleImplicitLod %4 %12 %23 Offset %28
)"};
    case ValidTextureOverload::kSampleDepthCubeF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 Cube 1 0 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
%11 = OpTypeSampledImage %3
%13 = OpTypeVector %4 2
%14 = OpConstant %4 1
%15 = OpConstant %4 2
%16 = OpConstant %4 3
%17 = OpConstantComposite %13 %14 %15 %16
)",
          R"(
%9 = OpLoad %7 %5
%10 = OpLoad %3 %1
%12 = OpSampledImage %11 %10 %9
%8 = OpImageSampleImplicitLod %4 %12 %17
)"};
    case ValidTextureOverload::kSampleDepthCubeArrayF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 Cube 1 1 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
%11 = OpTypeSampledImage %3
%13 = OpTypeVector %4 3
%14 = OpTypeVector %4 2
%15 = OpConstant %4 1
%16 = OpConstant %4 2
%17 = OpConstant %4 3
%18 = OpConstantComposite %14 %15 %16 %17
%22 = OpTypeInt 32 0
%23 = OpConstant %22 4
)",
          R"(
%9 = OpLoad %7 %5
%10 = OpLoad %3 %1
%12 = OpSampledImage %11 %10 %9
%19 = OpCompositeExtract %4 %18 0
%20 = OpCompositeExtract %4 %18 1
%21 = OpConvertUToF %4 %23
%24 = OpCompositeConstruct %13 %19 %20 %21
%8 = OpImageSampleImplicitLod %4 %12 %24
)"};
    case ValidTextureOverload::kSampleBias2dF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 0 0 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
%9 = OpTypeVector %4 4
%12 = OpTypeSampledImage %3
%14 = OpTypeVector %4 2
%15 = OpConstant %4 1
%16 = OpConstant %4 2
%17 = OpConstantComposite %14 %15 %16
%18 = OpConstant %4 3
)",
          R"(
%10 = OpLoad %7 %5
%11 = OpLoad %3 %1
%13 = OpSampledImage %12 %11 %10
%8 = OpImageSampleImplicitLod %9 %13 %17 Bias %18
)"};
    case ValidTextureOverload::kSampleBias2dOffsetF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 0 0 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
%9 = OpTypeVector %4 4
%12 = OpTypeSampledImage %3
%14 = OpTypeVector %4 2
%15 = OpConstant %4 1
%16 = OpConstant %4 2
%17 = OpConstantComposite %14 %15 %16
%18 = OpConstant %4 3
%20 = OpTypeInt 32 1
%19 = OpTypeVector %20 2
%21 = OpConstant %20 4
%22 = OpConstant %20 5
%23 = OpConstantComposite %19 %21 %22
)",
          R"(
%10 = OpLoad %7 %5
%11 = OpLoad %3 %1
%13 = OpSampledImage %12 %11 %10
%8 = OpImageSampleImplicitLod %9 %13 %17 Bias|Offset %18 %23
)"};
    case ValidTextureOverload::kSampleBias2dArrayF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 0 1 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
%9 = OpTypeVector %4 4
%12 = OpTypeSampledImage %3
%14 = OpTypeVector %4 3
%15 = OpTypeVector %4 2
%16 = OpConstant %4 1
%17 = OpConstant %4 2
%18 = OpConstantComposite %15 %16 %17
%22 = OpTypeInt 32 0
%23 = OpConstant %22 4
%25 = OpConstant %4 3
)",
          R"(
%10 = OpLoad %7 %5
%11 = OpLoad %3 %1
%13 = OpSampledImage %12 %11 %10
%19 = OpCompositeExtract %4 %18 0
%20 = OpCompositeExtract %4 %18 1
%21 = OpConvertUToF %4 %23
%24 = OpCompositeConstruct %14 %19 %20 %21
%8 = OpImageSampleImplicitLod %9 %13 %24 Bias %25
)"};
    case ValidTextureOverload::kSampleBias2dArrayOffsetF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 0 1 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
%9 = OpTypeVector %4 4
%12 = OpTypeSampledImage %3
%14 = OpTypeVector %4 3
%15 = OpTypeVector %4 2
%16 = OpConstant %4 1
%17 = OpConstant %4 2
%18 = OpConstantComposite %15 %16 %17
%22 = OpTypeInt 32 0
%23 = OpConstant %22 3
%25 = OpConstant %4 4
%27 = OpTypeInt 32 1
%26 = OpTypeVector %27 2
%28 = OpConstant %27 5
%29 = OpConstant %27 6
%30 = OpConstantComposite %26 %28 %29
)",
          R"(
%10 = OpLoad %7 %5
%11 = OpLoad %3 %1
%13 = OpSampledImage %12 %11 %10
%19 = OpCompositeExtract %4 %18 0
%20 = OpCompositeExtract %4 %18 1
%21 = OpConvertUToF %4 %23
%24 = OpCompositeConstruct %14 %19 %20 %21
%8 = OpImageSampleImplicitLod %9 %13 %24 Bias|Offset %25 %30
)"};
    case ValidTextureOverload::kSampleBias3dF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 3D 0 0 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
%9 = OpTypeVector %4 4
%12 = OpTypeSampledImage %3
%14 = OpTypeVector %4 3
%15 = OpConstant %4 1
%16 = OpConstant %4 2
%17 = OpConstant %4 3
%18 = OpConstantComposite %14 %15 %16 %17
%19 = OpConstant %4 4
)",
          R"(
%10 = OpLoad %7 %5
%11 = OpLoad %3 %1
%13 = OpSampledImage %12 %11 %10
%8 = OpImageSampleImplicitLod %9 %13 %18 Bias %19
)"};
    case ValidTextureOverload::kSampleBias3dOffsetF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 3D 0 0 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
%9 = OpTypeVector %4 4
%12 = OpTypeSampledImage %3
%14 = OpTypeVector %4 3
%15 = OpConstant %4 1
%16 = OpConstant %4 2
%17 = OpConstant %4 3
%18 = OpConstantComposite %14 %15 %16 %17
%19 = OpConstant %4 4
%21 = OpTypeInt 32 1
%20 = OpTypeVector %21 3
%22 = OpConstant %21 5
%23 = OpConstant %21 6
%24 = OpConstant %21 7
%25 = OpConstantComposite %20 %22 %23 %24
)",
          R"(
%10 = OpLoad %7 %5
%11 = OpLoad %3 %1
%13 = OpSampledImage %12 %11 %10
%8 = OpImageSampleImplicitLod %9 %13 %18 Bias|Offset %19 %25
)"};
    case ValidTextureOverload::kSampleBiasCubeF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 Cube 0 0 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
%9 = OpTypeVector %4 4
%12 = OpTypeSampledImage %3
%14 = OpTypeVector %4 3
%15 = OpConstant %4 1
%16 = OpConstant %4 2
%17 = OpConstant %4 3
%18 = OpConstantComposite %14 %15 %16 %17
%19 = OpConstant %4 4
)",
          R"(
%10 = OpLoad %7 %5
%11 = OpLoad %3 %1
%13 = OpSampledImage %12 %11 %10
%8 = OpImageSampleImplicitLod %9 %13 %18 Bias %19
)"};
    case ValidTextureOverload::kSampleBiasCubeArrayF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 Cube 0 1 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
%9 = OpTypeVector %4 4
%12 = OpTypeSampledImage %3
%14 = OpTypeVector %4 3
%15 = OpConstant %4 1
%16 = OpConstant %4 2
%17 = OpConstant %4 3
%18 = OpConstantComposite %14 %15 %16 %17
%23 = OpTypeInt 32 0
%24 = OpConstant %23 3
%26 = OpConstant %4 4
)",
          R"(
%10 = OpLoad %7 %5
%11 = OpLoad %3 %1
%13 = OpSampledImage %12 %11 %10
%19 = OpCompositeExtract %4 %18 0
%20 = OpCompositeExtract %4 %18 1
%21 = OpCompositeExtract %4 %18 2
%22 = OpConvertUToF %4 %24
%25 = OpCompositeConstruct %9 %19 %20 %21 %22
%8 = OpImageSampleImplicitLod %9 %13 %25 Bias %26
)"};
    case ValidTextureOverload::kSampleLevel2dF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 0 0 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
%9 = OpTypeVector %4 4
%12 = OpTypeSampledImage %3
%14 = OpTypeVector %4 2
%15 = OpConstant %4 1
%16 = OpConstant %4 2
%17 = OpConstantComposite %14 %15 %16
%18 = OpConstant %4 3
)",
          R"(
%10 = OpLoad %7 %5
%11 = OpLoad %3 %1
%13 = OpSampledImage %12 %11 %10
%8 = OpImageSampleExplicitLod %9 %13 %17 Lod %18
)"};
    case ValidTextureOverload::kSampleLevel2dOffsetF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 0 0 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
%9 = OpTypeVector %4 4
%12 = OpTypeSampledImage %3
%14 = OpTypeVector %4 2
%15 = OpConstant %4 1
%16 = OpConstant %4 2
%17 = OpConstantComposite %14 %15 %16
%18 = OpConstant %4 3
%20 = OpTypeInt 32 1
%19 = OpTypeVector %20 2
%21 = OpConstant %20 4
%22 = OpConstant %20 5
%23 = OpConstantComposite %19 %21 %22
)",
          R"(
%10 = OpLoad %7 %5
%11 = OpLoad %3 %1
%13 = OpSampledImage %12 %11 %10
%8 = OpImageSampleExplicitLod %9 %13 %17 Lod|Offset %18 %23
)"};
    case ValidTextureOverload::kSampleLevel2dArrayF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 0 1 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
%9 = OpTypeVector %4 4
%12 = OpTypeSampledImage %3
%14 = OpTypeVector %4 3
%15 = OpTypeVector %4 2
%16 = OpConstant %4 1
%17 = OpConstant %4 2
%18 = OpConstantComposite %15 %16 %17
%22 = OpTypeInt 32 0
%23 = OpConstant %22 3
%25 = OpConstant %4 4
)",
          R"(
%10 = OpLoad %7 %5
%11 = OpLoad %3 %1
%13 = OpSampledImage %12 %11 %10
%19 = OpCompositeExtract %4 %18 0
%20 = OpCompositeExtract %4 %18 1
%21 = OpConvertUToF %4 %23
%24 = OpCompositeConstruct %14 %19 %20 %21
%8 = OpImageSampleExplicitLod %9 %13 %24 Lod %25
)"};
    case ValidTextureOverload::kSampleLevel2dArrayOffsetF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 0 1 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
%9 = OpTypeVector %4 4
%12 = OpTypeSampledImage %3
%14 = OpTypeVector %4 3
%15 = OpTypeVector %4 2
%16 = OpConstant %4 1
%17 = OpConstant %4 2
%18 = OpConstantComposite %15 %16 %17
%22 = OpTypeInt 32 0
%23 = OpConstant %22 3
%25 = OpConstant %4 4
%27 = OpTypeInt 32 1
%26 = OpTypeVector %27 2
%28 = OpConstant %27 5
%29 = OpConstant %27 6
%30 = OpConstantComposite %26 %28 %29
)",
          R"(
%10 = OpLoad %7 %5
%11 = OpLoad %3 %1
%13 = OpSampledImage %12 %11 %10
%19 = OpCompositeExtract %4 %18 0
%20 = OpCompositeExtract %4 %18 1
%21 = OpConvertUToF %4 %23
%24 = OpCompositeConstruct %14 %19 %20 %21
%8 = OpImageSampleExplicitLod %9 %13 %24 Lod|Offset %25 %30
)"};
    case ValidTextureOverload::kSampleLevel3dF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 3D 0 0 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
%9 = OpTypeVector %4 4
%12 = OpTypeSampledImage %3
%14 = OpTypeVector %4 3
%15 = OpConstant %4 1
%16 = OpConstant %4 2
%17 = OpConstant %4 3
%18 = OpConstantComposite %14 %15 %16 %17
%19 = OpConstant %4 4
)",
          R"(
%10 = OpLoad %7 %5
%11 = OpLoad %3 %1
%13 = OpSampledImage %12 %11 %10
%8 = OpImageSampleExplicitLod %9 %13 %18 Lod %19
)"};
    case ValidTextureOverload::kSampleLevel3dOffsetF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 3D 0 0 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
%9 = OpTypeVector %4 4
%12 = OpTypeSampledImage %3
%14 = OpTypeVector %4 3
%15 = OpConstant %4 1
%16 = OpConstant %4 2
%17 = OpConstant %4 3
%18 = OpConstantComposite %14 %15 %16 %17
%19 = OpConstant %4 4
%21 = OpTypeInt 32 1
%20 = OpTypeVector %21 3
%22 = OpConstant %21 5
%23 = OpConstant %21 6
%24 = OpConstant %21 7
%25 = OpConstantComposite %20 %22 %23 %24
)",
          R"(
%10 = OpLoad %7 %5
%11 = OpLoad %3 %1
%13 = OpSampledImage %12 %11 %10
%8 = OpImageSampleExplicitLod %9 %13 %18 Lod|Offset %19 %25
)"};
    case ValidTextureOverload::kSampleLevelCubeF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 Cube 0 0 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
%9 = OpTypeVector %4 4
%12 = OpTypeSampledImage %3
%14 = OpTypeVector %4 3
%15 = OpConstant %4 1
%16 = OpConstant %4 2
%17 = OpConstant %4 3
%18 = OpConstantComposite %14 %15 %16 %17
%19 = OpConstant %4 4
)",
          R"(
%10 = OpLoad %7 %5
%11 = OpLoad %3 %1
%13 = OpSampledImage %12 %11 %10
%8 = OpImageSampleExplicitLod %9 %13 %18 Lod %19
)"};
    case ValidTextureOverload::kSampleLevelCubeArrayF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 Cube 0 1 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
%9 = OpTypeVector %4 4
%12 = OpTypeSampledImage %3
%14 = OpTypeVector %4 3
%15 = OpConstant %4 1
%16 = OpConstant %4 2
%17 = OpConstant %4 3
%18 = OpConstantComposite %14 %15 %16 %17
%23 = OpTypeInt 32 0
%24 = OpConstant %23 4
%26 = OpConstant %4 5
)",
          R"(
%10 = OpLoad %7 %5
%11 = OpLoad %3 %1
%13 = OpSampledImage %12 %11 %10
%19 = OpCompositeExtract %4 %18 0
%20 = OpCompositeExtract %4 %18 1
%21 = OpCompositeExtract %4 %18 2
%22 = OpConvertUToF %4 %24
%25 = OpCompositeConstruct %9 %19 %20 %21 %22
%8 = OpImageSampleExplicitLod %9 %13 %25 Lod %26
)"};
    case ValidTextureOverload::kSampleLevelDepth2dF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 1 0 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
%11 = OpTypeSampledImage %3
%13 = OpTypeVector %4 2
%14 = OpConstant %4 1
%15 = OpConstant %4 2
%16 = OpConstantComposite %13 %14 %15
%17 = OpTypeInt 32 0
%18 = OpConstant %17 3
)",
          R"(
%9 = OpLoad %7 %5
%10 = OpLoad %3 %1
%12 = OpSampledImage %11 %10 %9
%8 = OpImageSampleExplicitLod %4 %12 %16 Lod %18
)"};
    case ValidTextureOverload::kSampleLevelDepth2dOffsetF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 1 0 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
%11 = OpTypeSampledImage %3
%13 = OpTypeVector %4 2
%14 = OpConstant %4 1
%15 = OpConstant %4 2
%16 = OpConstantComposite %13 %14 %15
%17 = OpTypeInt 32 0
%18 = OpConstant %17 3
%20 = OpTypeInt 32 1
%19 = OpTypeVector %20 2
%21 = OpConstant %20 4
%22 = OpConstant %20 5
%23 = OpConstantComposite %19 %21 %22
)",
          R"(
%9 = OpLoad %7 %5
%10 = OpLoad %3 %1
%12 = OpSampledImage %11 %10 %9
%8 = OpImageSampleExplicitLod %4 %12 %16 Lod|Offset %18 %23
)"};
    case ValidTextureOverload::kSampleLevelDepth2dArrayF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 1 1 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
%11 = OpTypeSampledImage %3
%13 = OpTypeVector %4 3
%14 = OpTypeVector %4 2
%15 = OpConstant %4 1
%16 = OpConstant %4 2
%17 = OpConstantComposite %14 %15 %16
%21 = OpTypeInt 32 0
%22 = OpConstant %21 3
%24 = OpConstant %21 4
)",
          R"(
%9 = OpLoad %7 %5
%10 = OpLoad %3 %1
%12 = OpSampledImage %11 %10 %9
%18 = OpCompositeExtract %4 %17 0
%19 = OpCompositeExtract %4 %17 1
%20 = OpConvertUToF %4 %22
%23 = OpCompositeConstruct %13 %18 %19 %20
%8 = OpImageSampleExplicitLod %4 %12 %23 Lod %24
)"};
    case ValidTextureOverload::kSampleLevelDepth2dArrayOffsetF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 1 1 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
%11 = OpTypeSampledImage %3
%13 = OpTypeVector %4 3
%14 = OpTypeVector %4 2
%15 = OpConstant %4 1
%16 = OpConstant %4 2
%17 = OpConstantComposite %14 %15 %16
%21 = OpTypeInt 32 0
%22 = OpConstant %21 3
%24 = OpConstant %21 4
%26 = OpTypeInt 32 1
%25 = OpTypeVector %26 2
%27 = OpConstant %26 5
%28 = OpConstant %26 6
%29 = OpConstantComposite %25 %27 %28
)",
          R"(
%9 = OpLoad %7 %5
%10 = OpLoad %3 %1
%12 = OpSampledImage %11 %10 %9
%18 = OpCompositeExtract %4 %17 0
%19 = OpCompositeExtract %4 %17 1
%20 = OpConvertUToF %4 %22
%23 = OpCompositeConstruct %13 %18 %19 %20
%8 = OpImageSampleExplicitLod %4 %12 %23 Lod|Offset %24 %29
)"};
    case ValidTextureOverload::kSampleLevelDepthCubeF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 Cube 1 0 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
%11 = OpTypeSampledImage %3
%13 = OpTypeVector %4 3
%14 = OpConstant %4 1
%15 = OpConstant %4 2
%16 = OpConstant %4 3
%17 = OpConstantComposite %13 %14 %15 %16
%18 = OpTypeInt 32 0
%19 = OpConstant %18 4
)",
          R"(
%9 = OpLoad %7 %5
%10 = OpLoad %3 %1
%12 = OpSampledImage %11 %10 %9
%8 = OpImageSampleExplicitLod %4 %12 %17 Lod %19
)"};
    case ValidTextureOverload::kSampleLevelDepthCubeArrayF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 Cube 1 1 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
%11 = OpTypeSampledImage %3
%13 = OpTypeVector %4 4
%14 = OpTypeVector %4 3
%15 = OpConstant %4 1
%16 = OpConstant %4 2
%17 = OpConstant %4 3
%18 = OpConstantComposite %14 %15 %16 %17
%23 = OpTypeInt 32 0
%24 = OpConstant %23 4
%26 = OpConstant %23 5
)",
          R"(
%9 = OpLoad %7 %5
%10 = OpLoad %3 %1
%12 = OpSampledImage %11 %10 %9
%19 = OpCompositeExtract %4 %18 0
%20 = OpCompositeExtract %4 %18 1
%21 = OpCompositeExtract %4 %18 2
%22 = OpConvertUToF %4 %24
%25 = OpCompositeConstruct %13 %19 %20 %21 %22
%8 = OpImageSampleExplicitLod %4 %12 %25 Lod %26
)"};
    case ValidTextureOverload::kSampleGrad2dF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 0 0 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
%9 = OpTypeVector %4 4
%12 = OpTypeSampledImage %3
%14 = OpTypeVector %4 2
%15 = OpConstant %4 1
%16 = OpConstant %4 2
%17 = OpConstantComposite %14 %15 %16
%18 = OpConstant %4 3
%19 = OpConstant %4 4
%20 = OpConstantComposite %14 %18 %19
%21 = OpConstant %4 5
%22 = OpConstant %4 6
%23 = OpConstantComposite %14 %21 %22
)",
          R"(
%10 = OpLoad %7 %5
%11 = OpLoad %3 %1
%13 = OpSampledImage %12 %11 %10
%8 = OpImageSampleExplicitLod %9 %13 %17 Grad %20 %23
)"};
    case ValidTextureOverload::kSampleGrad2dOffsetF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 0 0 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
%9 = OpTypeVector %4 4
%12 = OpTypeSampledImage %3
%14 = OpTypeVector %4 2
%15 = OpConstant %4 1
%16 = OpConstant %4 2
%17 = OpConstantComposite %14 %15 %16
%18 = OpConstant %4 3
%19 = OpConstant %4 4
%20 = OpConstantComposite %14 %18 %19
%21 = OpConstant %4 5
%22 = OpConstant %4 6
%23 = OpConstantComposite %14 %21 %22
%25 = OpTypeInt 32 1
%24 = OpTypeVector %25 2
%26 = OpConstant %25 7
%27 = OpConstant %25 8
%28 = OpConstantComposite %24 %26 %27
)",
          R"(
%10 = OpLoad %7 %5
%11 = OpLoad %3 %1
%13 = OpSampledImage %12 %11 %10
%8 = OpImageSampleExplicitLod %9 %13 %17 Grad|Offset %20 %23 %28
)"};
    case ValidTextureOverload::kSampleGrad2dArrayF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 0 1 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
%9 = OpTypeVector %4 4
%12 = OpTypeSampledImage %3
%14 = OpTypeVector %4 3
%15 = OpTypeVector %4 2
%16 = OpConstant %4 1
%17 = OpConstant %4 2
%18 = OpConstantComposite %15 %16 %17
%22 = OpTypeInt 32 0
%23 = OpConstant %22 3
%25 = OpConstant %4 4
%26 = OpConstant %4 5
%27 = OpConstantComposite %15 %25 %26
%28 = OpConstant %4 6
%29 = OpConstant %4 7
%30 = OpConstantComposite %15 %28 %29
)",
          R"(
%10 = OpLoad %7 %5
%11 = OpLoad %3 %1
%13 = OpSampledImage %12 %11 %10
%19 = OpCompositeExtract %4 %18 0
%20 = OpCompositeExtract %4 %18 1
%21 = OpConvertUToF %4 %23
%24 = OpCompositeConstruct %14 %19 %20 %21
%8 = OpImageSampleExplicitLod %9 %13 %24 Grad %27 %30
)"};
    case ValidTextureOverload::kSampleGrad2dArrayOffsetF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 0 1 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
%9 = OpTypeVector %4 4
%12 = OpTypeSampledImage %3
%14 = OpTypeVector %4 3
%15 = OpTypeVector %4 2
%16 = OpConstant %4 1
%17 = OpConstant %4 2
%18 = OpConstantComposite %15 %16 %17
%22 = OpTypeInt 32 0
%23 = OpConstant %22 3
%25 = OpConstant %4 4
%26 = OpConstant %4 5
%27 = OpConstantComposite %15 %25 %26
%28 = OpConstant %4 6
%29 = OpConstant %4 7
%30 = OpConstantComposite %15 %28 %29
%32 = OpTypeInt 32 1
%31 = OpTypeVector %32 2
%33 = OpConstant %32 8
%34 = OpConstant %32 9
%35 = OpConstantComposite %31 %33 %34
)",
          R"(
%10 = OpLoad %7 %5
%11 = OpLoad %3 %1
%13 = OpSampledImage %12 %11 %10
%19 = OpCompositeExtract %4 %18 0
%20 = OpCompositeExtract %4 %18 1
%21 = OpConvertUToF %4 %23
%24 = OpCompositeConstruct %14 %19 %20 %21
%8 = OpImageSampleExplicitLod %9 %13 %24 Grad|Offset %27 %30 %35
)"};
    case ValidTextureOverload::kSampleGrad3dF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 3D 0 0 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
%9 = OpTypeVector %4 4
%12 = OpTypeSampledImage %3
%14 = OpTypeVector %4 3
%15 = OpConstant %4 1
%16 = OpConstant %4 2
%17 = OpConstant %4 3
%18 = OpConstantComposite %14 %15 %16 %17
%19 = OpConstant %4 4
%20 = OpConstant %4 5
%21 = OpConstant %4 6
%22 = OpConstantComposite %14 %19 %20 %21
%23 = OpConstant %4 7
%24 = OpConstant %4 8
%25 = OpConstant %4 9
%26 = OpConstantComposite %14 %23 %24 %25
)",
          R"(
%10 = OpLoad %7 %5
%11 = OpLoad %3 %1
%13 = OpSampledImage %12 %11 %10
%8 = OpImageSampleExplicitLod %9 %13 %18 Grad %22 %26
)"};
    case ValidTextureOverload::kSampleGrad3dOffsetF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 3D 0 0 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
%9 = OpTypeVector %4 4
%12 = OpTypeSampledImage %3
%14 = OpTypeVector %4 3
%15 = OpConstant %4 1
%16 = OpConstant %4 2
%17 = OpConstant %4 3
%18 = OpConstantComposite %14 %15 %16 %17
%19 = OpConstant %4 4
%20 = OpConstant %4 5
%21 = OpConstant %4 6
%22 = OpConstantComposite %14 %19 %20 %21
%23 = OpConstant %4 7
%24 = OpConstant %4 8
%25 = OpConstant %4 9
%26 = OpConstantComposite %14 %23 %24 %25
%28 = OpTypeInt 32 1
%27 = OpTypeVector %28 3
%29 = OpConstant %28 10
%30 = OpConstant %28 11
%31 = OpConstant %28 12
%32 = OpConstantComposite %27 %29 %30 %31
)",
          R"(
%10 = OpLoad %7 %5
%11 = OpLoad %3 %1
%13 = OpSampledImage %12 %11 %10
%8 = OpImageSampleExplicitLod %9 %13 %18 Grad|Offset %22 %26 %32
)"};
    case ValidTextureOverload::kSampleGradCubeF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 Cube 0 0 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
%9 = OpTypeVector %4 4
%12 = OpTypeSampledImage %3
%14 = OpTypeVector %4 3
%15 = OpConstant %4 1
%16 = OpConstant %4 2
%17 = OpConstant %4 3
%18 = OpConstantComposite %14 %15 %16 %17
%19 = OpConstant %4 4
%20 = OpConstant %4 5
%21 = OpConstant %4 6
%22 = OpConstantComposite %14 %19 %20 %21
%23 = OpConstant %4 7
%24 = OpConstant %4 8
%25 = OpConstant %4 9
%26 = OpConstantComposite %14 %23 %24 %25
)",
          R"(
%10 = OpLoad %7 %5
%11 = OpLoad %3 %1
%13 = OpSampledImage %12 %11 %10
%8 = OpImageSampleExplicitLod %9 %13 %18 Grad %22 %26
)"};
    case ValidTextureOverload::kSampleGradCubeArrayF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 Cube 0 1 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
%9 = OpTypeVector %4 4
%12 = OpTypeSampledImage %3
%14 = OpTypeVector %4 3
%15 = OpConstant %4 1
%16 = OpConstant %4 2
%17 = OpConstant %4 3
%18 = OpConstantComposite %14 %15 %16 %17
%23 = OpTypeInt 32 0
%24 = OpConstant %23 4
%26 = OpConstant %4 5
%27 = OpConstant %4 6
%28 = OpConstant %4 7
%29 = OpConstantComposite %14 %26 %27 %28
%30 = OpConstant %4 8
%31 = OpConstant %4 9
%32 = OpConstant %4 10
%33 = OpConstantComposite %14 %30 %31 %32
)",
          R"(
%10 = OpLoad %7 %5
%11 = OpLoad %3 %1
%13 = OpSampledImage %12 %11 %10
%19 = OpCompositeExtract %4 %18 0
%20 = OpCompositeExtract %4 %18 1
%21 = OpCompositeExtract %4 %18 2
%22 = OpConvertUToF %4 %24
%25 = OpCompositeConstruct %9 %19 %20 %21 %22
%8 = OpImageSampleExplicitLod %9 %13 %25 Grad %29 %33
)"};
    case ValidTextureOverload::kSampleGradDepth2dF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 1 0 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
%11 = OpTypeSampledImage %3
%13 = OpTypeVector %4 2
%14 = OpConstant %4 1
%15 = OpConstant %4 2
%16 = OpConstantComposite %13 %14 %15
%17 = OpConstant %4 3
%18 = OpConstant %4 0
)",
          R"(
%9 = OpLoad %7 %5
%10 = OpLoad %3 %1
%12 = OpSampledImage %11 %10 %9
%8 = OpImageSampleDrefExplicitLod %4 %12 %16 %17 Lod %18
)"};
    case ValidTextureOverload::kSampleGradDepth2dOffsetF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 1 0 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
%11 = OpTypeSampledImage %3
%13 = OpTypeVector %4 2
%14 = OpConstant %4 1
%15 = OpConstant %4 2
%16 = OpConstantComposite %13 %14 %15
%17 = OpConstant %4 3
%18 = OpConstant %4 0
%20 = OpTypeInt 32 1
%19 = OpTypeVector %20 2
%21 = OpConstant %20 4
%22 = OpConstant %20 5
%23 = OpConstantComposite %19 %21 %22
)",
          R"(
%9 = OpLoad %7 %5
%10 = OpLoad %3 %1
%12 = OpSampledImage %11 %10 %9
%8 = OpImageSampleDrefExplicitLod %4 %12 %16 %17 Lod|Offset %18 %23
)"};
    case ValidTextureOverload::kSampleGradDepth2dArrayF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 1 1 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
%11 = OpTypeSampledImage %3
%13 = OpTypeVector %4 3
%14 = OpTypeVector %4 2
%15 = OpConstant %4 1
%16 = OpConstant %4 2
%17 = OpConstantComposite %14 %15 %16
%21 = OpTypeInt 32 0
%22 = OpConstant %21 4
%24 = OpConstant %4 3
%25 = OpConstant %4 0
)",
          R"(
%9 = OpLoad %7 %5
%10 = OpLoad %3 %1
%12 = OpSampledImage %11 %10 %9
%18 = OpCompositeExtract %4 %17 0
%19 = OpCompositeExtract %4 %17 1
%20 = OpConvertUToF %4 %22
%23 = OpCompositeConstruct %13 %18 %19 %20
%8 = OpImageSampleDrefExplicitLod %4 %12 %23 %24 Lod %25
)"};
    case ValidTextureOverload::kSampleGradDepth2dArrayOffsetF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 1 1 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
%11 = OpTypeSampledImage %3
%13 = OpTypeVector %4 3
%14 = OpTypeVector %4 2
%15 = OpConstant %4 1
%16 = OpConstant %4 2
%17 = OpConstantComposite %14 %15 %16
%21 = OpTypeInt 32 0
%22 = OpConstant %21 4
%24 = OpConstant %4 3
%25 = OpConstant %4 0
%27 = OpTypeInt 32 1
%26 = OpTypeVector %27 2
%28 = OpConstant %27 5
%29 = OpConstant %27 6
%30 = OpConstantComposite %26 %28 %29
)",
          R"(
%9 = OpLoad %7 %5
%10 = OpLoad %3 %1
%12 = OpSampledImage %11 %10 %9
%18 = OpCompositeExtract %4 %17 0
%19 = OpCompositeExtract %4 %17 1
%20 = OpConvertUToF %4 %22
%23 = OpCompositeConstruct %13 %18 %19 %20
%8 = OpImageSampleDrefExplicitLod %4 %12 %23 %24 Lod|Offset %25 %30
)"};
    case ValidTextureOverload::kSampleGradDepthCubeF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 Cube 1 0 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
%11 = OpTypeSampledImage %3
%13 = OpTypeVector %4 3
%14 = OpConstant %4 1
%15 = OpConstant %4 2
%16 = OpConstant %4 3
%17 = OpConstantComposite %13 %14 %15 %16
%18 = OpConstant %4 4
%19 = OpConstant %4 0
)",
          R"(
%9 = OpLoad %7 %5
%10 = OpLoad %3 %1
%12 = OpSampledImage %11 %10 %9
%8 = OpImageSampleDrefExplicitLod %4 %12 %17 %18 Lod %19
)"};
    case ValidTextureOverload::kSampleGradDepthCubeArrayF32:
      return {
          R"(
%4 = OpTypeFloat 32
%3 = OpTypeImage %4 Cube 1 1 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
%11 = OpTypeSampledImage %3
%13 = OpTypeVector %4 4
%14 = OpTypeVector %4 3
%15 = OpConstant %4 1
%16 = OpConstant %4 2
%17 = OpConstant %4 3
%18 = OpConstantComposite %14 %15 %16 %17
%23 = OpTypeInt 32 0
%24 = OpConstant %23 4
%26 = OpConstant %4 5
%27 = OpConstant %4 0
)",
          R"(
%9 = OpLoad %7 %5
%10 = OpLoad %3 %1
%12 = OpSampledImage %11 %10 %9
%19 = OpCompositeExtract %4 %18 0
%20 = OpCompositeExtract %4 %18 1
%21 = OpCompositeExtract %4 %18 2
%22 = OpConvertUToF %4 %24
%25 = OpCompositeConstruct %13 %19 %20 %21 %22
%8 = OpImageSampleDrefExplicitLod %4 %12 %25 %26 Lod %27
)"};
  }
  return {"<unmatched texture overload>", "<unmatched texture overload>"};
}  // NOLINT - Ignore the length of this function

TEST_P(IntrinsicTextureTest, Call) {
  auto param = GetParam();

  b.push_function(Function{});

  ast::type::Type* datatype = nullptr;
  switch (param.texture_data_type) {
    case ast::intrinsic::test::TextureDataType::kF32:
      datatype = ty.f32;
      break;
    case ast::intrinsic::test::TextureDataType::kU32:
      datatype = ty.u32;
      break;
    case ast::intrinsic::test::TextureDataType::kI32:
      datatype = ty.i32;
      break;
  }

  ast::type::SamplerType sampler_type{param.sampler_kind};
  ast::Variable* tex = nullptr;
  switch (param.texture_kind) {
    case ast::intrinsic::test::TextureKind::kRegular:
      tex = Var("texture", ast::StorageClass::kNone,
                ctx->type_mgr().Get<ast::type::SampledTextureType>(
                    param.texture_dimension, datatype));
      break;

    case ast::intrinsic::test::TextureKind::kDepth:
      tex = Var("texture", ast::StorageClass::kNone,
                ctx->type_mgr().Get<ast::type::DepthTextureType>(
                    param.texture_dimension));
      break;
  }

  auto* sampler = Var("sampler", ast::StorageClass::kNone, &sampler_type);

  ASSERT_TRUE(b.GenerateGlobalVariable(tex)) << b.error();
  ASSERT_TRUE(b.GenerateGlobalVariable(sampler)) << b.error();

  ast::CallExpression call{Expr(param.function), param.args(this)};

  EXPECT_TRUE(td.DetermineResultType(&call)) << td.error();
  EXPECT_EQ(b.GenerateExpression(&call), 8u) << b.error();

  auto expected = expected_texture_overload(param.overload);
  EXPECT_EQ(expected.types, "\n" + DumpInstructions(b.types()));
  EXPECT_EQ(expected.instructions,
            "\n" + DumpInstructions(b.functions()[0].instructions()));
}

// This tests that we do not push OpTypeSampledImage and float_0 type twice.
TEST_F(IntrinsicBuilderTest, Call_TextureSampleCompare_Twice) {
  ast::type::SamplerType s(ast::type::SamplerKind::kComparisonSampler);
  ast::type::DepthTextureType t(ast::type::TextureDimension::k2d);

  b.push_function(Function{});

  auto* tex = Var("texture", ast::StorageClass::kNone, &t);
  ASSERT_TRUE(b.GenerateGlobalVariable(tex)) << b.error();

  auto* sampler = Var("sampler", ast::StorageClass::kNone, &s);
  ASSERT_TRUE(b.GenerateGlobalVariable(sampler)) << b.error();

  auto expr1 = Call("textureSampleCompare", "texture", "sampler",
                    vec2<f32>(1.0f, 2.0f), 2.0f);

  auto expr2 = Call("textureSampleCompare", "texture", "sampler",
                    vec2<f32>(1.0f, 2.0f), 2.0f);

  EXPECT_TRUE(td.DetermineResultType(&expr1)) << td.error();
  EXPECT_TRUE(td.DetermineResultType(&expr2)) << td.error();

  EXPECT_EQ(b.GenerateExpression(&expr1), 8u) << b.error();
  EXPECT_EQ(b.GenerateExpression(&expr2), 18u) << b.error();

  EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 1 0 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
%11 = OpTypeSampledImage %3
%13 = OpTypeVector %4 2
%14 = OpConstant %4 1
%15 = OpConstant %4 2
%16 = OpConstantComposite %13 %14 %15
%17 = OpConstant %4 0
)");

  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%9 = OpLoad %7 %5
%10 = OpLoad %3 %1
%12 = OpSampledImage %11 %10 %9
%8 = OpImageSampleDrefExplicitLod %4 %12 %16 %15 Lod %17
%19 = OpLoad %7 %5
%20 = OpLoad %3 %1
%21 = OpSampledImage %11 %20 %19
%18 = OpImageSampleDrefExplicitLod %4 %21 %16 %15 Lod %17
)");
}

TEST_F(IntrinsicBuilderTest, Call_GLSLMethod_WithLoad) {
  auto* var = Var("ident", ast::StorageClass::kPrivate, ty.f32);
  auto expr = Call("round", "ident");

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  ast::Function func("a_func", {}, ty.void_, create<ast::BlockStatement>());

  ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.error();
  ASSERT_TRUE(b.GenerateFunction(&func)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(&expr), 9u) << b.error();
  EXPECT_EQ(DumpBuilder(b), R"(%10 = OpExtInstImport "GLSL.std.450"
OpName %1 "tint_6964656e74"
OpName %7 "tint_615f66756e63"
%3 = OpTypeFloat 32
%2 = OpTypePointer Private %3
%4 = OpConstantNull %3
%1 = OpVariable %2 Private %4
%6 = OpTypeVoid
%5 = OpTypeFunction %6
%7 = OpFunction %6 None %5
%8 = OpLabel
%11 = OpLoad %3 %1
%9 = OpExtInst %3 %10 Round %11
OpReturn
OpFunctionEnd
)");
}

using Intrinsic_Builtin_SingleParam_Float_Test =
    IntrinsicBuilderTestWithParam<IntrinsicData>;
TEST_P(Intrinsic_Builtin_SingleParam_Float_Test, Call_Scalar) {
  auto param = GetParam();

  auto expr = Call(param.name, 1.0f);
  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  ast::Function func("a_func", {}, ty.void_, create<ast::BlockStatement>());

  ASSERT_TRUE(b.GenerateFunction(&func)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(&expr), 5u) << b.error();
  EXPECT_EQ(DumpBuilder(b), R"(%7 = OpExtInstImport "GLSL.std.450"
OpName %3 "tint_615f66756e63"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = OpTypeFloat 32
%8 = OpConstant %6 1
%3 = OpFunction %2 None %1
%4 = OpLabel
%5 = OpExtInst %6 %7 )" + param.op +
                                R"( %8
OpReturn
OpFunctionEnd
)");
}

TEST_P(Intrinsic_Builtin_SingleParam_Float_Test, Call_Vector) {
  auto param = GetParam();

  auto expr = Call(param.name, vec2<f32>(1.0f, 1.0f));
  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  ast::Function func("a_func", {}, ty.void_, create<ast::BlockStatement>());

  ASSERT_TRUE(b.GenerateFunction(&func)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(&expr), 5u) << b.error();
  EXPECT_EQ(DumpBuilder(b), R"(%8 = OpExtInstImport "GLSL.std.450"
OpName %3 "tint_615f66756e63"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%7 = OpTypeFloat 32
%6 = OpTypeVector %7 2
%9 = OpConstant %7 1
%10 = OpConstantComposite %6 %9 %9
%3 = OpFunction %2 None %1
%4 = OpLabel
%5 = OpExtInst %6 %8 )" + param.op +
                                R"( %10
OpReturn
OpFunctionEnd
)");
}
INSTANTIATE_TEST_SUITE_P(IntrinsicBuilderTest,
                         Intrinsic_Builtin_SingleParam_Float_Test,
                         testing::Values(IntrinsicData{"abs", "FAbs"},
                                         IntrinsicData{"acos", "Acos"},
                                         IntrinsicData{"asin", "Asin"},
                                         IntrinsicData{"atan", "Atan"},
                                         IntrinsicData{"ceil", "Ceil"},
                                         IntrinsicData{"cos", "Cos"},
                                         IntrinsicData{"cosh", "Cosh"},
                                         IntrinsicData{"exp", "Exp"},
                                         IntrinsicData{"exp2", "Exp2"},
                                         IntrinsicData{"floor", "Floor"},
                                         IntrinsicData{"fract", "Fract"},
                                         IntrinsicData{"inverseSqrt",
                                                       "InverseSqrt"},
                                         IntrinsicData{"log", "Log"},
                                         IntrinsicData{"log2", "Log2"},
                                         IntrinsicData{"round", "Round"},
                                         IntrinsicData{"sign", "FSign"},
                                         IntrinsicData{"sin", "Sin"},
                                         IntrinsicData{"sinh", "Sinh"},
                                         IntrinsicData{"sqrt", "Sqrt"},
                                         IntrinsicData{"tan", "Tan"},
                                         IntrinsicData{"tanh", "Tanh"},
                                         IntrinsicData{"trunc", "Trunc"}));

TEST_F(IntrinsicBuilderTest, Call_Length_Scalar) {
  auto expr = Call("length", 1.0f);

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  ast::Function func("a_func", {}, ty.void_, create<ast::BlockStatement>());

  ASSERT_TRUE(b.GenerateFunction(&func)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(&expr), 5u) << b.error();
  EXPECT_EQ(DumpBuilder(b), R"(%7 = OpExtInstImport "GLSL.std.450"
OpName %3 "tint_615f66756e63"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = OpTypeFloat 32
%8 = OpConstant %6 1
%3 = OpFunction %2 None %1
%4 = OpLabel
%5 = OpExtInst %6 %7 Length %8
OpReturn
OpFunctionEnd
)");
}

TEST_F(IntrinsicBuilderTest, Call_Length_Vector) {
  auto expr = Call("length", vec2<f32>(1.0f, 1.0f));
  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  ast::Function func("a_func", {}, ty.void_, create<ast::BlockStatement>());

  ASSERT_TRUE(b.GenerateFunction(&func)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(&expr), 5u) << b.error();
  EXPECT_EQ(DumpBuilder(b), R"(%7 = OpExtInstImport "GLSL.std.450"
OpName %3 "tint_615f66756e63"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = OpTypeFloat 32
%8 = OpTypeVector %6 2
%9 = OpConstant %6 1
%10 = OpConstantComposite %8 %9 %9
%3 = OpFunction %2 None %1
%4 = OpLabel
%5 = OpExtInst %6 %7 Length %10
OpReturn
OpFunctionEnd
)");
}

TEST_F(IntrinsicBuilderTest, Call_Normalize) {
  auto expr = Call("normalize", vec2<f32>(1.0f, 1.0f));
  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  ast::Function func("a_func", {}, ty.void_, create<ast::BlockStatement>());

  ASSERT_TRUE(b.GenerateFunction(&func)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(&expr), 5u) << b.error();
  EXPECT_EQ(DumpBuilder(b), R"(%8 = OpExtInstImport "GLSL.std.450"
OpName %3 "tint_615f66756e63"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%7 = OpTypeFloat 32
%6 = OpTypeVector %7 2
%9 = OpConstant %7 1
%10 = OpConstantComposite %6 %9 %9
%3 = OpFunction %2 None %1
%4 = OpLabel
%5 = OpExtInst %6 %8 Normalize %10
OpReturn
OpFunctionEnd
)");
}

using Intrinsic_Builtin_DualParam_Float_Test =
    IntrinsicBuilderTestWithParam<IntrinsicData>;
TEST_P(Intrinsic_Builtin_DualParam_Float_Test, Call_Scalar) {
  auto param = GetParam();

  auto expr = Call(param.name, 1.0f, 1.0f);

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  ast::Function func("a_func", {}, ty.void_, create<ast::BlockStatement>());

  ASSERT_TRUE(b.GenerateFunction(&func)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(&expr), 5u) << b.error();
  EXPECT_EQ(DumpBuilder(b), R"(%7 = OpExtInstImport "GLSL.std.450"
OpName %3 "tint_615f66756e63"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = OpTypeFloat 32
%8 = OpConstant %6 1
%3 = OpFunction %2 None %1
%4 = OpLabel
%5 = OpExtInst %6 %7 )" + param.op +
                                R"( %8 %8
OpReturn
OpFunctionEnd
)");
}

TEST_P(Intrinsic_Builtin_DualParam_Float_Test, Call_Vector) {
  auto param = GetParam();

  auto expr = Call(param.name, vec2<f32>(1.0f, 1.0f), vec2<f32>(1.0f, 1.0f));

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  ast::Function func("a_func", {}, ty.void_, create<ast::BlockStatement>());

  ASSERT_TRUE(b.GenerateFunction(&func)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(&expr), 5u) << b.error();
  EXPECT_EQ(DumpBuilder(b), R"(%8 = OpExtInstImport "GLSL.std.450"
OpName %3 "tint_615f66756e63"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%7 = OpTypeFloat 32
%6 = OpTypeVector %7 2
%9 = OpConstant %7 1
%10 = OpConstantComposite %6 %9 %9
%3 = OpFunction %2 None %1
%4 = OpLabel
%5 = OpExtInst %6 %8 )" + param.op +
                                R"( %10 %10
OpReturn
OpFunctionEnd
)");
}
INSTANTIATE_TEST_SUITE_P(IntrinsicBuilderTest,
                         Intrinsic_Builtin_DualParam_Float_Test,
                         testing::Values(IntrinsicData{"atan2", "Atan2"},
                                         IntrinsicData{"max", "NMax"},
                                         IntrinsicData{"min", "NMin"},
                                         IntrinsicData{"pow", "Pow"},
                                         IntrinsicData{"reflect", "Reflect"},
                                         IntrinsicData{"step", "Step"}));

TEST_F(IntrinsicBuilderTest, Call_Distance_Scalar) {
  auto expr = Call("distance", 1.0f, 1.0f);

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  ast::Function func("a_func", {}, ty.void_, create<ast::BlockStatement>());

  ASSERT_TRUE(b.GenerateFunction(&func)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(&expr), 5u) << b.error();
  EXPECT_EQ(DumpBuilder(b), R"(%7 = OpExtInstImport "GLSL.std.450"
OpName %3 "tint_615f66756e63"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = OpTypeFloat 32
%8 = OpConstant %6 1
%3 = OpFunction %2 None %1
%4 = OpLabel
%5 = OpExtInst %6 %7 Distance %8 %8
OpReturn
OpFunctionEnd
)");
}

TEST_F(IntrinsicBuilderTest, Call_Distance_Vector) {
  auto expr = Call("distance", vec2<f32>(1.0f, 1.0f), vec2<f32>(1.0f, 1.0f));

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  ast::Function func("a_func", {}, ty.void_, create<ast::BlockStatement>());

  ASSERT_TRUE(b.GenerateFunction(&func)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(&expr), 5u) << b.error();
  EXPECT_EQ(DumpBuilder(b), R"(%7 = OpExtInstImport "GLSL.std.450"
OpName %3 "tint_615f66756e63"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = OpTypeFloat 32
%8 = OpTypeVector %6 2
%9 = OpConstant %6 1
%10 = OpConstantComposite %8 %9 %9
%3 = OpFunction %2 None %1
%4 = OpLabel
%5 = OpExtInst %6 %7 Distance %10 %10
OpReturn
OpFunctionEnd
)");
}

TEST_F(IntrinsicBuilderTest, Call_Cross) {
  auto expr =
      Call("cross", vec3<f32>(1.0f, 1.0f, 1.0f), vec3<f32>(1.0f, 1.0f, 1.0f));

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  ast::Function func("a_func", {}, ty.void_, create<ast::BlockStatement>());

  ASSERT_TRUE(b.GenerateFunction(&func)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(&expr), 5u) << b.error();
  EXPECT_EQ(DumpBuilder(b), R"(%8 = OpExtInstImport "GLSL.std.450"
OpName %3 "tint_615f66756e63"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%7 = OpTypeFloat 32
%6 = OpTypeVector %7 3
%9 = OpConstant %7 1
%10 = OpConstantComposite %6 %9 %9 %9
%3 = OpFunction %2 None %1
%4 = OpLabel
%5 = OpExtInst %6 %8 Cross %10 %10
OpReturn
OpFunctionEnd
)");
}

using Intrinsic_Builtin_ThreeParam_Float_Test =
    IntrinsicBuilderTestWithParam<IntrinsicData>;
TEST_P(Intrinsic_Builtin_ThreeParam_Float_Test, Call_Scalar) {
  auto param = GetParam();

  auto expr = Call(param.name, 1.0f, 1.0f, 1.0f);
  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  ast::Function func("a_func", {}, ty.void_, create<ast::BlockStatement>());

  ASSERT_TRUE(b.GenerateFunction(&func)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(&expr), 5u) << b.error();
  EXPECT_EQ(DumpBuilder(b), R"(%7 = OpExtInstImport "GLSL.std.450"
OpName %3 "tint_615f66756e63"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = OpTypeFloat 32
%8 = OpConstant %6 1
%3 = OpFunction %2 None %1
%4 = OpLabel
%5 = OpExtInst %6 %7 )" + param.op +
                                R"( %8 %8 %8
OpReturn
OpFunctionEnd
)");
}

TEST_P(Intrinsic_Builtin_ThreeParam_Float_Test, Call_Vector) {
  auto param = GetParam();

  auto expr = Call(param.name, vec2<f32>(1.0f, 1.0f), vec2<f32>(1.0f, 1.0f),
                   vec2<f32>(1.0f, 1.0f));

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  ast::Function func("a_func", {}, ty.void_, create<ast::BlockStatement>());

  ASSERT_TRUE(b.GenerateFunction(&func)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(&expr), 5u) << b.error();
  EXPECT_EQ(DumpBuilder(b), R"(%8 = OpExtInstImport "GLSL.std.450"
OpName %3 "tint_615f66756e63"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%7 = OpTypeFloat 32
%6 = OpTypeVector %7 2
%9 = OpConstant %7 1
%10 = OpConstantComposite %6 %9 %9
%3 = OpFunction %2 None %1
%4 = OpLabel
%5 = OpExtInst %6 %8 )" + param.op +
                                R"( %10 %10 %10
OpReturn
OpFunctionEnd
)");
}
INSTANTIATE_TEST_SUITE_P(
    IntrinsicBuilderTest,
    Intrinsic_Builtin_ThreeParam_Float_Test,
    testing::Values(IntrinsicData{"clamp", "NClamp"},
                    IntrinsicData{"faceForward", "FaceForward"},
                    IntrinsicData{"fma", "Fma"},
                    IntrinsicData{"mix", "FMix"},

                    IntrinsicData{"smoothStep", "SmoothStep"}));

using Intrinsic_Builtin_SingleParam_Sint_Test =
    IntrinsicBuilderTestWithParam<IntrinsicData>;
TEST_P(Intrinsic_Builtin_SingleParam_Sint_Test, Call_Scalar) {
  auto param = GetParam();

  auto expr = Call(param.name, 1);
  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  ast::Function func("a_func", {}, ty.void_, create<ast::BlockStatement>());

  ASSERT_TRUE(b.GenerateFunction(&func)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(&expr), 5u) << b.error();
  EXPECT_EQ(DumpBuilder(b), R"(%7 = OpExtInstImport "GLSL.std.450"
OpName %3 "tint_615f66756e63"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = OpTypeInt 32 1
%8 = OpConstant %6 1
%3 = OpFunction %2 None %1
%4 = OpLabel
%5 = OpExtInst %6 %7 )" + param.op +
                                R"( %8
OpReturn
OpFunctionEnd
)");
}

TEST_P(Intrinsic_Builtin_SingleParam_Sint_Test, Call_Vector) {
  auto param = GetParam();

  auto expr = Call(param.name, vec2<i32>(1, 1));
  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  ast::Function func("a_func", {}, ty.void_, create<ast::BlockStatement>());

  ASSERT_TRUE(b.GenerateFunction(&func)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(&expr), 5u) << b.error();
  EXPECT_EQ(DumpBuilder(b), R"(%8 = OpExtInstImport "GLSL.std.450"
OpName %3 "tint_615f66756e63"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%7 = OpTypeInt 32 1
%6 = OpTypeVector %7 2
%9 = OpConstant %7 1
%10 = OpConstantComposite %6 %9 %9
%3 = OpFunction %2 None %1
%4 = OpLabel
%5 = OpExtInst %6 %8 )" + param.op +
                                R"( %10
OpReturn
OpFunctionEnd
)");
}
INSTANTIATE_TEST_SUITE_P(IntrinsicBuilderTest,
                         Intrinsic_Builtin_SingleParam_Sint_Test,
                         testing::Values(IntrinsicData{"abs", "SAbs"}));

using Intrinsic_Builtin_SingleParam_Uint_Test =
    IntrinsicBuilderTestWithParam<IntrinsicData>;
TEST_P(Intrinsic_Builtin_SingleParam_Uint_Test, Call_Scalar) {
  auto param = GetParam();

  auto expr = Call(param.name, 1u);
  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  ast::Function func("a_func", {}, ty.void_, create<ast::BlockStatement>());

  ASSERT_TRUE(b.GenerateFunction(&func)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(&expr), 5u) << b.error();
  EXPECT_EQ(DumpBuilder(b), R"(%7 = OpExtInstImport "GLSL.std.450"
OpName %3 "tint_615f66756e63"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = OpTypeInt 32 0
%8 = OpConstant %6 1
%3 = OpFunction %2 None %1
%4 = OpLabel
%5 = OpExtInst %6 %7 )" + param.op +
                                R"( %8
OpReturn
OpFunctionEnd
)");
}

TEST_P(Intrinsic_Builtin_SingleParam_Uint_Test, Call_Vector) {
  auto param = GetParam();

  auto expr = Call(param.name, vec2<u32>(1u, 1u));
  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  ast::Function func("a_func", {}, ty.void_, create<ast::BlockStatement>());

  ASSERT_TRUE(b.GenerateFunction(&func)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(&expr), 5u) << b.error();
  EXPECT_EQ(DumpBuilder(b), R"(%8 = OpExtInstImport "GLSL.std.450"
OpName %3 "tint_615f66756e63"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%7 = OpTypeInt 32 0
%6 = OpTypeVector %7 2
%9 = OpConstant %7 1
%10 = OpConstantComposite %6 %9 %9
%3 = OpFunction %2 None %1
%4 = OpLabel
%5 = OpExtInst %6 %8 )" + param.op +
                                R"( %10
OpReturn
OpFunctionEnd
)");
}
INSTANTIATE_TEST_SUITE_P(IntrinsicBuilderTest,
                         Intrinsic_Builtin_SingleParam_Uint_Test,
                         testing::Values(IntrinsicData{"abs", "SAbs"}));

using Intrinsic_Builtin_DualParam_SInt_Test =
    IntrinsicBuilderTestWithParam<IntrinsicData>;
TEST_P(Intrinsic_Builtin_DualParam_SInt_Test, Call_Scalar) {
  auto param = GetParam();

  auto expr = Call(param.name, 1, 1);
  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  ast::Function func("a_func", {}, ty.void_, create<ast::BlockStatement>());

  ASSERT_TRUE(b.GenerateFunction(&func)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(&expr), 5u) << b.error();
  EXPECT_EQ(DumpBuilder(b), R"(%7 = OpExtInstImport "GLSL.std.450"
OpName %3 "tint_615f66756e63"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = OpTypeInt 32 1
%8 = OpConstant %6 1
%3 = OpFunction %2 None %1
%4 = OpLabel
%5 = OpExtInst %6 %7 )" + param.op +
                                R"( %8 %8
OpReturn
OpFunctionEnd
)");
}

TEST_P(Intrinsic_Builtin_DualParam_SInt_Test, Call_Vector) {
  auto param = GetParam();

  auto expr = Call(param.name, vec2<i32>(1, 1), vec2<i32>(1, 1));
  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  ast::Function func("a_func", {}, ty.void_, create<ast::BlockStatement>());

  ASSERT_TRUE(b.GenerateFunction(&func)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(&expr), 5u) << b.error();
  EXPECT_EQ(DumpBuilder(b), R"(%8 = OpExtInstImport "GLSL.std.450"
OpName %3 "tint_615f66756e63"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%7 = OpTypeInt 32 1
%6 = OpTypeVector %7 2
%9 = OpConstant %7 1
%10 = OpConstantComposite %6 %9 %9
%3 = OpFunction %2 None %1
%4 = OpLabel
%5 = OpExtInst %6 %8 )" + param.op +
                                R"( %10 %10
OpReturn
OpFunctionEnd
)");
}
INSTANTIATE_TEST_SUITE_P(IntrinsicBuilderTest,
                         Intrinsic_Builtin_DualParam_SInt_Test,
                         testing::Values(IntrinsicData{"max", "SMax"},
                                         IntrinsicData{"min", "SMin"}));

using Intrinsic_Builtin_DualParam_UInt_Test =
    IntrinsicBuilderTestWithParam<IntrinsicData>;
TEST_P(Intrinsic_Builtin_DualParam_UInt_Test, Call_Scalar) {
  auto param = GetParam();

  auto expr = Call(param.name, 1u, 1u);
  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  ast::Function func("a_func", {}, ty.void_, create<ast::BlockStatement>());

  ASSERT_TRUE(b.GenerateFunction(&func)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(&expr), 5u) << b.error();
  EXPECT_EQ(DumpBuilder(b), R"(%7 = OpExtInstImport "GLSL.std.450"
OpName %3 "tint_615f66756e63"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = OpTypeInt 32 0
%8 = OpConstant %6 1
%3 = OpFunction %2 None %1
%4 = OpLabel
%5 = OpExtInst %6 %7 )" + param.op +
                                R"( %8 %8
OpReturn
OpFunctionEnd
)");
}

TEST_P(Intrinsic_Builtin_DualParam_UInt_Test, Call_Vector) {
  auto param = GetParam();

  auto expr = Call(param.name, vec2<u32>(1u, 1u), vec2<u32>(1u, 1u));
  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  ast::Function func("a_func", {}, ty.void_, create<ast::BlockStatement>());

  ASSERT_TRUE(b.GenerateFunction(&func)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(&expr), 5u) << b.error();
  EXPECT_EQ(DumpBuilder(b), R"(%8 = OpExtInstImport "GLSL.std.450"
OpName %3 "tint_615f66756e63"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%7 = OpTypeInt 32 0
%6 = OpTypeVector %7 2
%9 = OpConstant %7 1
%10 = OpConstantComposite %6 %9 %9
%3 = OpFunction %2 None %1
%4 = OpLabel
%5 = OpExtInst %6 %8 )" + param.op +
                                R"( %10 %10
OpReturn
OpFunctionEnd
)");
}
INSTANTIATE_TEST_SUITE_P(IntrinsicBuilderTest,
                         Intrinsic_Builtin_DualParam_UInt_Test,
                         testing::Values(IntrinsicData{"max", "UMax"},
                                         IntrinsicData{"min", "UMin"}));

using Intrinsic_Builtin_ThreeParam_Sint_Test =
    IntrinsicBuilderTestWithParam<IntrinsicData>;
TEST_P(Intrinsic_Builtin_ThreeParam_Sint_Test, Call_Scalar) {
  auto param = GetParam();

  auto expr = Call(param.name, 1, 1, 1);
  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  ast::Function func("a_func", {}, ty.void_, create<ast::BlockStatement>());

  ASSERT_TRUE(b.GenerateFunction(&func)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(&expr), 5u) << b.error();
  EXPECT_EQ(DumpBuilder(b), R"(%7 = OpExtInstImport "GLSL.std.450"
OpName %3 "tint_615f66756e63"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = OpTypeInt 32 1
%8 = OpConstant %6 1
%3 = OpFunction %2 None %1
%4 = OpLabel
%5 = OpExtInst %6 %7 )" + param.op +
                                R"( %8 %8 %8
OpReturn
OpFunctionEnd
)");
}

TEST_P(Intrinsic_Builtin_ThreeParam_Sint_Test, Call_Vector) {
  auto param = GetParam();

  auto expr =
      Call(param.name, vec2<i32>(1, 1), vec2<i32>(1, 1), vec2<i32>(1, 1));

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  ast::Function func("a_func", {}, ty.void_, create<ast::BlockStatement>());

  ASSERT_TRUE(b.GenerateFunction(&func)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(&expr), 5u) << b.error();
  EXPECT_EQ(DumpBuilder(b), R"(%8 = OpExtInstImport "GLSL.std.450"
OpName %3 "tint_615f66756e63"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%7 = OpTypeInt 32 1
%6 = OpTypeVector %7 2
%9 = OpConstant %7 1
%10 = OpConstantComposite %6 %9 %9
%3 = OpFunction %2 None %1
%4 = OpLabel
%5 = OpExtInst %6 %8 )" + param.op +
                                R"( %10 %10 %10
OpReturn
OpFunctionEnd
)");
}
INSTANTIATE_TEST_SUITE_P(IntrinsicBuilderTest,
                         Intrinsic_Builtin_ThreeParam_Sint_Test,
                         testing::Values(IntrinsicData{"clamp", "SClamp"}));

using Intrinsic_Builtin_ThreeParam_Uint_Test =
    IntrinsicBuilderTestWithParam<IntrinsicData>;
TEST_P(Intrinsic_Builtin_ThreeParam_Uint_Test, Call_Scalar) {
  auto param = GetParam();

  auto expr = Call(param.name, 1u, 1u, 1u);
  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  ast::Function func("a_func", {}, ty.void_, create<ast::BlockStatement>());

  ASSERT_TRUE(b.GenerateFunction(&func)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(&expr), 5u) << b.error();
  EXPECT_EQ(DumpBuilder(b), R"(%7 = OpExtInstImport "GLSL.std.450"
OpName %3 "tint_615f66756e63"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = OpTypeInt 32 0
%8 = OpConstant %6 1
%3 = OpFunction %2 None %1
%4 = OpLabel
%5 = OpExtInst %6 %7 )" + param.op +
                                R"( %8 %8 %8
OpReturn
OpFunctionEnd
)");
}

TEST_P(Intrinsic_Builtin_ThreeParam_Uint_Test, Call_Vector) {
  auto param = GetParam();

  auto expr =
      Call(param.name, vec2<u32>(1u, 1u), vec2<u32>(1u, 1u), vec2<u32>(1u, 1u));

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  ast::Function func("a_func", {}, ty.void_, create<ast::BlockStatement>());

  ASSERT_TRUE(b.GenerateFunction(&func)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(&expr), 5u) << b.error();
  EXPECT_EQ(DumpBuilder(b), R"(%8 = OpExtInstImport "GLSL.std.450"
OpName %3 "tint_615f66756e63"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%7 = OpTypeInt 32 0
%6 = OpTypeVector %7 2
%9 = OpConstant %7 1
%10 = OpConstantComposite %6 %9 %9
%3 = OpFunction %2 None %1
%4 = OpLabel
%5 = OpExtInst %6 %8 )" + param.op +
                                R"( %10 %10 %10
OpReturn
OpFunctionEnd
)");
}
INSTANTIATE_TEST_SUITE_P(IntrinsicBuilderTest,
                         Intrinsic_Builtin_ThreeParam_Uint_Test,
                         testing::Values(IntrinsicData{"clamp", "UClamp"}));

TEST_F(IntrinsicBuilderTest, Call_Determinant) {
  auto* var = Var("var", ast::StorageClass::kPrivate, ty.mat3x3<f32>());
  auto expr = Call("determinant", "var");

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  ast::Function func("a_func", {}, ty.void_, create<ast::BlockStatement>());

  ASSERT_TRUE(b.GenerateFunction(&func)) << b.error();

  ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.error();
  EXPECT_EQ(b.GenerateCallExpression(&expr), 11u) << b.error();

  EXPECT_EQ(DumpBuilder(b), R"(%12 = OpExtInstImport "GLSL.std.450"
OpName %3 "tint_615f66756e63"
OpName %5 "tint_766172"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%9 = OpTypeFloat 32
%8 = OpTypeVector %9 3
%7 = OpTypeMatrix %8 3
%6 = OpTypePointer Private %7
%10 = OpConstantNull %7
%5 = OpVariable %6 Private %10
%3 = OpFunction %2 None %1
%4 = OpLabel
%13 = OpLoad %7 %5
%11 = OpExtInst %9 %12 Determinant %13
OpReturn
OpFunctionEnd
)");
}

TEST_F(IntrinsicBuilderTest, Call_ArrayLength) {
  ast::StructMemberDecorationList decos;
  ast::StructMemberList members;
  members.push_back(create<ast::StructMember>("a", ty.array<f32>(), decos));

  auto* s = create<ast::Struct>(members);
  ast::type::StructType s_type("my_struct", s);

  auto* var = Var("b", ast::StorageClass::kPrivate, &s_type);

  auto expr = Call("arrayLength",
                   create<ast::MemberAccessorExpression>(Expr("b"), Expr("a")));

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  ast::Function func("a_func", {}, ty.void_, create<ast::BlockStatement>());

  ASSERT_TRUE(b.GenerateFunction(&func)) << b.error();
  ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.error();
  EXPECT_EQ(b.GenerateExpression(&expr), 11u) << b.error();

  EXPECT_EQ(DumpInstructions(b.types()),
            R"(%2 = OpTypeVoid
%1 = OpTypeFunction %2
%9 = OpTypeFloat 32
%8 = OpTypeRuntimeArray %9
%7 = OpTypeStruct %8
%6 = OpTypePointer Private %7
%10 = OpConstantNull %7
%5 = OpVariable %6 Private %10
%12 = OpTypeInt 32 0
)");

  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%11 = OpArrayLength %12 %5 0
)");
}

TEST_F(IntrinsicBuilderTest, Call_ArrayLength_OtherMembersInStruct) {
  ast::StructMemberDecorationList decos;
  ast::StructMemberList members;
  members.push_back(create<ast::StructMember>("z", ty.f32, decos));
  members.push_back(create<ast::StructMember>("a", ty.array<f32>(), decos));

  auto* s = create<ast::Struct>(members);
  ast::type::StructType s_type("my_struct", s);

  auto* var = Var("b", ast::StorageClass::kPrivate, &s_type);
  auto expr = Call("arrayLength",
                   create<ast::MemberAccessorExpression>(Expr("b"), Expr("a")));

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  ast::Function func("a_func", {}, ty.void_, create<ast::BlockStatement>());

  ASSERT_TRUE(b.GenerateFunction(&func)) << b.error();
  ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.error();
  EXPECT_EQ(b.GenerateExpression(&expr), 11u) << b.error();

  EXPECT_EQ(DumpInstructions(b.types()),
            R"(%2 = OpTypeVoid
%1 = OpTypeFunction %2
%8 = OpTypeFloat 32
%9 = OpTypeRuntimeArray %8
%7 = OpTypeStruct %8 %9
%6 = OpTypePointer Private %7
%10 = OpConstantNull %7
%5 = OpVariable %6 Private %10
%12 = OpTypeInt 32 0
)");

  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%11 = OpArrayLength %12 %5 1
)");
}

// TODO(dsinclair): https://bugs.chromium.org/p/tint/issues/detail?id=266
TEST_F(IntrinsicBuilderTest, DISABLED_Call_ArrayLength_Ptr) {
  ast::type::PointerType ptr(ty.array<f32>(),
                             ast::StorageClass::kStorageBuffer);

  ast::StructMemberDecorationList decos;
  ast::StructMemberList members;
  members.push_back(create<ast::StructMember>("z", ty.f32, decos));
  members.push_back(create<ast::StructMember>("a", ty.array<f32>(), decos));

  auto* s = create<ast::Struct>(members);
  ast::type::StructType s_type("my_struct", s);

  auto* var = Var("b", ast::StorageClass::kPrivate, &s_type);

  auto* ptr_var = Var("ptr_var", ast::StorageClass::kPrivate, &ptr);
  ptr_var->set_constructor(
      create<ast::MemberAccessorExpression>(Expr("b"), Expr("a")));

  auto expr = Call("arrayLength", "ptr_var");
  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  ast::Function func("a_func", {}, ty.void_, create<ast::BlockStatement>());

  ASSERT_TRUE(b.GenerateFunction(&func)) << b.error();
  ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.error();
  EXPECT_EQ(b.GenerateExpression(&expr), 11u) << b.error();

  EXPECT_EQ(DumpInstructions(b.types()), R"( ... )");

  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%11 = OpArrayLength %12 %5 1
)");
}

}  // namespace
}  // namespace spirv
}  // namespace writer
}  // namespace tint
