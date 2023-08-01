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

#include "src/tint/lang/glsl/writer/ast_printer/helper_test.h"
#include "src/tint/utils/text/string_stream.h"

#include "gmock/gmock.h"

namespace tint::glsl::writer {
namespace {

using GlslASTPrinterTest_Identifier = TestHelper;

TEST_F(GlslASTPrinterTest_Identifier, EmitIdentifierExpression) {
    GlobalVar("foo", ty.i32(), builtin::AddressSpace::kPrivate);

    auto* i = Expr("foo");
    WrapInFunction(i);

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitExpression(out, i);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "foo");
}

}  // namespace
}  // namespace tint::glsl::writer
