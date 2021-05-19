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
#include "src/writer/hlsl/test_helper.h"

namespace tint {
namespace writer {
namespace hlsl {
namespace {

using HlslGeneratorImplTest_ModuleConstant = TestHelper;

TEST_F(HlslGeneratorImplTest_ModuleConstant, Emit_ModuleConstant) {
  auto* var = Const("pos", ty.array<f32, 3>(), array<f32, 3>(1.f, 2.f, 3.f));
  WrapInFunction(Decl(var));

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitProgramConstVariable(out, var)) << gen.error();
  EXPECT_EQ(result(), "static const float pos[3] = {1.0f, 2.0f, 3.0f};\n");
}

TEST_F(HlslGeneratorImplTest_ModuleConstant, Emit_SpecConstant) {
  auto* var = GlobalConst("pos", ty.f32(), Expr(3.0f),
                          ast::DecorationList{
                              create<ast::OverrideDecoration>(23),
                          });

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitProgramConstVariable(out, var)) << gen.error();
  EXPECT_EQ(result(), R"(#ifndef WGSL_SPEC_CONSTANT_23
#define WGSL_SPEC_CONSTANT_23 3.0f
#endif
static const float pos = WGSL_SPEC_CONSTANT_23;
)");
}

TEST_F(HlslGeneratorImplTest_ModuleConstant, Emit_SpecConstant_NoConstructor) {
  auto* var = GlobalConst("pos", ty.f32(), nullptr,
                          ast::DecorationList{
                              create<ast::OverrideDecoration>(23),
                          });

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitProgramConstVariable(out, var)) << gen.error();
  EXPECT_EQ(result(), R"(#ifndef WGSL_SPEC_CONSTANT_23
#error spec constant required for constant id 23
#endif
static const float pos = WGSL_SPEC_CONSTANT_23;
)");
}

TEST_F(HlslGeneratorImplTest_ModuleConstant, Emit_SpecConstant_NoId) {
  auto* a = GlobalConst("a", ty.f32(), Expr(3.0f),
                        ast::DecorationList{
                            create<ast::OverrideDecoration>(0),
                        });
  auto* b = GlobalConst("b", ty.f32(), Expr(2.0f),
                        ast::DecorationList{
                            create<ast::OverrideDecoration>(),
                        });

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitProgramConstVariable(out, a)) << gen.error();
  ASSERT_TRUE(gen.EmitProgramConstVariable(out, b)) << gen.error();
  EXPECT_EQ(result(), R"(#ifndef WGSL_SPEC_CONSTANT_0
#define WGSL_SPEC_CONSTANT_0 3.0f
#endif
static const float a = WGSL_SPEC_CONSTANT_0;
#ifndef WGSL_SPEC_CONSTANT_1
#define WGSL_SPEC_CONSTANT_1 2.0f
#endif
static const float b = WGSL_SPEC_CONSTANT_1;
)");
}

}  // namespace
}  // namespace hlsl
}  // namespace writer
}  // namespace tint
