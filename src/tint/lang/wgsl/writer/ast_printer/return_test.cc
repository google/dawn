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

#include "gmock/gmock.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::wgsl::writer {
namespace {

using WgslASTPrinterTest = TestHelper;

TEST_F(WgslASTPrinterTest, Emit_Return) {
    auto* r = Return();
    WrapInFunction(r);

    ASTPrinter& gen = Build();

    gen.IncrementIndent();

    gen.EmitStatement(r);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), "  return;\n");
}

TEST_F(WgslASTPrinterTest, Emit_ReturnWithValue) {
    auto* r = Return(123_i);
    Func("f", tint::Empty, ty.i32(), Vector{r});

    ASTPrinter& gen = Build();

    gen.IncrementIndent();

    gen.EmitStatement(r);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), "  return 123i;\n");
}

}  // namespace
}  // namespace tint::wgsl::writer
