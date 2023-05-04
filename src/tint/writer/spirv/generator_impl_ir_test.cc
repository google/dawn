// Copyright 2023 The Tint Authors.
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

#include "gtest/gtest.h"

#include "src/tint/ir/module.h"
#include "src/tint/writer/spirv/generator_impl_ir.h"
#include "src/tint/writer/spirv/spv_dump.h"

namespace tint::writer::spirv {
namespace {

using SpvGeneratorImplTest = testing::Test;

TEST_F(SpvGeneratorImplTest, ModuleHeader) {
    ir::Module module;
    GeneratorImplIr generator(&module, false);
    ASSERT_TRUE(generator.Generate()) << generator.Diagnostics().str();
    auto got = Disassemble(generator.Result());
    EXPECT_EQ(got, R"(OpCapability Shader
OpMemoryModel Logical GLSL450
)");
}

}  // namespace
}  // namespace tint::writer::spirv
