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

#include "gmock/gmock.h"
#include "src/tint/resolver/resolver_test_helper.h"
#include "src/tint/type/storage_texture.h"

using ::testing::HasSubstr;

using namespace tint::number_suffixes;  // NOLINT

namespace tint::resolver {
namespace {

using ResolverCompoundAssignmentValidationTest = ResolverTest;

TEST_F(ResolverCompoundAssignmentValidationTest, CompatibleTypes) {
    // var a : i32 = 2;
    // a += 2
    auto* var = Var("a", ty.i32(), Expr(2_i));
    WrapInFunction(var, CompoundAssign(Source{{12, 34}}, "a", 2_i, ast::BinaryOp::kAdd));

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverCompoundAssignmentValidationTest, CompatibleTypesThroughAlias) {
    // alias myint = i32;
    // var a : myint = 2;
    // a += 2
    auto* myint = Alias("myint", ty.i32());
    auto* var = Var("a", ty.Of(myint), Expr(2_i));
    WrapInFunction(var, CompoundAssign(Source{{12, 34}}, "a", 2_i, ast::BinaryOp::kAdd));

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverCompoundAssignmentValidationTest, CompatibleTypesAssignThroughPointer) {
    // var a : i32;
    // let b : ptr<function,i32> = &a;
    // *b += 2;
    const auto func = builtin::AddressSpace::kFunction;
    auto* var_a = Var("a", ty.i32(), func, Expr(2_i));
    auto* var_b = Let("b", ty.pointer<i32>(func), AddressOf(Expr("a")));
    WrapInFunction(var_a, var_b,
                   CompoundAssign(Source{{12, 34}}, Deref("b"), 2_i, ast::BinaryOp::kAdd));

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverCompoundAssignmentValidationTest, IncompatibleTypes) {
    // {
    //   var a : i32 = 2;
    //   a += 2.3;
    // }

    auto* var = Var("a", ty.i32(), Expr(2_i));

    auto* assign = CompoundAssign(Source{{12, 34}}, "a", 2.3_f, ast::BinaryOp::kAdd);
    WrapInFunction(var, assign);

    ASSERT_FALSE(r()->Resolve());

    EXPECT_THAT(r()->error(),
                HasSubstr("12:34 error: no matching overload for operator += (i32, f32)"));
}

TEST_F(ResolverCompoundAssignmentValidationTest, IncompatibleOp) {
    // {
    //   var a : f32 = 1.0;
    //   a |= 2.0;
    // }

    auto* var = Var("a", ty.f32(), Expr(1_f));

    auto* assign = CompoundAssign(Source{{12, 34}}, "a", 2_f, ast::BinaryOp::kOr);
    WrapInFunction(var, assign);

    ASSERT_FALSE(r()->Resolve());

    EXPECT_THAT(r()->error(),
                HasSubstr("12:34 error: no matching overload for operator |= (f32, f32)"));
}

TEST_F(ResolverCompoundAssignmentValidationTest, VectorScalar_Pass) {
    // {
    //   var a : vec4<f32>;
    //   a += 1.0;
    // }

    auto* var = Var("a", ty.vec4<f32>());

    auto* assign = CompoundAssign(Source{{12, 34}}, "a", 1_f, ast::BinaryOp::kAdd);
    WrapInFunction(var, assign);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverCompoundAssignmentValidationTest, ScalarVector_Fail) {
    // {
    //   var a : f32;
    //   a += vec4<f32>();
    // }

    auto* var = Var("a", ty.f32());

    auto* assign = CompoundAssign(Source{{12, 34}}, "a", vec4<f32>(), ast::BinaryOp::kAdd);
    WrapInFunction(var, assign);

    ASSERT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(), "12:34 error: cannot assign 'vec4<f32>' to 'f32'");
}

TEST_F(ResolverCompoundAssignmentValidationTest, MatrixScalar_Pass) {
    // {
    //   var a : mat4x4<f32>;
    //   a *= 2.0;
    // }

    auto* var = Var("a", ty.mat4x4<f32>());

    auto* assign = CompoundAssign(Source{{12, 34}}, "a", 2_f, ast::BinaryOp::kMultiply);
    WrapInFunction(var, assign);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverCompoundAssignmentValidationTest, ScalarMatrix_Fail) {
    // {
    //   var a : f32;
    //   a *= mat4x4();
    // }

    auto* var = Var("a", ty.f32());

    auto* assign = CompoundAssign(Source{{12, 34}}, "a", mat4x4<f32>(), ast::BinaryOp::kMultiply);
    WrapInFunction(var, assign);

    ASSERT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(), "12:34 error: cannot assign 'mat4x4<f32>' to 'f32'");
}

TEST_F(ResolverCompoundAssignmentValidationTest, VectorMatrix_Pass) {
    // {
    //   var a : vec4<f32>;
    //   a *= mat4x4();
    // }

    auto* var = Var("a", ty.vec4<f32>());

    auto* assign = CompoundAssign(Source{{12, 34}}, "a", mat4x4<f32>(), ast::BinaryOp::kMultiply);
    WrapInFunction(var, assign);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverCompoundAssignmentValidationTest, VectorMatrix_ColumnMismatch) {
    // {
    //   var a : vec4<f32>;
    //   a *= mat4x2();
    // }

    auto* var = Var("a", ty.vec4<f32>());

    auto* assign = CompoundAssign(Source{{12, 34}}, "a", mat4x2<f32>(), ast::BinaryOp::kMultiply);
    WrapInFunction(var, assign);

    ASSERT_FALSE(r()->Resolve());

    EXPECT_THAT(
        r()->error(),
        HasSubstr("12:34 error: no matching overload for operator *= (vec4<f32>, mat4x2<f32>)"));
}

TEST_F(ResolverCompoundAssignmentValidationTest, VectorMatrix_ResultMismatch) {
    // {
    //   var a : vec4<f32>;
    //   a *= mat2x4();
    // }

    auto* var = Var("a", ty.vec4<f32>());

    auto* assign = CompoundAssign(Source{{12, 34}}, "a", mat2x4<f32>(), ast::BinaryOp::kMultiply);
    WrapInFunction(var, assign);

    ASSERT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(), "12:34 error: cannot assign 'vec2<f32>' to 'vec4<f32>'");
}

TEST_F(ResolverCompoundAssignmentValidationTest, MatrixVector_Fail) {
    // {
    //   var a : mat4x4<f32>;
    //   a *= vec4();
    // }

    auto* var = Var("a", ty.mat4x4<f32>());

    auto* assign = CompoundAssign(Source{{12, 34}}, "a", vec4<f32>(), ast::BinaryOp::kMultiply);
    WrapInFunction(var, assign);

    ASSERT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(), "12:34 error: cannot assign 'vec4<f32>' to 'mat4x4<f32>'");
}

TEST_F(ResolverCompoundAssignmentValidationTest, Phony) {
    // {
    //   _ += 1i;
    // }
    WrapInFunction(CompoundAssign(Source{{56, 78}}, Phony(), 1_i, ast::BinaryOp::kAdd));
    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(r()->error(),
                HasSubstr("56:78 error: no matching overload for operator += (void, i32)"));
}

TEST_F(ResolverCompoundAssignmentValidationTest, ReadOnlyBuffer) {
    // @group(0) @binding(0) var<storage,read> a : i32;
    // {
    //   a += 1i;
    // }
    GlobalVar(Source{{12, 34}}, "a", ty.i32(), builtin::AddressSpace::kStorage,
              builtin::Access::kRead, Group(0_a), Binding(0_a));
    WrapInFunction(CompoundAssign(Source{{56, 78}}, "a", 1_i, ast::BinaryOp::kAdd));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "56:78 error: cannot store into a read-only type 'ref<storage, i32, read>'");
}

TEST_F(ResolverCompoundAssignmentValidationTest, LhsLet) {
    // let a = 1i;
    // a += 1i;
    auto* a = Let(Source{{12, 34}}, "a", Expr(1_i));
    WrapInFunction(a, CompoundAssign(Expr(Source{{56, 78}}, "a"), 1_i, ast::BinaryOp::kAdd));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(56:78 error: cannot assign to let 'a'
56:78 note: 'let' variables are immutable
12:34 note: let 'a' declared here)");
}

TEST_F(ResolverCompoundAssignmentValidationTest, LhsLiteral) {
    // 1i += 1i;
    WrapInFunction(CompoundAssign(Expr(Source{{56, 78}}, 1_i), 1_i, ast::BinaryOp::kAdd));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(56:78 error: cannot assign to value expression of type 'i32')");
}

TEST_F(ResolverCompoundAssignmentValidationTest, LhsAtomic) {
    // var<workgroup> a : atomic<i32>;
    // a += a;
    GlobalVar(Source{{12, 34}}, "a", ty.atomic(ty.i32()), builtin::AddressSpace::kWorkgroup);
    WrapInFunction(CompoundAssign(Source{{56, 78}}, "a", "a", ast::BinaryOp::kAdd));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(
        r()->error(),
        HasSubstr("error: no matching overload for operator += (atomic<i32>, atomic<i32>)"));
}

}  // namespace
}  // namespace tint::resolver
