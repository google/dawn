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

#include "gtest/gtest.h"
#include "src/ast/discard_statement.h"
#include "src/context.h"
#include "src/type_determiner.h"
#include "src/writer/spirv/builder.h"
#include "src/writer/spirv/spv_dump.h"

namespace tint {
namespace writer {
namespace spirv {
namespace {

using BuilderTest = testing::Test;

TEST_F(BuilderTest, Discard) {
  ast::DiscardStatement expr;

  ast::Module mod;
  Builder b(&mod);
  b.push_function(Function{});
  EXPECT_EQ(b.GenerateStatement(&expr), 1u) << b.error();
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()), R"(OpKill
)");
}

}  // namespace
}  // namespace spirv
}  // namespace writer
}  // namespace tint
