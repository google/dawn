// Copyright 2021 The Tint Authors.
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

#include "src/tint/writer/glsl/test_helper.h"

#include "gmock/gmock.h"

namespace tint::writer::glsl {
namespace {

using GlslGeneratorImplTest_Continue = TestHelper;

TEST_F(GlslGeneratorImplTest_Continue, Emit_Continue) {
    auto* loop = Loop(Block(If(false, Block(Break())), Continue()));
    WrapInFunction(loop);

    GeneratorImpl& gen = Build();

    gen.increment_indent();

    gen.EmitStatement(loop);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(  while (true) {
    if (false) {
      break;
    }
    continue;
  }
)");
}

}  // namespace
}  // namespace tint::writer::glsl
