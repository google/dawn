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

#include "src/ast/decoration.h"

#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <utility>

#include "src/ast/access_decoration.h"
#include "src/ast/array_decoration.h"
#include "src/ast/binding_decoration.h"
#include "src/ast/builtin_decoration.h"
#include "src/ast/constant_id_decoration.h"
#include "src/ast/function_decoration.h"
#include "src/ast/location_decoration.h"
#include "src/ast/stage_decoration.h"
#include "src/ast/stride_decoration.h"
#include "src/ast/struct_decoration.h"
#include "src/ast/struct_member_decoration.h"
#include "src/ast/struct_member_offset_decoration.h"
#include "src/ast/test_helper.h"
#include "src/ast/type_decoration.h"
#include "src/ast/variable_decoration.h"
#include "src/ast/workgroup_decoration.h"

namespace tint {
namespace ast {
namespace {

using DecorationTest = TestHelper;

TEST_F(DecorationTest, AsCorrectType) {
  auto* decoration = create<ConstantIdDecoration>(1, Source{});
  auto* upcast = static_cast<Decoration*>(decoration);
  auto* downcast = As<VariableDecoration>(upcast);
  EXPECT_EQ(decoration, downcast);
}

TEST_F(DecorationTest, AsIncorrectType) {
  auto* decoration = create<ConstantIdDecoration>(1, Source{});
  auto* upcast = static_cast<Decoration*>(decoration);
  auto* downcast = As<ArrayDecoration>(upcast);
  EXPECT_EQ(nullptr, downcast);
}

TEST_F(DecorationTest, Is) {
  Decoration* decoration = create<ConstantIdDecoration>(1, Source{});
  EXPECT_TRUE(decoration->Is<VariableDecoration>());
  EXPECT_FALSE(decoration->Is<ArrayDecoration>());
}

TEST_F(DecorationTest, Kinds) {
  EXPECT_EQ(ArrayDecoration::Kind, DecorationKind::kArray);
  EXPECT_EQ(StrideDecoration::Kind, DecorationKind::kArray);
  EXPECT_EQ(FunctionDecoration::Kind, DecorationKind::kFunction);
  EXPECT_EQ(StageDecoration::Kind, DecorationKind::kFunction);
  EXPECT_EQ(WorkgroupDecoration::Kind, DecorationKind::kFunction);
  EXPECT_EQ(StructDecoration::Kind, DecorationKind::kStruct);
  EXPECT_EQ(StructMemberDecoration::Kind, DecorationKind::kStructMember);
  EXPECT_EQ(StructMemberOffsetDecoration::Kind, DecorationKind::kStructMember);
  EXPECT_EQ(TypeDecoration::Kind, DecorationKind::kType);
  EXPECT_EQ(AccessDecoration::Kind, DecorationKind::kType);
  EXPECT_EQ(VariableDecoration::Kind, DecorationKind::kVariable);
  EXPECT_EQ(BindingDecoration::Kind, DecorationKind::kVariable);
  EXPECT_EQ(BuiltinDecoration::Kind, DecorationKind::kVariable);
  EXPECT_EQ(ConstantIdDecoration::Kind, DecorationKind::kVariable);
  EXPECT_EQ(LocationDecoration::Kind, DecorationKind::kVariable);
}

}  // namespace
}  // namespace ast
}  // namespace tint
