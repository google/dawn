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

#include <type_traits>

#include "src/tint/lang/wgsl/ast/bitcast_expression.h"
#include "src/tint/lang/wgsl/resolver/resolver.h"
#include "src/tint/lang/wgsl/resolver/resolver_helper_test.h"

#include "gmock/gmock.h"

namespace tint::resolver {
namespace {

using namespace tint::builtin::fluent_types;  // NOLINT
using namespace tint::number_suffixes;        // NOLINT

struct Type {
    template <typename T, std::enable_if_t<IsVector<T>, bool> = true>
    static constexpr bool UsedF16() {
        return std::is_same_v<typename T::type, f16>;
    }

    template <typename T, std::enable_if_t<!IsVector<T>, bool> = true>
    static constexpr bool UsedF16() {
        return std::is_same_v<T, f16>;
    }

    template <typename T>
    static constexpr Type Create() {
        return Type{builder::DataType<T>::AST, builder::DataType<T>::Sem,
                    builder::DataType<T>::ExprFromDouble, UsedF16<T>()};
    }

    builder::ast_type_func_ptr ast;
    builder::sem_type_func_ptr sem;
    builder::ast_expr_from_double_func_ptr expr;
    bool used_f16;
};

// Valids numeric scalar and vector types of all bit width
static constexpr Type k16BitsNumericTypes[] = {
    Type::Create<f16>(),
};
static constexpr Type k32BitsNumericTypes[] = {
    Type::Create<f32>(),
    Type::Create<i32>(),
    Type::Create<u32>(),
    Type::Create<vec2<f16>>(),
};
static constexpr Type k48BitsNumericTypes[] = {
    Type::Create<vec3<f16>>(),
};
static constexpr Type k64BitsNumericTypes[] = {
    Type::Create<vec2<f32>>(),
    Type::Create<vec2<i32>>(),
    Type::Create<vec2<u32>>(),
    Type::Create<vec4<f16>>(),
};
static constexpr Type k96BitsNumericTypes[] = {
    Type::Create<vec3<f32>>(),
    Type::Create<vec3<i32>>(),
    Type::Create<vec3<u32>>(),
};
static constexpr Type k128BitsNumericTypes[] = {
    Type::Create<vec4<f32>>(),
    Type::Create<vec4<i32>>(),
    Type::Create<vec4<u32>>(),
};

static constexpr Type kInvalid[] = {
    // A non-exhaustive selection of uncastable types
    Type::Create<bool>(),
    Type::Create<vec2<bool>>(),
    Type::Create<vec3<bool>>(),
    Type::Create<vec4<bool>>(),
    Type::Create<array<i32, 2>>(),
    Type::Create<array<u32, 3>>(),
    Type::Create<array<f32, 4>>(),
    Type::Create<array<bool, 5>>(),
    Type::Create<mat2x2<f32>>(),
    Type::Create<mat3x3<f32>>(),
    Type::Create<mat4x4<f32>>(),
    Type::Create<ptr<private_, i32>>(),
    Type::Create<ptr<private_, array<i32, 2>>>(),
    Type::Create<ptr<private_, mat2x2<f32>>>(),
};

using ResolverBitcastValidationTest = ResolverTestWithParam<std::tuple<Type, Type>>;

////////////////////////////////////////////////////////////////////////////////
// Valid bitcasts
////////////////////////////////////////////////////////////////////////////////
using ResolverBitcastValidationTestPass = ResolverBitcastValidationTest;
TEST_P(ResolverBitcastValidationTestPass, Test) {
    auto src = std::get<0>(GetParam());
    auto dst = std::get<1>(GetParam());

    if (src.used_f16 || dst.used_f16) {
        Enable(builtin::Extension::kF16);
    }

    auto* cast = Bitcast(dst.ast(*this), src.expr(*this, 0));
    WrapInFunction(cast);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(TypeOf(cast), dst.sem(*this));
}
INSTANTIATE_TEST_SUITE_P(16Bits,
                         ResolverBitcastValidationTestPass,
                         testing::Combine(testing::ValuesIn(k16BitsNumericTypes),
                                          testing::ValuesIn(k16BitsNumericTypes)));
INSTANTIATE_TEST_SUITE_P(32Bits,
                         ResolverBitcastValidationTestPass,
                         testing::Combine(testing::ValuesIn(k32BitsNumericTypes),
                                          testing::ValuesIn(k32BitsNumericTypes)));
INSTANTIATE_TEST_SUITE_P(48Bits,
                         ResolverBitcastValidationTestPass,
                         testing::Combine(testing::ValuesIn(k48BitsNumericTypes),
                                          testing::ValuesIn(k48BitsNumericTypes)));
INSTANTIATE_TEST_SUITE_P(64Bits,
                         ResolverBitcastValidationTestPass,
                         testing::Combine(testing::ValuesIn(k64BitsNumericTypes),
                                          testing::ValuesIn(k64BitsNumericTypes)));
INSTANTIATE_TEST_SUITE_P(96Bits,
                         ResolverBitcastValidationTestPass,
                         testing::Combine(testing::ValuesIn(k96BitsNumericTypes),
                                          testing::ValuesIn(k96BitsNumericTypes)));
INSTANTIATE_TEST_SUITE_P(128Bits,
                         ResolverBitcastValidationTestPass,
                         testing::Combine(testing::ValuesIn(k128BitsNumericTypes),
                                          testing::ValuesIn(k128BitsNumericTypes)));

////////////////////////////////////////////////////////////////////////////////
// Invalid source type for bitcasts
////////////////////////////////////////////////////////////////////////////////
using ResolverBitcastValidationTestInvalidSrcTy = ResolverBitcastValidationTest;
TEST_P(ResolverBitcastValidationTestInvalidSrcTy, Test) {
    auto src = std::get<0>(GetParam());
    auto dst = std::get<1>(GetParam());

    if (src.used_f16 || dst.used_f16) {
        Enable(builtin::Extension::kF16);
    }

    auto* cast = Bitcast(dst.ast(*this), Expr(Source{{12, 34}}, "src"));
    WrapInFunction(Let("src", src.expr(*this, 0)), cast);

    auto expected = "12:34 error: '" + src.sem(*this)->FriendlyName() + "' cannot be bitcast";

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), expected);
}
INSTANTIATE_TEST_SUITE_P(16Bits,
                         ResolverBitcastValidationTestInvalidSrcTy,
                         testing::Combine(testing::ValuesIn(kInvalid),
                                          testing::ValuesIn(k16BitsNumericTypes)));
INSTANTIATE_TEST_SUITE_P(32Bits,
                         ResolverBitcastValidationTestInvalidSrcTy,
                         testing::Combine(testing::ValuesIn(kInvalid),
                                          testing::ValuesIn(k32BitsNumericTypes)));
INSTANTIATE_TEST_SUITE_P(48Bits,
                         ResolverBitcastValidationTestInvalidSrcTy,
                         testing::Combine(testing::ValuesIn(kInvalid),
                                          testing::ValuesIn(k48BitsNumericTypes)));
INSTANTIATE_TEST_SUITE_P(64Bits,
                         ResolverBitcastValidationTestInvalidSrcTy,
                         testing::Combine(testing::ValuesIn(kInvalid),
                                          testing::ValuesIn(k64BitsNumericTypes)));
INSTANTIATE_TEST_SUITE_P(96Bits,
                         ResolverBitcastValidationTestInvalidSrcTy,
                         testing::Combine(testing::ValuesIn(kInvalid),
                                          testing::ValuesIn(k96BitsNumericTypes)));
INSTANTIATE_TEST_SUITE_P(128Bits,
                         ResolverBitcastValidationTestInvalidSrcTy,
                         testing::Combine(testing::ValuesIn(kInvalid),
                                          testing::ValuesIn(k128BitsNumericTypes)));

////////////////////////////////////////////////////////////////////////////////
// Invalid target type for bitcasts
////////////////////////////////////////////////////////////////////////////////
using ResolverBitcastValidationTestInvalidDstTy = ResolverBitcastValidationTest;
TEST_P(ResolverBitcastValidationTestInvalidDstTy, Test) {
    auto src = std::get<0>(GetParam());
    auto dst = std::get<1>(GetParam());

    if (src.used_f16 || dst.used_f16) {
        Enable(builtin::Extension::kF16);
    }

    // Use an alias so we can put a Source on the bitcast type
    Alias("T", dst.ast(*this));
    WrapInFunction(Bitcast(ty(Source{{12, 34}}, "T"), src.expr(*this, 0)));

    auto expected = "12:34 error: cannot bitcast to '" + dst.sem(*this)->FriendlyName() + "'";

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), expected);
}
INSTANTIATE_TEST_SUITE_P(16Bits,
                         ResolverBitcastValidationTestInvalidDstTy,
                         testing::Combine(testing::ValuesIn(k16BitsNumericTypes),
                                          testing::ValuesIn(kInvalid)));
INSTANTIATE_TEST_SUITE_P(32Bits,
                         ResolverBitcastValidationTestInvalidDstTy,
                         testing::Combine(testing::ValuesIn(k32BitsNumericTypes),
                                          testing::ValuesIn(kInvalid)));
INSTANTIATE_TEST_SUITE_P(48Bits,
                         ResolverBitcastValidationTestInvalidDstTy,
                         testing::Combine(testing::ValuesIn(k48BitsNumericTypes),
                                          testing::ValuesIn(kInvalid)));
INSTANTIATE_TEST_SUITE_P(64Bits,
                         ResolverBitcastValidationTestInvalidDstTy,
                         testing::Combine(testing::ValuesIn(k64BitsNumericTypes),
                                          testing::ValuesIn(kInvalid)));
INSTANTIATE_TEST_SUITE_P(96Bits,
                         ResolverBitcastValidationTestInvalidDstTy,
                         testing::Combine(testing::ValuesIn(k96BitsNumericTypes),
                                          testing::ValuesIn(kInvalid)));
INSTANTIATE_TEST_SUITE_P(128Bits,
                         ResolverBitcastValidationTestInvalidDstTy,
                         testing::Combine(testing::ValuesIn(k128BitsNumericTypes),
                                          testing::ValuesIn(kInvalid)));

////////////////////////////////////////////////////////////////////////////////
// Incompatible bitcast, but both src and dst types are valid
////////////////////////////////////////////////////////////////////////////////
using ResolverBitcastValidationTestIncompatible = ResolverBitcastValidationTest;
TEST_P(ResolverBitcastValidationTestIncompatible, Test) {
    auto src = std::get<0>(GetParam());
    auto dst = std::get<1>(GetParam());

    if (src.used_f16 || dst.used_f16) {
        Enable(builtin::Extension::kF16);
    }

    WrapInFunction(Bitcast(Source{{12, 34}}, dst.ast(*this), src.expr(*this, 0)));

    auto expected = "12:34 error: cannot bitcast from '" + src.sem(*this)->FriendlyName() +
                    "' to '" + dst.sem(*this)->FriendlyName() + "'";

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), expected);
}
INSTANTIATE_TEST_SUITE_P(16BitsTo32Bits,
                         ResolverBitcastValidationTestIncompatible,
                         testing::Combine(testing::ValuesIn(k16BitsNumericTypes),
                                          testing::ValuesIn(k32BitsNumericTypes)));
INSTANTIATE_TEST_SUITE_P(16BitsTo48Bits,
                         ResolverBitcastValidationTestIncompatible,
                         testing::Combine(testing::ValuesIn(k16BitsNumericTypes),
                                          testing::ValuesIn(k48BitsNumericTypes)));
INSTANTIATE_TEST_SUITE_P(16BitsTo64Bits,
                         ResolverBitcastValidationTestIncompatible,
                         testing::Combine(testing::ValuesIn(k16BitsNumericTypes),
                                          testing::ValuesIn(k64BitsNumericTypes)));
INSTANTIATE_TEST_SUITE_P(16BitsTo96Bits,
                         ResolverBitcastValidationTestIncompatible,
                         testing::Combine(testing::ValuesIn(k16BitsNumericTypes),
                                          testing::ValuesIn(k96BitsNumericTypes)));
INSTANTIATE_TEST_SUITE_P(16BitsTo128Bits,
                         ResolverBitcastValidationTestIncompatible,
                         testing::Combine(testing::ValuesIn(k16BitsNumericTypes),
                                          testing::ValuesIn(k128BitsNumericTypes)));
INSTANTIATE_TEST_SUITE_P(32BitsTo16Bits,
                         ResolverBitcastValidationTestIncompatible,
                         testing::Combine(testing::ValuesIn(k32BitsNumericTypes),
                                          testing::ValuesIn(k16BitsNumericTypes)));
INSTANTIATE_TEST_SUITE_P(32BitsTo48Bits,
                         ResolverBitcastValidationTestIncompatible,
                         testing::Combine(testing::ValuesIn(k32BitsNumericTypes),
                                          testing::ValuesIn(k48BitsNumericTypes)));
INSTANTIATE_TEST_SUITE_P(32BitsTo64Bits,
                         ResolverBitcastValidationTestIncompatible,
                         testing::Combine(testing::ValuesIn(k32BitsNumericTypes),
                                          testing::ValuesIn(k64BitsNumericTypes)));
INSTANTIATE_TEST_SUITE_P(32BitsTo96Bits,
                         ResolverBitcastValidationTestIncompatible,
                         testing::Combine(testing::ValuesIn(k32BitsNumericTypes),
                                          testing::ValuesIn(k96BitsNumericTypes)));
INSTANTIATE_TEST_SUITE_P(32BitsTo128Bits,
                         ResolverBitcastValidationTestIncompatible,
                         testing::Combine(testing::ValuesIn(k32BitsNumericTypes),
                                          testing::ValuesIn(k128BitsNumericTypes)));
INSTANTIATE_TEST_SUITE_P(48BitsTo16Bits,
                         ResolverBitcastValidationTestIncompatible,
                         testing::Combine(testing::ValuesIn(k48BitsNumericTypes),
                                          testing::ValuesIn(k16BitsNumericTypes)));
INSTANTIATE_TEST_SUITE_P(48BitsTo32Bits,
                         ResolverBitcastValidationTestIncompatible,
                         testing::Combine(testing::ValuesIn(k48BitsNumericTypes),
                                          testing::ValuesIn(k32BitsNumericTypes)));
INSTANTIATE_TEST_SUITE_P(48BitsTo64Bits,
                         ResolverBitcastValidationTestIncompatible,
                         testing::Combine(testing::ValuesIn(k48BitsNumericTypes),
                                          testing::ValuesIn(k64BitsNumericTypes)));
INSTANTIATE_TEST_SUITE_P(48BitsTo96Bits,
                         ResolverBitcastValidationTestIncompatible,
                         testing::Combine(testing::ValuesIn(k48BitsNumericTypes),
                                          testing::ValuesIn(k96BitsNumericTypes)));
INSTANTIATE_TEST_SUITE_P(48BitsTo128Bits,
                         ResolverBitcastValidationTestIncompatible,
                         testing::Combine(testing::ValuesIn(k48BitsNumericTypes),
                                          testing::ValuesIn(k128BitsNumericTypes)));
INSTANTIATE_TEST_SUITE_P(64BitsTo16Bits,
                         ResolverBitcastValidationTestIncompatible,
                         testing::Combine(testing::ValuesIn(k64BitsNumericTypes),
                                          testing::ValuesIn(k16BitsNumericTypes)));
INSTANTIATE_TEST_SUITE_P(64BitsTo32Bits,
                         ResolverBitcastValidationTestIncompatible,
                         testing::Combine(testing::ValuesIn(k64BitsNumericTypes),
                                          testing::ValuesIn(k32BitsNumericTypes)));
INSTANTIATE_TEST_SUITE_P(64BitsTo48Bits,
                         ResolverBitcastValidationTestIncompatible,
                         testing::Combine(testing::ValuesIn(k64BitsNumericTypes),
                                          testing::ValuesIn(k48BitsNumericTypes)));
INSTANTIATE_TEST_SUITE_P(64BitsTo96Bits,
                         ResolverBitcastValidationTestIncompatible,
                         testing::Combine(testing::ValuesIn(k64BitsNumericTypes),
                                          testing::ValuesIn(k96BitsNumericTypes)));
INSTANTIATE_TEST_SUITE_P(64BitsTo128Bits,
                         ResolverBitcastValidationTestIncompatible,
                         testing::Combine(testing::ValuesIn(k64BitsNumericTypes),
                                          testing::ValuesIn(k128BitsNumericTypes)));
INSTANTIATE_TEST_SUITE_P(96BitsTo16Bits,
                         ResolverBitcastValidationTestIncompatible,
                         testing::Combine(testing::ValuesIn(k96BitsNumericTypes),
                                          testing::ValuesIn(k16BitsNumericTypes)));
INSTANTIATE_TEST_SUITE_P(96BitsTo32Bits,
                         ResolverBitcastValidationTestIncompatible,
                         testing::Combine(testing::ValuesIn(k96BitsNumericTypes),
                                          testing::ValuesIn(k32BitsNumericTypes)));
INSTANTIATE_TEST_SUITE_P(96BitsTo48Bits,
                         ResolverBitcastValidationTestIncompatible,
                         testing::Combine(testing::ValuesIn(k96BitsNumericTypes),
                                          testing::ValuesIn(k48BitsNumericTypes)));
INSTANTIATE_TEST_SUITE_P(96BitsTo64Bits,
                         ResolverBitcastValidationTestIncompatible,
                         testing::Combine(testing::ValuesIn(k96BitsNumericTypes),
                                          testing::ValuesIn(k64BitsNumericTypes)));
INSTANTIATE_TEST_SUITE_P(96BitsTo128Bits,
                         ResolverBitcastValidationTestIncompatible,
                         testing::Combine(testing::ValuesIn(k96BitsNumericTypes),
                                          testing::ValuesIn(k128BitsNumericTypes)));
INSTANTIATE_TEST_SUITE_P(128BitsTo16Bits,
                         ResolverBitcastValidationTestIncompatible,
                         testing::Combine(testing::ValuesIn(k128BitsNumericTypes),
                                          testing::ValuesIn(k16BitsNumericTypes)));
INSTANTIATE_TEST_SUITE_P(128BitsTo32Bits,
                         ResolverBitcastValidationTestIncompatible,
                         testing::Combine(testing::ValuesIn(k128BitsNumericTypes),
                                          testing::ValuesIn(k32BitsNumericTypes)));
INSTANTIATE_TEST_SUITE_P(128BitsTo48Bits,
                         ResolverBitcastValidationTestIncompatible,
                         testing::Combine(testing::ValuesIn(k128BitsNumericTypes),
                                          testing::ValuesIn(k48BitsNumericTypes)));
INSTANTIATE_TEST_SUITE_P(128BitsTo64Bits,
                         ResolverBitcastValidationTestIncompatible,
                         testing::Combine(testing::ValuesIn(k128BitsNumericTypes),
                                          testing::ValuesIn(k64BitsNumericTypes)));
INSTANTIATE_TEST_SUITE_P(128BitsTo96Bits,
                         ResolverBitcastValidationTestIncompatible,
                         testing::Combine(testing::ValuesIn(k128BitsNumericTypes),
                                          testing::ValuesIn(k96BitsNumericTypes)));

////////////////////////////////////////////////////////////////////////////////
// Compile-time bitcasts to NaN or Inf are invalid
////////////////////////////////////////////////////////////////////////////////
using ResolverBitcastValidationTestInvalidConst = tint::resolver::ResolverTest;
TEST_F(ResolverBitcastValidationTestInvalidConst, ConstBitcastToF16NaN) {
    Enable(builtin::Extension::kF16);

    // Lower 16 bits of const u32 0x7e10 is NaN in f16.
    auto* a = Const("a", Expr(u32(0x00007e10)));
    auto* b = Let("b", Bitcast(Source{{12, 34}}, ty.Of<vec2<f16>>(), Expr("a")));
    WrapInFunction(a, b);

    auto expected = "12:34 error: value nan cannot be represented as 'f16'";

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), expected);
}

TEST_F(ResolverBitcastValidationTestInvalidConst, ConstBitcastToF16Inf) {
    Enable(builtin::Extension::kF16);

    // 0xfc00 is -Inf in f16.
    auto* a = Const("a", Call<vec2<u32>>(u32(0x00007010), u32(0xfc008000)));
    auto* b = Let("b", Bitcast(Source{{12, 34}}, ty.Of<vec4<f16>>(), Expr("a")));
    WrapInFunction(a, b);

    auto expected = "12:34 error: value -inf cannot be represented as 'f16'";

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), expected);
}

TEST_F(ResolverBitcastValidationTestInvalidConst, ConstBitcastToF32NaN) {
    // 0xffc00000 is NaN in f32.
    auto* a = Const("a", Expr(u32(0xffc00000)));
    auto* b = Let("b", Bitcast(Source{{12, 34}}, ty.Of<f32>(), Expr("a")));
    WrapInFunction(a, b);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(r()->error(), ::testing::HasSubstr("cannot be represented as 'f32'"));
}

TEST_F(ResolverBitcastValidationTestInvalidConst, ConstBitcastToF32Inf) {
    Enable(builtin::Extension::kF16);

    // 0x7f800000 is Inf in f32.
    auto* a = Const("a", Call<vec3<u32>>(u32(0xA0008000), u32(0x7f800000), u32(0x40000000)));
    auto* b = Let("b", Bitcast(Source{{12, 34}}, ty.Of<vec3<f32>>(), Expr("a")));
    WrapInFunction(a, b);

    auto expected = "12:34 error: value inf cannot be represented as 'f32'";

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), expected);
}

}  // namespace
}  // namespace tint::resolver
