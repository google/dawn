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
#include "src/writer/spirv/spv_dump.h"

namespace tint {
namespace writer {
namespace spirv {
namespace {

using BuilderTest = testing::Test;

TEST_F(BuilderTest, InsertsPreambleWithImport) {
  ast::Module m;
  m.AddImport(std::make_unique<ast::Import>("GLSL.std.450", "glsl"));

  Builder b;
  ASSERT_TRUE(b.Build(m));
  ASSERT_EQ(b.preamble().size(), 4u);

  EXPECT_EQ(DumpBuilder(b), R"(OpCapability Shader
OpCapability VulkanMemoryModel
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical Vulkan
)");
}

TEST_F(BuilderTest, InsertsPreambleWithoutImport) {
  ast::Module m;
  Builder b;
  ASSERT_TRUE(b.Build(m));
  EXPECT_EQ(DumpBuilder(b), R"(OpCapability Shader
OpCapability VulkanMemoryModel
OpMemoryModel Logical Vulkan
)");
}

TEST_F(BuilderTest, TracksIdBounds) {
  Builder b;

  for (size_t i = 0; i < 5; i++) {
    EXPECT_EQ(b.next_id(), i + 1);
  }

  EXPECT_EQ(6u, b.id_bound());
}

}  // namespace
}  // namespace spirv
}  // namespace writer
}  // namespace tint
