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

using HlslGeneratorImplTest_Return = TestHelper;

TEST_F(HlslGeneratorImplTest_Return, Emit_Return) {
  auto* r = Return();
  WrapInFunction(r);

  GeneratorImpl& gen = Build();

  gen.increment_indent();

  ASSERT_TRUE(gen.EmitStatement(out, r)) << gen.error();
  EXPECT_EQ(result(), "  return;\n");
}

TEST_F(HlslGeneratorImplTest_Return, Emit_ReturnWithValue) {
  auto* r = Return(123);
  Func("f", {}, ty.i32(), {r});

  GeneratorImpl& gen = Build();

  gen.increment_indent();

  ASSERT_TRUE(gen.EmitStatement(out, r)) << gen.error();
  EXPECT_EQ(result(), "  return 123;\n");
}

}  // namespace
}  // namespace hlsl
}  // namespace writer
}  // namespace tint
