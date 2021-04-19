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

#include "src/writer/hlsl/test_helper.h"

namespace tint {
namespace writer {
namespace hlsl {
namespace {

using HlslGeneratorImplTest_Block = TestHelper;

TEST_F(HlslGeneratorImplTest_Block, Emit_Block) {
  auto* b = Block(create<ast::DiscardStatement>());
  WrapInFunction(b);

  GeneratorImpl& gen = Build();

  gen.increment_indent();

  ASSERT_TRUE(gen.EmitStatement(out, b)) << gen.error();
  EXPECT_EQ(result(), R"(  {
    discard;
  }
)");
}

TEST_F(HlslGeneratorImplTest_Block, Emit_Block_WithoutNewline) {
  auto* b = Block(create<ast::DiscardStatement>());
  WrapInFunction(b);

  GeneratorImpl& gen = Build();

  gen.increment_indent();

  ASSERT_TRUE(gen.EmitBlock(out, b)) << gen.error();
  EXPECT_EQ(result(), R"({
    discard;
  })");
}

}  // namespace
}  // namespace hlsl
}  // namespace writer
}  // namespace tint
