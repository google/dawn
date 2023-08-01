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

using WgslASTPrinterTest = TestHelper;

TEST_F(WgslASTPrinterTest, EmitExpression_MemberAccessor) {
    auto* s = Structure("Data", Vector{Member("mem", ty.f32())});
    GlobalVar("str", ty.Of(s), builtin::AddressSpace::kPrivate);

    auto* expr = MemberAccessor("str", "mem");
    WrapInFunction(expr);

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitExpression(out, expr);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "str.mem");
}

TEST_F(WgslASTPrinterTest, EmitExpression_MemberAccessor_OfDref) {
    auto* s = Structure("Data", Vector{Member("mem", ty.f32())});
    GlobalVar("str", ty.Of(s), builtin::AddressSpace::kPrivate);

    auto* p = Let("p", AddressOf("str"));
    auto* expr = MemberAccessor(Deref("p"), "mem");
    WrapInFunction(p, expr);

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitExpression(out, expr);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "(*(p)).mem");
}

}  // namespace
}  // namespace tint::wgsl::writer
