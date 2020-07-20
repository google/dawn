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

#include "src/writer/hlsl/generator_impl.h"

#include <memory>

#include "gtest/gtest.h"
#include "src/ast/entry_point.h"
#include "src/ast/function.h"
#include "src/ast/module.h"
#include "src/ast/type/void_type.h"

namespace tint {
namespace writer {
namespace hlsl {
namespace {

using HlslGeneratorImplTest = testing::Test;

TEST_F(HlslGeneratorImplTest, DISABLED_Generate) {
  ast::type::VoidType void_type;
  ast::Module m;
  m.AddFunction(std::make_unique<ast::Function>("my_func", ast::VariableList{},
                                                &void_type));
  m.AddEntryPoint(std::make_unique<ast::EntryPoint>(
      ast::PipelineStage::kFragment, "my_func", ""));

  GeneratorImpl g;

  ASSERT_TRUE(g.Generate(m)) << g.error();
  EXPECT_EQ(g.result(), R"(#import <metal_lib>

void my_func() {
}
)");
}

}  // namespace
}  // namespace hlsl
}  // namespace writer
}  // namespace tint
