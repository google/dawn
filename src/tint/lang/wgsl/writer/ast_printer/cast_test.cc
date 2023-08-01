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

#include "src/tint/lang/wgsl/writer/ast_printer/helper_test.h"
#include "src/tint/utils/text/string_stream.h"

#include "gmock/gmock.h"

namespace tint::wgsl::writer {
namespace {

using namespace tint::builtin::fluent_types;  // NOLINT
using namespace tint::number_suffixes;        // NOLINT

using WgslASTPrinterTest = TestHelper;

TEST_F(WgslASTPrinterTest, EmitExpression_Cast_Scalar_F32_From_I32) {
    auto* cast = Call<f32>(1_i);
    WrapInFunction(cast);

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitExpression(out, cast);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "f32(1i)");
}

TEST_F(WgslASTPrinterTest, EmitExpression_Cast_Scalar_F16_From_I32) {
    Enable(builtin::Extension::kF16);

    auto* cast = Call<f16>(1_i);
    WrapInFunction(cast);

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitExpression(out, cast);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "f16(1i)");
}

TEST_F(WgslASTPrinterTest, EmitExpression_Cast_Vector_F32_From_I32) {
    auto* cast = Call<vec3<f32>>(Call<vec3<i32>>(1_i, 2_i, 3_i));
    WrapInFunction(cast);

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitExpression(out, cast);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "vec3<f32>(vec3<i32>(1i, 2i, 3i))");
}

TEST_F(WgslASTPrinterTest, EmitExpression_Cast_Vector_F16_From_I32) {
    Enable(builtin::Extension::kF16);

    auto* cast = Call<vec3<f16>>(Call<vec3<i32>>(1_i, 2_i, 3_i));
    WrapInFunction(cast);

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitExpression(out, cast);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "vec3<f16>(vec3<i32>(1i, 2i, 3i))");
}

}  // namespace
}  // namespace tint::wgsl::writer
