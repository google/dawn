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

#include "src/ast/continue_statement.h"
#include "src/ast/module.h"
#include "src/writer/hlsl/test_helper.h"

namespace tint {
namespace writer {
namespace hlsl {
namespace {

using HlslGeneratorImplTest_Continue = TestHelper;

TEST_F(HlslGeneratorImplTest_Continue, Emit_Continue) {
  ast::ContinueStatement c;

  gen().increment_indent();

  ASSERT_TRUE(gen().EmitStatement(out(), &c)) << gen().error();
  EXPECT_EQ(result(), "  continue;\n");
}

}  // namespace
}  // namespace hlsl
}  // namespace writer
}  // namespace tint
