// Copyright 2020 The Tint Authors.
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

#include "src/tint/utils/string_stream.h"
#include "src/tint/writer/msl/test_helper.h"

namespace tint::writer::msl {
namespace {

using MslGeneratorImplTest = TestHelper;

TEST_F(MslGeneratorImplTest, EmitExpression_MemberAccessor) {
    GlobalVar("str", ty.Of(Structure("my_str", utils::Vector{Member("mem", ty.f32())})),
              builtin::AddressSpace::kPrivate);
    auto* expr = MemberAccessor("str", "mem");
    WrapInFunction(expr);

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    ASSERT_TRUE(gen.EmitExpression(out, expr)) << gen.Diagnostics();
    EXPECT_EQ(out.str(), "str.mem");
}

TEST_F(MslGeneratorImplTest, EmitExpression_MemberAccessor_Swizzle_xyz) {
    GlobalVar("my_vec", ty.vec4<f32>(), builtin::AddressSpace::kPrivate);

    auto* expr = MemberAccessor("my_vec", "xyz");
    WrapInFunction(expr);

    GeneratorImpl& gen = Build();
    utils::StringStream out;
    ASSERT_TRUE(gen.EmitExpression(out, expr)) << gen.Diagnostics();
    EXPECT_EQ(out.str(), "my_vec.xyz");
}

TEST_F(MslGeneratorImplTest, EmitExpression_MemberAccessor_Swizzle_gbr) {
    GlobalVar("my_vec", ty.vec4<f32>(), builtin::AddressSpace::kPrivate);

    auto* expr = MemberAccessor("my_vec", "gbr");
    WrapInFunction(expr);

    GeneratorImpl& gen = Build();
    utils::StringStream out;
    ASSERT_TRUE(gen.EmitExpression(out, expr)) << gen.Diagnostics();
    EXPECT_EQ(out.str(), "my_vec.gbr");
}

}  // namespace
}  // namespace tint::writer::msl
