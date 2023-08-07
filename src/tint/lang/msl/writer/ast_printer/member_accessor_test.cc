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

#include "src/tint/lang/msl/writer/ast_printer/helper_test.h"
#include "src/tint/utils/text/string_stream.h"

namespace tint::msl::writer {
namespace {

using MslASTPrinterTest = TestHelper;

TEST_F(MslASTPrinterTest, EmitExpression_MemberAccessor) {
    GlobalVar("str", ty.Of(Structure("my_str", Vector{Member("mem", ty.f32())})),
              core::AddressSpace::kPrivate);
    auto* expr = MemberAccessor("str", "mem");
    WrapInFunction(expr);

    ASTPrinter& gen = Build();

    StringStream out;
    ASSERT_TRUE(gen.EmitExpression(out, expr)) << gen.Diagnostics();
    EXPECT_EQ(out.str(), "str.mem");
}

TEST_F(MslASTPrinterTest, EmitExpression_MemberAccessor_Swizzle_xyz) {
    GlobalVar("my_vec", ty.vec4<f32>(), core::AddressSpace::kPrivate);

    auto* expr = MemberAccessor("my_vec", "xyz");
    WrapInFunction(expr);

    ASTPrinter& gen = Build();
    StringStream out;
    ASSERT_TRUE(gen.EmitExpression(out, expr)) << gen.Diagnostics();
    EXPECT_EQ(out.str(), "my_vec.xyz");
}

TEST_F(MslASTPrinterTest, EmitExpression_MemberAccessor_Swizzle_gbr) {
    GlobalVar("my_vec", ty.vec4<f32>(), core::AddressSpace::kPrivate);

    auto* expr = MemberAccessor("my_vec", "gbr");
    WrapInFunction(expr);

    ASTPrinter& gen = Build();
    StringStream out;
    ASSERT_TRUE(gen.EmitExpression(out, expr)) << gen.Diagnostics();
    EXPECT_EQ(out.str(), "my_vec.gbr");
}

}  // namespace
}  // namespace tint::msl::writer
