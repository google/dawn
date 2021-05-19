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

#include "gtest/gtest-spi.h"
#include "src/ast/discard_statement.h"
#include "src/ast/stage_decoration.h"
#include "src/ast/test_helper.h"
#include "src/ast/workgroup_decoration.h"

namespace tint {
namespace ast {
namespace {

using FunctionTest = TestHelper;

TEST_F(FunctionTest, Creation) {
  VariableList params;
  params.push_back(Param("var", ty.i32()));
  auto* var = params[0];

  auto* f = Func("func", params, ty.void_(), StatementList{}, DecorationList{});
  EXPECT_EQ(f->symbol(), Symbols().Get("func"));
  ASSERT_EQ(f->params().size(), 1u);
  EXPECT_TRUE(f->return_type()->Is<ast::Void>());
  EXPECT_EQ(f->params()[0], var);
}

TEST_F(FunctionTest, Creation_WithSource) {
  VariableList params;
  params.push_back(Param("var", ty.i32()));

  auto* f = Func(Source{Source::Location{20, 2}}, "func", params, ty.void_(),
                 StatementList{}, DecorationList{});
  auto src = f->source();
  EXPECT_EQ(src.range.begin.line, 20u);
  EXPECT_EQ(src.range.begin.column, 2u);
}

TEST_F(FunctionTest, Assert_InvalidName) {
  EXPECT_FATAL_FAILURE(
      {
        ProgramBuilder b;
        b.Func("", VariableList{}, b.ty.void_(), StatementList{},
               DecorationList{});
      },
      "internal compiler error");
}

TEST_F(FunctionTest, Assert_Null_ReturnType) {
  EXPECT_FATAL_FAILURE(
      {
        ProgramBuilder b;
        b.Func("f", VariableList{}, nullptr, StatementList{}, DecorationList{});
      },
      "internal compiler error");
}

TEST_F(FunctionTest, Assert_Null_Param) {
  EXPECT_FATAL_FAILURE(
      {
        ProgramBuilder b;
        VariableList params;
        params.push_back(b.Param("var", b.ty.i32()));
        params.push_back(nullptr);

        b.Func("f", params, b.ty.void_(), StatementList{}, DecorationList{});
      },
      "internal compiler error");
}

TEST_F(FunctionTest, Assert_DifferentProgramID_Symbol) {
  EXPECT_FATAL_FAILURE(
      {
        ProgramBuilder b1;
        ProgramBuilder b2;
        b1.Func(b2.Sym("func"), VariableList{}, b1.ty.void_(), StatementList{});
      },
      "internal compiler error");
}

TEST_F(FunctionTest, Assert_DifferentProgramID_Param) {
  EXPECT_FATAL_FAILURE(
      {
        ProgramBuilder b1;
        ProgramBuilder b2;
        b1.Func("func", VariableList{b2.Param("var", b2.ty.i32())},
                b1.ty.void_(), StatementList{});
      },
      "internal compiler error");
}

TEST_F(FunctionTest, Assert_DifferentProgramID_Deco) {
  EXPECT_FATAL_FAILURE(
      {
        ProgramBuilder b1;
        ProgramBuilder b2;
        b1.Func("func", VariableList{}, b1.ty.void_(), StatementList{},
                DecorationList{
                    b2.WorkgroupSize(2, 4, 6),
                });
      },
      "internal compiler error");
}

TEST_F(FunctionTest, Assert_DifferentProgramID_ReturnDeco) {
  EXPECT_FATAL_FAILURE(
      {
        ProgramBuilder b1;
        ProgramBuilder b2;
        b1.Func("func", VariableList{}, b1.ty.void_(), StatementList{},
                DecorationList{},
                DecorationList{
                    b2.WorkgroupSize(2, 4, 6),
                });
      },
      "internal compiler error");
}

TEST_F(FunctionTest, Assert_NonConstParam) {
  EXPECT_FATAL_FAILURE(
      {
        ProgramBuilder b;
        VariableList params;
        params.push_back(b.Var("var", b.ty.i32(), ast::StorageClass::kNone));

        b.Func("f", params, b.ty.void_(), StatementList{}, DecorationList{});
      },
      "internal compiler error");
}

TEST_F(FunctionTest, ToStr) {
  auto* f = Func("func", VariableList{}, ty.void_(),
                 StatementList{
                     create<DiscardStatement>(),
                 },
                 DecorationList{});

  EXPECT_EQ(str(f), R"(Function func -> __void
()
{
  Discard{}
}
)");
}

TEST_F(FunctionTest, ToStr_WithDecoration) {
  auto* f = Func("func", VariableList{}, ty.void_(),
                 StatementList{
                     create<DiscardStatement>(),
                 },
                 DecorationList{WorkgroupSize(2, 4, 6)});

  EXPECT_EQ(str(f), R"(Function func -> __void
WorkgroupDecoration{
  ScalarConstructor[not set]{2}
  ScalarConstructor[not set]{4}
  ScalarConstructor[not set]{6}
}
()
{
  Discard{}
}
)");
}

TEST_F(FunctionTest, ToStr_WithParams) {
  VariableList params;
  params.push_back(Param("var", ty.i32()));

  auto* f = Func("func", params, ty.void_(),
                 StatementList{
                     create<DiscardStatement>(),
                 },
                 DecorationList{});

  EXPECT_EQ(str(f), R"(Function func -> __void
(
  VariableConst{
    var
    none
    __i32
  }
)
{
  Discard{}
}
)");
}

TEST_F(FunctionTest, TypeName) {
  auto* f = Func("func", VariableList{}, ty.void_(), StatementList{},
                 DecorationList{});
  EXPECT_EQ(f->type_name(), "__func__void");
}

TEST_F(FunctionTest, TypeName_WithParams) {
  VariableList params;
  params.push_back(Param("var1", ty.i32()));
  params.push_back(Param("var2", ty.f32()));

  auto* f = Func("func", params, ty.void_(), StatementList{}, DecorationList{});
  EXPECT_EQ(f->type_name(), "__func__void__i32__f32");
}

TEST_F(FunctionTest, GetLastStatement) {
  VariableList params;
  auto* stmt = create<DiscardStatement>();
  auto* f =
      Func("func", params, ty.void_(), StatementList{stmt}, DecorationList{});

  EXPECT_EQ(f->get_last_statement(), stmt);
}

TEST_F(FunctionTest, GetLastStatement_nullptr) {
  VariableList params;
  auto* f = Func("func", params, ty.void_(), StatementList{}, DecorationList{});

  EXPECT_EQ(f->get_last_statement(), nullptr);
}

using FunctionListTest = TestHelper;

TEST_F(FunctionListTest, FindSymbol) {
  auto* func = Func("main", VariableList{}, ty.f32(), StatementList{},
                    ast::DecorationList{});
  FunctionList list;
  list.Add(func);
  EXPECT_EQ(func, list.Find(Symbols().Register("main")));
}

TEST_F(FunctionListTest, FindSymbolMissing) {
  FunctionList list;
  EXPECT_EQ(nullptr, list.Find(Symbols().Register("Missing")));
}

TEST_F(FunctionListTest, FindSymbolStage) {
  auto* fs = Func("main", VariableList{}, ty.f32(), StatementList{},
                  ast::DecorationList{
                      Stage(PipelineStage::kFragment),
                  });
  auto* vs = Func("main", VariableList{}, ty.f32(), StatementList{},
                  ast::DecorationList{
                      Stage(PipelineStage::kVertex),
                  });
  FunctionList list;
  list.Add(fs);
  list.Add(vs);
  EXPECT_EQ(fs,
            list.Find(Symbols().Register("main"), PipelineStage::kFragment));
  EXPECT_EQ(vs, list.Find(Symbols().Register("main"), PipelineStage::kVertex));
}

TEST_F(FunctionListTest, FindSymbolStageMissing) {
  FunctionList list;
  list.Add(Func("main", VariableList{}, ty.f32(), StatementList{},
                ast::DecorationList{
                    Stage(PipelineStage::kFragment),
                }));
  EXPECT_EQ(nullptr,
            list.Find(Symbols().Register("main"), PipelineStage::kVertex));
}

TEST_F(FunctionListTest, HasStage) {
  FunctionList list;
  list.Add(Func("main", VariableList{}, ty.f32(), StatementList{},
                ast::DecorationList{
                    Stage(PipelineStage::kFragment),
                }));
  EXPECT_TRUE(list.HasStage(PipelineStage::kFragment));
  EXPECT_FALSE(list.HasStage(PipelineStage::kVertex));
}

}  // namespace
}  // namespace ast
}  // namespace tint
