// Copyright 2025 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "src/tint/lang/spirv/reader/parser/helper_test.h"

namespace tint::spirv::reader {
namespace {

TEST_F(SpirvParserTest, Branch_Forward) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
          %1 = OpCopyObject %i32 %one
               OpBranch %bb_2
       %bb_2 = OpLabel
          %2 = OpCopyObject %i32 %two
               OpBranch %bb_3
       %bb_3 = OpLabel
          %3 = OpCopyObject %i32 %three
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = let 1i
    %3:i32 = let 2i
    %4:i32 = let 3i
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BranchConditional) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
       %true = OpConstantTrue %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
          %1 = OpCopyObject %i32 %one
               OpSelectionMerge %bb_merge None
               OpBranchConditional %true %bb_true %bb_false
    %bb_true = OpLabel
          %2 = OpCopyObject %i32 %two
               OpReturn
   %bb_false = OpLabel
          %3 = OpCopyObject %i32 %three
               OpBranch %bb_merge
   %bb_merge = OpLabel
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = let 1i
    if true [t: $B2, f: $B3] {  # if_1
      $B2: {  # true
        %3:i32 = let 2i
        ret
      }
      $B3: {  # false
        %4:i32 = let 3i
        exit_if  # if_1
      }
    }
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BranchConditional_TrueToMerge) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
       %true = OpConstantTrue %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
          %1 = OpCopyObject %i32 %one
               OpSelectionMerge %bb_merge None
               OpBranchConditional %true %bb_merge %bb_false
   %bb_false = OpLabel
          %3 = OpCopyObject %i32 %three
               OpBranch %bb_merge
   %bb_merge = OpLabel
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = let 1i
    if true [t: $B2, f: $B3] {  # if_1
      $B2: {  # true
        exit_if  # if_1
      }
      $B3: {  # false
        %3:i32 = let 3i
        exit_if  # if_1
      }
    }
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BranchConditional_FalseToMerge) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
       %true = OpConstantTrue %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
          %1 = OpCopyObject %i32 %one
               OpSelectionMerge %bb_merge None
               OpBranchConditional %true %bb_true %bb_merge
    %bb_true = OpLabel
          %2 = OpCopyObject %i32 %two
               OpReturn
   %bb_merge = OpLabel
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = let 1i
    if true [t: $B2] {  # if_1
      $B2: {  # true
        %3:i32 = let 2i
        ret
      }
    }
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BranchConditional_TrueMatchesFalse) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
       %true = OpConstantTrue %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
          %1 = OpCopyObject %i32 %one
               OpBranchConditional %true %bb_true %bb_true
    %bb_true = OpLabel
          %2 = OpCopyObject %i32 %two
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = let 1i
    %3:bool = or true, true
    if %3 [t: $B2, f: $B3] {  # if_1
      $B2: {  # true
        %4:i32 = let 2i
        ret
      }
      $B3: {  # false
        unreachable
      }
    }
    unreachable
  }
}
)");
}

TEST_F(SpirvParserTest, BranchConditional_Nested) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpConstant %i32 3
       %true = OpConstantTrue %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
          %1 = OpCopyObject %i32 %one
               OpSelectionMerge %bb_merge None
               OpBranchConditional %true %bb_true %bb_merge
    %bb_true = OpLabel
               OpBranchConditional %true %99 %bb_merge
         %99 = OpLabel
          %2 = OpCopyObject %i32 %two
               OpBranch %bb_merge
   %bb_merge = OpLabel
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = let 1i
    if true [t: $B2] {  # if_1
      $B2: {  # true
        if true [t: $B3] {  # if_2
          $B3: {  # true
            %3:i32 = let 2i
            exit_if  # if_2
          }
        }
        exit_if  # if_1
      }
    }
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BranchConditional_Hoisting) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
       %true = OpConstantTrue %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
          %1 = OpCopyObject %i32 %one
               OpSelectionMerge %bb_merge None
               OpBranchConditional %true %bb_true %bb_false
    %bb_true = OpLabel
          %2 = OpCopyObject %i32 %two
               OpBranch %bb_merge
   %bb_false = OpLabel
               OpReturn
   %bb_merge = OpLabel
          %3 = OpIAdd %i32 %2 %2
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = let 1i
    %3:i32 = if true [t: $B2, f: $B3] {  # if_1
      $B2: {  # true
        %4:i32 = let 2i
        exit_if %4  # if_1
      }
      $B3: {  # false
        ret
      }
    }
    %5:i32 = spirv.add<i32> %3, %3
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BranchConditional_HoistingIntoNested) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %two = OpConstant %i32 2
       %true = OpConstantTrue %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
        %502 = OpLabel
               OpSelectionMerge %50 None
               OpBranchConditional %true %20 %30
         %20 = OpLabel
        %200 = OpCopyObject %i32 %two
               OpBranch %50
         %30 = OpLabel
               OpReturn
         %50 = OpLabel
               OpSelectionMerge %60 None
               OpBranchConditional %true %70 %80
         %70 = OpLabel
        %201 = OpIAdd %i32 %200 %200
               OpBranch %60
         %80 = OpLabel
               OpReturn
         %60 = OpLabel
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = if true [t: $B2, f: $B3] {  # if_1
      $B2: {  # true
        %3:i32 = let 2i
        exit_if %3  # if_1
      }
      $B3: {  # false
        ret
      }
    }
    if true [t: $B4, f: $B5] {  # if_2
      $B4: {  # true
        %4:i32 = spirv.add<i32> %2, %2
        exit_if  # if_2
      }
      $B5: {  # false
        ret
      }
    }
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BranchConditional_HoistingIntoParentNested) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %two = OpConstant %i32 2
       %true = OpConstantTrue %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
          %1 = OpLabel
               OpSelectionMerge %90 None
               OpBranchConditional %true %10 %15
         %15 = OpLabel
               OpReturn
         %10 = OpLabel
               OpSelectionMerge %50 None
               OpBranchConditional %true %20 %30
         %20 = OpLabel
        %200 = OpCopyObject %i32 %two
               OpBranch %50
         %30 = OpLabel
               OpReturn
         %50 = OpLabel
               OpSelectionMerge %60 None
               OpBranchConditional %true %70 %80
         %70 = OpLabel
        %201 = OpIAdd %i32 %200 %200
               OpBranch %60
         %80 = OpLabel
               OpReturn
         %60 = OpLabel
               OpBranch %90
         %90 = OpLabel
        %202 = OpIAdd %i32 %200 %200
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = if true [t: $B2, f: $B3] {  # if_1
      $B2: {  # true
        %3:i32 = if true [t: $B4, f: $B5] {  # if_2
          $B4: {  # true
            %4:i32 = let 2i
            exit_if %4  # if_2
          }
          $B5: {  # false
            ret
          }
        }
        if true [t: $B6, f: $B7] {  # if_3
          $B6: {  # true
            %5:i32 = spirv.add<i32> %3, %3
            exit_if  # if_3
          }
          $B7: {  # false
            ret
          }
        }
        exit_if %3  # if_1
      }
      $B3: {  # false
        ret
      }
    }
    %6:i32 = spirv.add<i32> %2, %2
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, BranchConditional_DuplicateTrue_UsedValue) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
       %true = OpConstantTrue %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
          %1 = OpCopyObject %i32 %one
               OpBranchConditional %true %bb_true %bb_true
    %bb_true = OpLabel
          %2 = OpCopyObject %i32 %two
         %22 = OpCopyObject %i32 %2
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = let 1i
    %3:bool = or true, true
    if %3 [t: $B2, f: $B3] {  # if_1
      $B2: {  # true
        %4:i32 = let 2i
        %5:i32 = let %4
        ret
      }
      $B3: {  # false
        unreachable
      }
    }
    unreachable
  }
}
)");
}

TEST_F(SpirvParserTest, BranchConditional_DuplicateTrue_Premerge) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
       %bool = OpTypeBool
        %one = OpConstant %i32 1
       %true = OpConstantTrue %bool
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %12 = OpLabel
               OpSelectionMerge %13 None
               OpBranchConditional %true %14 %15
         %14 = OpLabel
               OpBranch %16
         %15 = OpLabel
               OpBranch %16
         %16 = OpLabel
          %1 = OpCopyObject %i32 %one
          %2 = OpCopyObject %i32 %1
               OpBranch %13
         %13 = OpLabel
          %3 = OpCopyObject %i32 %1
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    if true [t: $B2, f: $B3] {  # if_1
      $B2: {  # true
        exit_if  # if_1
      }
      $B3: {  # false
        exit_if  # if_1
      }
    }
    %2:i32 = if true [t: $B4, f: $B5] {  # if_2
      $B4: {  # true
        %3:i32 = let 1i
        %4:i32 = let %3
        exit_if %3  # if_2
      }
      $B5: {  # false
        unreachable
      }
    }
    %5:i32 = let %2
    ret
  }
}
)");
}

}  // namespace
}  // namespace tint::spirv::reader
