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
#include "src/ast/decorated_variable.h"
#include "src/ast/discard_statement.h"
#include "src/ast/function.h"
#include "src/ast/member_accessor_expression.h"
#include "src/ast/module.h"
#include "src/ast/pipeline_stage.h"
#include "src/ast/return_statement.h"
#include "src/ast/stage_decoration.h"
#include "src/ast/struct_block_decoration.h"
#include "src/ast/struct_member_offset_decoration.h"
#include "src/ast/type/access_control_type.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/i32_type.h"
#include "src/ast/type/void_type.h"
#include "src/ast/variable.h"
#include "src/ast/variable_decl_statement.h"
#include "src/ast/workgroup_decoration.h"
#include "src/type_determiner.h"
#include "src/writer/wgsl/generator_impl.h"
#include "src/writer/wgsl/test_helper.h"

namespace tint {
namespace writer {
namespace wgsl {
namespace {

using WgslGeneratorImplTest = TestHelper;

TEST_F(WgslGeneratorImplTest, Emit_Function) {
  auto* body = create<ast::BlockStatement>();
  body->append(create<ast::DiscardStatement>());
  body->append(create<ast::ReturnStatement>());

  ast::type::Void void_type;
  ast::Function func(Source{}, "my_func", {}, &void_type, body,
                     ast::FunctionDecorationList{});

  gen.increment_indent();

  ASSERT_TRUE(gen.EmitFunction(&func));
  EXPECT_EQ(gen.result(), R"(  fn my_func() -> void {
    discard;
    return;
  }
)");
}

TEST_F(WgslGeneratorImplTest, Emit_Function_WithParams) {
  auto* body = create<ast::BlockStatement>();
  body->append(create<ast::DiscardStatement>());
  body->append(create<ast::ReturnStatement>());

  ast::type::F32 f32;
  ast::type::I32 i32;
  ast::VariableList params;
  params.push_back(
      create<ast::Variable>(Source{}, "a", ast::StorageClass::kNone, &f32));
  params.push_back(
      create<ast::Variable>(Source{}, "b", ast::StorageClass::kNone, &i32));

  ast::type::Void void_type;
  ast::Function func(Source{}, "my_func", params, &void_type, body,
                     ast::FunctionDecorationList{});

  gen.increment_indent();

  ASSERT_TRUE(gen.EmitFunction(&func));
  EXPECT_EQ(gen.result(), R"(  fn my_func(a : f32, b : i32) -> void {
    discard;
    return;
  }
)");
}

TEST_F(WgslGeneratorImplTest, Emit_Function_WithDecoration_WorkgroupSize) {
  auto* body = create<ast::BlockStatement>();
  body->append(create<ast::DiscardStatement>());
  body->append(create<ast::ReturnStatement>());

  ast::type::Void void_type;
  ast::Function func(Source{}, "my_func", {}, &void_type, body,
                     ast::FunctionDecorationList{
                         create<ast::WorkgroupDecoration>(2u, 4u, 6u, Source{}),
                     });

  gen.increment_indent();

  ASSERT_TRUE(gen.EmitFunction(&func));
  EXPECT_EQ(gen.result(), R"(  [[workgroup_size(2, 4, 6)]]
  fn my_func() -> void {
    discard;
    return;
  }
)");
}

TEST_F(WgslGeneratorImplTest, Emit_Function_WithDecoration_Stage) {
  auto* body = create<ast::BlockStatement>();
  body->append(create<ast::DiscardStatement>());
  body->append(create<ast::ReturnStatement>());

  ast::type::Void void_type;
  ast::Function func(
      Source{}, "my_func", {}, &void_type, body,
      ast::FunctionDecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kFragment, Source{}),
      });

  gen.increment_indent();

  ASSERT_TRUE(gen.EmitFunction(&func));
  EXPECT_EQ(gen.result(), R"(  [[stage(fragment)]]
  fn my_func() -> void {
    discard;
    return;
  }
)");
}

TEST_F(WgslGeneratorImplTest, Emit_Function_WithDecoration_Multiple) {
  auto* body = create<ast::BlockStatement>();
  body->append(create<ast::DiscardStatement>());
  body->append(create<ast::ReturnStatement>());

  ast::type::Void void_type;
  ast::Function func(
      Source{}, "my_func", {}, &void_type, body,
      ast::FunctionDecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kFragment, Source{}),
          create<ast::WorkgroupDecoration>(2u, 4u, 6u, Source{}),
      });

  gen.increment_indent();

  ASSERT_TRUE(gen.EmitFunction(&func));
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
  // [[binding(0), set(0)]] var<storage_buffer> data : Data;
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

  ast::type::Void void_type;
  ast::type::F32 f32;

  ast::StructMemberList members;
  ast::StructMemberDecorationList a_deco;
  a_deco.push_back(create<ast::StructMemberOffsetDecoration>(0, Source{}));
  members.push_back(create<ast::StructMember>("d", &f32, a_deco));

  ast::StructDecorationList s_decos;
  s_decos.push_back(create<ast::StructBlockDecoration>(Source{}));

  auto* str = create<ast::Struct>(s_decos, members);

  ast::type::Struct s("Data", str);
  ast::type::AccessControl ac(ast::AccessControl::kReadWrite, &s);

  auto* data_var = create<ast::DecoratedVariable>(create<ast::Variable>(
      Source{}, "data", ast::StorageClass::kStorageBuffer, &ac));

  ast::VariableDecorationList decos;
  decos.push_back(create<ast::BindingDecoration>(0, Source{}));
  decos.push_back(create<ast::SetDecoration>(0, Source{}));
  data_var->set_decorations(decos);

  mod.AddConstructedType(&s);

  td.RegisterVariableForTesting(data_var);
  mod.AddGlobalVariable(data_var);

  {
    ast::VariableList params;
    auto* var = create<ast::Variable>(Source{}, "v",
                                      ast::StorageClass::kFunction, &f32);
    var->set_constructor(create<ast::MemberAccessorExpression>(
        create<ast::IdentifierExpression>("data"),
        create<ast::IdentifierExpression>("d")));

    auto* body = create<ast::BlockStatement>();
    body->append(create<ast::VariableDeclStatement>(var));
    body->append(create<ast::ReturnStatement>());

    auto* func =
        create<ast::Function>(Source{}, "a", params, &void_type, body,
                              ast::FunctionDecorationList{
                                  create<ast::StageDecoration>(
                                      ast::PipelineStage::kCompute, Source{}),
                              });

    mod.AddFunction(func);
  }

  {
    ast::VariableList params;
    auto* var = create<ast::Variable>(Source{}, "v",
                                      ast::StorageClass::kFunction, &f32);
    var->set_constructor(create<ast::MemberAccessorExpression>(
        create<ast::IdentifierExpression>("data"),
        create<ast::IdentifierExpression>("d")));

    auto* body = create<ast::BlockStatement>();
    body->append(create<ast::VariableDeclStatement>(var));
    body->append(create<ast::ReturnStatement>());

    auto* func =
        create<ast::Function>(Source{}, "b", params, &void_type, body,
                              ast::FunctionDecorationList{
                                  create<ast::StageDecoration>(
                                      ast::PipelineStage::kCompute, Source{}),
                              });

    mod.AddFunction(func);
  }

  ASSERT_TRUE(td.Determine()) << td.error();

  ASSERT_TRUE(gen.Generate(mod)) << gen.error();
  EXPECT_EQ(gen.result(), R"([[block]]
struct Data {
  [[offset(0)]]
  d : f32;
};

[[binding(0), set(0)]] var<storage_buffer> data : [[access(read_write)]]
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
