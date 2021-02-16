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

#include "gtest/gtest.h"
#include "src/ast/assignment_statement.h"
#include "src/ast/call_statement.h"
#include "src/ast/stage_decoration.h"
#include "src/ast/variable.h"
#include "src/ast/variable_decl_statement.h"
#include "src/type/f32_type.h"
#include "src/writer/wgsl/generator_impl.h"
#include "src/writer/wgsl/test_helper.h"

namespace tint {
namespace writer {
namespace wgsl {
namespace {

using WgslGeneratorImplTest = TestHelper;

TEST_F(WgslGeneratorImplTest, Emit_EntryPoint_UnusedFunction) {
  Func("func_unused", ast::VariableList{}, ty.void_(), ast::StatementList{},
       ast::FunctionDecorationList{});

  Func("func_used", ast::VariableList{}, ty.void_(), ast::StatementList{},
       ast::FunctionDecorationList{});

  auto* call_func = Call("func_used");

  Func("main", ast::VariableList{}, ty.void_(),
       ast::StatementList{
           create<ast::CallStatement>(call_func),
       },
       ast::FunctionDecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kCompute),
       });

  GeneratorImpl& gen = Build();

  gen.increment_indent();

  ASSERT_TRUE(
      gen.GenerateEntryPoint(tint::ast::PipelineStage::kCompute, "main"))
      << gen.error();
  EXPECT_EQ(gen.result(), R"(  fn func_used() -> void {
  }

  [[stage(compute)]]
  fn main() -> void {
    func_used();
  }
)");
}

TEST_F(WgslGeneratorImplTest, Emit_EntryPoint_UnusedVariable) {
  auto* global_unused =
      Global("global_unused", ty.f32(), ast::StorageClass::kInput);
  create<ast::VariableDeclStatement>(global_unused);

  auto* global_used =
      Global("global_used", ty.f32(), ast::StorageClass::kInput);
  create<ast::VariableDeclStatement>(global_used);

  Func("main", ast::VariableList{}, ty.void_(),
       ast::StatementList{
           create<ast::AssignmentStatement>(Expr("global_used"), Expr(1.f)),
       },
       ast::FunctionDecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kCompute),
       });

  GeneratorImpl& gen = Build();

  gen.increment_indent();

  ASSERT_TRUE(
      gen.GenerateEntryPoint(tint::ast::PipelineStage::kCompute, "main"))
      << gen.error();
  EXPECT_EQ(gen.result(), R"(  var<in> global_used : f32;

  [[stage(compute)]]
  fn main() -> void {
    global_used = 1.0;
  }
)");
}

TEST_F(WgslGeneratorImplTest, Emit_EntryPoint_GlobalsInterleaved) {
  auto* global0 = Global("a0", ty.f32(), ast::StorageClass::kInput);
  create<ast::VariableDeclStatement>(global0);

  auto* str0 = create<ast::Struct>(ast::StructMemberList{Member("a", ty.i32())},
                                   ast::StructDecorationList{});
  auto* s0 = ty.struct_("S0", str0);
  AST().AddConstructedType(s0);

  Func("func", ast::VariableList{}, ty.f32(),
       ast::StatementList{
           create<ast::ReturnStatement>(Expr("a0")),
       },
       ast::FunctionDecorationList{});

  auto* global1 = Global("a1", ty.f32(), ast::StorageClass::kOutput);
  create<ast::VariableDeclStatement>(global1);

  auto* str1 = create<ast::Struct>(ast::StructMemberList{Member("a", ty.i32())},
                                   ast::StructDecorationList{});
  auto* s1 = ty.struct_("S1", str1);
  AST().AddConstructedType(s1);

  auto* call_func = Call("func");

  Func("main", ast::VariableList{}, ty.void_(),
       ast::StatementList{
           create<ast::VariableDeclStatement>(
               Var("s0", s0, ast::StorageClass::kFunction)),
           create<ast::VariableDeclStatement>(
               Var("s1", s1, ast::StorageClass::kFunction)),
           create<ast::AssignmentStatement>(Expr("a1"), Expr(call_func)),
       },
       ast::FunctionDecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kCompute),
       });

  GeneratorImpl& gen = Build();

  gen.increment_indent();

  ASSERT_TRUE(
      gen.GenerateEntryPoint(tint::ast::PipelineStage::kCompute, "main"))
      << gen.error();
  EXPECT_EQ(gen.result(), R"(  var<in> a0 : f32;

  struct S0 {
    a : i32;
  };

  fn func() -> f32 {
    return a0;
  }

  var<out> a1 : f32;

  struct S1 {
    a : i32;
  };

  [[stage(compute)]]
  fn main() -> void {
    var s0 : S0;
    var s1 : S1;
    a1 = func();
  }
)");
}

}  // namespace
}  // namespace wgsl
}  // namespace writer
}  // namespace tint
