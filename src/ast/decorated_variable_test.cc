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

#include "src/ast/binding_decoration.h"
#include "src/ast/builtin_decoration.h"
#include "src/ast/constant_id_decoration.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/location_decoration.h"
#include "src/ast/set_decoration.h"
#include "src/ast/test_helper.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/i32_type.h"
#include "src/ast/variable.h"
#include "src/ast/variable_decoration.h"

namespace tint {
namespace ast {
namespace {

using DecoratedVariableTest = TestHelper;

TEST_F(DecoratedVariableTest, Creation) {
  type::I32Type t;
  auto var = create<Variable>("my_var", StorageClass::kFunction, &t);
  DecoratedVariable dv(std::move(var));

  EXPECT_EQ(dv.name(), "my_var");
  EXPECT_EQ(dv.storage_class(), StorageClass::kFunction);
  EXPECT_EQ(dv.type(), &t);
  EXPECT_EQ(dv.source().range.begin.line, 0u);
  EXPECT_EQ(dv.source().range.begin.column, 0u);
  EXPECT_EQ(dv.source().range.end.line, 0u);
  EXPECT_EQ(dv.source().range.end.column, 0u);
}

TEST_F(DecoratedVariableTest, CreationWithSource) {
  Source s{Source::Range{Source::Location{27, 4}, Source::Location{27, 5}}};
  type::F32Type t;
  auto var = create<Variable>(s, "i", StorageClass::kPrivate, &t);
  DecoratedVariable dv(std::move(var));

  EXPECT_EQ(dv.name(), "i");
  EXPECT_EQ(dv.storage_class(), StorageClass::kPrivate);
  EXPECT_EQ(dv.type(), &t);
  EXPECT_EQ(dv.source().range.begin.line, 27u);
  EXPECT_EQ(dv.source().range.begin.column, 4u);
  EXPECT_EQ(dv.source().range.end.line, 27u);
  EXPECT_EQ(dv.source().range.end.column, 5u);
}

TEST_F(DecoratedVariableTest, NoDecorations) {
  type::I32Type t;
  auto var = create<Variable>("my_var", StorageClass::kFunction, &t);
  DecoratedVariable dv(std::move(var));
  EXPECT_FALSE(dv.HasLocationDecoration());
  EXPECT_FALSE(dv.HasBuiltinDecoration());
  EXPECT_FALSE(dv.HasConstantIdDecoration());
}

TEST_F(DecoratedVariableTest, WithDecorations) {
  type::F32Type t;
  auto var = create<Variable>("my_var", StorageClass::kFunction, &t);
  DecoratedVariable dv(std::move(var));

  VariableDecorationList decos;
  decos.push_back(create<LocationDecoration>(1, Source{}));
  decos.push_back(create<BuiltinDecoration>(ast::Builtin::kPosition, Source{}));
  decos.push_back(create<ConstantIdDecoration>(1200, Source{}));

  dv.set_decorations(std::move(decos));

  EXPECT_TRUE(dv.HasLocationDecoration());
  EXPECT_TRUE(dv.HasBuiltinDecoration());
  EXPECT_TRUE(dv.HasConstantIdDecoration());
}

TEST_F(DecoratedVariableTest, ConstantId) {
  type::F32Type t;
  auto var = create<Variable>("my_var", StorageClass::kFunction, &t);
  DecoratedVariable dv(std::move(var));

  VariableDecorationList decos;
  decos.push_back(create<ConstantIdDecoration>(1200, Source{}));
  dv.set_decorations(std::move(decos));

  EXPECT_EQ(dv.constant_id(), 1200u);
}

TEST_F(DecoratedVariableTest, IsValid) {
  type::I32Type t;
  auto var = create<Variable>("my_var", StorageClass::kNone, &t);
  DecoratedVariable dv(std::move(var));
  EXPECT_TRUE(dv.IsValid());
}

TEST_F(DecoratedVariableTest, IsDecorated) {
  DecoratedVariable dv;
  EXPECT_TRUE(dv.IsDecorated());
}

TEST_F(DecoratedVariableTest, to_str) {
  type::F32Type t;
  auto var = create<Variable>("my_var", StorageClass::kFunction, &t);
  DecoratedVariable dv(std::move(var));
  dv.set_constructor(create<IdentifierExpression>("expr"));

  VariableDecorationList decos;
  decos.push_back(create<BindingDecoration>(2, Source{}));
  decos.push_back(create<SetDecoration>(1, Source{}));

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
      Identifier[not set]{expr}
    }
  }
)");
}

}  // namespace
}  // namespace ast
}  // namespace tint
