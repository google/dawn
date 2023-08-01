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

#include "src/tint/lang/msl/writer/ast_printer/helper_test.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::msl::writer {
namespace {

using MslASTPrinterTest = TestHelper;

TEST_F(MslASTPrinterTest, Emit_GlobalConstAssert) {
    GlobalConstAssert(true);

    ASTPrinter& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    // const asserts are not emitted
    EXPECT_EQ(gen.Result(), R"(#include <metal_stdlib>

using namespace metal;
)");
}

TEST_F(MslASTPrinterTest, Emit_FunctionConstAssert) {
    Func("f", tint::Empty, ty.void_(), Vector{ConstAssert(true)});

    ASTPrinter& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    // const asserts are not emitted
    EXPECT_EQ(gen.Result(), R"(#include <metal_stdlib>

using namespace metal;
void f() {
}

)");
}

}  // namespace
}  // namespace tint::msl::writer
