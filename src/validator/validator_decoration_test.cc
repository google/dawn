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
#include "src/ast/struct_member_align_decoration.h"
#include "src/ast/struct_member_offset_decoration.h"
#include "src/ast/struct_member_size_decoration.h"
#include "src/ast/workgroup_decoration.h"
#include "src/validator/validator_test_helper.h"

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
    case DecorationKind::kAlign:
      return builder.create<ast::StructMemberAlignDecoration>(4u);
    case DecorationKind::kBinding:
      return builder.create<ast::BindingDecoration>(1);
    case DecorationKind::kBuiltin:
      return builder.create<ast::BuiltinDecoration>(ast::Builtin::kPosition);
    case DecorationKind::kConstantId:
      return builder.create<ast::ConstantIdDecoration>(0u);
    case DecorationKind::kGroup:
      return builder.create<ast::GroupDecoration>(1u);
    case DecorationKind::kLocation:
      return builder.create<ast::LocationDecoration>(1);
    case DecorationKind::kOffset:
      return builder.create<ast::StructMemberOffsetDecoration>(4u);
    case DecorationKind::kSize:
      return builder.create<ast::StructMemberSizeDecoration>(4u);
    case DecorationKind::kStage:
      return builder.create<ast::StageDecoration>(ast::PipelineStage::kCompute);
    case DecorationKind::kStride:
      return builder.create<ast::StrideDecoration>(4u);
    case DecorationKind::kStructBlock:
      return builder.create<ast::StructBlockDecoration>();
    case DecorationKind::kWorkgroup:
      return builder.create<ast::WorkgroupDecoration>(1u, 1u, 1u);
  }
  return nullptr;
}

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
                    DecorationTestParams{DecorationKind::kAlign, false},
                    DecorationTestParams{DecorationKind::kBinding, false},
                    DecorationTestParams{DecorationKind::kBuiltin, false},
                    DecorationTestParams{DecorationKind::kConstantId, false},
                    DecorationTestParams{DecorationKind::kGroup, false},
                    DecorationTestParams{DecorationKind::kLocation, false},
                    DecorationTestParams{DecorationKind::kOffset, false},
                    DecorationTestParams{DecorationKind::kSize, false},
                    // Skip kStage as we always apply it in this test
                    DecorationTestParams{DecorationKind::kStride, false},
                    DecorationTestParams{DecorationKind::kStructBlock, false},
                    DecorationTestParams{DecorationKind::kWorkgroup, true}));

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
                    DecorationTestParams{DecorationKind::kAlign, false},
                    DecorationTestParams{DecorationKind::kBinding, true},
                    DecorationTestParams{DecorationKind::kBuiltin, true},
                    DecorationTestParams{DecorationKind::kConstantId, true},
                    DecorationTestParams{DecorationKind::kGroup, true},
                    DecorationTestParams{DecorationKind::kLocation, true},
                    DecorationTestParams{DecorationKind::kOffset, false},
                    DecorationTestParams{DecorationKind::kSize, false},
                    DecorationTestParams{DecorationKind::kStage, false},
                    DecorationTestParams{DecorationKind::kStride, false},
                    DecorationTestParams{DecorationKind::kStructBlock, false},
                    DecorationTestParams{DecorationKind::kWorkgroup, false}));

}  // namespace
}  // namespace tint
