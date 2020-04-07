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

#include "src/reader/spirv/function.h"

#include <string>
#include <vector>

#include "gmock/gmock.h"
#include "src/reader/spirv/parser_impl.h"
#include "src/reader/spirv/parser_impl_test_helper.h"
#include "src/reader/spirv/spirv_tools_helpers_test.h"

namespace tint {
namespace reader {
namespace spirv {
namespace {

using ::testing::HasSubstr;

/// @returns a SPIR-V assembly segment which assigns debug names
/// to particular IDs.
std::string Names(std::vector<std::string> ids) {
  std::ostringstream outs;
  for (auto& id : ids) {
    outs << "    OpName %" << id << " \"" << id << "\"\n";
  }
  return outs.str();
}

std::string CommonTypes() {
  return R"(
    %void = OpTypeVoid
    %voidfn = OpTypeFunction %void
    %float = OpTypeFloat 32
    %uint = OpTypeInt 32 0
    %int = OpTypeInt 32 1
    %float_0 = OpConstant %float 0.0
    %ptr_float = OpTypePointer Function %float
    %ptr_uint = OpTypePointer Function %uint
    %ptr_int = OpTypePointer Function %int
  )";
}

TEST_F(SpvParserTest, EmitFunctionVariables_AnonymousVars) {
  auto p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpVariable %ptr_uint Function
     %2 = OpVariable %ptr_uint Function
     %3 = OpVariable %ptr_uint Function
     OpReturn
     OpFunctionEnd
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(fe.EmitFunctionVariables());

  EXPECT_THAT(ToString(fe.ast_body()), HasSubstr(R"(VariableDeclStatement{
  Variable{
    x_1
    none
    __u32
  }
}
VariableDeclStatement{
  Variable{
    x_2
    none
    __u32
  }
}
VariableDeclStatement{
  Variable{
    x_3
    none
    __u32
  }
}
)"));
}

TEST_F(SpvParserTest, EmitFunctionVariables_NamedVars) {
  auto p = parser(test::Assemble(Names({"a", "b", "c"}) + CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %a = OpVariable %ptr_uint Function
     %b = OpVariable %ptr_uint Function
     %c = OpVariable %ptr_uint Function
     OpReturn
     OpFunctionEnd
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(fe.EmitFunctionVariables());

  EXPECT_THAT(ToString(fe.ast_body()), HasSubstr(R"(VariableDeclStatement{
  Variable{
    a
    none
    __u32
  }
}
VariableDeclStatement{
  Variable{
    b
    none
    __u32
  }
}
VariableDeclStatement{
  Variable{
    c
    none
    __u32
  }
}
)"));
}

TEST_F(SpvParserTest, EmitFunctionVariables_MixedTypes) {
  auto p = parser(test::Assemble(Names({"a", "b", "c"}) + CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %a = OpVariable %ptr_uint Function
     %b = OpVariable %ptr_int Function
     %c = OpVariable %ptr_float Function
     OpReturn
     OpFunctionEnd
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(fe.EmitFunctionVariables());

  EXPECT_THAT(ToString(fe.ast_body()), HasSubstr(R"(VariableDeclStatement{
  Variable{
    a
    none
    __u32
  }
}
VariableDeclStatement{
  Variable{
    b
    none
    __i32
  }
}
VariableDeclStatement{
  Variable{
    c
    none
    __f32
  }
}
)"));
}

}  // namespace
}  // namespace spirv
}  // namespace reader
}  // namespace tint
