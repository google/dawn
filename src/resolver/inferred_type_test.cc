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

#include "src/ast/struct_block_decoration.h"
#include "src/resolver/resolver.h"
#include "src/resolver/resolver_test_helper.h"

#include "gmock/gmock.h"

namespace tint {
namespace resolver {
namespace {

// Helpers and typedefs
using i32 = ProgramBuilder::i32;
using u32 = ProgramBuilder::u32;
using f32 = ProgramBuilder::f32;

struct ResolverInferredTypeTest : public resolver::TestHelper,
                                  public testing::Test {};

struct Params {
  create_ast_type_func_ptr create_type;
  create_sem_type_func_ptr create_expected_type;
};

Params all_cases[] = {
    {ast_bool, sem_bool},
    {ast_u32, sem_u32},
    {ast_i32, sem_i32},
    {ast_f32, sem_f32},
    {ast_vec3<bool>, sem_vec3<sem_bool>},
    {ast_vec3<i32>, sem_vec3<sem_i32>},
    {ast_vec3<u32>, sem_vec3<sem_u32>},
    {ast_vec3<f32>, sem_vec3<sem_f32>},
    {ast_mat3x3<i32>, sem_mat3x3<sem_i32>},
    {ast_mat3x3<u32>, sem_mat3x3<sem_u32>},
    {ast_mat3x3<f32>, sem_mat3x3<sem_f32>},

    {ast_alias<ast_bool>, sem_bool},
    {ast_alias<ast_u32>, sem_u32},
    {ast_alias<ast_i32>, sem_i32},
    {ast_alias<ast_f32>, sem_f32},
    {ast_alias<ast_vec3<bool>>, sem_vec3<sem_bool>},
    {ast_alias<ast_vec3<i32>>, sem_vec3<sem_i32>},
    {ast_alias<ast_vec3<u32>>, sem_vec3<sem_u32>},
    {ast_alias<ast_vec3<f32>>, sem_vec3<sem_f32>},
    {ast_alias<ast_mat3x3<i32>>, sem_mat3x3<sem_i32>},
    {ast_alias<ast_mat3x3<u32>>, sem_mat3x3<sem_u32>},
    {ast_alias<ast_mat3x3<f32>>, sem_mat3x3<sem_f32>},
};

using ResolverInferredTypeParamTest = ResolverTestWithParam<Params>;

TEST_P(ResolverInferredTypeParamTest, GlobalLet_Pass) {
  auto& params = GetParam();

  auto* type = params.create_type(ty);
  auto* expected_type = params.create_expected_type(ty);

  // let a = <type constructor>;
  auto* ctor_expr = ConstructValueFilledWith(type);
  auto* var = GlobalConst("a", nullptr, ctor_expr);
  WrapInFunction();

  EXPECT_TRUE(r()->Resolve()) << r()->error();
  EXPECT_EQ(TypeOf(var), expected_type);
}

TEST_P(ResolverInferredTypeParamTest, GlobalVar_Fail) {
  auto& params = GetParam();

  auto* type = params.create_type(ty);

  // var a = <type constructor>;
  auto* ctor_expr = ConstructValueFilledWith(type);
  Global(Source{{12, 34}}, "a", nullptr, ast::StorageClass::kPrivate,
         ctor_expr);
  WrapInFunction();

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: global var declaration must specify a type");
}

TEST_P(ResolverInferredTypeParamTest, LocalLet_Pass) {
  auto& params = GetParam();

  auto* type = params.create_type(ty);
  auto* expected_type = params.create_expected_type(ty);

  // let a = <type constructor>;
  auto* ctor_expr = ConstructValueFilledWith(type);
  auto* var = Const("a", nullptr, ctor_expr);
  WrapInFunction(var);

  EXPECT_TRUE(r()->Resolve()) << r()->error();
  EXPECT_EQ(TypeOf(var), expected_type);
}

TEST_P(ResolverInferredTypeParamTest, LocalVar_Pass) {
  auto& params = GetParam();

  auto* type = params.create_type(ty);
  auto* expected_type = params.create_expected_type(ty);

  // var a = <type constructor>;
  auto* ctor_expr = ConstructValueFilledWith(type);
  auto* var = Var("a", nullptr, ast::StorageClass::kFunction, ctor_expr);
  WrapInFunction(var);

  EXPECT_TRUE(r()->Resolve()) << r()->error();
  EXPECT_EQ(TypeOf(var)->UnwrapRef(), expected_type);
}

INSTANTIATE_TEST_SUITE_P(ResolverTest,
                         ResolverInferredTypeParamTest,
                         testing::ValuesIn(all_cases));

TEST_F(ResolverInferredTypeTest, InferArray_Pass) {
  auto* type = ty.array(ty.u32(), 10);
  auto* expected_type =
      create<sem::Array>(create<sem::U32>(), 10, 4, 4 * 10, 4, true);

  auto* ctor_expr = Construct(type);
  auto* var = Var("a", nullptr, ast::StorageClass::kFunction, ctor_expr);
  WrapInFunction(var);

  EXPECT_TRUE(r()->Resolve()) << r()->error();
  EXPECT_EQ(TypeOf(var)->UnwrapRef(), expected_type);
}

TEST_F(ResolverInferredTypeTest, InferStruct_Pass) {
  auto* member = Member("x", ty.i32());
  auto* type = Structure("S", {member}, {create<ast::StructBlockDecoration>()});

  auto* expected_type =
      create<sem::Struct>(type,
                          sem::StructMemberList{create<sem::StructMember>(
                              member, create<sem::I32>(), 0, 0, 0, 4)},
                          0, 4, 4);

  auto* ctor_expr = Construct(type);

  auto* var = Var("a", nullptr, ast::StorageClass::kFunction, ctor_expr);
  WrapInFunction(var);

  EXPECT_TRUE(r()->Resolve()) << r()->error();
  EXPECT_EQ(TypeOf(var)->UnwrapRef(), expected_type);
}

}  // namespace
}  // namespace resolver
}  // namespace tint
