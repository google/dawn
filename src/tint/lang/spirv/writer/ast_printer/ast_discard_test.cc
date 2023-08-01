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

#include "src/tint/lang/spirv/writer/ast_printer/helper_test.h"
#include "src/tint/lang/spirv/writer/common/spv_dump.h"

namespace tint::spirv::writer {
namespace {

using SpirvASTPrinterTest = TestHelper;

TEST_F(SpirvASTPrinterTest, Discard) {
    auto* stmt = Discard();

    Func("F", tint::Empty, ty.void_(), Vector{stmt}, Vector{Stage(ast::PipelineStage::kFragment)});

    Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_TRUE(b.GenerateStatement(stmt)) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()), R"(OpKill
)");
}

}  // namespace
}  // namespace tint::spirv::writer
