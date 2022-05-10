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

#include "src/tint/ast/stage_attribute.h"
#include "src/tint/ast/variable_decl_statement.h"
#include "src/tint/ast/workgroup_attribute.h"
#include "src/tint/writer/wgsl/test_helper.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::writer::wgsl {
namespace {

using WgslGeneratorImplTest = TestHelper;

TEST_F(WgslGeneratorImplTest, Emit_Function) {
    auto* func = Func("my_func", ast::VariableList{}, ty.void_(),
                      ast::StatementList{
                          Return(),
                      },
                      ast::AttributeList{});

    GeneratorImpl& gen = Build();

    gen.increment_indent();

    ASSERT_TRUE(gen.EmitFunction(func));
    EXPECT_EQ(gen.result(), R"(  fn my_func() {
    return;
  }
)");
}

TEST_F(WgslGeneratorImplTest, Emit_Function_WithParams) {
    auto* func =
        Func("my_func", ast::VariableList{Param("a", ty.f32()), Param("b", ty.i32())}, ty.void_(),
             ast::StatementList{
                 Return(),
             },
             ast::AttributeList{});

    GeneratorImpl& gen = Build();

    gen.increment_indent();

    ASSERT_TRUE(gen.EmitFunction(func));
    EXPECT_EQ(gen.result(), R"(  fn my_func(a : f32, b : i32) {
    return;
  }
)");
}

TEST_F(WgslGeneratorImplTest, Emit_Function_WithAttribute_WorkgroupSize) {
    auto* func = Func("my_func", ast::VariableList{}, ty.void_(), ast::StatementList{Return()},
                      ast::AttributeList{
                          Stage(ast::PipelineStage::kCompute),
                          WorkgroupSize(2_i, 4_i, 6_i),
                      });

    GeneratorImpl& gen = Build();

    gen.increment_indent();

    ASSERT_TRUE(gen.EmitFunction(func));
    EXPECT_EQ(gen.result(), R"(  @stage(compute) @workgroup_size(2i, 4i, 6i)
  fn my_func() {
    return;
  }
)");
}

TEST_F(WgslGeneratorImplTest, Emit_Function_WithAttribute_WorkgroupSize_WithIdent) {
    GlobalConst("height", ty.i32(), Expr(2_i));
    auto* func = Func("my_func", ast::VariableList{}, ty.void_(), ast::StatementList{Return()},
                      ast::AttributeList{
                          Stage(ast::PipelineStage::kCompute),
                          WorkgroupSize(2_i, "height"),
                      });

    GeneratorImpl& gen = Build();

    gen.increment_indent();

    ASSERT_TRUE(gen.EmitFunction(func));
    EXPECT_EQ(gen.result(), R"(  @stage(compute) @workgroup_size(2i, height)
  fn my_func() {
    return;
  }
)");
}

TEST_F(WgslGeneratorImplTest, Emit_Function_EntryPoint_Parameters) {
    auto* vec4 = ty.vec4<f32>();
    auto* coord = Param("coord", vec4, {Builtin(ast::Builtin::kPosition)});
    auto* loc1 = Param("loc1", ty.f32(), {Location(1u)});
    auto* func = Func("frag_main", ast::VariableList{coord, loc1}, ty.void_(), ast::StatementList{},
                      ast::AttributeList{
                          Stage(ast::PipelineStage::kFragment),
                      });

    GeneratorImpl& gen = Build();

    gen.increment_indent();

    ASSERT_TRUE(gen.EmitFunction(func));
    EXPECT_EQ(gen.result(), R"(  @stage(fragment)
  fn frag_main(@builtin(position) coord : vec4<f32>, @location(1) loc1 : f32) {
  }
)");
}

TEST_F(WgslGeneratorImplTest, Emit_Function_EntryPoint_ReturnValue) {
    auto* func = Func("frag_main", ast::VariableList{}, ty.f32(),
                      ast::StatementList{
                          Return(1_f),
                      },
                      ast::AttributeList{
                          Stage(ast::PipelineStage::kFragment),
                      },
                      ast::AttributeList{
                          Location(1u),
                      });

    GeneratorImpl& gen = Build();

    gen.increment_indent();

    ASSERT_TRUE(gen.EmitFunction(func));
    EXPECT_EQ(gen.result(), R"(  @stage(fragment)
  fn frag_main() -> @location(1) f32 {
    return 1.0;
  }
)");
}

// https://crbug.com/tint/297
TEST_F(WgslGeneratorImplTest, Emit_Function_Multiple_EntryPoint_With_Same_ModuleVar) {
    // struct Data {
    //   d : f32;
    // };
    // @binding(0) @group(0) var<storage> data : Data;
    //
    // @stage(compute) @workgroup_size(1)
    // fn a() {
    //   return;
    // }
    //
    // @stage(compute) @workgroup_size(1)
    // fn b() {
    //   return;
    // }

    auto* s = Structure("Data", {Member("d", ty.f32())});

    Global("data", ty.Of(s), ast::StorageClass::kStorage, ast::Access::kReadWrite,
           ast::AttributeList{
               create<ast::BindingAttribute>(0),
               create<ast::GroupAttribute>(0),
           });

    {
        auto* var = Var("v", ty.f32(), ast::StorageClass::kNone, MemberAccessor("data", "d"));

        Func("a", ast::VariableList{}, ty.void_(),
             ast::StatementList{
                 Decl(var),
                 Return(),
             },
             ast::AttributeList{
                 Stage(ast::PipelineStage::kCompute),
                 WorkgroupSize(1_i),
             });
    }

    {
        auto* var = Var("v", ty.f32(), ast::StorageClass::kNone, MemberAccessor("data", "d"));

        Func("b", ast::VariableList{}, ty.void_(),
             ast::StatementList{
                 Decl(var),
                 Return(),
             },
             ast::AttributeList{
                 Stage(ast::PipelineStage::kCompute),
                 WorkgroupSize(1_i),
             });
    }

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_EQ(gen.result(), R"(struct Data {
  d : f32,
}

@binding(0) @group(0) var<storage, read_write> data : Data;

@stage(compute) @workgroup_size(1i)
fn a() {
  var v : f32 = data.d;
  return;
}

@stage(compute) @workgroup_size(1i)
fn b() {
  var v : f32 = data.d;
  return;
}
)");
}

}  // namespace
}  // namespace tint::writer::wgsl
