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

#include "src/tint/resolver/resolver.h"
#include "src/tint/resolver/resolver_test_helper.h"

#include "gmock/gmock.h"

namespace tint::resolver {
namespace {

// Helpers and typedefs
template <typename T>
using DataType = builder::DataType<T>;
template <typename T>
using vec2 = builder::vec2<T>;
template <typename T>
using vec3 = builder::vec3<T>;
template <typename T>
using vec4 = builder::vec4<T>;
template <typename T>
using mat2x2 = builder::mat2x2<T>;
template <typename T>
using mat3x3 = builder::mat3x3<T>;
template <typename T>
using mat4x4 = builder::mat4x4<T>;
template <typename T>
using alias = builder::alias<T>;
using f32 = builder::f32;
using i32 = builder::i32;
using u32 = builder::u32;

struct ResolverInferredTypeTest : public resolver::TestHelper,
                                  public testing::Test {};

struct Params {
  builder::ast_expr_func_ptr create_value;
  builder::sem_type_func_ptr create_expected_type;
};

template <typename T>
constexpr Params ParamsFor() {
  return Params{DataType<T>::Expr, DataType<T>::Sem};
}

Params all_cases[] = {
    ParamsFor<bool>(),                //
    ParamsFor<u32>(),                 //
    ParamsFor<i32>(),                 //
    ParamsFor<f32>(),                 //
    ParamsFor<vec3<bool>>(),          //
    ParamsFor<vec3<i32>>(),           //
    ParamsFor<vec3<u32>>(),           //
    ParamsFor<vec3<f32>>(),           //
    ParamsFor<mat3x3<f32>>(),         //
    ParamsFor<alias<bool>>(),         //
    ParamsFor<alias<u32>>(),          //
    ParamsFor<alias<i32>>(),          //
    ParamsFor<alias<f32>>(),          //
    ParamsFor<alias<vec3<bool>>>(),   //
    ParamsFor<alias<vec3<i32>>>(),    //
    ParamsFor<alias<vec3<u32>>>(),    //
    ParamsFor<alias<vec3<f32>>>(),    //
    ParamsFor<alias<mat3x3<f32>>>(),  //
};

using ResolverInferredTypeParamTest = ResolverTestWithParam<Params>;

TEST_P(ResolverInferredTypeParamTest, GlobalLet_Pass) {
  auto& params = GetParam();

  auto* expected_type = params.create_expected_type(*this);

  // let a = <type constructor>;
  auto* ctor_expr = params.create_value(*this, 0);
  auto* var = GlobalConst("a", nullptr, ctor_expr);
  WrapInFunction();

  EXPECT_TRUE(r()->Resolve()) << r()->error();
  EXPECT_EQ(TypeOf(var), expected_type);
}

TEST_P(ResolverInferredTypeParamTest, GlobalVar_Fail) {
  auto& params = GetParam();

  // var a = <type constructor>;
  auto* ctor_expr = params.create_value(*this, 0);
  Global(Source{{12, 34}}, "a", nullptr, ast::StorageClass::kPrivate,
         ctor_expr);
  WrapInFunction();

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: global var declaration must specify a type");
}

TEST_P(ResolverInferredTypeParamTest, LocalLet_Pass) {
  auto& params = GetParam();

  auto* expected_type = params.create_expected_type(*this);

  // let a = <type constructor>;
  auto* ctor_expr = params.create_value(*this, 0);
  auto* var = Let("a", nullptr, ctor_expr);
  WrapInFunction(var);

  EXPECT_TRUE(r()->Resolve()) << r()->error();
  EXPECT_EQ(TypeOf(var), expected_type);
}

TEST_P(ResolverInferredTypeParamTest, LocalVar_Pass) {
  auto& params = GetParam();

  auto* expected_type = params.create_expected_type(*this);

  // var a = <type constructor>;
  auto* ctor_expr = params.create_value(*this, 0);
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
      create<sem::Array>(create<sem::U32>(), 10u, 4u, 4u * 10u, 4u, 4u);

  auto* ctor_expr = Construct(type);
  auto* var = Var("a", nullptr, ast::StorageClass::kFunction, ctor_expr);
  WrapInFunction(var);

  EXPECT_TRUE(r()->Resolve()) << r()->error();
  EXPECT_EQ(TypeOf(var)->UnwrapRef(), expected_type);
}

TEST_F(ResolverInferredTypeTest, InferStruct_Pass) {
  auto* member = Member("x", ty.i32());
  auto* str = Structure("S", {member});

  auto* expected_type = create<sem::Struct>(
      str, str->name,
      sem::StructMemberList{create<sem::StructMember>(
          member, member->symbol, create<sem::I32>(), 0u, 0u, 0u, 4u)},
      0u, 4u, 4u);

  auto* ctor_expr = Construct(ty.Of(str));

  auto* var = Var("a", nullptr, ast::StorageClass::kFunction, ctor_expr);
  WrapInFunction(var);

  EXPECT_TRUE(r()->Resolve()) << r()->error();
  EXPECT_EQ(TypeOf(var)->UnwrapRef(), expected_type);
}

}  // namespace
}  // namespace tint::resolver
