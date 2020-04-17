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

#include "gtest/gtest.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/i32_type.h"

namespace tint {
namespace ast {
namespace {

using VariableTest = testing::Test;

TEST_F(VariableTest, Creation) {
  type::I32Type t;
  Variable v("my_var", StorageClass::kFunction, &t);

  EXPECT_EQ(v.name(), "my_var");
  EXPECT_EQ(v.storage_class(), StorageClass::kFunction);
  EXPECT_EQ(v.type(), &t);
  EXPECT_EQ(v.line(), 0u);
  EXPECT_EQ(v.column(), 0u);
}

TEST_F(VariableTest, CreationWithSource) {
  Source s{27, 4};
  type::F32Type t;
  Variable v(s, "i", StorageClass::kPrivate, &t);

  EXPECT_EQ(v.name(), "i");
  EXPECT_EQ(v.storage_class(), StorageClass::kPrivate);
  EXPECT_EQ(v.type(), &t);
  EXPECT_EQ(v.line(), 27u);
  EXPECT_EQ(v.column(), 4u);
}

TEST_F(VariableTest, CreationEmpty) {
  Source s{27, 4};
  Variable v;
  v.set_source(s);
  v.set_storage_class(StorageClass::kWorkgroup);
  v.set_name("a_var");

  type::I32Type t;
  v.set_type(&t);

  EXPECT_EQ(v.name(), "a_var");
  EXPECT_EQ(v.storage_class(), StorageClass::kWorkgroup);
  EXPECT_EQ(v.type(), &t);
  EXPECT_EQ(v.line(), 27u);
  EXPECT_EQ(v.column(), 4u);
}

TEST_F(VariableTest, IsValid) {
  type::I32Type t;
  Variable v{"my_var", StorageClass::kNone, &t};
  EXPECT_TRUE(v.IsValid());
}

TEST_F(VariableTest, IsValid_WithConstructor) {
  type::I32Type t;
  Variable v{"my_var", StorageClass::kNone, &t};
  v.set_constructor(std::make_unique<IdentifierExpression>("ident"));
  EXPECT_TRUE(v.IsValid());
}

TEST_F(VariableTest, IsValid_MissinName) {
  type::I32Type t;
  Variable v{"", StorageClass::kNone, &t};
  EXPECT_FALSE(v.IsValid());
}

TEST_F(VariableTest, IsValid_MissingType) {
  Variable v{"x", StorageClass::kNone, nullptr};
  EXPECT_FALSE(v.IsValid());
}

TEST_F(VariableTest, IsValid_MissingBoth) {
  Variable v;
  EXPECT_FALSE(v.IsValid());
}

TEST_F(VariableTest, IsValid_InvalidConstructor) {
  type::I32Type t;
  Variable v{"my_var", StorageClass::kNone, &t};
  v.set_constructor(std::make_unique<IdentifierExpression>(""));
  EXPECT_FALSE(v.IsValid());
}

TEST_F(VariableTest, to_str) {
  type::F32Type t;
  Variable v{"my_var", StorageClass::kFunction, &t};
  std::ostringstream out;
  v.to_str(out, 2);
  EXPECT_EQ(out.str(), R"(  Variable{
    my_var
    function
    __f32
  }
)");
}

}  // namespace
}  // namespace ast
}  // namespace tint
