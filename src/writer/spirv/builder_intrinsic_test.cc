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
#include "src/ast/call_expression.h"
#include "src/ast/float_literal.h"
#include "src/ast/identifier_expression.h"
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

using BuilderTest = testing::Test;

struct IntrinsicData {
  std::string name;
  std::string op;
};
inline std::ostream& operator<<(std::ostream& out, IntrinsicData data) {
  out << data.name;
  return out;
}

using IntrinsicBoolTest = testing::TestWithParam<IntrinsicData>;
TEST_P(IntrinsicBoolTest, Call_Bool) {
  auto param = GetParam();

  ast::type::BoolType bool_type;
  ast::type::VectorType vec3(&bool_type, 3);

  auto var =
      std::make_unique<ast::Variable>("v", ast::StorageClass::kPrivate, &vec3);

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::IdentifierExpression>("v"));
  ast::CallExpression expr(
      std::make_unique<ast::IdentifierExpression>(param.name),
      std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  td.RegisterVariableForTesting(var.get());

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateGlobalVariable(var.get())) << b.error();

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
INSTANTIATE_TEST_SUITE_P(BuilderTest,
                         IntrinsicBoolTest,
                         testing::Values(IntrinsicData{"any", "OpAny"},
                                         IntrinsicData{"all", "OpAll"}));

using IntrinsicFloatTest = testing::TestWithParam<IntrinsicData>;
TEST_P(IntrinsicFloatTest, Call_Float_Scalar) {
  auto param = GetParam();

  ast::type::F32Type f32;

  auto var =
      std::make_unique<ast::Variable>("v", ast::StorageClass::kPrivate, &f32);

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::IdentifierExpression>("v"));
  ast::CallExpression expr(
      std::make_unique<ast::IdentifierExpression>(param.name),
      std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  td.RegisterVariableForTesting(var.get());

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateGlobalVariable(var.get())) << b.error();

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

  ast::type::F32Type f32;
  ast::type::VectorType vec3(&f32, 3);

  auto var =
      std::make_unique<ast::Variable>("v", ast::StorageClass::kPrivate, &vec3);

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::IdentifierExpression>("v"));
  ast::CallExpression expr(
      std::make_unique<ast::IdentifierExpression>(param.name),
      std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  td.RegisterVariableForTesting(var.get());

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateGlobalVariable(var.get())) << b.error();

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
INSTANTIATE_TEST_SUITE_P(BuilderTest,
                         IntrinsicFloatTest,
                         testing::Values(IntrinsicData{"isNan", "OpIsNan"},
                                         IntrinsicData{"isInf", "OpIsInf"}));

using IntrinsicIntTest = testing::TestWithParam<IntrinsicData>;
TEST_P(IntrinsicIntTest, Call_SInt_Scalar) {
  auto param = GetParam();

  ast::type::I32Type i32;

  auto var =
      std::make_unique<ast::Variable>("v", ast::StorageClass::kPrivate, &i32);

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::IdentifierExpression>("v"));
  ast::CallExpression expr(
      std::make_unique<ast::IdentifierExpression>(param.name),
      std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  td.RegisterVariableForTesting(var.get());

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateGlobalVariable(var.get())) << b.error();

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

  ast::type::I32Type i32;
  ast::type::VectorType vec3(&i32, 3);

  auto var =
      std::make_unique<ast::Variable>("v", ast::StorageClass::kPrivate, &vec3);

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::IdentifierExpression>("v"));
  ast::CallExpression expr(
      std::make_unique<ast::IdentifierExpression>(param.name),
      std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  td.RegisterVariableForTesting(var.get());

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateGlobalVariable(var.get())) << b.error();

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

  ast::type::U32Type u32;

  auto var =
      std::make_unique<ast::Variable>("v", ast::StorageClass::kPrivate, &u32);

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::IdentifierExpression>("v"));
  ast::CallExpression expr(
      std::make_unique<ast::IdentifierExpression>(param.name),
      std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  td.RegisterVariableForTesting(var.get());

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateGlobalVariable(var.get())) << b.error();

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

  ast::type::U32Type u32;
  ast::type::VectorType vec3(&u32, 3);

  auto var =
      std::make_unique<ast::Variable>("v", ast::StorageClass::kPrivate, &vec3);

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::IdentifierExpression>("v"));
  ast::CallExpression expr(
      std::make_unique<ast::IdentifierExpression>(param.name),
      std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  td.RegisterVariableForTesting(var.get());

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateGlobalVariable(var.get())) << b.error();

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
    BuilderTest,
    IntrinsicIntTest,
    testing::Values(IntrinsicData{"countOneBits", "OpBitCount"},
                    IntrinsicData{"reverseBits", "OpBitReverse"}));

TEST_F(BuilderTest, Call_Dot) {
  ast::type::F32Type f32;
  ast::type::VectorType vec3(&f32, 3);

  auto var =
      std::make_unique<ast::Variable>("v", ast::StorageClass::kPrivate, &vec3);

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::IdentifierExpression>("v"));
  params.push_back(std::make_unique<ast::IdentifierExpression>("v"));
  ast::CallExpression expr(std::make_unique<ast::IdentifierExpression>("dot"),
                           std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  td.RegisterVariableForTesting(var.get());

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateGlobalVariable(var.get())) << b.error();

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

using IntrinsicDeriveTest = testing::TestWithParam<IntrinsicData>;
TEST_P(IntrinsicDeriveTest, Call_Derivative_Scalar) {
  auto param = GetParam();

  ast::type::F32Type f32;

  auto var =
      std::make_unique<ast::Variable>("v", ast::StorageClass::kPrivate, &f32);

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::IdentifierExpression>("v"));
  ast::CallExpression expr(
      std::make_unique<ast::IdentifierExpression>(param.name),
      std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  td.RegisterVariableForTesting(var.get());

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateGlobalVariable(var.get())) << b.error();

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

  ast::type::F32Type f32;
  ast::type::VectorType vec3(&f32, 3);

  auto var =
      std::make_unique<ast::Variable>("v", ast::StorageClass::kPrivate, &vec3);

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::IdentifierExpression>("v"));
  ast::CallExpression expr(
      std::make_unique<ast::IdentifierExpression>(param.name),
      std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  td.RegisterVariableForTesting(var.get());

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateGlobalVariable(var.get())) << b.error();

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
    BuilderTest,
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

TEST_F(BuilderTest, Call_OuterProduct) {
  ast::type::F32Type f32;
  ast::type::VectorType vec2(&f32, 2);
  ast::type::VectorType vec3(&f32, 3);
  ast::type::MatrixType mat(&f32, 2, 3);

  auto v2 =
      std::make_unique<ast::Variable>("v2", ast::StorageClass::kPrivate, &vec2);
  auto v3 =
      std::make_unique<ast::Variable>("v3", ast::StorageClass::kPrivate, &vec3);

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::IdentifierExpression>("v2"));
  params.push_back(std::make_unique<ast::IdentifierExpression>("v3"));
  ast::CallExpression expr(
      std::make_unique<ast::IdentifierExpression>("outerProduct"),
      std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  td.RegisterVariableForTesting(v2.get());
  td.RegisterVariableForTesting(v3.get());

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateGlobalVariable(v2.get())) << b.error();
  ASSERT_TRUE(b.GenerateGlobalVariable(v3.get())) << b.error();

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

TEST_F(BuilderTest, Call_Select) {
  ast::type::F32Type f32;
  ast::type::BoolType bool_type;
  ast::type::VectorType bool_vec3(&bool_type, 3);
  ast::type::VectorType vec3(&f32, 3);

  auto v3 =
      std::make_unique<ast::Variable>("v3", ast::StorageClass::kPrivate, &vec3);
  auto bool_v3 = std::make_unique<ast::Variable>(
      "bool_v3", ast::StorageClass::kPrivate, &bool_vec3);

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::IdentifierExpression>("v3"));
  params.push_back(std::make_unique<ast::IdentifierExpression>("v3"));
  params.push_back(std::make_unique<ast::IdentifierExpression>("bool_v3"));
  ast::CallExpression expr(
      std::make_unique<ast::IdentifierExpression>("select"), std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  td.RegisterVariableForTesting(v3.get());
  td.RegisterVariableForTesting(bool_v3.get());

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateGlobalVariable(v3.get())) << b.error();
  ASSERT_TRUE(b.GenerateGlobalVariable(bool_v3.get())) << b.error();

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

TEST_F(BuilderTest, Call_TextureLoad_Storage_RO_1d) {
  ast::type::F32Type f32;
  ast::type::I32Type i32;
  ast::type::StorageTextureType s(ast::type::TextureDimension::k1d,
                                  ast::type::StorageAccess::kRead,
                                  ast::type::ImageFormat::kR16Float);

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  Builder b(&mod);

  ASSERT_TRUE(td.DetermineStorageTextureSubtype(&s)) << td.error();

  b.push_function(Function{});

  ast::Variable tex("texture", ast::StorageClass::kNone, &s);
  td.RegisterVariableForTesting(&tex);
  ASSERT_TRUE(b.GenerateGlobalVariable(&tex)) << b.error();

  ast::ExpressionList call_params;
  call_params.push_back(std::make_unique<ast::IdentifierExpression>("texture"));
  call_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0)));
  call_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 2)));

  ast::CallExpression expr(
      std::make_unique<ast::IdentifierExpression>("textureLoad"),
      std::move(call_params));

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

TEST_F(BuilderTest, Call_TextureLoad_Storage_RO_2d) {
  ast::type::F32Type f32;
  ast::type::I32Type i32;
  ast::type::VectorType vec2(&f32, 2);
  ast::type::StorageTextureType s(ast::type::TextureDimension::k2d,
                                  ast::type::StorageAccess::kRead,
                                  ast::type::ImageFormat::kR16Float);

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  Builder b(&mod);

  ASSERT_TRUE(td.DetermineStorageTextureSubtype(&s)) << td.error();

  b.push_function(Function{});

  ast::Variable tex("texture", ast::StorageClass::kNone, &s);
  td.RegisterVariableForTesting(&tex);
  ASSERT_TRUE(b.GenerateGlobalVariable(&tex)) << b.error();

  ast::ExpressionList vals;
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));

  ast::ExpressionList call_params;
  call_params.push_back(std::make_unique<ast::IdentifierExpression>("texture"));
  call_params.push_back(
      std::make_unique<ast::TypeConstructorExpression>(&vec2, std::move(vals)));
  call_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 2)));

  ast::CallExpression expr(
      std::make_unique<ast::IdentifierExpression>("textureLoad"),
      std::move(call_params));

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

TEST_F(BuilderTest, Call_TextureLoad_Sampled_1d) {
  ast::type::F32Type f32;
  ast::type::I32Type i32;
  ast::type::SampledTextureType s(ast::type::TextureDimension::k1d, &f32);

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  Builder b(&mod);

  b.push_function(Function{});

  ast::Variable tex("texture", ast::StorageClass::kNone, &s);
  td.RegisterVariableForTesting(&tex);
  ASSERT_TRUE(b.GenerateGlobalVariable(&tex)) << b.error();

  ast::ExpressionList call_params;
  call_params.push_back(std::make_unique<ast::IdentifierExpression>("texture"));
  call_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0)));
  call_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 2)));

  ast::CallExpression expr(
      std::make_unique<ast::IdentifierExpression>("textureLoad"),
      std::move(call_params));

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

TEST_F(BuilderTest, Call_TextureLoad_Sampled_2d) {
  ast::type::F32Type f32;
  ast::type::I32Type i32;
  ast::type::VectorType vec2(&f32, 2);
  ast::type::SampledTextureType s(ast::type::TextureDimension::k2d, &f32);

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  Builder b(&mod);

  b.push_function(Function{});

  ast::Variable tex("texture", ast::StorageClass::kNone, &s);
  td.RegisterVariableForTesting(&tex);
  ASSERT_TRUE(b.GenerateGlobalVariable(&tex)) << b.error();

  ast::ExpressionList vals;
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));

  ast::ExpressionList call_params;
  call_params.push_back(std::make_unique<ast::IdentifierExpression>("texture"));
  call_params.push_back(
      std::make_unique<ast::TypeConstructorExpression>(&vec2, std::move(vals)));
  call_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 2)));

  ast::CallExpression expr(
      std::make_unique<ast::IdentifierExpression>("textureLoad"),
      std::move(call_params));

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

TEST_F(BuilderTest, Call_TextureLoad_Multisampled_2d) {
  ast::type::F32Type f32;
  ast::type::I32Type i32;
  ast::type::VectorType vec2(&f32, 2);
  ast::type::MultisampledTextureType s(ast::type::TextureDimension::k2d, &f32);

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  Builder b(&mod);

  b.push_function(Function{});

  ast::Variable tex("texture", ast::StorageClass::kNone, &s);
  td.RegisterVariableForTesting(&tex);
  ASSERT_TRUE(b.GenerateGlobalVariable(&tex)) << b.error();

  ast::ExpressionList vals;
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));

  ast::ExpressionList call_params;
  call_params.push_back(std::make_unique<ast::IdentifierExpression>("texture"));
  call_params.push_back(
      std::make_unique<ast::TypeConstructorExpression>(&vec2, std::move(vals)));
  call_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 2)));

  ast::CallExpression expr(
      std::make_unique<ast::IdentifierExpression>("textureLoad"),
      std::move(call_params));

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

TEST_F(BuilderTest, Call_TextureSample_1d) {
  ast::type::F32Type f32;

  ast::type::SamplerType s(ast::type::SamplerKind::kSampler);
  ast::type::SampledTextureType t(ast::type::TextureDimension::k1d, &s);

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  Builder b(&mod);

  b.push_function(Function{});

  ast::Variable tex("texture", ast::StorageClass::kNone, &t);
  td.RegisterVariableForTesting(&tex);
  ASSERT_TRUE(b.GenerateGlobalVariable(&tex)) << b.error();

  ast::Variable sampler("sampler", ast::StorageClass::kNone, &s);
  td.RegisterVariableForTesting(&sampler);
  ASSERT_TRUE(b.GenerateGlobalVariable(&sampler)) << b.error();

  ast::ExpressionList call_params;
  call_params.push_back(std::make_unique<ast::IdentifierExpression>("texture"));
  call_params.push_back(std::make_unique<ast::IdentifierExpression>("sampler"));
  call_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0)));

  ast::CallExpression expr(
      std::make_unique<ast::IdentifierExpression>("textureSample"),
      std::move(call_params));

  EXPECT_TRUE(td.DetermineResultType(&expr)) << td.error();
  EXPECT_EQ(b.GenerateExpression(&expr), 7u) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()),
            R"(%4 = OpTypeSampler
%3 = OpTypeImage %4 1D 0 0 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%6 = OpTypePointer Private %4
%5 = OpVariable %6 Private
%8 = OpTypeVector %4 4
%11 = OpTypeFloat 32
%12 = OpConstant %11 1
%13 = OpTypeSampledImage %3
)");

  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%9 = OpLoad %3 %1
%10 = OpLoad %4 %5
%14 = OpSampledImage %13 %9 %10
%7 = OpImageSampleImplicitLod %8 %14 %12
)");
}

TEST_F(BuilderTest, Call_TextureSample_2d) {
  ast::type::F32Type f32;
  ast::type::VectorType vec2(&f32, 2);

  ast::type::SamplerType s(ast::type::SamplerKind::kSampler);
  ast::type::SampledTextureType t(ast::type::TextureDimension::k2d, &s);

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  Builder b(&mod);

  b.push_function(Function{});

  ast::Variable tex("texture", ast::StorageClass::kNone, &t);
  td.RegisterVariableForTesting(&tex);
  ASSERT_TRUE(b.GenerateGlobalVariable(&tex)) << b.error();

  ast::Variable sampler("sampler", ast::StorageClass::kNone, &s);
  td.RegisterVariableForTesting(&sampler);
  ASSERT_TRUE(b.GenerateGlobalVariable(&sampler)) << b.error();

  ast::ExpressionList vals;
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));

  ast::ExpressionList call_params;
  call_params.push_back(std::make_unique<ast::IdentifierExpression>("texture"));
  call_params.push_back(std::make_unique<ast::IdentifierExpression>("sampler"));
  call_params.push_back(
      std::make_unique<ast::TypeConstructorExpression>(&vec2, std::move(vals)));

  ast::CallExpression expr(
      std::make_unique<ast::IdentifierExpression>("textureSample"),
      std::move(call_params));

  EXPECT_TRUE(td.DetermineResultType(&expr)) << td.error();
  EXPECT_EQ(b.GenerateExpression(&expr), 7u) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()),
            R"(%4 = OpTypeSampler
%3 = OpTypeImage %4 2D 0 0 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%6 = OpTypePointer Private %4
%5 = OpVariable %6 Private
%8 = OpTypeVector %4 4
%12 = OpTypeFloat 32
%11 = OpTypeVector %12 2
%13 = OpConstant %12 1
%14 = OpConstant %12 2
%15 = OpConstantComposite %11 %13 %14
%16 = OpTypeSampledImage %3
)");

  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%9 = OpLoad %3 %1
%10 = OpLoad %4 %5
%17 = OpSampledImage %16 %9 %10
%7 = OpImageSampleImplicitLod %8 %17 %15
)");
}

TEST_F(BuilderTest, Call_TextureSampleLevel_1d) {
  ast::type::F32Type f32;

  ast::type::SamplerType s(ast::type::SamplerKind::kSampler);
  ast::type::SampledTextureType t(ast::type::TextureDimension::k1d, &f32);

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  Builder b(&mod);

  b.push_function(Function{});

  ast::Variable tex("texture", ast::StorageClass::kNone, &t);
  td.RegisterVariableForTesting(&tex);
  ASSERT_TRUE(b.GenerateGlobalVariable(&tex)) << b.error();

  ast::Variable sampler("sampler", ast::StorageClass::kNone, &s);
  td.RegisterVariableForTesting(&sampler);
  ASSERT_TRUE(b.GenerateGlobalVariable(&sampler)) << b.error();

  ast::ExpressionList call_params;
  call_params.push_back(std::make_unique<ast::IdentifierExpression>("texture"));
  call_params.push_back(std::make_unique<ast::IdentifierExpression>("sampler"));
  call_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0)));
  call_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));

  ast::CallExpression expr(
      std::make_unique<ast::IdentifierExpression>("textureSampleLevel"),
      std::move(call_params));

  EXPECT_TRUE(td.DetermineResultType(&expr)) << td.error();
  EXPECT_EQ(b.GenerateExpression(&expr), 8u) << b.error();

  EXPECT_EQ(DumpInstructions(b.types()),
            R"(%4 = OpTypeFloat 32
%3 = OpTypeImage %4 1D 0 0 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
%9 = OpTypeVector %4 4
%12 = OpConstant %4 1
%13 = OpConstant %4 2
%14 = OpTypeSampledImage %3
)");

  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%10 = OpLoad %3 %1
%11 = OpLoad %7 %5
%15 = OpSampledImage %14 %10 %11
%8 = OpImageSampleExplicitLod %9 %15 %12 Lod %13
)");
}

TEST_F(BuilderTest, Call_TextureSampleLevel_2d) {
  ast::type::F32Type f32;
  ast::type::VectorType vec2(&f32, 2);

  ast::type::SamplerType s(ast::type::SamplerKind::kSampler);
  ast::type::SampledTextureType t(ast::type::TextureDimension::k2d, &f32);

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  Builder b(&mod);

  b.push_function(Function{});

  ast::Variable tex("texture", ast::StorageClass::kNone, &t);
  td.RegisterVariableForTesting(&tex);
  ASSERT_TRUE(b.GenerateGlobalVariable(&tex)) << b.error();

  ast::Variable sampler("sampler", ast::StorageClass::kNone, &s);
  td.RegisterVariableForTesting(&sampler);
  ASSERT_TRUE(b.GenerateGlobalVariable(&sampler)) << b.error();

  ast::ExpressionList vals;
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));

  ast::ExpressionList call_params;
  call_params.push_back(std::make_unique<ast::IdentifierExpression>("texture"));
  call_params.push_back(std::make_unique<ast::IdentifierExpression>("sampler"));
  call_params.push_back(
      std::make_unique<ast::TypeConstructorExpression>(&vec2, std::move(vals)));
  call_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));

  ast::CallExpression expr(
      std::make_unique<ast::IdentifierExpression>("textureSampleLevel"),
      std::move(call_params));

  EXPECT_TRUE(td.DetermineResultType(&expr)) << td.error();
  EXPECT_EQ(b.GenerateExpression(&expr), 8u) << b.error();

  EXPECT_EQ(DumpInstructions(b.types()),
            R"(%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 0 0 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
%9 = OpTypeVector %4 4
%12 = OpTypeVector %4 2
%13 = OpConstant %4 1
%14 = OpConstant %4 2
%15 = OpConstantComposite %12 %13 %14
%16 = OpTypeSampledImage %3
)");

  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%10 = OpLoad %3 %1
%11 = OpLoad %7 %5
%17 = OpSampledImage %16 %10 %11
%8 = OpImageSampleExplicitLod %9 %17 %15 Lod %14
)");
}

TEST_F(BuilderTest, Call_TextureSampleBias_1d) {
  ast::type::F32Type f32;

  ast::type::SamplerType s(ast::type::SamplerKind::kSampler);
  ast::type::SampledTextureType t(ast::type::TextureDimension::k1d, &f32);

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  Builder b(&mod);

  b.push_function(Function{});

  ast::Variable tex("texture", ast::StorageClass::kNone, &t);
  td.RegisterVariableForTesting(&tex);
  ASSERT_TRUE(b.GenerateGlobalVariable(&tex)) << b.error();

  ast::Variable sampler("sampler", ast::StorageClass::kNone, &s);
  td.RegisterVariableForTesting(&sampler);
  ASSERT_TRUE(b.GenerateGlobalVariable(&sampler)) << b.error();

  ast::ExpressionList call_params;
  call_params.push_back(std::make_unique<ast::IdentifierExpression>("texture"));
  call_params.push_back(std::make_unique<ast::IdentifierExpression>("sampler"));
  call_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0)));
  call_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));

  ast::CallExpression expr(
      std::make_unique<ast::IdentifierExpression>("textureSampleBias"),
      std::move(call_params));

  EXPECT_TRUE(td.DetermineResultType(&expr)) << td.error();
  EXPECT_EQ(b.GenerateExpression(&expr), 8u) << b.error();

  EXPECT_EQ(DumpInstructions(b.types()),
            R"(%4 = OpTypeFloat 32
%3 = OpTypeImage %4 1D 0 0 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
%9 = OpTypeVector %4 4
%12 = OpConstant %4 1
%13 = OpConstant %4 2
%14 = OpTypeSampledImage %3
)");

  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%10 = OpLoad %3 %1
%11 = OpLoad %7 %5
%15 = OpSampledImage %14 %10 %11
%8 = OpImageSampleImplicitLod %9 %15 %12 Bias %13
)");
}

TEST_F(BuilderTest, Call_TextureSampleBias_2d) {
  ast::type::F32Type f32;
  ast::type::VectorType vec2(&f32, 2);

  ast::type::SamplerType s(ast::type::SamplerKind::kSampler);
  ast::type::SampledTextureType t(ast::type::TextureDimension::k1d, &f32);

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  Builder b(&mod);

  b.push_function(Function{});

  ast::Variable tex("texture", ast::StorageClass::kNone, &t);
  td.RegisterVariableForTesting(&tex);
  ASSERT_TRUE(b.GenerateGlobalVariable(&tex)) << b.error();

  ast::Variable sampler("sampler", ast::StorageClass::kNone, &s);
  td.RegisterVariableForTesting(&sampler);
  ASSERT_TRUE(b.GenerateGlobalVariable(&sampler)) << b.error();

  ast::ExpressionList vals;
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));

  ast::ExpressionList call_params;
  call_params.push_back(std::make_unique<ast::IdentifierExpression>("texture"));
  call_params.push_back(std::make_unique<ast::IdentifierExpression>("sampler"));
  call_params.push_back(
      std::make_unique<ast::TypeConstructorExpression>(&vec2, std::move(vals)));
  call_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));

  ast::CallExpression expr(
      std::make_unique<ast::IdentifierExpression>("textureSampleBias"),
      std::move(call_params));

  EXPECT_TRUE(td.DetermineResultType(&expr)) << td.error();
  EXPECT_EQ(b.GenerateExpression(&expr), 8u) << b.error();

  EXPECT_EQ(DumpInstructions(b.types()),
            R"(%4 = OpTypeFloat 32
%3 = OpTypeImage %4 1D 0 0 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
%9 = OpTypeVector %4 4
%12 = OpTypeVector %4 2
%13 = OpConstant %4 1
%14 = OpConstant %4 2
%15 = OpConstantComposite %12 %13 %14
%16 = OpTypeSampledImage %3
)");

  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%10 = OpLoad %3 %1
%11 = OpLoad %7 %5
%17 = OpSampledImage %16 %10 %11
%8 = OpImageSampleImplicitLod %9 %17 %15 Bias %14
)");
}

TEST_F(BuilderTest, Call_TextureSampleCompare) {
  ast::type::F32Type f32;
  ast::type::VectorType vec2(&f32, 2);

  ast::type::SamplerType s(ast::type::SamplerKind::kComparisonSampler);
  ast::type::DepthTextureType t(ast::type::TextureDimension::k2d);

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  Builder b(&mod);

  b.push_function(Function{});

  ast::Variable tex("texture", ast::StorageClass::kNone, &t);
  td.RegisterVariableForTesting(&tex);
  ASSERT_TRUE(b.GenerateGlobalVariable(&tex)) << b.error();

  ast::Variable sampler("sampler", ast::StorageClass::kNone, &s);
  td.RegisterVariableForTesting(&sampler);
  ASSERT_TRUE(b.GenerateGlobalVariable(&sampler)) << b.error();

  ast::ExpressionList vals;
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));

  ast::ExpressionList call_params;
  call_params.push_back(std::make_unique<ast::IdentifierExpression>("texture"));
  call_params.push_back(std::make_unique<ast::IdentifierExpression>("sampler"));
  call_params.push_back(
      std::make_unique<ast::TypeConstructorExpression>(&vec2, std::move(vals)));
  call_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));

  ast::CallExpression expr(
      std::make_unique<ast::IdentifierExpression>("textureSampleCompare"),
      std::move(call_params));

  EXPECT_TRUE(td.DetermineResultType(&expr)) << td.error();
  EXPECT_EQ(b.GenerateExpression(&expr), 8u) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()),
            R"(%4 = OpTypeFloat 32
%3 = OpTypeImage %4 2D 1 0 0 1 Unknown
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%7 = OpTypeSampler
%6 = OpTypePointer Private %7
%5 = OpVariable %6 Private
%11 = OpTypeVector %4 2
%12 = OpConstant %4 1
%13 = OpConstant %4 2
%14 = OpConstantComposite %11 %12 %13
%15 = OpTypeSampledImage %3
%17 = OpConstant %4 0
)");

  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%9 = OpLoad %3 %1
%10 = OpLoad %7 %5
%16 = OpSampledImage %15 %9 %10
%8 = OpImageSampleDrefExplicitLod %4 %16 %14 %13 Lod %17
)");
}

// This tests that we do not push OpTypeSampledImage and float_0 type twice.
TEST_F(BuilderTest, Call_TextureSampleCompare_Twice) {
  ast::type::F32Type f32;
  ast::type::VectorType vec2(&f32, 2);

  ast::type::SamplerType s(ast::type::SamplerKind::kComparisonSampler);
  ast::type::DepthTextureType t(ast::type::TextureDimension::k2d);

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  Builder b(&mod);

  b.push_function(Function{});

  ast::Variable tex("texture", ast::StorageClass::kNone, &t);
  td.RegisterVariableForTesting(&tex);
  ASSERT_TRUE(b.GenerateGlobalVariable(&tex)) << b.error();

  ast::Variable sampler("sampler", ast::StorageClass::kNone, &s);
  td.RegisterVariableForTesting(&sampler);
  ASSERT_TRUE(b.GenerateGlobalVariable(&sampler)) << b.error();

  ast::ExpressionList vals;
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));

  ast::ExpressionList call_params;
  call_params.push_back(std::make_unique<ast::IdentifierExpression>("texture"));
  call_params.push_back(std::make_unique<ast::IdentifierExpression>("sampler"));
  call_params.push_back(
      std::make_unique<ast::TypeConstructorExpression>(&vec2, std::move(vals)));
  call_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));

  ast::CallExpression expr1(
      std::make_unique<ast::IdentifierExpression>("textureSampleCompare"),
      std::move(call_params));

  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));

  call_params.push_back(std::make_unique<ast::IdentifierExpression>("texture"));
  call_params.push_back(std::make_unique<ast::IdentifierExpression>("sampler"));
  call_params.push_back(
      std::make_unique<ast::TypeConstructorExpression>(&vec2, std::move(vals)));
  call_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0)));

  ast::CallExpression expr2(
      std::make_unique<ast::IdentifierExpression>("textureSampleCompare"),
      std::move(call_params));

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
%11 = OpTypeVector %4 2
%12 = OpConstant %4 1
%13 = OpConstant %4 2
%14 = OpConstantComposite %11 %12 %13
%15 = OpTypeSampledImage %3
%17 = OpConstant %4 0
)");

  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%9 = OpLoad %3 %1
%10 = OpLoad %7 %5
%16 = OpSampledImage %15 %9 %10
%8 = OpImageSampleDrefExplicitLod %4 %16 %14 %13 Lod %17
%19 = OpLoad %3 %1
%20 = OpLoad %7 %5
%21 = OpSampledImage %15 %19 %20
%18 = OpImageSampleDrefExplicitLod %4 %21 %14 %13 Lod %17
)");
}

TEST_F(BuilderTest, Call_GLSLMethod_WithLoad) {
  ast::type::F32Type f32;
  ast::type::VoidType void_type;

  auto var = std::make_unique<ast::Variable>("ident",
                                             ast::StorageClass::kPrivate, &f32);

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::IdentifierExpression>("ident"));

  ast::CallExpression expr(std::make_unique<ast::IdentifierExpression>("round"),
                           std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  td.RegisterVariableForTesting(var.get());

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  ast::Function func("a_func", {}, &void_type);

  Builder b(&mod);
  ASSERT_TRUE(b.GenerateGlobalVariable(var.get())) << b.error();
  ASSERT_TRUE(b.GenerateFunction(&func)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(&expr), 9u) << b.error();
  EXPECT_EQ(DumpBuilder(b), R"(%10 = OpExtInstImport "GLSL.std.450"
OpName %1 "ident"
OpName %7 "a_func"
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
OpFunctionEnd
)");
}

using Intrinsic_Builtin_SingleParam_Float_Test =
    testing::TestWithParam<IntrinsicData>;
TEST_P(Intrinsic_Builtin_SingleParam_Float_Test, Call_Scalar) {
  auto param = GetParam();

  ast::type::F32Type f32;
  ast::type::VoidType void_type;

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));

  ast::CallExpression expr(
      std::make_unique<ast::IdentifierExpression>(param.name),
      std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  ast::Function func("a_func", {}, &void_type);

  Builder b(&mod);
  ASSERT_TRUE(b.GenerateFunction(&func)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(&expr), 5u) << b.error();
  EXPECT_EQ(DumpBuilder(b), R"(%7 = OpExtInstImport "GLSL.std.450"
OpName %3 "a_func"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = OpTypeFloat 32
%8 = OpConstant %6 1
%3 = OpFunction %2 None %1
%4 = OpLabel
%5 = OpExtInst %6 %7 )" + param.op +
                                R"( %8
OpFunctionEnd
)");
}

TEST_P(Intrinsic_Builtin_SingleParam_Float_Test, Call_Vector) {
  auto param = GetParam();

  ast::type::F32Type f32;
  ast::type::VectorType vec(&f32, 2);
  ast::type::VoidType void_type;

  ast::ExpressionList vals;
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));

  ast::ExpressionList params;
  params.push_back(
      std::make_unique<ast::TypeConstructorExpression>(&vec, std::move(vals)));

  ast::CallExpression expr(
      std::make_unique<ast::IdentifierExpression>(param.name),
      std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  ast::Function func("a_func", {}, &void_type);

  Builder b(&mod);
  ASSERT_TRUE(b.GenerateFunction(&func)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(&expr), 5u) << b.error();
  EXPECT_EQ(DumpBuilder(b), R"(%8 = OpExtInstImport "GLSL.std.450"
OpName %3 "a_func"
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
OpFunctionEnd
)");
}
INSTANTIATE_TEST_SUITE_P(BuilderTest,
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

TEST_F(BuilderTest, Call_Length_Scalar) {
  ast::type::F32Type f32;
  ast::type::VoidType void_type;

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));

  ast::CallExpression expr(
      std::make_unique<ast::IdentifierExpression>("length"), std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  ast::Function func("a_func", {}, &void_type);

  Builder b(&mod);
  ASSERT_TRUE(b.GenerateFunction(&func)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(&expr), 5u) << b.error();
  EXPECT_EQ(DumpBuilder(b), R"(%7 = OpExtInstImport "GLSL.std.450"
OpName %3 "a_func"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = OpTypeFloat 32
%8 = OpConstant %6 1
%3 = OpFunction %2 None %1
%4 = OpLabel
%5 = OpExtInst %6 %7 Length %8
OpFunctionEnd
)");
}

TEST_F(BuilderTest, Call_Length_Vector) {
  ast::type::F32Type f32;
  ast::type::VectorType vec(&f32, 2);
  ast::type::VoidType void_type;

  ast::ExpressionList vals;
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));

  ast::ExpressionList params;
  params.push_back(
      std::make_unique<ast::TypeConstructorExpression>(&vec, std::move(vals)));

  ast::CallExpression expr(
      std::make_unique<ast::IdentifierExpression>("length"), std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  ast::Function func("a_func", {}, &void_type);

  Builder b(&mod);
  ASSERT_TRUE(b.GenerateFunction(&func)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(&expr), 5u) << b.error();
  EXPECT_EQ(DumpBuilder(b), R"(%7 = OpExtInstImport "GLSL.std.450"
OpName %3 "a_func"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = OpTypeFloat 32
%8 = OpTypeVector %6 2
%9 = OpConstant %6 1
%10 = OpConstantComposite %8 %9 %9
%3 = OpFunction %2 None %1
%4 = OpLabel
%5 = OpExtInst %6 %7 Length %10
OpFunctionEnd
)");
}

TEST_F(BuilderTest, Call_Normalize) {
  ast::type::F32Type f32;
  ast::type::VectorType vec(&f32, 2);
  ast::type::VoidType void_type;

  ast::ExpressionList vals;
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));

  ast::ExpressionList params;
  params.push_back(
      std::make_unique<ast::TypeConstructorExpression>(&vec, std::move(vals)));

  ast::CallExpression expr(
      std::make_unique<ast::IdentifierExpression>("normalize"),
      std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  ast::Function func("a_func", {}, &void_type);

  Builder b(&mod);
  ASSERT_TRUE(b.GenerateFunction(&func)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(&expr), 5u) << b.error();
  EXPECT_EQ(DumpBuilder(b), R"(%8 = OpExtInstImport "GLSL.std.450"
OpName %3 "a_func"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%7 = OpTypeFloat 32
%6 = OpTypeVector %7 2
%9 = OpConstant %7 1
%10 = OpConstantComposite %6 %9 %9
%3 = OpFunction %2 None %1
%4 = OpLabel
%5 = OpExtInst %6 %8 Normalize %10
OpFunctionEnd
)");
}

using Intrinsic_Builtin_DualParam_Float_Test =
    testing::TestWithParam<IntrinsicData>;
TEST_P(Intrinsic_Builtin_DualParam_Float_Test, Call_Scalar) {
  auto param = GetParam();

  ast::type::F32Type f32;
  ast::type::VoidType void_type;

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));

  ast::CallExpression expr(
      std::make_unique<ast::IdentifierExpression>(param.name),
      std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  ast::Function func("a_func", {}, &void_type);

  Builder b(&mod);
  ASSERT_TRUE(b.GenerateFunction(&func)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(&expr), 5u) << b.error();
  EXPECT_EQ(DumpBuilder(b), R"(%7 = OpExtInstImport "GLSL.std.450"
OpName %3 "a_func"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = OpTypeFloat 32
%8 = OpConstant %6 1
%3 = OpFunction %2 None %1
%4 = OpLabel
%5 = OpExtInst %6 %7 )" + param.op +
                                R"( %8 %8
OpFunctionEnd
)");
}

TEST_P(Intrinsic_Builtin_DualParam_Float_Test, Call_Vector) {
  auto param = GetParam();

  ast::type::F32Type f32;
  ast::type::VectorType vec(&f32, 2);
  ast::type::VoidType void_type;

  ast::ExpressionList vals_1;
  vals_1.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));
  vals_1.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));

  ast::ExpressionList vals_2;
  vals_2.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));
  vals_2.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec, std::move(vals_1)));
  params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec, std::move(vals_2)));

  ast::CallExpression expr(
      std::make_unique<ast::IdentifierExpression>(param.name),
      std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  ast::Function func("a_func", {}, &void_type);

  Builder b(&mod);
  ASSERT_TRUE(b.GenerateFunction(&func)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(&expr), 5u) << b.error();
  EXPECT_EQ(DumpBuilder(b), R"(%8 = OpExtInstImport "GLSL.std.450"
OpName %3 "a_func"
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
OpFunctionEnd
)");
}
INSTANTIATE_TEST_SUITE_P(BuilderTest,
                         Intrinsic_Builtin_DualParam_Float_Test,
                         testing::Values(IntrinsicData{"atan2", "Atan2"},
                                         IntrinsicData{"max", "NMax"},
                                         IntrinsicData{"min", "NMin"},
                                         IntrinsicData{"pow", "Pow"},
                                         IntrinsicData{"reflect", "Reflect"},
                                         IntrinsicData{"step", "Step"}));

TEST_F(BuilderTest, Call_Distance_Scalar) {
  ast::type::F32Type f32;
  ast::type::VoidType void_type;

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));

  ast::CallExpression expr(
      std::make_unique<ast::IdentifierExpression>("distance"),
      std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  ast::Function func("a_func", {}, &void_type);

  Builder b(&mod);
  ASSERT_TRUE(b.GenerateFunction(&func)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(&expr), 5u) << b.error();
  EXPECT_EQ(DumpBuilder(b), R"(%7 = OpExtInstImport "GLSL.std.450"
OpName %3 "a_func"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = OpTypeFloat 32
%8 = OpConstant %6 1
%3 = OpFunction %2 None %1
%4 = OpLabel
%5 = OpExtInst %6 %7 Distance %8 %8
OpFunctionEnd
)");
}

TEST_F(BuilderTest, Call_Distance_Vector) {
  ast::type::F32Type f32;
  ast::type::VectorType vec(&f32, 2);
  ast::type::VoidType void_type;

  ast::ExpressionList vals_1;
  vals_1.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));
  vals_1.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));

  ast::ExpressionList vals_2;
  vals_2.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));
  vals_2.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec, std::move(vals_1)));
  params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec, std::move(vals_2)));

  ast::CallExpression expr(
      std::make_unique<ast::IdentifierExpression>("distance"),
      std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  ast::Function func("a_func", {}, &void_type);

  Builder b(&mod);
  ASSERT_TRUE(b.GenerateFunction(&func)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(&expr), 5u) << b.error();
  EXPECT_EQ(DumpBuilder(b), R"(%7 = OpExtInstImport "GLSL.std.450"
OpName %3 "a_func"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = OpTypeFloat 32
%8 = OpTypeVector %6 2
%9 = OpConstant %6 1
%10 = OpConstantComposite %8 %9 %9
%3 = OpFunction %2 None %1
%4 = OpLabel
%5 = OpExtInst %6 %7 Distance %10 %10
OpFunctionEnd
)");
}

TEST_F(BuilderTest, Call_Cross) {
  ast::type::F32Type f32;
  ast::type::VectorType vec(&f32, 3);
  ast::type::VoidType void_type;

  ast::ExpressionList vals_1;
  vals_1.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));
  vals_1.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));
  vals_1.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));

  ast::ExpressionList vals_2;
  vals_2.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));
  vals_2.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));
  vals_2.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec, std::move(vals_1)));
  params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec, std::move(vals_2)));

  ast::CallExpression expr(std::make_unique<ast::IdentifierExpression>("cross"),
                           std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  ast::Function func("a_func", {}, &void_type);

  Builder b(&mod);
  ASSERT_TRUE(b.GenerateFunction(&func)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(&expr), 5u) << b.error();
  EXPECT_EQ(DumpBuilder(b), R"(%8 = OpExtInstImport "GLSL.std.450"
OpName %3 "a_func"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%7 = OpTypeFloat 32
%6 = OpTypeVector %7 3
%9 = OpConstant %7 1
%10 = OpConstantComposite %6 %9 %9 %9
%3 = OpFunction %2 None %1
%4 = OpLabel
%5 = OpExtInst %6 %8 Cross %10 %10
OpFunctionEnd
)");
}

using Intrinsic_Builtin_ThreeParam_Float_Test =
    testing::TestWithParam<IntrinsicData>;
TEST_P(Intrinsic_Builtin_ThreeParam_Float_Test, Call_Scalar) {
  auto param = GetParam();

  ast::type::F32Type f32;
  ast::type::VoidType void_type;

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));

  ast::CallExpression expr(
      std::make_unique<ast::IdentifierExpression>(param.name),
      std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  ast::Function func("a_func", {}, &void_type);

  Builder b(&mod);
  ASSERT_TRUE(b.GenerateFunction(&func)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(&expr), 5u) << b.error();
  EXPECT_EQ(DumpBuilder(b), R"(%7 = OpExtInstImport "GLSL.std.450"
OpName %3 "a_func"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = OpTypeFloat 32
%8 = OpConstant %6 1
%3 = OpFunction %2 None %1
%4 = OpLabel
%5 = OpExtInst %6 %7 )" + param.op +
                                R"( %8 %8 %8
OpFunctionEnd
)");
}

TEST_P(Intrinsic_Builtin_ThreeParam_Float_Test, Call_Vector) {
  auto param = GetParam();

  ast::type::F32Type f32;
  ast::type::VectorType vec(&f32, 2);
  ast::type::VoidType void_type;

  ast::ExpressionList vals_1;
  vals_1.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));
  vals_1.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));

  ast::ExpressionList vals_2;
  vals_2.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));
  vals_2.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));

  ast::ExpressionList vals_3;
  vals_3.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));
  vals_3.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec, std::move(vals_1)));
  params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec, std::move(vals_2)));
  params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec, std::move(vals_3)));

  ast::CallExpression expr(
      std::make_unique<ast::IdentifierExpression>(param.name),
      std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  ast::Function func("a_func", {}, &void_type);

  Builder b(&mod);
  ASSERT_TRUE(b.GenerateFunction(&func)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(&expr), 5u) << b.error();
  EXPECT_EQ(DumpBuilder(b), R"(%8 = OpExtInstImport "GLSL.std.450"
OpName %3 "a_func"
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
OpFunctionEnd
)");
}
INSTANTIATE_TEST_SUITE_P(
    BuilderTest,
    Intrinsic_Builtin_ThreeParam_Float_Test,
    testing::Values(IntrinsicData{"clamp", "NClamp"},
                    IntrinsicData{"faceForward", "FaceForward"},
                    IntrinsicData{"fma", "Fma"},
                    IntrinsicData{"mix", "FMix"},

                    IntrinsicData{"smoothStep", "SmoothStep"}));

using Intrinsic_Builtin_SingleParam_Sint_Test =
    testing::TestWithParam<IntrinsicData>;
TEST_P(Intrinsic_Builtin_SingleParam_Sint_Test, Call_Scalar) {
  auto param = GetParam();

  ast::type::I32Type i32;
  ast::type::VoidType void_type;

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 1)));

  ast::CallExpression expr(
      std::make_unique<ast::IdentifierExpression>(param.name),
      std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  ast::Function func("a_func", {}, &void_type);

  Builder b(&mod);
  ASSERT_TRUE(b.GenerateFunction(&func)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(&expr), 5u) << b.error();
  EXPECT_EQ(DumpBuilder(b), R"(%7 = OpExtInstImport "GLSL.std.450"
OpName %3 "a_func"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = OpTypeInt 32 1
%8 = OpConstant %6 1
%3 = OpFunction %2 None %1
%4 = OpLabel
%5 = OpExtInst %6 %7 )" + param.op +
                                R"( %8
OpFunctionEnd
)");
}

TEST_P(Intrinsic_Builtin_SingleParam_Sint_Test, Call_Vector) {
  auto param = GetParam();

  ast::type::I32Type i32;
  ast::type::VectorType vec(&i32, 2);
  ast::type::VoidType void_type;

  ast::ExpressionList vals;
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 1)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 1)));

  ast::ExpressionList params;
  params.push_back(
      std::make_unique<ast::TypeConstructorExpression>(&vec, std::move(vals)));

  ast::CallExpression expr(
      std::make_unique<ast::IdentifierExpression>(param.name),
      std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  ast::Function func("a_func", {}, &void_type);

  Builder b(&mod);
  ASSERT_TRUE(b.GenerateFunction(&func)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(&expr), 5u) << b.error();
  EXPECT_EQ(DumpBuilder(b), R"(%8 = OpExtInstImport "GLSL.std.450"
OpName %3 "a_func"
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
OpFunctionEnd
)");
}
INSTANTIATE_TEST_SUITE_P(BuilderTest,
                         Intrinsic_Builtin_SingleParam_Sint_Test,
                         testing::Values(IntrinsicData{"abs", "SAbs"}));

using Intrinsic_Builtin_SingleParam_Uint_Test =
    testing::TestWithParam<IntrinsicData>;
TEST_P(Intrinsic_Builtin_SingleParam_Uint_Test, Call_Scalar) {
  auto param = GetParam();

  ast::type::U32Type u32;
  ast::type::VoidType void_type;

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::UintLiteral>(&u32, 1)));

  ast::CallExpression expr(
      std::make_unique<ast::IdentifierExpression>(param.name),
      std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  ast::Function func("a_func", {}, &void_type);

  Builder b(&mod);
  ASSERT_TRUE(b.GenerateFunction(&func)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(&expr), 5u) << b.error();
  EXPECT_EQ(DumpBuilder(b), R"(%7 = OpExtInstImport "GLSL.std.450"
OpName %3 "a_func"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = OpTypeInt 32 0
%8 = OpConstant %6 1
%3 = OpFunction %2 None %1
%4 = OpLabel
%5 = OpExtInst %6 %7 )" + param.op +
                                R"( %8
OpFunctionEnd
)");
}

TEST_P(Intrinsic_Builtin_SingleParam_Uint_Test, Call_Vector) {
  auto param = GetParam();

  ast::type::U32Type u32;
  ast::type::VectorType vec(&u32, 2);
  ast::type::VoidType void_type;

  ast::ExpressionList vals;
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::UintLiteral>(&u32, 1)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::UintLiteral>(&u32, 1)));

  ast::ExpressionList params;
  params.push_back(
      std::make_unique<ast::TypeConstructorExpression>(&vec, std::move(vals)));

  ast::CallExpression expr(
      std::make_unique<ast::IdentifierExpression>(param.name),
      std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  ast::Function func("a_func", {}, &void_type);

  Builder b(&mod);
  ASSERT_TRUE(b.GenerateFunction(&func)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(&expr), 5u) << b.error();
  EXPECT_EQ(DumpBuilder(b), R"(%8 = OpExtInstImport "GLSL.std.450"
OpName %3 "a_func"
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
OpFunctionEnd
)");
}
INSTANTIATE_TEST_SUITE_P(BuilderTest,
                         Intrinsic_Builtin_SingleParam_Uint_Test,
                         testing::Values(IntrinsicData{"abs", "SAbs"}));

using Intrinsic_Builtin_DualParam_SInt_Test =
    testing::TestWithParam<IntrinsicData>;
TEST_P(Intrinsic_Builtin_DualParam_SInt_Test, Call_Scalar) {
  auto param = GetParam();

  ast::type::I32Type i32;
  ast::type::VoidType void_type;

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 1)));
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 1)));

  ast::CallExpression expr(
      std::make_unique<ast::IdentifierExpression>(param.name),
      std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  ast::Function func("a_func", {}, &void_type);

  Builder b(&mod);
  ASSERT_TRUE(b.GenerateFunction(&func)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(&expr), 5u) << b.error();
  EXPECT_EQ(DumpBuilder(b), R"(%7 = OpExtInstImport "GLSL.std.450"
OpName %3 "a_func"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = OpTypeInt 32 1
%8 = OpConstant %6 1
%3 = OpFunction %2 None %1
%4 = OpLabel
%5 = OpExtInst %6 %7 )" + param.op +
                                R"( %8 %8
OpFunctionEnd
)");
}

TEST_P(Intrinsic_Builtin_DualParam_SInt_Test, Call_Vector) {
  auto param = GetParam();

  ast::type::I32Type i32;
  ast::type::VectorType vec(&i32, 2);
  ast::type::VoidType void_type;

  ast::ExpressionList vals_1;
  vals_1.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 1)));
  vals_1.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 1)));

  ast::ExpressionList vals_2;
  vals_2.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 1)));
  vals_2.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 1)));

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec, std::move(vals_1)));
  params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec, std::move(vals_2)));

  ast::CallExpression expr(
      std::make_unique<ast::IdentifierExpression>(param.name),
      std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  ast::Function func("a_func", {}, &void_type);

  Builder b(&mod);
  ASSERT_TRUE(b.GenerateFunction(&func)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(&expr), 5u) << b.error();
  EXPECT_EQ(DumpBuilder(b), R"(%8 = OpExtInstImport "GLSL.std.450"
OpName %3 "a_func"
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
OpFunctionEnd
)");
}
INSTANTIATE_TEST_SUITE_P(BuilderTest,
                         Intrinsic_Builtin_DualParam_SInt_Test,
                         testing::Values(IntrinsicData{"max", "SMax"},
                                         IntrinsicData{"min", "SMin"}));

using Intrinsic_Builtin_DualParam_UInt_Test =
    testing::TestWithParam<IntrinsicData>;
TEST_P(Intrinsic_Builtin_DualParam_UInt_Test, Call_Scalar) {
  auto param = GetParam();

  ast::type::U32Type u32;
  ast::type::VoidType void_type;

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::UintLiteral>(&u32, 1)));
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::UintLiteral>(&u32, 1)));

  ast::CallExpression expr(
      std::make_unique<ast::IdentifierExpression>(param.name),
      std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  ast::Function func("a_func", {}, &void_type);

  Builder b(&mod);
  ASSERT_TRUE(b.GenerateFunction(&func)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(&expr), 5u) << b.error();
  EXPECT_EQ(DumpBuilder(b), R"(%7 = OpExtInstImport "GLSL.std.450"
OpName %3 "a_func"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = OpTypeInt 32 0
%8 = OpConstant %6 1
%3 = OpFunction %2 None %1
%4 = OpLabel
%5 = OpExtInst %6 %7 )" + param.op +
                                R"( %8 %8
OpFunctionEnd
)");
}

TEST_P(Intrinsic_Builtin_DualParam_UInt_Test, Call_Vector) {
  auto param = GetParam();

  ast::type::U32Type u32;
  ast::type::VectorType vec(&u32, 2);
  ast::type::VoidType void_type;

  ast::ExpressionList vals_1;
  vals_1.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::UintLiteral>(&u32, 1)));
  vals_1.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::UintLiteral>(&u32, 1)));

  ast::ExpressionList vals_2;
  vals_2.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::UintLiteral>(&u32, 1)));
  vals_2.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::UintLiteral>(&u32, 1)));

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec, std::move(vals_1)));
  params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec, std::move(vals_2)));

  ast::CallExpression expr(
      std::make_unique<ast::IdentifierExpression>(param.name),
      std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  ast::Function func("a_func", {}, &void_type);

  Builder b(&mod);
  ASSERT_TRUE(b.GenerateFunction(&func)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(&expr), 5u) << b.error();
  EXPECT_EQ(DumpBuilder(b), R"(%8 = OpExtInstImport "GLSL.std.450"
OpName %3 "a_func"
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
OpFunctionEnd
)");
}
INSTANTIATE_TEST_SUITE_P(BuilderTest,
                         Intrinsic_Builtin_DualParam_UInt_Test,
                         testing::Values(IntrinsicData{"max", "UMax"},
                                         IntrinsicData{"min", "UMin"}));

using Intrinsic_Builtin_ThreeParam_Sint_Test =
    testing::TestWithParam<IntrinsicData>;
TEST_P(Intrinsic_Builtin_ThreeParam_Sint_Test, Call_Scalar) {
  auto param = GetParam();

  ast::type::I32Type i32;
  ast::type::VoidType void_type;

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 1)));
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 1)));
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 1)));

  ast::CallExpression expr(
      std::make_unique<ast::IdentifierExpression>(param.name),
      std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  ast::Function func("a_func", {}, &void_type);

  Builder b(&mod);
  ASSERT_TRUE(b.GenerateFunction(&func)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(&expr), 5u) << b.error();
  EXPECT_EQ(DumpBuilder(b), R"(%7 = OpExtInstImport "GLSL.std.450"
OpName %3 "a_func"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = OpTypeInt 32 1
%8 = OpConstant %6 1
%3 = OpFunction %2 None %1
%4 = OpLabel
%5 = OpExtInst %6 %7 )" + param.op +
                                R"( %8 %8 %8
OpFunctionEnd
)");
}

TEST_P(Intrinsic_Builtin_ThreeParam_Sint_Test, Call_Vector) {
  auto param = GetParam();

  ast::type::I32Type i32;
  ast::type::VectorType vec(&i32, 2);
  ast::type::VoidType void_type;

  ast::ExpressionList vals_1;
  vals_1.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 1)));
  vals_1.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 1)));

  ast::ExpressionList vals_2;
  vals_2.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 1)));
  vals_2.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 1)));

  ast::ExpressionList vals_3;
  vals_3.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 1)));
  vals_3.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 1)));

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec, std::move(vals_1)));
  params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec, std::move(vals_2)));
  params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec, std::move(vals_3)));

  ast::CallExpression expr(
      std::make_unique<ast::IdentifierExpression>(param.name),
      std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  ast::Function func("a_func", {}, &void_type);

  Builder b(&mod);
  ASSERT_TRUE(b.GenerateFunction(&func)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(&expr), 5u) << b.error();
  EXPECT_EQ(DumpBuilder(b), R"(%8 = OpExtInstImport "GLSL.std.450"
OpName %3 "a_func"
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
OpFunctionEnd
)");
}
INSTANTIATE_TEST_SUITE_P(BuilderTest,
                         Intrinsic_Builtin_ThreeParam_Sint_Test,
                         testing::Values(IntrinsicData{"clamp", "SClamp"}));

using Intrinsic_Builtin_ThreeParam_Uint_Test =
    testing::TestWithParam<IntrinsicData>;
TEST_P(Intrinsic_Builtin_ThreeParam_Uint_Test, Call_Scalar) {
  auto param = GetParam();

  ast::type::U32Type u32;
  ast::type::VoidType void_type;

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::UintLiteral>(&u32, 1)));
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::UintLiteral>(&u32, 1)));
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::UintLiteral>(&u32, 1)));

  ast::CallExpression expr(
      std::make_unique<ast::IdentifierExpression>(param.name),
      std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  ast::Function func("a_func", {}, &void_type);

  Builder b(&mod);
  ASSERT_TRUE(b.GenerateFunction(&func)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(&expr), 5u) << b.error();
  EXPECT_EQ(DumpBuilder(b), R"(%7 = OpExtInstImport "GLSL.std.450"
OpName %3 "a_func"
%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = OpTypeInt 32 0
%8 = OpConstant %6 1
%3 = OpFunction %2 None %1
%4 = OpLabel
%5 = OpExtInst %6 %7 )" + param.op +
                                R"( %8 %8 %8
OpFunctionEnd
)");
}

TEST_P(Intrinsic_Builtin_ThreeParam_Uint_Test, Call_Vector) {
  auto param = GetParam();

  ast::type::U32Type u32;
  ast::type::VectorType vec(&u32, 2);
  ast::type::VoidType void_type;

  ast::ExpressionList vals_1;
  vals_1.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::UintLiteral>(&u32, 1)));
  vals_1.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::UintLiteral>(&u32, 1)));

  ast::ExpressionList vals_2;
  vals_2.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::UintLiteral>(&u32, 1)));
  vals_2.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::UintLiteral>(&u32, 1)));

  ast::ExpressionList vals_3;
  vals_3.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::UintLiteral>(&u32, 1)));
  vals_3.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::UintLiteral>(&u32, 1)));

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec, std::move(vals_1)));
  params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec, std::move(vals_2)));
  params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec, std::move(vals_3)));

  ast::CallExpression expr(
      std::make_unique<ast::IdentifierExpression>(param.name),
      std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  ast::Function func("a_func", {}, &void_type);

  Builder b(&mod);
  ASSERT_TRUE(b.GenerateFunction(&func)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(&expr), 5u) << b.error();
  EXPECT_EQ(DumpBuilder(b), R"(%8 = OpExtInstImport "GLSL.std.450"
OpName %3 "a_func"
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
OpFunctionEnd
)");
}
INSTANTIATE_TEST_SUITE_P(BuilderTest,
                         Intrinsic_Builtin_ThreeParam_Uint_Test,
                         testing::Values(IntrinsicData{"clamp", "UClamp"}));

TEST_F(BuilderTest, Call_Determinant) {
  ast::type::F32Type f32;
  ast::type::VoidType void_type;
  ast::type::MatrixType mat(&f32, 3, 3);

  auto var =
      std::make_unique<ast::Variable>("var", ast::StorageClass::kPrivate, &mat);

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::IdentifierExpression>("var"));

  ast::CallExpression expr(
      std::make_unique<ast::IdentifierExpression>("determinant"),
      std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  td.RegisterVariableForTesting(var.get());

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  ast::Function func("a_func", {}, &void_type);

  Builder b(&mod);
  ASSERT_TRUE(b.GenerateFunction(&func)) << b.error();

  ASSERT_TRUE(b.GenerateGlobalVariable(var.get())) << b.error();
  EXPECT_EQ(b.GenerateCallExpression(&expr), 11u) << b.error();

  EXPECT_EQ(DumpBuilder(b), R"(%12 = OpExtInstImport "GLSL.std.450"
OpName %3 "a_func"
OpName %5 "var"
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
OpFunctionEnd
)");
}

TEST_F(BuilderTest, Call_ArrayLength) {
  ast::type::F32Type f32;
  ast::type::VoidType void_type;
  ast::type::ArrayType ary(&f32);

  ast::StructMemberDecorationList decos;
  ast::StructMemberList members;
  members.push_back(
      std::make_unique<ast::StructMember>("a", &ary, std::move(decos)));

  auto s = std::make_unique<ast::Struct>(std::move(members));
  ast::type::StructType s_type("my_struct", std::move(s));

  auto var = std::make_unique<ast::Variable>("b", ast::StorageClass::kPrivate,
                                             &s_type);

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::MemberAccessorExpression>(
      std::make_unique<ast::IdentifierExpression>("b"),
      std::make_unique<ast::IdentifierExpression>("a")));

  ast::CallExpression expr(
      std::make_unique<ast::IdentifierExpression>("arrayLength"),
      std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  td.RegisterVariableForTesting(var.get());

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  ast::Function func("a_func", {}, &void_type);

  Builder b(&mod);
  ASSERT_TRUE(b.GenerateFunction(&func)) << b.error();
  ASSERT_TRUE(b.GenerateGlobalVariable(var.get())) << b.error();
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

TEST_F(BuilderTest, Call_ArrayLength_OtherMembersInStruct) {
  ast::type::F32Type f32;
  ast::type::VoidType void_type;
  ast::type::ArrayType ary(&f32);

  ast::StructMemberDecorationList decos;
  ast::StructMemberList members;
  members.push_back(
      std::make_unique<ast::StructMember>("z", &f32, std::move(decos)));
  members.push_back(
      std::make_unique<ast::StructMember>("a", &ary, std::move(decos)));

  auto s = std::make_unique<ast::Struct>(std::move(members));
  ast::type::StructType s_type("my_struct", std::move(s));

  auto var = std::make_unique<ast::Variable>("b", ast::StorageClass::kPrivate,
                                             &s_type);

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::MemberAccessorExpression>(
      std::make_unique<ast::IdentifierExpression>("b"),
      std::make_unique<ast::IdentifierExpression>("a")));

  ast::CallExpression expr(
      std::make_unique<ast::IdentifierExpression>("arrayLength"),
      std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  td.RegisterVariableForTesting(var.get());

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  ast::Function func("a_func", {}, &void_type);

  Builder b(&mod);
  ASSERT_TRUE(b.GenerateFunction(&func)) << b.error();
  ASSERT_TRUE(b.GenerateGlobalVariable(var.get())) << b.error();
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
TEST_F(BuilderTest, DISABLED_Call_ArrayLength_Ptr) {
  ast::type::F32Type f32;
  ast::type::VoidType void_type;
  ast::type::ArrayType ary(&f32);
  ast::type::PointerType ptr(&ary, ast::StorageClass::kStorageBuffer);

  ast::StructMemberDecorationList decos;
  ast::StructMemberList members;
  members.push_back(
      std::make_unique<ast::StructMember>("z", &f32, std::move(decos)));
  members.push_back(
      std::make_unique<ast::StructMember>("a", &ary, std::move(decos)));

  auto s = std::make_unique<ast::Struct>(std::move(members));
  ast::type::StructType s_type("my_struct", std::move(s));

  auto var = std::make_unique<ast::Variable>("b", ast::StorageClass::kPrivate,
                                             &s_type);

  auto ptr_var = std::make_unique<ast::Variable>(
      "ptr_var", ast::StorageClass::kPrivate, &ptr);
  ptr_var->set_constructor(std::make_unique<ast::MemberAccessorExpression>(
      std::make_unique<ast::IdentifierExpression>("b"),
      std::make_unique<ast::IdentifierExpression>("a")));

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::IdentifierExpression>("ptr_var"));

  ast::CallExpression expr(
      std::make_unique<ast::IdentifierExpression>("arrayLength"),
      std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  td.RegisterVariableForTesting(var.get());
  td.RegisterVariableForTesting(ptr_var.get());

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  ast::Function func("a_func", {}, &void_type);

  Builder b(&mod);
  ASSERT_TRUE(b.GenerateFunction(&func)) << b.error();
  ASSERT_TRUE(b.GenerateGlobalVariable(var.get())) << b.error();
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
