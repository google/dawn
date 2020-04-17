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

#include "src/ast/decorated_variable.h"

#include "gtest/gtest.h"
#include "src/ast/binding_decoration.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/set_decoration.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/i32_type.h"
#include "src/ast/variable.h"
#include "src/ast/variable_decoration.h"

namespace tint {
namespace ast {
namespace {

using DecoratedVariableTest = testing::Test;

TEST_F(DecoratedVariableTest, Creation) {
  type::I32Type t;
  auto var = std::make_unique<Variable>("my_var", StorageClass::kFunction, &t);
  DecoratedVariable dv(std::move(var));

  EXPECT_EQ(dv.name(), "my_var");
  EXPECT_EQ(dv.storage_class(), StorageClass::kFunction);
  EXPECT_EQ(dv.type(), &t);
  EXPECT_EQ(dv.line(), 0u);
  EXPECT_EQ(dv.column(), 0u);
}

TEST_F(DecoratedVariableTest, CreationWithSource) {
  Source s{27, 4};
  type::F32Type t;
  auto var = std::make_unique<Variable>(s, "i", StorageClass::kPrivate, &t);
  DecoratedVariable dv(std::move(var));

  EXPECT_EQ(dv.name(), "i");
  EXPECT_EQ(dv.storage_class(), StorageClass::kPrivate);
  EXPECT_EQ(dv.type(), &t);
  EXPECT_EQ(dv.line(), 27u);
  EXPECT_EQ(dv.column(), 4u);
}

TEST_F(DecoratedVariableTest, IsValid) {
  type::I32Type t;
  auto var = std::make_unique<Variable>("my_var", StorageClass::kNone, &t);
  DecoratedVariable dv(std::move(var));
  EXPECT_TRUE(dv.IsValid());
}

TEST_F(DecoratedVariableTest, IsDecorated) {
  DecoratedVariable dv;
  EXPECT_TRUE(dv.IsDecorated());
}

TEST_F(DecoratedVariableTest, to_str) {
  type::F32Type t;
  auto var = std::make_unique<Variable>("my_var", StorageClass::kFunction, &t);
  DecoratedVariable dv(std::move(var));
  dv.set_constructor(std::make_unique<IdentifierExpression>("expr"));

  VariableDecorationList decos;
  decos.push_back(std::make_unique<BindingDecoration>(2));
  decos.push_back(std::make_unique<SetDecoration>(1));

  dv.set_decorations(std::move(decos));
  std::ostringstream out;
  dv.to_str(out, 2);
  EXPECT_EQ(out.str(), R"(  DecoratedVariable{
    Decorations{
      BindingDecoration{2}
      SetDecoration{1}
    }
    my_var
    function
    __f32
    {
      Identifier{expr}
    }
  }
)");
}

}  // namespace
}  // namespace ast
}  // namespace tint
