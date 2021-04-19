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

#include "src/ast/call_statement.h"
#include "src/ast/stage_decoration.h"
#include "src/ast/variable_decl_statement.h"
#include "src/writer/wgsl/test_helper.h"

namespace tint {
namespace writer {
namespace wgsl {
namespace {

using WgslGeneratorImplTest = TestHelper;

TEST_F(WgslGeneratorImplTest, Emit_EntryPoint_UnusedFunction) {
  Func("func_unused", ast::VariableList{}, ty.void_(), ast::StatementList{},
       ast::DecorationList{});

  Func("func_used", ast::VariableList{}, ty.void_(), ast::StatementList{},
       ast::DecorationList{});

  auto* call_func = Call("func_used");

  Func("main", ast::VariableList{}, ty.void_(),
       ast::StatementList{
           create<ast::CallStatement>(call_func),
       },
       ast::DecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kCompute),
       });

  GeneratorImpl& gen = Build();

  gen.increment_indent();

  ASSERT_TRUE(
      gen.GenerateEntryPoint(tint::ast::PipelineStage::kCompute, "main"))
      << gen.error();
  EXPECT_EQ(gen.result(), R"(  fn func_used() {
  }

  [[stage(compute)]]
  fn main() {
    func_used();
  }
)");
}

TEST_F(WgslGeneratorImplTest, Emit_EntryPoint_UnusedVariable) {
  Global("global_unused", ty.f32(), ast::StorageClass::kPrivate);

  Global("global_used", ty.f32(), ast::StorageClass::kPrivate);

  Func("main", {}, ty.void_(),
       {
           create<ast::AssignmentStatement>(Expr("global_used"), Expr(1.f)),
       },
       {
           create<ast::StageDecoration>(ast::PipelineStage::kCompute),
       });

  GeneratorImpl& gen = Build();

  gen.increment_indent();

  ASSERT_TRUE(
      gen.GenerateEntryPoint(tint::ast::PipelineStage::kCompute, "main"))
      << gen.error();
  EXPECT_EQ(gen.result(), R"(  var<private> global_used : f32;

  [[stage(compute)]]
  fn main() {
    global_used = 1.0;
  }
)");
}

TEST_F(WgslGeneratorImplTest, Emit_EntryPoint_GlobalsInterleaved) {
  Global("a0", ty.f32(), ast::StorageClass::kPrivate);

  auto* s0 = Structure("S0", {Member("a", ty.i32())});

  Func("func", {}, ty.f32(),
       {
           Return("a0"),
       });

  Global("a1", ty.f32(), ast::StorageClass::kOutput);

  auto* s1 = Structure("S1", {Member("a", ty.i32())});

  Func("main", {}, ty.void_(),
       {
           Decl(Var("s0", s0, ast::StorageClass::kFunction)),
           Decl(Var("s1", s1, ast::StorageClass::kFunction)),
           create<ast::AssignmentStatement>(Expr("a1"), Call("func")),
       },
       {
           create<ast::StageDecoration>(ast::PipelineStage::kCompute),
       });

  GeneratorImpl& gen = Build();

  gen.increment_indent();

  ASSERT_TRUE(
      gen.GenerateEntryPoint(tint::ast::PipelineStage::kCompute, "main"))
      << gen.error();
  EXPECT_EQ(gen.result(), R"(  var<private> a0 : f32;

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
  fn main() {
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
