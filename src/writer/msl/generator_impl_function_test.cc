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

#include "gtest/gtest.h"
#include "src/ast/function.h"
#include "src/ast/module.h"
#include "src/ast/return_statement.h"
#include "src/ast/type/array_type.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/i32_type.h"
#include "src/ast/type/void_type.h"
#include "src/ast/variable.h"
#include "src/writer/msl/generator_impl.h"

namespace tint {
namespace writer {
namespace msl {
namespace {

using MslGeneratorImplTest = testing::Test;

TEST_F(MslGeneratorImplTest, Emit_Function) {
  ast::type::VoidType void_type;

  auto func = std::make_unique<ast::Function>("my_func", ast::VariableList{},
                                              &void_type);

  ast::StatementList body;
  body.push_back(std::make_unique<ast::ReturnStatement>());
  func->set_body(std::move(body));

  ast::Module m;
  m.AddFunction(std::move(func));

  GeneratorImpl g;
  g.increment_indent();

  ASSERT_TRUE(g.Generate(m)) << g.error();
  EXPECT_EQ(g.result(), R"(  void my_func() {
    return;
  }

)");
}

TEST_F(MslGeneratorImplTest, Emit_Function_WithParams) {
  ast::type::F32Type f32;
  ast::type::I32Type i32;

  ast::VariableList params;
  params.push_back(
      std::make_unique<ast::Variable>("a", ast::StorageClass::kNone, &f32));
  params.push_back(
      std::make_unique<ast::Variable>("b", ast::StorageClass::kNone, &i32));

  ast::type::VoidType void_type;
  auto func =
      std::make_unique<ast::Function>("my_func", std::move(params), &void_type);

  ast::StatementList body;
  body.push_back(std::make_unique<ast::ReturnStatement>());
  func->set_body(std::move(body));

  ast::Module m;
  m.AddFunction(std::move(func));

  GeneratorImpl g;
  g.increment_indent();

  ASSERT_TRUE(g.Generate(m)) << g.error();
  EXPECT_EQ(g.result(), R"(  void my_func(float a, int b) {
    return;
  }

)");
}

TEST_F(MslGeneratorImplTest, Emit_Function_EntryPoint_NoName) {
  ast::type::VoidType void_type;

  auto func = std::make_unique<ast::Function>("frag_main", ast::VariableList{},
                                              &void_type);
  auto ep = std::make_unique<ast::EntryPoint>(ast::PipelineStage::kFragment, "",
                                              "frag_main");

  ast::Module m;
  m.AddFunction(std::move(func));
  m.AddEntryPoint(std::move(ep));

  GeneratorImpl g;
  ASSERT_TRUE(g.Generate(m)) << g.error();
  EXPECT_EQ(g.result(), R"(fragment void frag_main() {
}

)");
}

TEST_F(MslGeneratorImplTest, Emit_Function_EntryPoint_WithName) {
  ast::type::VoidType void_type;

  auto func = std::make_unique<ast::Function>("comp_main", ast::VariableList{},
                                              &void_type);
  auto ep = std::make_unique<ast::EntryPoint>(ast::PipelineStage::kCompute,
                                              "main", "comp_main");

  ast::Module m;
  m.AddFunction(std::move(func));
  m.AddEntryPoint(std::move(ep));

  GeneratorImpl g;
  ASSERT_TRUE(g.Generate(m)) << g.error();
  EXPECT_EQ(g.result(), R"(kernel void main() {
}

)");
}

TEST_F(MslGeneratorImplTest, Emit_Function_WithArrayParams) {
  ast::type::F32Type f32;
  ast::type::ArrayType ary(&f32, 5);

  ast::VariableList params;
  params.push_back(
      std::make_unique<ast::Variable>("a", ast::StorageClass::kNone, &ary));

  ast::type::VoidType void_type;
  auto func =
      std::make_unique<ast::Function>("my_func", std::move(params), &void_type);

  ast::StatementList body;
  body.push_back(std::make_unique<ast::ReturnStatement>());
  func->set_body(std::move(body));

  ast::Module m;
  m.AddFunction(std::move(func));

  GeneratorImpl g;
  g.increment_indent();

  ASSERT_TRUE(g.Generate(m)) << g.error();
  EXPECT_EQ(g.result(), R"(  void my_func(float a[5]) {
    return;
  }

)");
}

}  // namespace
}  // namespace msl
}  // namespace writer
}  // namespace tint
