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
#include "src/ast/cast_expression.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/module.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/vector_type.h"
#include "src/writer/msl/generator_impl.h"

namespace tint {
namespace writer {
namespace msl {
namespace {

using MslGeneratorImplTest = testing::Test;

TEST_F(MslGeneratorImplTest, EmitExpression_Cast_Scalar) {
  ast::type::F32Type f32;
  auto id = std::make_unique<ast::IdentifierExpression>("id");
  ast::CastExpression cast(&f32, std::move(id));

  ast::Module m;
  GeneratorImpl g(&m);
  ASSERT_TRUE(g.EmitExpression(&cast)) << g.error();
  EXPECT_EQ(g.result(), "float(id)");
}

TEST_F(MslGeneratorImplTest, EmitExpression_Cast_Vector) {
  ast::type::F32Type f32;
  ast::type::VectorType vec3(&f32, 3);

  auto id = std::make_unique<ast::IdentifierExpression>("id");
  ast::CastExpression cast(&vec3, std::move(id));

  ast::Module m;
  GeneratorImpl g(&m);
  ASSERT_TRUE(g.EmitExpression(&cast)) << g.error();
  EXPECT_EQ(g.result(), "float3(id)");
}

}  // namespace
}  // namespace msl
}  // namespace writer
}  // namespace tint
