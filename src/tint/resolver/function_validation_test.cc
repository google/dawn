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
#include "src/tint/builtin/builtin_value.h"
#include "src/tint/resolver/resolver.h"
#include "src/tint/resolver/resolver_test_helper.h"
#include "src/tint/utils/string_stream.h"

#include "gmock/gmock.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::resolver {
namespace {

class ResolverFunctionValidationTest : public TestHelper, public testing::Test {};

TEST_F(ResolverFunctionValidationTest, DuplicateParameterName) {
    // fn func_a(common_name : f32) { }
    // fn func_b(common_name : f32) { }
    Func("func_a", utils::Vector{Param("common_name", ty.f32())}, ty.void_(), utils::Empty);
    Func("func_b", utils::Vector{Param("common_name", ty.f32())}, ty.void_(), utils::Empty);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverFunctionValidationTest, ParameterMayShadowGlobal) {
    // var<private> common_name : f32;
    // fn func(common_name : f32) { }
    GlobalVar("common_name", ty.f32(), builtin::AddressSpace::kPrivate);
    Func("func", utils::Vector{Param("common_name", ty.f32())}, ty.void_(), utils::Empty);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverFunctionValidationTest, LocalConflictsWithParameter) {
    // fn func(common_name : f32) {
    //   let common_name = 1i;
    // }
    Func("func", utils::Vector{Param(Source{{12, 34}}, "common_name", ty.f32())}, ty.void_(),
         utils::Vector{
             Decl(Let(Source{{56, 78}}, "common_name", Expr(1_i))),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(56:78 error: redeclaration of 'common_name'
12:34 note: 'common_name' previously declared here)");
}

TEST_F(ResolverFunctionValidationTest, NestedLocalMayShadowParameter) {
    // fn func(common_name : f32) {
    // utils::Vector  {
    //     let common_name = 1i;
    //   }
    // }
    Func("func", utils::Vector{Param(Source{{12, 34}}, "common_name", ty.f32())}, ty.void_(),
         utils::Vector{
             Block(Decl(Let(Source{{56, 78}}, "common_name", Expr(1_i)))),
         });

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverFunctionValidationTest, VoidFunctionEndWithoutReturnStatement_Pass) {
    // fn func { var a:i32 = 2i; }
    auto* var = Var("a", ty.i32(), Expr(2_i));

    Func(Source{{12, 34}}, "func", utils::Empty, ty.void_(),
         utils::Vector{
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
    Func("func", utils::Empty, ty.i32(),
         utils::Vector{
             Decl(var),
             Return(Source{{12, 34}}, Expr("func")),
         });

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverFunctionValidationTest, FunctionNameSameAsFunctionScopeVariableName_Pass) {
    // fn a() -> void { var b:i32 = 0i; }
    // fn b() -> i32 { return 2; }

    auto* var = Var("b", ty.i32(), Expr(0_i));
    Func("a", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(var),
         });

    Func(Source{{12, 34}}, "b", utils::Empty, ty.i32(),
         utils::Vector{
             Return(2_i),
         });

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

    Func("func", utils::Empty, ty.void_(), utils::Vector{decl_a, ret, assign_a});

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

    Func("func", utils::Empty, ty.void_(),
         utils::Vector{decl_a, Block(Block(Block(ret))), assign_a});

    ASSERT_TRUE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 warning: code is unreachable");
    EXPECT_TRUE(Sem().Get(decl_a)->IsReachable());
    EXPECT_TRUE(Sem().Get(ret)->IsReachable());
    EXPECT_FALSE(Sem().Get(assign_a)->IsReachable());
}

TEST_F(ResolverFunctionValidationTest, UnreachableCode_discard_nowarning) {
    // fn func() -> {
    //  var a : i32;
    //  discard;
    //  a = 2i;
    //}

    auto* decl_a = Decl(Var("a", ty.i32()));
    auto* discard = Discard();
    auto* assign_a = Assign(Source{{12, 34}}, "a", 2_i);

    Func("func", utils::Empty, ty.void_(), utils::Vector{decl_a, discard, assign_a});

    ASSERT_TRUE(r()->Resolve());
    EXPECT_TRUE(Sem().Get(decl_a)->IsReachable());
    EXPECT_TRUE(Sem().Get(discard)->IsReachable());
    EXPECT_TRUE(Sem().Get(assign_a)->IsReachable());
}

TEST_F(ResolverFunctionValidationTest, DiscardCalledDirectlyFromVertexEntryPoint) {
    // @vertex() fn func() -> @position(0) vec4<f32> { discard; return; }
    Func(Source{{1, 2}}, "func", utils::Empty, ty.vec4<f32>(),
         utils::Vector{
             Discard(Source{{12, 34}}),
             Return(Call(ty.vec4<f32>())),
         },
         utils::Vector{Stage(ast::PipelineStage::kVertex)},
         utils::Vector{Builtin(builtin::BuiltinValue::kPosition)});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: discard statement cannot be used in vertex pipeline stage");
}

TEST_F(ResolverFunctionValidationTest, DiscardCalledIndirectlyFromComputeEntryPoint) {
    // fn f0 { discard; }
    // fn f1 { f0(); }
    // fn f2 { f1(); }
    // @compute @workgroup_size(1) fn main { return f2(); }

    Func(Source{{1, 2}}, "f0", utils::Empty, ty.void_(),
         utils::Vector{
             Discard(Source{{12, 34}}),
         });

    Func(Source{{3, 4}}, "f1", utils::Empty, ty.void_(),
         utils::Vector{
             CallStmt(Call("f0")),
         });

    Func(Source{{5, 6}}, "f2", utils::Empty, ty.void_(),
         utils::Vector{
             CallStmt(Call("f1")),
         });

    Func(Source{{7, 8}}, "main", utils::Empty, ty.void_(),
         utils::Vector{
             CallStmt(Call("f2")),
         },
         utils::Vector{
             Stage(ast::PipelineStage::kCompute),
             WorkgroupSize(1_i),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: discard statement cannot be used in compute pipeline stage
1:2 note: called by function 'f0'
3:4 note: called by function 'f1'
5:6 note: called by function 'f2'
7:8 note: called by entry point 'main')");
}

TEST_F(ResolverFunctionValidationTest, FunctionEndWithoutReturnStatement_Fail) {
    // fn func() -> int { var a:i32 = 2i; }

    auto* var = Var("a", ty.i32(), Expr(2_i));

    Func(Source{{12, 34}}, "func", utils::Empty, ty.i32(),
         utils::Vector{
             Decl(var),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: missing return at end of function");
}

TEST_F(ResolverFunctionValidationTest, VoidFunctionEndWithoutReturnStatementEmptyBody_Pass) {
    // fn func {}

    Func(Source{{12, 34}}, "func", utils::Empty, ty.void_(), utils::Empty);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverFunctionValidationTest, FunctionEndWithoutReturnStatementEmptyBody_Fail) {
    // fn func() -> int {}

    Func(Source{{12, 34}}, "func", utils::Empty, ty.i32(), utils::Empty);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: missing return at end of function");
}

TEST_F(ResolverFunctionValidationTest, FunctionTypeMustMatchReturnStatementType_Pass) {
    // fn func { return; }

    Func("func", utils::Empty, ty.void_(),
         utils::Vector{
             Return(),
         });

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverFunctionValidationTest, VoidFunctionReturnsAInt) {
    // fn func { return 2; }
    Func("func", utils::Empty, ty.void_(),
         utils::Vector{
             Return(Source{{12, 34}}, Expr(2_a)),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: return statement type must match its function return type, returned "
              "'abstract-int', expected 'void'");
}

TEST_F(ResolverFunctionValidationTest, VoidFunctionReturnsAFloat) {
    // fn func { return 2.0; }
    Func("func", utils::Empty, ty.void_(),
         utils::Vector{
             Return(Source{{12, 34}}, Expr(2.0_a)),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: return statement type must match its function return type, returned "
              "'abstract-float', expected 'void'");
}

TEST_F(ResolverFunctionValidationTest, VoidFunctionReturnsI32) {
    // fn func { return 2i; }
    Func("func", utils::Empty, ty.void_(),
         utils::Vector{
             Return(Source{{12, 34}}, Expr(2_i)),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: return statement type must match its function return type, returned "
              "'i32', expected 'void'");
}

TEST_F(ResolverFunctionValidationTest, FunctionTypeMustMatchReturnStatementType_void_fail) {
    // fn v { return; }
    // fn func { return v(); }
    Func("v", utils::Empty, ty.void_(),
         utils::Vector{
             Return(),
         });
    Func("func", utils::Empty, ty.void_(),
         utils::Vector{
             Return(Call(Source{{12, 34}}, "v")),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: function 'v' does not return a value");
}

TEST_F(ResolverFunctionValidationTest, FunctionTypeMustMatchReturnStatementTypeMissing_fail) {
    // fn func() -> f32 { return; }
    Func("func", utils::Empty, ty.f32(),
         utils::Vector{
             Return(Source{{12, 34}}, nullptr),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: return statement type must match its function return type, returned "
              "'void', expected 'f32'");
}

TEST_F(ResolverFunctionValidationTest, FunctionTypeMustMatchReturnStatementTypeF32_pass) {
    // fn func() -> f32 { return 2.0; }
    Func("func", utils::Empty, ty.f32(),
         utils::Vector{
             Return(Source{{12, 34}}, Expr(2_f)),
         });

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverFunctionValidationTest, FunctionTypeMustMatchReturnStatementTypeF32_fail) {
    // fn func() -> f32 { return 2i; }
    Func("func", utils::Empty, ty.f32(),
         utils::Vector{
             Return(Source{{12, 34}}, Expr(2_i)),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: return statement type must match its function return type, returned "
              "'i32', expected 'f32'");
}

TEST_F(ResolverFunctionValidationTest, FunctionTypeMustMatchReturnStatementTypeF32Alias_pass) {
    // type myf32 = f32;
    // fn func() -> myf32 { return 2.0; }
    auto* myf32 = Alias("myf32", ty.f32());
    Func("func", utils::Empty, ty.Of(myf32),
         utils::Vector{
             Return(Source{{12, 34}}, Expr(2_f)),
         });

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverFunctionValidationTest, FunctionTypeMustMatchReturnStatementTypeF32Alias_fail) {
    // type myf32 = f32;
    // fn func() -> myf32 { return 2u; }
    auto* myf32 = Alias("myf32", ty.f32());
    Func("func", utils::Empty, ty.Of(myf32),
         utils::Vector{
             Return(Source{{12, 34}}, Expr(2_u)),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: return statement type must match its function return type, returned "
              "'u32', expected 'f32'");
}

TEST_F(ResolverFunctionValidationTest, CannotCallEntryPoint) {
    // @compute @workgroup_size(1) fn entrypoint() {}
    // fn func() { return entrypoint(); }
    Func("entrypoint", utils::Empty, ty.void_(), utils::Empty,
         utils::Vector{
             Stage(ast::PipelineStage::kCompute),
             WorkgroupSize(1_i),
         });

    Func("func", utils::Empty, ty.void_(),
         utils::Vector{
             CallStmt(Call(Source{{12, 34}}, "entrypoint")),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: entry point functions cannot be the target of a function call)");
}

TEST_F(ResolverFunctionValidationTest, CannotCallFunctionAtModuleScope) {
    // fn F() -> i32 { return 1; }
    // var x = F();
    Func("F", utils::Empty, ty.i32(),
         utils::Vector{
             Return(1_i),
         });
    GlobalVar("x", Call(Source{{12, 34}}, "F"), builtin::AddressSpace::kPrivate);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: functions cannot be called at module-scope)");
}

TEST_F(ResolverFunctionValidationTest, PipelineStage_MustBeUnique_Fail) {
    // @fragment
    // @vertex
    // fn main() { return; }
    Func(Source{{12, 34}}, "main", utils::Empty, ty.void_(),
         utils::Vector{
             Return(),
         },
         utils::Vector{
             Stage(Source{{12, 34}}, ast::PipelineStage::kVertex),
             Stage(Source{{56, 78}}, ast::PipelineStage::kFragment),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(56:78 error: duplicate stage attribute
12:34 note: first attribute declared here)");
}

TEST_F(ResolverFunctionValidationTest, NoPipelineEntryPoints) {
    Func("vtx_func", utils::Empty, ty.void_(),
         utils::Vector{
             Return(),
         });

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverFunctionValidationTest, FunctionVarInitWithParam) {
    // fn foo(bar : f32){
    //   var baz : f32 = bar;
    // }

    auto* bar = Param("bar", ty.f32());
    auto* baz = Var("baz", ty.f32(), Expr("bar"));

    Func("foo", utils::Vector{bar}, ty.void_(),
         utils::Vector{
             Decl(baz),
         });

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverFunctionValidationTest, FunctionConstInitWithParam) {
    // fn foo(bar : f32){
    //   let baz : f32 = bar;
    // }

    auto* bar = Param("bar", ty.f32());
    auto* baz = Let("baz", ty.f32(), Expr("bar"));

    Func("foo", utils::Vector{bar}, ty.void_(),
         utils::Vector{
             Decl(baz),
         });

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverFunctionValidationTest, WorkgroupSize_GoodType_ConstU32) {
    // const x = 4u;
    // const y = 8u;
    // @compute @workgroup_size(x, y, 16u)
    // fn main() {}
    auto* x = GlobalConst("x", ty.u32(), Expr(4_u));
    auto* y = GlobalConst("y", ty.u32(), Expr(8_u));
    auto* func = Func("main", utils::Empty, ty.void_(), utils::Empty,
                      utils::Vector{
                          Stage(ast::PipelineStage::kCompute),
                          WorkgroupSize("x", "y", 16_u),
                      });

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem_func = Sem().Get(func);
    auto* sem_x = Sem().Get<sem::GlobalVariable>(x);
    auto* sem_y = Sem().Get<sem::GlobalVariable>(y);

    ASSERT_NE(sem_func, nullptr);
    ASSERT_NE(sem_x, nullptr);
    ASSERT_NE(sem_y, nullptr);

    EXPECT_EQ(sem_func->WorkgroupSize(), (sem::WorkgroupSize{4u, 8u, 16u}));

    EXPECT_TRUE(sem_func->DirectlyReferencedGlobals().Contains(sem_x));
    EXPECT_TRUE(sem_func->DirectlyReferencedGlobals().Contains(sem_y));
}

TEST_F(ResolverFunctionValidationTest, WorkgroupSize_Cast) {
    // @compute @workgroup_size(i32(5))
    // fn main() {}
    auto* func = Func("main", utils::Empty, ty.void_(), utils::Empty,
                      utils::Vector{
                          Stage(ast::PipelineStage::kCompute),
                          WorkgroupSize(Call(Source{{12, 34}}, ty.i32(), 5_a)),
                      });

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem_func = Sem().Get(func);

    ASSERT_NE(sem_func, nullptr);
    EXPECT_EQ(sem_func->WorkgroupSize(), (sem::WorkgroupSize{5u, 1u, 1u}));
}

TEST_F(ResolverFunctionValidationTest, WorkgroupSize_GoodType_I32) {
    // @compute @workgroup_size(1i, 2i, 3i)
    // fn main() {}

    Func("main", utils::Empty, ty.void_(), utils::Empty,
         utils::Vector{
             Stage(ast::PipelineStage::kCompute),
             WorkgroupSize(Source{{12, 34}}, 1_i, 2_i, 3_i),
         });

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverFunctionValidationTest, WorkgroupSize_GoodType_U32) {
    // @compute @workgroup_size(1u, 2u, 3u)
    // fn main() {}

    Func("main", utils::Empty, ty.void_(), utils::Empty,
         utils::Vector{
             Stage(ast::PipelineStage::kCompute),
             WorkgroupSize(Source{{12, 34}}, 1_u, 2_u, 3_u),
         });

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverFunctionValidationTest, WorkgroupSize_GoodType_I32_AInt) {
    // @compute @workgroup_size(1, 2i, 3)
    // fn main() {}

    Func("main", utils::Empty, ty.void_(), utils::Empty,
         utils::Vector{
             Stage(ast::PipelineStage::kCompute),
             WorkgroupSize(Source{{12, 34}}, 1_a, 2_i, 3_a),
         });

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverFunctionValidationTest, WorkgroupSize_GoodType_U32_AInt) {
    // @compute @workgroup_size(1u, 2, 3u)
    // fn main() {}

    Func("main", utils::Empty, ty.void_(), utils::Empty,
         utils::Vector{
             Stage(ast::PipelineStage::kCompute),
             WorkgroupSize(Source{{12, 34}}, 1_u, 2_a, 3_u),
         });

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverFunctionValidationTest, WorkgroupSize_Expr) {
    // @compute @workgroup_size(1 + 2)
    // fn main() {}

    Func("main", utils::Empty, ty.void_(), utils::Empty,
         utils::Vector{
             Stage(ast::PipelineStage::kCompute),
             WorkgroupSize(Source{{12, 34}}, Add(1_u, 2_u)),
         });

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverFunctionValidationTest, WorkgroupSize_MismatchType_U32) {
    // @compute @workgroup_size(1u, 2, 3_i)
    // fn main() {}

    Func("main", utils::Empty, ty.void_(), utils::Empty,
         utils::Vector{
             Stage(ast::PipelineStage::kCompute),
             WorkgroupSize(Source{{12, 34}}, 1_u, 2_a, 3_i),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: workgroup_size arguments must be of the same type, either i32 or u32");
}

TEST_F(ResolverFunctionValidationTest, WorkgroupSize_MismatchType_I32) {
    // @compute @workgroup_size(1_i, 2u, 3)
    // fn main() {}

    Func("main", utils::Empty, ty.void_(), utils::Empty,
         utils::Vector{
             Stage(ast::PipelineStage::kCompute),
             WorkgroupSize(Source{{12, 34}}, 1_i, 2_u, 3_a),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: workgroup_size arguments must be of the same type, either i32 or u32");
}

TEST_F(ResolverFunctionValidationTest, WorkgroupSize_Const_TypeMismatch) {
    // const x = 64u;
    // @compute @workgroup_size(1i, x)
    // fn main() {}
    GlobalConst("x", ty.u32(), Expr(64_u));
    Func("main", utils::Empty, ty.void_(), utils::Empty,
         utils::Vector{
             Stage(ast::PipelineStage::kCompute),
             WorkgroupSize(Source{{12, 34}}, 1_i, "x"),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: workgroup_size arguments must be of the same type, either i32 or u32");
}

TEST_F(ResolverFunctionValidationTest, WorkgroupSize_Const_TypeMismatch2) {
    // const x = 64u;
    // const y = 32i;
    // @compute @workgroup_size(x, y)
    // fn main() {}
    GlobalConst("x", ty.u32(), Expr(64_u));
    GlobalConst("y", ty.i32(), Expr(32_i));
    Func("main", utils::Empty, ty.void_(), utils::Empty,
         utils::Vector{
             Stage(ast::PipelineStage::kCompute),
             WorkgroupSize(Source{{12, 34}}, "x", "y"),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: workgroup_size arguments must be of the same type, either i32 or u32");
}

TEST_F(ResolverFunctionValidationTest, WorkgroupSize_Mismatch_ConstU32) {
    // const x = 4u;
    // const x = 8u;
    // @compute @workgroup_size(x, y, 16i)
    // fn main() {}
    GlobalConst("x", ty.u32(), Expr(4_u));
    GlobalConst("y", ty.u32(), Expr(8_u));
    Func("main", utils::Empty, ty.void_(), utils::Empty,
         utils::Vector{
             Stage(ast::PipelineStage::kCompute),
             WorkgroupSize(Source{{12, 34}}, "x", "y", 16_i),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: workgroup_size arguments must be of the same type, either i32 or u32");
}

TEST_F(ResolverFunctionValidationTest, WorkgroupSize_Literal_BadType) {
    // @compute @workgroup_size(64.0)
    // fn main() {}

    Func("main", utils::Empty, ty.void_(), utils::Empty,
         utils::Vector{
             Stage(ast::PipelineStage::kCompute),
             WorkgroupSize(Expr(Source{{12, 34}}, 64_f)),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        "12:34 error: workgroup_size argument must be a constant or override-expression of type "
        "abstract-integer, i32 or u32");
}

TEST_F(ResolverFunctionValidationTest, WorkgroupSize_Literal_Negative) {
    // @compute @workgroup_size(-2i)
    // fn main() {}

    Func("main", utils::Empty, ty.void_(), utils::Empty,
         utils::Vector{
             Stage(ast::PipelineStage::kCompute),
             WorkgroupSize(Expr(Source{{12, 34}}, -2_i)),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: workgroup_size argument must be at least 1");
}

TEST_F(ResolverFunctionValidationTest, WorkgroupSize_Literal_Zero) {
    // @compute @workgroup_size(0i)
    // fn main() {}

    Func("main", utils::Empty, ty.void_(), utils::Empty,
         utils::Vector{
             Stage(ast::PipelineStage::kCompute),
             WorkgroupSize(Expr(Source{{12, 34}}, 0_i)),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: workgroup_size argument must be at least 1");
}

TEST_F(ResolverFunctionValidationTest, WorkgroupSize_Const_BadType) {
    // const x = 64.0;
    // @compute @workgroup_size(x)
    // fn main() {}
    GlobalConst("x", ty.f32(), Expr(64_f));
    Func("main", utils::Empty, ty.void_(), utils::Empty,
         utils::Vector{
             Stage(ast::PipelineStage::kCompute),
             WorkgroupSize(Expr(Source{{12, 34}}, "x")),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        "12:34 error: workgroup_size argument must be a constant or override-expression of type "
        "abstract-integer, i32 or u32");
}

TEST_F(ResolverFunctionValidationTest, WorkgroupSize_Const_Negative) {
    // const x = -2i;
    // @compute @workgroup_size(x)
    // fn main() {}
    GlobalConst("x", ty.i32(), Expr(-2_i));
    Func("main", utils::Empty, ty.void_(), utils::Empty,
         utils::Vector{
             Stage(ast::PipelineStage::kCompute),
             WorkgroupSize(Expr(Source{{12, 34}}, "x")),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: workgroup_size argument must be at least 1");
}

TEST_F(ResolverFunctionValidationTest, WorkgroupSize_Const_Zero) {
    // const x = 0i;
    // @compute @workgroup_size(x)
    // fn main() {}
    GlobalConst("x", ty.i32(), Expr(0_i));
    Func("main", utils::Empty, ty.void_(), utils::Empty,
         utils::Vector{
             Stage(ast::PipelineStage::kCompute),
             WorkgroupSize(Expr(Source{{12, 34}}, "x")),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: workgroup_size argument must be at least 1");
}

TEST_F(ResolverFunctionValidationTest, WorkgroupSize_Const_NestedZeroValueInitializer) {
    // const x = i32(i32(i32()));
    // @compute @workgroup_size(x)
    // fn main() {}
    GlobalConst("x", ty.i32(), Call<i32>(Call<i32>(Call<i32>())));
    Func("main", utils::Empty, ty.void_(), utils::Empty,
         utils::Vector{
             Stage(ast::PipelineStage::kCompute),
             WorkgroupSize(Expr(Source{{12, 34}}, "x")),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: workgroup_size argument must be at least 1");
}

TEST_F(ResolverFunctionValidationTest, WorkgroupSize_OverflowsU32_0x10000_0x100_0x100) {
    // @compute @workgroup_size(0x10000, 0x100, 0x100)
    // fn main() {}
    Func("main", utils::Empty, ty.void_(), utils::Empty,
         utils::Vector{
             Stage(ast::PipelineStage::kCompute),
             WorkgroupSize(0x10000_a, 0x100_a, Expr(Source{{12, 34}}, 0x100_a)),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: total workgroup grid size cannot exceed 0xffffffff");
}

TEST_F(ResolverFunctionValidationTest, WorkgroupSize_OverflowsU32_0x10000_0x10000) {
    // @compute @workgroup_size(0x10000, 0x10000)
    // fn main() {}
    Func("main", utils::Empty, ty.void_(), utils::Empty,
         utils::Vector{
             Stage(ast::PipelineStage::kCompute),
             WorkgroupSize(0x10000_a, Expr(Source{{12, 34}}, 0x10000_a)),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: total workgroup grid size cannot exceed 0xffffffff");
}

TEST_F(ResolverFunctionValidationTest, WorkgroupSize_OverflowsU32_0x10000_C_0x10000) {
    // const C = 1;
    // @compute @workgroup_size(0x10000, C, 0x10000)
    // fn main() {}
    GlobalConst("C", ty.u32(), Expr(1_a));
    Func("main", utils::Empty, ty.void_(), utils::Empty,
         utils::Vector{
             Stage(ast::PipelineStage::kCompute),
             WorkgroupSize(0x10000_a, "C", Expr(Source{{12, 34}}, 0x10000_a)),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: total workgroup grid size cannot exceed 0xffffffff");
}

TEST_F(ResolverFunctionValidationTest, WorkgroupSize_OverflowsU32_0x10000_C) {
    // const C = 0x10000;
    // @compute @workgroup_size(0x10000, C)
    // fn main() {}
    GlobalConst("C", ty.u32(), Expr(0x10000_a));
    Func("main", utils::Empty, ty.void_(), utils::Empty,
         utils::Vector{
             Stage(ast::PipelineStage::kCompute),
             WorkgroupSize(0x10000_a, Expr(Source{{12, 34}}, "C")),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: total workgroup grid size cannot exceed 0xffffffff");
}

TEST_F(ResolverFunctionValidationTest, WorkgroupSize_OverflowsU32_0x10000_O_0x10000) {
    // override O = 0;
    // @compute @workgroup_size(0x10000, O, 0x10000)
    // fn main() {}
    Override("O", ty.u32(), Expr(0_a));
    Func("main", utils::Empty, ty.void_(), utils::Empty,
         utils::Vector{
             Stage(ast::PipelineStage::kCompute),
             WorkgroupSize(0x10000_a, "O", Expr(Source{{12, 34}}, 0x10000_a)),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: total workgroup grid size cannot exceed 0xffffffff");
}

TEST_F(ResolverFunctionValidationTest, WorkgroupSize_NonConst) {
    // var<private> x = 64i;
    // @compute @workgroup_size(x)
    // fn main() {}
    GlobalVar("x", ty.i32(), builtin::AddressSpace::kPrivate, Expr(64_i));
    Func("main", utils::Empty, ty.void_(), utils::Empty,
         utils::Vector{
             Stage(ast::PipelineStage::kCompute),
             WorkgroupSize(Expr(Source{{12, 34}}, "x")),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: workgroup_size argument must be a constant or override-expression of "
              "type abstract-integer, i32 or u32");
}

TEST_F(ResolverFunctionValidationTest, WorkgroupSize_InvalidExpr_x) {
    // @compute @workgroup_size(1 << 2 + 4)
    // fn main() {}
    GlobalVar("x", ty.i32(), builtin::AddressSpace::kPrivate, Expr(0_i));
    Func("main", utils::Empty, ty.void_(), utils::Empty,
         utils::Vector{
             Stage(ast::PipelineStage::kCompute),
             WorkgroupSize(Call(Source{{12, 34}}, ty.i32(), "x")),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: workgroup_size argument must be a constant or override-expression of "
              "type abstract-integer, i32 or u32");
}

TEST_F(ResolverFunctionValidationTest, WorkgroupSize_InvalidExpr_y) {
    // @compute @workgroup_size(1, 1 << 2 + 4)
    // fn main() {}
    GlobalVar("x", ty.i32(), builtin::AddressSpace::kPrivate, Expr(0_i));
    Func("main", utils::Empty, ty.void_(), utils::Empty,
         utils::Vector{
             Stage(ast::PipelineStage::kCompute),
             WorkgroupSize(Call(Source{{12, 34}}, ty.i32(), "x")),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: workgroup_size argument must be a constant or override-expression of "
              "type abstract-integer, i32 or u32");
}

TEST_F(ResolverFunctionValidationTest, WorkgroupSize_InvalidExpr_z) {
    // @compute @workgroup_size(1, 1, 1 << 2 + 4)
    // fn main() {}
    GlobalVar("x", ty.i32(), builtin::AddressSpace::kPrivate, Expr(0_i));
    Func("main", utils::Empty, ty.void_(), utils::Empty,
         utils::Vector{
             Stage(ast::PipelineStage::kCompute),
             WorkgroupSize(Call(Source{{12, 34}}, ty.i32(), "x")),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: workgroup_size argument must be a constant or override-expression of "
              "type abstract-integer, i32 or u32");
}

TEST_F(ResolverFunctionValidationTest, ReturnIsConstructible_NonPlain) {
    auto ret_type = ty.pointer(Source{{12, 34}}, ty.i32(), builtin::AddressSpace::kFunction);
    Func("f", utils::Empty, ret_type, utils::Empty);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: function return type must be a constructible type");
}

TEST_F(ResolverFunctionValidationTest, ReturnIsConstructible_AtomicInt) {
    auto ret_type = ty.atomic(Source{{12, 34}}, ty.i32());
    Func("f", utils::Empty, ret_type, utils::Empty);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: function return type must be a constructible type");
}

TEST_F(ResolverFunctionValidationTest, ReturnIsConstructible_ArrayOfAtomic) {
    auto ret_type = ty.array(Source{{12, 34}}, ty.atomic(ty.i32()), 10_u);
    Func("f", utils::Empty, ret_type, utils::Empty);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: function return type must be a constructible type");
}

TEST_F(ResolverFunctionValidationTest, ReturnIsConstructible_StructOfAtomic) {
    Structure("S", utils::Vector{
                       Member("m", ty.atomic(ty.i32())),
                   });
    auto ret_type = ty(Source{{12, 34}}, "S");
    Func("f", utils::Empty, ret_type, utils::Empty);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: function return type must be a constructible type");
}

TEST_F(ResolverFunctionValidationTest, ReturnIsConstructible_RuntimeArray) {
    auto ret_type = ty.array(Source{{12, 34}}, ty.i32());
    Func("f", utils::Empty, ret_type, utils::Empty);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: function return type must be a constructible type");
}

TEST_F(ResolverFunctionValidationTest, ParameterStoreType_NonAtomicFree) {
    Structure("S", utils::Vector{
                       Member("m", ty.atomic(ty.i32())),
                   });
    auto ret_type = ty(Source{{12, 34}}, "S");
    auto* bar = Param("bar", ret_type);
    Func("f", utils::Vector{bar}, ty.void_(), utils::Empty);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: type of function parameter must be constructible");
}

TEST_F(ResolverFunctionValidationTest, ParameterStoreType_AtomicFree) {
    Structure("S", utils::Vector{
                       Member("m", ty.i32()),
                   });
    auto ret_type = ty(Source{{12, 34}}, "S");
    auto* bar = Param(Source{{12, 34}}, "bar", ret_type);
    Func("f", utils::Vector{bar}, ty.void_(), utils::Empty);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverFunctionValidationTest, ParametersAtLimit) {
    utils::Vector<const ast::Parameter*, 256> params;
    for (int i = 0; i < 255; i++) {
        params.Push(Param("param_" + std::to_string(i), ty.i32()));
    }
    Func(Source{{12, 34}}, "f", params, ty.void_(), utils::Empty);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverFunctionValidationTest, ParametersOverLimit) {
    utils::Vector<const ast::Parameter*, 256> params;
    for (int i = 0; i < 256; i++) {
        params.Push(Param("param_" + std::to_string(i), ty.i32()));
    }
    Func(Source{{12, 34}}, "f", params, ty.void_(), utils::Empty);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: function declares 256 parameters, maximum is 255");
}

TEST_F(ResolverFunctionValidationTest, ParameterVectorNoType) {
    // fn f(p : vec3) {}

    Func(Source{{12, 34}}, "f", utils::Vector{Param("p", ty.vec3<Infer>(Source{{12, 34}}))},
         ty.void_(), utils::Empty);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: expected '<' for 'vec3'");
}

TEST_F(ResolverFunctionValidationTest, ParameterMatrixNoType) {
    // fn f(p : mat3x3) {}

    Func(Source{{12, 34}}, "f", utils::Vector{Param("p", ty.mat3x3<Infer>(Source{{12, 34}}))},
         ty.void_(), utils::Empty);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: expected '<' for 'mat3x3'");
}

enum class Expectation {
    kAlwaysPass,
    kPassWithFullPtrParameterExtension,
    kAlwaysFail,
    kInvalid,
};
struct TestParams {
    builtin::AddressSpace address_space;
    Expectation expectation;
};

struct TestWithParams : ResolverTestWithParam<TestParams> {};

using ResolverFunctionParameterValidationTest = TestWithParams;
TEST_P(ResolverFunctionParameterValidationTest, AddressSpaceNoExtension) {
    auto& param = GetParam();
    auto ptr_type = ty("ptr", Ident(Source{{12, 34}}, param.address_space), ty.i32());
    auto* arg = Param(Source{{12, 34}}, "p", ptr_type);
    Func("f", utils::Vector{arg}, ty.void_(), utils::Empty);

    if (param.expectation == Expectation::kAlwaysPass) {
        ASSERT_TRUE(r()->Resolve()) << r()->error();
    } else {
        utils::StringStream ss;
        ss << param.address_space;
        EXPECT_FALSE(r()->Resolve());
        if (param.expectation == Expectation::kInvalid) {
            std::string err = R"(12:34 error: unresolved address space '${addr_space}'
12:34 note: Possible values: 'function', 'private', 'push_constant', 'storage', 'uniform', 'workgroup')";
            err = utils::ReplaceAll(err, "${addr_space}", utils::ToString(param.address_space));
            EXPECT_EQ(r()->error(), err);
        } else {
            EXPECT_EQ(r()->error(),
                      "12:34 error: function parameter of pointer type cannot be in '" +
                          utils::ToString(param.address_space) + "' address space");
        }
    }
}
TEST_P(ResolverFunctionParameterValidationTest, AddressSpaceWithExtension) {
    auto& param = GetParam();
    auto ptr_type = ty("ptr", Ident(Source{{12, 34}}, param.address_space), ty.i32());
    auto* arg = Param(Source{{12, 34}}, "p", ptr_type);
    Enable(builtin::Extension::kChromiumExperimentalFullPtrParameters);
    Func("f", utils::Vector{arg}, ty.void_(), utils::Empty);

    if (param.expectation == Expectation::kAlwaysPass ||
        param.expectation == Expectation::kPassWithFullPtrParameterExtension) {
        ASSERT_TRUE(r()->Resolve()) << r()->error();
    } else {
        EXPECT_FALSE(r()->Resolve());
        if (param.expectation == Expectation::kInvalid) {
            std::string err = R"(12:34 error: unresolved address space '${addr_space}'
12:34 note: Possible values: 'function', 'private', 'push_constant', 'storage', 'uniform', 'workgroup')";
            err = utils::ReplaceAll(err, "${addr_space}", utils::ToString(param.address_space));
            EXPECT_EQ(r()->error(), err);
        } else {
            EXPECT_EQ(r()->error(),
                      "12:34 error: function parameter of pointer type cannot be in '" +
                          utils::ToString(param.address_space) + "' address space");
        }
    }
}
INSTANTIATE_TEST_SUITE_P(
    ResolverTest,
    ResolverFunctionParameterValidationTest,
    testing::Values(TestParams{builtin::AddressSpace::kUndefined, Expectation::kInvalid},
                    TestParams{builtin::AddressSpace::kIn, Expectation::kAlwaysFail},
                    TestParams{builtin::AddressSpace::kOut, Expectation::kAlwaysFail},
                    TestParams{builtin::AddressSpace::kUniform,
                               Expectation::kPassWithFullPtrParameterExtension},
                    TestParams{builtin::AddressSpace::kWorkgroup,
                               Expectation::kPassWithFullPtrParameterExtension},
                    TestParams{builtin::AddressSpace::kHandle, Expectation::kInvalid},
                    TestParams{builtin::AddressSpace::kStorage,
                               Expectation::kPassWithFullPtrParameterExtension},
                    TestParams{builtin::AddressSpace::kPrivate, Expectation::kAlwaysPass},
                    TestParams{builtin::AddressSpace::kFunction, Expectation::kAlwaysPass}));

}  // namespace
}  // namespace tint::resolver
