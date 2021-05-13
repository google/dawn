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

#include "src/ast/override_decoration.h"
#include "src/writer/msl/test_helper.h"

namespace tint {
namespace writer {
namespace msl {
namespace {

using MslGeneratorImplTest = TestHelper;

TEST_F(MslGeneratorImplTest, Emit_ModuleConstant) {
  auto* var =
      GlobalConst("pos", ty.array<f32, 3>(), array<f32, 3>(1.f, 2.f, 3.f));

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitProgramConstVariable(var)) << gen.error();
  EXPECT_EQ(gen.result(), "constant float pos[3] = {1.0f, 2.0f, 3.0f};\n");
}

TEST_F(MslGeneratorImplTest, Emit_SpecConstant) {
  auto* var = GlobalConst("pos", ty.f32(), Expr(3.f),
                          ast::DecorationList{
                              create<ast::OverrideDecoration>(23),
                          });

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitProgramConstVariable(var)) << gen.error();
  EXPECT_EQ(gen.result(), "constant float pos [[function_constant(23)]];\n");
}

TEST_F(MslGeneratorImplTest, Emit_SpecConstant_NoId) {
  auto* var_a = GlobalConst("a", ty.f32(), nullptr,
                            ast::DecorationList{
                                create<ast::OverrideDecoration>(0),
                            });
  auto* var_b = GlobalConst("b", ty.f32(), nullptr,
                            ast::DecorationList{
                                create<ast::OverrideDecoration>(),
                            });

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitProgramConstVariable(var_a)) << gen.error();
  ASSERT_TRUE(gen.EmitProgramConstVariable(var_b)) << gen.error();
  EXPECT_EQ(gen.result(), R"(constant float a [[function_constant(0)]];
constant float b [[function_constant(1)]];
)");
}

}  // namespace
}  // namespace msl
}  // namespace writer
}  // namespace tint
