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

#include "src/tint/lang/wgsl/writer/ast_printer/helper_test.h"

#include "gmock/gmock.h"

namespace tint::wgsl::writer {
namespace {

using WgslASTPrinterTest = TestHelper;

TEST_F(WgslASTPrinterTest, Emit_DiagnosticDirective) {
    DiagnosticDirective(builtin::DiagnosticSeverity::kError, "chromium", "unreachable_code");

    ASTPrinter& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(diagnostic(error, chromium.unreachable_code);

)");
}

TEST_F(WgslASTPrinterTest, Emit_DiagnosticAttribute) {
    auto* attr =
        DiagnosticAttribute(builtin::DiagnosticSeverity::kError, "chromium", "unreachable_code");
    Func("foo", {}, ty.void_(), {}, Vector{attr});

    ASTPrinter& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(@diagnostic(error, chromium.unreachable_code)
fn foo() {
}
)");
}

}  // namespace
}  // namespace tint::wgsl::writer
