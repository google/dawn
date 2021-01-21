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

#include "gtest/gtest.h"
#include "src/ast/discard_statement.h"
#include "src/ast/function.h"
#include "src/ast/member_accessor_expression.h"
#include "src/ast/module.h"
#include "src/ast/pipeline_stage.h"
#include "src/ast/return_statement.h"
#include "src/ast/stage_decoration.h"
#include "src/ast/struct_block_decoration.h"
#include "src/ast/struct_member_offset_decoration.h"
#include "src/ast/variable.h"
#include "src/ast/variable_decl_statement.h"
#include "src/ast/workgroup_decoration.h"
#include "src/type/access_control_type.h"
#include "src/type/f32_type.h"
#include "src/type/i32_type.h"
#include "src/type/void_type.h"
#include "src/type_determiner.h"
#include "src/writer/wgsl/generator_impl.h"
#include "src/writer/wgsl/test_helper.h"

namespace tint {
namespace writer {
namespace wgsl {
namespace {

using WgslGeneratorImplTest = TestHelper;

TEST_F(WgslGeneratorImplTest, Emit_Function) {
  auto* func = Func("my_func", ast::VariableList{}, ty.void_,
                    ast::StatementList{
                        create<ast::DiscardStatement>(),
                        create<ast::ReturnStatement>(),
                    },
                    ast::FunctionDecorationList{});

  GeneratorImpl& gen = Build();

  gen.increment_indent();

  ASSERT_TRUE(gen.EmitFunction(func));
  EXPECT_EQ(gen.result(), R"(  fn my_func() -> void {
    discard;
    return;
  }
)");
}

TEST_F(WgslGeneratorImplTest, Emit_Function_WithParams) {
  auto* func =
      Func("my_func",
           ast::VariableList{Var("a", ast::StorageClass::kNone, ty.f32),
                             Var("b", ast::StorageClass::kNone, ty.i32)},
           ty.void_,
           ast::StatementList{
               create<ast::DiscardStatement>(),
               create<ast::ReturnStatement>(),
           },
           ast::FunctionDecorationList{});

  GeneratorImpl& gen = Build();

  gen.increment_indent();

  ASSERT_TRUE(gen.EmitFunction(func));
  EXPECT_EQ(gen.result(), R"(  fn my_func(a : f32, b : i32) -> void {
    discard;
    return;
  }
)");
}

TEST_F(WgslGeneratorImplTest, Emit_Function_WithDecoration_WorkgroupSize) {
  auto* func = Func("my_func", ast::VariableList{}, ty.void_,
                    ast::StatementList{
                        create<ast::DiscardStatement>(),
                        create<ast::ReturnStatement>(),
                    },
                    ast::FunctionDecorationList{
                        create<ast::WorkgroupDecoration>(2u, 4u, 6u),
                    });

  GeneratorImpl& gen = Build();

  gen.increment_indent();

  ASSERT_TRUE(gen.EmitFunction(func));
  EXPECT_EQ(gen.result(), R"(  [[workgroup_size(2, 4, 6)]]
  fn my_func() -> void {
    discard;
    return;
  }
)");
}

TEST_F(WgslGeneratorImplTest, Emit_Function_WithDecoration_Stage) {
  auto* func =
      Func("my_func", ast::VariableList{}, ty.void_,
           ast::StatementList{
               create<ast::DiscardStatement>(),
               create<ast::ReturnStatement>(),
           },
           ast::FunctionDecorationList{
               create<ast::StageDecoration>(ast::PipelineStage::kFragment),
           });

  GeneratorImpl& gen = Build();

  gen.increment_indent();

  ASSERT_TRUE(gen.EmitFunction(func));
  EXPECT_EQ(gen.result(), R"(  [[stage(fragment)]]
  fn my_func() -> void {
    discard;
    return;
  }
)");
}

TEST_F(WgslGeneratorImplTest, Emit_Function_WithDecoration_Multiple) {
  auto* func =
      Func("my_func", ast::VariableList{}, ty.void_,
           ast::StatementList{
               create<ast::DiscardStatement>(),
               create<ast::ReturnStatement>(),
           },
           ast::FunctionDecorationList{
               create<ast::StageDecoration>(ast::PipelineStage::kFragment),
               create<ast::WorkgroupDecoration>(2u, 4u, 6u),
           });

  GeneratorImpl& gen = Build();

  gen.increment_indent();

  ASSERT_TRUE(gen.EmitFunction(func));
  EXPECT_EQ(gen.result(), R"(  [[stage(fragment)]]
  [[workgroup_size(2, 4, 6)]]
  fn my_func() -> void {
    discard;
    return;
  }
)");
}

// https://crbug.com/tint/297
TEST_F(WgslGeneratorImplTest,
       Emit_Function_Multiple_EntryPoint_With_Same_ModuleVar) {
  // [[block]] struct Data {
  //   [[offset(0)]] d : f32;
  // };
  // [[binding(0), group(0)]] var<storage> data : Data;
  //
  // [[stage(compute)]]
  // fn a() -> void {
  //   return;
  // }
  //
  // [[stage(compute)]]
  // fn b() -> void {
  //   return;
  // }

  ast::StructDecorationList s_decos;
  s_decos.push_back(create<ast::StructBlockDecoration>());

  auto* str = create<ast::Struct>(
      ast::StructMemberList{Member("d", ty.f32, {MemberOffset(0)})}, s_decos);

  auto* s = ty.struct_("Data", str);
  type::AccessControl ac(ast::AccessControl::kReadWrite, s);

  auto* data_var = Var("data", ast::StorageClass::kStorage, &ac, nullptr,
                       ast::VariableDecorationList{
                           // decorations
                           create<ast::BindingDecoration>(0),
                           create<ast::GroupDecoration>(0),
                       });

  mod->AddConstructedType(s);

  td.RegisterVariableForTesting(data_var);
  mod->AddGlobalVariable(data_var);

  {
    auto* var =
        Var("v", ast::StorageClass::kFunction, ty.f32,
            create<ast::MemberAccessorExpression>(Expr("data"), Expr("d")),
            ast::VariableDecorationList{});

    auto* func =
        Func("a", ast::VariableList{}, ty.void_,
             ast::StatementList{
                 create<ast::VariableDeclStatement>(var),
                 create<ast::ReturnStatement>(),
             },
             ast::FunctionDecorationList{
                 create<ast::StageDecoration>(ast::PipelineStage::kCompute),
             });

    mod->Functions().Add(func);
  }

  {
    auto* var =
        Var("v", ast::StorageClass::kFunction, ty.f32,
            create<ast::MemberAccessorExpression>(Expr("data"), Expr("d")),
            ast::VariableDecorationList{});

    auto* func =
        Func("b", ast::VariableList{}, ty.void_,
             ast::StatementList{
                 create<ast::VariableDeclStatement>(var),
                 create<ast::ReturnStatement>(),
             },
             ast::FunctionDecorationList{
                 create<ast::StageDecoration>(ast::PipelineStage::kCompute),
             });

    mod->Functions().Add(func);
  }

  ASSERT_TRUE(td.Determine()) << td.error();

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_EQ(gen.result(), R"([[block]]
struct Data {
  [[offset(0)]]
  d : f32;
};

[[binding(0), group(0)]] var<storage> data : [[access(read_write)]]
Data;

[[stage(compute)]]
fn a() -> void {
  var v : f32 = data.d;
  return;
}

[[stage(compute)]]
fn b() -> void {
  var v : f32 = data.d;
  return;
}

)");
}

}  // namespace
}  // namespace wgsl
}  // namespace writer
}  // namespace tint
