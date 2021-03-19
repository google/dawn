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

#include "src/ast/return_statement.h"
#include "src/ast/stage_decoration.h"
#include "src/ast/struct_block_decoration.h"
#include "src/resolver/resolver.h"
#include "src/resolver/resolver_test_helper.h"

#include "gmock/gmock.h"

namespace tint {
namespace {

class ResolverTypeValidationTest : public resolver::TestHelper,
                                   public testing::Test {};

TEST_F(ResolverTypeValidationTest, RuntimeArrayIsLast_Pass) {
  // [[Block]]
  // struct Foo {
  //   vf: f32;
  //   rt: array<f32>;
  // };

  ast::DecorationList decos;
  decos.push_back(create<ast::StructBlockDecoration>());
  auto* st =
      create<ast::Struct>(ast::StructMemberList{Member("vf", ty.f32()),
                                                Member("rt", ty.array<f32>())},
                          decos);

  auto* struct_type = ty.struct_("Foo", st);
  AST().AddConstructedType(struct_type);

  WrapInFunction();

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverTypeValidationTest, RuntimeArrayIsLastNoBlock_Fail) {
  // struct Foo {
  //   vf: f32;
  //   rt: array<f32>;
  // };

  ast::DecorationList decos;
  auto* st = create<ast::Struct>(
      ast::StructMemberList{Member("vf", ty.f32()),
                            Member(Source{{12, 34}}, "rt", ty.array<f32>())},
      decos);

  auto* struct_type = ty.struct_("Foo", st);
  AST().AddConstructedType(struct_type);

  WrapInFunction();

  EXPECT_FALSE(r()->Resolve()) << r()->error();
  EXPECT_EQ(r()->error(),
            "12:34 error v-0015: a struct containing a runtime-sized array "
            "requires the [[block]] attribute: 'Foo'");
}

TEST_F(ResolverTypeValidationTest, RuntimeArrayIsNotLast_Fail) {
  // [[Block]]
  // struct Foo {
  //   rt: array<f32>;
  //   vf: f32;
  // };

  ast::DecorationList decos;
  decos.push_back(create<ast::StructBlockDecoration>());

  auto* rt = Member(Source{{12, 34}}, "rt", ty.array<f32>());
  auto* st = create<ast::Struct>(
      ast::StructMemberList{rt, Member("vf", ty.f32())}, decos);

  auto* struct_type = ty.struct_("Foo", st);

  AST().AddConstructedType(struct_type);

  WrapInFunction();

  EXPECT_FALSE(r()->Resolve()) << r()->error();
  EXPECT_EQ(r()->error(),
            "12:34 error v-0015: runtime arrays may only appear as the last "
            "member of a struct");
}

TEST_F(ResolverTypeValidationTest, RuntimeArrayAsParameter_Fail) {
  // fn func(a : array<u32>) {}
  // [[stage(vertex)]] fn main() {}

  auto* param = Var(Source{Source::Location{12, 34}}, "a", ty.array<i32>(),
                    ast::StorageClass::kNone);

  Func("func", ast::VariableList{param}, ty.void_(),
       ast::StatementList{
           create<ast::ReturnStatement>(),
       },
       ast::DecorationList{});

  Func("main", ast::VariableList{}, ty.void_(),
       ast::StatementList{
           create<ast::ReturnStatement>(),
       },
       ast::DecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kVertex),
       });

  EXPECT_FALSE(r()->Resolve()) << r()->error();
  EXPECT_EQ(
      r()->error(),
      "12:34 error v-0015: runtime arrays may only appear as the last member "
      "of a struct");
}

TEST_F(ResolverTypeValidationTest, AliasRuntimeArrayIsNotLast_Fail) {
  // [[Block]]
  // type RTArr = array<u32>;
  // struct s {
  //  b: RTArr;
  //  a: u32;
  //}

  auto* alias = ty.alias("RTArr", ty.array<u32>());

  ast::DecorationList decos;
  decos.push_back(create<ast::StructBlockDecoration>());
  auto* st = create<ast::Struct>(
      ast::StructMemberList{Member(Source{{12, 34}}, "b", alias),
                            Member("a", ty.u32())},
      decos);

  auto* struct_type = ty.struct_("s", st);
  AST().AddConstructedType(struct_type);

  WrapInFunction();

  EXPECT_FALSE(r()->Resolve()) << r()->error();
  EXPECT_EQ(
      r()->error(),
      "12:34 error v-0015: runtime arrays may only appear as the last member "
      "of a struct");
}

TEST_F(ResolverTypeValidationTest, AliasRuntimeArrayIsLast_Pass) {
  // [[Block]]
  // type RTArr = array<u32>;
  // struct s {
  //  a: u32;
  //  b: RTArr;
  //}

  auto* alias = ty.alias("RTArr", ty.array<u32>());

  ast::DecorationList decos;
  decos.push_back(create<ast::StructBlockDecoration>());
  auto* st = create<ast::Struct>(
      ast::StructMemberList{Member("a", ty.u32()), Member("b", alias)}, decos);

  auto* struct_type = ty.struct_("s", st);
  AST().AddConstructedType(struct_type);

  WrapInFunction();

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

}  // namespace
}  // namespace tint
