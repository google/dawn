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

#include "src/writer/spirv/builder.h"

#include <memory>

#include "gtest/gtest.h"
#include "spirv/unified1/spirv.h"
#include "spirv/unified1/spirv.hpp11"
#include "src/ast/import.h"
#include "src/ast/module.h"

namespace tint {
namespace writer {
namespace spirv {

using BuilderTest = testing::Test;

TEST_F(BuilderTest, InsertsPreambleWithImport) {
  ast::Module m;
  m.AddImport(std::make_unique<ast::Import>("GLSL.std.450", "glsl"));

  Builder b;
  ASSERT_TRUE(b.Build(m));
  ASSERT_EQ(b.preamble().size(), 4);

  auto pre = b.preamble();
  EXPECT_EQ(pre[0].opcode(), spv::Op::OpCapability);
  EXPECT_EQ(pre[0].operands()[0].to_i(), SpvCapabilityShader);
  EXPECT_EQ(pre[1].opcode(), spv::Op::OpCapability);
  EXPECT_EQ(pre[1].operands()[0].to_i(), SpvCapabilityVulkanMemoryModel);
  EXPECT_EQ(pre[2].opcode(), spv::Op::OpExtInstImport);
  EXPECT_EQ(pre[2].operands()[1].to_s(), "GLSL.std.450");
  EXPECT_EQ(pre[3].opcode(), spv::Op::OpMemoryModel);
}

TEST_F(BuilderTest, InsertsPreambleWithoutImport) {
  ast::Module m;
  Builder b;
  ASSERT_TRUE(b.Build(m));
  ASSERT_EQ(b.preamble().size(), 3);

  auto pre = b.preamble();
  EXPECT_EQ(pre[0].opcode(), spv::Op::OpCapability);
  EXPECT_EQ(pre[0].operands()[0].to_i(), SpvCapabilityShader);
  EXPECT_EQ(pre[1].opcode(), spv::Op::OpCapability);
  EXPECT_EQ(pre[1].operands()[0].to_i(), SpvCapabilityVulkanMemoryModel);
  EXPECT_EQ(pre[2].opcode(), spv::Op::OpMemoryModel);
}

TEST_F(BuilderTest, TracksIdBounds) {
  Builder b;

  for (size_t i = 0; i < 5; i++) {
    EXPECT_EQ(b.next_id(), i + 1);
  }

  EXPECT_EQ(6, b.id_bound());
}

}  // namespace spirv
}  // namespace writer
}  // namespace tint
