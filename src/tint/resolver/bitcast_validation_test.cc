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

#include "src/tint/ast/bitcast_expression.h"
#include "src/tint/resolver/resolver.h"
#include "src/tint/resolver/resolver_test_helper.h"

#include "gmock/gmock.h"

namespace tint::resolver {
namespace {

struct Type {
    template <typename T>
    static constexpr Type Create() {
        return Type{builder::DataType<T>::AST, builder::DataType<T>::Sem,
                    builder::DataType<T>::ExprFromDouble};
    }

    builder::ast_type_func_ptr ast;
    builder::sem_type_func_ptr sem;
    builder::ast_expr_from_double_func_ptr expr;
};

static constexpr Type kNumericScalars[] = {
    Type::Create<f32>(),
    Type::Create<i32>(),
    Type::Create<u32>(),
};
static constexpr Type kVec2NumericScalars[] = {
    Type::Create<builder::vec2<f32>>(),
    Type::Create<builder::vec2<i32>>(),
    Type::Create<builder::vec2<u32>>(),
};
static constexpr Type kVec3NumericScalars[] = {
    Type::Create<builder::vec3<f32>>(),
    Type::Create<builder::vec3<i32>>(),
    Type::Create<builder::vec3<u32>>(),
};
static constexpr Type kVec4NumericScalars[] = {
    Type::Create<builder::vec4<f32>>(),
    Type::Create<builder::vec4<i32>>(),
    Type::Create<builder::vec4<u32>>(),
};
static constexpr Type kInvalid[] = {
    // A non-exhaustive selection of uncastable types
    Type::Create<bool>(),
    Type::Create<builder::vec2<bool>>(),
    Type::Create<builder::vec3<bool>>(),
    Type::Create<builder::vec4<bool>>(),
    Type::Create<builder::array<2, i32>>(),
    Type::Create<builder::array<3, u32>>(),
    Type::Create<builder::array<4, f32>>(),
    Type::Create<builder::array<5, bool>>(),
    Type::Create<builder::mat2x2<f32>>(),
    Type::Create<builder::mat3x3<f32>>(),
    Type::Create<builder::mat4x4<f32>>(),
    Type::Create<builder::ptr<i32>>(),
    Type::Create<builder::ptr<builder::array<2, i32>>>(),
    Type::Create<builder::ptr<builder::mat2x2<f32>>>(),
};

using ResolverBitcastValidationTest = ResolverTestWithParam<std::tuple<Type, Type>>;

////////////////////////////////////////////////////////////////////////////////
// Valid bitcasts
////////////////////////////////////////////////////////////////////////////////
using ResolverBitcastValidationTestPass = ResolverBitcastValidationTest;
TEST_P(ResolverBitcastValidationTestPass, Test) {
    auto src = std::get<0>(GetParam());
    auto dst = std::get<1>(GetParam());

    auto* cast = Bitcast(dst.ast(*this), src.expr(*this, 0));
    WrapInFunction(cast);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(TypeOf(cast), dst.sem(*this));
}
INSTANTIATE_TEST_SUITE_P(Scalars,
                         ResolverBitcastValidationTestPass,
                         testing::Combine(testing::ValuesIn(kNumericScalars),
                                          testing::ValuesIn(kNumericScalars)));
INSTANTIATE_TEST_SUITE_P(Vec2,
                         ResolverBitcastValidationTestPass,
                         testing::Combine(testing::ValuesIn(kVec2NumericScalars),
                                          testing::ValuesIn(kVec2NumericScalars)));
INSTANTIATE_TEST_SUITE_P(Vec3,
                         ResolverBitcastValidationTestPass,
                         testing::Combine(testing::ValuesIn(kVec3NumericScalars),
                                          testing::ValuesIn(kVec3NumericScalars)));
INSTANTIATE_TEST_SUITE_P(Vec4,
                         ResolverBitcastValidationTestPass,
                         testing::Combine(testing::ValuesIn(kVec4NumericScalars),
                                          testing::ValuesIn(kVec4NumericScalars)));

////////////////////////////////////////////////////////////////////////////////
// Invalid source type for bitcasts
////////////////////////////////////////////////////////////////////////////////
using ResolverBitcastValidationTestInvalidSrcTy = ResolverBitcastValidationTest;
TEST_P(ResolverBitcastValidationTestInvalidSrcTy, Test) {
    auto src = std::get<0>(GetParam());
    auto dst = std::get<1>(GetParam());

    auto* cast = Bitcast(dst.ast(*this), Expr(Source{{12, 34}}, "src"));
    WrapInFunction(Let("src", src.expr(*this, 0)), cast);

    auto expected = "12:34 error: '" + src.sem(*this)->FriendlyName() + "' cannot be bitcast";

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), expected);
}
INSTANTIATE_TEST_SUITE_P(Scalars,
                         ResolverBitcastValidationTestInvalidSrcTy,
                         testing::Combine(testing::ValuesIn(kInvalid),
                                          testing::ValuesIn(kNumericScalars)));
INSTANTIATE_TEST_SUITE_P(Vec2,
                         ResolverBitcastValidationTestInvalidSrcTy,
                         testing::Combine(testing::ValuesIn(kInvalid),
                                          testing::ValuesIn(kVec2NumericScalars)));
INSTANTIATE_TEST_SUITE_P(Vec3,
                         ResolverBitcastValidationTestInvalidSrcTy,
                         testing::Combine(testing::ValuesIn(kInvalid),
                                          testing::ValuesIn(kVec3NumericScalars)));
INSTANTIATE_TEST_SUITE_P(Vec4,
                         ResolverBitcastValidationTestInvalidSrcTy,
                         testing::Combine(testing::ValuesIn(kInvalid),
                                          testing::ValuesIn(kVec4NumericScalars)));

////////////////////////////////////////////////////////////////////////////////
// Invalid target type for bitcasts
////////////////////////////////////////////////////////////////////////////////
using ResolverBitcastValidationTestInvalidDstTy = ResolverBitcastValidationTest;
TEST_P(ResolverBitcastValidationTestInvalidDstTy, Test) {
    auto src = std::get<0>(GetParam());
    auto dst = std::get<1>(GetParam());

    // Use an alias so we can put a Source on the bitcast type
    Alias("T", dst.ast(*this));
    WrapInFunction(Bitcast(ty(Source{{12, 34}}, "T"), src.expr(*this, 0)));

    auto expected = "12:34 error: cannot bitcast to '" + dst.sem(*this)->FriendlyName() + "'";

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), expected);
}
INSTANTIATE_TEST_SUITE_P(Scalars,
                         ResolverBitcastValidationTestInvalidDstTy,
                         testing::Combine(testing::ValuesIn(kNumericScalars),
                                          testing::ValuesIn(kInvalid)));
INSTANTIATE_TEST_SUITE_P(Vec2,
                         ResolverBitcastValidationTestInvalidDstTy,
                         testing::Combine(testing::ValuesIn(kVec2NumericScalars),
                                          testing::ValuesIn(kInvalid)));
INSTANTIATE_TEST_SUITE_P(Vec3,
                         ResolverBitcastValidationTestInvalidDstTy,
                         testing::Combine(testing::ValuesIn(kVec3NumericScalars),
                                          testing::ValuesIn(kInvalid)));
INSTANTIATE_TEST_SUITE_P(Vec4,
                         ResolverBitcastValidationTestInvalidDstTy,
                         testing::Combine(testing::ValuesIn(kVec4NumericScalars),
                                          testing::ValuesIn(kInvalid)));

////////////////////////////////////////////////////////////////////////////////
// Incompatible bitcast, but both src and dst types are valid
////////////////////////////////////////////////////////////////////////////////
using ResolverBitcastValidationTestIncompatible = ResolverBitcastValidationTest;
TEST_P(ResolverBitcastValidationTestIncompatible, Test) {
    auto src = std::get<0>(GetParam());
    auto dst = std::get<1>(GetParam());

    WrapInFunction(Bitcast(Source{{12, 34}}, dst.ast(*this), src.expr(*this, 0)));

    auto expected = "12:34 error: cannot bitcast from '" + src.sem(*this)->FriendlyName() +
                    "' to '" + dst.sem(*this)->FriendlyName() + "'";

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), expected);
}
INSTANTIATE_TEST_SUITE_P(ScalarToVec2,
                         ResolverBitcastValidationTestIncompatible,
                         testing::Combine(testing::ValuesIn(kNumericScalars),
                                          testing::ValuesIn(kVec2NumericScalars)));
INSTANTIATE_TEST_SUITE_P(Vec2ToVec3,
                         ResolverBitcastValidationTestIncompatible,
                         testing::Combine(testing::ValuesIn(kVec2NumericScalars),
                                          testing::ValuesIn(kVec3NumericScalars)));
INSTANTIATE_TEST_SUITE_P(Vec3ToVec4,
                         ResolverBitcastValidationTestIncompatible,
                         testing::Combine(testing::ValuesIn(kVec3NumericScalars),
                                          testing::ValuesIn(kVec4NumericScalars)));
INSTANTIATE_TEST_SUITE_P(Vec4ToScalar,
                         ResolverBitcastValidationTestIncompatible,
                         testing::Combine(testing::ValuesIn(kVec4NumericScalars),
                                          testing::ValuesIn(kNumericScalars)));

}  // namespace
}  // namespace tint::resolver
