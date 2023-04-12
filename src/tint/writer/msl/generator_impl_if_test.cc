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

#include "src/tint/writer/msl/test_helper.h"

namespace tint::writer::msl {
namespace {

using MslGeneratorImplTest = TestHelper;

TEST_F(MslGeneratorImplTest, Emit_If) {
    auto* cond = Var("cond", ty.bool_());
    auto* i = If(cond, Block(Return()));
    WrapInFunction(cond, i);

    GeneratorImpl& gen = Build();

    gen.increment_indent();

    ASSERT_TRUE(gen.EmitStatement(i)) << gen.Diagnostics();
    EXPECT_EQ(gen.result(), R"(  if (cond) {
    return;
  }
)");
}

TEST_F(MslGeneratorImplTest, Emit_IfWithElseIf) {
    auto* cond = Var("cond", ty.bool_());
    auto* else_cond = Var("else_cond", ty.bool_());
    auto* i = If(cond, Block(Return()), Else(If(else_cond, Block(Return()))));
    WrapInFunction(cond, else_cond, i);

    GeneratorImpl& gen = Build();

    gen.increment_indent();

    ASSERT_TRUE(gen.EmitStatement(i)) << gen.Diagnostics();
    EXPECT_EQ(gen.result(), R"(  if (cond) {
    return;
  } else {
    if (else_cond) {
      return;
    }
  }
)");
}

TEST_F(MslGeneratorImplTest, Emit_IfWithElse) {
    auto* cond = Var("cond", ty.bool_());
    auto* i = If(cond, Block(Return()), Else(Block(Return())));
    WrapInFunction(cond, i);

    GeneratorImpl& gen = Build();

    gen.increment_indent();

    ASSERT_TRUE(gen.EmitStatement(i)) << gen.Diagnostics();
    EXPECT_EQ(gen.result(), R"(  if (cond) {
    return;
  } else {
    return;
  }
)");
}

TEST_F(MslGeneratorImplTest, Emit_IfWithMultiple) {
    auto* cond = Var("cond", ty.bool_());
    auto* else_cond = Var("else_cond", ty.bool_());
    auto* i =
        If(cond, Block(Return()), Else(If(else_cond, Block(Return()), Else(Block(Return())))));
    WrapInFunction(cond, else_cond, i);

    GeneratorImpl& gen = Build();

    gen.increment_indent();

    ASSERT_TRUE(gen.EmitStatement(i)) << gen.Diagnostics();
    EXPECT_EQ(gen.result(), R"(  if (cond) {
    return;
  } else {
    if (else_cond) {
      return;
    } else {
      return;
    }
  }
)");
}

}  // namespace
}  // namespace tint::writer::msl
