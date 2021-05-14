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

#include "src/resolver/resolver.h"

#include "gmock/gmock.h"
#include "src/ast/struct_block_decoration.h"
#include "src/resolver/resolver_test_helper.h"
#include "src/sem/struct.h"

namespace tint {
namespace resolver {
namespace {

using ResolverHostShareableValidationTest = ResolverTest;

TEST_F(ResolverHostShareableValidationTest, BoolMember) {
  auto* s = Structure("S", {Member(Source{{12, 34}}, "x", ty.bool_())},
                      {create<ast::StructBlockDecoration>()});
  auto* a = ty.access(ast::AccessControl::kReadOnly, s);
  Global(Source{{56, 78}}, "g", a, ast::StorageClass::kStorage, nullptr,
         {
             create<ast::BindingDecoration>(0),
             create<ast::GroupDecoration>(0),
         });

  ASSERT_FALSE(r()->Resolve());

  EXPECT_EQ(
      r()->error(),
      R"(56:78 error: Type 'bool' cannot be used in storage class 'storage' as it is non-host-shareable
12:34 note: while analysing structure member S.x
56:78 note: while instantiating variable g)");
}

TEST_F(ResolverHostShareableValidationTest, BoolVectorMember) {
  auto* s = Structure("S", {Member(Source{{12, 34}}, "x", ty.vec3<bool>())},
                      {create<ast::StructBlockDecoration>()});
  auto* a = ty.access(ast::AccessControl::kReadOnly, s);
  Global(Source{{56, 78}}, "g", a, ast::StorageClass::kStorage, nullptr,
         {
             create<ast::BindingDecoration>(0),
             create<ast::GroupDecoration>(0),
         });

  ASSERT_FALSE(r()->Resolve());

  EXPECT_EQ(
      r()->error(),
      R"(56:78 error: Type 'vec3<bool>' cannot be used in storage class 'storage' as it is non-host-shareable
12:34 note: while analysing structure member S.x
56:78 note: while instantiating variable g)");
}

TEST_F(ResolverHostShareableValidationTest, Aliases) {
  auto* a1 = ty.alias("a1", ty.bool_());
  AST().AddConstructedType(a1);
  auto* s = Structure("S", {Member(Source{{12, 34}}, "x", a1)},
                      {create<ast::StructBlockDecoration>()});
  auto* ac = ty.access(ast::AccessControl::kReadOnly, s);
  auto* a2 = ty.alias("a2", ac);
  AST().AddConstructedType(a2);
  Global(Source{{56, 78}}, "g", a2, ast::StorageClass::kStorage, nullptr,
         {
             create<ast::BindingDecoration>(0),
             create<ast::GroupDecoration>(0),
         });

  ASSERT_FALSE(r()->Resolve());

  EXPECT_EQ(
      r()->error(),
      R"(56:78 error: Type 'bool' cannot be used in storage class 'storage' as it is non-host-shareable
12:34 note: while analysing structure member S.x
56:78 note: while instantiating variable g)");
}

TEST_F(ResolverHostShareableValidationTest, NestedStructures) {
  auto* i1 = Structure("I1", {Member(Source{{1, 2}}, "x", ty.bool_())});
  auto* i2 = Structure("I2", {Member(Source{{3, 4}}, "y", i1)});
  auto* i3 = Structure("I3", {Member(Source{{5, 6}}, "z", i2)});

  auto* s = Structure("S", {Member(Source{{7, 8}}, "m", i3)},
                      {create<ast::StructBlockDecoration>()});
  auto* a = ty.access(ast::AccessControl::kReadOnly, s);
  Global(Source{{9, 10}}, "g", a, ast::StorageClass::kStorage, nullptr,
         {
             create<ast::BindingDecoration>(0),
             create<ast::GroupDecoration>(0),
         });

  ASSERT_FALSE(r()->Resolve());

  EXPECT_EQ(
      r()->error(),
      R"(9:10 error: Type 'bool' cannot be used in storage class 'storage' as it is non-host-shareable
1:2 note: while analysing structure member I1.x
3:4 note: while analysing structure member I2.y
5:6 note: while analysing structure member I3.z
7:8 note: while analysing structure member S.m
9:10 note: while instantiating variable g)");
}

TEST_F(ResolverHostShareableValidationTest, NoError) {
  auto* i1 =
      Structure("I1", {
                          Member(Source{{1, 1}}, "x1", ty.f32()),
                          Member(Source{{2, 1}}, "y1", ty.vec3<f32>()),
                          Member(Source{{3, 1}}, "z1", ty.array<i32, 4>()),
                      });
  auto* a1 = ty.alias("a1", i1);
  AST().AddConstructedType(a1);
  auto* i2 = Structure("I2", {
                                 Member(Source{{4, 1}}, "x2", ty.mat2x2<f32>()),
                                 Member(Source{{5, 1}}, "y2", i1),
                             });
  auto* a2 = ty.alias("a2", i2);
  AST().AddConstructedType(a2);
  auto* i3 = Structure("I3", {
                                 Member(Source{{4, 1}}, "x3", a1),
                                 Member(Source{{5, 1}}, "y3", i2),
                                 Member(Source{{6, 1}}, "z3", a2),
                             });

  auto* s = Structure("S", {Member(Source{{7, 8}}, "m", i3)},
                      {create<ast::StructBlockDecoration>()});
  auto* a = ty.access(ast::AccessControl::kReadOnly, s);
  Global(Source{{9, 10}}, "g", a, ast::StorageClass::kStorage, nullptr,
         {
             create<ast::BindingDecoration>(0),
             create<ast::GroupDecoration>(0),
         });
  WrapInFunction();

  ASSERT_TRUE(r()->Resolve()) << r()->error();
}

}  // namespace
}  // namespace resolver
}  // namespace tint
