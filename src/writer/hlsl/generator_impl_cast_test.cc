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

#include "src/ast/cast_expression.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/module.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/vector_type.h"
#include "src/writer/hlsl/test_helper.h"

namespace tint {
namespace writer {
namespace hlsl {
namespace {

using HlslGeneratorImplTest_Cast = TestHelper;

TEST_F(HlslGeneratorImplTest_Cast, EmitExpression_Cast_Scalar) {
  ast::type::F32Type f32;
  auto id = std::make_unique<ast::IdentifierExpression>("id");
  ast::CastExpression cast(&f32, std::move(id));

  ASSERT_TRUE(gen().EmitExpression(out(), &cast)) << gen().error();
  EXPECT_EQ(result(), "float(id)");
}

TEST_F(HlslGeneratorImplTest_Cast, EmitExpression_Cast_Vector) {
  ast::type::F32Type f32;
  ast::type::VectorType vec3(&f32, 3);

  auto id = std::make_unique<ast::IdentifierExpression>("id");
  ast::CastExpression cast(&vec3, std::move(id));

  ASSERT_TRUE(gen().EmitExpression(out(), &cast)) << gen().error();
  EXPECT_EQ(result(), "vector<float, 3>(id)");
}

}  // namespace
}  // namespace hlsl
}  // namespace writer
}  // namespace tint
