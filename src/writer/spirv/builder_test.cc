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

#include "src/writer/spirv/spv_dump.h"
#include "src/writer/spirv/test_helper.h"

namespace tint {
namespace writer {
namespace spirv {
namespace {

using BuilderTest = TestHelper;

TEST_F(BuilderTest, InsertsPreamble) {
  spirv::Builder& b = Build();

  ASSERT_TRUE(b.Build());
  EXPECT_EQ(DumpBuilder(b), R"(OpCapability Shader
OpMemoryModel Logical GLSL450
)");
}

TEST_F(BuilderTest, TracksIdBounds) {
  spirv::Builder& b = Build();

  for (size_t i = 0; i < 5; i++) {
    EXPECT_EQ(b.next_id(), i + 1);
  }

  EXPECT_EQ(6u, b.id_bound());
}

TEST_F(BuilderTest, Capabilities_Dedup) {
  spirv::Builder& b = Build();

  b.push_capability(SpvCapabilityShader);
  b.push_capability(SpvCapabilityShader);
  b.push_capability(SpvCapabilityShader);

  EXPECT_EQ(DumpInstructions(b.capabilities()), "OpCapability Shader\n");
}

}  // namespace
}  // namespace spirv
}  // namespace writer
}  // namespace tint
