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
#include "src/ast/type/f32_type.h"
#include "src/ast/type/i32_type.h"

namespace tint {
namespace ast {
namespace {

using VariableTest = TestHelper;

TEST_F(VariableTest, Creation) {
  type::I32 t;
  Variable v(Source{}, "my_var", StorageClass::kFunction, &t, false, nullptr,
             ast::VariableDecorationList{});

  EXPECT_EQ(v.name(), "my_var");
  EXPECT_EQ(v.storage_class(), StorageClass::kFunction);
  EXPECT_EQ(v.type(), &t);
  EXPECT_EQ(v.source().range.begin.line, 0u);
  EXPECT_EQ(v.source().range.begin.column, 0u);
  EXPECT_EQ(v.source().range.end.line, 0u);
  EXPECT_EQ(v.source().range.end.column, 0u);
}

TEST_F(VariableTest, CreationWithSource) {
  Source s{Source::Range{Source::Location{27, 4}, Source::Location{27, 5}}};
  type::F32 t;
  Variable v(s, "i", StorageClass::kPrivate, &t, false, nullptr,
             ast::VariableDecorationList{});

  EXPECT_EQ(v.name(), "i");
  EXPECT_EQ(v.storage_class(), StorageClass::kPrivate);
  EXPECT_EQ(v.type(), &t);
  EXPECT_EQ(v.source().range.begin.line, 27u);
  EXPECT_EQ(v.source().range.begin.column, 4u);
  EXPECT_EQ(v.source().range.end.line, 27u);
  EXPECT_EQ(v.source().range.end.column, 5u);
}

TEST_F(VariableTest, CreationEmpty) {
  type::I32 t;
  Source s{Source::Range{Source::Location{27, 4}, Source::Location{27, 7}}};
  Variable v(s, "a_var", StorageClass::kWorkgroup, &t, false, nullptr,
             ast::VariableDecorationList{});

  EXPECT_EQ(v.name(), "a_var");
  EXPECT_EQ(v.storage_class(), StorageClass::kWorkgroup);
  EXPECT_EQ(v.type(), &t);
  EXPECT_EQ(v.source().range.begin.line, 27u);
  EXPECT_EQ(v.source().range.begin.column, 4u);
  EXPECT_EQ(v.source().range.end.line, 27u);
  EXPECT_EQ(v.source().range.end.column, 7u);
}

TEST_F(VariableTest, IsValid) {
  type::I32 t;
  Variable v{Source{}, "my_var", StorageClass::kNone,          &t,
             false,    nullptr,  ast::VariableDecorationList{}};
  EXPECT_TRUE(v.IsValid());
}

TEST_F(VariableTest, IsValid_WithConstructor) {
  type::I32 t;
  Variable v{Source{},
             "my_var",
             StorageClass::kNone,
             &t,
             false,
             create<IdentifierExpression>(mod.RegisterSymbol("ident"), "ident"),
             ast::VariableDecorationList{}};
  EXPECT_TRUE(v.IsValid());
}

TEST_F(VariableTest, IsValid_MissinName) {
  type::I32 t;
  Variable v{Source{}, "",      StorageClass::kNone,          &t,
             false,    nullptr, ast::VariableDecorationList{}};
  EXPECT_FALSE(v.IsValid());
}

TEST_F(VariableTest, IsValid_MissingType) {
  Variable v{Source{}, "x",     StorageClass::kNone,          nullptr,
             false,    nullptr, ast::VariableDecorationList{}};
  EXPECT_FALSE(v.IsValid());
}

TEST_F(VariableTest, IsValid_MissingBoth) {
  Variable v{Source{}, "",      StorageClass::kNone,          nullptr,
             false,    nullptr, ast::VariableDecorationList{}};
  EXPECT_FALSE(v.IsValid());
}

TEST_F(VariableTest, IsValid_InvalidConstructor) {
  type::I32 t;
  Variable v{Source{},
             "my_var",
             StorageClass::kNone,
             &t,
             false,
             create<IdentifierExpression>(mod.RegisterSymbol(""), ""),
             ast::VariableDecorationList{}};
  EXPECT_FALSE(v.IsValid());
}

TEST_F(VariableTest, to_str) {
  type::F32 t;
  Variable v{Source{}, "my_var", StorageClass::kFunction,      &t,
             false,    nullptr,  ast::VariableDecorationList{}};
  std::ostringstream out;
  v.to_str(out, 2);
  EXPECT_EQ(out.str(), R"(  Variable{
    my_var
    function
    __f32
  }
)");
}

TEST_F(VariableTest, WithDecorations) {
  type::F32 t;
  auto* var = create<Variable>(
      Source{}, "my_var", StorageClass::kFunction, &t, false, nullptr,
      VariableDecorationList{
          create<LocationDecoration>(1, Source{}),
          create<BuiltinDecoration>(Builtin::kPosition, Source{}),
          create<ConstantIdDecoration>(1200, Source{}),
      });

  EXPECT_TRUE(var->HasLocationDecoration());
  EXPECT_TRUE(var->HasBuiltinDecoration());
  EXPECT_TRUE(var->HasConstantIdDecoration());
}

TEST_F(VariableTest, ConstantId) {
  type::F32 t;
  auto* var = create<Variable>(Source{}, "my_var", StorageClass::kFunction, &t,
                               false, nullptr,
                               VariableDecorationList{
                                   create<ConstantIdDecoration>(1200, Source{}),
                               });

  EXPECT_EQ(var->constant_id(), 1200u);
}

TEST_F(VariableTest, Decorated_to_str) {
  type::F32 t;
  auto* var = create<Variable>(
      Source{}, "my_var", StorageClass::kFunction, &t, false,
      create<IdentifierExpression>(mod.RegisterSymbol("expr"), "expr"),
      VariableDecorationList{
          create<BindingDecoration>(2, Source{}),
          create<SetDecoration>(1, Source{}),
      });

  std::ostringstream out;
  var->to_str(out, 2);
  EXPECT_EQ(demangle(out.str()), R"(  Variable{
    Decorations{
      BindingDecoration{2}
      SetDecoration{1}
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
