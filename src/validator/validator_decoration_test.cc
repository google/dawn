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
#include "src/ast/binding_decoration.h"
#include "src/ast/builtin_decoration.h"
#include "src/ast/constant_id_decoration.h"
#include "src/ast/group_decoration.h"
#include "src/ast/location_decoration.h"
#include "src/ast/stage_decoration.h"
#include "src/ast/struct_block_decoration.h"
#include "src/ast/struct_member_offset_decoration.h"
#include "src/ast/workgroup_decoration.h"
#include "src/validator/validator_test_helper.h"

namespace tint {
namespace {

enum class DecorationKind {
  kAccess,
  kBinding,
  kBuiltin,
  kConstantId,
  kGroup,
  kLocation,
  kStage,
  kStride,
  kStructBlock,
  kStructMemberOffset,
  kWorkgroup,
};
struct DecorationTestParams {
  DecorationKind kind;
  bool should_pass;
};
class ValidatorDecorationsTestWithParams
    : public ValidatorTestHelper,
      public testing::TestWithParam<DecorationTestParams> {};

ast::Decoration* createDecoration(ProgramBuilder& builder,
                                  DecorationKind kind) {
  switch (kind) {
    case DecorationKind::kAccess:
      return builder.create<ast::AccessDecoration>(
          ast::AccessControl::kReadOnly);
    case DecorationKind::kLocation:
      return builder.create<ast::LocationDecoration>(1);
    case DecorationKind::kBinding:
      return builder.create<ast::BindingDecoration>(1);
    case DecorationKind::kGroup:
      return builder.create<ast::GroupDecoration>(1u);
    case DecorationKind::kBuiltin:
      return builder.create<ast::BuiltinDecoration>(ast::Builtin::kPosition);
    case DecorationKind::kWorkgroup:
      return builder.create<ast::WorkgroupDecoration>(1u, 1u, 1u);
    case DecorationKind::kStage:
      return builder.create<ast::StageDecoration>(ast::PipelineStage::kCompute);
    case DecorationKind::kStructBlock:
      return builder.create<ast::StructBlockDecoration>();
    case DecorationKind::kStride:
      return builder.create<ast::StrideDecoration>(4u);
    case DecorationKind::kStructMemberOffset:
      return builder.create<ast::StructMemberOffsetDecoration>(4u);
    case DecorationKind::kConstantId:
      return builder.create<ast::ConstantIdDecoration>(0u);
  }
}

using ArrayDecorationTest = ValidatorDecorationsTestWithParams;
TEST_P(ArrayDecorationTest, Decoration_IsValid) {
  auto params = GetParam();

  ast::StructMemberList members{Member(
      "a", create<type::Array>(
               ty.f32(), 0,
               ast::DecorationList{createDecoration(*this, params.kind)}))};
  auto* s = create<ast::Struct>(
      members, ast::DecorationList{create<ast::StructBlockDecoration>()});
  auto* s_ty = ty.struct_("mystruct", s);

  ValidatorImpl& v = Build();

  if (params.should_pass) {
    EXPECT_TRUE(v.ValidateConstructedType(s_ty));
  } else {
    EXPECT_FALSE(v.ValidateConstructedType(s_ty));
    EXPECT_EQ(v.error(), "decoration is not valid for array types");
  }
}
INSTANTIATE_TEST_SUITE_P(
    ValidatorTest,
    ArrayDecorationTest,
    testing::Values(DecorationTestParams{DecorationKind::kAccess, false},
                    DecorationTestParams{DecorationKind::kBinding, false},
                    DecorationTestParams{DecorationKind::kBuiltin, false},
                    DecorationTestParams{DecorationKind::kConstantId, false},
                    DecorationTestParams{DecorationKind::kGroup, false},
                    DecorationTestParams{DecorationKind::kLocation, false},
                    DecorationTestParams{DecorationKind::kStage, false},
                    DecorationTestParams{DecorationKind::kStride, true},
                    DecorationTestParams{DecorationKind::kStructBlock, false},
                    DecorationTestParams{DecorationKind::kStructMemberOffset,
                                         false},
                    DecorationTestParams{DecorationKind::kWorkgroup, false}));

using FunctionDecorationTest = ValidatorDecorationsTestWithParams;
TEST_P(FunctionDecorationTest, Decoration_IsValid) {
  auto params = GetParam();

  Func("foo", ast::VariableList{}, ty.void_(), ast::StatementList{},
       ast::DecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kCompute),
           createDecoration(*this, params.kind)});

  ValidatorImpl& v = Build();

  if (params.should_pass) {
    EXPECT_TRUE(v.Validate());
  } else {
    EXPECT_FALSE(v.Validate());
    EXPECT_EQ(v.error(), "decoration is not valid for functions");
  }
}
INSTANTIATE_TEST_SUITE_P(
    ValidatorTest,
    FunctionDecorationTest,
    testing::Values(DecorationTestParams{DecorationKind::kAccess, false},
                    DecorationTestParams{DecorationKind::kBinding, false},
                    DecorationTestParams{DecorationKind::kBuiltin, false},
                    DecorationTestParams{DecorationKind::kConstantId, false},
                    DecorationTestParams{DecorationKind::kGroup, false},
                    DecorationTestParams{DecorationKind::kLocation, false},
                    // Skip kStage as we always apply it in this test
                    DecorationTestParams{DecorationKind::kStride, false},
                    DecorationTestParams{DecorationKind::kStructBlock, false},
                    DecorationTestParams{DecorationKind::kStructMemberOffset,
                                         false},
                    DecorationTestParams{DecorationKind::kWorkgroup, true}));

using StructDecorationTest = ValidatorDecorationsTestWithParams;
TEST_P(StructDecorationTest, Decoration_IsValid) {
  auto params = GetParam();

  auto* s = create<ast::Struct>(
      ast::StructMemberList{},
      ast::DecorationList{createDecoration(*this, params.kind)});
  auto* s_ty = ty.struct_("mystruct", s);

  ValidatorImpl& v = Build();

  if (params.should_pass) {
    EXPECT_TRUE(v.ValidateConstructedType(s_ty));
  } else {
    EXPECT_FALSE(v.ValidateConstructedType(s_ty));
    EXPECT_EQ(v.error(), "decoration is not valid for struct declarations");
  }
}
INSTANTIATE_TEST_SUITE_P(
    ValidatorTest,
    StructDecorationTest,
    testing::Values(DecorationTestParams{DecorationKind::kAccess, false},
                    DecorationTestParams{DecorationKind::kBinding, false},
                    DecorationTestParams{DecorationKind::kBuiltin, false},
                    DecorationTestParams{DecorationKind::kConstantId, false},
                    DecorationTestParams{DecorationKind::kGroup, false},
                    DecorationTestParams{DecorationKind::kLocation, false},
                    DecorationTestParams{DecorationKind::kStage, false},
                    DecorationTestParams{DecorationKind::kStride, false},
                    DecorationTestParams{DecorationKind::kStructBlock, true},
                    DecorationTestParams{DecorationKind::kStructMemberOffset,
                                         false},
                    DecorationTestParams{DecorationKind::kWorkgroup, false}));

using StructMemberDecorations = ValidatorDecorationsTestWithParams;
TEST_P(StructMemberDecorations, Decoration_IsValid) {
  auto params = GetParam();

  ast::StructMemberList members{
      Member("a", ty.i32(),
             ast::DecorationList{createDecoration(*this, params.kind)})};
  auto* s = create<ast::Struct>(members, ast::DecorationList{});
  auto* s_ty = ty.struct_("mystruct", s);

  ValidatorImpl& v = Build();

  if (params.should_pass) {
    EXPECT_TRUE(v.ValidateConstructedType(s_ty));
  } else {
    EXPECT_FALSE(v.ValidateConstructedType(s_ty));
    EXPECT_EQ(v.error(), "decoration is not valid for structure members");
  }
}
INSTANTIATE_TEST_SUITE_P(
    ValidatorTest,
    StructMemberDecorations,
    testing::Values(DecorationTestParams{DecorationKind::kAccess, false},
                    DecorationTestParams{DecorationKind::kBinding, false},
                    DecorationTestParams{DecorationKind::kBuiltin, true},
                    DecorationTestParams{DecorationKind::kConstantId, false},
                    DecorationTestParams{DecorationKind::kGroup, false},
                    DecorationTestParams{DecorationKind::kLocation, true},
                    DecorationTestParams{DecorationKind::kStage, false},
                    DecorationTestParams{DecorationKind::kStride, false},
                    DecorationTestParams{DecorationKind::kStructBlock, false},
                    DecorationTestParams{DecorationKind::kStructMemberOffset,
                                         true},
                    DecorationTestParams{DecorationKind::kWorkgroup, false}));

using VariableDecorationTest = ValidatorDecorationsTestWithParams;
TEST_P(VariableDecorationTest, Decoration_IsValid) {
  auto params = GetParam();

  auto* var = Global("a", ty.f32(), ast::StorageClass::kInput, nullptr,
                     ast::DecorationList{createDecoration(*this, params.kind)});

  ValidatorImpl& v = Build();

  if (params.should_pass) {
    EXPECT_TRUE(v.ValidateGlobalVariable(var));
  } else {
    EXPECT_FALSE(v.ValidateGlobalVariable(var));
    EXPECT_EQ(v.error(), "decoration is not valid for variables");
  }
}
INSTANTIATE_TEST_SUITE_P(
    ValidatorTest,
    VariableDecorationTest,
    testing::Values(DecorationTestParams{DecorationKind::kAccess, false},
                    DecorationTestParams{DecorationKind::kBinding, true},
                    DecorationTestParams{DecorationKind::kBuiltin, true},
                    DecorationTestParams{DecorationKind::kConstantId, true},
                    DecorationTestParams{DecorationKind::kGroup, true},
                    DecorationTestParams{DecorationKind::kLocation, true},
                    DecorationTestParams{DecorationKind::kStage, false},
                    DecorationTestParams{DecorationKind::kStride, false},
                    DecorationTestParams{DecorationKind::kStructBlock, false},
                    DecorationTestParams{DecorationKind::kStructMemberOffset,
                                         false},
                    DecorationTestParams{DecorationKind::kWorkgroup, false}));

}  // namespace
}  // namespace tint
