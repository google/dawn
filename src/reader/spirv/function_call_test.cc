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

using ::testing::Eq;
using ::testing::HasSubstr;

TEST_F(SpvParserTest, EmitStatement_VoidCallNoParams) {
  auto* p = parser(test::Assemble(R"(
     %void = OpTypeVoid
     %voidfn = OpTypeFunction %void

     %50 = OpFunction %void None %voidfn
     %entry_50 = OpLabel
     OpReturn
     OpFunctionEnd

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpFunctionCall %void %50
     OpReturn
     OpFunctionEnd
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModule()) << p->error();
  const auto module_ast_str = p->module().to_str();
  EXPECT_THAT(module_ast_str, Eq(R"(Module{
  Function x_50 -> __void
  ()
  {
    Return{}
  }
  Function x_100 -> __void
  ()
  {
    Call{
      Identifier{x_50}
      (
      )
    }
    Return{}
  }
}
)")) << module_ast_str;
}

TEST_F(SpvParserTest, EmitStatement_ScalarCallNoParams) {
  auto* p = parser(test::Assemble(R"(
     %void = OpTypeVoid
     %voidfn = OpTypeFunction %void
     %uint = OpTypeInt 32 0
     %uintfn = OpTypeFunction %uint
     %val = OpConstant %uint 42

     %50 = OpFunction %uint None %uintfn
     %entry_50 = OpLabel
     OpReturnValue %val
     OpFunctionEnd

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpFunctionCall %uint %50
     OpReturn
     OpFunctionEnd
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  {
    FunctionEmitter fe(p, *spirv_function(100));
    EXPECT_TRUE(fe.EmitBody());
    EXPECT_THAT(ToString(fe.ast_body()), HasSubstr(R"(VariableDeclStatement{
  Variable{
    x_1
    none
    __u32
    {
      Call{
        Identifier{x_50}
        (
        )
      }
    }
  }
}
Return{})"))
        << ToString(fe.ast_body());
  }

  {
    FunctionEmitter fe(p, *spirv_function(50));
    EXPECT_TRUE(fe.EmitBody());
    EXPECT_THAT(ToString(fe.ast_body()), HasSubstr(R"(Return{
  {
    ScalarConstructor{42}
  }
})")) << ToString(fe.ast_body());
  }
}

TEST_F(SpvParserTest, EmitStatement_ScalarCallNoParamsUsedTwice) {
  auto* p = parser(test::Assemble(R"(
     %void = OpTypeVoid
     %voidfn = OpTypeFunction %void
     %uint = OpTypeInt 32 0
     %uintfn = OpTypeFunction %uint
     %val = OpConstant %uint 42
     %ptr_uint = OpTypePointer Function %uint

     %50 = OpFunction %uint None %uintfn
     %entry_50 = OpLabel
     OpReturnValue %val
     OpFunctionEnd

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %10 = OpVariable %ptr_uint Function
     %1 = OpFunctionCall %uint %50
     OpStore %10 %1
     OpStore %10 %1
     OpReturn
     OpFunctionEnd
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  {
    FunctionEmitter fe(p, *spirv_function(100));
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    EXPECT_THAT(ToString(fe.ast_body()), HasSubstr(R"(VariableDeclStatement{
  Variable{
    x_10
    function
    __u32
  }
}
VariableDeclStatement{
  Variable{
    x_1
    none
    __u32
    {
      Call{
        Identifier{x_50}
        (
        )
      }
    }
  }
}
Assignment{
  Identifier{x_10}
  Identifier{x_1}
}
Assignment{
  Identifier{x_10}
  Identifier{x_1}
}
Return{})"))
        << ToString(fe.ast_body());
  }
  {
    FunctionEmitter fe(p, *spirv_function(50));
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    EXPECT_THAT(ToString(fe.ast_body()), HasSubstr(R"(Return{
  {
    ScalarConstructor{42}
  }
})")) << ToString(fe.ast_body());
  }
}

TEST_F(SpvParserTest, EmitStatement_CallWithParams) {
  auto* p = parser(test::Assemble(R"(
     %void = OpTypeVoid
     %voidfn = OpTypeFunction %void
     %uint = OpTypeInt 32 0
     %uintfn_uint_uint = OpTypeFunction %uint %uint %uint
     %val = OpConstant %uint 42
     %val2 = OpConstant %uint 84

     %50 = OpFunction %uint None %uintfn_uint_uint
     %51 = OpFunctionParameter %uint
     %52 = OpFunctionParameter %uint
     %entry_50 = OpLabel
     %sum = OpIAdd %uint %51 %52
     OpReturnValue %sum
     OpFunctionEnd

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpFunctionCall %uint %50 %val %val2
     OpReturn
     OpFunctionEnd
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModule()) << p->error();
  EXPECT_TRUE(p->error().empty());
  const auto module_ast_str = p->module().to_str();
  EXPECT_THAT(module_ast_str, HasSubstr(R"(Module{
  Function x_50 -> __u32
  (
    Variable{
      x_51
      none
      __u32
    }
    Variable{
      x_52
      none
      __u32
    }
  )
  {
    Return{
      {
        Binary{
          Identifier{x_51}
          add
          Identifier{x_52}
        }
      }
    }
  }
  Function x_100 -> __void
  ()
  {
    VariableDeclStatement{
      Variable{
        x_1
        none
        __u32
        {
          Call{
            Identifier{x_50}
            (
              ScalarConstructor{42}
              ScalarConstructor{84}
            )
          }
        }
      }
    }
    Return{}
  }
})")) << module_ast_str;
}

}  // namespace
}  // namespace spirv
}  // namespace reader
}  // namespace tint
