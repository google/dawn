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

#include <string>

#include "gtest/gtest.h"
#include "spirv/unified1/spirv.h"
#include "spirv/unified1/spirv.hpp11"
#include "src/ast/function.h"
#include "src/ast/type/void_type.h"
#include "src/writer/spirv/builder.h"
#include "src/writer/spirv/spv_dump.h"

namespace tint {
namespace writer {
namespace spirv {
namespace {

using BuilderTest = testing::Test;

TEST_F(BuilderTest, Function_Empty) {
  ast::type::VoidType void_type;
  ast::Function func("a_func", {}, &void_type);

  Builder b;
  ASSERT_TRUE(b.GenerateFunction(&func));

  EXPECT_EQ(DumpInstructions(b.debug()), R"(OpName %3 "a_func"
)");
  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeVoid
%1 = OpTypeFunction %2
)");
  EXPECT_EQ(DumpInstructions(b.instructions()), R"(%3 = OpFunction %2 None %1
%4 = OpLabel
OpFunctionEnd
)");
}

TEST_F(BuilderTest, DISABLED_Function_WithParams) {}

TEST_F(BuilderTest, DISABLED_Function_WithBody) {}

TEST_F(BuilderTest, FunctionType) {
  ast::type::VoidType void_type;
  ast::Function func("a_func", {}, &void_type);

  Builder b;
  ASSERT_TRUE(b.GenerateFunction(&func));
  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeVoid
%1 = OpTypeFunction %2
)");
}

TEST_F(BuilderTest, FunctionType_DeDuplicate) {
  ast::type::VoidType void_type;
  ast::Function func1("a_func", {}, &void_type);
  ast::Function func2("b_func", {}, &void_type);

  Builder b;
  ASSERT_TRUE(b.GenerateFunction(&func1));
  ASSERT_TRUE(b.GenerateFunction(&func2));
  EXPECT_EQ(DumpInstructions(b.types()), R"(%2 = OpTypeVoid
%1 = OpTypeFunction %2
)");
}

}  // namespace
}  // namespace spirv
}  // namespace writer
}  // namespace tint
