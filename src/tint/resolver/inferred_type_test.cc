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

using namespace tint::number_suffixes;  // NOLINT

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

struct ResolverInferredTypeTest : public resolver::TestHelper, public testing::Test {};

struct Params {
    // builder::ast_expr_func_ptr_default_arg create_value;
    builder::ast_expr_from_double_func_ptr create_value;
    builder::sem_type_func_ptr create_expected_type;
};

template <typename T>
constexpr Params ParamsFor() {
    // return Params{builder::CreateExprWithDefaultArg<T>(), DataType<T>::Sem};
    return Params{DataType<T>::ExprFromDouble, DataType<T>::Sem};
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

TEST_P(ResolverInferredTypeParamTest, GlobalConst_Pass) {
    auto& params = GetParam();

    auto* expected_type = params.create_expected_type(*this);

    // const a = <type initializer>;
    auto* ctor_expr = params.create_value(*this, 0);
    auto* a = GlobalConst("a", ctor_expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(TypeOf(a), expected_type);
}

TEST_P(ResolverInferredTypeParamTest, GlobalVar_Pass) {
    auto& params = GetParam();

    auto* expected_type = params.create_expected_type(*this);

    // var a = <type initializer>;
    auto* ctor_expr = params.create_value(*this, 0);
    auto* var = GlobalVar("a", builtin::AddressSpace::kPrivate, ctor_expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(TypeOf(var)->UnwrapRef(), expected_type);
}

TEST_P(ResolverInferredTypeParamTest, LocalLet_Pass) {
    auto& params = GetParam();

    auto* expected_type = params.create_expected_type(*this);

    // let a = <type initializer>;
    auto* ctor_expr = params.create_value(*this, 0);
    auto* var = Let("a", ctor_expr);
    WrapInFunction(var);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(TypeOf(var), expected_type);
}

TEST_P(ResolverInferredTypeParamTest, LocalVar_Pass) {
    auto& params = GetParam();

    auto* expected_type = params.create_expected_type(*this);

    // var a = <type initializer>;
    auto* ctor_expr = params.create_value(*this, 0);
    auto* var = Var("a", builtin::AddressSpace::kFunction, ctor_expr);
    WrapInFunction(var);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(TypeOf(var)->UnwrapRef(), expected_type);
}

INSTANTIATE_TEST_SUITE_P(ResolverTest, ResolverInferredTypeParamTest, testing::ValuesIn(all_cases));

TEST_F(ResolverInferredTypeTest, InferArray_Pass) {
    auto type = ty.array<u32, 10>();
    auto* expected_type = create<type::Array>(
        create<type::U32>(), create<type::ConstantArrayCount>(10u), 4u, 4u * 10u, 4u, 4u);

    auto* ctor_expr = Call(type);
    auto* var = Var("a", builtin::AddressSpace::kFunction, ctor_expr);
    WrapInFunction(var);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(TypeOf(var)->UnwrapRef(), expected_type);
}

TEST_F(ResolverInferredTypeTest, InferStruct_Pass) {
    auto* member = Member("x", ty.i32());
    auto* str = Structure("S", utils::Vector{member});

    auto* expected_type = create<sem::Struct>(
        str, str->name->symbol,
        utils::Vector{create<sem::StructMember>(member, member->name->symbol, create<type::I32>(),
                                                0u, 0u, 0u, 4u, type::StructMemberAttributes{})},
        0u, 4u, 4u);

    auto* ctor_expr = Call(ty.Of(str));

    auto* var = Var("a", builtin::AddressSpace::kFunction, ctor_expr);
    WrapInFunction(var);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(TypeOf(var)->UnwrapRef(), expected_type);
}

}  // namespace
}  // namespace tint::resolver
