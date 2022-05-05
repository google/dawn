// Copyright 2021 The Tint Authors.
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

#include "src/tint/ast/discard_statement.h"
#include "src/tint/ast/return_statement.h"
#include "src/tint/ast/stage_attribute.h"
#include "src/tint/resolver/resolver.h"
#include "src/tint/resolver/resolver_test_helper.h"

#include "gmock/gmock.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::resolver {
namespace {

class ResolverFunctionValidationTest : public TestHelper, public testing::Test {};

TEST_F(ResolverFunctionValidationTest, DuplicateParameterName) {
    // fn func_a(common_name : f32) { }
    // fn func_b(common_name : f32) { }
    Func("func_a", {Param("common_name", ty.f32())}, ty.void_(), {});
    Func("func_b", {Param("common_name", ty.f32())}, ty.void_(), {});

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverFunctionValidationTest, ParameterMayShadowGlobal) {
    // var<private> common_name : f32;
    // fn func(common_name : f32) { }
    Global("common_name", ty.f32(), ast::StorageClass::kPrivate);
    Func("func", {Param("common_name", ty.f32())}, ty.void_(), {});

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverFunctionValidationTest, LocalConflictsWithParameter) {
    // fn func(common_name : f32) {
    //   let common_name = 1i;
    // }
    Func("func", {Param(Source{{12, 34}}, "common_name", ty.f32())}, ty.void_(),
         {Decl(Let(Source{{56, 78}}, "common_name", nullptr, Expr(1_i)))});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(56:78 error: redeclaration of 'common_name'
12:34 note: 'common_name' previously declared here)");
}

TEST_F(ResolverFunctionValidationTest, NestedLocalMayShadowParameter) {
    // fn func(common_name : f32) {
    //   {
    //     let common_name = 1i;
    //   }
    // }
    Func("func", {Param(Source{{12, 34}}, "common_name", ty.f32())}, ty.void_(),
         {Block(Decl(Let(Source{{56, 78}}, "common_name", nullptr, Expr(1_i))))});

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverFunctionValidationTest, VoidFunctionEndWithoutReturnStatement_Pass) {
    // fn func { var a:i32 = 2i; }
    auto* var = Var("a", ty.i32(), Expr(2_i));

    Func(Source{{12, 34}}, "func", ast::VariableList{}, ty.void_(),
         ast::StatementList{
             Decl(var),
         });

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverFunctionValidationTest, FunctionUsingSameVariableName_Pass) {
    // fn func() -> i32 {
    //   var func:i32 = 0i;
    //   return func;
    // }

    auto* var = Var("func", ty.i32(), Expr(0_i));
    Func("func", ast::VariableList{}, ty.i32(),
         ast::StatementList{
             Decl(var),
             Return(Source{{12, 34}}, Expr("func")),
         },
         ast::AttributeList{});

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverFunctionValidationTest, FunctionNameSameAsFunctionScopeVariableName_Pass) {
    // fn a() -> void { var b:i32 = 0i; }
    // fn b() -> i32 { return 2; }

    auto* var = Var("b", ty.i32(), Expr(0_i));
    Func("a", ast::VariableList{}, ty.void_(),
         ast::StatementList{
             Decl(var),
         },
         ast::AttributeList{});

    Func(Source{{12, 34}}, "b", ast::VariableList{}, ty.i32(),
         ast::StatementList{
             Return(2_i),
         },
         ast::AttributeList{});

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverFunctionValidationTest, UnreachableCode_return) {
    // fn func() -> {
    //  var a : i32;
    //  return;
    //  a = 2i;
    //}

    auto* decl_a = Decl(Var("a", ty.i32()));
    auto* ret = Return();
    auto* assign_a = Assign(Source{{12, 34}}, "a", 2_i);

    Func("func", ast::VariableList{}, ty.void_(), {decl_a, ret, assign_a});

    ASSERT_TRUE(r()->Resolve());

    EXPECT_EQ(r()->error(), "12:34 warning: code is unreachable");
    EXPECT_TRUE(Sem().Get(decl_a)->IsReachable());
    EXPECT_TRUE(Sem().Get(ret)->IsReachable());
    EXPECT_FALSE(Sem().Get(assign_a)->IsReachable());
}

TEST_F(ResolverFunctionValidationTest, UnreachableCode_return_InBlocks) {
    // fn func() -> {
    //  var a : i32;
    //  {{{return;}}}
    //  a = 2i;
    //}

    auto* decl_a = Decl(Var("a", ty.i32()));
    auto* ret = Return();
    auto* assign_a = Assign(Source{{12, 34}}, "a", 2_i);

    Func("func", ast::VariableList{}, ty.void_(), {decl_a, Block(Block(Block(ret))), assign_a});

    ASSERT_TRUE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 warning: code is unreachable");
    EXPECT_TRUE(Sem().Get(decl_a)->IsReachable());
    EXPECT_TRUE(Sem().Get(ret)->IsReachable());
    EXPECT_FALSE(Sem().Get(assign_a)->IsReachable());
}

TEST_F(ResolverFunctionValidationTest, UnreachableCode_discard) {
    // fn func() -> {
    //  var a : i32;
    //  discard;
    //  a = 2i;
    //}

    auto* decl_a = Decl(Var("a", ty.i32()));
    auto* discard = Discard();
    auto* assign_a = Assign(Source{{12, 34}}, "a", 2_i);

    Func("func", ast::VariableList{}, ty.void_(), {decl_a, discard, assign_a});

    ASSERT_TRUE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 warning: code is unreachable");
    EXPECT_TRUE(Sem().Get(decl_a)->IsReachable());
    EXPECT_TRUE(Sem().Get(discard)->IsReachable());
    EXPECT_FALSE(Sem().Get(assign_a)->IsReachable());
}

TEST_F(ResolverFunctionValidationTest, UnreachableCode_discard_InBlocks) {
    // fn func() -> {
    //  var a : i32;
    //  {{{discard;}}}
    //  a = 2i;
    //}

    auto* decl_a = Decl(Var("a", ty.i32()));
    auto* discard = Discard();
    auto* assign_a = Assign(Source{{12, 34}}, "a", 2_i);

    Func("func", ast::VariableList{}, ty.void_(), {decl_a, Block(Block(Block(discard))), assign_a});

    ASSERT_TRUE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 warning: code is unreachable");
    EXPECT_TRUE(Sem().Get(decl_a)->IsReachable());
    EXPECT_TRUE(Sem().Get(discard)->IsReachable());
    EXPECT_FALSE(Sem().Get(assign_a)->IsReachable());
}

TEST_F(ResolverFunctionValidationTest, FunctionEndWithoutReturnStatement_Fail) {
    // fn func() -> int { var a:i32 = 2i; }

    auto* var = Var("a", ty.i32(), Expr(2_i));

    Func(Source{{12, 34}}, "func", ast::VariableList{}, ty.i32(),
         ast::StatementList{
             Decl(var),
         },
         ast::AttributeList{});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: missing return at end of function");
}

TEST_F(ResolverFunctionValidationTest, VoidFunctionEndWithoutReturnStatementEmptyBody_Pass) {
    // fn func {}

    Func(Source{{12, 34}}, "func", ast::VariableList{}, ty.void_(), ast::StatementList{});

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverFunctionValidationTest, FunctionEndWithoutReturnStatementEmptyBody_Fail) {
    // fn func() -> int {}

    Func(Source{{12, 34}}, "func", ast::VariableList{}, ty.i32(), ast::StatementList{},
         ast::AttributeList{});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: missing return at end of function");
}

TEST_F(ResolverFunctionValidationTest, FunctionTypeMustMatchReturnStatementType_Pass) {
    // fn func { return; }

    Func("func", ast::VariableList{}, ty.void_(),
         ast::StatementList{
             Return(),
         });

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverFunctionValidationTest, FunctionTypeMustMatchReturnStatementType_fail) {
    // fn func { return 2i; }
    Func("func", ast::VariableList{}, ty.void_(),
         ast::StatementList{
             Return(Source{{12, 34}}, Expr(2_i)),
         },
         ast::AttributeList{});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: return statement type must match its function return "
              "type, returned 'i32', expected 'void'");
}

TEST_F(ResolverFunctionValidationTest, FunctionTypeMustMatchReturnStatementType_void_fail) {
    // fn v { return; }
    // fn func { return v(); }
    Func("v", {}, ty.void_(), {Return()});
    Func("func", {}, ty.void_(),
         {
             Return(Call(Source{{12, 34}}, "v")),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: function 'v' does not return a value");
}

TEST_F(ResolverFunctionValidationTest, FunctionTypeMustMatchReturnStatementTypeMissing_fail) {
    // fn func() -> f32 { return; }
    Func("func", ast::VariableList{}, ty.f32(),
         ast::StatementList{
             Return(Source{{12, 34}}, nullptr),
         },
         ast::AttributeList{});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: return statement type must match its function return "
              "type, returned 'void', expected 'f32'");
}

TEST_F(ResolverFunctionValidationTest, FunctionTypeMustMatchReturnStatementTypeF32_pass) {
    // fn func() -> f32 { return 2.0; }
    Func("func", ast::VariableList{}, ty.f32(),
         ast::StatementList{
             Return(Source{{12, 34}}, Expr(2.f)),
         },
         ast::AttributeList{});

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverFunctionValidationTest, FunctionTypeMustMatchReturnStatementTypeF32_fail) {
    // fn func() -> f32 { return 2i; }
    Func("func", ast::VariableList{}, ty.f32(),
         ast::StatementList{
             Return(Source{{12, 34}}, Expr(2_i)),
         },
         ast::AttributeList{});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: return statement type must match its function return "
              "type, returned 'i32', expected 'f32'");
}

TEST_F(ResolverFunctionValidationTest, FunctionTypeMustMatchReturnStatementTypeF32Alias_pass) {
    // type myf32 = f32;
    // fn func() -> myf32 { return 2.0; }
    auto* myf32 = Alias("myf32", ty.f32());
    Func("func", ast::VariableList{}, ty.Of(myf32),
         ast::StatementList{
             Return(Source{{12, 34}}, Expr(2.f)),
         },
         ast::AttributeList{});

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverFunctionValidationTest, FunctionTypeMustMatchReturnStatementTypeF32Alias_fail) {
    // type myf32 = f32;
    // fn func() -> myf32 { return 2u; }
    auto* myf32 = Alias("myf32", ty.f32());
    Func("func", ast::VariableList{}, ty.Of(myf32),
         ast::StatementList{
             Return(Source{{12, 34}}, Expr(2_u)),
         },
         ast::AttributeList{});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: return statement type must match its function return "
              "type, returned 'u32', expected 'f32'");
}

TEST_F(ResolverFunctionValidationTest, CannotCallEntryPoint) {
    // @stage(compute) @workgroup_size(1) fn entrypoint() {}
    // fn func() { return entrypoint(); }
    Func("entrypoint", ast::VariableList{}, ty.void_(), {},
         {Stage(ast::PipelineStage::kCompute), WorkgroupSize(1_i)});

    Func("func", ast::VariableList{}, ty.void_(),
         {
             CallStmt(Call(Source{{12, 34}}, "entrypoint")),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),

              R"(12:34 error: entry point functions cannot be the target of a function call)");
}

TEST_F(ResolverFunctionValidationTest, PipelineStage_MustBeUnique_Fail) {
    // @stage(fragment)
    // @stage(vertex)
    // fn main() { return; }
    Func(Source{{12, 34}}, "main", ast::VariableList{}, ty.void_(),
         ast::StatementList{
             Return(),
         },
         ast::AttributeList{
             Stage(Source{{12, 34}}, ast::PipelineStage::kVertex),
             Stage(Source{{56, 78}}, ast::PipelineStage::kFragment),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(56:78 error: duplicate stage attribute
12:34 note: first attribute declared here)");
}

TEST_F(ResolverFunctionValidationTest, NoPipelineEntryPoints) {
    Func("vtx_func", ast::VariableList{}, ty.void_(),
         ast::StatementList{
             Return(),
         },
         ast::AttributeList{});

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverFunctionValidationTest, FunctionVarInitWithParam) {
    // fn foo(bar : f32){
    //   var baz : f32 = bar;
    // }

    auto* bar = Param("bar", ty.f32());
    auto* baz = Var("baz", ty.f32(), Expr("bar"));

    Func("foo", ast::VariableList{bar}, ty.void_(), ast::StatementList{Decl(baz)},
         ast::AttributeList{});

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverFunctionValidationTest, FunctionConstInitWithParam) {
    // fn foo(bar : f32){
    //   let baz : f32 = bar;
    // }

    auto* bar = Param("bar", ty.f32());
    auto* baz = Let("baz", ty.f32(), Expr("bar"));

    Func("foo", ast::VariableList{bar}, ty.void_(), ast::StatementList{Decl(baz)},
         ast::AttributeList{});

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverFunctionValidationTest, FunctionParamsConst) {
    Func("foo", {Param(Sym("arg"), ty.i32())}, ty.void_(),
         {Assign(Expr(Source{{12, 34}}, "arg"), Expr(1_i)), Return()});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: cannot assign to function parameter\nnote: 'arg' is "
              "declared here:");
}

TEST_F(ResolverFunctionValidationTest, WorkgroupSize_GoodType_ConstU32) {
    // let x = 4u;
    // let x = 8u;
    // @stage(compute) @workgroup_size(x, y, 16u)
    // fn main() {}
    auto* x = GlobalConst("x", ty.u32(), Expr(4_u));
    auto* y = GlobalConst("y", ty.u32(), Expr(8_u));
    auto* func = Func(
        "main", {}, ty.void_(), {},
        {Stage(ast::PipelineStage::kCompute), WorkgroupSize(Expr("x"), Expr("y"), Expr(16_u))});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem_func = Sem().Get(func);
    auto* sem_x = Sem().Get<sem::GlobalVariable>(x);
    auto* sem_y = Sem().Get<sem::GlobalVariable>(y);

    ASSERT_NE(sem_func, nullptr);
    ASSERT_NE(sem_x, nullptr);
    ASSERT_NE(sem_y, nullptr);

    EXPECT_TRUE(sem_func->DirectlyReferencedGlobals().contains(sem_x));
    EXPECT_TRUE(sem_func->DirectlyReferencedGlobals().contains(sem_y));
}

TEST_F(ResolverFunctionValidationTest, WorkgroupSize_GoodType_U32) {
    // @stage(compute) @workgroup_size(1u, 2u, 3u)
    // fn main() {}

    Func("main", {}, ty.void_(), {},
         {Stage(ast::PipelineStage::kCompute),
          WorkgroupSize(Source{{12, 34}}, Expr(1_u), Expr(2_u), Expr(3_u))});

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverFunctionValidationTest, WorkgroupSize_MismatchTypeU32) {
    // @stage(compute) @workgroup_size(1u, 2u, 3_i)
    // fn main() {}

    Func("main", {}, ty.void_(), {},
         {Stage(ast::PipelineStage::kCompute),
          WorkgroupSize(Expr(1_u), Expr(2_u), Expr(Source{{12, 34}}, 3_i))});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: workgroup_size arguments must be of the same type, "
              "either i32 or u32");
}

TEST_F(ResolverFunctionValidationTest, WorkgroupSize_MismatchTypeI32) {
    // @stage(compute) @workgroup_size(1_i, 2u, 3_i)
    // fn main() {}

    Func("main", {}, ty.void_(), {},
         {Stage(ast::PipelineStage::kCompute),
          WorkgroupSize(Expr(1_i), Expr(Source{{12, 34}}, 2_u), Expr(3_i))});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: workgroup_size arguments must be of the same type, "
              "either i32 or u32");
}

TEST_F(ResolverFunctionValidationTest, WorkgroupSize_Const_TypeMismatch) {
    // let x = 64u;
    // @stage(compute) @workgroup_size(1i, x)
    // fn main() {}
    GlobalConst("x", ty.u32(), Expr(64_u));
    Func("main", {}, ty.void_(), {},
         {Stage(ast::PipelineStage::kCompute),
          WorkgroupSize(Expr(1_i), Expr(Source{{12, 34}}, "x"))});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: workgroup_size arguments must be of the same type, "
              "either i32 or u32");
}

TEST_F(ResolverFunctionValidationTest, WorkgroupSize_Const_TypeMismatch2) {
    // let x = 64u;
    // let y = 32i;
    // @stage(compute) @workgroup_size(x, y)
    // fn main() {}
    GlobalConst("x", ty.u32(), Expr(64_u));
    GlobalConst("y", ty.i32(), Expr(32_i));
    Func("main", {}, ty.void_(), {},
         {Stage(ast::PipelineStage::kCompute),
          WorkgroupSize(Expr("x"), Expr(Source{{12, 34}}, "y"))});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: workgroup_size arguments must be of the same type, "
              "either i32 or u32");
}
TEST_F(ResolverFunctionValidationTest, WorkgroupSize_Mismatch_ConstU32) {
    // let x = 4u;
    // let x = 8u;
    // @stage(compute) @workgroup_size(x, y, 16i)
    // fn main() {}
    GlobalConst("x", ty.u32(), Expr(4_u));
    GlobalConst("y", ty.u32(), Expr(8_u));
    Func("main", {}, ty.void_(), {},
         {Stage(ast::PipelineStage::kCompute),
          WorkgroupSize(Expr("x"), Expr("y"), Expr(Source{{12, 34}}, 16_i))});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: workgroup_size arguments must be of the same type, "
              "either i32 or u32");
}

TEST_F(ResolverFunctionValidationTest, WorkgroupSize_Literal_BadType) {
    // @stage(compute) @workgroup_size(64.0)
    // fn main() {}

    Func("main", {}, ty.void_(), {},
         {Stage(ast::PipelineStage::kCompute), WorkgroupSize(Expr(Source{{12, 34}}, 64.f))});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: workgroup_size argument must be either literal or "
              "module-scope constant of type i32 or u32");
}

TEST_F(ResolverFunctionValidationTest, WorkgroupSize_Literal_Negative) {
    // @stage(compute) @workgroup_size(-2i)
    // fn main() {}

    Func("main", {}, ty.void_(), {},
         {Stage(ast::PipelineStage::kCompute), WorkgroupSize(Expr(Source{{12, 34}}, i32(-2)))});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: workgroup_size argument must be at least 1");
}

TEST_F(ResolverFunctionValidationTest, WorkgroupSize_Literal_Zero) {
    // @stage(compute) @workgroup_size(0i)
    // fn main() {}

    Func("main", {}, ty.void_(), {},
         {Stage(ast::PipelineStage::kCompute), WorkgroupSize(Expr(Source{{12, 34}}, 0_i))});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: workgroup_size argument must be at least 1");
}

TEST_F(ResolverFunctionValidationTest, WorkgroupSize_Const_BadType) {
    // let x = 64.0;
    // @stage(compute) @workgroup_size(x)
    // fn main() {}
    GlobalConst("x", ty.f32(), Expr(64.f));
    Func("main", {}, ty.void_(), {},
         {Stage(ast::PipelineStage::kCompute), WorkgroupSize(Expr(Source{{12, 34}}, "x"))});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: workgroup_size argument must be either literal or "
              "module-scope constant of type i32 or u32");
}

TEST_F(ResolverFunctionValidationTest, WorkgroupSize_Const_Negative) {
    // let x = -2i;
    // @stage(compute) @workgroup_size(x)
    // fn main() {}
    GlobalConst("x", ty.i32(), Expr(i32(-2)));
    Func("main", {}, ty.void_(), {},
         {Stage(ast::PipelineStage::kCompute), WorkgroupSize(Expr(Source{{12, 34}}, "x"))});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: workgroup_size argument must be at least 1");
}

TEST_F(ResolverFunctionValidationTest, WorkgroupSize_Const_Zero) {
    // let x = 0i;
    // @stage(compute) @workgroup_size(x)
    // fn main() {}
    GlobalConst("x", ty.i32(), Expr(0_i));
    Func("main", {}, ty.void_(), {},
         {Stage(ast::PipelineStage::kCompute), WorkgroupSize(Expr(Source{{12, 34}}, "x"))});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: workgroup_size argument must be at least 1");
}

TEST_F(ResolverFunctionValidationTest, WorkgroupSize_Const_NestedZeroValueConstructor) {
    // let x = i32(i32(i32()));
    // @stage(compute) @workgroup_size(x)
    // fn main() {}
    GlobalConst("x", ty.i32(), Construct(ty.i32(), Construct(ty.i32(), Construct(ty.i32()))));
    Func("main", {}, ty.void_(), {},
         {Stage(ast::PipelineStage::kCompute), WorkgroupSize(Expr(Source{{12, 34}}, "x"))});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: workgroup_size argument must be at least 1");
}

TEST_F(ResolverFunctionValidationTest, WorkgroupSize_NonConst) {
    // var<private> x = 64i;
    // @stage(compute) @workgroup_size(x)
    // fn main() {}
    Global("x", ty.i32(), ast::StorageClass::kPrivate, Expr(64_i));
    Func("main", {}, ty.void_(), {},
         {Stage(ast::PipelineStage::kCompute), WorkgroupSize(Expr(Source{{12, 34}}, "x"))});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: workgroup_size argument must be either literal or "
              "module-scope constant of type i32 or u32");
}

TEST_F(ResolverFunctionValidationTest, WorkgroupSize_InvalidExpr) {
    // @stage(compute) @workgroup_size(i32(1))
    // fn main() {}
    Func("main", {}, ty.void_(), {},
         {Stage(ast::PipelineStage::kCompute),
          WorkgroupSize(Construct(Source{{12, 34}}, ty.i32(), 1_i))});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: workgroup_size argument must be either a literal or "
              "a module-scope constant");
}

TEST_F(ResolverFunctionValidationTest, ReturnIsConstructible_NonPlain) {
    auto* ret_type = ty.pointer(Source{{12, 34}}, ty.i32(), ast::StorageClass::kFunction);
    Func("f", {}, ret_type, {});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: function return type must be a constructible type");
}

TEST_F(ResolverFunctionValidationTest, ReturnIsConstructible_AtomicInt) {
    auto* ret_type = ty.atomic(Source{{12, 34}}, ty.i32());
    Func("f", {}, ret_type, {});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: function return type must be a constructible type");
}

TEST_F(ResolverFunctionValidationTest, ReturnIsConstructible_ArrayOfAtomic) {
    auto* ret_type = ty.array(Source{{12, 34}}, ty.atomic(ty.i32()), 10_u);
    Func("f", {}, ret_type, {});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: function return type must be a constructible type");
}

TEST_F(ResolverFunctionValidationTest, ReturnIsConstructible_StructOfAtomic) {
    Structure("S", {Member("m", ty.atomic(ty.i32()))});
    auto* ret_type = ty.type_name(Source{{12, 34}}, "S");
    Func("f", {}, ret_type, {});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: function return type must be a constructible type");
}

TEST_F(ResolverFunctionValidationTest, ReturnIsConstructible_RuntimeArray) {
    auto* ret_type = ty.array(Source{{12, 34}}, ty.i32());
    Func("f", {}, ret_type, {});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: function return type must be a constructible type");
}

TEST_F(ResolverFunctionValidationTest, ParameterStoreType_NonAtomicFree) {
    Structure("S", {Member("m", ty.atomic(ty.i32()))});
    auto* ret_type = ty.type_name(Source{{12, 34}}, "S");
    auto* bar = Param(Source{{12, 34}}, "bar", ret_type);
    Func("f", ast::VariableList{bar}, ty.void_(), {});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: store type of function parameter must be a "
              "constructible type");
}

TEST_F(ResolverFunctionValidationTest, ParameterSotreType_AtomicFree) {
    Structure("S", {Member("m", ty.i32())});
    auto* ret_type = ty.type_name(Source{{12, 34}}, "S");
    auto* bar = Param(Source{{12, 34}}, "bar", ret_type);
    Func("f", ast::VariableList{bar}, ty.void_(), {});

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverFunctionValidationTest, ParametersAtLimit) {
    ast::VariableList params;
    for (int i = 0; i < 255; i++) {
        params.emplace_back(Param("param_" + std::to_string(i), ty.i32()));
    }
    Func(Source{{12, 34}}, "f", params, ty.void_(), {});

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverFunctionValidationTest, ParametersOverLimit) {
    ast::VariableList params;
    for (int i = 0; i < 256; i++) {
        params.emplace_back(Param("param_" + std::to_string(i), ty.i32()));
    }
    Func(Source{{12, 34}}, "f", params, ty.void_(), {});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: functions may declare at most 255 parameters");
}

TEST_F(ResolverFunctionValidationTest, ParameterVectorNoType) {
    // fn f(p : vec3) {}

    Func(Source{{12, 34}}, "f", {Param("p", create<ast::Vector>(Source{{12, 34}}, nullptr, 3))},
         ty.void_(), {});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: missing vector element type");
}

TEST_F(ResolverFunctionValidationTest, ParameterMatrixNoType) {
    // fn f(p : vec3) {}

    Func(Source{{12, 34}}, "f", {Param("p", create<ast::Matrix>(Source{{12, 34}}, nullptr, 3, 3))},
         ty.void_(), {});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: missing matrix element type");
}

struct TestParams {
    ast::StorageClass storage_class;
    bool should_pass;
};

struct TestWithParams : ResolverTestWithParam<TestParams> {};

using ResolverFunctionParameterValidationTest = TestWithParams;
TEST_P(ResolverFunctionParameterValidationTest, StorageClass) {
    auto& param = GetParam();
    auto* ptr_type = ty.pointer(Source{{12, 34}}, ty.i32(), param.storage_class);
    auto* arg = Param(Source{{12, 34}}, "p", ptr_type);
    Func("f", ast::VariableList{arg}, ty.void_(), {});

    if (param.should_pass) {
        ASSERT_TRUE(r()->Resolve()) << r()->error();
    } else {
        std::stringstream ss;
        ss << param.storage_class;
        EXPECT_FALSE(r()->Resolve());
        EXPECT_EQ(r()->error(), "12:34 error: function parameter of pointer type cannot be in '" +
                                    ss.str() + "' storage class");
    }
}
INSTANTIATE_TEST_SUITE_P(ResolverTest,
                         ResolverFunctionParameterValidationTest,
                         testing::Values(TestParams{ast::StorageClass::kNone, false},
                                         TestParams{ast::StorageClass::kInput, false},
                                         TestParams{ast::StorageClass::kOutput, false},
                                         TestParams{ast::StorageClass::kUniform, false},
                                         TestParams{ast::StorageClass::kWorkgroup, true},
                                         TestParams{ast::StorageClass::kHandle, false},
                                         TestParams{ast::StorageClass::kStorage, false},
                                         TestParams{ast::StorageClass::kPrivate, true},
                                         TestParams{ast::StorageClass::kFunction, true}));

}  // namespace
}  // namespace tint::resolver
