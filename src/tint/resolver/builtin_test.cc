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
#include "src/tint/ast/assignment_statement.h"
#include "src/tint/ast/bitcast_expression.h"
#include "src/tint/ast/break_statement.h"
#include "src/tint/ast/builtin_texture_helper_test.h"
#include "src/tint/ast/call_statement.h"
#include "src/tint/ast/continue_statement.h"
#include "src/tint/ast/if_statement.h"
#include "src/tint/ast/loop_statement.h"
#include "src/tint/ast/return_statement.h"
#include "src/tint/ast/stage_attribute.h"
#include "src/tint/ast/switch_statement.h"
#include "src/tint/ast/unary_op_expression.h"
#include "src/tint/ast/variable_decl_statement.h"
#include "src/tint/resolver/resolver_test_helper.h"
#include "src/tint/sem/call.h"
#include "src/tint/sem/function.h"
#include "src/tint/sem/member_accessor_expression.h"
#include "src/tint/sem/statement.h"
#include "src/tint/sem/variable.h"
#include "src/tint/type/sampled_texture.h"
#include "src/tint/type/test_helper.h"
#include "src/tint/type/texture_dimension.h"
#include "src/tint/utils/string.h"
#include "src/tint/utils/string_stream.h"

using ::testing::ElementsAre;
using ::testing::HasSubstr;

using namespace tint::number_suffixes;  // NOLINT

namespace tint::resolver {
namespace {

using ExpressionList = utils::Vector<const ast::Expression*, 8>;

using ResolverBuiltinTest = ResolverTest;

struct BuiltinData {
    const char* name;
    builtin::Function builtin;
};

inline std::ostream& operator<<(std::ostream& out, BuiltinData data) {
    out << data.name;
    return out;
}

TEST_F(ResolverBuiltinTest, ModuleScopeUsage) {
    GlobalConst("c", ty.f32(), Call(Source{{12, 34}}, "dpdy", 1._f));

    EXPECT_FALSE(r()->Resolve());

    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: const initializer requires a const-expression, but expression is a runtime-expression)");
}

// Tests for Logical builtins
namespace logical_builtin_tests {

using ResolverBuiltinTest_BoolMethod = ResolverTestWithParam<std::string>;

TEST_P(ResolverBuiltinTest_BoolMethod, Scalar) {
    auto name = GetParam();

    GlobalVar("my_var", ty.bool_(), builtin::AddressSpace::kPrivate);

    auto* expr = Call(name, "my_var");
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(expr), nullptr);
    EXPECT_TRUE(TypeOf(expr)->Is<type::Bool>());
}
TEST_P(ResolverBuiltinTest_BoolMethod, Vector) {
    auto name = GetParam();

    GlobalVar("my_var", ty.vec3<bool>(), builtin::AddressSpace::kPrivate);

    auto* expr = Call(name, "my_var");
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(expr), nullptr);
    EXPECT_TRUE(TypeOf(expr)->Is<type::Bool>());
}
INSTANTIATE_TEST_SUITE_P(ResolverTest,
                         ResolverBuiltinTest_BoolMethod,
                         testing::Values("any", "all"));

TEST_F(ResolverBuiltinTest, Select) {
    GlobalVar("my_var", ty.vec3<f32>(), builtin::AddressSpace::kPrivate);

    GlobalVar("bool_var", ty.vec3<bool>(), builtin::AddressSpace::kPrivate);

    auto* expr = Call("select", "my_var", "my_var", "bool_var");
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(expr), nullptr);
    EXPECT_TRUE(TypeOf(expr)->Is<type::Vector>());
    EXPECT_EQ(TypeOf(expr)->As<type::Vector>()->Width(), 3u);
    EXPECT_TRUE(TypeOf(expr)->As<type::Vector>()->type()->Is<type::F32>());
}

TEST_F(ResolverBuiltinTest, Select_Error_NoParams) {
    auto* expr = Call("select");
    WrapInFunction(expr);

    EXPECT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(),
              R"(error: no matching call to select()

3 candidate functions:
  select(T, T, bool) -> T  where: T is abstract-int, abstract-float, f32, f16, i32, u32 or bool
  select(vecN<T>, vecN<T>, bool) -> vecN<T>  where: T is abstract-int, abstract-float, f32, f16, i32, u32 or bool
  select(vecN<T>, vecN<T>, vecN<bool>) -> vecN<T>  where: T is abstract-int, abstract-float, f32, f16, i32, u32 or bool
)");
}

TEST_F(ResolverBuiltinTest, Select_Error_SelectorInt) {
    auto* expr = Call("select", 1_i, 1_i, 1_i);
    WrapInFunction(expr);

    EXPECT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(),
              R"(error: no matching call to select(i32, i32, i32)

3 candidate functions:
  select(T, T, bool) -> T  where: T is abstract-int, abstract-float, f32, f16, i32, u32 or bool
  select(vecN<T>, vecN<T>, bool) -> vecN<T>  where: T is abstract-int, abstract-float, f32, f16, i32, u32 or bool
  select(vecN<T>, vecN<T>, vecN<bool>) -> vecN<T>  where: T is abstract-int, abstract-float, f32, f16, i32, u32 or bool
)");
}

TEST_F(ResolverBuiltinTest, Select_Error_Matrix) {
    auto* expr = Call("select", mat2x2<f32>(vec2<f32>(1_f, 1_f), vec2<f32>(1_f, 1_f)),
                      mat2x2<f32>(vec2<f32>(1_f, 1_f), vec2<f32>(1_f, 1_f)), Expr(true));
    WrapInFunction(expr);

    EXPECT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(),
              R"(error: no matching call to select(mat2x2<f32>, mat2x2<f32>, bool)

3 candidate functions:
  select(T, T, bool) -> T  where: T is abstract-int, abstract-float, f32, f16, i32, u32 or bool
  select(vecN<T>, vecN<T>, bool) -> vecN<T>  where: T is abstract-int, abstract-float, f32, f16, i32, u32 or bool
  select(vecN<T>, vecN<T>, vecN<bool>) -> vecN<T>  where: T is abstract-int, abstract-float, f32, f16, i32, u32 or bool
)");
}

TEST_F(ResolverBuiltinTest, Select_Error_MismatchTypes) {
    auto* expr = Call("select", 1_f, vec2<f32>(2_f, 3_f), Expr(true));
    WrapInFunction(expr);

    EXPECT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(),
              R"(error: no matching call to select(f32, vec2<f32>, bool)

3 candidate functions:
  select(T, T, bool) -> T  where: T is abstract-int, abstract-float, f32, f16, i32, u32 or bool
  select(vecN<T>, vecN<T>, bool) -> vecN<T>  where: T is abstract-int, abstract-float, f32, f16, i32, u32 or bool
  select(vecN<T>, vecN<T>, vecN<bool>) -> vecN<T>  where: T is abstract-int, abstract-float, f32, f16, i32, u32 or bool
)");
}

TEST_F(ResolverBuiltinTest, Select_Error_MismatchVectorSize) {
    auto* expr = Call("select", vec2<f32>(1_f, 2_f), vec3<f32>(3_f, 4_f, 5_f), Expr(true));
    WrapInFunction(expr);

    EXPECT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(),
              R"(error: no matching call to select(vec2<f32>, vec3<f32>, bool)

3 candidate functions:
  select(T, T, bool) -> T  where: T is abstract-int, abstract-float, f32, f16, i32, u32 or bool
  select(vecN<T>, vecN<T>, bool) -> vecN<T>  where: T is abstract-int, abstract-float, f32, f16, i32, u32 or bool
  select(vecN<T>, vecN<T>, vecN<bool>) -> vecN<T>  where: T is abstract-int, abstract-float, f32, f16, i32, u32 or bool
)");
}

}  // namespace logical_builtin_tests

// Tests for Array builtins
namespace array_builtin_tests {

using ResolverBuiltinArrayTest = ResolverTest;

TEST_F(ResolverBuiltinArrayTest, ArrayLength_Vector) {
    auto ary = ty.array<i32>();
    auto* str = Structure("S", utils::Vector{Member("x", ary)});
    GlobalVar("a", ty.Of(str), builtin::AddressSpace::kStorage, builtin::Access::kRead,
              Binding(0_a), Group(0_a));

    auto* call = Call("arrayLength", AddressOf(MemberAccessor("a", "x")));
    WrapInFunction(call);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(call), nullptr);
    EXPECT_TRUE(TypeOf(call)->Is<type::U32>());
}

TEST_F(ResolverBuiltinArrayTest, ArrayLength_Error_ArraySized) {
    GlobalVar("arr", ty.array<i32, 4>(), builtin::AddressSpace::kPrivate);
    auto* call = Call("arrayLength", AddressOf("arr"));
    WrapInFunction(call);

    EXPECT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(),
              R"(error: no matching call to arrayLength(ptr<private, array<i32, 4>, read_write>)

1 candidate function:
  arrayLength(ptr<storage, array<T>, A>) -> u32
)");
}

}  // namespace array_builtin_tests

// Tests for Numeric builtins with float parameter
namespace float_builtin_tests {

// Testcase parameters for float built-in having signature of (T, ...) -> T and (vecN<T>, ...) ->
// vecN<T>
struct BuiltinDataWithParamNum {
    uint32_t args_number;
    const char* name;
    builtin::Function builtin;
};

inline std::ostream& operator<<(std::ostream& out, BuiltinDataWithParamNum data) {
    out << data.name;
    return out;
}

// Tests for float built-ins that has signiture (T, ...) -> T and (vecN<T>, ...) -> vecN<T>
using ResolverBuiltinTest_FloatBuiltin_IdenticalType =
    ResolverTestWithParam<BuiltinDataWithParamNum>;

TEST_P(ResolverBuiltinTest_FloatBuiltin_IdenticalType, Error_NoParams) {
    auto param = GetParam();

    auto* call = Call(param.name);
    WrapInFunction(call);

    EXPECT_FALSE(r()->Resolve());

    EXPECT_THAT(r()->error(),
                HasSubstr("error: no matching call to " + std::string(param.name) + "()"));
}

TEST_P(ResolverBuiltinTest_FloatBuiltin_IdenticalType, OneParam_Scalar_f32) {
    auto param = GetParam();

    auto val = 0.5_f;
    if (param.name == std::string("acosh")) {
        val = 1.0_f;
    }

    auto* call = Call(param.name, val);
    WrapInFunction(call);

    if (param.args_number == 1u) {
        // Parameter count matched.
        EXPECT_TRUE(r()->Resolve()) << r()->error();

        ASSERT_NE(TypeOf(call), nullptr);
        EXPECT_TRUE(TypeOf(call)->Is<type::F32>());
    } else {
        // Invalid parameter count.
        EXPECT_FALSE(r()->Resolve());

        EXPECT_THAT(r()->error(),
                    HasSubstr("error: no matching call to " + std::string(param.name) + "(f32)"));
    }
}

TEST_P(ResolverBuiltinTest_FloatBuiltin_IdenticalType, OneParam_Vector_f32) {
    auto param = GetParam();

    auto val = param.name == std::string("acosh") ? vec3<f32>(1.0_f, 2.0_f, 3.0_f)
                                                  : vec3<f32>(0.5_f, 0.5_f, 0.8_f);

    auto* call = Call(param.name, val);
    WrapInFunction(call);

    if (param.args_number == 1u) {
        // Parameter count matched.
        EXPECT_TRUE(r()->Resolve()) << r()->error();

        ASSERT_NE(TypeOf(call), nullptr);
        EXPECT_TRUE(TypeOf(call)->is_float_vector());
        EXPECT_EQ(TypeOf(call)->As<type::Vector>()->Width(), 3u);
        ASSERT_NE(TypeOf(call)->As<type::Vector>()->type(), nullptr);
        EXPECT_TRUE(TypeOf(call)->As<type::Vector>()->type()->Is<type::F32>());
    } else {
        // Invalid parameter count.
        EXPECT_FALSE(r()->Resolve());

        EXPECT_THAT(r()->error(), HasSubstr("error: no matching call to " +
                                            std::string(param.name) + "(vec3<f32>)"));
    }
}

TEST_P(ResolverBuiltinTest_FloatBuiltin_IdenticalType, TwoParams_Scalar_f32) {
    auto param = GetParam();

    auto* call = Call(param.name, 1_f, 1_f);
    WrapInFunction(call);

    if (param.args_number == 2u) {
        // Parameter count matched.
        EXPECT_TRUE(r()->Resolve()) << r()->error();

        ASSERT_NE(TypeOf(call), nullptr);
        EXPECT_TRUE(TypeOf(call)->Is<type::F32>());
    } else {
        // Invalid parameter count.
        EXPECT_FALSE(r()->Resolve());

        EXPECT_THAT(r()->error(), HasSubstr("error: no matching call to " +
                                            std::string(param.name) + "(f32, f32)"));
    }
}

TEST_P(ResolverBuiltinTest_FloatBuiltin_IdenticalType, TwoParams_Vector_f32) {
    auto param = GetParam();

    auto* call = Call(param.name, vec3<f32>(1_f, 1_f, 3_f), vec3<f32>(1_f, 1_f, 3_f));
    WrapInFunction(call);

    if (param.args_number == 2u) {
        // Parameter count matched.
        EXPECT_TRUE(r()->Resolve()) << r()->error();

        ASSERT_NE(TypeOf(call), nullptr);
        EXPECT_TRUE(TypeOf(call)->is_float_vector());
        EXPECT_EQ(TypeOf(call)->As<type::Vector>()->Width(), 3u);
        ASSERT_NE(TypeOf(call)->As<type::Vector>()->type(), nullptr);
        EXPECT_TRUE(TypeOf(call)->As<type::Vector>()->type()->Is<type::F32>());
    } else {
        // Invalid parameter count.
        EXPECT_FALSE(r()->Resolve());

        EXPECT_THAT(r()->error(), HasSubstr("error: no matching call to " +
                                            std::string(param.name) + "(vec3<f32>, vec3<f32>)"));
    }
}

TEST_P(ResolverBuiltinTest_FloatBuiltin_IdenticalType, ThreeParams_Scalar_f32) {
    auto param = GetParam();

    auto* call = Call(param.name, 0_f, 1_f, 2_f);
    WrapInFunction(call);

    if (param.args_number == 3u) {
        // Parameter count matched.
        EXPECT_TRUE(r()->Resolve()) << r()->error();

        ASSERT_NE(TypeOf(call), nullptr);
        EXPECT_TRUE(TypeOf(call)->Is<type::F32>());
    } else {
        // Invalid parameter count.
        EXPECT_FALSE(r()->Resolve());

        EXPECT_THAT(r()->error(), HasSubstr("error: no matching call to " +
                                            std::string(param.name) + "(f32, f32, f32)"));
    }
}

TEST_P(ResolverBuiltinTest_FloatBuiltin_IdenticalType, ThreeParams_Vector_f32) {
    auto param = GetParam();

    auto* call = Call(param.name, vec3<f32>(0_f, 0_f, 0_f), vec3<f32>(1_f, 1_f, 1_f),
                      vec3<f32>(2_f, 2_f, 2_f));
    WrapInFunction(call);

    if (param.args_number == 3u) {
        // Parameter count matched.
        EXPECT_TRUE(r()->Resolve()) << r()->error();

        ASSERT_NE(TypeOf(call), nullptr);
        EXPECT_TRUE(TypeOf(call)->is_float_vector());
        EXPECT_EQ(TypeOf(call)->As<type::Vector>()->Width(), 3u);
        ASSERT_NE(TypeOf(call)->As<type::Vector>()->type(), nullptr);
        EXPECT_TRUE(TypeOf(call)->As<type::Vector>()->type()->Is<type::F32>());
    } else {
        // Invalid parameter count.
        EXPECT_FALSE(r()->Resolve());

        EXPECT_THAT(r()->error(),
                    HasSubstr("error: no matching call to " + std::string(param.name) +
                              "(vec3<f32>, vec3<f32>, vec3<f32>)"));
    }
}

TEST_P(ResolverBuiltinTest_FloatBuiltin_IdenticalType, FourParams_Scalar_f32) {
    auto param = GetParam();

    auto* call = Call(param.name, 1_f, 1_f, 1_f, 1_f);
    WrapInFunction(call);

    if (param.args_number == 4u) {
        // Parameter count matched.
        EXPECT_TRUE(r()->Resolve()) << r()->error();

        ASSERT_NE(TypeOf(call), nullptr);
        EXPECT_TRUE(TypeOf(call)->Is<type::F32>());
    } else {
        // Invalid parameter count.
        EXPECT_FALSE(r()->Resolve());

        EXPECT_THAT(r()->error(), HasSubstr("error: no matching call to " +
                                            std::string(param.name) + "(f32, f32, f32, f32)"));
    }
}

TEST_P(ResolverBuiltinTest_FloatBuiltin_IdenticalType, FourParams_Vector_f32) {
    auto param = GetParam();

    auto* call = Call(param.name, vec3<f32>(1_f, 1_f, 3_f), vec3<f32>(1_f, 1_f, 3_f),
                      vec3<f32>(1_f, 1_f, 3_f), vec3<f32>(1_f, 1_f, 3_f));
    WrapInFunction(call);

    if (param.args_number == 4u) {
        // Parameter count matched.
        EXPECT_TRUE(r()->Resolve()) << r()->error();

        ASSERT_NE(TypeOf(call), nullptr);
        EXPECT_TRUE(TypeOf(call)->is_float_vector());
        EXPECT_EQ(TypeOf(call)->As<type::Vector>()->Width(), 3u);
        ASSERT_NE(TypeOf(call)->As<type::Vector>()->type(), nullptr);
        EXPECT_TRUE(TypeOf(call)->As<type::Vector>()->type()->Is<type::F32>());
    } else {
        // Invalid parameter count.
        EXPECT_FALSE(r()->Resolve());

        EXPECT_THAT(r()->error(),
                    HasSubstr("error: no matching call to " + std::string(param.name) +
                              "(vec3<f32>, vec3<f32>, vec3<f32>, vec3<f32>)"));
    }
}

TEST_P(ResolverBuiltinTest_FloatBuiltin_IdenticalType, OneParam_Scalar_f16) {
    auto param = GetParam();

    Enable(builtin::Extension::kF16);

    auto val = 0.5_h;
    if (param.name == std::string("acosh")) {
        val = 1.0_h;
    }

    auto* call = Call(param.name, val);
    WrapInFunction(call);

    if (param.args_number == 1u) {
        // Parameter count matched.
        EXPECT_TRUE(r()->Resolve()) << r()->error();

        ASSERT_NE(TypeOf(call), nullptr);
        EXPECT_TRUE(TypeOf(call)->Is<type::F16>());
    } else {
        // Invalid parameter count.
        EXPECT_FALSE(r()->Resolve());

        EXPECT_THAT(r()->error(),
                    HasSubstr("error: no matching call to " + std::string(param.name) + "(f16)"));
    }
}

TEST_P(ResolverBuiltinTest_FloatBuiltin_IdenticalType, OneParam_Vector_f16) {
    auto param = GetParam();

    Enable(builtin::Extension::kF16);

    auto val = param.name == std::string("acosh") ? vec3<f16>(1.0_h, 2.0_h, 3.0_h)
                                                  : vec3<f16>(0.5_h, 0.5_h, 0.8_h);

    auto* call = Call(param.name, val);
    WrapInFunction(call);

    if (param.args_number == 1u) {
        // Parameter count matched.
        EXPECT_TRUE(r()->Resolve()) << r()->error();

        ASSERT_NE(TypeOf(call), nullptr);
        EXPECT_TRUE(TypeOf(call)->is_float_vector());
        EXPECT_EQ(TypeOf(call)->As<type::Vector>()->Width(), 3u);
        ASSERT_NE(TypeOf(call)->As<type::Vector>()->type(), nullptr);
        EXPECT_TRUE(TypeOf(call)->As<type::Vector>()->type()->Is<type::F16>());
    } else {
        // Invalid parameter count.
        EXPECT_FALSE(r()->Resolve());

        EXPECT_THAT(r()->error(), HasSubstr("error: no matching call to " +
                                            std::string(param.name) + "(vec3<f16>)"));
    }
}

TEST_P(ResolverBuiltinTest_FloatBuiltin_IdenticalType, TwoParams_Scalar_f16) {
    auto param = GetParam();

    Enable(builtin::Extension::kF16);

    auto* call = Call(param.name, 1_h, 1_h);
    WrapInFunction(call);

    if (param.args_number == 2u) {
        // Parameter count matched.
        EXPECT_TRUE(r()->Resolve()) << r()->error();

        ASSERT_NE(TypeOf(call), nullptr);
        EXPECT_TRUE(TypeOf(call)->Is<type::F16>());
    } else {
        // Invalid parameter count.
        EXPECT_FALSE(r()->Resolve());

        EXPECT_THAT(r()->error(), HasSubstr("error: no matching call to " +
                                            std::string(param.name) + "(f16, f16)"));
    }
}

TEST_P(ResolverBuiltinTest_FloatBuiltin_IdenticalType, TwoParams_Vector_f16) {
    auto param = GetParam();

    Enable(builtin::Extension::kF16);

    auto* call = Call(param.name, vec3<f16>(1_h, 1_h, 3_h), vec3<f16>(1_h, 1_h, 3_h));
    WrapInFunction(call);

    if (param.args_number == 2u) {
        // Parameter count matched.
        EXPECT_TRUE(r()->Resolve()) << r()->error();

        ASSERT_NE(TypeOf(call), nullptr);
        EXPECT_TRUE(TypeOf(call)->is_float_vector());
        EXPECT_EQ(TypeOf(call)->As<type::Vector>()->Width(), 3u);
        ASSERT_NE(TypeOf(call)->As<type::Vector>()->type(), nullptr);
        EXPECT_TRUE(TypeOf(call)->As<type::Vector>()->type()->Is<type::F16>());
    } else {
        // Invalid parameter count.
        EXPECT_FALSE(r()->Resolve());

        EXPECT_THAT(r()->error(), HasSubstr("error: no matching call to " +
                                            std::string(param.name) + "(vec3<f16>, vec3<f16>)"));
    }
}

TEST_P(ResolverBuiltinTest_FloatBuiltin_IdenticalType, ThreeParams_Scalar_f16) {
    auto param = GetParam();

    Enable(builtin::Extension::kF16);

    auto* call = Call(param.name, 0_h, 1_h, 2_h);
    WrapInFunction(call);

    if (param.args_number == 3u) {
        // Parameter count matched.
        EXPECT_TRUE(r()->Resolve()) << r()->error();

        ASSERT_NE(TypeOf(call), nullptr);
        EXPECT_TRUE(TypeOf(call)->Is<type::F16>());
    } else {
        // Invalid parameter count.
        EXPECT_FALSE(r()->Resolve());

        EXPECT_THAT(r()->error(), HasSubstr("error: no matching call to " +
                                            std::string(param.name) + "(f16, f16, f16)"));
    }
}

TEST_P(ResolverBuiltinTest_FloatBuiltin_IdenticalType, ThreeParams_Vector_f16) {
    auto param = GetParam();

    Enable(builtin::Extension::kF16);

    auto* call = Call(param.name, vec3<f16>(0_h, 0_h, 0_h), vec3<f16>(1_h, 1_h, 1_h),
                      vec3<f16>(2_h, 2_h, 2_h));
    WrapInFunction(call);

    if (param.args_number == 3u) {
        // Parameter count matched.
        EXPECT_TRUE(r()->Resolve()) << r()->error();

        ASSERT_NE(TypeOf(call), nullptr);
        EXPECT_TRUE(TypeOf(call)->is_float_vector());
        EXPECT_EQ(TypeOf(call)->As<type::Vector>()->Width(), 3u);
        ASSERT_NE(TypeOf(call)->As<type::Vector>()->type(), nullptr);
        EXPECT_TRUE(TypeOf(call)->As<type::Vector>()->type()->Is<type::F16>());
    } else {
        // Invalid parameter count.
        EXPECT_FALSE(r()->Resolve());

        EXPECT_THAT(r()->error(),
                    HasSubstr("error: no matching call to " + std::string(param.name) +
                              "(vec3<f16>, vec3<f16>, vec3<f16>)"));
    }
}

TEST_P(ResolverBuiltinTest_FloatBuiltin_IdenticalType, FourParams_Scalar_f16) {
    auto param = GetParam();

    Enable(builtin::Extension::kF16);

    auto* call = Call(param.name, 1_h, 1_h, 1_h, 1_h);
    WrapInFunction(call);

    if (param.args_number == 4u) {
        // Parameter count matched.
        EXPECT_TRUE(r()->Resolve()) << r()->error();

        ASSERT_NE(TypeOf(call), nullptr);
        EXPECT_TRUE(TypeOf(call)->Is<type::F16>());
    } else {
        // Invalid parameter count.
        EXPECT_FALSE(r()->Resolve());

        EXPECT_THAT(r()->error(), HasSubstr("error: no matching call to " +
                                            std::string(param.name) + "(f16, f16, f16, f16)"));
    }
}

TEST_P(ResolverBuiltinTest_FloatBuiltin_IdenticalType, FourParams_Vector_f16) {
    auto param = GetParam();

    Enable(builtin::Extension::kF16);

    auto* call = Call(param.name, vec3<f16>(1_h, 1_h, 3_h), vec3<f16>(1_h, 1_h, 3_h),
                      vec3<f16>(1_h, 1_h, 3_h), vec3<f16>(1_h, 1_h, 3_h));
    WrapInFunction(call);

    if (param.args_number == 4u) {
        // Parameter count matched.
        EXPECT_TRUE(r()->Resolve()) << r()->error();

        ASSERT_NE(TypeOf(call), nullptr);
        EXPECT_TRUE(TypeOf(call)->is_float_vector());
        EXPECT_EQ(TypeOf(call)->As<type::Vector>()->Width(), 3u);
        ASSERT_NE(TypeOf(call)->As<type::Vector>()->type(), nullptr);
        EXPECT_TRUE(TypeOf(call)->As<type::Vector>()->type()->Is<type::F16>());
    } else {
        // Invalid parameter count.
        EXPECT_FALSE(r()->Resolve());

        EXPECT_THAT(r()->error(),
                    HasSubstr("error: no matching call to " + std::string(param.name) +
                              "(vec3<f16>, vec3<f16>, vec3<f16>, vec3<f16>)"));
    }
}

INSTANTIATE_TEST_SUITE_P(
    ResolverTest,
    ResolverBuiltinTest_FloatBuiltin_IdenticalType,
    testing::Values(BuiltinDataWithParamNum{1, "abs", builtin::Function::kAbs},
                    BuiltinDataWithParamNum{1, "acos", builtin::Function::kAcos},
                    BuiltinDataWithParamNum{1, "acosh", builtin::Function::kAcos},
                    BuiltinDataWithParamNum{1, "asin", builtin::Function::kAsin},
                    BuiltinDataWithParamNum{1, "asinh", builtin::Function::kAsin},
                    BuiltinDataWithParamNum{1, "atan", builtin::Function::kAtan},
                    BuiltinDataWithParamNum{1, "atanh", builtin::Function::kAtan},
                    BuiltinDataWithParamNum{2, "atan2", builtin::Function::kAtan2},
                    BuiltinDataWithParamNum{1, "ceil", builtin::Function::kCeil},
                    BuiltinDataWithParamNum{3, "clamp", builtin::Function::kClamp},
                    BuiltinDataWithParamNum{1, "cos", builtin::Function::kCos},
                    BuiltinDataWithParamNum{1, "cosh", builtin::Function::kCosh},
                    // cross: (vec3<T>, vec3<T>) -> vec3<T>
                    BuiltinDataWithParamNum{1, "degrees", builtin::Function::kDegrees},
                    // distance: (T, T) -> T, (vecN<T>, vecN<T>) -> T
                    BuiltinDataWithParamNum{1, "exp", builtin::Function::kExp},
                    BuiltinDataWithParamNum{1, "exp2", builtin::Function::kExp2},
                    // faceForward: (vecN<T>, vecN<T>, vecN<T>) -> vecN<T>
                    BuiltinDataWithParamNum{1, "floor", builtin::Function::kFloor},
                    BuiltinDataWithParamNum{3, "fma", builtin::Function::kFma},
                    BuiltinDataWithParamNum{1, "fract", builtin::Function::kFract},
                    // frexp
                    BuiltinDataWithParamNum{1, "inverseSqrt", builtin::Function::kInverseSqrt},
                    // ldexp: (T, i32) -> T, (vecN<T>, vecN<i32>) -> vecN<T>
                    // length: (vecN<T>) -> T
                    BuiltinDataWithParamNum{1, "log", builtin::Function::kLog},
                    BuiltinDataWithParamNum{1, "log2", builtin::Function::kLog2},
                    BuiltinDataWithParamNum{2, "max", builtin::Function::kMax},
                    BuiltinDataWithParamNum{2, "min", builtin::Function::kMin},
                    // Note that `mix(vecN<f32>, vecN<f32>, f32) -> vecN<f32>` is not tested here.
                    BuiltinDataWithParamNum{3, "mix", builtin::Function::kMix},
                    // modf
                    // normalize: (vecN<T>) -> vecN<T>
                    BuiltinDataWithParamNum{2, "pow", builtin::Function::kPow},
                    // quantizeToF16 is not implemented yet.
                    BuiltinDataWithParamNum{1, "radians", builtin::Function::kRadians},
                    // reflect: (vecN<T>, vecN<T>) -> vecN<T>
                    // refract: (vecN<T>, vecN<T>, T) -> vecN<T>
                    BuiltinDataWithParamNum{1, "round", builtin::Function::kRound},
                    // saturate not implemented yet.
                    BuiltinDataWithParamNum{1, "sign", builtin::Function::kSign},
                    BuiltinDataWithParamNum{1, "sin", builtin::Function::kSin},
                    BuiltinDataWithParamNum{1, "sinh", builtin::Function::kSinh},
                    BuiltinDataWithParamNum{3, "smoothstep", builtin::Function::kSmoothstep},
                    BuiltinDataWithParamNum{1, "sqrt", builtin::Function::kSqrt},
                    BuiltinDataWithParamNum{2, "step", builtin::Function::kStep},
                    BuiltinDataWithParamNum{1, "tan", builtin::Function::kTan},
                    BuiltinDataWithParamNum{1, "tanh", builtin::Function::kTanh},
                    BuiltinDataWithParamNum{1, "trunc", builtin::Function::kTrunc}));

using ResolverBuiltinFloatTest = ResolverTest;

// cross: (vec3<T>, vec3<T>) -> vec3<T>
TEST_F(ResolverBuiltinFloatTest, Cross_f32) {
    auto* call = Call("cross", vec3<f32>(1_f, 2_f, 3_f), vec3<f32>(1_f, 2_f, 3_f));
    WrapInFunction(call);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(call), nullptr);
    EXPECT_TRUE(TypeOf(call)->is_float_vector());
    EXPECT_EQ(TypeOf(call)->As<type::Vector>()->Width(), 3u);
    EXPECT_TRUE(TypeOf(call)->As<type::Vector>()->type()->Is<type::F32>());
}

TEST_F(ResolverBuiltinFloatTest, Cross_f16) {
    Enable(builtin::Extension::kF16);

    auto* call = Call("cross", vec3<f16>(1_h, 2_h, 3_h), vec3<f16>(1_h, 2_h, 3_h));
    WrapInFunction(call);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(call), nullptr);
    EXPECT_TRUE(TypeOf(call)->is_float_vector());
    EXPECT_EQ(TypeOf(call)->As<type::Vector>()->Width(), 3u);
    EXPECT_TRUE(TypeOf(call)->As<type::Vector>()->type()->Is<type::F16>());
}

TEST_F(ResolverBuiltinFloatTest, Cross_Error_NoArgs) {
    auto* call = Call("cross");
    WrapInFunction(call);

    EXPECT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(), R"(error: no matching call to cross()

1 candidate function:
  cross(vec3<T>, vec3<T>) -> vec3<T>  where: T is abstract-float, f32 or f16
)");
}

TEST_F(ResolverBuiltinFloatTest, Cross_Error_Scalar) {
    auto* call = Call("cross", 1_f, 1_f);
    WrapInFunction(call);

    EXPECT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(), R"(error: no matching call to cross(f32, f32)

1 candidate function:
  cross(vec3<T>, vec3<T>) -> vec3<T>  where: T is abstract-float, f32 or f16
)");
}

TEST_F(ResolverBuiltinFloatTest, Cross_Error_Vec3Int) {
    auto* call = Call("cross", vec3<i32>(1_i, 2_i, 3_i), vec3<i32>(1_i, 2_i, 3_i));
    WrapInFunction(call);

    EXPECT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(),
              R"(error: no matching call to cross(vec3<i32>, vec3<i32>)

1 candidate function:
  cross(vec3<T>, vec3<T>) -> vec3<T>  where: T is abstract-float, f32 or f16
)");
}

TEST_F(ResolverBuiltinFloatTest, Cross_Error_Vec4) {
    auto* call = Call("cross", vec4<f32>(1_f, 2_f, 3_f, 4_f), vec4<f32>(1_f, 2_f, 3_f, 4_f));

    WrapInFunction(call);

    EXPECT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(),
              R"(error: no matching call to cross(vec4<f32>, vec4<f32>)

1 candidate function:
  cross(vec3<T>, vec3<T>) -> vec3<T>  where: T is abstract-float, f32 or f16
)");
}

TEST_F(ResolverBuiltinFloatTest, Cross_Error_TooManyParams) {
    auto* call =
        Call("cross", vec3<f32>(1_f, 2_f, 3_f), vec3<f32>(1_f, 2_f, 3_f), vec3<f32>(1_f, 2_f, 3_f));

    WrapInFunction(call);

    EXPECT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(),
              R"(error: no matching call to cross(vec3<f32>, vec3<f32>, vec3<f32>)

1 candidate function:
  cross(vec3<T>, vec3<T>) -> vec3<T>  where: T is abstract-float, f32 or f16
)");
}

// distance: (T, T) -> T, (vecN<T>, vecN<T>) -> T
TEST_F(ResolverBuiltinFloatTest, Distance_Scalar_f32) {
    auto* call = Call("distance", 1_f, 1_f);
    WrapInFunction(call);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(call), nullptr);
    EXPECT_TRUE(TypeOf(call)->Is<type::F32>());
}

TEST_F(ResolverBuiltinFloatTest, Distance_Scalar_f16) {
    Enable(builtin::Extension::kF16);

    auto* call = Call("distance", 1_h, 1_h);
    WrapInFunction(call);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(call), nullptr);
    EXPECT_TRUE(TypeOf(call)->Is<type::F16>());
}

TEST_F(ResolverBuiltinFloatTest, Distance_Vector_f32) {
    auto* call = Call("distance", vec3<f32>(1_f, 1_f, 3_f), vec3<f32>(1_f, 1_f, 3_f));
    WrapInFunction(call);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(call), nullptr);
    EXPECT_TRUE(TypeOf(call)->Is<type::F32>());
}

TEST_F(ResolverBuiltinFloatTest, Distance_Vector_f16) {
    Enable(builtin::Extension::kF16);

    auto* call = Call("distance", vec3<f16>(1_h, 1_h, 3_h), vec3<f16>(1_h, 1_h, 3_h));
    WrapInFunction(call);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(call), nullptr);
    EXPECT_TRUE(TypeOf(call)->Is<type::F16>());
}

TEST_F(ResolverBuiltinFloatTest, Distance_TooManyParams) {
    auto* call = Call("distance", vec3<f32>(1_f, 1_f, 3_f), vec3<f32>(1_f, 1_f, 3_f),
                      vec3<f32>(1_f, 1_f, 3_f));
    WrapInFunction(call);

    EXPECT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(), R"(error: no matching call to distance(vec3<f32>, vec3<f32>, vec3<f32>)

2 candidate functions:
  distance(T, T) -> T  where: T is abstract-float, f32 or f16
  distance(vecN<T>, vecN<T>) -> T  where: T is abstract-float, f32 or f16
)");
}

TEST_F(ResolverBuiltinFloatTest, Distance_TooFewParams) {
    auto* call = Call("distance", vec3<f32>(1_f, 1_f, 3_f));
    WrapInFunction(call);

    EXPECT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(), R"(error: no matching call to distance(vec3<f32>)

2 candidate functions:
  distance(T, T) -> T  where: T is abstract-float, f32 or f16
  distance(vecN<T>, vecN<T>) -> T  where: T is abstract-float, f32 or f16
)");
}

TEST_F(ResolverBuiltinFloatTest, Distance_NoParams) {
    auto* call = Call("distance");
    WrapInFunction(call);

    EXPECT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(), R"(error: no matching call to distance()

2 candidate functions:
  distance(T, T) -> T  where: T is abstract-float, f32 or f16
  distance(vecN<T>, vecN<T>) -> T  where: T is abstract-float, f32 or f16
)");
}

// frexp: (f32) -> __frexp_result, (vecN<f32>) -> __frexp_result_vecN, (f16) -> __frexp_result_16,
// (vecN<f16>) -> __frexp_result_vecN_f16
TEST_F(ResolverBuiltinFloatTest, FrexpScalar_f32) {
    auto* call = Call("frexp", 1_f);
    WrapInFunction(call);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(call), nullptr);
    auto* ty = TypeOf(call)->As<type::Struct>();
    ASSERT_NE(ty, nullptr);
    ASSERT_EQ(ty->Members().Length(), 2u);

    auto* fract = ty->Members()[0];
    EXPECT_TRUE(fract->Type()->Is<type::F32>());
    EXPECT_EQ(fract->Offset(), 0u);
    EXPECT_EQ(fract->Size(), 4u);
    EXPECT_EQ(fract->Align(), 4u);
    EXPECT_EQ(fract->Name(), Sym("fract"));

    auto* exp = ty->Members()[1];
    EXPECT_TRUE(exp->Type()->Is<type::I32>());
    EXPECT_EQ(exp->Offset(), 4u);
    EXPECT_EQ(exp->Size(), 4u);
    EXPECT_EQ(exp->Align(), 4u);
    EXPECT_EQ(exp->Name(), Sym("exp"));

    EXPECT_EQ(ty->Size(), 8u);
    EXPECT_EQ(ty->SizeNoPadding(), 8u);
}

TEST_F(ResolverBuiltinFloatTest, FrexpScalar_f16) {
    Enable(builtin::Extension::kF16);

    auto* call = Call("frexp", 1_h);
    WrapInFunction(call);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(call), nullptr);
    auto* ty = TypeOf(call)->As<type::Struct>();
    ASSERT_NE(ty, nullptr);
    ASSERT_EQ(ty->Members().Length(), 2u);

    auto* fract = ty->Members()[0];
    EXPECT_TRUE(fract->Type()->Is<type::F16>());
    EXPECT_EQ(fract->Offset(), 0u);
    EXPECT_EQ(fract->Size(), 2u);
    EXPECT_EQ(fract->Align(), 2u);
    EXPECT_EQ(fract->Name(), Sym("fract"));

    auto* exp = ty->Members()[1];
    EXPECT_TRUE(exp->Type()->Is<type::I32>());
    EXPECT_EQ(exp->Offset(), 4u);
    EXPECT_EQ(exp->Size(), 4u);
    EXPECT_EQ(exp->Align(), 4u);
    EXPECT_EQ(exp->Name(), Sym("exp"));

    EXPECT_EQ(ty->Size(), 8u);
    EXPECT_EQ(ty->SizeNoPadding(), 8u);
}

TEST_F(ResolverBuiltinFloatTest, FrexpVector_f32) {
    auto* call = Call("frexp", vec3<f32>());
    WrapInFunction(call);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(call), nullptr);
    auto* ty = TypeOf(call)->As<type::Struct>();
    ASSERT_NE(ty, nullptr);
    ASSERT_EQ(ty->Members().Length(), 2u);

    auto* fract = ty->Members()[0];
    ASSERT_TRUE(fract->Type()->Is<type::Vector>());
    EXPECT_EQ(fract->Type()->As<type::Vector>()->Width(), 3u);
    EXPECT_TRUE(fract->Type()->As<type::Vector>()->type()->Is<type::F32>());
    EXPECT_EQ(fract->Offset(), 0u);
    EXPECT_EQ(fract->Size(), 12u);
    EXPECT_EQ(fract->Align(), 16u);
    EXPECT_EQ(fract->Name(), Sym("fract"));

    auto* exp = ty->Members()[1];
    ASSERT_TRUE(exp->Type()->Is<type::Vector>());
    EXPECT_EQ(exp->Type()->As<type::Vector>()->Width(), 3u);
    EXPECT_TRUE(exp->Type()->As<type::Vector>()->type()->Is<type::I32>());
    EXPECT_EQ(exp->Offset(), 16u);
    EXPECT_EQ(exp->Size(), 12u);
    EXPECT_EQ(exp->Align(), 16u);
    EXPECT_EQ(exp->Name(), Sym("exp"));

    EXPECT_EQ(ty->Size(), 32u);
    EXPECT_EQ(ty->SizeNoPadding(), 28u);
}

TEST_F(ResolverBuiltinFloatTest, FrexpVector_f16) {
    Enable(builtin::Extension::kF16);

    auto* call = Call("frexp", vec3<f16>());
    WrapInFunction(call);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(call), nullptr);
    auto* ty = TypeOf(call)->As<type::Struct>();
    ASSERT_NE(ty, nullptr);
    ASSERT_EQ(ty->Members().Length(), 2u);

    auto* fract = ty->Members()[0];
    ASSERT_TRUE(fract->Type()->Is<type::Vector>());
    EXPECT_EQ(fract->Type()->As<type::Vector>()->Width(), 3u);
    EXPECT_TRUE(fract->Type()->As<type::Vector>()->type()->Is<type::F16>());
    EXPECT_EQ(fract->Offset(), 0u);
    EXPECT_EQ(fract->Size(), 6u);
    EXPECT_EQ(fract->Align(), 8u);
    EXPECT_EQ(fract->Name(), Sym("fract"));

    auto* exp = ty->Members()[1];
    ASSERT_TRUE(exp->Type()->Is<type::Vector>());
    EXPECT_EQ(exp->Type()->As<type::Vector>()->Width(), 3u);
    EXPECT_TRUE(exp->Type()->As<type::Vector>()->type()->Is<type::I32>());
    EXPECT_EQ(exp->Offset(), 16u);
    EXPECT_EQ(exp->Size(), 12u);
    EXPECT_EQ(exp->Align(), 16u);
    EXPECT_EQ(exp->Name(), Sym("exp"));

    EXPECT_EQ(ty->Size(), 32u);
    EXPECT_EQ(ty->SizeNoPadding(), 28u);
}

TEST_F(ResolverBuiltinFloatTest, Frexp_Error_FirstParamInt) {
    GlobalVar("v", ty.i32(), builtin::AddressSpace::kWorkgroup);
    auto* call = Call("frexp", 1_i, AddressOf("v"));
    WrapInFunction(call);

    EXPECT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(),
              R"(error: no matching call to frexp(i32, ptr<workgroup, i32, read_write>)

2 candidate functions:
  frexp(T) -> __frexp_result_T  where: T is abstract-float, f32 or f16
  frexp(vecN<T>) -> __frexp_result_vecN_T  where: T is abstract-float, f32 or f16
)");
}

// length: (T) -> T, (vecN<T>) -> T
TEST_F(ResolverBuiltinFloatTest, Length_Scalar_f32) {
    auto* call = Call("length", 1_f);
    WrapInFunction(call);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(call), nullptr);
    EXPECT_TRUE(TypeOf(call)->Is<type::F32>());
}

TEST_F(ResolverBuiltinFloatTest, Length_Scalar_f16) {
    Enable(builtin::Extension::kF16);

    auto* call = Call("length", 1_h);
    WrapInFunction(call);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(call), nullptr);
    EXPECT_TRUE(TypeOf(call)->Is<type::F16>());
}

TEST_F(ResolverBuiltinFloatTest, Length_FloatVector_f32) {
    auto* call = Call("length", vec3<f32>(1_f, 1_f, 3_f));
    WrapInFunction(call);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(call), nullptr);
    EXPECT_TRUE(TypeOf(call)->Is<type::F32>());
}

TEST_F(ResolverBuiltinFloatTest, Length_FloatVector_f16) {
    Enable(builtin::Extension::kF16);

    auto* call = Call("length", vec3<f16>(1_h, 1_h, 3_h));
    WrapInFunction(call);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(call), nullptr);
    EXPECT_TRUE(TypeOf(call)->Is<type::F16>());
}

TEST_F(ResolverBuiltinFloatTest, Length_NoParams) {
    auto* call = Call("length");
    WrapInFunction(call);

    EXPECT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(), R"(error: no matching call to length()

2 candidate functions:
  length(T) -> T  where: T is abstract-float, f32 or f16
  length(vecN<T>) -> T  where: T is abstract-float, f32 or f16
)");
}

TEST_F(ResolverBuiltinFloatTest, Length_TooManyParams) {
    auto* call = Call("length", 1_f, 2_f);
    WrapInFunction(call);

    EXPECT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(), R"(error: no matching call to length(f32, f32)

2 candidate functions:
  length(T) -> T  where: T is abstract-float, f32 or f16
  length(vecN<T>) -> T  where: T is abstract-float, f32 or f16
)");
}

// mix(vecN<T>, vecN<T>, T) -> vecN<T>. Other overloads are tested in
// ResolverBuiltinTest_FloatBuiltin_IdenticalType above.
TEST_F(ResolverBuiltinFloatTest, Mix_VectorScalar_f32) {
    auto* call = Call("mix", vec3<f32>(1_f, 1_f, 3_f), vec3<f32>(1_f, 1_f, 3_f), 4_f);
    WrapInFunction(call);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(call), nullptr);
    EXPECT_TRUE(TypeOf(call)->is_float_vector());
    EXPECT_EQ(TypeOf(call)->As<type::Vector>()->Width(), 3u);
    ASSERT_NE(TypeOf(call)->As<type::Vector>()->type(), nullptr);
    EXPECT_TRUE(TypeOf(call)->As<type::Vector>()->type()->Is<type::F32>());
}

TEST_F(ResolverBuiltinFloatTest, Mix_VectorScalar_f16) {
    Enable(builtin::Extension::kF16);

    auto* call = Call("mix", vec3<f16>(1_h, 1_h, 1_h), vec3<f16>(1_h, 1_h, 1_h), 4_h);
    WrapInFunction(call);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(call), nullptr);
    EXPECT_TRUE(TypeOf(call)->is_float_vector());
    EXPECT_EQ(TypeOf(call)->As<type::Vector>()->Width(), 3u);
    ASSERT_NE(TypeOf(call)->As<type::Vector>()->type(), nullptr);
    EXPECT_TRUE(TypeOf(call)->As<type::Vector>()->type()->Is<type::F16>());
}

// modf: (f32) -> __modf_result, (vecN<f32>) -> __modf_result_vecN, (f16) -> __modf_result_f16,
// (vecN<f16>) -> __modf_result_vecN_f16
TEST_F(ResolverBuiltinFloatTest, ModfScalar_f32) {
    auto* call = Call("modf", 1_f);
    WrapInFunction(call);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(call), nullptr);
    auto* ty = TypeOf(call)->As<type::Struct>();
    ASSERT_NE(ty, nullptr);
    ASSERT_EQ(ty->Members().Length(), 2u);

    auto* fract = ty->Members()[0];
    EXPECT_TRUE(fract->Type()->Is<type::F32>());
    EXPECT_EQ(fract->Offset(), 0u);
    EXPECT_EQ(fract->Size(), 4u);
    EXPECT_EQ(fract->Align(), 4u);
    EXPECT_EQ(fract->Name(), Sym("fract"));

    auto* whole = ty->Members()[1];
    EXPECT_TRUE(whole->Type()->Is<type::F32>());
    EXPECT_EQ(whole->Offset(), 4u);
    EXPECT_EQ(whole->Size(), 4u);
    EXPECT_EQ(whole->Align(), 4u);
    EXPECT_EQ(whole->Name(), Sym("whole"));

    EXPECT_EQ(ty->Size(), 8u);
    EXPECT_EQ(ty->SizeNoPadding(), 8u);
}

TEST_F(ResolverBuiltinFloatTest, ModfScalar_f16) {
    Enable(builtin::Extension::kF16);

    auto* call = Call("modf", 1_h);
    WrapInFunction(call);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(call), nullptr);
    auto* ty = TypeOf(call)->As<type::Struct>();
    ASSERT_NE(ty, nullptr);
    ASSERT_EQ(ty->Members().Length(), 2u);

    auto* fract = ty->Members()[0];
    EXPECT_TRUE(fract->Type()->Is<type::F16>());
    EXPECT_EQ(fract->Offset(), 0u);
    EXPECT_EQ(fract->Size(), 2u);
    EXPECT_EQ(fract->Align(), 2u);
    EXPECT_EQ(fract->Name(), Sym("fract"));

    auto* whole = ty->Members()[1];
    EXPECT_TRUE(whole->Type()->Is<type::F16>());
    EXPECT_EQ(whole->Offset(), 2u);
    EXPECT_EQ(whole->Size(), 2u);
    EXPECT_EQ(whole->Align(), 2u);
    EXPECT_EQ(whole->Name(), Sym("whole"));

    EXPECT_EQ(ty->Size(), 4u);
    EXPECT_EQ(ty->SizeNoPadding(), 4u);
}

TEST_F(ResolverBuiltinFloatTest, ModfVector_f32) {
    auto* call = Call("modf", vec3<f32>());
    WrapInFunction(call);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(call), nullptr);
    auto* ty = TypeOf(call)->As<type::Struct>();
    ASSERT_NE(ty, nullptr);
    ASSERT_EQ(ty->Members().Length(), 2u);

    auto* fract = ty->Members()[0];
    ASSERT_TRUE(fract->Type()->Is<type::Vector>());
    EXPECT_EQ(fract->Type()->As<type::Vector>()->Width(), 3u);
    EXPECT_TRUE(fract->Type()->As<type::Vector>()->type()->Is<type::F32>());
    EXPECT_EQ(fract->Offset(), 0u);
    EXPECT_EQ(fract->Size(), 12u);
    EXPECT_EQ(fract->Align(), 16u);
    EXPECT_EQ(fract->Name(), Sym("fract"));

    auto* whole = ty->Members()[1];
    ASSERT_TRUE(whole->Type()->Is<type::Vector>());
    EXPECT_EQ(whole->Type()->As<type::Vector>()->Width(), 3u);
    EXPECT_TRUE(whole->Type()->As<type::Vector>()->type()->Is<type::F32>());
    EXPECT_EQ(whole->Offset(), 16u);
    EXPECT_EQ(whole->Size(), 12u);
    EXPECT_EQ(whole->Align(), 16u);
    EXPECT_EQ(whole->Name(), Sym("whole"));

    EXPECT_EQ(ty->Size(), 32u);
    EXPECT_EQ(ty->SizeNoPadding(), 28u);
}

TEST_F(ResolverBuiltinFloatTest, ModfVector_f16) {
    Enable(builtin::Extension::kF16);

    auto* call = Call("modf", vec3<f16>());
    WrapInFunction(call);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(call), nullptr);
    auto* ty = TypeOf(call)->As<type::Struct>();
    ASSERT_NE(ty, nullptr);
    ASSERT_EQ(ty->Members().Length(), 2u);

    auto* fract = ty->Members()[0];
    ASSERT_TRUE(fract->Type()->Is<type::Vector>());
    EXPECT_EQ(fract->Type()->As<type::Vector>()->Width(), 3u);
    EXPECT_TRUE(fract->Type()->As<type::Vector>()->type()->Is<type::F16>());
    EXPECT_EQ(fract->Offset(), 0u);
    EXPECT_EQ(fract->Size(), 6u);
    EXPECT_EQ(fract->Align(), 8u);
    EXPECT_EQ(fract->Name(), Sym("fract"));

    auto* whole = ty->Members()[1];
    ASSERT_TRUE(whole->Type()->Is<type::Vector>());
    EXPECT_EQ(whole->Type()->As<type::Vector>()->Width(), 3u);
    EXPECT_TRUE(whole->Type()->As<type::Vector>()->type()->Is<type::F16>());
    EXPECT_EQ(whole->Offset(), 8u);
    EXPECT_EQ(whole->Size(), 6u);
    EXPECT_EQ(whole->Align(), 8u);
    EXPECT_EQ(whole->Name(), Sym("whole"));

    EXPECT_EQ(ty->Size(), 16u);
    EXPECT_EQ(ty->SizeNoPadding(), 14u);
}

TEST_F(ResolverBuiltinFloatTest, Modf_Error_FirstParamInt) {
    GlobalVar("whole", ty.f32(), builtin::AddressSpace::kWorkgroup);
    auto* call = Call("modf", 1_i, AddressOf("whole"));
    WrapInFunction(call);

    EXPECT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(),
              R"(error: no matching call to modf(i32, ptr<workgroup, f32, read_write>)

2 candidate functions:
  modf(T) -> __modf_result_T  where: T is abstract-float, f32 or f16
  modf(vecN<T>) -> __modf_result_vecN_T  where: T is abstract-float, f32 or f16
)");
}

TEST_F(ResolverBuiltinFloatTest, Modf_Error_SecondParamIntPtr) {
    GlobalVar("whole", ty.i32(), builtin::AddressSpace::kWorkgroup);
    auto* call = Call("modf", 1_f, AddressOf("whole"));
    WrapInFunction(call);

    EXPECT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(),
              R"(error: no matching call to modf(f32, ptr<workgroup, i32, read_write>)

2 candidate functions:
  modf(T) -> __modf_result_T  where: T is abstract-float, f32 or f16
  modf(vecN<T>) -> __modf_result_vecN_T  where: T is abstract-float, f32 or f16
)");
}

TEST_F(ResolverBuiltinFloatTest, Modf_Error_SecondParamNotAPointer) {
    auto* call = Call("modf", 1_f, 1_f);
    WrapInFunction(call);

    EXPECT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(), R"(error: no matching call to modf(f32, f32)

2 candidate functions:
  modf(T) -> __modf_result_T  where: T is abstract-float, f32 or f16
  modf(vecN<T>) -> __modf_result_vecN_T  where: T is abstract-float, f32 or f16
)");
}

TEST_F(ResolverBuiltinFloatTest, Modf_Error_VectorSizesDontMatch) {
    GlobalVar("whole", ty.vec4<f32>(), builtin::AddressSpace::kWorkgroup);
    auto* call = Call("modf", vec2<f32>(1_f, 2_f), AddressOf("whole"));
    WrapInFunction(call);

    EXPECT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(),
              R"(error: no matching call to modf(vec2<f32>, ptr<workgroup, vec4<f32>, read_write>)

2 candidate functions:
  modf(T) -> __modf_result_T  where: T is abstract-float, f32 or f16
  modf(vecN<T>) -> __modf_result_vecN_T  where: T is abstract-float, f32 or f16
)");
}

// normalize: (vecN<T>) -> vecN<T>
TEST_F(ResolverBuiltinFloatTest, Normalize_Vector_f32) {
    auto* call = Call("normalize", vec3<f32>(1_f, 1_f, 3_f));
    WrapInFunction(call);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(call), nullptr);
    EXPECT_TRUE(TypeOf(call)->is_float_vector());
    EXPECT_EQ(TypeOf(call)->As<type::Vector>()->Width(), 3u);
    EXPECT_TRUE(TypeOf(call)->As<type::Vector>()->type()->Is<type::F32>());
}

TEST_F(ResolverBuiltinFloatTest, Normalize_Vector_f16) {
    Enable(builtin::Extension::kF16);

    auto* call = Call("normalize", vec3<f16>(1_h, 1_h, 3_h));
    WrapInFunction(call);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(call), nullptr);
    EXPECT_TRUE(TypeOf(call)->is_float_vector());
    EXPECT_EQ(TypeOf(call)->As<type::Vector>()->Width(), 3u);
    EXPECT_TRUE(TypeOf(call)->As<type::Vector>()->type()->Is<type::F16>());
}

TEST_F(ResolverBuiltinFloatTest, Normalize_Error_NoParams) {
    auto* call = Call("normalize");
    WrapInFunction(call);

    EXPECT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(), R"(error: no matching call to normalize()

1 candidate function:
  normalize(vecN<T>) -> vecN<T>  where: T is abstract-float, f32 or f16
)");
}

}  // namespace float_builtin_tests

// Tests for Numeric builtins with all integer parameter
namespace integer_builtin_tests {

// Testcase parameters for integer built-in having signature of (T, ...) -> T and (vecN<T>, ...) ->
// vecN<T>, where T is i32 and u32
struct BuiltinDataWithParamNum {
    uint32_t args_number;
    const char* name;
    builtin::Function builtin;
};

inline std::ostream& operator<<(std::ostream& out, BuiltinDataWithParamNum data) {
    out << data.name;
    return out;
}

// Tests for integer built-ins that has signiture (T, ...) -> T and (vecN<T>, ...) -> vecN<T>
using ResolverBuiltinTest_IntegerBuiltin_IdenticalType =
    ResolverTestWithParam<BuiltinDataWithParamNum>;

TEST_P(ResolverBuiltinTest_IntegerBuiltin_IdenticalType, Error_NoParams) {
    auto param = GetParam();

    auto* call = Call(param.name);
    WrapInFunction(call);

    EXPECT_FALSE(r()->Resolve());

    EXPECT_THAT(r()->error(),
                HasSubstr("error: no matching call to " + std::string(param.name) + "()"));
}

TEST_P(ResolverBuiltinTest_IntegerBuiltin_IdenticalType, OneParams_Scalar_i32) {
    auto param = GetParam();

    auto* call = Call(param.name, 1_i);
    WrapInFunction(call);

    if (param.args_number == 1u) {
        // Parameter count matched.
        EXPECT_TRUE(r()->Resolve()) << r()->error();

        ASSERT_NE(TypeOf(call), nullptr);
        EXPECT_TRUE(TypeOf(call)->Is<type::I32>());
    } else {
        // Invalid parameter count.
        EXPECT_FALSE(r()->Resolve());

        EXPECT_THAT(r()->error(),
                    HasSubstr("error: no matching call to " + std::string(param.name) + "(i32)"));
    }
}

TEST_P(ResolverBuiltinTest_IntegerBuiltin_IdenticalType, OneParams_Vector_i32) {
    auto param = GetParam();

    auto* call = Call(param.name, vec3<i32>(1_i, 1_i, 3_i));
    WrapInFunction(call);

    if (param.args_number == 1u) {
        // Parameter count matched.
        EXPECT_TRUE(r()->Resolve()) << r()->error();

        ASSERT_NE(TypeOf(call), nullptr);
        EXPECT_TRUE(TypeOf(call)->is_signed_integer_vector());
        EXPECT_EQ(TypeOf(call)->As<type::Vector>()->Width(), 3u);
        ASSERT_NE(TypeOf(call)->As<type::Vector>()->type(), nullptr);
        EXPECT_TRUE(TypeOf(call)->As<type::Vector>()->type()->Is<type::I32>());
    } else {
        // Invalid parameter count.
        EXPECT_FALSE(r()->Resolve());

        EXPECT_THAT(r()->error(), HasSubstr("error: no matching call to " +
                                            std::string(param.name) + "(vec3<i32>)"));
    }
}

TEST_P(ResolverBuiltinTest_IntegerBuiltin_IdenticalType, OneParams_Scalar_u32) {
    auto param = GetParam();

    auto* call = Call(param.name, 1_u);
    WrapInFunction(call);

    if (param.args_number == 1u) {
        // Parameter count matched.
        EXPECT_TRUE(r()->Resolve()) << r()->error();

        ASSERT_NE(TypeOf(call), nullptr);
        EXPECT_TRUE(TypeOf(call)->Is<type::U32>());
    } else {
        // Invalid parameter count.
        EXPECT_FALSE(r()->Resolve());

        EXPECT_THAT(r()->error(),
                    HasSubstr("error: no matching call to " + std::string(param.name) + "(u32)"));
    }
}

TEST_P(ResolverBuiltinTest_IntegerBuiltin_IdenticalType, OneParams_Vector_u32) {
    auto param = GetParam();

    auto* call = Call(param.name, vec3<u32>(1_u, 1_u, 3_u));
    WrapInFunction(call);

    if (param.args_number == 1u) {
        // Parameter count matched.
        EXPECT_TRUE(r()->Resolve()) << r()->error();

        ASSERT_NE(TypeOf(call), nullptr);
        EXPECT_TRUE(TypeOf(call)->is_unsigned_integer_vector());
        EXPECT_EQ(TypeOf(call)->As<type::Vector>()->Width(), 3u);
        ASSERT_NE(TypeOf(call)->As<type::Vector>()->type(), nullptr);
        EXPECT_TRUE(TypeOf(call)->As<type::Vector>()->type()->Is<type::U32>());
    } else {
        // Invalid parameter count.
        EXPECT_FALSE(r()->Resolve());

        EXPECT_THAT(r()->error(), HasSubstr("error: no matching call to " +
                                            std::string(param.name) + "(vec3<u32>)"));
    }
}

TEST_P(ResolverBuiltinTest_IntegerBuiltin_IdenticalType, TwoParams_Scalar_i32) {
    auto param = GetParam();

    auto* call = Call(param.name, 1_i, 1_i);
    WrapInFunction(call);

    if (param.args_number == 2u) {
        // Parameter count matched.
        EXPECT_TRUE(r()->Resolve()) << r()->error();

        ASSERT_NE(TypeOf(call), nullptr);
        EXPECT_TRUE(TypeOf(call)->Is<type::I32>());
    } else {
        // Invalid parameter count.
        EXPECT_FALSE(r()->Resolve());

        EXPECT_THAT(r()->error(), HasSubstr("error: no matching call to " +
                                            std::string(param.name) + "(i32, i32)"));
    }
}

TEST_P(ResolverBuiltinTest_IntegerBuiltin_IdenticalType, TwoParams_Vector_i32) {
    auto param = GetParam();

    auto* call = Call(param.name, vec3<i32>(1_i, 1_i, 3_i), vec3<i32>(1_i, 1_i, 3_i));
    WrapInFunction(call);

    if (param.args_number == 2u) {
        // Parameter count matched.
        EXPECT_TRUE(r()->Resolve()) << r()->error();

        ASSERT_NE(TypeOf(call), nullptr);
        EXPECT_TRUE(TypeOf(call)->is_signed_integer_vector());
        EXPECT_EQ(TypeOf(call)->As<type::Vector>()->Width(), 3u);
        ASSERT_NE(TypeOf(call)->As<type::Vector>()->type(), nullptr);
        EXPECT_TRUE(TypeOf(call)->As<type::Vector>()->type()->Is<type::I32>());
    } else {
        // Invalid parameter count.
        EXPECT_FALSE(r()->Resolve());

        EXPECT_THAT(r()->error(), HasSubstr("error: no matching call to " +
                                            std::string(param.name) + "(vec3<i32>, vec3<i32>)"));
    }
}

TEST_P(ResolverBuiltinTest_IntegerBuiltin_IdenticalType, TwoParams_Scalar_u32) {
    auto param = GetParam();

    auto* call = Call(param.name, 1_u, 1_u);
    WrapInFunction(call);

    if (param.args_number == 2u) {
        // Parameter count matched.
        EXPECT_TRUE(r()->Resolve()) << r()->error();

        ASSERT_NE(TypeOf(call), nullptr);
        EXPECT_TRUE(TypeOf(call)->Is<type::U32>());
    } else {
        // Invalid parameter count.
        EXPECT_FALSE(r()->Resolve());

        EXPECT_THAT(r()->error(), HasSubstr("error: no matching call to " +
                                            std::string(param.name) + "(u32, u32)"));
    }
}

TEST_P(ResolverBuiltinTest_IntegerBuiltin_IdenticalType, TwoParams_Vector_u32) {
    auto param = GetParam();

    auto* call = Call(param.name, vec3<u32>(1_u, 1_u, 3_u), vec3<u32>(1_u, 1_u, 3_u));
    WrapInFunction(call);

    if (param.args_number == 2u) {
        // Parameter count matched.
        EXPECT_TRUE(r()->Resolve()) << r()->error();

        ASSERT_NE(TypeOf(call), nullptr);
        EXPECT_TRUE(TypeOf(call)->is_unsigned_integer_vector());
        EXPECT_EQ(TypeOf(call)->As<type::Vector>()->Width(), 3u);
        ASSERT_NE(TypeOf(call)->As<type::Vector>()->type(), nullptr);
        EXPECT_TRUE(TypeOf(call)->As<type::Vector>()->type()->Is<type::U32>());
    } else {
        // Invalid parameter count.
        EXPECT_FALSE(r()->Resolve());

        EXPECT_THAT(r()->error(), HasSubstr("error: no matching call to " +
                                            std::string(param.name) + "(vec3<u32>, vec3<u32>)"));
    }
}

TEST_P(ResolverBuiltinTest_IntegerBuiltin_IdenticalType, ThreeParams_Scalar_i32) {
    auto param = GetParam();

    auto* call = Call(param.name, 1_i, 1_i, 1_i);
    WrapInFunction(call);

    if (param.args_number == 3u) {
        // Parameter count matched.
        EXPECT_TRUE(r()->Resolve()) << r()->error();

        ASSERT_NE(TypeOf(call), nullptr);
        EXPECT_TRUE(TypeOf(call)->Is<type::I32>());
    } else {
        // Invalid parameter count.
        EXPECT_FALSE(r()->Resolve());

        EXPECT_THAT(r()->error(), HasSubstr("error: no matching call to " +
                                            std::string(param.name) + "(i32, i32, i32)"));
    }
}

TEST_P(ResolverBuiltinTest_IntegerBuiltin_IdenticalType, ThreeParams_Vector_i32) {
    auto param = GetParam();

    auto* call = Call(param.name, vec3<i32>(1_i, 1_i, 3_i), vec3<i32>(1_i, 1_i, 3_i),
                      vec3<i32>(1_i, 1_i, 3_i));
    WrapInFunction(call);

    if (param.args_number == 3u) {
        // Parameter count matched.
        EXPECT_TRUE(r()->Resolve()) << r()->error();

        ASSERT_NE(TypeOf(call), nullptr);
        EXPECT_TRUE(TypeOf(call)->is_signed_integer_vector());
        EXPECT_EQ(TypeOf(call)->As<type::Vector>()->Width(), 3u);
        ASSERT_NE(TypeOf(call)->As<type::Vector>()->type(), nullptr);
        EXPECT_TRUE(TypeOf(call)->As<type::Vector>()->type()->Is<type::I32>());
    } else {
        // Invalid parameter count.
        EXPECT_FALSE(r()->Resolve());

        EXPECT_THAT(r()->error(),
                    HasSubstr("error: no matching call to " + std::string(param.name) +
                              "(vec3<i32>, vec3<i32>, vec3<i32>)"));
    }
}

TEST_P(ResolverBuiltinTest_IntegerBuiltin_IdenticalType, ThreeParams_Scalar_u32) {
    auto param = GetParam();

    auto* call = Call(param.name, 1_u, 1_u, 1_u);
    WrapInFunction(call);

    if (param.args_number == 3u) {
        // Parameter count matched.
        EXPECT_TRUE(r()->Resolve()) << r()->error();

        ASSERT_NE(TypeOf(call), nullptr);
        EXPECT_TRUE(TypeOf(call)->Is<type::U32>());
    } else {
        // Invalid parameter count.
        EXPECT_FALSE(r()->Resolve());

        EXPECT_THAT(r()->error(), HasSubstr("error: no matching call to " +
                                            std::string(param.name) + "(u32, u32, u32)"));
    }
}

TEST_P(ResolverBuiltinTest_IntegerBuiltin_IdenticalType, ThreeParams_Vector_u32) {
    auto param = GetParam();

    auto* call = Call(param.name, vec3<u32>(1_u, 1_u, 3_u), vec3<u32>(1_u, 1_u, 3_u),
                      vec3<u32>(1_u, 1_u, 3_u));
    WrapInFunction(call);

    if (param.args_number == 3u) {
        // Parameter count matched.
        EXPECT_TRUE(r()->Resolve()) << r()->error();

        ASSERT_NE(TypeOf(call), nullptr);
        EXPECT_TRUE(TypeOf(call)->is_unsigned_integer_vector());
        EXPECT_EQ(TypeOf(call)->As<type::Vector>()->Width(), 3u);
        ASSERT_NE(TypeOf(call)->As<type::Vector>()->type(), nullptr);
        EXPECT_TRUE(TypeOf(call)->As<type::Vector>()->type()->Is<type::U32>());
    } else {
        // Invalid parameter count.
        EXPECT_FALSE(r()->Resolve());

        EXPECT_THAT(r()->error(),
                    HasSubstr("error: no matching call to " + std::string(param.name) +
                              "(vec3<u32>, vec3<u32>, vec3<u32>)"));
    }
}

TEST_P(ResolverBuiltinTest_IntegerBuiltin_IdenticalType, FourParams_Scalar_i32) {
    auto param = GetParam();

    auto* call = Call(param.name, 1_i, 1_i, 1_i, 1_i);
    WrapInFunction(call);

    if (param.args_number == 4u) {
        // Parameter count matched.
        EXPECT_TRUE(r()->Resolve()) << r()->error();

        ASSERT_NE(TypeOf(call), nullptr);
        EXPECT_TRUE(TypeOf(call)->Is<type::I32>());
    } else {
        // Invalid parameter count.
        EXPECT_FALSE(r()->Resolve());

        EXPECT_THAT(r()->error(), HasSubstr("error: no matching call to " +
                                            std::string(param.name) + "(i32, i32, i32, i32)"));
    }
}

TEST_P(ResolverBuiltinTest_IntegerBuiltin_IdenticalType, FourParams_Vector_i32) {
    auto param = GetParam();

    auto* call = Call(param.name, vec3<i32>(1_i, 1_i, 3_i), vec3<i32>(1_i, 1_i, 3_i),
                      vec3<i32>(1_i, 1_i, 3_i), vec3<i32>(1_i, 1_i, 3_i));
    WrapInFunction(call);

    if (param.args_number == 4u) {
        // Parameter count matched.
        EXPECT_TRUE(r()->Resolve()) << r()->error();

        ASSERT_NE(TypeOf(call), nullptr);
        EXPECT_TRUE(TypeOf(call)->is_signed_integer_vector());
        EXPECT_EQ(TypeOf(call)->As<type::Vector>()->Width(), 3u);
        ASSERT_NE(TypeOf(call)->As<type::Vector>()->type(), nullptr);
        EXPECT_TRUE(TypeOf(call)->As<type::Vector>()->type()->Is<type::I32>());
    } else {
        // Invalid parameter count.
        EXPECT_FALSE(r()->Resolve());

        EXPECT_THAT(r()->error(),
                    HasSubstr("error: no matching call to " + std::string(param.name) +
                              "(vec3<i32>, vec3<i32>, vec3<i32>, vec3<i32>)"));
    }
}

TEST_P(ResolverBuiltinTest_IntegerBuiltin_IdenticalType, FourParams_Scalar_u32) {
    auto param = GetParam();

    auto* call = Call(param.name, 1_u, 1_u, 1_u, 1_u);
    WrapInFunction(call);

    if (param.args_number == 4u) {
        // Parameter count matched.
        EXPECT_TRUE(r()->Resolve()) << r()->error();

        ASSERT_NE(TypeOf(call), nullptr);
        EXPECT_TRUE(TypeOf(call)->Is<type::U32>());
    } else {
        // Invalid parameter count.
        EXPECT_FALSE(r()->Resolve());

        EXPECT_THAT(r()->error(), HasSubstr("error: no matching call to " +
                                            std::string(param.name) + "(u32, u32, u32, u32)"));
    }
}

TEST_P(ResolverBuiltinTest_IntegerBuiltin_IdenticalType, FourParams_Vector_u32) {
    auto param = GetParam();

    auto* call = Call(param.name, vec3<u32>(1_u, 1_u, 3_u), vec3<u32>(1_u, 1_u, 3_u),
                      vec3<u32>(1_u, 1_u, 3_u), vec3<u32>(1_u, 1_u, 3_u));
    WrapInFunction(call);

    if (param.args_number == 4u) {
        // Parameter count matched.
        EXPECT_TRUE(r()->Resolve()) << r()->error();

        ASSERT_NE(TypeOf(call), nullptr);
        EXPECT_TRUE(TypeOf(call)->is_unsigned_integer_vector());
        EXPECT_EQ(TypeOf(call)->As<type::Vector>()->Width(), 3u);
        ASSERT_NE(TypeOf(call)->As<type::Vector>()->type(), nullptr);
        EXPECT_TRUE(TypeOf(call)->As<type::Vector>()->type()->Is<type::U32>());
    } else {
        // Invalid parameter count.
        EXPECT_FALSE(r()->Resolve());

        EXPECT_THAT(r()->error(),
                    HasSubstr("error: no matching call to " + std::string(param.name) +
                              "(vec3<u32>, vec3<u32>, vec3<u32>, vec3<u32>)"));
    }
}

INSTANTIATE_TEST_SUITE_P(
    ResolverTest,
    ResolverBuiltinTest_IntegerBuiltin_IdenticalType,
    testing::Values(
        BuiltinDataWithParamNum{1, "abs", builtin::Function::kAbs},
        BuiltinDataWithParamNum{3, "clamp", builtin::Function::kClamp},
        BuiltinDataWithParamNum{1, "countLeadingZeros", builtin::Function::kCountLeadingZeros},
        BuiltinDataWithParamNum{1, "countOneBits", builtin::Function::kCountOneBits},
        BuiltinDataWithParamNum{1, "countTrailingZeros", builtin::Function::kCountTrailingZeros},
        // extractBits: (T, u32, u32) -> T
        BuiltinDataWithParamNum{1, "firstLeadingBit", builtin::Function::kFirstLeadingBit},
        BuiltinDataWithParamNum{1, "firstTrailingBit", builtin::Function::kFirstTrailingBit},
        // insertBits: (T, T, u32, u32) -> T
        BuiltinDataWithParamNum{2, "max", builtin::Function::kMax},
        BuiltinDataWithParamNum{2, "min", builtin::Function::kMin},
        BuiltinDataWithParamNum{1, "reverseBits", builtin::Function::kReverseBits}));

}  // namespace integer_builtin_tests

// Tests for Numeric builtins with matrix parameter, i.e. "determinant" and "transpose"
namespace matrix_builtin_tests {

TEST_F(ResolverBuiltinTest, Determinant_2x2_f32) {
    GlobalVar("var", ty.mat2x2<f32>(), builtin::AddressSpace::kPrivate);

    auto* call = Call("determinant", "var");
    WrapInFunction(call);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(call), nullptr);
    EXPECT_TRUE(TypeOf(call)->Is<type::F32>());
}

TEST_F(ResolverBuiltinTest, Determinant_2x2_f16) {
    Enable(builtin::Extension::kF16);

    GlobalVar("var", ty.mat2x2<f16>(), builtin::AddressSpace::kPrivate);

    auto* call = Call("determinant", "var");
    WrapInFunction(call);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(call), nullptr);
    EXPECT_TRUE(TypeOf(call)->Is<type::F16>());
}

TEST_F(ResolverBuiltinTest, Determinant_3x3_f32) {
    GlobalVar("var", ty.mat3x3<f32>(), builtin::AddressSpace::kPrivate);

    auto* call = Call("determinant", "var");
    WrapInFunction(call);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(call), nullptr);
    EXPECT_TRUE(TypeOf(call)->Is<type::F32>());
}

TEST_F(ResolverBuiltinTest, Determinant_3x3_f16) {
    Enable(builtin::Extension::kF16);

    GlobalVar("var", ty.mat3x3<f16>(), builtin::AddressSpace::kPrivate);

    auto* call = Call("determinant", "var");
    WrapInFunction(call);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(call), nullptr);
    EXPECT_TRUE(TypeOf(call)->Is<type::F16>());
}

TEST_F(ResolverBuiltinTest, Determinant_4x4_f32) {
    GlobalVar("var", ty.mat4x4<f32>(), builtin::AddressSpace::kPrivate);

    auto* call = Call("determinant", "var");
    WrapInFunction(call);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(call), nullptr);
    EXPECT_TRUE(TypeOf(call)->Is<type::F32>());
}

TEST_F(ResolverBuiltinTest, Determinant_4x4_f16) {
    Enable(builtin::Extension::kF16);

    GlobalVar("var", ty.mat4x4<f16>(), builtin::AddressSpace::kPrivate);

    auto* call = Call("determinant", "var");
    WrapInFunction(call);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(call), nullptr);
    EXPECT_TRUE(TypeOf(call)->Is<type::F16>());
}

TEST_F(ResolverBuiltinTest, Determinant_NotSquare) {
    GlobalVar("var", ty.mat2x3<f32>(), builtin::AddressSpace::kPrivate);

    auto* call = Call("determinant", "var");
    WrapInFunction(call);

    EXPECT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(), R"(error: no matching call to determinant(mat2x3<f32>)

1 candidate function:
  determinant(matNxN<T>) -> T  where: T is abstract-float, f32 or f16
)");
}

TEST_F(ResolverBuiltinTest, Determinant_NotMatrix) {
    GlobalVar("var", ty.f32(), builtin::AddressSpace::kPrivate);

    auto* call = Call("determinant", "var");
    WrapInFunction(call);

    EXPECT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(), R"(error: no matching call to determinant(f32)

1 candidate function:
  determinant(matNxN<T>) -> T  where: T is abstract-float, f32 or f16
)");
}

}  // namespace matrix_builtin_tests

// Tests for Numeric builtins with float and integer vector parameter, i.e. "dot"
namespace vector_builtin_tests {

TEST_F(ResolverBuiltinTest, Dot_Vec2_f32) {
    GlobalVar("my_var", ty.vec2<f32>(), builtin::AddressSpace::kPrivate);

    auto* expr = Call("dot", "my_var", "my_var");
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(expr), nullptr);
    EXPECT_TRUE(TypeOf(expr)->Is<type::F32>());
}

TEST_F(ResolverBuiltinTest, Dot_Vec2_f16) {
    Enable(builtin::Extension::kF16);

    GlobalVar("my_var", ty.vec2<f16>(), builtin::AddressSpace::kPrivate);

    auto* expr = Call("dot", "my_var", "my_var");
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(expr), nullptr);
    EXPECT_TRUE(TypeOf(expr)->Is<type::F16>());
}

TEST_F(ResolverBuiltinTest, Dot_Vec3_i32) {
    GlobalVar("my_var", ty.vec3<i32>(), builtin::AddressSpace::kPrivate);

    auto* expr = Call("dot", "my_var", "my_var");
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(expr), nullptr);
    EXPECT_TRUE(TypeOf(expr)->Is<type::I32>());
}

TEST_F(ResolverBuiltinTest, Dot_Vec4_u32) {
    GlobalVar("my_var", ty.vec4<u32>(), builtin::AddressSpace::kPrivate);

    auto* expr = Call("dot", "my_var", "my_var");
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(expr), nullptr);
    EXPECT_TRUE(TypeOf(expr)->Is<type::U32>());
}

TEST_F(ResolverBuiltinTest, Dot_Error_Scalar) {
    auto* expr = Call("dot", 1_f, 1_f);
    WrapInFunction(expr);

    EXPECT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(),
              R"(error: no matching call to dot(f32, f32)

1 candidate function:
  dot(vecN<T>, vecN<T>) -> T  where: T is abstract-float, abstract-int, f32, i32, u32 or f16
)");
}

}  // namespace vector_builtin_tests

// Tests for Derivative builtins
namespace derivative_builtin_tests {

using ResolverBuiltinDerivativeTest = ResolverTestWithParam<std::string>;

TEST_P(ResolverBuiltinDerivativeTest, Scalar) {
    auto name = GetParam();

    GlobalVar("ident", ty.f32(), builtin::AddressSpace::kPrivate);

    auto* expr = Call(name, "ident");
    Func("func", utils::Empty, ty.void_(), utils::Vector{Ignore(expr)},
         utils::Vector{create<ast::StageAttribute>(ast::PipelineStage::kFragment)});

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(expr), nullptr);
    ASSERT_TRUE(TypeOf(expr)->Is<type::F32>());
}

TEST_P(ResolverBuiltinDerivativeTest, Vector) {
    auto name = GetParam();
    GlobalVar("ident", ty.vec4<f32>(), builtin::AddressSpace::kPrivate);

    auto* expr = Call(name, "ident");
    Func("func", utils::Empty, ty.void_(), utils::Vector{Ignore(expr)},
         utils::Vector{create<ast::StageAttribute>(ast::PipelineStage::kFragment)});

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(expr), nullptr);
    ASSERT_TRUE(TypeOf(expr)->Is<type::Vector>());
    EXPECT_TRUE(TypeOf(expr)->As<type::Vector>()->type()->Is<type::F32>());
    EXPECT_EQ(TypeOf(expr)->As<type::Vector>()->Width(), 4u);
}

TEST_P(ResolverBuiltinDerivativeTest, MissingParam) {
    auto name = GetParam();

    auto* expr = Call(name);
    WrapInFunction(expr);

    EXPECT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(), "error: no matching call to " + name +
                                "()\n\n"
                                "2 candidate functions:\n  " +
                                name + "(f32) -> f32\n  " + name + "(vecN<f32>) -> vecN<f32>\n");
}

INSTANTIATE_TEST_SUITE_P(ResolverTest,
                         ResolverBuiltinDerivativeTest,
                         testing::Values("dpdx",
                                         "dpdxCoarse",
                                         "dpdxFine",
                                         "dpdy",
                                         "dpdyCoarse",
                                         "dpdyFine",
                                         "fwidth",
                                         "fwidthCoarse",
                                         "fwidthFine"));

}  // namespace derivative_builtin_tests

// Tests for Texture builtins
namespace texture_builtin_tests {

enum class Texture { kF32, kI32, kU32 };
inline utils::StringStream& operator<<(utils::StringStream& out, Texture data) {
    if (data == Texture::kF32) {
        out << "f32";
    } else if (data == Texture::kI32) {
        out << "i32";
    } else {
        out << "u32";
    }
    return out;
}

struct TextureTestParams {
    type::TextureDimension dim;
    Texture type = Texture::kF32;
    builtin::TexelFormat format = builtin::TexelFormat::kR32Float;
};
inline std::ostream& operator<<(std::ostream& out, TextureTestParams data) {
    utils::StringStream str;
    str << data.dim << "_" << data.type;
    out << str.str();
    return out;
}

class ResolverBuiltinTest_TextureOperation : public ResolverTestWithParam<TextureTestParams> {
  public:
    /// Gets an appropriate type for the coords parameter depending the the
    /// dimensionality of the texture being sampled.
    /// @param dim dimensionality of the texture being sampled
    /// @param scalar the scalar type
    /// @returns a pointer to a type appropriate for the coord param
    ast::Type GetCoordsType(type::TextureDimension dim, ast::Type scalar) {
        switch (dim) {
            case type::TextureDimension::k1d:
                return ty(scalar);
            case type::TextureDimension::k2d:
            case type::TextureDimension::k2dArray:
                return ty.vec2(scalar);
            case type::TextureDimension::k3d:
            case type::TextureDimension::kCube:
            case type::TextureDimension::kCubeArray:
                return ty.vec3(scalar);
            default:
                [=]() {
                    utils::StringStream str;
                    str << dim;
                    FAIL() << "Unsupported texture dimension: " << str.str();
                }();
        }
        return ast::Type{};
    }

    void add_call_param(std::string name, ast::Type type, ExpressionList* call_params) {
        std::string type_name = type->identifier->symbol.Name();
        if (utils::HasPrefix(type_name, "texture") || utils::HasPrefix(type_name, "sampler")) {
            GlobalVar(name, type, Binding(0_a), Group(0_a));
        } else {
            GlobalVar(name, type, builtin::AddressSpace::kPrivate);
        }

        call_params->Push(Expr(name));
    }
    ast::Type subtype(Texture type) {
        if (type == Texture::kF32) {
            return ty.f32();
        }
        if (type == Texture::kI32) {
            return ty.i32();
        }
        return ty.u32();
    }
};

using ResolverBuiltinTest_SampledTextureOperation = ResolverBuiltinTest_TextureOperation;
TEST_P(ResolverBuiltinTest_SampledTextureOperation, TextureLoadSampled) {
    auto dim = GetParam().dim;
    auto type = GetParam().type;

    ast::Type s = subtype(type);
    ast::Type coords_type = GetCoordsType(dim, ty.i32());
    auto texture_type = ty.sampled_texture(dim, s);

    ExpressionList call_params;

    add_call_param("texture", texture_type, &call_params);
    add_call_param("coords", coords_type, &call_params);
    if (dim == type::TextureDimension::k2dArray) {
        add_call_param("array_index", ty.i32(), &call_params);
    }
    add_call_param("level", ty.i32(), &call_params);

    auto* expr = Call("textureLoad", call_params);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_NE(TypeOf(expr), nullptr);
    ASSERT_TRUE(TypeOf(expr)->Is<type::Vector>());
    if (type == Texture::kF32) {
        EXPECT_TRUE(TypeOf(expr)->As<type::Vector>()->type()->Is<type::F32>());
    } else if (type == Texture::kI32) {
        EXPECT_TRUE(TypeOf(expr)->As<type::Vector>()->type()->Is<type::I32>());
    } else {
        EXPECT_TRUE(TypeOf(expr)->As<type::Vector>()->type()->Is<type::U32>());
    }
    EXPECT_EQ(TypeOf(expr)->As<type::Vector>()->Width(), 4u);
}

INSTANTIATE_TEST_SUITE_P(ResolverTest,
                         ResolverBuiltinTest_SampledTextureOperation,
                         testing::Values(TextureTestParams{type::TextureDimension::k1d},
                                         TextureTestParams{type::TextureDimension::k2d},
                                         TextureTestParams{type::TextureDimension::k2dArray},
                                         TextureTestParams{type::TextureDimension::k3d}));

using ResolverBuiltinTest_Texture = ResolverTestWithParam<ast::builtin::test::TextureOverloadCase>;

INSTANTIATE_TEST_SUITE_P(ResolverTest,
                         ResolverBuiltinTest_Texture,
                         testing::ValuesIn(ast::builtin::test::TextureOverloadCase::ValidCases()));

static std::string to_str(const std::string& function,
                          utils::VectorRef<const sem::Parameter*> params) {
    utils::StringStream out;
    out << function << "(";
    bool first = true;
    for (auto* param : params) {
        if (!first) {
            out << ", ";
        }
        out << sem::str(param->Usage());
        first = false;
    }
    out << ")";
    return out.str();
}

static const char* expected_texture_overload(ast::builtin::test::ValidTextureOverload overload) {
    using ValidTextureOverload = ast::builtin::test::ValidTextureOverload;
    switch (overload) {
        case ValidTextureOverload::kDimensions1d:
        case ValidTextureOverload::kDimensions2d:
        case ValidTextureOverload::kDimensions2dArray:
        case ValidTextureOverload::kDimensions3d:
        case ValidTextureOverload::kDimensionsCube:
        case ValidTextureOverload::kDimensionsCubeArray:
        case ValidTextureOverload::kDimensionsMultisampled2d:
        case ValidTextureOverload::kDimensionsDepth2d:
        case ValidTextureOverload::kDimensionsDepth2dArray:
        case ValidTextureOverload::kDimensionsDepthCube:
        case ValidTextureOverload::kDimensionsDepthCubeArray:
        case ValidTextureOverload::kDimensionsDepthMultisampled2d:
        case ValidTextureOverload::kDimensionsStorageWO1d:
        case ValidTextureOverload::kDimensionsStorageWO2d:
        case ValidTextureOverload::kDimensionsStorageWO2dArray:
        case ValidTextureOverload::kDimensionsStorageWO3d:
            return R"(textureDimensions(texture))";
        case ValidTextureOverload::kGather2dF32:
            return R"(textureGather(component, texture, sampler, coords))";
        case ValidTextureOverload::kGather2dOffsetF32:
            return R"(textureGather(component, texture, sampler, coords, offset))";
        case ValidTextureOverload::kGather2dArrayF32:
            return R"(textureGather(component, texture, sampler, coords, array_index))";
        case ValidTextureOverload::kGather2dArrayOffsetF32:
            return R"(textureGather(component, texture, sampler, coords, array_index, offset))";
        case ValidTextureOverload::kGatherCubeF32:
            return R"(textureGather(component, texture, sampler, coords))";
        case ValidTextureOverload::kGatherCubeArrayF32:
            return R"(textureGather(component, texture, sampler, coords, array_index))";
        case ValidTextureOverload::kGatherDepth2dF32:
            return R"(textureGather(texture, sampler, coords))";
        case ValidTextureOverload::kGatherDepth2dOffsetF32:
            return R"(textureGather(texture, sampler, coords, offset))";
        case ValidTextureOverload::kGatherDepth2dArrayF32:
            return R"(textureGather(texture, sampler, coords, array_index))";
        case ValidTextureOverload::kGatherDepth2dArrayOffsetF32:
            return R"(textureGather(texture, sampler, coords, array_index, offset))";
        case ValidTextureOverload::kGatherDepthCubeF32:
            return R"(textureGather(texture, sampler, coords))";
        case ValidTextureOverload::kGatherDepthCubeArrayF32:
            return R"(textureGather(texture, sampler, coords, array_index))";
        case ValidTextureOverload::kGatherCompareDepth2dF32:
            return R"(textureGatherCompare(texture, sampler, coords, depth_ref))";
        case ValidTextureOverload::kGatherCompareDepth2dOffsetF32:
            return R"(textureGatherCompare(texture, sampler, coords, depth_ref, offset))";
        case ValidTextureOverload::kGatherCompareDepth2dArrayF32:
            return R"(textureGatherCompare(texture, sampler, coords, array_index, depth_ref))";
        case ValidTextureOverload::kGatherCompareDepth2dArrayOffsetF32:
            return R"(textureGatherCompare(texture, sampler, coords, array_index, depth_ref, offset))";
        case ValidTextureOverload::kGatherCompareDepthCubeF32:
            return R"(textureGatherCompare(texture, sampler, coords, depth_ref))";
        case ValidTextureOverload::kGatherCompareDepthCubeArrayF32:
            return R"(textureGatherCompare(texture, sampler, coords, array_index, depth_ref))";
        case ValidTextureOverload::kNumLayers2dArray:
        case ValidTextureOverload::kNumLayersCubeArray:
        case ValidTextureOverload::kNumLayersDepth2dArray:
        case ValidTextureOverload::kNumLayersDepthCubeArray:
        case ValidTextureOverload::kNumLayersStorageWO2dArray:
            return R"(textureNumLayers(texture))";
        case ValidTextureOverload::kNumLevels2d:
        case ValidTextureOverload::kNumLevels2dArray:
        case ValidTextureOverload::kNumLevels3d:
        case ValidTextureOverload::kNumLevelsCube:
        case ValidTextureOverload::kNumLevelsCubeArray:
        case ValidTextureOverload::kNumLevelsDepth2d:
        case ValidTextureOverload::kNumLevelsDepth2dArray:
        case ValidTextureOverload::kNumLevelsDepthCube:
        case ValidTextureOverload::kNumLevelsDepthCubeArray:
            return R"(textureNumLevels(texture))";
        case ValidTextureOverload::kNumSamplesDepthMultisampled2d:
        case ValidTextureOverload::kNumSamplesMultisampled2d:
            return R"(textureNumSamples(texture))";
        case ValidTextureOverload::kDimensions2dLevel:
        case ValidTextureOverload::kDimensions2dArrayLevel:
        case ValidTextureOverload::kDimensions3dLevel:
        case ValidTextureOverload::kDimensionsCubeLevel:
        case ValidTextureOverload::kDimensionsCubeArrayLevel:
        case ValidTextureOverload::kDimensionsDepth2dLevel:
        case ValidTextureOverload::kDimensionsDepth2dArrayLevel:
        case ValidTextureOverload::kDimensionsDepthCubeLevel:
        case ValidTextureOverload::kDimensionsDepthCubeArrayLevel:
            return R"(textureDimensions(texture, level))";
        case ValidTextureOverload::kSample1dF32:
            return R"(textureSample(texture, sampler, coords))";
        case ValidTextureOverload::kSample2dF32:
            return R"(textureSample(texture, sampler, coords))";
        case ValidTextureOverload::kSample2dOffsetF32:
            return R"(textureSample(texture, sampler, coords, offset))";
        case ValidTextureOverload::kSample2dArrayF32:
            return R"(textureSample(texture, sampler, coords, array_index))";
        case ValidTextureOverload::kSample2dArrayOffsetF32:
            return R"(textureSample(texture, sampler, coords, array_index, offset))";
        case ValidTextureOverload::kSample3dF32:
            return R"(textureSample(texture, sampler, coords))";
        case ValidTextureOverload::kSample3dOffsetF32:
            return R"(textureSample(texture, sampler, coords, offset))";
        case ValidTextureOverload::kSampleCubeF32:
            return R"(textureSample(texture, sampler, coords))";
        case ValidTextureOverload::kSampleCubeArrayF32:
            return R"(textureSample(texture, sampler, coords, array_index))";
        case ValidTextureOverload::kSampleDepth2dF32:
            return R"(textureSample(texture, sampler, coords))";
        case ValidTextureOverload::kSampleDepth2dOffsetF32:
            return R"(textureSample(texture, sampler, coords, offset))";
        case ValidTextureOverload::kSampleDepth2dArrayF32:
            return R"(textureSample(texture, sampler, coords, array_index))";
        case ValidTextureOverload::kSampleDepth2dArrayOffsetF32:
            return R"(textureSample(texture, sampler, coords, array_index, offset))";
        case ValidTextureOverload::kSampleDepthCubeF32:
            return R"(textureSample(texture, sampler, coords))";
        case ValidTextureOverload::kSampleDepthCubeArrayF32:
            return R"(textureSample(texture, sampler, coords, array_index))";
        case ValidTextureOverload::kSampleBias2dF32:
            return R"(textureSampleBias(texture, sampler, coords, bias))";
        case ValidTextureOverload::kSampleBias2dOffsetF32:
            return R"(textureSampleBias(texture, sampler, coords, bias, offset))";
        case ValidTextureOverload::kSampleBias2dArrayF32:
            return R"(textureSampleBias(texture, sampler, coords, array_index, bias))";
        case ValidTextureOverload::kSampleBias2dArrayOffsetF32:
            return R"(textureSampleBias(texture, sampler, coords, array_index, bias, offset))";
        case ValidTextureOverload::kSampleBias3dF32:
            return R"(textureSampleBias(texture, sampler, coords, bias))";
        case ValidTextureOverload::kSampleBias3dOffsetF32:
            return R"(textureSampleBias(texture, sampler, coords, bias, offset))";
        case ValidTextureOverload::kSampleBiasCubeF32:
            return R"(textureSampleBias(texture, sampler, coords, bias))";
        case ValidTextureOverload::kSampleBiasCubeArrayF32:
            return R"(textureSampleBias(texture, sampler, coords, array_index, bias))";
        case ValidTextureOverload::kSampleLevel2dF32:
            return R"(textureSampleLevel(texture, sampler, coords, level))";
        case ValidTextureOverload::kSampleLevel2dOffsetF32:
            return R"(textureSampleLevel(texture, sampler, coords, level, offset))";
        case ValidTextureOverload::kSampleLevel2dArrayF32:
            return R"(textureSampleLevel(texture, sampler, coords, array_index, level))";
        case ValidTextureOverload::kSampleLevel2dArrayOffsetF32:
            return R"(textureSampleLevel(texture, sampler, coords, array_index, level, offset))";
        case ValidTextureOverload::kSampleLevel3dF32:
            return R"(textureSampleLevel(texture, sampler, coords, level))";
        case ValidTextureOverload::kSampleLevel3dOffsetF32:
            return R"(textureSampleLevel(texture, sampler, coords, level, offset))";
        case ValidTextureOverload::kSampleLevelCubeF32:
            return R"(textureSampleLevel(texture, sampler, coords, level))";
        case ValidTextureOverload::kSampleLevelCubeArrayF32:
            return R"(textureSampleLevel(texture, sampler, coords, array_index, level))";
        case ValidTextureOverload::kSampleLevelDepth2dF32:
            return R"(textureSampleLevel(texture, sampler, coords, level))";
        case ValidTextureOverload::kSampleLevelDepth2dOffsetF32:
            return R"(textureSampleLevel(texture, sampler, coords, level, offset))";
        case ValidTextureOverload::kSampleLevelDepth2dArrayF32:
            return R"(textureSampleLevel(texture, sampler, coords, array_index, level))";
        case ValidTextureOverload::kSampleLevelDepth2dArrayOffsetF32:
            return R"(textureSampleLevel(texture, sampler, coords, array_index, level, offset))";
        case ValidTextureOverload::kSampleLevelDepthCubeF32:
            return R"(textureSampleLevel(texture, sampler, coords, level))";
        case ValidTextureOverload::kSampleLevelDepthCubeArrayF32:
            return R"(textureSampleLevel(texture, sampler, coords, array_index, level))";
        case ValidTextureOverload::kSampleGrad2dF32:
            return R"(textureSampleGrad(texture, sampler, coords, ddx, ddy))";
        case ValidTextureOverload::kSampleGrad2dOffsetF32:
            return R"(textureSampleGrad(texture, sampler, coords, ddx, ddy, offset))";
        case ValidTextureOverload::kSampleGrad2dArrayF32:
            return R"(textureSampleGrad(texture, sampler, coords, array_index, ddx, ddy))";
        case ValidTextureOverload::kSampleGrad2dArrayOffsetF32:
            return R"(textureSampleGrad(texture, sampler, coords, array_index, ddx, ddy, offset))";
        case ValidTextureOverload::kSampleGrad3dF32:
            return R"(textureSampleGrad(texture, sampler, coords, ddx, ddy))";
        case ValidTextureOverload::kSampleGrad3dOffsetF32:
            return R"(textureSampleGrad(texture, sampler, coords, ddx, ddy, offset))";
        case ValidTextureOverload::kSampleGradCubeF32:
            return R"(textureSampleGrad(texture, sampler, coords, ddx, ddy))";
        case ValidTextureOverload::kSampleGradCubeArrayF32:
            return R"(textureSampleGrad(texture, sampler, coords, array_index, ddx, ddy))";
        case ValidTextureOverload::kSampleCompareDepth2dF32:
            return R"(textureSampleCompare(texture, sampler, coords, depth_ref))";
        case ValidTextureOverload::kSampleCompareDepth2dOffsetF32:
            return R"(textureSampleCompare(texture, sampler, coords, depth_ref, offset))";
        case ValidTextureOverload::kSampleCompareDepth2dArrayF32:
            return R"(textureSampleCompare(texture, sampler, coords, array_index, depth_ref))";
        case ValidTextureOverload::kSampleCompareDepth2dArrayOffsetF32:
            return R"(textureSampleCompare(texture, sampler, coords, array_index, depth_ref, offset))";
        case ValidTextureOverload::kSampleCompareDepthCubeF32:
            return R"(textureSampleCompare(texture, sampler, coords, depth_ref))";
        case ValidTextureOverload::kSampleCompareDepthCubeArrayF32:
            return R"(textureSampleCompare(texture, sampler, coords, array_index, depth_ref))";
        case ValidTextureOverload::kSampleCompareLevelDepth2dF32:
            return R"(textureSampleCompareLevel(texture, sampler, coords, depth_ref))";
        case ValidTextureOverload::kSampleCompareLevelDepth2dOffsetF32:
            return R"(textureSampleCompareLevel(texture, sampler, coords, depth_ref, offset))";
        case ValidTextureOverload::kSampleCompareLevelDepth2dArrayF32:
            return R"(textureSampleCompareLevel(texture, sampler, coords, array_index, depth_ref))";
        case ValidTextureOverload::kSampleCompareLevelDepth2dArrayOffsetF32:
            return R"(textureSampleCompareLevel(texture, sampler, coords, array_index, depth_ref, offset))";
        case ValidTextureOverload::kSampleCompareLevelDepthCubeF32:
            return R"(textureSampleCompareLevel(texture, sampler, coords, depth_ref))";
        case ValidTextureOverload::kSampleCompareLevelDepthCubeArrayF32:
            return R"(textureSampleCompareLevel(texture, sampler, coords, array_index, depth_ref))";
        case ValidTextureOverload::kLoad1dLevelF32:
        case ValidTextureOverload::kLoad1dLevelU32:
        case ValidTextureOverload::kLoad1dLevelI32:
        case ValidTextureOverload::kLoad2dLevelF32:
        case ValidTextureOverload::kLoad2dLevelU32:
        case ValidTextureOverload::kLoad2dLevelI32:
            return R"(textureLoad(texture, coords, level))";
        case ValidTextureOverload::kLoad2dArrayLevelF32:
        case ValidTextureOverload::kLoad2dArrayLevelU32:
        case ValidTextureOverload::kLoad2dArrayLevelI32:
            return R"(textureLoad(texture, coords, array_index, level))";
        case ValidTextureOverload::kLoad3dLevelF32:
        case ValidTextureOverload::kLoad3dLevelU32:
        case ValidTextureOverload::kLoad3dLevelI32:
        case ValidTextureOverload::kLoadDepth2dLevelF32:
            return R"(textureLoad(texture, coords, level))";
        case ValidTextureOverload::kLoadDepthMultisampled2dF32:
        case ValidTextureOverload::kLoadMultisampled2dF32:
        case ValidTextureOverload::kLoadMultisampled2dU32:
        case ValidTextureOverload::kLoadMultisampled2dI32:
            return R"(textureLoad(texture, coords, sample_index))";
        case ValidTextureOverload::kLoadDepth2dArrayLevelF32:
            return R"(textureLoad(texture, coords, array_index, level))";
        case ValidTextureOverload::kStoreWO1dRgba32float:
        case ValidTextureOverload::kStoreWO2dRgba32float:
        case ValidTextureOverload::kStoreWO3dRgba32float:
            return R"(textureStore(texture, coords, value))";
        case ValidTextureOverload::kStoreWO2dArrayRgba32float:
            return R"(textureStore(texture, coords, array_index, value))";
    }
    return "<unmatched texture overload>";
}

TEST_P(ResolverBuiltinTest_Texture, Call) {
    auto param = GetParam();

    param.BuildTextureVariable(this);
    param.BuildSamplerVariable(this);

    auto* call = Call(param.function, param.args(this));
    auto* stmt = param.returns_value ? static_cast<const ast::Statement*>(Assign(Phony(), call))
                                     : static_cast<const ast::Statement*>(CallStmt(call));
    Func("func", utils::Empty, ty.void_(), utils::Vector{stmt},
         utils::Vector{Stage(ast::PipelineStage::kFragment)});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    if (std::string(param.function) == "textureDimensions") {
        switch (param.texture_dimension) {
            default: {
                utils::StringStream str;
                str << param.texture_dimension;
                FAIL() << "invalid texture dimensions: " << str.str();
            }
            case type::TextureDimension::k1d:
                EXPECT_TRUE(TypeOf(call)->Is<type::U32>());
                break;
            case type::TextureDimension::k2d:
            case type::TextureDimension::k2dArray:
            case type::TextureDimension::kCube:
            case type::TextureDimension::kCubeArray: {
                auto* vec = As<type::Vector>(TypeOf(call));
                ASSERT_NE(vec, nullptr);
                EXPECT_EQ(vec->Width(), 2u);
                EXPECT_TRUE(vec->type()->Is<type::U32>());
                break;
            }
            case type::TextureDimension::k3d: {
                auto* vec = As<type::Vector>(TypeOf(call));
                ASSERT_NE(vec, nullptr);
                EXPECT_EQ(vec->Width(), 3u);
                EXPECT_TRUE(vec->type()->Is<type::U32>());
                break;
            }
        }
    } else if (std::string(param.function) == "textureNumLayers") {
        EXPECT_TRUE(TypeOf(call)->Is<type::U32>());
    } else if (std::string(param.function) == "textureNumLevels") {
        EXPECT_TRUE(TypeOf(call)->Is<type::U32>());
    } else if (std::string(param.function) == "textureNumSamples") {
        EXPECT_TRUE(TypeOf(call)->Is<type::U32>());
    } else if (std::string(param.function) == "textureStore") {
        EXPECT_TRUE(TypeOf(call)->Is<type::Void>());
    } else if (std::string(param.function) == "textureGather") {
        auto* vec = As<type::Vector>(TypeOf(call));
        ASSERT_NE(vec, nullptr);
        EXPECT_EQ(vec->Width(), 4u);
        switch (param.texture_data_type) {
            case ast::builtin::test::TextureDataType::kF32:
                EXPECT_TRUE(vec->type()->Is<type::F32>());
                break;
            case ast::builtin::test::TextureDataType::kU32:
                EXPECT_TRUE(vec->type()->Is<type::U32>());
                break;
            case ast::builtin::test::TextureDataType::kI32:
                EXPECT_TRUE(vec->type()->Is<type::I32>());
                break;
        }
    } else if (std::string(param.function) == "textureGatherCompare") {
        auto* vec = As<type::Vector>(TypeOf(call));
        ASSERT_NE(vec, nullptr);
        EXPECT_EQ(vec->Width(), 4u);
        EXPECT_TRUE(vec->type()->Is<type::F32>());
    } else {
        switch (param.texture_kind) {
            case ast::builtin::test::TextureKind::kRegular:
            case ast::builtin::test::TextureKind::kMultisampled:
            case ast::builtin::test::TextureKind::kStorage: {
                auto* vec = TypeOf(call)->As<type::Vector>();
                ASSERT_NE(vec, nullptr);
                switch (param.texture_data_type) {
                    case ast::builtin::test::TextureDataType::kF32:
                        EXPECT_TRUE(vec->type()->Is<type::F32>());
                        break;
                    case ast::builtin::test::TextureDataType::kU32:
                        EXPECT_TRUE(vec->type()->Is<type::U32>());
                        break;
                    case ast::builtin::test::TextureDataType::kI32:
                        EXPECT_TRUE(vec->type()->Is<type::I32>());
                        break;
                }
                break;
            }
            case ast::builtin::test::TextureKind::kDepth:
            case ast::builtin::test::TextureKind::kDepthMultisampled: {
                EXPECT_TRUE(TypeOf(call)->Is<type::F32>());
                break;
            }
        }
    }

    auto* call_sem = Sem().Get<sem::Call>(call);
    ASSERT_NE(call_sem, nullptr);
    auto* target = call_sem->Target();
    ASSERT_NE(target, nullptr);

    auto got = texture_builtin_tests::to_str(param.function, target->Parameters());
    auto* expected = expected_texture_overload(param.overload);
    EXPECT_EQ(got, expected);
}

}  // namespace texture_builtin_tests

// Tests for Data Packing builtins
namespace data_packing_builtin_tests {

using ResolverBuiltinTest_DataPacking = ResolverTestWithParam<BuiltinData>;
TEST_P(ResolverBuiltinTest_DataPacking, InferType) {
    auto param = GetParam();

    bool pack4 = param.builtin == builtin::Function::kPack4X8Snorm ||
                 param.builtin == builtin::Function::kPack4X8Unorm;

    auto* call = pack4 ? Call(param.name, vec4<f32>(1_f, 2_f, 3_f, 4_f))
                       : Call(param.name, vec2<f32>(1_f, 2_f));
    WrapInFunction(call);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    ASSERT_NE(TypeOf(call), nullptr);
    EXPECT_TRUE(TypeOf(call)->Is<type::U32>());
}

TEST_P(ResolverBuiltinTest_DataPacking, Error_IncorrectParamType) {
    auto param = GetParam();

    bool pack4 = param.builtin == builtin::Function::kPack4X8Snorm ||
                 param.builtin == builtin::Function::kPack4X8Unorm;

    auto* call = pack4 ? Call(param.name, vec4<i32>(1_i, 2_i, 3_i, 4_i))
                       : Call(param.name, vec2<i32>(1_i, 2_i));
    WrapInFunction(call);

    EXPECT_FALSE(r()->Resolve());

    EXPECT_THAT(r()->error(), HasSubstr("error: no matching call to " + std::string(param.name)));
}

TEST_P(ResolverBuiltinTest_DataPacking, Error_NoParams) {
    auto param = GetParam();

    auto* call = Call(param.name);
    WrapInFunction(call);

    EXPECT_FALSE(r()->Resolve());

    EXPECT_THAT(r()->error(), HasSubstr("error: no matching call to " + std::string(param.name)));
}

TEST_P(ResolverBuiltinTest_DataPacking, Error_TooManyParams) {
    auto param = GetParam();

    bool pack4 = param.builtin == builtin::Function::kPack4X8Snorm ||
                 param.builtin == builtin::Function::kPack4X8Unorm;

    auto* call = pack4 ? Call(param.name, vec4<f32>(1_f, 2_f, 3_f, 4_f), 1_f)
                       : Call(param.name, vec2<f32>(1_f, 2_f), 1_f);
    WrapInFunction(call);

    EXPECT_FALSE(r()->Resolve());

    EXPECT_THAT(r()->error(), HasSubstr("error: no matching call to " + std::string(param.name)));
}

INSTANTIATE_TEST_SUITE_P(
    ResolverTest,
    ResolverBuiltinTest_DataPacking,
    testing::Values(BuiltinData{"pack4x8snorm", builtin::Function::kPack4X8Snorm},
                    BuiltinData{"pack4x8unorm", builtin::Function::kPack4X8Unorm},
                    BuiltinData{"pack2x16snorm", builtin::Function::kPack2X16Snorm},
                    BuiltinData{"pack2x16unorm", builtin::Function::kPack2X16Unorm},
                    BuiltinData{"pack2x16float", builtin::Function::kPack2X16Float}));

}  // namespace data_packing_builtin_tests

// Tests for Data Unpacking builtins
namespace data_unpacking_builtin_tests {

using ResolverBuiltinTest_DataUnpacking = ResolverTestWithParam<BuiltinData>;
TEST_P(ResolverBuiltinTest_DataUnpacking, InferType) {
    auto param = GetParam();

    bool pack4 = param.builtin == builtin::Function::kUnpack4X8Snorm ||
                 param.builtin == builtin::Function::kUnpack4X8Unorm;

    auto* call = Call(param.name, 1_u);
    WrapInFunction(call);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    ASSERT_NE(TypeOf(call), nullptr);
    EXPECT_TRUE(TypeOf(call)->is_float_vector());
    if (pack4) {
        EXPECT_EQ(TypeOf(call)->As<type::Vector>()->Width(), 4u);
    } else {
        EXPECT_EQ(TypeOf(call)->As<type::Vector>()->Width(), 2u);
    }
}

INSTANTIATE_TEST_SUITE_P(
    ResolverTest,
    ResolverBuiltinTest_DataUnpacking,
    testing::Values(BuiltinData{"unpack4x8snorm", builtin::Function::kUnpack4X8Snorm},
                    BuiltinData{"unpack4x8unorm", builtin::Function::kUnpack4X8Unorm},
                    BuiltinData{"unpack2x16snorm", builtin::Function::kUnpack2X16Snorm},
                    BuiltinData{"unpack2x16unorm", builtin::Function::kUnpack2X16Unorm},
                    BuiltinData{"unpack2x16float", builtin::Function::kUnpack2X16Float}));

}  // namespace data_unpacking_builtin_tests

// Tests for Synchronization builtins
namespace synchronization_builtin_tests {

using ResolverBuiltinTest_Barrier = ResolverTestWithParam<BuiltinData>;
TEST_P(ResolverBuiltinTest_Barrier, InferType) {
    auto param = GetParam();

    auto* call = Call(param.name);
    WrapInFunction(CallStmt(call));

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    ASSERT_NE(TypeOf(call), nullptr);
    EXPECT_TRUE(TypeOf(call)->Is<type::Void>());
}

TEST_P(ResolverBuiltinTest_Barrier, Error_TooManyParams) {
    auto param = GetParam();

    auto* call = Call(param.name, vec4<f32>(1_f, 2_f, 3_f, 4_f), 1_f);
    WrapInFunction(CallStmt(call));

    EXPECT_FALSE(r()->Resolve());

    EXPECT_THAT(r()->error(), HasSubstr("error: no matching call to " + std::string(param.name)));
}

INSTANTIATE_TEST_SUITE_P(
    ResolverTest,
    ResolverBuiltinTest_Barrier,
    testing::Values(BuiltinData{"storageBarrier", builtin::Function::kStorageBarrier},
                    BuiltinData{"workgroupBarrier", builtin::Function::kWorkgroupBarrier}));

}  // namespace synchronization_builtin_tests

}  // namespace
}  // namespace tint::resolver
