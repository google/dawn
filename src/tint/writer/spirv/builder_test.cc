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

#include "src/tint/writer/spirv/test_helper.h"

namespace tint::writer::spirv {
namespace {

using BuilderTest = TestHelper;

TEST_F(BuilderTest, InvalidProgram) {
    Diagnostics().add_error(diag::System::Writer, "make the program invalid");
    ASSERT_FALSE(IsValid());
    auto program = std::make_unique<Program>(std::move(*this));
    ASSERT_FALSE(program->IsValid());
    auto result = Generate(program.get(), Options{});
    EXPECT_EQ(result.error, "input program is not valid");
}

TEST_F(BuilderTest, UnsupportedExtension) {
    Enable(Source{{12, 34}}, builtin::Extension::kUndefined);

    auto program = std::make_unique<Program>(std::move(*this));
    auto result = Generate(program.get(), Options{});
    EXPECT_EQ(result.error,
              R"(12:34 error: SPIR-V backend does not support extension 'undefined')");
}

}  // namespace
}  // namespace tint::writer::spirv
