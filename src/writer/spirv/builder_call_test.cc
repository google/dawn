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
#include "src/ast/call_expression.h"
#include "src/ast/float_literal.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/scalar_constructor_expression.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/void_type.h"
#include "src/context.h"
#include "src/type_determiner.h"
#include "src/writer/spirv/builder.h"
#include "src/writer/spirv/spv_dump.h"

namespace tint {
namespace writer {
namespace spirv {
namespace {

using BuilderTest = testing::Test;

TEST_F(BuilderTest, Call_GLSLMethod) {
  ast::type::F32Type f32;
  ast::type::VoidType void_type;

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));

  ast::CallExpression expr(std::make_unique<ast::IdentifierExpression>(
                               std::vector<std::string>{"std", "round"}),
                           std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);

  auto imp = std::make_unique<ast::Import>("GLSL.std.450", "std");
  auto* glsl = imp.get();
  mod.AddImport(std::move(imp));

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  ast::Function func("a_func", {}, &void_type);

  Builder b(&mod);
  b.GenerateImport(glsl);
  ASSERT_TRUE(b.GenerateFunction(&func)) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(&expr), 7u) << b.error();
  EXPECT_EQ(DumpBuilder(b), R"(%1 = OpExtInstImport "GLSL.std.450"
OpName %4 "a_func"
%3 = OpTypeVoid
%2 = OpTypeFunction %3
%6 = OpTypeFloat 32
%8 = OpConstant %6 1
%4 = OpFunction %3 None %2
%5 = OpLabel
%7 = OpExtInst %6 %1 Round %8
OpFunctionEnd
)");
}

}  // namespace
}  // namespace spirv
}  // namespace writer
}  // namespace tint
