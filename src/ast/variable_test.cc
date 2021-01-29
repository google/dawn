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

#include "src/ast/variable.h"

#include "src/ast/constant_id_decoration.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/test_helper.h"
#include "src/type/f32_type.h"
#include "src/type/i32_type.h"

namespace tint {
namespace ast {
namespace {

using VariableTest = TestHelper;

TEST_F(VariableTest, Creation) {
  auto* v = Var("my_var", StorageClass::kFunction, ty.i32());

  EXPECT_EQ(v->symbol(), Symbol(1));
  EXPECT_EQ(v->storage_class(), StorageClass::kFunction);
  EXPECT_EQ(v->type(), ty.i32());
  EXPECT_EQ(v->source().range.begin.line, 0u);
  EXPECT_EQ(v->source().range.begin.column, 0u);
  EXPECT_EQ(v->source().range.end.line, 0u);
  EXPECT_EQ(v->source().range.end.column, 0u);
}

TEST_F(VariableTest, CreationWithSource) {
  auto* v = Var(
      Source{Source::Range{Source::Location{27, 4}, Source::Location{27, 5}}},
      "i", StorageClass::kPrivate, ty.f32(), nullptr, VariableDecorationList{});

  EXPECT_EQ(v->symbol(), Symbol(1));
  EXPECT_EQ(v->storage_class(), StorageClass::kPrivate);
  EXPECT_EQ(v->type(), ty.f32());
  EXPECT_EQ(v->source().range.begin.line, 27u);
  EXPECT_EQ(v->source().range.begin.column, 4u);
  EXPECT_EQ(v->source().range.end.line, 27u);
  EXPECT_EQ(v->source().range.end.column, 5u);
}

TEST_F(VariableTest, CreationEmpty) {
  auto* v = Var(
      Source{Source::Range{Source::Location{27, 4}, Source::Location{27, 7}}},
      "a_var", StorageClass::kWorkgroup, ty.i32(), nullptr,
      VariableDecorationList{});

  EXPECT_EQ(v->symbol(), Symbol(1));
  EXPECT_EQ(v->storage_class(), StorageClass::kWorkgroup);
  EXPECT_EQ(v->type(), ty.i32());
  EXPECT_EQ(v->source().range.begin.line, 27u);
  EXPECT_EQ(v->source().range.begin.column, 4u);
  EXPECT_EQ(v->source().range.end.line, 27u);
  EXPECT_EQ(v->source().range.end.column, 7u);
}

TEST_F(VariableTest, IsValid) {
  auto* v = Var("my_var", StorageClass::kNone, ty.i32());
  EXPECT_TRUE(v->IsValid());
}

TEST_F(VariableTest, IsValid_WithConstructor) {
  auto* v = Var("my_var", StorageClass::kNone, ty.i32(), Expr("ident"),
                ast::VariableDecorationList{});
  EXPECT_TRUE(v->IsValid());
}

TEST_F(VariableTest, IsValid_MissingSymbol) {
  auto* v = Var("", StorageClass::kNone, ty.i32());
  EXPECT_FALSE(v->IsValid());
}

TEST_F(VariableTest, IsValid_MissingType) {
  auto* v = Var("x", StorageClass::kNone, nullptr);
  EXPECT_FALSE(v->IsValid());
}

TEST_F(VariableTest, IsValid_MissingBoth) {
  auto* v = Var("", StorageClass::kNone, nullptr);
  EXPECT_FALSE(v->IsValid());
}

TEST_F(VariableTest, IsValid_InvalidConstructor) {
  auto* v = Var("my_var", StorageClass::kNone, ty.i32(), Expr(""),
                ast::VariableDecorationList{});
  EXPECT_FALSE(v->IsValid());
}

TEST_F(VariableTest, to_str) {
  auto* v = Var("my_var", StorageClass::kFunction, ty.f32(), nullptr,
                ast::VariableDecorationList{});
  std::ostringstream out;
  v->to_str(Sem(), out, 2);
  EXPECT_EQ(demangle(out.str()), R"(  Variable{
    my_var
    function
    __f32
  }
)");
}

TEST_F(VariableTest, WithDecorations) {
  auto* var = Var("my_var", StorageClass::kFunction, ty.i32(), nullptr,
                  VariableDecorationList{
                      create<LocationDecoration>(1),
                      create<BuiltinDecoration>(Builtin::kPosition),
                      create<ConstantIdDecoration>(1200),
                  });

  EXPECT_TRUE(var->HasLocationDecoration());
  EXPECT_TRUE(var->HasBuiltinDecoration());
  EXPECT_TRUE(var->HasConstantIdDecoration());

  auto* location = var->GetLocationDecoration();
  ASSERT_NE(nullptr, location);
  EXPECT_EQ(1u, location->value());
}

TEST_F(VariableTest, ConstantId) {
  auto* var = Var("my_var", StorageClass::kFunction, ty.i32(), nullptr,
                  VariableDecorationList{
                      create<ConstantIdDecoration>(1200),
                  });

  EXPECT_EQ(var->constant_id(), 1200u);
}

TEST_F(VariableTest, Decorated_to_str) {
  auto* var = Var("my_var", StorageClass::kFunction, ty.f32(), Expr("expr"),
                  VariableDecorationList{
                      create<BindingDecoration>(2),
                      create<GroupDecoration>(1),
                  });

  std::ostringstream out;
  var->to_str(Sem(), out, 2);
  EXPECT_EQ(demangle(out.str()), R"(  Variable{
    Decorations{
      BindingDecoration{2}
      GroupDecoration{1}
    }
    my_var
    function
    __f32
    {
      Identifier[not set]{expr}
    }
  }
)");
}

}  // namespace
}  // namespace ast
}  // namespace tint
