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

#include "src/ast/access_decoration.h"
#include "src/ast/constant_id_decoration.h"
#include "src/ast/return_statement.h"
#include "src/ast/stage_decoration.h"
#include "src/ast/struct_block_decoration.h"
#include "src/ast/workgroup_decoration.h"
#include "src/resolver/resolver.h"
#include "src/resolver/resolver_test_helper.h"

#include "gmock/gmock.h"

namespace tint {
namespace {

enum class DecorationKind {
  kAccess,
  kAlign,
  kBinding,
  kBuiltin,
  kConstantId,
  kGroup,
  kLocation,
  kOffset,
  kSize,
  kStage,
  kStride,
  kStructBlock,
  kWorkgroup,
};
struct TestParams {
  DecorationKind kind;
  bool should_pass;
};
class TestWithParams : public resolver::TestHelper,
                       public testing::TestWithParam<TestParams> {};

ast::Decoration* createDecoration(const Source& source,
                                  ProgramBuilder& builder,
                                  DecorationKind kind) {
  switch (kind) {
    case DecorationKind::kAccess:
      return builder.create<ast::AccessDecoration>(
          source, ast::AccessControl::kReadOnly);
    case DecorationKind::kAlign:
      return builder.create<ast::StructMemberAlignDecoration>(source, 4u);
    case DecorationKind::kBinding:
      return builder.create<ast::BindingDecoration>(source, 1);
    case DecorationKind::kBuiltin:
      return builder.create<ast::BuiltinDecoration>(source,
                                                    ast::Builtin::kPosition);
    case DecorationKind::kConstantId:
      return builder.create<ast::ConstantIdDecoration>(source, 0u);
    case DecorationKind::kGroup:
      return builder.create<ast::GroupDecoration>(source, 1u);
    case DecorationKind::kLocation:
      return builder.create<ast::LocationDecoration>(source, 1);
    case DecorationKind::kOffset:
      return builder.create<ast::StructMemberOffsetDecoration>(source, 4u);
    case DecorationKind::kSize:
      return builder.create<ast::StructMemberSizeDecoration>(source, 4u);
    case DecorationKind::kStage:
      return builder.create<ast::StageDecoration>(source,
                                                  ast::PipelineStage::kCompute);
    case DecorationKind::kStride:
      return builder.create<ast::StrideDecoration>(source, 4u);
    case DecorationKind::kStructBlock:
      return builder.create<ast::StructBlockDecoration>(source);
    case DecorationKind::kWorkgroup:
      return builder.create<ast::WorkgroupDecoration>(source, 1u, 1u, 1u);
  }
  return nullptr;
}

using FunctionReturnTypeDecorationTest = TestWithParams;
TEST_P(FunctionReturnTypeDecorationTest, IsValid) {
  auto params = GetParam();

  Func("main", ast::VariableList{}, ty.f32(),
       ast::StatementList{create<ast::ReturnStatement>(Expr(1.f))},
       ast::DecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kVertex)},
       ast::DecorationList{createDecoration({}, *this, params.kind)});

  if (params.should_pass) {
    EXPECT_TRUE(r()->Resolve()) << r()->error();
  } else {
    EXPECT_FALSE(r()->Resolve()) << r()->error();
    EXPECT_EQ(r()->error(),
              "error: decoration is not valid for function return types");
  }
}
INSTANTIATE_TEST_SUITE_P(
    ResolverDecorationValidationTest,
    FunctionReturnTypeDecorationTest,
    testing::Values(TestParams{DecorationKind::kAccess, false},
                    TestParams{DecorationKind::kAlign, false},
                    TestParams{DecorationKind::kBinding, false},
                    TestParams{DecorationKind::kBuiltin, true},
                    TestParams{DecorationKind::kConstantId, false},
                    TestParams{DecorationKind::kGroup, false},
                    TestParams{DecorationKind::kLocation, true},
                    TestParams{DecorationKind::kOffset, false},
                    TestParams{DecorationKind::kSize, false},
                    TestParams{DecorationKind::kStage, false},
                    TestParams{DecorationKind::kStride, false},
                    TestParams{DecorationKind::kStructBlock, false},
                    TestParams{DecorationKind::kWorkgroup, false}));

using ArrayDecorationTest = TestWithParams;

TEST_P(ArrayDecorationTest, IsValid) {
  auto params = GetParam();

  ast::StructMemberList members{Member(
      "a", create<type::Array>(ty.f32(), 0,
                               ast::DecorationList{createDecoration(
                                   Source{{12, 34}}, *this, params.kind)}))};
  auto* s = create<ast::Struct>(
      members, ast::DecorationList{create<ast::StructBlockDecoration>()});
  auto* s_ty = ty.struct_("mystruct", s);
  AST().AddConstructedType(s_ty);

  WrapInFunction();

  if (params.should_pass) {
    EXPECT_TRUE(r()->Resolve()) << r()->error();
  } else {
    EXPECT_FALSE(r()->Resolve()) << r()->error();
    EXPECT_EQ(r()->error(),
              "12:34 error: decoration is not valid for array types");
  }
}
INSTANTIATE_TEST_SUITE_P(
    ResolverDecorationValidationTest,
    ArrayDecorationTest,
    testing::Values(TestParams{DecorationKind::kAccess, false},
                    TestParams{DecorationKind::kAlign, false},
                    TestParams{DecorationKind::kBinding, false},
                    TestParams{DecorationKind::kBuiltin, false},
                    TestParams{DecorationKind::kConstantId, false},
                    TestParams{DecorationKind::kGroup, false},
                    TestParams{DecorationKind::kLocation, false},
                    TestParams{DecorationKind::kOffset, false},
                    TestParams{DecorationKind::kSize, false},
                    TestParams{DecorationKind::kStage, false},
                    TestParams{DecorationKind::kStride, true},
                    TestParams{DecorationKind::kStructBlock, false},
                    TestParams{DecorationKind::kWorkgroup, false}));

using StructDecorationTest = TestWithParams;
TEST_P(StructDecorationTest, IsValid) {
  auto params = GetParam();

  auto* s = create<ast::Struct>(ast::StructMemberList{},
                                ast::DecorationList{createDecoration(
                                    Source{{12, 34}}, *this, params.kind)});
  auto* s_ty = ty.struct_("mystruct", s);
  AST().AddConstructedType(s_ty);

  WrapInFunction();

  if (params.should_pass) {
    EXPECT_TRUE(r()->Resolve()) << r()->error();
  } else {
    EXPECT_FALSE(r()->Resolve()) << r()->error();
    EXPECT_EQ(r()->error(),
              "12:34 error: decoration is not valid for struct declarations");
  }
}
INSTANTIATE_TEST_SUITE_P(
    ResolverDecorationValidationTest,
    StructDecorationTest,
    testing::Values(TestParams{DecorationKind::kAccess, false},
                    TestParams{DecorationKind::kAlign, false},
                    TestParams{DecorationKind::kBinding, false},
                    TestParams{DecorationKind::kBuiltin, false},
                    TestParams{DecorationKind::kConstantId, false},
                    TestParams{DecorationKind::kGroup, false},
                    TestParams{DecorationKind::kLocation, false},
                    TestParams{DecorationKind::kOffset, false},
                    TestParams{DecorationKind::kSize, false},
                    TestParams{DecorationKind::kStage, false},
                    TestParams{DecorationKind::kStride, false},
                    TestParams{DecorationKind::kStructBlock, true},
                    TestParams{DecorationKind::kWorkgroup, false}));

using StructMemberDecorationTest = TestWithParams;
TEST_P(StructMemberDecorationTest, IsValid) {
  auto params = GetParam();

  ast::StructMemberList members{
      Member("a", ty.i32(),
             ast::DecorationList{
                 createDecoration(Source{{12, 34}}, *this, params.kind)})};
  auto* s = create<ast::Struct>(members, ast::DecorationList{});
  auto* s_ty = ty.struct_("mystruct", s);
  AST().AddConstructedType(s_ty);

  WrapInFunction();

  if (params.should_pass) {
    EXPECT_TRUE(r()->Resolve()) << r()->error();
  } else {
    EXPECT_FALSE(r()->Resolve()) << r()->error();
    EXPECT_EQ(r()->error(),
              "12:34 error: decoration is not valid for structure members");
  }
}
INSTANTIATE_TEST_SUITE_P(
    ResolverDecorationValidationTest,
    StructMemberDecorationTest,
    testing::Values(TestParams{DecorationKind::kAccess, false},
                    TestParams{DecorationKind::kAlign, true},
                    TestParams{DecorationKind::kBinding, false},
                    TestParams{DecorationKind::kBuiltin, true},
                    TestParams{DecorationKind::kConstantId, false},
                    TestParams{DecorationKind::kGroup, false},
                    TestParams{DecorationKind::kLocation, true},
                    TestParams{DecorationKind::kOffset, true},
                    TestParams{DecorationKind::kSize, true},
                    TestParams{DecorationKind::kStage, false},
                    TestParams{DecorationKind::kStride, false},
                    TestParams{DecorationKind::kStructBlock, false},
                    TestParams{DecorationKind::kWorkgroup, false}));

using VariableDecorationTest = TestWithParams;
TEST_P(VariableDecorationTest, IsValid) {
  auto params = GetParam();

  Global("a", ty.f32(), ast::StorageClass::kInput, nullptr,
         ast::DecorationList{
             createDecoration(Source{{12, 34}}, *this, params.kind)});

  WrapInFunction();

  if (params.should_pass) {
    EXPECT_TRUE(r()->Resolve()) << r()->error();
  } else {
    EXPECT_FALSE(r()->Resolve()) << r()->error();
    EXPECT_EQ(r()->error(),
              "12:34 error: decoration is not valid for variables");
  }
}
INSTANTIATE_TEST_SUITE_P(
    ResolverDecorationValidationTest,
    VariableDecorationTest,
    testing::Values(TestParams{DecorationKind::kAccess, false},
                    TestParams{DecorationKind::kAlign, false},
                    TestParams{DecorationKind::kBinding, true},
                    TestParams{DecorationKind::kBuiltin, true},
                    TestParams{DecorationKind::kConstantId, true},
                    TestParams{DecorationKind::kGroup, true},
                    TestParams{DecorationKind::kLocation, true},
                    TestParams{DecorationKind::kOffset, false},
                    TestParams{DecorationKind::kSize, false},
                    TestParams{DecorationKind::kStage, false},
                    TestParams{DecorationKind::kStride, false},
                    TestParams{DecorationKind::kStructBlock, false},
                    TestParams{DecorationKind::kWorkgroup, false}));

using FunctionDecorationTest = TestWithParams;
TEST_P(FunctionDecorationTest, IsValid) {
  auto params = GetParam();

  Func("foo", ast::VariableList{}, ty.void_(), ast::StatementList{},
       ast::DecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kCompute),
           createDecoration(Source{{12, 34}}, *this, params.kind)});

  if (params.should_pass) {
    EXPECT_TRUE(r()->Resolve()) << r()->error();
  } else {
    EXPECT_FALSE(r()->Resolve()) << r()->error();
    EXPECT_EQ(r()->error(),
              "12:34 error: decoration is not valid for functions");
  }
}
INSTANTIATE_TEST_SUITE_P(
    ResolverDecorationValidationTest,
    FunctionDecorationTest,
    testing::Values(TestParams{DecorationKind::kAccess, false},
                    TestParams{DecorationKind::kAlign, false},
                    TestParams{DecorationKind::kBinding, false},
                    TestParams{DecorationKind::kBuiltin, false},
                    TestParams{DecorationKind::kConstantId, false},
                    TestParams{DecorationKind::kGroup, false},
                    TestParams{DecorationKind::kLocation, false},
                    TestParams{DecorationKind::kOffset, false},
                    TestParams{DecorationKind::kSize, false},
                    // Skip kStage as we always apply it in this test
                    TestParams{DecorationKind::kStride, false},
                    TestParams{DecorationKind::kStructBlock, false},
                    TestParams{DecorationKind::kWorkgroup, true}));

}  // namespace
}  // namespace tint
