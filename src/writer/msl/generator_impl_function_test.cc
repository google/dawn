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

#include "src/ast/stage_decoration.h"
#include "src/ast/struct_block_decoration.h"
#include "src/ast/variable_decl_statement.h"
#include "src/type/access_control_type.h"
#include "src/writer/msl/test_helper.h"

namespace tint {
namespace writer {
namespace msl {
namespace {

using MslGeneratorImplTest = TestHelper;

TEST_F(MslGeneratorImplTest, Emit_Function) {
  Func("my_func", ast::VariableList{}, ty.void_(),
       ast::StatementList{
           create<ast::ReturnStatement>(),
       },
       ast::DecorationList{});

  GeneratorImpl& gen = Build();

  gen.increment_indent();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

using namespace metal;
  void my_func() {
    return;
  }

)");
}

TEST_F(MslGeneratorImplTest, Emit_Function_WithParams) {
  ast::VariableList params;
  params.push_back(Var("a", ty.f32(), ast::StorageClass::kNone));
  params.push_back(Var("b", ty.i32(), ast::StorageClass::kNone));

  Func("my_func", params, ty.void_(),
       ast::StatementList{
           create<ast::ReturnStatement>(),
       },
       ast::DecorationList{});

  GeneratorImpl& gen = Build();

  gen.increment_indent();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

using namespace metal;
  void my_func(float a, int b) {
    return;
  }

)");
}

TEST_F(MslGeneratorImplTest, Emit_Decoration_EntryPoint_NoReturn_Void) {
  Func("main", ast::VariableList{}, ty.void_(),
       ast::StatementList{/* no explicit return */},
       ast::DecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kFragment)});

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

using namespace metal;
fragment void main() {
  return;
}

)");
}

TEST_F(MslGeneratorImplTest, Emit_Decoration_EntryPoint_NoReturn_InOut) {
  auto* foo_in = Var("foo", ty.f32(), ast::StorageClass::kNone, nullptr,
                     ast::DecorationList{create<ast::LocationDecoration>(0)});

  // TODO(jrprice): Make this the return value when supported.
  Global("bar", ty.f32(), ast::StorageClass::kOutput, nullptr,
         ast::DecorationList{create<ast::LocationDecoration>(1)});

  Func("main", ast::VariableList{foo_in}, ty.void_(),
       ast::StatementList{
           create<ast::AssignmentStatement>(Expr("bar"), Expr("foo")),
           /* no explicit return */},
       ast::DecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kFragment)});

  GeneratorImpl& gen = SanitizeAndBuild();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

using namespace metal;
struct tint_symbol_2 {
  float tint_symbol_1 [[user(locn0)]];
};

struct _tint_main_out {
  float bar [[color(1)]];
};

fragment _tint_main_out _tint_main(tint_symbol_2 tint_symbol_3 [[stage_in]]) {
  _tint_main_out _tint_out = {};
  const float tint_symbol_4 = tint_symbol_3.tint_symbol_1;
  _tint_out.bar = tint_symbol_4;
  return _tint_out;
}

)");
}

TEST_F(MslGeneratorImplTest, Emit_Decoration_EntryPoint_WithInOutVars) {
  auto* foo_in = Var("foo", ty.f32(), ast::StorageClass::kNone, nullptr,
                     ast::DecorationList{create<ast::LocationDecoration>(0)});

  // TODO(jrprice): Make this the return value when supported.
  Global("bar", ty.f32(), ast::StorageClass::kOutput, nullptr,
         ast::DecorationList{create<ast::LocationDecoration>(1)});

  auto body = ast::StatementList{
      create<ast::AssignmentStatement>(Expr("bar"), Expr("foo")),
      create<ast::ReturnStatement>(),
  };
  Func("frag_main", ast::VariableList{foo_in}, ty.void_(), body,
       ast::DecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kFragment)});

  GeneratorImpl& gen = SanitizeAndBuild();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

using namespace metal;
struct tint_symbol_2 {
  float tint_symbol_1 [[user(locn0)]];
};

struct frag_main_out {
  float bar [[color(1)]];
};

fragment frag_main_out frag_main(tint_symbol_2 tint_symbol_3 [[stage_in]]) {
  frag_main_out _tint_out = {};
  const float tint_symbol_4 = tint_symbol_3.tint_symbol_1;
  _tint_out.bar = tint_symbol_4;
  return _tint_out;
}

)");
}

TEST_F(MslGeneratorImplTest, Emit_Decoration_EntryPoint_WithInOut_Builtins) {
  auto* coord_in =
      Var("coord", ty.vec4<f32>(), ast::StorageClass::kNone, nullptr,
          ast::DecorationList{
              create<ast::BuiltinDecoration>(ast::Builtin::kFragCoord)});

  // TODO(jrprice): Make this the return value when supported.
  Global("depth", ty.f32(), ast::StorageClass::kOutput, nullptr,
         ast::DecorationList{
             create<ast::BuiltinDecoration>(ast::Builtin::kFragDepth)});

  auto body = ast::StatementList{
      create<ast::AssignmentStatement>(Expr("depth"),
                                       MemberAccessor("coord", "x")),
      create<ast::ReturnStatement>(),
  };

  Func("frag_main", ast::VariableList{coord_in}, ty.void_(), body,
       ast::DecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kFragment),
       });

  GeneratorImpl& gen = SanitizeAndBuild();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

using namespace metal;
struct frag_main_out {
  float depth [[depth(any)]];
};

fragment frag_main_out frag_main(float4 coord [[position]]) {
  frag_main_out _tint_out = {};
  _tint_out.depth = coord.x;
  return _tint_out;
}

)");
}

TEST_F(MslGeneratorImplTest, Emit_Decoration_EntryPoint_With_Uniform) {
  Global("coord", ty.vec4<f32>(), ast::StorageClass::kUniform, nullptr,
         ast::DecorationList{create<ast::BindingDecoration>(0),
                             create<ast::GroupDecoration>(1)});

  auto* var = Var("v", ty.f32(), ast::StorageClass::kFunction,
                  MemberAccessor("coord", "x"));

  Func("frag_main", ast::VariableList{}, ty.void_(),
       ast::StatementList{
           create<ast::VariableDeclStatement>(var),
           create<ast::ReturnStatement>(),
       },
       ast::DecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kFragment),
       });

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

using namespace metal;
fragment void frag_main(constant float4& coord [[buffer(0)]]) {
  float v = coord.x;
  return;
}

)");
}

TEST_F(MslGeneratorImplTest,
       Emit_FunctionDecoration_EntryPoint_With_RW_StorageBuffer) {
  auto* s = Structure("Data", {
                                  Member("a", ty.i32()),
                                  Member("b", ty.f32()),
                              });

  type::AccessControl ac(ast::AccessControl::kReadWrite, s);

  Global("coord", &ac, ast::StorageClass::kStorage, nullptr,
         ast::DecorationList{create<ast::BindingDecoration>(0),
                             create<ast::GroupDecoration>(1)});

  auto* var = Var("v", ty.f32(), ast::StorageClass::kFunction,
                  MemberAccessor("coord", "b"));

  Func("frag_main", ast::VariableList{}, ty.void_(),
       ast::StatementList{
           create<ast::VariableDeclStatement>(var),
           create<ast::ReturnStatement>(),
       },
       ast::DecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kFragment),
       });

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

using namespace metal;
struct Data {
  int a;
  float b;
};

fragment void frag_main(device Data& coord [[buffer(0)]]) {
  float v = coord.b;
  return;
}

)");
}

TEST_F(MslGeneratorImplTest,
       Emit_FunctionDecoration_EntryPoint_With_RO_StorageBuffer) {
  auto* s = Structure("Data", {
                                  Member("a", ty.i32()),
                                  Member("b", ty.f32()),
                              });

  type::AccessControl ac(ast::AccessControl::kReadOnly, s);

  Global("coord", &ac, ast::StorageClass::kStorage, nullptr,
         ast::DecorationList{create<ast::BindingDecoration>(0),
                             create<ast::GroupDecoration>(1)});

  auto* var = Var("v", ty.f32(), ast::StorageClass::kFunction,
                  MemberAccessor("coord", "b"));

  Func("frag_main", ast::VariableList{}, ty.void_(),
       ast::StatementList{
           create<ast::VariableDeclStatement>(var),
           create<ast::ReturnStatement>(),
       },
       ast::DecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kFragment),
       });

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

using namespace metal;
struct Data {
  int a;
  float b;
};

fragment void frag_main(const device Data& coord [[buffer(0)]]) {
  float v = coord.b;
  return;
}

)");
}

TEST_F(
    MslGeneratorImplTest,
    Emit_Decoration_Called_By_EntryPoints_WithLocationGlobals_And_Params) {  // NOLINT
  auto* foo_in = Var("foo", ty.f32(), ast::StorageClass::kNone, nullptr,
                     ast::DecorationList{create<ast::LocationDecoration>(0)});

  Global("bar", ty.f32(), ast::StorageClass::kOutput, nullptr,
         ast::DecorationList{create<ast::LocationDecoration>(1)});

  Global("val", ty.f32(), ast::StorageClass::kOutput, nullptr,
         ast::DecorationList{create<ast::LocationDecoration>(0)});

  ast::VariableList params;
  params.push_back(Var("param", ty.f32(), ast::StorageClass::kNone));
  params.push_back(Var("foo", ty.f32(), ast::StorageClass::kNone));

  auto body = ast::StatementList{
      create<ast::AssignmentStatement>(Expr("bar"), Expr("foo")),
      create<ast::AssignmentStatement>(Expr("val"), Expr("param")),
      create<ast::ReturnStatement>(Expr("foo"))};

  Func("sub_func", params, ty.f32(), body, ast::DecorationList{});

  body = ast::StatementList{
      create<ast::AssignmentStatement>(Expr("bar"),
                                       Call("sub_func", 1.0f, Expr("foo"))),
      create<ast::ReturnStatement>(),
  };

  Func("ep_1", ast::VariableList{foo_in}, ty.void_(), body,
       ast::DecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kFragment),
       });

  GeneratorImpl& gen = SanitizeAndBuild();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

using namespace metal;
struct tint_symbol_2 {
  float tint_symbol_1 [[user(locn0)]];
};

struct ep_1_out {
  float bar [[color(1)]];
  float val [[color(0)]];
};

float sub_func_ep_1(thread ep_1_out& _tint_out, float param, float foo) {
  _tint_out.bar = foo;
  _tint_out.val = param;
  return foo;
}

fragment ep_1_out ep_1(tint_symbol_2 tint_symbol_3 [[stage_in]]) {
  ep_1_out _tint_out = {};
  const float tint_symbol_4 = tint_symbol_3.tint_symbol_1;
  _tint_out.bar = sub_func_ep_1(_tint_out, 1.0f, tint_symbol_4);
  return _tint_out;
}

)");
}

TEST_F(MslGeneratorImplTest,
       Emit_Decoration_Called_By_EntryPoints_NoUsedGlobals) {
  Global("depth", ty.f32(), ast::StorageClass::kOutput, nullptr,
         ast::DecorationList{
             create<ast::BuiltinDecoration>(ast::Builtin::kFragDepth)});

  ast::VariableList params;
  params.push_back(Var("param", ty.f32(), ast::StorageClass::kFunction));

  Func("sub_func", params, ty.f32(),
       ast::StatementList{
           create<ast::ReturnStatement>(Expr("param")),
       },
       ast::DecorationList{});

  auto body = ast::StatementList{
      create<ast::AssignmentStatement>(Expr("depth"), Call("sub_func", 1.0f)),
      create<ast::ReturnStatement>(),
  };

  Func("ep_1", ast::VariableList{}, ty.void_(), body,
       ast::DecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kFragment),
       });

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

using namespace metal;
struct ep_1_out {
  float depth [[depth(any)]];
};

float sub_func(float param) {
  return param;
}

fragment ep_1_out ep_1() {
  ep_1_out _tint_out = {};
  _tint_out.depth = sub_func(1.0f);
  return _tint_out;
}

)");
}

TEST_F(
    MslGeneratorImplTest,
    Emit_Decoration_Called_By_EntryPoints_WithBuiltinGlobals_And_Params) {  // NOLINT
  auto* coord_in =
      Var("coord", ty.vec4<f32>(), ast::StorageClass::kNone, nullptr,
          ast::DecorationList{
              create<ast::BuiltinDecoration>(ast::Builtin::kFragCoord)});

  Global("depth", ty.f32(), ast::StorageClass::kOutput, nullptr,
         ast::DecorationList{
             create<ast::BuiltinDecoration>(ast::Builtin::kFragDepth)});

  ast::VariableList params;
  params.push_back(Var("param", ty.f32(), ast::StorageClass::kNone));
  params.push_back(Var("coord", ty.vec4<f32>(), ast::StorageClass::kNone));

  auto body = ast::StatementList{
      create<ast::AssignmentStatement>(Expr("depth"),
                                       MemberAccessor("coord", "x")),
      create<ast::ReturnStatement>(Expr("param")),
  };

  Func("sub_func", params, ty.f32(), body, ast::DecorationList{});

  body = ast::StatementList{
      create<ast::AssignmentStatement>(Expr("depth"),
                                       Call("sub_func", 1.0f, Expr("coord"))),
      create<ast::ReturnStatement>(),
  };

  Func("ep_1", ast::VariableList{coord_in}, ty.void_(), body,
       ast::DecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kFragment),
       });

  GeneratorImpl& gen = SanitizeAndBuild();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

using namespace metal;
struct ep_1_out {
  float depth [[depth(any)]];
};

float sub_func_ep_1(thread ep_1_out& _tint_out, float param, float4 coord) {
  _tint_out.depth = coord.x;
  return param;
}

fragment ep_1_out ep_1(float4 coord [[position]]) {
  ep_1_out _tint_out = {};
  _tint_out.depth = sub_func_ep_1(_tint_out, 1.0f, coord);
  return _tint_out;
}

)");
}

TEST_F(MslGeneratorImplTest,
       Emit_Decoration_Called_By_EntryPoint_With_Uniform) {
  Global("coord", ty.vec4<f32>(), ast::StorageClass::kUniform, nullptr,
         ast::DecorationList{create<ast::BindingDecoration>(0),
                             create<ast::GroupDecoration>(1)});

  ast::VariableList params;
  params.push_back(Var("param", ty.f32(), ast::StorageClass::kFunction));

  auto body = ast::StatementList{
      create<ast::ReturnStatement>(MemberAccessor("coord", "x")),
  };

  Func("sub_func", params, ty.f32(), body, ast::DecorationList{});

  ast::ExpressionList expr;
  expr.push_back(Expr(1.0f));

  auto* var =
      Var("v", ty.f32(), ast::StorageClass::kFunction, Call("sub_func", 1.0f));

  Func("frag_main", ast::VariableList{}, ty.void_(),
       ast::StatementList{
           create<ast::VariableDeclStatement>(var),
           create<ast::ReturnStatement>(),
       },
       ast::DecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kFragment),
       });

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

using namespace metal;
float sub_func(constant float4& coord, float param) {
  return coord.x;
}

fragment void frag_main(constant float4& coord [[buffer(0)]]) {
  float v = sub_func(coord, 1.0f);
  return;
}

)");
}

TEST_F(MslGeneratorImplTest,
       Emit_FunctionDecoration_Called_By_EntryPoint_With_RW_StorageBuffer) {
  auto* s = Structure("Data", {
                                  Member("a", ty.i32()),
                                  Member("b", ty.f32()),
                              });

  type::AccessControl ac(ast::AccessControl::kReadWrite, s);

  Global("coord", &ac, ast::StorageClass::kStorage, nullptr,
         ast::DecorationList{create<ast::BindingDecoration>(0),
                             create<ast::GroupDecoration>(1)});

  ast::VariableList params;
  params.push_back(Var("param", ty.f32(), ast::StorageClass::kFunction));

  auto body = ast::StatementList{
      create<ast::ReturnStatement>(MemberAccessor("coord", "b"))};

  Func("sub_func", params, ty.f32(), body, ast::DecorationList{});

  auto* var =
      Var("v", ty.f32(), ast::StorageClass::kFunction, Call("sub_func", 1.0f));

  Func("frag_main", ast::VariableList{}, ty.void_(),
       ast::StatementList{
           create<ast::VariableDeclStatement>(var),
           create<ast::ReturnStatement>(),
       },
       ast::DecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kFragment),
       });

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

using namespace metal;
struct Data {
  int a;
  float b;
};

float sub_func(device Data& coord, float param) {
  return coord.b;
}

fragment void frag_main(device Data& coord [[buffer(0)]]) {
  float v = sub_func(coord, 1.0f);
  return;
}

)");
}

TEST_F(MslGeneratorImplTest,
       Emit_FunctionDecoration_Called_By_EntryPoint_With_RO_StorageBuffer) {
  auto* s = Structure("Data", {
                                  Member("a", ty.i32()),
                                  Member("b", ty.f32()),
                              });

  type::AccessControl ac(ast::AccessControl::kReadOnly, s);

  Global("coord", &ac, ast::StorageClass::kStorage, nullptr,
         ast::DecorationList{create<ast::BindingDecoration>(0),
                             create<ast::GroupDecoration>(1)});

  ast::VariableList params;
  params.push_back(Var("param", ty.f32(), ast::StorageClass::kFunction));

  auto body = ast::StatementList{
      create<ast::ReturnStatement>(MemberAccessor("coord", "b"))};

  Func("sub_func", params, ty.f32(), body, ast::DecorationList{});

  ast::ExpressionList expr;
  expr.push_back(Expr(1.0f));

  auto* var =
      Var("v", ty.f32(), ast::StorageClass::kFunction, Call("sub_func", 1.0f));

  Func("frag_main", ast::VariableList{}, ty.void_(),
       ast::StatementList{
           create<ast::VariableDeclStatement>(var),
           create<ast::ReturnStatement>(),
       },
       ast::DecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kFragment),
       });

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

using namespace metal;
struct Data {
  int a;
  float b;
};

float sub_func(const device Data& coord, float param) {
  return coord.b;
}

fragment void frag_main(const device Data& coord [[buffer(0)]]) {
  float v = sub_func(coord, 1.0f);
  return;
}

)");
}

TEST_F(MslGeneratorImplTest,
       Emit_Decoration_EntryPoints_WithGlobal_Nested_Return) {
  Global("bar", ty.f32(), ast::StorageClass::kOutput, nullptr,
         ast::DecorationList{create<ast::LocationDecoration>(1)});

  auto* list = create<ast::BlockStatement>(ast::StatementList{
      create<ast::ReturnStatement>(),
  });

  auto body = ast::StatementList{
      create<ast::AssignmentStatement>(Expr("bar"), Expr(1.f)),
      create<ast::IfStatement>(create<ast::BinaryExpression>(
                                   ast::BinaryOp::kEqual, Expr(1), Expr(1)),
                               list, ast::ElseStatementList{}),
      create<ast::ReturnStatement>(),
  };

  Func("ep_1", ast::VariableList{}, ty.void_(), body,
       ast::DecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kFragment),
       });

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

using namespace metal;
struct ep_1_out {
  float bar [[color(1)]];
};

fragment ep_1_out ep_1() {
  ep_1_out _tint_out = {};
  _tint_out.bar = 1.0f;
  if ((1 == 1)) {
    return _tint_out;
  }
  return _tint_out;
}

)");
}

TEST_F(MslGeneratorImplTest, Emit_Function_WithArrayParams) {
  ast::VariableList params;
  params.push_back(Var("a", ty.array<f32, 5>(), ast::StorageClass::kNone));

  Func("my_func", params, ty.void_(),
       ast::StatementList{
           create<ast::ReturnStatement>(),
       },
       ast::DecorationList{});

  GeneratorImpl& gen = Build();

  gen.increment_indent();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

using namespace metal;
  void my_func(float a[5]) {
    return;
  }

)");
}

// https://crbug.com/tint/297
TEST_F(MslGeneratorImplTest,
       Emit_Function_Multiple_EntryPoint_With_Same_ModuleVar) {
  // [[block]] struct Data {
  //   d : f32;
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

  auto* s = Structure("Data", {Member("d", ty.f32())},
                      {create<ast::StructBlockDecoration>()});

  type::AccessControl ac(ast::AccessControl::kReadWrite, s);

  Global("data", &ac, ast::StorageClass::kStorage, nullptr,
         ast::DecorationList{create<ast::BindingDecoration>(0),
                             create<ast::GroupDecoration>(0)});

  {
    auto* var = Var("v", ty.f32(), ast::StorageClass::kFunction,
                    MemberAccessor("data", "d"));

    Func("a", ast::VariableList{}, ty.void_(),
         ast::StatementList{
             create<ast::VariableDeclStatement>(var),
             create<ast::ReturnStatement>(),
         },
         ast::DecorationList{
             create<ast::StageDecoration>(ast::PipelineStage::kCompute),
         });
  }

  {
    auto* var = Var("v", ty.f32(), ast::StorageClass::kFunction,
                    MemberAccessor("data", "d"));

    Func("b", ast::VariableList{}, ty.void_(),
         ast::StatementList{create<ast::VariableDeclStatement>(var),
                            create<ast::ReturnStatement>()},
         ast::DecorationList{
             create<ast::StageDecoration>(ast::PipelineStage::kCompute)});
  }

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

using namespace metal;
struct Data {
  float d;
};

kernel void a(device Data& data [[buffer(0)]]) {
  float v = data.d;
  return;
}

kernel void b(device Data& data [[buffer(0)]]) {
  float v = data.d;
  return;
}

)");
}

}  // namespace
}  // namespace msl
}  // namespace writer
}  // namespace tint
