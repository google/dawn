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
  params.push_back(Var("var", ty.i32(), StorageClass::kNone));
  auto* var = params[0];

  auto* f = Func("func", params, ty.void_(), StatementList{},
                 FunctionDecorationList{});
  EXPECT_EQ(f->symbol(), Symbols().Get("func"));
  ASSERT_EQ(f->params().size(), 1u);
  EXPECT_EQ(f->return_type(), ty.void_());
  EXPECT_EQ(f->params()[0], var);
}

TEST_F(FunctionTest, Creation_WithSource) {
  VariableList params;
  params.push_back(Var("var", ty.i32(), StorageClass::kNone));

  auto* f = Func(Source{Source::Location{20, 2}}, "func", params, ty.void_(),
                 StatementList{}, FunctionDecorationList{});
  auto src = f->source();
  EXPECT_EQ(src.range.begin.line, 20u);
  EXPECT_EQ(src.range.begin.column, 2u);
}

TEST_F(FunctionTest, IsValid) {
  VariableList params;
  params.push_back(Var("var", ty.i32(), StorageClass::kNone));

  auto* f = Func("func", params, ty.void_(),
                 StatementList{
                     create<DiscardStatement>(),
                 },
                 FunctionDecorationList{});
  EXPECT_TRUE(f->IsValid());
}

TEST_F(FunctionTest, IsValid_InvalidName) {
  VariableList params;
  params.push_back(Var("var", ty.i32(), StorageClass::kNone));

  auto* f =
      Func("", params, ty.void_(), StatementList{}, FunctionDecorationList{});
  EXPECT_FALSE(f->IsValid());
}

TEST_F(FunctionTest, IsValid_MissingReturnType) {
  VariableList params;
  params.push_back(Var("var", ty.i32(), StorageClass::kNone));

  auto* f =
      Func("func", params, nullptr, StatementList{}, FunctionDecorationList{});
  EXPECT_FALSE(f->IsValid());
}

TEST_F(FunctionTest, IsValid_NullParam) {
  VariableList params;
  params.push_back(Var("var", ty.i32(), StorageClass::kNone));
  params.push_back(nullptr);

  auto* f = Func("func", params, ty.void_(), StatementList{},
                 FunctionDecorationList{});
  EXPECT_FALSE(f->IsValid());
}

TEST_F(FunctionTest, IsValid_InvalidParam) {
  VariableList params;
  params.push_back(Var("var", nullptr, StorageClass::kNone));

  auto* f = Func("func", params, ty.void_(), StatementList{},
                 FunctionDecorationList{});
  EXPECT_FALSE(f->IsValid());
}

TEST_F(FunctionTest, IsValid_NullBodyStatement) {
  VariableList params;
  params.push_back(Var("var", ty.i32(), StorageClass::kNone));

  auto* f = Func("func", params, ty.void_(),
                 StatementList{
                     create<DiscardStatement>(),
                     nullptr,
                 },
                 FunctionDecorationList{});

  EXPECT_FALSE(f->IsValid());
}

TEST_F(FunctionTest, IsValid_InvalidBodyStatement) {
  VariableList params;
  params.push_back(Var("var", ty.i32(), StorageClass::kNone));

  auto* f = Func("func", params, ty.void_(),
                 StatementList{
                     create<DiscardStatement>(),
                     nullptr,
                 },
                 FunctionDecorationList{});
  EXPECT_FALSE(f->IsValid());
}

TEST_F(FunctionTest, ToStr) {
  auto* f = Func("func", VariableList{}, ty.void_(),
                 StatementList{
                     create<DiscardStatement>(),
                 },
                 FunctionDecorationList{});

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
                 FunctionDecorationList{create<WorkgroupDecoration>(2, 4, 6)});

  EXPECT_EQ(str(f), R"(Function func -> __void
WorkgroupDecoration{2 4 6}
()
{
  Discard{}
}
)");
}

TEST_F(FunctionTest, ToStr_WithParams) {
  VariableList params;
  params.push_back(Var("var", ty.i32(), StorageClass::kNone));

  auto* f = Func("func", params, ty.void_(),
                 StatementList{
                     create<DiscardStatement>(),
                 },
                 FunctionDecorationList{});

  EXPECT_EQ(str(f), R"(Function func -> __void
(
  Variable{
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
                 FunctionDecorationList{});
  EXPECT_EQ(f->type_name(), "__func__void");
}

TEST_F(FunctionTest, TypeName_WithParams) {
  VariableList params;
  params.push_back(Var("var1", ty.i32(), StorageClass::kNone));
  params.push_back(Var("var2", ty.f32(), StorageClass::kNone));

  auto* f = Func("func", params, ty.void_(), StatementList{},
                 FunctionDecorationList{});
  EXPECT_EQ(f->type_name(), "__func__void__i32__f32");
}

TEST_F(FunctionTest, GetLastStatement) {
  VariableList params;
  auto* stmt = create<DiscardStatement>();
  auto* f = Func("func", params, ty.void_(), StatementList{stmt},
                 FunctionDecorationList{});

  EXPECT_EQ(f->get_last_statement(), stmt);
}

TEST_F(FunctionTest, GetLastStatement_nullptr) {
  VariableList params;
  auto* f = Func("func", params, ty.void_(), StatementList{},
                 FunctionDecorationList{});

  EXPECT_EQ(f->get_last_statement(), nullptr);
}

TEST_F(FunctionTest, WorkgroupSize_NoneSet) {
  auto* f = Func("func", VariableList{}, ty.void_(), StatementList{},
                 FunctionDecorationList{});
  uint32_t x = 0;
  uint32_t y = 0;
  uint32_t z = 0;
  std::tie(x, y, z) = f->workgroup_size();
  EXPECT_EQ(x, 1u);
  EXPECT_EQ(y, 1u);
  EXPECT_EQ(z, 1u);
}

TEST_F(FunctionTest, WorkgroupSize) {
  auto* f =
      Func("func", VariableList{}, ty.void_(), StatementList{},
           FunctionDecorationList{create<WorkgroupDecoration>(2u, 4u, 6u)});

  uint32_t x = 0;
  uint32_t y = 0;
  uint32_t z = 0;
  std::tie(x, y, z) = f->workgroup_size();
  EXPECT_EQ(x, 2u);
  EXPECT_EQ(y, 4u);
  EXPECT_EQ(z, 6u);
}

using FunctionListTest = TestHelper;

TEST_F(FunctionListTest, FindSymbol) {
  auto* func = Func("main", VariableList{}, ty.f32(), StatementList{},
                    ast::FunctionDecorationList{});
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
                  ast::FunctionDecorationList{
                      create<ast::StageDecoration>(PipelineStage::kFragment),
                  });
  auto* vs = Func("main", VariableList{}, ty.f32(), StatementList{},
                  ast::FunctionDecorationList{
                      create<ast::StageDecoration>(PipelineStage::kVertex),
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
                ast::FunctionDecorationList{
                    create<ast::StageDecoration>(PipelineStage::kFragment),
                }));
  EXPECT_EQ(nullptr,
            list.Find(Symbols().Register("main"), PipelineStage::kVertex));
}

TEST_F(FunctionListTest, HasStage) {
  FunctionList list;
  list.Add(Func("main", VariableList{}, ty.f32(), StatementList{},
                ast::FunctionDecorationList{
                    create<ast::StageDecoration>(PipelineStage::kFragment),
                }));
  EXPECT_TRUE(list.HasStage(PipelineStage::kFragment));
  EXPECT_FALSE(list.HasStage(PipelineStage::kVertex));
}

}  // namespace
}  // namespace ast
}  // namespace tint
