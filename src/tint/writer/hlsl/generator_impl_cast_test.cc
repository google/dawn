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
#include "src/tint/writer/hlsl/test_helper.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::writer::hlsl {
namespace {

using HlslGeneratorImplTest_Cast = TestHelper;

TEST_F(HlslGeneratorImplTest_Cast, EmitExpression_Cast_Scalar) {
    auto* cast = Call<f32>(1_i);
    WrapInFunction(cast);

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    ASSERT_TRUE(gen.EmitExpression(out, cast)) << gen.Diagnostics();
    EXPECT_EQ(out.str(), "1.0f");
}

TEST_F(HlslGeneratorImplTest_Cast, EmitExpression_Cast_Vector) {
    auto* cast = vec3<f32>(vec3<i32>(1_i, 2_i, 3_i));
    WrapInFunction(cast);

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    ASSERT_TRUE(gen.EmitExpression(out, cast)) << gen.Diagnostics();
    EXPECT_EQ(out.str(), "float3(1.0f, 2.0f, 3.0f)");
}

}  // namespace
}  // namespace tint::writer::hlsl
