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
#include "src/writer/wgsl/test_helper.h"

namespace tint {
namespace writer {
namespace wgsl {
namespace {

using WgslGeneratorImplTest = TestHelper;

TEST_F(WgslGeneratorImplTest, EmitVariable) {
  auto* v = Global("a", ty.f32(), ast::StorageClass::kPrivate);

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitVariable(v)) << gen.error();
  EXPECT_EQ(gen.result(), R"(var<private> a : f32;
)");
}

TEST_F(WgslGeneratorImplTest, EmitVariable_StorageClass) {
  auto* v = Global("a", ty.f32(), ast::StorageClass::kPrivate);

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitVariable(v)) << gen.error();
  EXPECT_EQ(gen.result(), R"(var<private> a : f32;
)");
}

TEST_F(WgslGeneratorImplTest, EmitVariable_Decorated) {
  auto* v = Global("a", ty.f32(), ast::StorageClass::kPrivate, nullptr,
                   ast::DecorationList{
                       Location(2),
                   });

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitVariable(v)) << gen.error();
  EXPECT_EQ(gen.result(), R"([[location(2)]] var<private> a : f32;
)");
}

TEST_F(WgslGeneratorImplTest, EmitVariable_Decorated_Multiple) {
  auto* v = Global("a", ty.f32(), ast::StorageClass::kPrivate, nullptr,
                   ast::DecorationList{
                       Builtin(ast::Builtin::kPosition),
                       Location(2),
                   });

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitVariable(v)) << gen.error();
  EXPECT_EQ(
      gen.result(),
      R"([[builtin(position), location(2)]] var<private> a : f32;
)");
}

TEST_F(WgslGeneratorImplTest, EmitVariable_Constructor) {
  auto* v = Global("a", ty.f32(), ast::StorageClass::kPrivate, Expr(1.0f));

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitVariable(v)) << gen.error();
  EXPECT_EQ(gen.result(), R"(var<private> a : f32 = 1.0;
)");
}

TEST_F(WgslGeneratorImplTest, EmitVariable_Const) {
  auto* v = Const("a", ty.f32(), Expr(1.0f));
  WrapInFunction(Decl(v));

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitVariable(v)) << gen.error();
  EXPECT_EQ(gen.result(), R"(let a : f32 = 1.0;
)");
}

}  // namespace
}  // namespace wgsl
}  // namespace writer
}  // namespace tint
