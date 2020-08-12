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

#include "src/ast/function.h"

#include "gtest/gtest.h"
#include "src/ast/builtin_decoration.h"
#include "src/ast/decorated_variable.h"
#include "src/ast/discard_statement.h"
#include "src/ast/location_decoration.h"
#include "src/ast/pipeline_stage.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/i32_type.h"
#include "src/ast/type/void_type.h"
#include "src/ast/variable.h"

namespace tint {
namespace ast {
namespace {

using FunctionTest = testing::Test;

TEST_F(FunctionTest, Creation) {
  type::VoidType void_type;
  type::I32Type i32;

  VariableList params;
  params.push_back(
      std::make_unique<Variable>("var", StorageClass::kNone, &i32));
  auto* var_ptr = params[0].get();

  Function f("func", std::move(params), &void_type);
  EXPECT_EQ(f.name(), "func");
  ASSERT_EQ(f.params().size(), 1u);
  EXPECT_EQ(f.return_type(), &void_type);
  EXPECT_EQ(f.params()[0].get(), var_ptr);
}

TEST_F(FunctionTest, Creation_WithSource) {
  type::VoidType void_type;
  type::I32Type i32;

  VariableList params;
  params.push_back(
      std::make_unique<Variable>("var", StorageClass::kNone, &i32));

  Function f(Source{20, 2}, "func", std::move(params), &void_type);
  auto src = f.source();
  EXPECT_EQ(src.line, 20u);
  EXPECT_EQ(src.column, 2u);
}

TEST_F(FunctionTest, AddDuplicateReferencedVariables) {
  type::VoidType void_type;
  type::I32Type i32;

  Variable v("var", StorageClass::kInput, &i32);
  Function f("func", VariableList{}, &void_type);

  f.add_referenced_module_variable(&v);
  ASSERT_EQ(f.referenced_module_variables().size(), 1u);
  EXPECT_EQ(f.referenced_module_variables()[0], &v);

  f.add_referenced_module_variable(&v);
  ASSERT_EQ(f.referenced_module_variables().size(), 1u);

  Variable v2("var2", StorageClass::kOutput, &i32);
  f.add_referenced_module_variable(&v2);
  ASSERT_EQ(f.referenced_module_variables().size(), 2u);
  EXPECT_EQ(f.referenced_module_variables()[1], &v2);
}

TEST_F(FunctionTest, GetReferenceLocations) {
  type::VoidType void_type;
  type::I32Type i32;

  VariableDecorationList decos;
  DecoratedVariable loc1(
      std::make_unique<ast::Variable>("loc1", StorageClass::kInput, &i32));
  decos.push_back(std::make_unique<ast::LocationDecoration>(0));
  loc1.set_decorations(std::move(decos));

  DecoratedVariable loc2(
      std::make_unique<ast::Variable>("loc2", StorageClass::kInput, &i32));
  decos.push_back(std::make_unique<ast::LocationDecoration>(1));
  loc2.set_decorations(std::move(decos));

  DecoratedVariable builtin1(
      std::make_unique<ast::Variable>("builtin1", StorageClass::kInput, &i32));
  decos.push_back(
      std::make_unique<ast::BuiltinDecoration>(ast::Builtin::kPosition));
  builtin1.set_decorations(std::move(decos));

  DecoratedVariable builtin2(
      std::make_unique<ast::Variable>("builtin2", StorageClass::kInput, &i32));
  decos.push_back(
      std::make_unique<ast::BuiltinDecoration>(ast::Builtin::kFragDepth));
  builtin2.set_decorations(std::move(decos));

  Function f("func", VariableList{}, &void_type);

  f.add_referenced_module_variable(&loc1);
  f.add_referenced_module_variable(&builtin1);
  f.add_referenced_module_variable(&loc2);
  f.add_referenced_module_variable(&builtin2);
  ASSERT_EQ(f.referenced_module_variables().size(), 4u);

  auto ref_locs = f.referenced_location_variables();
  ASSERT_EQ(ref_locs.size(), 2u);
  EXPECT_EQ(ref_locs[0].first, &loc1);
  EXPECT_EQ(ref_locs[0].second->value(), 0u);
  EXPECT_EQ(ref_locs[1].first, &loc2);
  EXPECT_EQ(ref_locs[1].second->value(), 1u);
}

TEST_F(FunctionTest, GetReferenceBuiltins) {
  type::VoidType void_type;
  type::I32Type i32;

  VariableDecorationList decos;
  DecoratedVariable loc1(
      std::make_unique<ast::Variable>("loc1", StorageClass::kInput, &i32));
  decos.push_back(std::make_unique<ast::LocationDecoration>(0));
  loc1.set_decorations(std::move(decos));

  DecoratedVariable loc2(
      std::make_unique<ast::Variable>("loc2", StorageClass::kInput, &i32));
  decos.push_back(std::make_unique<ast::LocationDecoration>(1));
  loc2.set_decorations(std::move(decos));

  DecoratedVariable builtin1(
      std::make_unique<ast::Variable>("builtin1", StorageClass::kInput, &i32));
  decos.push_back(
      std::make_unique<ast::BuiltinDecoration>(ast::Builtin::kPosition));
  builtin1.set_decorations(std::move(decos));

  DecoratedVariable builtin2(
      std::make_unique<ast::Variable>("builtin2", StorageClass::kInput, &i32));
  decos.push_back(
      std::make_unique<ast::BuiltinDecoration>(ast::Builtin::kFragDepth));
  builtin2.set_decorations(std::move(decos));

  Function f("func", VariableList{}, &void_type);

  f.add_referenced_module_variable(&loc1);
  f.add_referenced_module_variable(&builtin1);
  f.add_referenced_module_variable(&loc2);
  f.add_referenced_module_variable(&builtin2);
  ASSERT_EQ(f.referenced_module_variables().size(), 4u);

  auto ref_locs = f.referenced_builtin_variables();
  ASSERT_EQ(ref_locs.size(), 2u);
  EXPECT_EQ(ref_locs[0].first, &builtin1);
  EXPECT_EQ(ref_locs[0].second->value(), ast::Builtin::kPosition);
  EXPECT_EQ(ref_locs[1].first, &builtin2);
  EXPECT_EQ(ref_locs[1].second->value(), ast::Builtin::kFragDepth);
}

TEST_F(FunctionTest, AddDuplicateEntryPoints) {
  ast::type::VoidType void_type;
  Function f("func", VariableList{}, &void_type);

  f.add_ancestor_entry_point("main");
  ASSERT_EQ(1u, f.ancestor_entry_points().size());
  EXPECT_EQ("main", f.ancestor_entry_points()[0]);

  f.add_ancestor_entry_point("main");
  ASSERT_EQ(1u, f.ancestor_entry_points().size());
  EXPECT_EQ("main", f.ancestor_entry_points()[0]);
}

TEST_F(FunctionTest, IsValid) {
  type::VoidType void_type;
  type::I32Type i32;

  VariableList params;
  params.push_back(
      std::make_unique<Variable>("var", StorageClass::kNone, &i32));

  auto block = std::make_unique<ast::BlockStatement>();
  block->append(std::make_unique<DiscardStatement>());

  Function f("func", std::move(params), &void_type);
  f.set_body(std::move(block));
  EXPECT_TRUE(f.IsValid());
}

TEST_F(FunctionTest, IsValid_EmptyName) {
  type::VoidType void_type;
  type::I32Type i32;

  VariableList params;
  params.push_back(
      std::make_unique<Variable>("var", StorageClass::kNone, &i32));

  Function f("", std::move(params), &void_type);
  EXPECT_FALSE(f.IsValid());
}

TEST_F(FunctionTest, IsValid_MissingReturnType) {
  type::I32Type i32;

  VariableList params;
  params.push_back(
      std::make_unique<Variable>("var", StorageClass::kNone, &i32));

  Function f("func", std::move(params), nullptr);
  EXPECT_FALSE(f.IsValid());
}

TEST_F(FunctionTest, IsValid_NullParam) {
  type::VoidType void_type;
  type::I32Type i32;

  VariableList params;
  params.push_back(
      std::make_unique<Variable>("var", StorageClass::kNone, &i32));
  params.push_back(nullptr);

  Function f("func", std::move(params), &void_type);
  EXPECT_FALSE(f.IsValid());
}

TEST_F(FunctionTest, IsValid_InvalidParam) {
  type::VoidType void_type;

  VariableList params;
  params.push_back(
      std::make_unique<Variable>("var", StorageClass::kNone, nullptr));

  Function f("func", std::move(params), &void_type);
  EXPECT_FALSE(f.IsValid());
}

TEST_F(FunctionTest, IsValid_NullBodyStatement) {
  type::VoidType void_type;
  type::I32Type i32;

  VariableList params;
  params.push_back(
      std::make_unique<Variable>("var", StorageClass::kNone, &i32));

  auto block = std::make_unique<ast::BlockStatement>();
  block->append(std::make_unique<DiscardStatement>());
  block->append(nullptr);

  Function f("func", std::move(params), &void_type);
  f.set_body(std::move(block));
  EXPECT_FALSE(f.IsValid());
}

TEST_F(FunctionTest, IsValid_InvalidBodyStatement) {
  type::VoidType void_type;
  type::I32Type i32;

  VariableList params;
  params.push_back(
      std::make_unique<Variable>("var", StorageClass::kNone, &i32));

  auto block = std::make_unique<ast::BlockStatement>();
  block->append(std::make_unique<DiscardStatement>());
  block->append(nullptr);

  Function f("func", std::move(params), &void_type);
  f.set_body(std::move(block));
  EXPECT_FALSE(f.IsValid());
}

TEST_F(FunctionTest, ToStr) {
  type::VoidType void_type;
  type::I32Type i32;

  auto block = std::make_unique<ast::BlockStatement>();
  block->append(std::make_unique<DiscardStatement>());

  Function f("func", {}, &void_type);
  f.set_body(std::move(block));

  std::ostringstream out;
  f.to_str(out, 2);
  EXPECT_EQ(out.str(), R"(  Function func -> __void
  ()
  {
    Discard{}
  }
)");
}

TEST_F(FunctionTest, ToStr_WithParams) {
  type::VoidType void_type;
  type::I32Type i32;

  VariableList params;
  params.push_back(
      std::make_unique<Variable>("var", StorageClass::kNone, &i32));

  auto block = std::make_unique<ast::BlockStatement>();
  block->append(std::make_unique<DiscardStatement>());

  Function f("func", std::move(params), &void_type);
  f.set_body(std::move(block));

  std::ostringstream out;
  f.to_str(out, 2);
  EXPECT_EQ(out.str(), R"(  Function func -> __void
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
  type::VoidType void_type;

  Function f("func", {}, &void_type);
  EXPECT_EQ(f.type_name(), "__func__void");
}

TEST_F(FunctionTest, TypeName_WithParams) {
  type::VoidType void_type;
  type::I32Type i32;
  type::F32Type f32;

  VariableList params;
  params.push_back(
      std::make_unique<Variable>("var1", StorageClass::kNone, &i32));
  params.push_back(
      std::make_unique<Variable>("var2", StorageClass::kNone, &f32));

  Function f("func", std::move(params), &void_type);
  EXPECT_EQ(f.type_name(), "__func__void__i32__f32");
}

TEST_F(FunctionTest, GetLastStatement) {
  type::VoidType void_type;

  VariableList params;
  auto body = std::make_unique<ast::BlockStatement>();
  auto stmt = std::make_unique<DiscardStatement>();
  auto* stmt_ptr = stmt.get();
  body->append(std::move(stmt));
  Function f("func", std::move(params), &void_type);
  f.set_body(std::move(body));

  EXPECT_EQ(f.get_last_statement(), stmt_ptr);
}

TEST_F(FunctionTest, GetLastStatement_nullptr) {
  type::VoidType void_type;

  VariableList params;
  auto body = std::make_unique<ast::BlockStatement>();
  Function f("func", std::move(params), &void_type);
  f.set_body(std::move(body));

  EXPECT_EQ(f.get_last_statement(), nullptr);
}
}  // namespace
}  // namespace ast
}  // namespace tint
