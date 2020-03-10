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
#include "src/ast/nop_statement.h"
#include "src/ast/type/i32_type.h"
#include "src/ast/type/void_type.h"
#include "src/ast/variable.h"

namespace tint {
namespace ast {

using FunctionTest = testing::Test;

TEST_F(FunctionTest, Creation) {
  type::VoidType void_type;
  type::I32Type i32;

  std::vector<std::unique_ptr<Variable>> params;
  params.push_back(
      std::make_unique<Variable>("var", StorageClass::kNone, &i32));
  auto var_ptr = params[0].get();

  Function f("func", std::move(params), &void_type);
  EXPECT_EQ(f.name(), "func");
  ASSERT_EQ(f.params().size(), 1);
  EXPECT_EQ(f.return_type(), &void_type);
  EXPECT_EQ(f.params()[0].get(), var_ptr);
}

TEST_F(FunctionTest, Creation_WithSource) {
  type::VoidType void_type;
  type::I32Type i32;

  std::vector<std::unique_ptr<Variable>> params;
  params.push_back(
      std::make_unique<Variable>("var", StorageClass::kNone, &i32));

  Function f(Source{20, 2}, "func", std::move(params), &void_type);
  auto src = f.source();
  EXPECT_EQ(src.line, 20);
  EXPECT_EQ(src.column, 2);
}

TEST_F(FunctionTest, IsValid) {
  type::VoidType void_type;
  type::I32Type i32;

  std::vector<std::unique_ptr<Variable>> params;
  params.push_back(
      std::make_unique<Variable>("var", StorageClass::kNone, &i32));

  std::vector<std::unique_ptr<Statement>> body;
  body.push_back(std::make_unique<NopStatement>());

  Function f("func", std::move(params), &void_type);
  f.set_body(std::move(body));
  EXPECT_TRUE(f.IsValid());
}

TEST_F(FunctionTest, IsValid_EmptyName) {
  type::VoidType void_type;
  type::I32Type i32;

  std::vector<std::unique_ptr<Variable>> params;
  params.push_back(
      std::make_unique<Variable>("var", StorageClass::kNone, &i32));

  Function f("", std::move(params), &void_type);
  EXPECT_FALSE(f.IsValid());
}

TEST_F(FunctionTest, IsValid_MissingReturnType) {
  type::I32Type i32;

  std::vector<std::unique_ptr<Variable>> params;
  params.push_back(
      std::make_unique<Variable>("var", StorageClass::kNone, &i32));

  Function f("func", std::move(params), nullptr);
  EXPECT_FALSE(f.IsValid());
}

TEST_F(FunctionTest, IsValid_NullParam) {
  type::VoidType void_type;
  type::I32Type i32;

  std::vector<std::unique_ptr<Variable>> params;
  params.push_back(
      std::make_unique<Variable>("var", StorageClass::kNone, &i32));
  params.push_back(nullptr);

  Function f("func", std::move(params), &void_type);
  EXPECT_FALSE(f.IsValid());
}

TEST_F(FunctionTest, IsValid_InvalidParam) {
  type::VoidType void_type;

  std::vector<std::unique_ptr<Variable>> params;
  params.push_back(
      std::make_unique<Variable>("var", StorageClass::kNone, nullptr));

  Function f("func", std::move(params), &void_type);
  EXPECT_FALSE(f.IsValid());
}

TEST_F(FunctionTest, IsValid_NullBodyStatement) {
  type::VoidType void_type;
  type::I32Type i32;

  std::vector<std::unique_ptr<Variable>> params;
  params.push_back(
      std::make_unique<Variable>("var", StorageClass::kNone, &i32));

  std::vector<std::unique_ptr<Statement>> body;
  body.push_back(std::make_unique<NopStatement>());
  body.push_back(nullptr);

  Function f("func", std::move(params), &void_type);
  f.set_body(std::move(body));
  EXPECT_FALSE(f.IsValid());
}

TEST_F(FunctionTest, IsValid_InvalidBodyStatement) {
  type::VoidType void_type;
  type::I32Type i32;

  std::vector<std::unique_ptr<Variable>> params;
  params.push_back(
      std::make_unique<Variable>("var", StorageClass::kNone, &i32));

  std::vector<std::unique_ptr<Statement>> body;
  body.push_back(std::make_unique<NopStatement>());
  body.push_back(nullptr);

  Function f("func", std::move(params), &void_type);
  f.set_body(std::move(body));
  EXPECT_FALSE(f.IsValid());
}

TEST_F(FunctionTest, ToStr) {
  type::VoidType void_type;
  type::I32Type i32;

  std::vector<std::unique_ptr<Statement>> body;
  body.push_back(std::make_unique<NopStatement>());

  Function f("func", {}, &void_type);
  f.set_body(std::move(body));

  std::ostringstream out;
  f.to_str(out, 2);
  EXPECT_EQ(out.str(), R"(  Function func -> __void
  ()
  {
    Nop{}
  }
)");
}

TEST_F(FunctionTest, ToStr_WithParams) {
  type::VoidType void_type;
  type::I32Type i32;

  std::vector<std::unique_ptr<Variable>> params;
  params.push_back(
      std::make_unique<Variable>("var", StorageClass::kNone, &i32));

  std::vector<std::unique_ptr<Statement>> body;
  body.push_back(std::make_unique<NopStatement>());

  Function f("func", std::move(params), &void_type);
  f.set_body(std::move(body));

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
    Nop{}
  }
)");
}

}  // namespace ast
}  // namespace tint
