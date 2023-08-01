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

using namespace tint::number_suffixes;  // NOLINT

namespace tint::glsl::writer {
namespace {

using GlslASTPrinterTest_Expression = TestHelper;

TEST_F(GlslASTPrinterTest_Expression, IndexAccessor) {
    GlobalVar("ary", ty.array<i32, 10>(), builtin::AddressSpace::kPrivate);
    auto* expr = IndexAccessor("ary", 5_i);
    WrapInFunction(expr);

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitExpression(out, expr);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "ary[5]");
}

}  // namespace
}  // namespace tint::glsl::writer
