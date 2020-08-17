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
#include "src/ast/binding_decoration.h"
#include "src/ast/builtin.h"
#include "src/ast/builtin_decoration.h"
#include "src/ast/decorated_variable.h"
#include "src/ast/float_literal.h"
#include "src/ast/location_decoration.h"
#include "src/ast/scalar_constructor_expression.h"
#include "src/ast/set_decoration.h"
#include "src/ast/storage_class.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/vector_type.h"
#include "src/ast/type_constructor_expression.h"
#include "src/ast/variable.h"
#include "src/ast/variable_decoration.h"
#include "src/context.h"
#include "src/type_determiner.h"
#include "src/writer/spirv/builder.h"
#include "src/writer/spirv/spv_dump.h"

namespace tint {
namespace writer {
namespace spirv {
namespace {

using BuilderTest = testing::Test;

TEST_F(BuilderTest, GlobalVar_NoStorageClass) {
  ast::type::F32Type f32;
  ast::Variable v("var", ast::StorageClass::kNone, &f32);

  ast::Module mod;
  Builder b(&mod);
  EXPECT_TRUE(b.GenerateGlobalVariable(&v)) << b.error();
  EXPECT_EQ(DumpInstructions(b.debug()), R"(OpName %1 "var"
)");
  EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeFloat 32
%2 = OpTypePointer Private %3
%4 = OpConstantNull %3
%1 = OpVariable %2 Private %4
)");
}

TEST_F(BuilderTest, GlobalVar_WithStorageClass) {
  ast::type::F32Type f32;
  ast::Variable v("var", ast::StorageClass::kOutput, &f32);

  ast::Module mod;
  Builder b(&mod);
  EXPECT_TRUE(b.GenerateGlobalVariable(&v)) << b.error();
  EXPECT_EQ(DumpInstructions(b.debug()), R"(OpName %1 "var"
)");
  EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeFloat 32
%2 = OpTypePointer Output %3
%4 = OpConstantNull %3
%1 = OpVariable %2 Output %4
)");
}

TEST_F(BuilderTest, GlobalVar_WithStorageClass_Input) {
  ast::type::F32Type f32;
  ast::Variable v("var", ast::StorageClass::kInput, &f32);

  ast::Module mod;
  Builder b(&mod);
  EXPECT_TRUE(b.GenerateGlobalVariable(&v)) << b.error();
  EXPECT_EQ(DumpInstructions(b.debug()), R"(OpName %1 "var"
)");
  EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeFloat 32
%2 = OpTypePointer Input %3
%1 = OpVariable %2 Input
)");
}

TEST_F(BuilderTest, GlobalVar_WithConstructor) {
  ast::type::F32Type f32;
  ast::type::VectorType vec(&f32, 3);

  ast::ExpressionList vals;
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0f)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0f)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 3.0f)));

  auto init =
      std::make_unique<ast::TypeConstructorExpression>(&vec, std::move(vals));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  EXPECT_TRUE(td.DetermineResultType(init.get())) << td.error();

  ast::Variable v("var", ast::StorageClass::kOutput, &f32);
  v.set_constructor(std::move(init));
  td.RegisterVariableForTesting(&v);

  Builder b(&mod);
  EXPECT_TRUE(b.GenerateGlobalVariable(&v)) << b.error();
  ASSERT_FALSE(b.has_error()) << b.error();

  EXPECT_EQ(DumpInstructions(b.debug()), R"(OpName %6 "var"
)");
  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 3
%3 = OpConstant %2 1
%4 = OpConstant %2 3
%5 = OpConstantComposite %1 %3 %3 %4
%7 = OpTypePointer Output %2
%6 = OpVariable %7 Output %5
)");
}

TEST_F(BuilderTest, GlobalVar_Const) {
  ast::type::F32Type f32;
  ast::type::VectorType vec(&f32, 3);

  ast::ExpressionList vals;
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0f)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0f)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 3.0f)));

  auto init =
      std::make_unique<ast::TypeConstructorExpression>(&vec, std::move(vals));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  EXPECT_TRUE(td.DetermineResultType(init.get())) << td.error();

  ast::Variable v("var", ast::StorageClass::kOutput, &f32);
  v.set_constructor(std::move(init));
  v.set_is_const(true);
  td.RegisterVariableForTesting(&v);

  Builder b(&mod);
  EXPECT_TRUE(b.GenerateGlobalVariable(&v)) << b.error();
  ASSERT_FALSE(b.has_error()) << b.error();

  EXPECT_EQ(DumpInstructions(b.debug()), R"(OpName %5 "var"
)");
  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 3
%3 = OpConstant %2 1
%4 = OpConstant %2 3
%5 = OpConstantComposite %1 %3 %3 %4
)");
}

TEST_F(BuilderTest, GlobalVar_Complex_Constructor) {
  ast::type::F32Type f32;
  ast::type::VectorType vec3(&f32, 3);
  ast::type::VectorType vec(&vec3, 3);

  ast::ExpressionList vals;
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0f)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0f)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 3.0f)));
  auto first =
      std::make_unique<ast::TypeConstructorExpression>(&vec3, std::move(vals));

  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 3.0f)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0f)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0f)));
  auto second =
      std::make_unique<ast::TypeConstructorExpression>(&vec3, std::move(vals));

  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0f)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0f)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 3.0f)));
  auto third =
      std::make_unique<ast::TypeConstructorExpression>(&vec3, std::move(vals));

  vals.push_back(std::move(first));
  vals.push_back(std::move(second));
  vals.push_back(std::move(third));

  auto init =
      std::make_unique<ast::TypeConstructorExpression>(&vec, std::move(vals));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  EXPECT_TRUE(td.DetermineResultType(init.get())) << td.error();

  ast::Variable v("var", ast::StorageClass::kOutput, &f32);
  v.set_constructor(std::move(init));
  v.set_is_const(true);
  td.RegisterVariableForTesting(&v);

  Builder b(&mod);
  EXPECT_TRUE(b.GenerateGlobalVariable(&v)) << b.error();
  ASSERT_FALSE(b.has_error()) << b.error();

  EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeFloat 32
%2 = OpTypeVector %3 3
%1 = OpTypeVector %2 3
%4 = OpConstant %3 1
%5 = OpConstant %3 2
%6 = OpConstant %3 3
%7 = OpConstantComposite %2 %4 %5 %6
%8 = OpConstantComposite %2 %6 %5 %4
%9 = OpConstantComposite %2 %5 %4 %6
%10 = OpConstantComposite %1 %7 %8 %9
)");
}

TEST_F(BuilderTest, GlobalVar_Complex_ConstructorWithExtract) {
  ast::type::F32Type f32;
  ast::type::VectorType vec3(&f32, 3);
  ast::type::VectorType vec2(&f32, 2);

  ast::ExpressionList vals;
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0f)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.0f)));
  auto first =
      std::make_unique<ast::TypeConstructorExpression>(&vec2, std::move(vals));

  vals.push_back(std::move(first));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 3.0f)));

  auto init =
      std::make_unique<ast::TypeConstructorExpression>(&vec3, std::move(vals));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  EXPECT_TRUE(td.DetermineResultType(init.get())) << td.error();

  ast::Variable v("var", ast::StorageClass::kOutput, &f32);
  v.set_constructor(std::move(init));
  v.set_is_const(true);
  td.RegisterVariableForTesting(&v);

  Builder b(&mod);
  EXPECT_TRUE(b.GenerateGlobalVariable(&v)) << b.error();
  ASSERT_FALSE(b.has_error()) << b.error();

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 3
%3 = OpTypeVector %2 2
%4 = OpConstant %2 1
%5 = OpConstant %2 2
%6 = OpConstantComposite %3 %4 %5
%8 = OpTypeInt 32 0
%9 = OpConstant %8 0
%7 = OpSpecConstantOp %2 CompositeExtract %6 9
%11 = OpConstant %8 1
%10 = OpSpecConstantOp %2 CompositeExtract %6 11
%12 = OpConstant %2 3
%13 = OpSpecConstantComposite %1 %7 %10 %12
)");
}

TEST_F(BuilderTest, GlobalVar_WithLocation) {
  ast::type::F32Type f32;
  auto v =
      std::make_unique<ast::Variable>("var", ast::StorageClass::kOutput, &f32);
  ast::VariableDecorationList decos;
  decos.push_back(std::make_unique<ast::LocationDecoration>(5));

  ast::DecoratedVariable dv(std::move(v));
  dv.set_decorations(std::move(decos));

  ast::Module mod;
  Builder b(&mod);
  EXPECT_TRUE(b.GenerateGlobalVariable(&dv)) << b.error();
  EXPECT_EQ(DumpInstructions(b.debug()), R"(OpName %1 "var"
)");
  EXPECT_EQ(DumpInstructions(b.annots()), R"(OpDecorate %1 Location 5
)");
  EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeFloat 32
%2 = OpTypePointer Output %3
%4 = OpConstantNull %3
%1 = OpVariable %2 Output %4
)");
}

TEST_F(BuilderTest, GlobalVar_WithBindingAndSet) {
  ast::type::F32Type f32;
  auto v =
      std::make_unique<ast::Variable>("var", ast::StorageClass::kOutput, &f32);
  ast::VariableDecorationList decos;
  decos.push_back(std::make_unique<ast::BindingDecoration>(2));
  decos.push_back(std::make_unique<ast::SetDecoration>(3));

  ast::DecoratedVariable dv(std::move(v));
  dv.set_decorations(std::move(decos));

  ast::Module mod;
  Builder b(&mod);
  EXPECT_TRUE(b.GenerateGlobalVariable(&dv)) << b.error();
  EXPECT_EQ(DumpInstructions(b.debug()), R"(OpName %1 "var"
)");
  EXPECT_EQ(DumpInstructions(b.annots()), R"(OpDecorate %1 Binding 2
OpDecorate %1 DescriptorSet 3
)");
  EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeFloat 32
%2 = OpTypePointer Output %3
%4 = OpConstantNull %3
%1 = OpVariable %2 Output %4
)");
}

TEST_F(BuilderTest, GlobalVar_WithBuiltin) {
  ast::type::F32Type f32;
  auto v =
      std::make_unique<ast::Variable>("var", ast::StorageClass::kOutput, &f32);
  ast::VariableDecorationList decos;
  decos.push_back(
      std::make_unique<ast::BuiltinDecoration>(ast::Builtin::kPosition));

  ast::DecoratedVariable dv(std::move(v));
  dv.set_decorations(std::move(decos));

  ast::Module mod;
  Builder b(&mod);
  EXPECT_TRUE(b.GenerateGlobalVariable(&dv)) << b.error();
  EXPECT_EQ(DumpInstructions(b.debug()), R"(OpName %1 "var"
)");
  EXPECT_EQ(DumpInstructions(b.annots()), R"(OpDecorate %1 BuiltIn Position
)");
  EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeFloat 32
%2 = OpTypePointer Output %3
%4 = OpConstantNull %3
%1 = OpVariable %2 Output %4
)");
}

struct BuiltinData {
  ast::Builtin builtin;
  SpvBuiltIn result;
};
inline std::ostream& operator<<(std::ostream& out, BuiltinData data) {
  out << data.builtin;
  return out;
}
using BuiltinDataTest = testing::TestWithParam<BuiltinData>;
TEST_P(BuiltinDataTest, Convert) {
  auto params = GetParam();

  ast::Module mod;
  Builder b(&mod);
  EXPECT_EQ(b.ConvertBuiltin(params.builtin), params.result);
}
INSTANTIATE_TEST_SUITE_P(
    BuilderTest_Type,
    BuiltinDataTest,
    testing::Values(
        BuiltinData{ast::Builtin::kNone, SpvBuiltInMax},
        BuiltinData{ast::Builtin::kPosition, SpvBuiltInPosition},
        BuiltinData{
            ast::Builtin::kVertexIdx,
            SpvBuiltInVertexIndex,
        },
        BuiltinData{ast::Builtin::kInstanceIdx, SpvBuiltInInstanceIndex},
        BuiltinData{ast::Builtin::kFrontFacing, SpvBuiltInFrontFacing},
        BuiltinData{ast::Builtin::kFragCoord, SpvBuiltInFragCoord},
        BuiltinData{ast::Builtin::kFragDepth, SpvBuiltInFragDepth},
        BuiltinData{ast::Builtin::kWorkgroupSize, SpvBuiltInWorkgroupSize},
        BuiltinData{ast::Builtin::kLocalInvocationId,
                    SpvBuiltInLocalInvocationId},
        BuiltinData{ast::Builtin::kLocalInvocationIdx,
                    SpvBuiltInLocalInvocationIndex},
        BuiltinData{ast::Builtin::kGlobalInvocationId,
                    SpvBuiltInGlobalInvocationId}));

}  // namespace
}  // namespace spirv
}  // namespace writer
}  // namespace tint
