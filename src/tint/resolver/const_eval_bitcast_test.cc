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

struct Case {
    Value input;
    struct Success {
        Value value;
    };
    struct Failure {
        builder::CreatePtrs create_ptrs;
    };
    utils::Result<Success, Failure> expected;
};

static std::ostream& operator<<(std::ostream& o, const Case& c) {
    o << "input: " << c.input;
    if (c.expected) {
        o << ", expected: " << c.expected.Get().value;
    } else {
        o << ", expected failed bitcast to " << c.expected.Failure().create_ptrs;
    }
    return o;
}

template <typename TO, typename FROM>
Case Success(FROM input, TO expected) {
    return Case{input, Case::Success{expected}};
}

template <typename TO, typename FROM>
Case Failure(FROM input) {
    return Case{input, Case::Failure{builder::CreatePtrsFor<TO>()}};
}

using ResolverConstEvalBitcastTest = ResolverTestWithParam<Case>;

TEST_P(ResolverConstEvalBitcastTest, Test) {
    const auto& input = GetParam().input;
    const auto& expected = GetParam().expected;

    // Get the target type CreatePtrs
    builder::CreatePtrs target_create_ptrs;
    if (expected) {
        target_create_ptrs = expected.Get().value.create_ptrs;
    } else {
        target_create_ptrs = expected.Failure().create_ptrs;
    }

    auto target_ty = target_create_ptrs.ast(*this);
    ASSERT_NE(target_ty, nullptr);
    auto* input_val = input.Expr(*this);
    const ast::Expression* expr = Bitcast(Source{{12, 34}}, target_ty, input_val);

    WrapInFunction(expr);

    auto* target_sem_ty = target_create_ptrs.sem(*this);

    if (expected) {
        EXPECT_TRUE(r()->Resolve()) << r()->error();

        auto* sem = Sem().GetVal(expr);
        ASSERT_NE(sem, nullptr);
        EXPECT_TYPE(sem->Type(), target_sem_ty);
        ASSERT_NE(sem->ConstantValue(), nullptr);
        EXPECT_TYPE(sem->ConstantValue()->Type(), target_sem_ty);

        auto expected_values = expected.Get().value.args;
        auto got_values = ScalarsFrom(sem->ConstantValue());
        EXPECT_EQ(expected_values, got_values);
    } else {
        ASSERT_FALSE(r()->Resolve());
        EXPECT_THAT(r()->error(), testing::StartsWith("12:34 error:"));
        EXPECT_THAT(r()->error(), testing::HasSubstr("cannot be represented as"));
    }
}

const u32 nan_as_u32 = utils::Bitcast<u32>(std::numeric_limits<float>::quiet_NaN());
const i32 nan_as_i32 = utils::Bitcast<i32>(std::numeric_limits<float>::quiet_NaN());
const u32 inf_as_u32 = utils::Bitcast<u32>(std::numeric_limits<float>::infinity());
const i32 inf_as_i32 = utils::Bitcast<i32>(std::numeric_limits<float>::infinity());
const u32 neg_inf_as_u32 = utils::Bitcast<u32>(-std::numeric_limits<float>::infinity());
const i32 neg_inf_as_i32 = utils::Bitcast<i32>(-std::numeric_limits<float>::infinity());

INSTANTIATE_TEST_SUITE_P(Bitcast,
                         ResolverConstEvalBitcastTest,
                         testing::ValuesIn({
                             // Bitcast to same (concrete) type, no change
                             Success(Val(0_u), Val(0_u)),                        //
                             Success(Val(0_i), Val(0_i)),                        //
                             Success(Val(0_f), Val(0_f)),                        //
                             Success(Val(123_u), Val(123_u)),                    //
                             Success(Val(123_i), Val(123_i)),                    //
                             Success(Val(123.456_f), Val(123.456_f)),            //
                             Success(Val(u32::Highest()), Val(u32::Highest())),  //
                             Success(Val(u32::Lowest()), Val(u32::Lowest())),    //
                             Success(Val(i32::Highest()), Val(i32::Highest())),  //
                             Success(Val(i32::Lowest()), Val(i32::Lowest())),    //
                             Success(Val(f32::Highest()), Val(f32::Highest())),  //
                             Success(Val(f32::Lowest()), Val(f32::Lowest())),    //

                             // Bitcast to different type
                             Success(Val(0_u), Val(0_i)),               //
                             Success(Val(0_u), Val(0_f)),               //
                             Success(Val(0_i), Val(0_u)),               //
                             Success(Val(0_i), Val(0_f)),               //
                             Success(Val(0.0_f), Val(0_i)),             //
                             Success(Val(0.0_f), Val(0_u)),             //
                             Success(Val(1_u), Val(1_i)),               //
                             Success(Val(1_u), Val(1.4013e-45_f)),      //
                             Success(Val(1_i), Val(1_u)),               //
                             Success(Val(1_i), Val(1.4013e-45_f)),      //
                             Success(Val(1.0_f), Val(0x3F800000_u)),    //
                             Success(Val(1.0_f), Val(0x3F800000_i)),    //
                             Success(Val(123_u), Val(123_i)),           //
                             Success(Val(123_u), Val(1.7236e-43_f)),    //
                             Success(Val(123_i), Val(123_u)),           //
                             Success(Val(123_i), Val(1.7236e-43_f)),    //
                             Success(Val(123.0_f), Val(0x42F60000_u)),  //
                             Success(Val(123.0_f), Val(0x42F60000_i)),  //

                             // Bitcast from abstract materializes lhs first,
                             // so same results as above.
                             Success(Val(0_a), Val(0_i)),               //
                             Success(Val(0_a), Val(0_f)),               //
                             Success(Val(0_a), Val(0_u)),               //
                             Success(Val(0_a), Val(0_f)),               //
                             Success(Val(0_a), Val(0_i)),               //
                             Success(Val(0_a), Val(0_u)),               //
                             Success(Val(1_a), Val(1_i)),               //
                             Success(Val(1_a), Val(1.4013e-45_f)),      //
                             Success(Val(1_a), Val(1_u)),               //
                             Success(Val(1_a), Val(1.4013e-45_f)),      //
                             Success(Val(1.0_a), Val(0x3F800000_u)),    //
                             Success(Val(1.0_a), Val(0x3F800000_i)),    //
                             Success(Val(123_a), Val(123_i)),           //
                             Success(Val(123_a), Val(1.7236e-43_f)),    //
                             Success(Val(123_a), Val(123_u)),           //
                             Success(Val(123_a), Val(1.7236e-43_f)),    //
                             Success(Val(123.0_a), Val(0x42F60000_u)),  //
                             Success(Val(123.0_a), Val(0x42F60000_i)),  //

                             // u32 <-> i32 sign bit
                             Success(Val(0xFFFFFFFF_u), Val(-1_i)),           //
                             Success(Val(-1_i), Val(0xFFFFFFFF_u)),           //
                             Success(Val(0x80000000_u), Val(i32::Lowest())),  //
                             Success(Val(i32::Lowest()), Val(0x80000000_u)),  //

                             // Vector tests
                             Success(Vec(0_u, 1_u, 123_u), Vec(0_i, 1_i, 123_i)),
                             Success(Vec(0.0_f, 1.0_f, 123.0_f),
                                     Vec(0_i, 0x3F800000_i, 0x42F60000_i)),

                             // Unrepresentable
                             Failure<f32>(Val(nan_as_u32)),                          //
                             Failure<f32>(Val(nan_as_i32)),                          //
                             Failure<f32>(Val(inf_as_u32)),                          //
                             Failure<f32>(Val(inf_as_i32)),                          //
                             Failure<f32>(Val(neg_inf_as_u32)),                      //
                             Failure<f32>(Val(neg_inf_as_i32)),                      //
                             Failure<builder::vec2<f32>>(Vec(nan_as_u32, 0_u)),      //
                             Failure<builder::vec2<f32>>(Vec(nan_as_i32, 0_i)),      //
                             Failure<builder::vec2<f32>>(Vec(inf_as_u32, 0_u)),      //
                             Failure<builder::vec2<f32>>(Vec(inf_as_i32, 0_i)),      //
                             Failure<builder::vec2<f32>>(Vec(neg_inf_as_u32, 0_u)),  //
                             Failure<builder::vec2<f32>>(Vec(neg_inf_as_i32, 0_i)),  //
                             Failure<builder::vec2<f32>>(Vec(0_u, nan_as_u32)),      //
                             Failure<builder::vec2<f32>>(Vec(0_i, nan_as_i32)),      //
                             Failure<builder::vec2<f32>>(Vec(0_u, inf_as_u32)),      //
                             Failure<builder::vec2<f32>>(Vec(0_i, inf_as_i32)),      //
                             Failure<builder::vec2<f32>>(Vec(0_u, neg_inf_as_u32)),  //
                             Failure<builder::vec2<f32>>(Vec(0_i, neg_inf_as_i32)),  //
                         }));

}  // namespace
}  // namespace tint::resolver
