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

TEST_F(ResolverConstEvalTest, StructMemberAccess) {
    Structure("Inner", utils::Vector{
                           Member("i1", ty.i32()),
                           Member("i2", ty.u32()),
                           Member("i3", ty.f32()),
                           Member("i4", ty.bool_()),
                       });

    Structure("Outer", utils::Vector{
                           Member("o1", ty("Inner")),
                           Member("o2", ty("Inner")),
                       });
    auto* outer_expr = Call("Outer",  //
                            Call("Inner", 1_i, 2_u, 3_f, true), Call("Inner"));
    auto* o1_expr = MemberAccessor(outer_expr, "o1");
    auto* i2_expr = MemberAccessor(o1_expr, "i2");
    WrapInFunction(i2_expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* outer = Sem().Get(outer_expr);
    ASSERT_NE(outer, nullptr);
    auto* str = outer->Type()->As<type::Struct>();
    ASSERT_NE(str, nullptr);
    EXPECT_EQ(str->Members().Length(), 2u);
    ASSERT_NE(outer->ConstantValue(), nullptr);
    EXPECT_TYPE(outer->ConstantValue()->Type(), outer->Type());
    EXPECT_TRUE(outer->ConstantValue()->AnyZero());
    EXPECT_FALSE(outer->ConstantValue()->AllZero());

    auto* o1 = Sem().Get(o1_expr);
    ASSERT_NE(o1->ConstantValue(), nullptr);
    EXPECT_FALSE(o1->ConstantValue()->AnyZero());
    EXPECT_FALSE(o1->ConstantValue()->AllZero());
    EXPECT_TRUE(o1->ConstantValue()->Type()->Is<type::Struct>());
    EXPECT_EQ(o1->ConstantValue()->Index(0)->ValueAs<i32>(), 1_i);
    EXPECT_EQ(o1->ConstantValue()->Index(1)->ValueAs<u32>(), 2_u);
    EXPECT_EQ(o1->ConstantValue()->Index(2)->ValueAs<f32>(), 3_f);
    EXPECT_EQ(o1->ConstantValue()->Index(2)->ValueAs<bool>(), true);

    auto* i2 = Sem().Get(i2_expr);
    ASSERT_NE(i2->ConstantValue(), nullptr);
    EXPECT_FALSE(i2->ConstantValue()->AnyZero());
    EXPECT_FALSE(i2->ConstantValue()->AllZero());
    EXPECT_TRUE(i2->ConstantValue()->Type()->Is<type::U32>());
    EXPECT_EQ(i2->ConstantValue()->ValueAs<u32>(), 2_u);
}

TEST_F(ResolverConstEvalTest, Matrix_AFloat_Construct_From_AInt_Vectors) {
    auto* c = Const("a", Call(ty.mat2x2<Infer>(),  //
                              Call(ty.vec<Infer>(2), Expr(1_a), Expr(2_a)),
                              Call(ty.vec<Infer>(2), Expr(3_a), Expr(4_a))));
    WrapInFunction(c);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(c);
    ASSERT_NE(sem, nullptr);
    EXPECT_TRUE(sem->Type()->Is<type::Matrix>());
    auto* cv = sem->ConstantValue();
    EXPECT_TYPE(cv->Type(), sem->Type());
    EXPECT_TRUE(cv->Index(0)->Type()->Is<type::Vector>());
    EXPECT_TRUE(cv->Index(0)->Index(0)->Type()->Is<type::AbstractFloat>());
    EXPECT_FALSE(cv->AnyZero());
    EXPECT_FALSE(cv->AllZero());
    auto* c0 = cv->Index(0);
    auto* c1 = cv->Index(1);
    EXPECT_EQ(c0->Index(0)->ValueAs<AFloat>(), 1.0);
    EXPECT_EQ(c0->Index(1)->ValueAs<AFloat>(), 2.0);
    EXPECT_EQ(c1->Index(0)->ValueAs<AFloat>(), 3.0);
    EXPECT_EQ(c1->Index(1)->ValueAs<AFloat>(), 4.0);
}

TEST_F(ResolverConstEvalTest, MatrixMemberAccess_AFloat) {
    auto* c = Const("a", Call(ty.mat2x3<Infer>(),  //
                              Call(ty.vec3<Infer>(), Expr(1.0_a), Expr(2.0_a), Expr(3.0_a)),
                              Call(ty.vec3<Infer>(), Expr(4.0_a), Expr(5.0_a), Expr(6.0_a))));

    auto* col_0 = Const("col_0", IndexAccessor("a", Expr(0_i)));
    auto* col_1 = Const("col_1", IndexAccessor("a", Expr(1_i)));
    auto* e00 = Const("e00", IndexAccessor("col_0", Expr(0_i)));
    auto* e01 = Const("e01", IndexAccessor("col_0", Expr(1_i)));
    auto* e02 = Const("e02", IndexAccessor("col_0", Expr(2_i)));
    auto* e10 = Const("e10", IndexAccessor("col_1", Expr(0_i)));
    auto* e11 = Const("e11", IndexAccessor("col_1", Expr(1_i)));
    auto* e12 = Const("e12", IndexAccessor("col_1", Expr(2_i)));

    (void)col_0;
    (void)col_1;

    WrapInFunction(c, col_0, col_1, e00, e01, e02, e10, e11, e12);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(c);
    ASSERT_NE(sem, nullptr);
    EXPECT_TRUE(sem->Type()->Is<type::Matrix>());
    auto* cv = sem->ConstantValue();
    EXPECT_TYPE(cv->Type(), sem->Type());
    EXPECT_TRUE(cv->Index(0)->Type()->Is<type::Vector>());
    EXPECT_TRUE(cv->Index(0)->Index(0)->Type()->Is<type::AbstractFloat>());
    EXPECT_FALSE(cv->AnyZero());
    EXPECT_FALSE(cv->AllZero());

    auto* sem_col0 = Sem().Get(col_0);
    ASSERT_NE(sem_col0, nullptr);
    EXPECT_TRUE(sem_col0->Type()->Is<type::Vector>());
    EXPECT_EQ(sem_col0->ConstantValue()->Index(0)->ValueAs<AFloat>(), 1.0);
    EXPECT_EQ(sem_col0->ConstantValue()->Index(1)->ValueAs<AFloat>(), 2.0);
    EXPECT_EQ(sem_col0->ConstantValue()->Index(2)->ValueAs<AFloat>(), 3.0);

    auto* sem_col1 = Sem().Get(col_1);
    ASSERT_NE(sem_col1, nullptr);
    EXPECT_TRUE(sem_col1->Type()->Is<type::Vector>());
    EXPECT_EQ(sem_col1->ConstantValue()->Index(0)->ValueAs<AFloat>(), 4.0);
    EXPECT_EQ(sem_col1->ConstantValue()->Index(1)->ValueAs<AFloat>(), 5.0);
    EXPECT_EQ(sem_col1->ConstantValue()->Index(2)->ValueAs<AFloat>(), 6.0);

    auto* sem_e00 = Sem().Get(e00);
    ASSERT_NE(sem_e00, nullptr);
    EXPECT_TRUE(sem_e00->Type()->Is<type::AbstractFloat>());
    EXPECT_EQ(sem_e00->ConstantValue()->ValueAs<AFloat>(), 1.0);

    auto* sem_e01 = Sem().Get(e01);
    ASSERT_NE(sem_e01, nullptr);
    EXPECT_TRUE(sem_e01->Type()->Is<type::AbstractFloat>());
    EXPECT_EQ(sem_e01->ConstantValue()->ValueAs<AFloat>(), 2.0);

    auto* sem_e02 = Sem().Get(e02);
    ASSERT_NE(sem_e02, nullptr);
    EXPECT_TRUE(sem_e02->Type()->Is<type::AbstractFloat>());
    EXPECT_EQ(sem_e02->ConstantValue()->ValueAs<AFloat>(), 3.0);

    auto* sem_e10 = Sem().Get(e10);
    ASSERT_NE(sem_e10, nullptr);
    EXPECT_TRUE(sem_e10->Type()->Is<type::AbstractFloat>());
    EXPECT_EQ(sem_e10->ConstantValue()->ValueAs<AFloat>(), 4.0);

    auto* sem_e11 = Sem().Get(e11);
    ASSERT_NE(sem_e11, nullptr);
    EXPECT_TRUE(sem_e11->Type()->Is<type::AbstractFloat>());
    EXPECT_EQ(sem_e11->ConstantValue()->ValueAs<AFloat>(), 5.0);

    auto* sem_e12 = Sem().Get(e12);
    ASSERT_NE(sem_e12, nullptr);
    EXPECT_TRUE(sem_e12->Type()->Is<type::AbstractFloat>());
    EXPECT_EQ(sem_e12->ConstantValue()->ValueAs<AFloat>(), 6.0);
}

TEST_F(ResolverConstEvalTest, MatrixMemberAccess_f32) {
    auto* c = Const("a", Call(ty.mat2x3<Infer>(),  //
                              Call(ty.vec3<Infer>(), Expr(1.0_f), Expr(2.0_f), Expr(3.0_f)),
                              Call(ty.vec3<Infer>(), Expr(4.0_f), Expr(5.0_f), Expr(6.0_f))));

    auto* col_0 = Const("col_0", IndexAccessor("a", Expr(0_i)));
    auto* col_1 = Const("col_1", IndexAccessor("a", Expr(1_i)));
    auto* e00 = Const("e00", IndexAccessor("col_0", Expr(0_i)));
    auto* e01 = Const("e01", IndexAccessor("col_0", Expr(1_i)));
    auto* e02 = Const("e02", IndexAccessor("col_0", Expr(2_i)));
    auto* e10 = Const("e10", IndexAccessor("col_1", Expr(0_i)));
    auto* e11 = Const("e11", IndexAccessor("col_1", Expr(1_i)));
    auto* e12 = Const("e12", IndexAccessor("col_1", Expr(2_i)));

    (void)col_0;
    (void)col_1;

    WrapInFunction(c, col_0, col_1, e00, e01, e02, e10, e11, e12);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(c);
    ASSERT_NE(sem, nullptr);
    EXPECT_TRUE(sem->Type()->Is<type::Matrix>());
    auto* cv = sem->ConstantValue();
    EXPECT_TYPE(cv->Type(), sem->Type());
    EXPECT_TRUE(cv->Index(0)->Type()->Is<type::Vector>());
    EXPECT_TRUE(cv->Index(0)->Index(0)->Type()->Is<type::F32>());
    EXPECT_FALSE(cv->AnyZero());
    EXPECT_FALSE(cv->AllZero());

    auto* sem_col0 = Sem().Get(col_0);
    ASSERT_NE(sem_col0, nullptr);
    EXPECT_TRUE(sem_col0->Type()->Is<type::Vector>());
    EXPECT_EQ(sem_col0->ConstantValue()->Index(0)->ValueAs<f32>(), 1.0f);
    EXPECT_EQ(sem_col0->ConstantValue()->Index(1)->ValueAs<f32>(), 2.0f);
    EXPECT_EQ(sem_col0->ConstantValue()->Index(2)->ValueAs<f32>(), 3.0f);

    auto* sem_col1 = Sem().Get(col_1);
    ASSERT_NE(sem_col1, nullptr);
    EXPECT_TRUE(sem_col1->Type()->Is<type::Vector>());
    EXPECT_EQ(sem_col1->ConstantValue()->Index(0)->ValueAs<f32>(), 4.0f);
    EXPECT_EQ(sem_col1->ConstantValue()->Index(1)->ValueAs<f32>(), 5.0f);
    EXPECT_EQ(sem_col1->ConstantValue()->Index(2)->ValueAs<f32>(), 6.0f);

    auto* sem_e00 = Sem().Get(e00);
    ASSERT_NE(sem_e00, nullptr);
    EXPECT_TRUE(sem_e00->Type()->Is<type::F32>());
    EXPECT_EQ(sem_e00->ConstantValue()->ValueAs<f32>(), 1.0f);

    auto* sem_e01 = Sem().Get(e01);
    ASSERT_NE(sem_e01, nullptr);
    EXPECT_TRUE(sem_e01->Type()->Is<type::F32>());
    EXPECT_EQ(sem_e01->ConstantValue()->ValueAs<f32>(), 2.0f);

    auto* sem_e02 = Sem().Get(e02);
    ASSERT_NE(sem_e02, nullptr);
    EXPECT_TRUE(sem_e02->Type()->Is<type::F32>());
    EXPECT_EQ(sem_e02->ConstantValue()->ValueAs<f32>(), 3.0f);

    auto* sem_e10 = Sem().Get(e10);
    ASSERT_NE(sem_e10, nullptr);
    EXPECT_TRUE(sem_e10->Type()->Is<type::F32>());
    EXPECT_EQ(sem_e10->ConstantValue()->ValueAs<f32>(), 4.0f);

    auto* sem_e11 = Sem().Get(e11);
    ASSERT_NE(sem_e11, nullptr);
    EXPECT_TRUE(sem_e11->Type()->Is<type::F32>());
    EXPECT_EQ(sem_e11->ConstantValue()->ValueAs<f32>(), 5.0f);

    auto* sem_e12 = Sem().Get(e12);
    ASSERT_NE(sem_e12, nullptr);
    EXPECT_TRUE(sem_e12->Type()->Is<type::F32>());
    EXPECT_EQ(sem_e12->ConstantValue()->ValueAs<f32>(), 6.0f);
}

namespace ArrayAccess {
struct Case {
    Value input;
};
static Case C(Value input) {
    return Case{std::move(input)};
}
static std::ostream& operator<<(std::ostream& o, const Case& c) {
    return o << "input: " << c.input;
}

using ResolverConstEvalArrayAccessTest = ResolverTestWithParam<Case>;
TEST_P(ResolverConstEvalArrayAccessTest, Test) {
    Enable(builtin::Extension::kF16);

    auto& param = GetParam();
    auto* expr = param.input.Expr(*this);
    auto* a = Const("a", expr);

    utils::Vector<const ast::IndexAccessorExpression*, 4> index_accessors;
    for (size_t i = 0; i < param.input.args.Length(); ++i) {
        auto* index = IndexAccessor("a", Expr(i32(i)));
        index_accessors.Push(index);
    }

    utils::Vector<const ast::Statement*, 5> stmts;
    stmts.Push(WrapInStatement(a));
    for (auto* ia : index_accessors) {
        stmts.Push(WrapInStatement(ia));
    }
    WrapInFunction(std::move(stmts));

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().GetVal(expr);
    ASSERT_NE(sem, nullptr);
    auto* arr = sem->Type()->As<type::Array>();
    ASSERT_NE(arr, nullptr);

    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    for (size_t i = 0; i < index_accessors.Length(); ++i) {
        auto* ia_sem = Sem().Get(index_accessors[i]);
        ASSERT_NE(ia_sem, nullptr);
        ASSERT_NE(ia_sem->ConstantValue(), nullptr);
        EXPECT_EQ(ia_sem->ConstantValue()->ValueAs<AInt>(), i);
    }
}
template <typename T>
std::vector<Case> ArrayAccessCases() {
    if constexpr (std::is_same_v<T, bool>) {
        return {
            C(Array(false, true)),
        };
    } else {
        return {
            C(Array(T(0))),                          //
            C(Array(T(0), T(1))),                    //
            C(Array(T(0), T(1), T(2))),              //
            C(Array(T(0), T(1), T(2), T(3))),        //
            C(Array(T(0), T(1), T(2), T(3), T(4))),  //
        };
    }
}
INSTANTIATE_TEST_SUITE_P(  //
    ArrayAccess,
    ResolverConstEvalArrayAccessTest,
    testing::ValuesIn(Concat(ArrayAccessCases<AInt>(),    //
                             ArrayAccessCases<AFloat>(),  //
                             ArrayAccessCases<i32>(),     //
                             ArrayAccessCases<u32>(),     //
                             ArrayAccessCases<f32>(),     //
                             ArrayAccessCases<f16>(),     //
                             ArrayAccessCases<bool>())));
}  // namespace ArrayAccess

namespace VectorAccess {
struct Case {
    Value input;
};
static Case C(Value input) {
    return Case{std::move(input)};
}
static std::ostream& operator<<(std::ostream& o, const Case& c) {
    return o << "input: " << c.input;
}

using ResolverConstEvalVectorAccessTest = ResolverTestWithParam<Case>;
TEST_P(ResolverConstEvalVectorAccessTest, Test) {
    Enable(builtin::Extension::kF16);

    auto& param = GetParam();
    auto* expr = param.input.Expr(*this);
    auto* a = Const("a", expr);

    utils::Vector<const ast::IndexAccessorExpression*, 4> index_accessors;
    for (size_t i = 0; i < param.input.args.Length(); ++i) {
        auto* index = IndexAccessor("a", Expr(i32(i)));
        index_accessors.Push(index);
    }

    utils::Vector<const ast::Statement*, 5> stmts;
    stmts.Push(WrapInStatement(a));
    for (auto* ia : index_accessors) {
        stmts.Push(WrapInStatement(ia));
    }
    WrapInFunction(std::move(stmts));

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().GetVal(expr);
    ASSERT_NE(sem, nullptr);
    auto* vec = sem->Type()->As<type::Vector>();
    ASSERT_NE(vec, nullptr);

    EXPECT_TYPE(sem->ConstantValue()->Type(), sem->Type());
    for (size_t i = 0; i < index_accessors.Length(); ++i) {
        auto* ia_sem = Sem().Get(index_accessors[i]);
        ASSERT_NE(ia_sem, nullptr);
        ASSERT_NE(ia_sem->ConstantValue(), nullptr);
        EXPECT_EQ(ia_sem->ConstantValue()->ValueAs<AInt>(), i);
    }
}
template <typename T>
std::vector<Case> VectorAccessCases() {
    if constexpr (std::is_same_v<T, bool>) {
        return {
            C(Vec(false, true)),
        };
    } else {
        return {
            C(Vec(T(0), T(1))),              //
            C(Vec(T(0), T(1), T(2))),        //
            C(Vec(T(0), T(1), T(2), T(3))),  //
        };
    }
}
INSTANTIATE_TEST_SUITE_P(  //
    VectorAccess,
    ResolverConstEvalVectorAccessTest,
    testing::ValuesIn(Concat(VectorAccessCases<AInt>(),    //
                             VectorAccessCases<AFloat>(),  //
                             VectorAccessCases<i32>(),     //
                             VectorAccessCases<u32>(),     //
                             VectorAccessCases<f32>(),     //
                             VectorAccessCases<f16>(),     //
                             VectorAccessCases<bool>())));
}  // namespace VectorAccess

}  // namespace
}  // namespace tint::resolver
