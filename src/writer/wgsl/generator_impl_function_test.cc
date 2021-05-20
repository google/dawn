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
#include "src/ast/workgroup_decoration.h"
#include "src/writer/wgsl/test_helper.h"

namespace tint {
namespace writer {
namespace wgsl {
namespace {

using WgslGeneratorImplTest = TestHelper;

TEST_F(WgslGeneratorImplTest, Emit_Function) {
  auto* func = Func("my_func", ast::VariableList{}, ty.void_(),
                    ast::StatementList{
                        create<ast::DiscardStatement>(),
                        Return(),
                    },
                    ast::DecorationList{});

  GeneratorImpl& gen = Build();

  gen.increment_indent();

  ASSERT_TRUE(gen.EmitFunction(func));
  EXPECT_EQ(gen.result(), R"(  fn my_func() {
    discard;
    return;
  }
)");
}

TEST_F(WgslGeneratorImplTest, Emit_Function_WithParams) {
  auto* func = Func(
      "my_func", ast::VariableList{Param("a", ty.f32()), Param("b", ty.i32())},
      ty.void_(),
      ast::StatementList{
          create<ast::DiscardStatement>(),
          Return(),
      },
      ast::DecorationList{});

  GeneratorImpl& gen = Build();

  gen.increment_indent();

  ASSERT_TRUE(gen.EmitFunction(func));
  EXPECT_EQ(gen.result(), R"(  fn my_func(a : f32, b : i32) {
    discard;
    return;
  }
)");
}

TEST_F(WgslGeneratorImplTest, Emit_Function_WithDecoration_WorkgroupSize) {
  auto* func = Func("my_func", ast::VariableList{}, ty.void_(),
                    ast::StatementList{Return()},
                    ast::DecorationList{
                        Stage(ast::PipelineStage::kCompute),
                        WorkgroupSize(2, 4, 6),
                    });

  GeneratorImpl& gen = Build();

  gen.increment_indent();

  ASSERT_TRUE(gen.EmitFunction(func));
  EXPECT_EQ(gen.result(), R"(  [[stage(compute), workgroup_size(2, 4, 6)]]
  fn my_func() {
    return;
  }
)");
}

TEST_F(WgslGeneratorImplTest,
       Emit_Function_WithDecoration_WorkgroupSize_WithIdent) {
  GlobalConst("height", ty.i32(), Expr(2));
  auto* func = Func("my_func", ast::VariableList{}, ty.void_(),
                    ast::StatementList{Return()},
                    ast::DecorationList{
                        Stage(ast::PipelineStage::kCompute),
                        WorkgroupSize(2, "height"),
                    });

  GeneratorImpl& gen = Build();

  gen.increment_indent();

  ASSERT_TRUE(gen.EmitFunction(func));
  EXPECT_EQ(gen.result(), R"(  [[stage(compute), workgroup_size(2, height)]]
  fn my_func() {
    return;
  }
)");
}

TEST_F(WgslGeneratorImplTest, Emit_Function_EntryPoint_Parameters) {
  auto* vec4 = ty.vec4<f32>();
  auto* coord = Param("coord", vec4, {Builtin(ast::Builtin::kPosition)});
  auto* loc1 = Param("loc1", ty.f32(), {Location(1u)});
  auto* func = Func("frag_main", ast::VariableList{coord, loc1}, ty.void_(),
                    ast::StatementList{},
                    ast::DecorationList{
                        Stage(ast::PipelineStage::kFragment),
                    });

  GeneratorImpl& gen = Build();

  gen.increment_indent();

  ASSERT_TRUE(gen.EmitFunction(func));
  EXPECT_EQ(gen.result(), R"(  [[stage(fragment)]]
  fn frag_main([[builtin(position)]] coord : vec4<f32>, [[location(1)]] loc1 : f32) {
  }
)");
}

TEST_F(WgslGeneratorImplTest, Emit_Function_EntryPoint_ReturnValue) {
  auto* func = Func("frag_main", ast::VariableList{}, ty.f32(),
                    ast::StatementList{
                        Return(1.f),
                    },
                    ast::DecorationList{
                        Stage(ast::PipelineStage::kFragment),
                    },
                    ast::DecorationList{
                        Location(1u),
                    });

  GeneratorImpl& gen = Build();

  gen.increment_indent();

  ASSERT_TRUE(gen.EmitFunction(func));
  EXPECT_EQ(gen.result(), R"(  [[stage(fragment)]]
  fn frag_main() -> [[location(1)]] f32 {
    return 1.0;
  }
)");
}

// https://crbug.com/tint/297
TEST_F(WgslGeneratorImplTest,
       Emit_Function_Multiple_EntryPoint_With_Same_ModuleVar) {
  // [[block]] struct Data {
  //   d : f32;
  // };
  // [[binding(0), group(0)]] var<storage> data : Data;
  //
  // [[stage(compute)]]
  // fn a() {
  //   return;
  // }
  //
  // [[stage(compute)]]
  // fn b() {
  //   return;
  // }

  auto* s = Structure("Data", {Member("d", ty.f32())},
                      {create<ast::StructBlockDecoration>()});

  auto* ac = ty.access(ast::AccessControl::kReadWrite, s);

  Global("data", ac, ast::StorageClass::kStorage, nullptr,
         ast::DecorationList{
             create<ast::BindingDecoration>(0),
             create<ast::GroupDecoration>(0),
         });

  {
    auto* var = Var("v", ty.f32(), ast::StorageClass::kNone,
                    MemberAccessor("data", "d"));

    Func("a", ast::VariableList{}, ty.void_(),
         ast::StatementList{
             Decl(var),
             Return(),
         },
         ast::DecorationList{
             Stage(ast::PipelineStage::kCompute),
         });
  }

  {
    auto* var = Var("v", ty.f32(), ast::StorageClass::kNone,
                    MemberAccessor("data", "d"));

    Func("b", ast::VariableList{}, ty.void_(),
         ast::StatementList{
             Decl(var),
             Return(),
         },
         ast::DecorationList{
             Stage(ast::PipelineStage::kCompute),
         });
  }

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_EQ(gen.result(), R"([[block]]
struct Data {
  d : f32;
};

[[binding(0), group(0)]] var<storage> data : [[access(read_write)]] Data;

[[stage(compute)]]
fn a() {
  var v : f32 = data.d;
  return;
}

[[stage(compute)]]
fn b() {
  var v : f32 = data.d;
  return;
}
)");
}

}  // namespace
}  // namespace wgsl
}  // namespace writer
}  // namespace tint
