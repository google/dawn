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

#include "src/tint/lang/spirv/writer/ast_printer/helper_test.h"
#include "src/tint/lang/spirv/writer/common/spv_dump.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::spirv::writer {
namespace {

using SpirvASTPrinterTest = TestHelper;

TEST_F(SpirvASTPrinterTest, GlobalConstAssert) {
    GlobalConstAssert(true);

    Builder& b = Build();

    ASSERT_TRUE(b.Build()) << b.Diagnostics();

    // const asserts are not emitted
    EXPECT_EQ(DumpInstructions(b.Module().Types()), "");
    EXPECT_EQ(b.Module().Functions().size(), 0u);
}

TEST_F(SpirvASTPrinterTest, FunctionConstAssert) {
    Func("f", tint::Empty, ty.void_(), Vector{ConstAssert(true)});

    Builder& b = Build();

    ASSERT_TRUE(b.Build()) << b.Diagnostics();

    // const asserts are not emitted
    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeVoid
%1 = OpTypeFunction %2
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].instructions()), R"(OpReturn
)");
}

}  // namespace
}  // namespace tint::spirv::writer
