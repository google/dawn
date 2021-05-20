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

TEST_F(BuilderTest, ArrayAccessor_VectorRef_Literal) {
  // var ary : vec3<f32>;
  // ary[1]  -> ref<f32>

  auto* var = Var("ary", ty.vec3<f32>());

  auto* ary = Expr("ary");
  auto* idx_expr = Expr(1);

  auto* expr = IndexAccessor(ary, idx_expr);
  WrapInFunction(var, expr);

  spirv::Builder& b = Build();

  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateFunctionVariable(var)) << b.error();

  EXPECT_EQ(b.GenerateAccessorExpression(expr), 9u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeFloat 32
%3 = OpTypeVector %4 3
%2 = OpTypePointer Function %3
%5 = OpConstantNull %3
%6 = OpTypeInt 32 1
%7 = OpConstant %6 1
%8 = OpTypePointer Function %4
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].variables()),
            R"(%1 = OpVariable %2 Function %5
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%9 = OpAccessChain %8 %1 %7
)");
}

TEST_F(BuilderTest, ArrayAccessor_VectorRef_Dynamic) {
  // var ary : vec3<f32>;
  // var idx : i32;
  // ary[idx]  -> ref<f32>

  auto* var = Var("ary", ty.vec3<f32>());
  auto* idx = Var("idx", ty.i32());

  auto* ary = Expr("ary");
  auto* idx_expr = Expr("idx");

  auto* expr = IndexAccessor(ary, idx_expr);
  WrapInFunction(var, idx, expr);

  spirv::Builder& b = Build();

  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateFunctionVariable(var)) << b.error();
  ASSERT_TRUE(b.GenerateFunctionVariable(idx)) << b.error();

  EXPECT_EQ(b.GenerateAccessorExpression(expr), 12u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeFloat 32
%3 = OpTypeVector %4 3
%2 = OpTypePointer Function %3
%5 = OpConstantNull %3
%8 = OpTypeInt 32 1
%7 = OpTypePointer Function %8
%9 = OpConstantNull %8
%11 = OpTypePointer Function %4
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].variables()),
            R"(%1 = OpVariable %2 Function %5
%6 = OpVariable %7 Function %9
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%10 = OpLoad %8 %6
%12 = OpAccessChain %11 %1 %10
)");
}

TEST_F(BuilderTest, ArrayAccessor_VectorRef_Dynamic2) {
  // var ary : vec3<f32>;
  // ary[1 + 2]  -> ref<f32>

  auto* var = Var("ary", ty.vec3<f32>());

  auto* ary = Expr("ary");

  auto* expr = IndexAccessor(ary, Add(1, 2));
  WrapInFunction(var, expr);

  spirv::Builder& b = Build();

  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateFunctionVariable(var)) << b.error();

  EXPECT_EQ(b.GenerateAccessorExpression(expr), 11u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeFloat 32
%3 = OpTypeVector %4 3
%2 = OpTypePointer Function %3
%5 = OpConstantNull %3
%6 = OpTypeInt 32 1
%7 = OpConstant %6 1
%8 = OpConstant %6 2
%10 = OpTypePointer Function %4
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].variables()),
            R"(%1 = OpVariable %2 Function %5
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%9 = OpIAdd %6 %7 %8
%11 = OpAccessChain %10 %1 %9
)");
}

TEST_F(BuilderTest, ArrayAccessor_ArrayRef_MultiLevel) {
  auto* ary4 = ty.array(ty.vec3<f32>(), 4);

  // var ary : array<vec3<f32>, 4>
  // ary[3][2];

  auto* var = Var("ary", ary4);

  auto* expr = IndexAccessor(IndexAccessor("ary", 3), 2);
  WrapInFunction(var, expr);

  spirv::Builder& b = Build();

  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateFunctionVariable(var)) << b.error();

  EXPECT_EQ(b.GenerateAccessorExpression(expr), 13u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%5 = OpTypeFloat 32
%4 = OpTypeVector %5 3
%6 = OpTypeInt 32 0
%7 = OpConstant %6 4
%3 = OpTypeArray %4 %7
%2 = OpTypePointer Function %3
%8 = OpConstantNull %3
%9 = OpTypeInt 32 1
%10 = OpConstant %9 3
%11 = OpConstant %9 2
%12 = OpTypePointer Function %5
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].variables()),
            R"(%1 = OpVariable %2 Function %8
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%13 = OpAccessChain %12 %1 %10 %11
)");
}

TEST_F(BuilderTest, ArrayAccessor_ArrayRef_ArrayWithSwizzle) {
  auto* ary4 = ty.array(ty.vec3<f32>(), 4);

  // var a : array<vec3<f32>, 4>;
  // a[2].xy;

  auto* var = Var("ary", ary4);

  auto* expr = MemberAccessor(IndexAccessor("ary", 2), "xy");
  WrapInFunction(var, expr);

  spirv::Builder& b = Build();

  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateFunctionVariable(var)) << b.error();
  EXPECT_EQ(b.GenerateAccessorExpression(expr), 15u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%5 = OpTypeFloat 32
%4 = OpTypeVector %5 3
%6 = OpTypeInt 32 0
%7 = OpConstant %6 4
%3 = OpTypeArray %4 %7
%2 = OpTypePointer Function %3
%8 = OpConstantNull %3
%9 = OpTypeInt 32 1
%10 = OpConstant %9 2
%11 = OpTypePointer Function %4
%13 = OpTypeVector %5 2
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].variables()),
            R"(%1 = OpVariable %2 Function %8
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%12 = OpAccessChain %11 %1 %10
%14 = OpLoad %4 %12
%15 = OpVectorShuffle %13 %14 %14 0 1
)");
}

TEST_F(BuilderTest, MemberAccessor) {
  // my_struct {
  //   a : f32
  //   b : f32
  // }
  // var ident : my_struct
  // ident.b

  auto* s = Structure("my_struct", {
                                       Member("a", ty.f32()),
                                       Member("b", ty.f32()),
                                   });

  auto* var = Var("ident", s);

  auto* expr = MemberAccessor("ident", "b");
  WrapInFunction(var, expr);

  spirv::Builder& b = Build();

  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateFunctionVariable(var)) << b.error();

  EXPECT_EQ(b.GenerateAccessorExpression(expr), 9u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeFloat 32
%3 = OpTypeStruct %4 %4
%2 = OpTypePointer Function %3
%5 = OpConstantNull %3
%6 = OpTypeInt 32 0
%7 = OpConstant %6 1
%8 = OpTypePointer Function %4
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].variables()),
            R"(%1 = OpVariable %2 Function %5
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%9 = OpAccessChain %8 %1 %7
)");
}

TEST_F(BuilderTest, MemberAccessor_Nested) {
  // inner_struct {
  //   a : f32
  //   b : f32
  // }
  // my_struct {
  //   inner : inner_struct
  // }
  //
  // var ident : my_struct
  // ident.inner.a
  auto* inner_struct = Structure("Inner", {
                                              Member("a", ty.f32()),
                                              Member("b", ty.f32()),
                                          });

  auto* s_type = Structure("my_struct", {Member("inner", inner_struct)});

  auto* var = Var("ident", s_type);
  auto* expr = MemberAccessor(MemberAccessor("ident", "inner"), "b");
  WrapInFunction(var, expr);

  spirv::Builder& b = Build();

  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateFunctionVariable(var)) << b.error();

  EXPECT_EQ(b.GenerateAccessorExpression(expr), 11u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%5 = OpTypeFloat 32
%4 = OpTypeStruct %5 %5
%3 = OpTypeStruct %4
%2 = OpTypePointer Function %3
%6 = OpConstantNull %3
%7 = OpTypeInt 32 0
%8 = OpConstant %7 0
%9 = OpConstant %7 1
%10 = OpTypePointer Function %5
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].variables()),
            R"(%1 = OpVariable %2 Function %6
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%11 = OpAccessChain %10 %1 %8 %9
)");
}

TEST_F(BuilderTest, MemberAccessor_NonPointer) {
  // my_struct {
  //   a : f32
  //   b : f32
  // }
  // let ident : my_struct = my_struct();
  // ident.b

  auto* s = Structure("my_struct", {
                                       Member("a", ty.f32()),
                                       Member("b", ty.f32()),
                                   });

  auto* var = Const("ident", s, Construct(s, 0.f, 0.f));

  auto* expr = MemberAccessor("ident", "b");
  WrapInFunction(var, expr);

  spirv::Builder& b = Build();

  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateFunctionVariable(var)) << b.error();

  EXPECT_EQ(b.GenerateAccessorExpression(expr), 5u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeStruct %2 %2
%3 = OpConstant %2 0
%4 = OpConstantComposite %1 %3 %3
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%5 = OpCompositeExtract %2 %4 1
)");
}

TEST_F(BuilderTest, MemberAccessor_Nested_NonPointer) {
  // inner_struct {
  //   a : f32
  //   b : f32
  // }
  // my_struct {
  //   inner : inner_struct
  // }
  //
  // let ident : my_struct = my_struct();
  // ident.inner.a
  auto* inner_struct = Structure("Inner", {
                                              Member("a", ty.f32()),
                                              Member("b", ty.f32()),
                                          });

  auto* s_type = Structure("my_struct", {Member("inner", inner_struct)});

  auto* var = Const("ident", s_type,
                    Construct(s_type, Construct(inner_struct, 0.f, 0.f)));
  auto* expr = MemberAccessor(MemberAccessor("ident", "inner"), "b");
  WrapInFunction(var, expr);

  spirv::Builder& b = Build();

  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateFunctionVariable(var)) << b.error();

  EXPECT_EQ(b.GenerateAccessorExpression(expr), 8u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeFloat 32
%2 = OpTypeStruct %3 %3
%1 = OpTypeStruct %2
%4 = OpConstant %3 0
%5 = OpConstantComposite %2 %4 %4
%6 = OpConstantComposite %1 %5
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%7 = OpCompositeExtract %2 %6 0
%8 = OpCompositeExtract %3 %7 1
)");
}

TEST_F(BuilderTest, MemberAccessor_Nested_WithAlias) {
  // type Inner = struct {
  //   a : f32
  //   b : f32
  // }
  // my_struct {
  //   inner : Inner
  // }
  //
  // var ident : my_struct
  // ident.inner.a
  auto* inner_struct = Structure("Inner", {
                                              Member("a", ty.f32()),
                                              Member("b", ty.f32()),
                                          });

  auto* alias = ty.alias("Inner", inner_struct);
  auto* s_type = Structure("Outer", {Member("inner", alias)});

  auto* var = Var("ident", s_type);
  auto* expr = MemberAccessor(MemberAccessor("ident", "inner"), "a");
  WrapInFunction(var, expr);

  spirv::Builder& b = Build();

  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateFunctionVariable(var)) << b.error();

  EXPECT_EQ(b.GenerateAccessorExpression(expr), 10u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%5 = OpTypeFloat 32
%4 = OpTypeStruct %5 %5
%3 = OpTypeStruct %4
%2 = OpTypePointer Function %3
%6 = OpConstantNull %3
%7 = OpTypeInt 32 0
%8 = OpConstant %7 0
%9 = OpTypePointer Function %5
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].variables()),
            R"(%1 = OpVariable %2 Function %6
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%10 = OpAccessChain %9 %1 %8 %8
)");
}

TEST_F(BuilderTest, MemberAccessor_Nested_Assignment_LHS) {
  // inner_struct {
  //   a : f32
  // }
  // my_struct {
  //   inner : inner_struct
  // }
  //
  // var ident : my_struct
  // ident.inner.a = 2.0f;
  auto* inner_struct = Structure("Inner", {
                                              Member("a", ty.f32()),
                                              Member("b", ty.f32()),
                                          });

  auto* s_type = Structure("my_struct", {Member("inner", inner_struct)});

  auto* var = Var("ident", s_type);
  auto* expr =
      Assign(MemberAccessor(MemberAccessor("ident", "inner"), "a"), Expr(2.0f));
  WrapInFunction(var, expr);

  spirv::Builder& b = Build();

  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateFunctionVariable(var)) << b.error();

  EXPECT_TRUE(b.GenerateAssignStatement(expr)) << b.error();

  EXPECT_EQ(DumpInstructions(b.types()), R"(%5 = OpTypeFloat 32
%4 = OpTypeStruct %5 %5
%3 = OpTypeStruct %4
%2 = OpTypePointer Function %3
%6 = OpConstantNull %3
%7 = OpTypeInt 32 0
%8 = OpConstant %7 0
%9 = OpTypePointer Function %5
%11 = OpConstant %5 2
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].variables()),
            R"(%1 = OpVariable %2 Function %6
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%10 = OpAccessChain %9 %1 %8 %8
OpStore %10 %11
)");
}

TEST_F(BuilderTest, MemberAccessor_Nested_Assignment_RHS) {
  // inner_struct {
  //   a : f32
  // }
  // my_struct {
  //   inner : inner_struct
  // }
  //
  // var ident : my_struct
  // var store : f32 = ident.inner.a

  auto* inner_struct = Structure("Inner", {
                                              Member("a", ty.f32()),
                                              Member("b", ty.f32()),
                                          });

  auto* s_type = Structure("my_struct", {Member("inner", inner_struct)});

  auto* var = Var("ident", s_type);
  auto* store = Var("store", ty.f32());

  auto* rhs = MemberAccessor(MemberAccessor("ident", "inner"), "a");
  auto* expr = Assign("store", rhs);
  WrapInFunction(var, store, expr);

  spirv::Builder& b = Build();

  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateFunctionVariable(var)) << b.error();
  ASSERT_TRUE(b.GenerateFunctionVariable(store)) << b.error();

  EXPECT_TRUE(b.GenerateAssignStatement(expr)) << b.error();

  EXPECT_EQ(DumpInstructions(b.types()), R"(%5 = OpTypeFloat 32
%4 = OpTypeStruct %5 %5
%3 = OpTypeStruct %4
%2 = OpTypePointer Function %3
%6 = OpConstantNull %3
%8 = OpTypePointer Function %5
%9 = OpConstantNull %5
%10 = OpTypeInt 32 0
%11 = OpConstant %10 0
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].variables()),
            R"(%1 = OpVariable %2 Function %6
%7 = OpVariable %8 Function %9
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%12 = OpAccessChain %8 %1 %11 %11
%13 = OpLoad %5 %12
OpStore %7 %13
)");
}

TEST_F(BuilderTest, MemberAccessor_Swizzle_Single) {
  // ident.y

  auto* var = Var("ident", ty.vec3<f32>());

  auto* expr = MemberAccessor("ident", "y");
  WrapInFunction(var, expr);

  spirv::Builder& b = Build();

  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateFunctionVariable(var)) << b.error();

  EXPECT_EQ(b.GenerateAccessorExpression(expr), 9u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeFloat 32
%3 = OpTypeVector %4 3
%2 = OpTypePointer Function %3
%5 = OpConstantNull %3
%6 = OpTypeInt 32 0
%7 = OpConstant %6 1
%8 = OpTypePointer Function %4
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].variables()),
            R"(%1 = OpVariable %2 Function %5
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%9 = OpAccessChain %8 %1 %7
)");
}

TEST_F(BuilderTest, MemberAccessor_Swizzle_MultipleNames) {
  // ident.yx

  auto* var = Var("ident", ty.vec3<f32>());

  auto* expr = MemberAccessor("ident", "yx");
  WrapInFunction(var, expr);

  spirv::Builder& b = Build();

  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateFunctionVariable(var)) << b.error();

  EXPECT_EQ(b.GenerateAccessorExpression(expr), 8u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeFloat 32
%3 = OpTypeVector %4 3
%2 = OpTypePointer Function %3
%5 = OpConstantNull %3
%6 = OpTypeVector %4 2
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].variables()),
            R"(%1 = OpVariable %2 Function %5
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%7 = OpLoad %3 %1
%8 = OpVectorShuffle %6 %7 %7 1 0
)");
}

TEST_F(BuilderTest, MemberAccessor_Swizzle_of_Swizzle) {
  // ident.yxz.xz

  auto* var = Var("ident", ty.vec3<f32>());

  auto* expr = MemberAccessor(MemberAccessor("ident", "yxz"), "xz");
  WrapInFunction(var, expr);

  spirv::Builder& b = Build();

  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateFunctionVariable(var)) << b.error();

  EXPECT_EQ(b.GenerateAccessorExpression(expr), 9u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeFloat 32
%3 = OpTypeVector %4 3
%2 = OpTypePointer Function %3
%5 = OpConstantNull %3
%8 = OpTypeVector %4 2
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].variables()),
            R"(%1 = OpVariable %2 Function %5
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%6 = OpLoad %3 %1
%7 = OpVectorShuffle %3 %6 %6 1 0 2
%9 = OpVectorShuffle %8 %7 %7 0 2
)");
}

TEST_F(BuilderTest, MemberAccessor_Member_of_Swizzle) {
  // ident.yxz.x

  auto* var = Var("ident", ty.vec3<f32>());

  auto* expr = MemberAccessor(MemberAccessor("ident", "yxz"), "x");
  WrapInFunction(var, expr);

  spirv::Builder& b = Build();

  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateFunctionVariable(var)) << b.error();

  EXPECT_EQ(b.GenerateAccessorExpression(expr), 8u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeFloat 32
%3 = OpTypeVector %4 3
%2 = OpTypePointer Function %3
%5 = OpConstantNull %3
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].variables()),
            R"(%1 = OpVariable %2 Function %5
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%6 = OpLoad %3 %1
%7 = OpVectorShuffle %3 %6 %6 1 0 2
%8 = OpCompositeExtract %4 %7 0
)");
}

TEST_F(BuilderTest, MemberAccessor_Array_of_Swizzle) {
  // index.yxz[1]

  auto* var = Var("ident", ty.vec3<f32>());

  auto* expr = IndexAccessor(MemberAccessor("ident", "yxz"), 1);
  WrapInFunction(var, expr);

  spirv::Builder& b = Build();

  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateFunctionVariable(var)) << b.error();

  EXPECT_EQ(b.GenerateAccessorExpression(expr), 10u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeFloat 32
%3 = OpTypeVector %4 3
%2 = OpTypePointer Function %3
%5 = OpConstantNull %3
%8 = OpTypeInt 32 1
%9 = OpConstant %8 1
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].variables()),
            R"(%1 = OpVariable %2 Function %5
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%6 = OpLoad %3 %1
%7 = OpVectorShuffle %3 %6 %6 1 0 2
%10 = OpCompositeExtract %4 %7 1
)");
}

TEST_F(BuilderTest, ArrayAccessor_Mixed_ArrayAndMember) {
  // type C = struct {
  //   baz : vec3<f32>
  // }
  // type B = struct {
  //  bar : C;
  // }
  // type A = struct {
  //   foo : array<B, 3>
  // }
  // var index : array<A, 2>
  // index[0].foo[2].bar.baz.yx

  auto* c_type = Structure("C", {Member("baz", ty.vec3<f32>())});

  auto* b_type = Structure("B", {Member("bar", c_type)});
  auto* b_ary_type = ty.array(b_type, 3);
  auto* a_type = Structure("A", {Member("foo", b_ary_type)});

  auto* a_ary_type = ty.array(a_type, 2);
  auto* var = Var("index", a_ary_type);
  auto* expr = MemberAccessor(
      MemberAccessor(
          MemberAccessor(
              IndexAccessor(MemberAccessor(IndexAccessor("index", 0), "foo"),
                            2),
              "bar"),
          "baz"),
      "yx");
  WrapInFunction(var, expr);

  spirv::Builder& b = Build();

  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateFunctionVariable(var)) << b.error();

  EXPECT_EQ(b.GenerateAccessorExpression(expr), 22u);

  EXPECT_EQ(DumpInstructions(b.types()), R"(%9 = OpTypeFloat 32
%8 = OpTypeVector %9 3
%7 = OpTypeStruct %8
%6 = OpTypeStruct %7
%10 = OpTypeInt 32 0
%11 = OpConstant %10 3
%5 = OpTypeArray %6 %11
%4 = OpTypeStruct %5
%12 = OpConstant %10 2
%3 = OpTypeArray %4 %12
%2 = OpTypePointer Function %3
%13 = OpConstantNull %3
%14 = OpTypeInt 32 1
%15 = OpConstant %14 0
%16 = OpConstant %10 0
%17 = OpConstant %14 2
%18 = OpTypePointer Function %8
%20 = OpTypeVector %9 2
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].variables()),
            R"(%1 = OpVariable %2 Function %13
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%19 = OpAccessChain %18 %1 %15 %16 %17 %16 %16
%21 = OpLoad %8 %19
%22 = OpVectorShuffle %20 %21 %21 1 0
)");
}

TEST_F(BuilderTest, ArrayAccessor_Of_Vec) {
  // let pos : array<vec2<f32>, 3> = array<vec2<f32>, 3>(
  //   vec2<f32>(0.0, 0.5),
  //   vec2<f32>(-0.5, -0.5),
  //   vec2<f32>(0.5, -0.5));
  // pos[1]

  auto* var =
      Const("pos", ty.array(ty.vec2<f32>(), 3),
            Construct(ty.array(ty.vec2<f32>(), 3), vec2<f32>(0.0f, 0.5f),
                      vec2<f32>(-0.5f, -0.5f), vec2<f32>(0.5f, -0.5f)));

  auto* expr = IndexAccessor("pos", 1u);
  WrapInFunction(var, expr);

  spirv::Builder& b = SanitizeAndBuild();

  ASSERT_TRUE(b.Build());

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeVoid
%1 = OpTypeFunction %2
%7 = OpTypeFloat 32
%6 = OpTypeVector %7 2
%8 = OpTypeInt 32 0
%9 = OpConstant %8 3
%5 = OpTypeArray %6 %9
%10 = OpConstant %7 0
%11 = OpConstant %7 0.5
%12 = OpConstantComposite %6 %10 %11
%13 = OpConstant %7 -0.5
%14 = OpConstantComposite %6 %13 %13
%15 = OpConstantComposite %6 %11 %13
%16 = OpConstantComposite %5 %12 %14 %15
%17 = OpConstant %8 1
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].variables()), R"()");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%18 = OpCompositeExtract %6 %16 1
)");

  Validate(b);
}

TEST_F(BuilderTest, ArrayAccessor_Of_Array_Of_f32) {
  // let pos : array<array<f32, 2>, 3> = array<vec2<f32, 2>, 3>(
  //   array<f32, 2>(0.0, 0.5),
  //   array<f32, 2>(-0.5, -0.5),
  //   array<f32, 2>(0.5, -0.5));
  // pos[2][1]

  auto* var =
      Const("pos", ty.array(ty.vec2<f32>(), 3),
            Construct(ty.array(ty.vec2<f32>(), 3), vec2<f32>(0.0f, 0.5f),
                      vec2<f32>(-0.5f, -0.5f), vec2<f32>(0.5f, -0.5f)));

  auto* expr = IndexAccessor(IndexAccessor("pos", 2u), 1u);
  WrapInFunction(var, expr);

  spirv::Builder& b = SanitizeAndBuild();

  ASSERT_TRUE(b.Build());

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeVoid
%1 = OpTypeFunction %2
%7 = OpTypeFloat 32
%6 = OpTypeVector %7 2
%8 = OpTypeInt 32 0
%9 = OpConstant %8 3
%5 = OpTypeArray %6 %9
%10 = OpConstant %7 0
%11 = OpConstant %7 0.5
%12 = OpConstantComposite %6 %10 %11
%13 = OpConstant %7 -0.5
%14 = OpConstantComposite %6 %13 %13
%15 = OpConstantComposite %6 %11 %13
%16 = OpConstantComposite %5 %12 %14 %15
%17 = OpConstant %8 2
%19 = OpConstant %8 1
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].variables()), R"()");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%18 = OpCompositeExtract %6 %16 2
%20 = OpCompositeExtract %7 %18 1
)");

  Validate(b);
}

TEST_F(BuilderTest, ArrayAccessor_Vec_Literal) {
  // let pos : vec2<f32> = vec2<f32>(0.0, 0.5);
  // pos[1]

  auto* var = Const("pos", ty.vec2<f32>(), vec2<f32>(0.0f, 0.5f));

  auto* expr = IndexAccessor("pos", 1u);
  WrapInFunction(var, expr);

  spirv::Builder& b = Build();

  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.error();
  EXPECT_EQ(b.GenerateAccessorExpression(expr), 8u) << b.error();

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 2
%3 = OpConstant %2 0
%4 = OpConstant %2 0.5
%5 = OpConstantComposite %1 %3 %4
%6 = OpTypeInt 32 0
%7 = OpConstant %6 1
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].variables()), "");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%8 = OpCompositeExtract %2 %5 1
)");
}

TEST_F(BuilderTest, ArrayAccessor_Vec_Dynamic) {
  // let pos : vec2<f32> = vec2<f32>(0.0, 0.5);
  // idx : i32
  // pos[idx]

  auto* var = Const("pos", ty.vec2<f32>(), vec2<f32>(0.0f, 0.5f));
  auto* idx = Var("idx", ty.i32());
  auto* expr = IndexAccessor("pos", idx);

  WrapInFunction(var, idx, expr);

  spirv::Builder& b = Build();

  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.error();
  ASSERT_TRUE(b.GenerateFunctionVariable(idx)) << b.error();
  EXPECT_EQ(b.GenerateAccessorExpression(expr), 11u) << b.error();

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeFloat 32
%1 = OpTypeVector %2 2
%3 = OpConstant %2 0
%4 = OpConstant %2 0.5
%5 = OpConstantComposite %1 %3 %4
%8 = OpTypeInt 32 1
%7 = OpTypePointer Function %8
%9 = OpConstantNull %8
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].variables()),
            R"(%6 = OpVariable %7 Function %9
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%10 = OpLoad %8 %6
%11 = OpVectorExtractDynamic %2 %5 %10
)");
}

TEST_F(BuilderTest, ArrayAccessor_Array_Literal) {
  // let a : array<f32, 3>;
  // a[2]

  auto* var = Const("a", ty.array<f32, 3>(),
                    Construct(ty.array<f32, 3>(), 0.0f, 0.5f, 1.0f));
  auto* expr = IndexAccessor("a", 2);
  WrapInFunction(var, expr);

  spirv::Builder& b = SanitizeAndBuild();

  ASSERT_TRUE(b.Build());

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = OpTypeFloat 32
%7 = OpTypeInt 32 0
%8 = OpConstant %7 3
%5 = OpTypeArray %6 %8
%9 = OpConstant %6 0
%10 = OpConstant %6 0.5
%11 = OpConstant %6 1
%12 = OpConstantComposite %5 %9 %10 %11
%13 = OpTypeInt 32 1
%14 = OpConstant %13 2
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].variables()), "");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%15 = OpCompositeExtract %6 %12 2
)");

  Validate(b);
}

TEST_F(BuilderTest, ArrayAccessor_Array_Dynamic) {
  // let a : array<f32, 3>;
  // idx : i32
  // a[idx]

  auto* var = Const("a", ty.array<f32, 3>(),
                    Construct(ty.array<f32, 3>(), 0.0f, 0.5f, 1.0f));

  auto* idx = Var("idx", ty.i32());
  auto* expr = IndexAccessor("a", idx);

  WrapInFunction(var, idx, expr);

  spirv::Builder& b = SanitizeAndBuild();

  ASSERT_TRUE(b.Build());

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeVoid
%1 = OpTypeFunction %2
%6 = OpTypeFloat 32
%7 = OpTypeInt 32 0
%8 = OpConstant %7 3
%5 = OpTypeArray %6 %8
%9 = OpConstant %6 0
%10 = OpConstant %6 0.5
%11 = OpConstant %6 1
%12 = OpConstantComposite %5 %9 %10 %11
%15 = OpTypeInt 32 1
%14 = OpTypePointer Function %15
%16 = OpConstantNull %15
%18 = OpTypePointer Function %5
%19 = OpConstantNull %5
%21 = OpTypePointer Function %6
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].variables()),
            R"(%13 = OpVariable %14 Function %16
%17 = OpVariable %18 Function %19
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(OpStore %17 %12
%20 = OpLoad %15 %13
%22 = OpAccessChain %21 %17 %20
%23 = OpLoad %6 %22
)");

  Validate(b);
}

TEST_F(BuilderTest, ArrayAccessor_Matrix_Dynamic) {
  // let a : mat2x2<f32>(vec2<f32>(1., 2.), vec2<f32>(3., 4.));
  // idx : i32
  // a[idx]

  auto* var =
      Const("a", ty.mat2x2<f32>(),
            Construct(ty.mat2x2<f32>(), Construct(ty.vec2<f32>(), 1.f, 2.f),
                      Construct(ty.vec2<f32>(), 3.f, 4.f)));

  auto* idx = Var("idx", ty.i32());
  auto* expr = IndexAccessor("a", idx);

  WrapInFunction(var, idx, expr);

  spirv::Builder& b = SanitizeAndBuild();

  ASSERT_TRUE(b.Build());

  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeVoid
%1 = OpTypeFunction %2
%7 = OpTypeFloat 32
%6 = OpTypeVector %7 2
%5 = OpTypeMatrix %6 2
%8 = OpConstant %7 1
%9 = OpConstant %7 2
%10 = OpConstantComposite %6 %8 %9
%11 = OpConstant %7 3
%12 = OpConstant %7 4
%13 = OpConstantComposite %6 %11 %12
%14 = OpConstantComposite %5 %10 %13
%17 = OpTypeInt 32 1
%16 = OpTypePointer Function %17
%18 = OpConstantNull %17
%20 = OpTypePointer Function %5
%21 = OpConstantNull %5
%23 = OpTypePointer Function %6
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].variables()),
            R"(%15 = OpVariable %16 Function %18
%19 = OpVariable %20 Function %21
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(OpStore %19 %14
%22 = OpLoad %17 %15
%24 = OpAccessChain %23 %19 %22
%25 = OpLoad %6 %24
)");

  Validate(b);
}

}  // namespace
}  // namespace spirv
}  // namespace writer
}  // namespace tint
