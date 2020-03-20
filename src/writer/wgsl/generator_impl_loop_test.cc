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

#include <memory>
#include <vector>

#include "gtest/gtest.h"
#include "src/ast/kill_statement.h"
#include "src/ast/loop_statement.h"
#include "src/ast/nop_statement.h"
#include "src/writer/wgsl/generator_impl.h"

namespace tint {
namespace writer {
namespace wgsl {
namespace {

using GeneratorImplTest = testing::Test;

TEST_F(GeneratorImplTest, Emit_Loop) {
  std::vector<std::unique_ptr<ast::Statement>> body;
  body.push_back(std::make_unique<ast::KillStatement>());

  ast::LoopStatement l(std::move(body), {});

  GeneratorImpl g;
  g.increment_indent();

  ASSERT_TRUE(g.EmitStatement(&l)) << g.error();
  EXPECT_EQ(g.result(), R"(  loop {
    kill;
  }
)");
}

TEST_F(GeneratorImplTest, Emit_LoopWithContinuing) {
  std::vector<std::unique_ptr<ast::Statement>> body;
  body.push_back(std::make_unique<ast::KillStatement>());

  std::vector<std::unique_ptr<ast::Statement>> continuing;
  continuing.push_back(std::make_unique<ast::NopStatement>());

  ast::LoopStatement l(std::move(body), std::move(continuing));

  GeneratorImpl g;
  g.increment_indent();

  ASSERT_TRUE(g.EmitStatement(&l)) << g.error();
  EXPECT_EQ(g.result(), R"(  loop {
    kill;

    continuing {
      nop;
    }
  }
)");
}

}  // namespace
}  // namespace wgsl
}  // namespace writer
}  // namespace tint
