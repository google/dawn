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

#include "src/writer/spirv/spv_dump.h"
#include "src/writer/spirv/test_helper.h"

namespace tint {
namespace writer {
namespace spirv {
namespace {

using BuilderTest = TestHelper;

TEST_F(BuilderTest, FunctionVar_NoStorageClass) {
  auto* v = Var("var", ty.f32(), ast::StorageClass::kFunction);
  WrapInFunction(v);

  spirv::Builder& b = Build();

  b.push_function(Function{});
  EXPECT_TRUE(b.GenerateFunctionVariable(v)) << b.error();
  EXPECT_EQ(DumpInstructions(b.debug()), R"(OpName %1 "var"
)");
  EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeFloat 32
%2 = OpTypePointer Function %3
%4 = OpConstantNull %3
)");

  const auto& func = b.functions()[0];
  EXPECT_EQ(DumpInstructions(func.variables()),
            R"(%1 = OpVariable %2 Function %4
)");
}

TEST_F(BuilderTest, FunctionVar_WithConstantConstructor) {
  auto* init = vec3<f32>(1.f, 1.f, 3.f);
  auto* v = Var("var", ty.vec3<f32>(), ast::StorageClass::kFunction, init);
  WrapInFunction(v);

  spirv::Builder& b = Build();

  b.push_function(Function{});
  EXPECT_TRUE(b.GenerateFunctionVariable(v)) << b.error();
  ASSERT_FALSE(b.has_error()) << b.error();

  EXPECT_EQ(DumpInstructions(b.debug()), R"(OpName %6 "var"
)");
  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 3
%3 = OpConstant %2 1
%4 = OpConstant %2 3
%5 = OpConstantComposite %1 %3 %3 %4
%7 = OpTypePointer Function %1
%8 = OpConstantNull %1
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].variables()),
            R"(%6 = OpVariable %7 Function %8
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(OpStore %6 %5
)");
}

TEST_F(BuilderTest, FunctionVar_WithNonConstantConstructor) {
  auto* init = vec2<f32>(1.f, Add(3.f, 3.f));

  auto* v = Var("var", ty.vec2<f32>(), ast::StorageClass::kNone, init);
  WrapInFunction(v);

  spirv::Builder& b = Build();

  b.push_function(Function{});
  EXPECT_TRUE(b.GenerateFunctionVariable(v)) << b.error();
  ASSERT_FALSE(b.has_error()) << b.error();

  EXPECT_EQ(DumpInstructions(b.debug()), R"(OpName %7 "var"
)");
  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 2
%3 = OpConstant %2 1
%4 = OpConstant %2 3
%8 = OpTypePointer Function %1
%9 = OpConstantNull %1
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].variables()),
            R"(%7 = OpVariable %8 Function %9
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%5 = OpFAdd %2 %4 %4
%6 = OpCompositeConstruct %1 %3 %5
OpStore %7 %6
)");
}

TEST_F(BuilderTest, FunctionVar_WithNonConstantConstructorLoadedFromVar) {
  // var v : f32 = 1.0;
  // var v2 : f32 = v; // Should generate the load and store automatically.

  auto* v = Var("v", ty.f32(), ast::StorageClass::kNone, Expr(1.f));

  auto* v2 = Var("v2", ty.f32(), ast::StorageClass::kNone, Expr("v"));
  WrapInFunction(v, v2);

  spirv::Builder& b = Build();

  b.push_function(Function{});
  EXPECT_TRUE(b.GenerateFunctionVariable(v)) << b.error();
  EXPECT_TRUE(b.GenerateFunctionVariable(v2)) << b.error();
  ASSERT_FALSE(b.has_error()) << b.error();

  EXPECT_EQ(DumpInstructions(b.debug()), R"(OpName %3 "v"
OpName %7 "v2"
)");
  EXPECT_EQ(DumpInstructions(b.types()), R"(%1 = OpTypeFloat 32
%2 = OpConstant %1 1
%4 = OpTypePointer Function %1
%5 = OpConstantNull %1
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].variables()),
            R"(%3 = OpVariable %4 Function %5
%7 = OpVariable %4 Function %5
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(OpStore %3 %2
%6 = OpLoad %1 %3
OpStore %7 %6
)");
}

TEST_F(BuilderTest, FunctionVar_ConstWithVarInitializer) {
  // var v : f32 = 1.0;
  // let v2 : f32 = v; // Should generate the load

  auto* v = Var("v", ty.f32(), ast::StorageClass::kNone, Expr(1.f));

  auto* v2 = Var("v2", ty.f32(), ast::StorageClass::kNone, Expr("v"));
  WrapInFunction(v, v2);

  spirv::Builder& b = Build();

  b.push_function(Function{});
  EXPECT_TRUE(b.GenerateFunctionVariable(v)) << b.error();
  EXPECT_TRUE(b.GenerateFunctionVariable(v2)) << b.error();
  ASSERT_FALSE(b.has_error()) << b.error();

  EXPECT_EQ(DumpInstructions(b.debug()), R"(OpName %3 "v"
OpName %7 "v2"
)");
  EXPECT_EQ(DumpInstructions(b.types()), R"(%1 = OpTypeFloat 32
%2 = OpConstant %1 1
%4 = OpTypePointer Function %1
%5 = OpConstantNull %1
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].variables()),
            R"(%3 = OpVariable %4 Function %5
%7 = OpVariable %4 Function %5
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(OpStore %3 %2
%6 = OpLoad %1 %3
OpStore %7 %6
)");
}

TEST_F(BuilderTest, FunctionVar_Const) {
  auto* init = vec3<f32>(1.f, 1.f, 3.f);

  auto* v = Const("var", ty.vec3<f32>(), init);

  WrapInFunction(v);

  spirv::Builder& b = Build();

  EXPECT_TRUE(b.GenerateFunctionVariable(v)) << b.error();
  ASSERT_FALSE(b.has_error()) << b.error();

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 3
%3 = OpConstant %2 1
%4 = OpConstant %2 3
%5 = OpConstantComposite %1 %3 %3 %4
)");
}

}  // namespace
}  // namespace spirv
}  // namespace writer
}  // namespace tint
