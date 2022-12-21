// Copyright 2022 The Tint Authors.
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

#include "src/tint/resolver/const_eval_test.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::resolver {
namespace {

TEST_F(ResolverConstEvalTest, MemberAccess) {
    Structure("Inner", utils::Vector{
                           Member("i1", ty.i32()),
                           Member("i2", ty.u32()),
                           Member("i3", ty.f32()),
                           Member("i4", ty.bool_()),
                       });

    Structure("Outer", utils::Vector{
                           Member("o1", ty.type_name("Inner")),
                           Member("o2", ty.type_name("Inner")),
                       });
    auto* outer_expr = Construct(ty.type_name("Outer"),  //
                                 Construct(ty.type_name("Inner"), 1_i, 2_u, 3_f, true),
                                 Construct(ty.type_name("Inner")));
    auto* o1_expr = MemberAccessor(outer_expr, "o1");
    auto* i2_expr = MemberAccessor(o1_expr, "i2");
    WrapInFunction(i2_expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* outer = Sem().Get(outer_expr);
    ASSERT_NE(outer, nullptr);
    auto* str = outer->Type()->As<sem::Struct>();
    ASSERT_NE(str, nullptr);
    EXPECT_EQ(str->Members().Length(), 2u);
    ASSERT_NE(outer->ConstantValue(), nullptr);
    EXPECT_TYPE(outer->ConstantValue()->Type(), outer->Type());
    EXPECT_FALSE(outer->ConstantValue()->AllEqual());
    EXPECT_TRUE(outer->ConstantValue()->AnyZero());
    EXPECT_FALSE(outer->ConstantValue()->AllZero());

    auto* o1 = Sem().Get(o1_expr);
    ASSERT_NE(o1->ConstantValue(), nullptr);
    EXPECT_FALSE(o1->ConstantValue()->AllEqual());
    EXPECT_FALSE(o1->ConstantValue()->AnyZero());
    EXPECT_FALSE(o1->ConstantValue()->AllZero());
    EXPECT_TRUE(o1->ConstantValue()->Type()->Is<sem::Struct>());
    EXPECT_EQ(o1->ConstantValue()->Index(0)->ValueAs<i32>(), 1_i);
    EXPECT_EQ(o1->ConstantValue()->Index(1)->ValueAs<u32>(), 2_u);
    EXPECT_EQ(o1->ConstantValue()->Index(2)->ValueAs<f32>(), 3_f);
    EXPECT_EQ(o1->ConstantValue()->Index(2)->ValueAs<bool>(), true);

    auto* i2 = Sem().Get(i2_expr);
    ASSERT_NE(i2->ConstantValue(), nullptr);
    EXPECT_TRUE(i2->ConstantValue()->AllEqual());
    EXPECT_FALSE(i2->ConstantValue()->AnyZero());
    EXPECT_FALSE(i2->ConstantValue()->AllZero());
    EXPECT_TRUE(i2->ConstantValue()->Type()->Is<type::U32>());
    EXPECT_EQ(i2->ConstantValue()->ValueAs<u32>(), 2_u);
}

TEST_F(ResolverConstEvalTest, Matrix_AFloat_Construct_From_AInt_Vectors) {
    auto* c = Const("a", Construct(ty.mat(nullptr, 2, 2),  //
                                   Construct(ty.vec(nullptr, 2), Expr(1_a), Expr(2_a)),
                                   Construct(ty.vec(nullptr, 2), Expr(3_a), Expr(4_a))));
    WrapInFunction(c);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(c);
    ASSERT_NE(sem, nullptr);
    EXPECT_TRUE(sem->Type()->Is<type::Matrix>());
    auto* cv = sem->ConstantValue();
    EXPECT_TYPE(cv->Type(), sem->Type());
    EXPECT_TRUE(cv->Index(0)->Type()->Is<type::Vector>());
    EXPECT_TRUE(cv->Index(0)->Index(0)->Type()->Is<type::AbstractFloat>());
    EXPECT_FALSE(cv->AllEqual());
    EXPECT_FALSE(cv->AnyZero());
    EXPECT_FALSE(cv->AllZero());
    auto* c0 = cv->Index(0);
    auto* c1 = cv->Index(1);
    EXPECT_EQ(c0->Index(0)->ValueAs<AFloat>(), 1.0);
    EXPECT_EQ(c0->Index(1)->ValueAs<AFloat>(), 2.0);
    EXPECT_EQ(c1->Index(0)->ValueAs<AFloat>(), 3.0);
    EXPECT_EQ(c1->Index(1)->ValueAs<AFloat>(), 4.0);
}
}  // namespace
}  // namespace tint::resolver
