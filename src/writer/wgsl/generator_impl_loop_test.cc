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

#include "gtest/gtest.h"
#include "src/ast/discard_statement.h"
#include "src/ast/loop_statement.h"
#include "src/writer/wgsl/generator_impl.h"
#include "src/writer/wgsl/test_helper.h"

namespace tint {
namespace writer {
namespace wgsl {
namespace {

using WgslGeneratorImplTest = TestHelper;

TEST_F(WgslGeneratorImplTest, Emit_Loop) {
  auto body = create<ast::BlockStatement>();
  body->append(create<ast::DiscardStatement>());
  ast::LoopStatement l(std::move(body), {});

  gen.increment_indent();

  ASSERT_TRUE(gen.EmitStatement(&l)) << gen.error();
  EXPECT_EQ(gen.result(), R"(  loop {
    discard;
  }
)");
}

TEST_F(WgslGeneratorImplTest, Emit_LoopWithContinuing) {
  auto body = create<ast::BlockStatement>();
  body->append(create<ast::DiscardStatement>());

  auto continuing = create<ast::BlockStatement>();
  continuing->append(create<ast::DiscardStatement>());

  ast::LoopStatement l(std::move(body), std::move(continuing));

  gen.increment_indent();

  ASSERT_TRUE(gen.EmitStatement(&l)) << gen.error();
  EXPECT_EQ(gen.result(), R"(  loop {
    discard;

    continuing {
      discard;
    }
  }
)");
}

}  // namespace
}  // namespace wgsl
}  // namespace writer
}  // namespace tint
