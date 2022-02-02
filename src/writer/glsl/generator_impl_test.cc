// Copyright 2021 The Tint Authors.
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

#include "src/writer/glsl/test_helper.h"

namespace tint {
namespace writer {
namespace glsl {
namespace {

using GlslGeneratorImplTest = TestHelper;

TEST_F(GlslGeneratorImplTest, Generate) {
  Func("my_func", ast::VariableList{}, ty.void_(), ast::StatementList{},
       ast::AttributeList{});

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_EQ(gen.result(), R"(#version 310 es
precision mediump float;

void my_func() {
}

)");
}

}  // namespace
}  // namespace glsl
}  // namespace writer
}  // namespace tint
