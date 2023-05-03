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

#include "src/tint/writer/spirv/spv_dump.h"
#include "src/tint/writer/spirv/test_helper.h"

namespace tint::writer::spirv {
namespace {

using SpvModuleTest = TestHelper;

TEST_F(SpvModuleTest, TracksIdBounds) {
    spirv::Module m;

    for (size_t i = 0; i < 5; i++) {
        EXPECT_EQ(m.NextId(), i + 1);
    }

    EXPECT_EQ(6u, m.IdBound());
}

TEST_F(SpvModuleTest, Capabilities_Dedup) {
    spirv::Module m;

    m.PushCapability(SpvCapabilityShader);
    m.PushCapability(SpvCapabilityShader);
    m.PushCapability(SpvCapabilityShader);

    EXPECT_EQ(DumpInstructions(m.Capabilities()), "OpCapability Shader\n");
}

TEST_F(SpvModuleTest, DeclareExtension) {
    spirv::Module m;

    m.PushExtension("SPV_KHR_integer_dot_product");

    EXPECT_EQ(DumpInstructions(m.Extensions()), "OpExtension \"SPV_KHR_integer_dot_product\"\n");
}

}  // namespace
}  // namespace tint::writer::spirv
