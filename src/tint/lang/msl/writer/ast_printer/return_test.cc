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

using namespace tint::number_suffixes;  // NOLINT

namespace tint::msl::writer {
namespace {

using MslASTPrinterTest = TestHelper;

TEST_F(MslASTPrinterTest, Emit_Return) {
    auto* r = Return();
    WrapInFunction(r);

    ASTPrinter& gen = Build();

    gen.IncrementIndent();

    ASSERT_TRUE(gen.EmitStatement(r)) << gen.Diagnostics();
    EXPECT_EQ(gen.Result(), "  return;\n");
}

TEST_F(MslASTPrinterTest, Emit_ReturnWithValue) {
    auto* r = Return(123_i);
    Func("f", tint::Empty, ty.i32(), Vector{r});

    ASTPrinter& gen = Build();

    gen.IncrementIndent();

    ASSERT_TRUE(gen.EmitStatement(r)) << gen.Diagnostics();
    EXPECT_EQ(gen.Result(), "  return 123;\n");
}

}  // namespace
}  // namespace tint::msl::writer
