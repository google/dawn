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
#include "src/ast/binding_decoration.h"
#include "src/ast/builtin_decoration.h"
#include "src/ast/decorated_variable.h"
#include "src/ast/location_decoration.h"
#include "src/ast/set_decoration.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/variable_decoration.h"
#include "src/writer/wgsl/generator_impl.h"

namespace tint {
namespace writer {
namespace wgsl {
namespace {

using WgslGeneratorImplTest = testing::Test;

TEST_F(WgslGeneratorImplTest, EmitVariable) {
  ast::type::F32Type f32;
  ast::Variable v("a", ast::StorageClass::kNone, &f32);

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitVariable(&v)) << g.error();
  EXPECT_EQ(g.result(), R"(var a : f32;
)");
}

TEST_F(WgslGeneratorImplTest, EmitVariable_StorageClass) {
  ast::type::F32Type f32;
  ast::Variable v("a", ast::StorageClass::kInput, &f32);

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitVariable(&v)) << g.error();
  EXPECT_EQ(g.result(), R"(var<in> a : f32;
)");
}

TEST_F(WgslGeneratorImplTest, EmitVariable_Decorated) {
  ast::type::F32Type f32;

  ast::VariableDecorationList decos;
  decos.push_back(std::make_unique<ast::LocationDecoration>(2));

  ast::DecoratedVariable dv;
  dv.set_name("a");
  dv.set_type(&f32);
  dv.set_decorations(std::move(decos));

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitVariable(&dv)) << g.error();
  EXPECT_EQ(g.result(), R"([[location 2]] var a : f32;
)");
}

TEST_F(WgslGeneratorImplTest, EmitVariable_Decorated_Multiple) {
  ast::type::F32Type f32;

  ast::VariableDecorationList decos;
  decos.push_back(
      std::make_unique<ast::BuiltinDecoration>(ast::Builtin::kPosition));
  decos.push_back(std::make_unique<ast::BindingDecoration>(0));
  decos.push_back(std::make_unique<ast::SetDecoration>(1));
  decos.push_back(std::make_unique<ast::LocationDecoration>(2));

  ast::DecoratedVariable dv;
  dv.set_name("a");
  dv.set_type(&f32);
  dv.set_decorations(std::move(decos));

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitVariable(&dv)) << g.error();
  EXPECT_EQ(g.result(),
            R"([[builtin position, binding 0, set 1, location 2]] var a : f32;
)");
}

TEST_F(WgslGeneratorImplTest, EmitVariable_Constructor) {
  auto ident = std::make_unique<ast::IdentifierExpression>("initializer");

  ast::type::F32Type f32;
  ast::Variable v("a", ast::StorageClass::kNone, &f32);
  v.set_constructor(std::move(ident));

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitVariable(&v)) << g.error();
  EXPECT_EQ(g.result(), R"(var a : f32 = initializer;
)");
}

TEST_F(WgslGeneratorImplTest, EmitVariable_Const) {
  auto ident = std::make_unique<ast::IdentifierExpression>("initializer");

  ast::type::F32Type f32;
  ast::Variable v("a", ast::StorageClass::kNone, &f32);
  v.set_constructor(std::move(ident));
  v.set_is_const(true);

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitVariable(&v)) << g.error();
  EXPECT_EQ(g.result(), R"(const a : f32 = initializer;
)");
}

}  // namespace
}  // namespace wgsl
}  // namespace writer
}  // namespace tint
