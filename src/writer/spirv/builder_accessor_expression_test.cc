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

TEST_F(BuilderTest, ArrayAccessor) {
  // vec3<f32> ary;
  // ary[1]  -> ptr<f32>

  auto* var = Global("ary", ty.vec3<f32>(), ast::StorageClass::kFunction);

  auto* ary = Expr("ary");
  auto* idx_expr = Expr(1);

  auto* expr = IndexAccessor(ary, idx_expr);
  WrapInFunction(expr);

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

TEST_F(BuilderTest, Accessor_Array_LoadIndex) {
  // ary : vec3<f32>;
  // idx : i32;
  // ary[idx]  -> ptr<f32>

  auto* var = Global("ary", ty.vec3<f32>(), ast::StorageClass::kFunction);
  auto* idx = Global("idx", ty.i32(), ast::StorageClass::kFunction);

  auto* ary = Expr("ary");
  auto* idx_expr = Expr("idx");

  auto* expr = IndexAccessor(ary, idx_expr);
  WrapInFunction(expr);

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

TEST_F(BuilderTest, ArrayAccessor_Dynamic) {
  // vec3<f32> ary;
  // ary[1 + 2]  -> ptr<f32>

  auto* var = Global("ary", ty.vec3<f32>(), ast::StorageClass::kFunction);

  auto* ary = Expr("ary");

  auto* expr = IndexAccessor(ary, Add(1, 2));
  WrapInFunction(expr);

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

TEST_F(BuilderTest, ArrayAccessor_MultiLevel) {
  auto* ary4 = ty.array(ty.vec3<f32>(), 4);

  // ary = array<vec3<f32>, 4>
  // ary[3][2];

  auto* var = Global("ary", ary4, ast::StorageClass::kFunction);

  auto* expr = IndexAccessor(IndexAccessor("ary", 3), 2);
  WrapInFunction(expr);

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

TEST_F(BuilderTest, Accessor_ArrayWithSwizzle) {
  auto* ary4 = ty.array(ty.vec3<f32>(), 4);

  // var a : array<vec3<f32>, 4>;
  // a[2].xy;

  auto* var = Global("ary", ary4, ast::StorageClass::kFunction);

  auto* expr = MemberAccessor(IndexAccessor("ary", 2), "xy");
  WrapInFunction(expr);

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

  auto* var = Global("ident", s, ast::StorageClass::kFunction);

  auto* expr = MemberAccessor("ident", "b");
  WrapInFunction(expr);

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

  auto* var = Global("ident", s_type, ast::StorageClass::kFunction);
  auto* expr = MemberAccessor(MemberAccessor("ident", "inner"), "b");
  WrapInFunction(expr);

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
  // const ident : my_struct = my_struct();
  // ident.b

  auto* s = Structure("my_struct", {
                                       Member("a", ty.f32()),
                                       Member("b", ty.f32()),
                                   });

  auto* var = GlobalConst("ident", s, Construct(s, 0.f, 0.f));

  auto* expr = MemberAccessor("ident", "b");
  WrapInFunction(expr);

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
  // const ident : my_struct = my_struct();
  // ident.inner.a
  auto* inner_struct = Structure("Inner", {
                                              Member("a", ty.f32()),
                                              Member("b", ty.f32()),
                                          });

  auto* s_type = Structure("my_struct", {Member("inner", inner_struct)});

  auto* var = GlobalConst("ident", s_type,
                          Construct(s_type, Construct(inner_struct, 0.f, 0.f)));
  auto* expr = MemberAccessor(MemberAccessor("ident", "inner"), "b");
  WrapInFunction(expr);

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

  auto* var = Global("ident", s_type, ast::StorageClass::kFunction);
  auto* expr = MemberAccessor(MemberAccessor("ident", "inner"), "a");
  WrapInFunction(expr);

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

  auto* var = Global("ident", s_type, ast::StorageClass::kFunction);
  auto* expr = create<ast::AssignmentStatement>(
      MemberAccessor(MemberAccessor("ident", "inner"), "a"), Expr(2.0f));
  WrapInFunction(expr);

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

  auto* var = Global("ident", s_type, ast::StorageClass::kFunction);
  auto* store = Global("store", ty.f32(), ast::StorageClass::kFunction);

  auto* rhs = MemberAccessor(MemberAccessor("ident", "inner"), "a");
  auto* expr = create<ast::AssignmentStatement>(Expr("store"), rhs);
  WrapInFunction(expr);

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

  auto* var = Global("ident", ty.vec3<f32>(), ast::StorageClass::kFunction);

  auto* expr = MemberAccessor("ident", "y");
  WrapInFunction(expr);

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

  auto* var = Global("ident", ty.vec3<f32>(), ast::StorageClass::kFunction);

  auto* expr = MemberAccessor("ident", "yx");
  WrapInFunction(expr);

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

  auto* var = Global("ident", ty.vec3<f32>(), ast::StorageClass::kFunction);

  auto* expr = MemberAccessor(MemberAccessor("ident", "yxz"), "xz");
  WrapInFunction(expr);

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

  auto* var = Global("ident", ty.vec3<f32>(), ast::StorageClass::kFunction);

  auto* expr = MemberAccessor(MemberAccessor("ident", "yxz"), "x");
  WrapInFunction(expr);

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

  auto* var = Global("ident", ty.vec3<f32>(), ast::StorageClass::kFunction);

  auto* expr = IndexAccessor(MemberAccessor("ident", "yxz"), 1);
  WrapInFunction(expr);

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
%10 = OpVectorExtractDynamic %4 %7 %9
)");
}

TEST_F(BuilderTest, Accessor_Mixed_ArrayAndMember) {
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
  auto* var = Global("index", a_ary_type, ast::StorageClass::kFunction);
  auto* expr = MemberAccessor(
      MemberAccessor(
          MemberAccessor(
              IndexAccessor(MemberAccessor(IndexAccessor("index", 0), "foo"),
                            2),
              "bar"),
          "baz"),
      "yx");
  WrapInFunction(expr);

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

TEST_F(BuilderTest, Accessor_Array_Of_Vec) {
  // const pos : array<vec2<f32>, 3> = array<vec2<f32>, 3>(
  //   vec2<f32>(0.0, 0.5),
  //   vec2<f32>(-0.5, -0.5),
  //   vec2<f32>(0.5, -0.5));
  // pos[1]

  auto* arr = ty.array(ty.vec2<f32>(), 3);

  auto* var =
      GlobalConst("pos", arr,
                  Construct(arr, vec2<f32>(0.0f, 0.5f), vec2<f32>(-0.5f, -0.5f),
                            vec2<f32>(0.5f, -0.5f)));

  auto* expr = IndexAccessor("pos", 1u);
  WrapInFunction(expr);

  spirv::Builder& b = Build();

  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateFunctionVariable(var)) << b.error();
  EXPECT_EQ(b.GenerateAccessorExpression(expr), 18u) << b.error();

  EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeFloat 32
%2 = OpTypeVector %3 2
%4 = OpTypeInt 32 0
%5 = OpConstant %4 3
%1 = OpTypeArray %2 %5
%6 = OpConstant %3 0
%7 = OpConstant %3 0.5
%8 = OpConstantComposite %2 %6 %7
%9 = OpConstant %3 -0.5
%10 = OpConstantComposite %2 %9 %9
%11 = OpConstantComposite %2 %7 %9
%12 = OpConstantComposite %1 %8 %10 %11
%13 = OpTypePointer Function %1
%15 = OpConstantNull %1
%16 = OpConstant %4 1
%17 = OpTypePointer Function %2
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].variables()),
            R"(%14 = OpVariable %13 Function %15
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(OpStore %14 %12
%18 = OpAccessChain %17 %14 %16
)");
}

TEST_F(BuilderTest, Accessor_Const_Vec) {
  // const pos : vec2<f32> = vec2<f32>(0.0, 0.5);
  // pos[1]

  auto* var = GlobalConst("pos", ty.vec2<f32>(), vec2<f32>(0.0f, 0.5f));

  auto* expr = IndexAccessor("pos", 1u);
  WrapInFunction(expr);

  spirv::Builder& b = Build();

  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateFunctionVariable(var)) << b.error();
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
            R"(%8 = OpVectorExtractDynamic %2 %5 %7
)");
}

TEST_F(BuilderTest, DISABLED_Accessor_Array_NonPointer) {
  // const a : array<f32, 3>;
  // a[2]
  //
  // This has to generate an OpConstantExtract and will need to read the 3 value
  // out of the ScalarConstructor as extract requires integer indices.
}

}  // namespace
}  // namespace spirv
}  // namespace writer
}  // namespace tint
