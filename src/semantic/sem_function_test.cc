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

#include "src/semantic/function.h"

#include "src/ast/builtin_decoration.h"
#include "src/ast/location_decoration.h"
#include "src/semantic/test_helper.h"

namespace tint {
namespace semantic {
namespace {

using FunctionTest = TestHelper;

TEST_F(FunctionTest, GetReferenceLocations) {
  auto* loc1 = Var("loc1", ast::StorageClass::kInput, ty.i32(), nullptr,
                   ast::VariableDecorationList{
                       create<ast::LocationDecoration>(0),
                   });

  auto* loc2 = Var("loc2", ast::StorageClass::kInput, ty.i32(), nullptr,
                   ast::VariableDecorationList{
                       create<ast::LocationDecoration>(1),
                   });

  auto* builtin1 =
      Var("builtin1", ast::StorageClass::kInput, ty.i32(), nullptr,
          ast::VariableDecorationList{
              create<ast::BuiltinDecoration>(ast::Builtin::kPosition),
          });

  auto* builtin2 =
      Var("builtin2", ast::StorageClass::kInput, ty.i32(), nullptr,
          ast::VariableDecorationList{
              create<ast::BuiltinDecoration>(ast::Builtin::kFragDepth),
          });

  auto* f = create<Function>(
      /* referenced_module_vars */ std::vector<ast::Variable*>{loc1, builtin1,
                                                               loc2, builtin2},
      /* local_referenced_module_vars */ std::vector<ast::Variable*>{},
      /* ancestor_entry_points */ std::vector<Symbol>{});

  ASSERT_EQ(f->ReferencedModuleVariables().size(), 4u);

  auto ref_locs = f->ReferencedLocationVariables();
  ASSERT_EQ(ref_locs.size(), 2u);
  EXPECT_EQ(ref_locs[0].first, loc1);
  EXPECT_EQ(ref_locs[0].second->value(), 0u);
  EXPECT_EQ(ref_locs[1].first, loc2);
  EXPECT_EQ(ref_locs[1].second->value(), 1u);
}

TEST_F(FunctionTest, GetReferenceBuiltins) {
  auto* loc1 = Var("loc1", ast::StorageClass::kInput, ty.i32(), nullptr,
                   ast::VariableDecorationList{
                       create<ast::LocationDecoration>(0),
                   });

  auto* loc2 = Var("loc2", ast::StorageClass::kInput, ty.i32(), nullptr,
                   ast::VariableDecorationList{
                       create<ast::LocationDecoration>(1),
                   });

  auto* builtin1 =
      Var("builtin1", ast::StorageClass::kInput, ty.i32(), nullptr,
          ast::VariableDecorationList{
              create<ast::BuiltinDecoration>(ast::Builtin::kPosition),
          });

  auto* builtin2 =
      Var("builtin2", ast::StorageClass::kInput, ty.i32(), nullptr,
          ast::VariableDecorationList{
              create<ast::BuiltinDecoration>(ast::Builtin::kFragDepth),
          });

  auto* f = create<Function>(
      /* referenced_module_vars */ std::vector<ast::Variable*>{loc1, builtin1,
                                                               loc2, builtin2},
      /* local_referenced_module_vars */ std::vector<ast::Variable*>{},
      /* ancestor_entry_points */ std::vector<Symbol>{});

  ASSERT_EQ(f->ReferencedModuleVariables().size(), 4u);

  auto ref_locs = f->ReferencedBuiltinVariables();
  ASSERT_EQ(ref_locs.size(), 2u);
  EXPECT_EQ(ref_locs[0].first, builtin1);
  EXPECT_EQ(ref_locs[0].second->value(), ast::Builtin::kPosition);
  EXPECT_EQ(ref_locs[1].first, builtin2);
  EXPECT_EQ(ref_locs[1].second->value(), ast::Builtin::kFragDepth);
}

}  // namespace
}  // namespace semantic
}  // namespace tint
