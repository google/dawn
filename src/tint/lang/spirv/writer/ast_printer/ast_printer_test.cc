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
#include "src/tint/lang/spirv/writer/writer.h"

namespace tint::spirv::writer {
namespace {

using SpirvASTPrinterTest = TestHelper;

TEST_F(SpirvASTPrinterTest, InvalidProgram) {
    Diagnostics().add_error(diag::System::Writer, "make the program invalid");
    ASSERT_FALSE(IsValid());
    auto program = std::make_unique<Program>(resolver::Resolve(*this));
    ASSERT_FALSE(program->IsValid());
    auto result = Generate(program.get(), Options{});
    EXPECT_FALSE(result);
    EXPECT_EQ(result.Failure(), "input program is not valid");
}

TEST_F(SpirvASTPrinterTest, UnsupportedExtension) {
    Enable(Source{{12, 34}}, builtin::Extension::kUndefined);

    auto program = std::make_unique<Program>(resolver::Resolve(*this));
    auto result = Generate(program.get(), Options{});
    EXPECT_FALSE(result);
    EXPECT_EQ(result.Failure(),
              R"(12:34 error: SPIR-V backend does not support extension 'undefined')");
}

}  // namespace
}  // namespace tint::spirv::writer
