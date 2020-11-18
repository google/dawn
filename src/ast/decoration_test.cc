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
  auto* decoration = create<ConstantIdDecoration>(1, Source{});
  EXPECT_TRUE(decoration->Is<VariableDecoration>());
  EXPECT_FALSE(decoration->Is<ArrayDecoration>());
}

TEST_F(DecorationTest, Kinds) {
  EXPECT_EQ(ArrayDecoration::Kind, DecorationKind::kArray);
  EXPECT_EQ(StrideDecoration::Kind, DecorationKind::kStride);
  EXPECT_EQ(FunctionDecoration::Kind, DecorationKind::kFunction);
  EXPECT_EQ(StageDecoration::Kind, DecorationKind::kStage);
  EXPECT_EQ(WorkgroupDecoration::Kind, DecorationKind::kWorkgroup);
  EXPECT_EQ(StructDecoration::Kind, DecorationKind::kStruct);
  EXPECT_EQ(StructMemberDecoration::Kind, DecorationKind::kStructMember);
  EXPECT_EQ(StructMemberOffsetDecoration::Kind,
            DecorationKind::kStructMemberOffset);
  EXPECT_EQ(VariableDecoration::Kind, DecorationKind::kVariable);
  EXPECT_EQ(BindingDecoration::Kind, DecorationKind::kBinding);
  EXPECT_EQ(BuiltinDecoration::Kind, DecorationKind::kBuiltin);
  EXPECT_EQ(ConstantIdDecoration::Kind, DecorationKind::kConstantId);
  EXPECT_EQ(LocationDecoration::Kind, DecorationKind::kLocation);
}

TEST_F(DecorationTest, IsKind) {
  std::vector<DecorationKind> all_kinds{
      DecorationKind::kArray,        DecorationKind::kStride,
      DecorationKind::kFunction,     DecorationKind::kStage,
      DecorationKind::kWorkgroup,    DecorationKind::kStruct,
      DecorationKind::kStructMember, DecorationKind::kStructMemberOffset,
      DecorationKind::kVariable,     DecorationKind::kBinding,
      DecorationKind::kBuiltin,      DecorationKind::kConstantId,
      DecorationKind::kLocation,
  };

  struct ExpectedKinds {
    DecorationKind kind;
    std::unordered_set<DecorationKind> expect_true;
  };

  //  kArray
  //   | kStride
  //  kFunction
  //   | kStage
  //   | kWorkgroup
  //  kStruct
  //  kStructMember
  //   | kStructMemberOffset
  //  kVariable
  //   | kBinding
  //   | kBuiltin
  //   | kConstantId
  //   | kLocation
  std::unordered_map<DecorationKind, std::unordered_set<DecorationKind>>
      kind_is{
          {
              DecorationKind::kStride,
              {DecorationKind::kArray, DecorationKind::kStride},
          },
          {
              DecorationKind::kStage,
              {DecorationKind::kFunction, DecorationKind::kStage},
          },
          {
              DecorationKind::kWorkgroup,
              {DecorationKind::kFunction, DecorationKind::kWorkgroup},
          },
          {
              DecorationKind::kStruct,
              {DecorationKind::kStruct},
          },
          {
              DecorationKind::kStructMemberOffset,
              {DecorationKind::kStructMember,
               DecorationKind::kStructMemberOffset},
          },
          {
              DecorationKind::kBinding,
              {DecorationKind::kVariable, DecorationKind::kBinding},
          },
          {
              DecorationKind::kBuiltin,
              {DecorationKind::kVariable, DecorationKind::kBuiltin},
          },
          {
              DecorationKind::kConstantId,
              {DecorationKind::kVariable, DecorationKind::kConstantId},
          },
          {
              DecorationKind::kLocation,
              {DecorationKind::kVariable, DecorationKind::kLocation},
          },
      };

  auto check = [&](Decoration* d) {
    auto& is_set = kind_is[d->GetKind()];
    for (auto test : all_kinds) {
      bool is_kind = is_set.find(test) != is_set.end();
      EXPECT_EQ(d->IsKind(test), is_kind)
          << "decoration: " << d->GetKind() << " IsKind(" << test << ")";
    }
  };
  StrideDecoration stride(0, {});
  StageDecoration stage(PipelineStage::kNone, {});
  WorkgroupDecoration workgroup(0, {});
  StructMemberOffsetDecoration struct_member_offset(0, {});
  BindingDecoration binding(0, {});
  BuiltinDecoration builtin(Builtin::kNone, {});
  ConstantIdDecoration constant_id(0, {});
  LocationDecoration location(0, {});

  check(&stride);
  check(&stage);
  check(&workgroup);
  check(&struct_member_offset);
  check(&binding);
  check(&builtin);
  check(&constant_id);
  check(&location);
}

}  // namespace
}  // namespace ast
}  // namespace tint
