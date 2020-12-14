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
#include <unordered_set>

#include "src/ast/assignment_statement.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/location_decoration.h"
#include "src/ast/member_accessor_expression.h"
#include "src/ast/module.h"
#include "src/ast/pipeline_stage.h"
#include "src/ast/return_statement.h"
#include "src/ast/stage_decoration.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/i32_type.h"
#include "src/ast/type/vector_type.h"
#include "src/ast/type/void_type.h"
#include "src/ast/variable.h"
#include "src/type_determiner.h"
#include "src/writer/hlsl/test_helper.h"

namespace tint {
namespace writer {
namespace hlsl {
namespace {

using HlslGeneratorImplTest_EntryPoint = TestHelper;

TEST_F(HlslGeneratorImplTest_EntryPoint,
       Emit_Function_EntryPointData_Vertex_Input) {
  // [[location 0]] var<in> foo : f32;
  // [[location 1]] var<in> bar : i32;
  //
  // struct vtx_main_in {
  //   float foo : TEXCOORD0;
  //   int bar : TEXCOORD1;
  // };

  ast::type::F32 f32;
  ast::type::I32 i32;

  auto* foo_var =
      create<ast::Variable>(Source{},                   // source
                            "foo",                      // name
                            ast::StorageClass::kInput,  // storage_class
                            &f32,                       // type
                            false,                      // is_const
                            nullptr,                    // constructor
                            ast::VariableDecorationList{
                                // decorations
                                create<ast::LocationDecoration>(0, Source{}),
                            });

  auto* bar_var =
      create<ast::Variable>(Source{},                   // source
                            "bar",                      // name
                            ast::StorageClass::kInput,  // storage_class
                            &i32,                       // type
                            false,                      // is_const
                            nullptr,                    // constructor
                            ast::VariableDecorationList{
                                // decorations
                                create<ast::LocationDecoration>(1, Source{}),
                            });

  td.RegisterVariableForTesting(foo_var);
  td.RegisterVariableForTesting(bar_var);

  mod.AddGlobalVariable(foo_var);
  mod.AddGlobalVariable(bar_var);

  ast::VariableList params;
  auto* body = create<ast::BlockStatement>(
      Source{}, ast::StatementList{
                    create<ast::AssignmentStatement>(
                        Source{},
                        create<ast::IdentifierExpression>(
                            Source{}, mod.RegisterSymbol("foo"), "foo"),
                        create<ast::IdentifierExpression>(
                            Source{}, mod.RegisterSymbol("foo"), "foo")),
                    create<ast::AssignmentStatement>(
                        Source{},
                        create<ast::IdentifierExpression>(
                            Source{}, mod.RegisterSymbol("bar"), "bar"),
                        create<ast::IdentifierExpression>(
                            Source{}, mod.RegisterSymbol("bar"), "bar")),
                });
  auto* func = create<ast::Function>(
      Source{}, mod.RegisterSymbol("vtx_main"), "vtx_main", params, &f32, body,
      ast::FunctionDecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kVertex, Source{}),
      });

  mod.AddFunction(func);

  std::unordered_set<std::string> globals;

  ASSERT_TRUE(td.Determine()) << td.error();
  ASSERT_TRUE(gen.EmitEntryPointData(out, func, globals)) << gen.error();
  EXPECT_EQ(result(), R"(struct vtx_main_in {
  float foo : TEXCOORD0;
  int bar : TEXCOORD1;
};

)");
}

TEST_F(HlslGeneratorImplTest_EntryPoint,
       Emit_Function_EntryPointData_Vertex_Output) {
  // [[location 0]] var<out> foo : f32;
  // [[location 1]] var<out> bar : i32;
  //
  // struct vtx_main_out {
  //   float foo : TEXCOORD0;
  //   int bar : TEXCOORD1;
  // };

  ast::type::F32 f32;
  ast::type::I32 i32;

  auto* foo_var =
      create<ast::Variable>(Source{},                    // source
                            "foo",                       // name
                            ast::StorageClass::kOutput,  // storage_class
                            &f32,                        // type
                            false,                       // is_const
                            nullptr,                     // constructor
                            ast::VariableDecorationList{
                                // decorations
                                create<ast::LocationDecoration>(0, Source{}),
                            });

  auto* bar_var =
      create<ast::Variable>(Source{},                    // source
                            "bar",                       // name
                            ast::StorageClass::kOutput,  // storage_class
                            &i32,                        // type
                            false,                       // is_const
                            nullptr,                     // constructor
                            ast::VariableDecorationList{
                                // decorations
                                create<ast::LocationDecoration>(1, Source{}),
                            });

  td.RegisterVariableForTesting(foo_var);
  td.RegisterVariableForTesting(bar_var);

  mod.AddGlobalVariable(foo_var);
  mod.AddGlobalVariable(bar_var);

  ast::VariableList params;

  auto* body = create<ast::BlockStatement>(
      Source{}, ast::StatementList{
                    create<ast::AssignmentStatement>(
                        Source{},
                        create<ast::IdentifierExpression>(
                            Source{}, mod.RegisterSymbol("foo"), "foo"),
                        create<ast::IdentifierExpression>(
                            Source{}, mod.RegisterSymbol("foo"), "foo")),
                    create<ast::AssignmentStatement>(
                        Source{},
                        create<ast::IdentifierExpression>(
                            Source{}, mod.RegisterSymbol("bar"), "bar"),
                        create<ast::IdentifierExpression>(
                            Source{}, mod.RegisterSymbol("bar"), "bar")),
                });
  auto* func = create<ast::Function>(
      Source{}, mod.RegisterSymbol("vtx_main"), "vtx_main", params, &f32, body,
      ast::FunctionDecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kVertex, Source{}),
      });

  mod.AddFunction(func);

  std::unordered_set<std::string> globals;

  ASSERT_TRUE(td.Determine()) << td.error();
  ASSERT_TRUE(gen.EmitEntryPointData(out, func, globals)) << gen.error();
  EXPECT_EQ(result(), R"(struct vtx_main_out {
  float foo : TEXCOORD0;
  int bar : TEXCOORD1;
};

)");
}

TEST_F(HlslGeneratorImplTest_EntryPoint,
       Emit_Function_EntryPointData_Fragment_Input) {
  // [[location 0]] var<in> foo : f32;
  // [[location 1]] var<in> bar : i32;
  //
  // struct frag_main_in {
  //   float foo : TEXCOORD0;
  //   int bar : TEXCOORD1;
  // };

  ast::type::F32 f32;
  ast::type::I32 i32;

  auto* foo_var =
      create<ast::Variable>(Source{},                   // source
                            "foo",                      // name
                            ast::StorageClass::kInput,  // storage_class
                            &f32,                       // type
                            false,                      // is_const
                            nullptr,                    // constructor
                            ast::VariableDecorationList{
                                // decorations
                                create<ast::LocationDecoration>(0, Source{}),
                            });

  auto* bar_var =
      create<ast::Variable>(Source{},                   // source
                            "bar",                      // name
                            ast::StorageClass::kInput,  // storage_class
                            &i32,                       // type
                            false,                      // is_const
                            nullptr,                    // constructor
                            ast::VariableDecorationList{
                                // decorations
                                create<ast::LocationDecoration>(1, Source{}),
                            });

  td.RegisterVariableForTesting(foo_var);
  td.RegisterVariableForTesting(bar_var);

  mod.AddGlobalVariable(foo_var);
  mod.AddGlobalVariable(bar_var);

  ast::VariableList params;

  auto* body = create<ast::BlockStatement>(
      Source{}, ast::StatementList{
                    create<ast::AssignmentStatement>(
                        Source{},
                        create<ast::IdentifierExpression>(
                            Source{}, mod.RegisterSymbol("foo"), "foo"),
                        create<ast::IdentifierExpression>(
                            Source{}, mod.RegisterSymbol("foo"), "foo")),
                    create<ast::AssignmentStatement>(
                        Source{},
                        create<ast::IdentifierExpression>(
                            Source{}, mod.RegisterSymbol("bar"), "bar"),
                        create<ast::IdentifierExpression>(
                            Source{}, mod.RegisterSymbol("bar"), "bar")),
                });
  auto* func = create<ast::Function>(
      Source{}, mod.RegisterSymbol("main"), "main", params, &f32, body,
      ast::FunctionDecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kVertex, Source{}),
      });

  mod.AddFunction(func);

  std::unordered_set<std::string> globals;

  ASSERT_TRUE(td.Determine()) << td.error();
  ASSERT_TRUE(gen.EmitEntryPointData(out, func, globals)) << gen.error();
  EXPECT_EQ(result(), R"(struct main_in {
  float foo : TEXCOORD0;
  int bar : TEXCOORD1;
};

)");
}

TEST_F(HlslGeneratorImplTest_EntryPoint,
       Emit_Function_EntryPointData_Fragment_Output) {
  // [[location 0]] var<out> foo : f32;
  // [[location 1]] var<out> bar : i32;
  //
  // struct frag_main_out {
  //   float foo : SV_Target0;
  //   int bar : SV_Target1;
  // };

  ast::type::F32 f32;
  ast::type::I32 i32;

  auto* foo_var =
      create<ast::Variable>(Source{},                    // source
                            "foo",                       // name
                            ast::StorageClass::kOutput,  // storage_class
                            &f32,                        // type
                            false,                       // is_const
                            nullptr,                     // constructor
                            ast::VariableDecorationList{
                                // decorations
                                create<ast::LocationDecoration>(0, Source{}),
                            });

  auto* bar_var =
      create<ast::Variable>(Source{},                    // source
                            "bar",                       // name
                            ast::StorageClass::kOutput,  // storage_class
                            &i32,                        // type
                            false,                       // is_const
                            nullptr,                     // constructor
                            ast::VariableDecorationList{
                                // decorations
                                create<ast::LocationDecoration>(1, Source{}),
                            });

  td.RegisterVariableForTesting(foo_var);
  td.RegisterVariableForTesting(bar_var);

  mod.AddGlobalVariable(foo_var);
  mod.AddGlobalVariable(bar_var);

  ast::VariableList params;
  auto* body = create<ast::BlockStatement>(
      Source{}, ast::StatementList{
                    create<ast::AssignmentStatement>(
                        Source{},
                        create<ast::IdentifierExpression>(
                            Source{}, mod.RegisterSymbol("foo"), "foo"),
                        create<ast::IdentifierExpression>(
                            Source{}, mod.RegisterSymbol("foo"), "foo")),
                    create<ast::AssignmentStatement>(
                        Source{},
                        create<ast::IdentifierExpression>(
                            Source{}, mod.RegisterSymbol("bar"), "bar"),
                        create<ast::IdentifierExpression>(
                            Source{}, mod.RegisterSymbol("bar"), "bar")),
                });
  auto* func = create<ast::Function>(
      Source{}, mod.RegisterSymbol("main"), "main", params, &f32, body,
      ast::FunctionDecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kFragment, Source{}),
      });

  mod.AddFunction(func);

  std::unordered_set<std::string> globals;

  ASSERT_TRUE(td.Determine()) << td.error();
  ASSERT_TRUE(gen.EmitEntryPointData(out, func, globals)) << gen.error();
  EXPECT_EQ(result(), R"(struct main_out {
  float foo : SV_Target0;
  int bar : SV_Target1;
};

)");
}

TEST_F(HlslGeneratorImplTest_EntryPoint,
       Emit_Function_EntryPointData_Compute_Input) {
  // [[location 0]] var<in> foo : f32;
  // [[location 1]] var<in> bar : i32;
  //
  // -> Error, not allowed

  ast::type::F32 f32;
  ast::type::I32 i32;

  auto* foo_var =
      create<ast::Variable>(Source{},                   // source
                            "foo",                      // name
                            ast::StorageClass::kInput,  // storage_class
                            &f32,                       // type
                            false,                      // is_const
                            nullptr,                    // constructor
                            ast::VariableDecorationList{
                                // decorations
                                create<ast::LocationDecoration>(0, Source{}),
                            });

  auto* bar_var =
      create<ast::Variable>(Source{},                   // source
                            "bar",                      // name
                            ast::StorageClass::kInput,  // storage_class
                            &i32,                       // type
                            false,                      // is_const
                            nullptr,                    // constructor
                            ast::VariableDecorationList{
                                // decorations
                                create<ast::LocationDecoration>(1, Source{}),
                            });

  td.RegisterVariableForTesting(foo_var);
  td.RegisterVariableForTesting(bar_var);

  mod.AddGlobalVariable(foo_var);
  mod.AddGlobalVariable(bar_var);

  ast::VariableList params;
  auto* body = create<ast::BlockStatement>(
      Source{}, ast::StatementList{
                    create<ast::AssignmentStatement>(
                        Source{},
                        create<ast::IdentifierExpression>(
                            Source{}, mod.RegisterSymbol("foo"), "foo"),
                        create<ast::IdentifierExpression>(
                            Source{}, mod.RegisterSymbol("foo"), "foo")),
                    create<ast::AssignmentStatement>(
                        Source{},
                        create<ast::IdentifierExpression>(
                            Source{}, mod.RegisterSymbol("bar"), "bar"),
                        create<ast::IdentifierExpression>(
                            Source{}, mod.RegisterSymbol("bar"), "bar")),
                });
  auto* func = create<ast::Function>(
      Source{}, mod.RegisterSymbol("main"), "main", params, &f32, body,
      ast::FunctionDecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kCompute, Source{}),
      });

  mod.AddFunction(func);

  std::unordered_set<std::string> globals;

  ASSERT_TRUE(td.Determine()) << td.error();
  ASSERT_FALSE(gen.EmitEntryPointData(out, func, globals)) << gen.error();
  EXPECT_EQ(gen.error(), R"(invalid location variable for pipeline stage)");
}

TEST_F(HlslGeneratorImplTest_EntryPoint,
       Emit_Function_EntryPointData_Compute_Output) {
  // [[location 0]] var<out> foo : f32;
  // [[location 1]] var<out> bar : i32;
  //
  // -> Error not allowed

  ast::type::F32 f32;
  ast::type::I32 i32;

  auto* foo_var =
      create<ast::Variable>(Source{},                    // source
                            "foo",                       // name
                            ast::StorageClass::kOutput,  // storage_class
                            &f32,                        // type
                            false,                       // is_const
                            nullptr,                     // constructor
                            ast::VariableDecorationList{
                                // decorations
                                create<ast::LocationDecoration>(0, Source{}),
                            });

  auto* bar_var =
      create<ast::Variable>(Source{},                    // source
                            "bar",                       // name
                            ast::StorageClass::kOutput,  // storage_class
                            &i32,                        // type
                            false,                       // is_const
                            nullptr,                     // constructor
                            ast::VariableDecorationList{
                                // decorations
                                create<ast::LocationDecoration>(1, Source{}),
                            });

  td.RegisterVariableForTesting(foo_var);
  td.RegisterVariableForTesting(bar_var);

  mod.AddGlobalVariable(foo_var);
  mod.AddGlobalVariable(bar_var);

  ast::VariableList params;
  auto* body = create<ast::BlockStatement>(
      Source{}, ast::StatementList{
                    create<ast::AssignmentStatement>(
                        Source{},
                        create<ast::IdentifierExpression>(
                            Source{}, mod.RegisterSymbol("foo"), "foo"),
                        create<ast::IdentifierExpression>(
                            Source{}, mod.RegisterSymbol("foo"), "foo")),
                    create<ast::AssignmentStatement>(
                        Source{},
                        create<ast::IdentifierExpression>(
                            Source{}, mod.RegisterSymbol("bar"), "bar"),
                        create<ast::IdentifierExpression>(
                            Source{}, mod.RegisterSymbol("bar"), "bar")),
                });
  auto* func = create<ast::Function>(
      Source{}, mod.RegisterSymbol("main"), "main", params, &f32, body,
      ast::FunctionDecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kCompute, Source{}),
      });

  mod.AddFunction(func);

  std::unordered_set<std::string> globals;

  ASSERT_TRUE(td.Determine()) << td.error();
  ASSERT_FALSE(gen.EmitEntryPointData(out, func, globals)) << gen.error();
  EXPECT_EQ(gen.error(), R"(invalid location variable for pipeline stage)");
}

TEST_F(HlslGeneratorImplTest_EntryPoint,
       Emit_Function_EntryPointData_Builtins) {
  // [[builtin frag_coord]] var<in> coord : vec4<f32>;
  // [[builtin frag_depth]] var<out> depth : f32;
  //
  // struct main_in {
  //   float4 coord : SV_Position;
  // };
  //
  // struct main_out {
  //   float depth : SV_Depth;
  // };

  ast::type::F32 f32;
  ast::type::Void void_type;
  ast::type::Vector vec4(&f32, 4);

  auto* coord_var = create<ast::Variable>(
      Source{},                   // source
      "coord",                    // name
      ast::StorageClass::kInput,  // storage_class
      &vec4,                      // type
      false,                      // is_const
      nullptr,                    // constructor
      ast::VariableDecorationList{
          // decorations
          create<ast::BuiltinDecoration>(ast::Builtin::kFragCoord, Source{}),
      });

  auto* depth_var = create<ast::Variable>(
      Source{},                    // source
      "depth",                     // name
      ast::StorageClass::kOutput,  // storage_class
      &f32,                        // type
      false,                       // is_const
      nullptr,                     // constructor
      ast::VariableDecorationList{
          // decorations
          create<ast::BuiltinDecoration>(ast::Builtin::kFragDepth, Source{}),
      });

  td.RegisterVariableForTesting(coord_var);
  td.RegisterVariableForTesting(depth_var);

  mod.AddGlobalVariable(coord_var);
  mod.AddGlobalVariable(depth_var);

  ast::VariableList params;
  auto* body = create<ast::BlockStatement>(
      Source{}, ast::StatementList{
                    create<ast::AssignmentStatement>(
                        Source{},
                        create<ast::IdentifierExpression>(
                            Source{}, mod.RegisterSymbol("depth"), "depth"),
                        create<ast::MemberAccessorExpression>(
                            Source{},
                            create<ast::IdentifierExpression>(
                                Source{}, mod.RegisterSymbol("coord"), "coord"),
                            create<ast::IdentifierExpression>(
                                Source{}, mod.RegisterSymbol("x"), "x"))),
                });
  auto* func = create<ast::Function>(
      Source{}, mod.RegisterSymbol("main"), "main", params, &void_type, body,
      ast::FunctionDecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kFragment, Source{}),
      });

  mod.AddFunction(func);

  std::unordered_set<std::string> globals;

  ASSERT_TRUE(td.Determine()) << td.error();
  ASSERT_TRUE(gen.EmitEntryPointData(out, func, globals)) << gen.error();
  EXPECT_EQ(result(), R"(struct main_in {
  float4 coord : SV_Position;
};

struct main_out {
  float depth : SV_Depth;
};

)");
}

}  // namespace
}  // namespace hlsl
}  // namespace writer
}  // namespace tint
