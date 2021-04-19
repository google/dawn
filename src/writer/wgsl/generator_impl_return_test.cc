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

#include "src/writer/wgsl/test_helper.h"

namespace tint {
namespace writer {
namespace wgsl {
namespace {

using WgslGeneratorImplTest = TestHelper;

TEST_F(WgslGeneratorImplTest, Emit_Return) {
  auto* r = Return();
  WrapInFunction(r);

  GeneratorImpl& gen = Build();

  gen.increment_indent();

  ASSERT_TRUE(gen.EmitStatement(r)) << gen.error();
  EXPECT_EQ(gen.result(), "  return;\n");
}

TEST_F(WgslGeneratorImplTest, Emit_ReturnWithValue) {
  auto* r = Return(123);
  Func("f", {}, ty.i32(), {r});

  GeneratorImpl& gen = Build();

  gen.increment_indent();

  ASSERT_TRUE(gen.EmitStatement(r)) << gen.error();
  EXPECT_EQ(gen.result(), "  return 123;\n");
}

}  // namespace
}  // namespace wgsl
}  // namespace writer
}  // namespace tint
