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

#include <sstream>
#include <string>
#include <vector>

#include "gmock/gmock.h"
#include "src/reader/spirv/function.h"
#include "src/reader/spirv/parser_impl.h"
#include "src/reader/spirv/parser_impl_test_helper.h"
#include "src/reader/spirv/spirv_tools_helpers_test.h"

namespace tint {
namespace reader {
namespace spirv {
namespace {

std::string Dump(const std::vector<uint32_t>& v) {
  std::ostringstream o;
  o << "{";
  for (auto a : v) {
    o << a << " ";
  }
  o << "}";
  return o.str();
}

using ::testing::ElementsAre;
using ::testing::Eq;
using ::testing::UnorderedElementsAre;

std::string CommonTypes() {
  return R"(
    OpCapability Shader
    OpMemoryModel Logical Simple

    %void = OpTypeVoid
    %voidfn = OpTypeFunction %void

    %bool = OpTypeBool
    %cond = OpUndef %bool
    %cond2 = OpUndef %bool
    %cond3 = OpUndef %bool

    %uint = OpTypeInt 32 0
    %selector = OpUndef %uint

    %999 = OpConstant %uint 999
  )";
}

/// Runs the necessary flow until and including classify CFG edges,
/// @returns the result of classify CFG edges.
bool FlowClassifyCFGEdges(FunctionEmitter* fe) {
  fe->RegisterBasicBlocks();
  fe->ComputeBlockOrderAndPositions();
  EXPECT_TRUE(fe->VerifyHeaderContinueMergeOrder()) << fe->parser()->error();
  EXPECT_TRUE(fe->RegisterMerges()) << fe->parser()->error();
  EXPECT_TRUE(fe->LabelControlFlowConstructs()) << fe->parser()->error();
  EXPECT_TRUE(fe->FindSwitchCaseHeaders()) << fe->parser()->error();
  return fe->ClassifyCFGEdges();
}

/// Runs the necessary flow until and including finding if-selection
/// internal headers.
/// @returns the result of classify CFG edges.
bool FlowFindIfSelectionInternalHeaders(FunctionEmitter* fe) {
  EXPECT_TRUE(FlowClassifyCFGEdges(fe)) << fe->parser()->error();
  return fe->FindIfSelectionInternalHeaders();
}

TEST_F(SpvParserTest, TerminatorsAreSane_SingleBlock) {
  auto* p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %42 = OpLabel
     OpReturn

     OpFunctionEnd
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  EXPECT_TRUE(fe.TerminatorsAreSane());
}

TEST_F(SpvParserTest, TerminatorsAreSane_Sequence) {
  auto* p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %20 = OpLabel
     OpBranch %30

     %30 = OpLabel
     OpReturn

     OpFunctionEnd
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  EXPECT_TRUE(fe.TerminatorsAreSane()) << p->error();
}

TEST_F(SpvParserTest, TerminatorsAreSane_If) {
  auto* p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %20 = OpLabel
     OpSelectionMerge %99 None
     OpBranchConditional %cond %30 %40

     %30 = OpLabel
     OpBranch %99

     %40 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  EXPECT_TRUE(fe.TerminatorsAreSane()) << p->error();
}

TEST_F(SpvParserTest, TerminatorsAreSane_Switch) {
  auto* p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpSwitch %selector %80 20 %20 30 %30

     %20 = OpLabel
     OpBranch %30 ; fall through

     %30 = OpLabel
     OpBranch %99

     %80 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  EXPECT_TRUE(fe.TerminatorsAreSane());
}

TEST_F(SpvParserTest, TerminatorsAreSane_Loop_SingleBlock) {
  auto* p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %20 None
     OpBranchConditional %cond %20 %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  EXPECT_TRUE(fe.TerminatorsAreSane());
}

TEST_F(SpvParserTest, TerminatorsAreSane_Loop_Simple) {
  auto* p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %40 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpBranch %40

     %40 = OpLabel
     OpBranch %20 ; back edge

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  EXPECT_TRUE(fe.TerminatorsAreSane());
}

TEST_F(SpvParserTest, TerminatorsAreSane_Kill) {
  auto* p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpKill

     OpFunctionEnd
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  EXPECT_TRUE(fe.TerminatorsAreSane());
}

TEST_F(SpvParserTest, TerminatorsAreSane_Unreachable) {
  auto* p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpUnreachable

     OpFunctionEnd
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  EXPECT_TRUE(fe.TerminatorsAreSane());
}

TEST_F(SpvParserTest, TerminatorsAreSane_MissingTerminator) {
  auto* p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel

     OpFunctionEnd
  )"));
  // The SPIRV-Tools internal representation rejects this case earlier.
  EXPECT_FALSE(p->BuildAndParseInternalModuleExceptFunctions());
}

TEST_F(SpvParserTest, TerminatorsAreSane_DisallowLoopToEntryBlock) {
  auto* p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpBranch %10 ; not allowed

     OpFunctionEnd
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  EXPECT_FALSE(fe.TerminatorsAreSane());
  EXPECT_THAT(p->error(), Eq("Block 20 branches to function entry block 10"));
}

TEST_F(SpvParserTest, TerminatorsAreSane_DisallowNonBlock) {
  auto* p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %999 ; definitely wrong

     OpFunctionEnd
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  EXPECT_FALSE(fe.TerminatorsAreSane());
  EXPECT_THAT(p->error(),
              Eq("Block 10 in function 100 branches to 999 which is "
                 "not a block in the function"));
}

TEST_F(SpvParserTest, TerminatorsAreSane_DisallowBlockInDifferentFunction) {
  auto* p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %210

     OpFunctionEnd


     %200 = OpFunction %void None %voidfn

     %210 = OpLabel
     OpReturn

     OpFunctionEnd
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  EXPECT_FALSE(fe.TerminatorsAreSane());
  EXPECT_THAT(p->error(), Eq("Block 10 in function 100 branches to 210 which "
                             "is not a block in the function"));
}

TEST_F(SpvParserTest, RegisterMerges_NoMerges) {
  auto* p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpReturn

     OpFunctionEnd
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  EXPECT_TRUE(fe.RegisterMerges());

  const auto* bi = fe.GetBlockInfo(10);
  ASSERT_NE(bi, nullptr);
  EXPECT_EQ(bi->merge_for_header, 0u);
  EXPECT_EQ(bi->continue_for_header, 0u);
  EXPECT_EQ(bi->header_for_merge, 0u);
  EXPECT_EQ(bi->header_for_continue, 0u);
  EXPECT_FALSE(bi->is_single_block_loop);
}

TEST_F(SpvParserTest, RegisterMerges_GoodSelectionMerge_BranchConditional) {
  auto* p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpBranchConditional %cond %20 %99

     %20 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  EXPECT_TRUE(fe.RegisterMerges());

  // Header points to the merge
  const auto* bi10 = fe.GetBlockInfo(10);
  ASSERT_NE(bi10, nullptr);
  EXPECT_EQ(bi10->merge_for_header, 99u);
  EXPECT_EQ(bi10->continue_for_header, 0u);
  EXPECT_EQ(bi10->header_for_merge, 0u);
  EXPECT_EQ(bi10->header_for_continue, 0u);
  EXPECT_FALSE(bi10->is_single_block_loop);

  // Middle block is neither header nor merge
  const auto* bi20 = fe.GetBlockInfo(20);
  ASSERT_NE(bi20, nullptr);
  EXPECT_EQ(bi20->merge_for_header, 0u);
  EXPECT_EQ(bi20->continue_for_header, 0u);
  EXPECT_EQ(bi20->header_for_merge, 0u);
  EXPECT_EQ(bi20->header_for_continue, 0u);
  EXPECT_FALSE(bi20->is_single_block_loop);

  // Merge block points to the header
  const auto* bi99 = fe.GetBlockInfo(99);
  ASSERT_NE(bi99, nullptr);
  EXPECT_EQ(bi99->merge_for_header, 0u);
  EXPECT_EQ(bi99->continue_for_header, 0u);
  EXPECT_EQ(bi99->header_for_merge, 10u);
  EXPECT_EQ(bi99->header_for_continue, 0u);
  EXPECT_FALSE(bi99->is_single_block_loop);
}

TEST_F(SpvParserTest, RegisterMerges_GoodSelectionMerge_Switch) {
  auto* p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpSwitch %selector %99 20 %20

     %20 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  EXPECT_TRUE(fe.RegisterMerges());

  // Header points to the merge
  const auto* bi10 = fe.GetBlockInfo(10);
  ASSERT_NE(bi10, nullptr);
  EXPECT_EQ(bi10->merge_for_header, 99u);
  EXPECT_EQ(bi10->continue_for_header, 0u);
  EXPECT_EQ(bi10->header_for_merge, 0u);
  EXPECT_EQ(bi10->header_for_continue, 0u);
  EXPECT_FALSE(bi10->is_single_block_loop);

  // Middle block is neither header nor merge
  const auto* bi20 = fe.GetBlockInfo(20);
  ASSERT_NE(bi20, nullptr);
  EXPECT_EQ(bi20->merge_for_header, 0u);
  EXPECT_EQ(bi20->continue_for_header, 0u);
  EXPECT_EQ(bi20->header_for_merge, 0u);
  EXPECT_EQ(bi20->header_for_continue, 0u);
  EXPECT_FALSE(bi20->is_single_block_loop);

  // Merge block points to the header
  const auto* bi99 = fe.GetBlockInfo(99);
  ASSERT_NE(bi99, nullptr);
  EXPECT_EQ(bi99->merge_for_header, 0u);
  EXPECT_EQ(bi99->continue_for_header, 0u);
  EXPECT_EQ(bi99->header_for_merge, 10u);
  EXPECT_EQ(bi99->header_for_continue, 0u);
  EXPECT_FALSE(bi99->is_single_block_loop);
}

TEST_F(SpvParserTest, RegisterMerges_GoodLoopMerge_SingleBlockLoop) {
  auto* p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %20 None
     OpBranchConditional %cond %20 %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  EXPECT_TRUE(fe.RegisterMerges());

  // Entry block is not special
  const auto* bi10 = fe.GetBlockInfo(10);
  ASSERT_NE(bi10, nullptr);
  EXPECT_EQ(bi10->merge_for_header, 0u);
  EXPECT_EQ(bi10->continue_for_header, 0u);
  EXPECT_EQ(bi10->header_for_merge, 0u);
  EXPECT_EQ(bi10->header_for_continue, 0u);
  EXPECT_FALSE(bi10->is_single_block_loop);

  // Single block loop is its own continue, and marked as single block loop.
  const auto* bi20 = fe.GetBlockInfo(20);
  ASSERT_NE(bi20, nullptr);
  EXPECT_EQ(bi20->merge_for_header, 99u);
  EXPECT_EQ(bi20->continue_for_header, 20u);
  EXPECT_EQ(bi20->header_for_merge, 0u);
  EXPECT_EQ(bi20->header_for_continue, 20u);
  EXPECT_TRUE(bi20->is_single_block_loop);

  // Merge block points to the header
  const auto* bi99 = fe.GetBlockInfo(99);
  ASSERT_NE(bi99, nullptr);
  EXPECT_EQ(bi99->merge_for_header, 0u);
  EXPECT_EQ(bi99->continue_for_header, 0u);
  EXPECT_EQ(bi99->header_for_merge, 20u);
  EXPECT_EQ(bi99->header_for_continue, 0u);
  EXPECT_FALSE(bi99->is_single_block_loop);
}

TEST_F(SpvParserTest, RegisterMerges_GoodLoopMerge_MultiBlockLoop_Branch) {
  auto* p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %40 None
     OpBranch %30

     %30 = OpLabel
     OpBranchConditional %cond %40 %99

     %40 = OpLabel
     OpBranch %20

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  EXPECT_TRUE(fe.RegisterMerges());

  // Loop header points to continue and merge
  const auto* bi20 = fe.GetBlockInfo(20);
  ASSERT_NE(bi20, nullptr);
  EXPECT_EQ(bi20->merge_for_header, 99u);
  EXPECT_EQ(bi20->continue_for_header, 40u);
  EXPECT_EQ(bi20->header_for_merge, 0u);
  EXPECT_EQ(bi20->header_for_continue, 0u);
  EXPECT_FALSE(bi20->is_single_block_loop);

  // Continue block points to header
  const auto* bi40 = fe.GetBlockInfo(40);
  ASSERT_NE(bi40, nullptr);
  EXPECT_EQ(bi40->merge_for_header, 0u);
  EXPECT_EQ(bi40->continue_for_header, 0u);
  EXPECT_EQ(bi40->header_for_merge, 0u);
  EXPECT_EQ(bi40->header_for_continue, 20u);
  EXPECT_FALSE(bi40->is_single_block_loop);

  // Merge block points to the header
  const auto* bi99 = fe.GetBlockInfo(99);
  ASSERT_NE(bi99, nullptr);
  EXPECT_EQ(bi99->merge_for_header, 0u);
  EXPECT_EQ(bi99->continue_for_header, 0u);
  EXPECT_EQ(bi99->header_for_merge, 20u);
  EXPECT_EQ(bi99->header_for_continue, 0u);
  EXPECT_FALSE(bi99->is_single_block_loop);
}

TEST_F(SpvParserTest,
       RegisterMerges_GoodLoopMerge_MultiBlockLoop_BranchConditional) {
  auto* p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %40 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpBranch %40

     %40 = OpLabel
     OpBranch %20

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  EXPECT_TRUE(fe.RegisterMerges());

  // Loop header points to continue and merge
  const auto* bi20 = fe.GetBlockInfo(20);
  ASSERT_NE(bi20, nullptr);
  EXPECT_EQ(bi20->merge_for_header, 99u);
  EXPECT_EQ(bi20->continue_for_header, 40u);
  EXPECT_EQ(bi20->header_for_merge, 0u);
  EXPECT_EQ(bi20->header_for_continue, 0u);
  EXPECT_FALSE(bi20->is_single_block_loop);

  // Continue block points to header
  const auto* bi40 = fe.GetBlockInfo(40);
  ASSERT_NE(bi40, nullptr);
  EXPECT_EQ(bi40->merge_for_header, 0u);
  EXPECT_EQ(bi40->continue_for_header, 0u);
  EXPECT_EQ(bi40->header_for_merge, 0u);
  EXPECT_EQ(bi40->header_for_continue, 20u);
  EXPECT_FALSE(bi40->is_single_block_loop);

  // Merge block points to the header
  const auto* bi99 = fe.GetBlockInfo(99);
  ASSERT_NE(bi99, nullptr);
  EXPECT_EQ(bi99->merge_for_header, 0u);
  EXPECT_EQ(bi99->continue_for_header, 0u);
  EXPECT_EQ(bi99->header_for_merge, 20u);
  EXPECT_EQ(bi99->header_for_continue, 0u);
  EXPECT_FALSE(bi99->is_single_block_loop);
}

TEST_F(SpvParserTest, RegisterMerges_SelectionMerge_BadTerminator) {
  auto* p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpBranch %20

     %20 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  EXPECT_FALSE(fe.RegisterMerges());
  EXPECT_THAT(p->error(), Eq("Selection header 10 does not end in an "
                             "OpBranchConditional or OpSwitch instruction"));
}

TEST_F(SpvParserTest, RegisterMerges_LoopMerge_BadTerminator) {
  auto* p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %40 None
     OpSwitch %selector %99 30 %30

     %30 = OpLabel
     OpBranch %99

     %40 = OpLabel
     OpBranch %20

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  EXPECT_FALSE(fe.RegisterMerges());
  EXPECT_THAT(p->error(), Eq("Loop header 20 does not end in an OpBranch or "
                             "OpBranchConditional instruction"));
}

TEST_F(SpvParserTest, RegisterMerges_BadMergeBlock) {
  auto* p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %void None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  EXPECT_FALSE(fe.RegisterMerges());
  EXPECT_THAT(p->error(),
              Eq("Structured header block 10 declares invalid merge block 1"));
}

TEST_F(SpvParserTest, RegisterMerges_HeaderIsItsOwnMerge) {
  auto* p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %10 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  EXPECT_FALSE(fe.RegisterMerges());
  EXPECT_THAT(p->error(),
              Eq("Structured header block 10 cannot be its own merge block"));
}

TEST_F(SpvParserTest, RegisterMerges_MergeReused) {
  auto* p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %49 None
     OpBranchConditional %cond %20 %49

     %20 = OpLabel
     OpBranch %49

     %49 = OpLabel
     OpBranch %50

     %50 = OpLabel
     OpSelectionMerge %49 None  ; can't reuse merge block
     OpBranchConditional %cond %60 %99

     %60 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  EXPECT_FALSE(fe.RegisterMerges());
  EXPECT_THAT(
      p->error(),
      Eq("Block 49 declared as merge block for more than one header: 10, 50"));
}

TEST_F(SpvParserTest, RegisterMerges_EntryBlockIsLoopHeader) {
  auto* p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpLoopMerge %99 %30 None
     OpBranchConditional %cond %10 %99

     %30 = OpLabel
     OpBranch %10

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  EXPECT_FALSE(fe.RegisterMerges());
  EXPECT_THAT(p->error(),
              Eq("Function entry block 10 cannot be a loop header"));
}

TEST_F(SpvParserTest, RegisterMerges_BadContinueTarget) {
  auto* p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %999 None
     OpBranchConditional %cond %20 %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  EXPECT_FALSE(fe.RegisterMerges());
  EXPECT_THAT(p->error(),
              Eq("Structured header 20 declares invalid continue target 999"));
}

TEST_F(SpvParserTest, RegisterMerges_MergeSameAsContinue) {
  auto* p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %50 %50 None
     OpBranchConditional %cond %20 %99


     %50 = OpLabel
     OpBranch %20

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  EXPECT_FALSE(fe.RegisterMerges());
  EXPECT_THAT(p->error(),
              Eq("Invalid structured header block 20: declares block 50 as "
                 "both its merge block and continue target"));
}

TEST_F(SpvParserTest, RegisterMerges_ContinueReused) {
  auto* p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %49 %40 None
     OpBranchConditional %cond %30 %49

     %30 = OpLabel
     OpBranch %40

     %40 = OpLabel
     OpBranch %20

     %49 = OpLabel
     OpBranch %50

     %50 = OpLabel
     OpLoopMerge %99 %40 None
     OpBranchConditional %cond %60 %99

     %60 = OpLabel
     OpBranch %70

     %70 = OpLabel
     OpBranch %50

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  EXPECT_FALSE(fe.RegisterMerges());
  EXPECT_THAT(p->error(), Eq("Block 40 declared as continue target for more "
                             "than one header: 20, 50"));
}

TEST_F(SpvParserTest, RegisterMerges_SingleBlockLoop_NotItsOwnContinue) {
  auto* p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %30 None
     OpBranchConditional %cond %20 %99

     %30 = OpLabel
     OpBranch %20

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  EXPECT_FALSE(fe.RegisterMerges());
  EXPECT_THAT(
      p->error(),
      Eq("Block 20 branches to itself but is not its own continue target"));
}

TEST_F(SpvParserTest, RegisterMerges_NotSingleBlockLoop_IsItsOwnContinue) {
  auto* p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %20 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpBranch %20

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  EXPECT_FALSE(fe.RegisterMerges());
  EXPECT_THAT(p->error(), Eq("Loop header block 20 declares itself as its own "
                             "continue target, but does not branch to itself"));
}

TEST_F(SpvParserTest, ComputeBlockOrder_OneBlock) {
  auto* p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %42 = OpLabel
     OpReturn

     OpFunctionEnd
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  fe.ComputeBlockOrderAndPositions();

  EXPECT_THAT(fe.block_order(), ElementsAre(42));

  const auto* bi = fe.GetBlockInfo(42);
  ASSERT_NE(bi, nullptr);
  EXPECT_EQ(bi->pos, 0u);
}

TEST_F(SpvParserTest, ComputeBlockOrder_IgnoreStaticalyUnreachable) {
  auto* p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %15 = OpLabel ; statically dead
     OpReturn

     %20 = OpLabel
     OpReturn

     OpFunctionEnd
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  fe.ComputeBlockOrderAndPositions();

  EXPECT_THAT(fe.block_order(), ElementsAre(10, 20));
}

TEST_F(SpvParserTest, ComputeBlockOrder_KillIsDeadEnd) {
  auto* p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %15 = OpLabel ; statically dead
     OpReturn

     %20 = OpLabel
     OpKill        ; Kill doesn't lead anywhere

     OpFunctionEnd
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  fe.ComputeBlockOrderAndPositions();

  EXPECT_THAT(fe.block_order(), ElementsAre(10, 20));
}

TEST_F(SpvParserTest, ComputeBlockOrder_UnreachableIsDeadEnd) {
  auto* p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %15 = OpLabel ; statically dead
     OpReturn

     %20 = OpLabel
     OpUnreachable ; Unreachable doesn't lead anywhere

     OpFunctionEnd
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  fe.ComputeBlockOrderAndPositions();

  EXPECT_THAT(fe.block_order(), ElementsAre(10, 20));
}

TEST_F(SpvParserTest, ComputeBlockOrder_ReorderSequence) {
  auto* p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %30 = OpLabel
     OpReturn

     %20 = OpLabel
     OpBranch %30 ; backtrack

     OpFunctionEnd
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  fe.ComputeBlockOrderAndPositions();

  EXPECT_THAT(fe.block_order(), ElementsAre(10, 20, 30));

  const auto* bi10 = fe.GetBlockInfo(10);
  ASSERT_NE(bi10, nullptr);
  EXPECT_EQ(bi10->pos, 0u);
  const auto* bi20 = fe.GetBlockInfo(20);
  ASSERT_NE(bi20, nullptr);
  EXPECT_EQ(bi20->pos, 1u);
  const auto* bi30 = fe.GetBlockInfo(30);
  ASSERT_NE(bi30, nullptr);
  EXPECT_EQ(bi30->pos, 2u);
}

TEST_F(SpvParserTest, ComputeBlockOrder_DupConditionalBranch) {
  auto* p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpBranchConditional %cond %20 %20

     %99 = OpLabel
     OpReturn

     %20 = OpLabel
     OpBranch %99

     OpFunctionEnd
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  fe.ComputeBlockOrderAndPositions();

  EXPECT_THAT(fe.block_order(), ElementsAre(10, 20, 99));
}

TEST_F(SpvParserTest, ComputeBlockOrder_RespectConditionalBranchOrder) {
  auto* p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpBranchConditional %cond %20 %30

     %99 = OpLabel
     OpReturn

     %30 = OpLabel
     OpReturn

     %20 = OpLabel
     OpBranch %99

     OpFunctionEnd
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  fe.ComputeBlockOrderAndPositions();

  EXPECT_THAT(fe.block_order(), ElementsAre(10, 20, 30, 99));
}

TEST_F(SpvParserTest, ComputeBlockOrder_TrueOnlyBranch) {
  auto* p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpBranchConditional %cond %20 %99

     %99 = OpLabel
     OpReturn

     %20 = OpLabel
     OpBranch %99

     OpFunctionEnd
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  fe.ComputeBlockOrderAndPositions();

  EXPECT_THAT(fe.block_order(), ElementsAre(10, 20, 99));
}

TEST_F(SpvParserTest, ComputeBlockOrder_FalseOnlyBranch) {
  auto* p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpBranchConditional %cond %99 %20

     %99 = OpLabel
     OpReturn

     %20 = OpLabel
     OpBranch %99

     OpFunctionEnd
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  fe.ComputeBlockOrderAndPositions();

  EXPECT_THAT(fe.block_order(), ElementsAre(10, 20, 99));
}

TEST_F(SpvParserTest, ComputeBlockOrder_SwitchOrderNaturallyReversed) {
  auto* p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpSwitch %selector %99 20 %20 30 %30

     %99 = OpLabel
     OpReturn

     %30 = OpLabel
     OpReturn

     %20 = OpLabel
     OpBranch %99

     OpFunctionEnd
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  fe.ComputeBlockOrderAndPositions();

  EXPECT_THAT(fe.block_order(), ElementsAre(10, 30, 20, 99));
}

TEST_F(SpvParserTest,
       ComputeBlockOrder_SwitchWithDefaultOrderNaturallyReversed) {
  auto* p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpSwitch %selector %80 20 %20 30 %30

     %80 = OpLabel ; the default case
     OpBranch %99

     %99 = OpLabel
     OpReturn

     %30 = OpLabel
     OpReturn

     %20 = OpLabel
     OpBranch %99

     OpFunctionEnd
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  fe.ComputeBlockOrderAndPositions();

  EXPECT_THAT(fe.block_order(), ElementsAre(10, 30, 20, 80, 99));
}

TEST_F(SpvParserTest, ComputeBlockOrder_Switch_DefaultSameAsACase) {
  auto* p = parser(test::Assemble(CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpSwitch %selector %30 20 %20 30 %30 40 %40

     %99 = OpLabel
     OpReturn

     %30 = OpLabel
     OpBranch %99

     %20 = OpLabel
     OpBranch %99

     %40 = OpLabel
     OpBranch %99

     OpFunctionEnd
  )"));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  fe.ComputeBlockOrderAndPositions();

  EXPECT_THAT(fe.block_order(), ElementsAre(10, 40, 20, 30, 99));
}

TEST_F(SpvParserTest, ComputeBlockOrder_RespectSwitchCaseFallthrough) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpSwitch %selector %99 20 %20 30 %30 40 %40 50 %50

     %50 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpReturn

     %40 = OpLabel
     OpBranch %99

     %30 = OpLabel
     OpBranch %50 ; fallthrough

     %20 = OpLabel
     OpBranch %40 ; fallthrough

     OpFunctionEnd
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  fe.ComputeBlockOrderAndPositions();

  EXPECT_THAT(fe.block_order(), ElementsAre(10, 30, 50, 20, 40, 99))
      << assembly;
}

TEST_F(SpvParserTest,
       ComputeBlockOrder_RespectSwitchCaseFallthrough_FromDefault) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpSwitch %selector %80 20 %20 30 %30 40 %40

     %80 = OpLabel ; the default case
     OpBranch %30 ; fallthrough to another case

     %99 = OpLabel
     OpReturn

     %40 = OpLabel
     OpBranch %99

     %30 = OpLabel
     OpBranch %40

     %20 = OpLabel
     OpBranch %99

     OpFunctionEnd
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  fe.ComputeBlockOrderAndPositions();

  EXPECT_THAT(fe.block_order(), ElementsAre(10, 20, 80, 30, 40, 99))
      << assembly;
}

TEST_F(SpvParserTest,
       ComputeBlockOrder_RespectSwitchCaseFallthrough_FromCaseToDefaultToCase) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpSwitch %selector %80 20 %20 30 %30

     %99 = OpLabel
     OpReturn

     %20 = OpLabel
     OpBranch %80 ; fallthrough to default

     %80 = OpLabel ; the default case
     OpBranch %30 ; fallthrough to 30

     %30 = OpLabel
     OpBranch %99

     OpFunctionEnd
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  fe.ComputeBlockOrderAndPositions();

  EXPECT_THAT(fe.block_order(), ElementsAre(10, 20, 80, 30, 99)) << assembly;
}

TEST_F(SpvParserTest,
       ComputeBlockOrder_SwitchCasesFallthrough_OppositeDirections) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpSwitch %selector %99 20 %20 30 %30 40 %40 50 %50

     %99 = OpLabel
     OpReturn

     %20 = OpLabel
     OpBranch %30 ; forward

     %40 = OpLabel
     OpBranch %99

     %30 = OpLabel
     OpBranch %99

     ; SPIR-V doesn't actually allow a fall-through that goes backward in the
     ; module. But the block ordering algorithm tolerates it.
     %50 = OpLabel
     OpBranch %40 ; backward

     OpFunctionEnd
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  fe.ComputeBlockOrderAndPositions();

  EXPECT_THAT(fe.block_order(), ElementsAre(10, 50, 40, 20, 30, 99))
      << assembly;
}

TEST_F(SpvParserTest,
       ComputeBlockOrder_RespectSwitchCaseFallthrough_Interleaved) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpSwitch %selector %99 20 %20 30 %30 40 %40 50 %50

     %99 = OpLabel
     OpReturn

     %20 = OpLabel
     OpBranch %40

     %30 = OpLabel
     OpBranch %50

     %40 = OpLabel
     OpBranch %60

     %50 = OpLabel
     OpBranch %70

     %60 = OpLabel
     OpBranch %99

     %70 = OpLabel
     OpBranch %99

     OpFunctionEnd
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  fe.ComputeBlockOrderAndPositions();

  EXPECT_THAT(fe.block_order(), ElementsAre(10, 30, 50, 70, 20, 40, 60, 99))
      << assembly;
}

TEST_F(SpvParserTest, ComputeBlockOrder_Nest_If_Contains_If) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpBranchConditional %cond %20 %50

     %99 = OpLabel
     OpReturn

     %20 = OpLabel
     OpSelectionMerge %49 None
     OpBranchConditional %cond %30 %40

     %49 = OpLabel
     OpBranch %99

     %30 = OpLabel
     OpBranch %49

     %40 = OpLabel
     OpBranch %49

     %50 = OpLabel
     OpSelectionMerge %79 None
     OpBranchConditional %cond %60 %70

     %79 = OpLabel
     OpBranch %99

     %60 = OpLabel
     OpBranch %79

     %70 = OpLabel
     OpBranch %79

     OpFunctionEnd
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  fe.ComputeBlockOrderAndPositions();

  EXPECT_THAT(fe.block_order(),
              ElementsAre(10, 20, 30, 40, 49, 50, 60, 70, 79, 99))
      << assembly;
}

TEST_F(SpvParserTest, ComputeBlockOrder_Nest_If_In_SwitchCase) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpSwitch %selector %50 20 %20 50 %50

     %99 = OpLabel
     OpReturn

     %20 = OpLabel
     OpSelectionMerge %49 None
     OpBranchConditional %cond %30 %40

     %49 = OpLabel
     OpBranch %99

     %30 = OpLabel
     OpBranch %49

     %40 = OpLabel
     OpBranch %49

     %50 = OpLabel
     OpSelectionMerge %79 None
     OpBranchConditional %cond %60 %70

     %79 = OpLabel
     OpBranch %99

     %60 = OpLabel
     OpBranch %79

     %70 = OpLabel
     OpBranch %79

     OpFunctionEnd
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  fe.ComputeBlockOrderAndPositions();

  EXPECT_THAT(fe.block_order(),
              ElementsAre(10, 20, 30, 40, 49, 50, 60, 70, 79, 99))
      << assembly;
}

TEST_F(SpvParserTest, ComputeBlockOrder_Nest_IfFallthrough_In_SwitchCase) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpSwitch %selector %50 20 %20 50 %50

     %99 = OpLabel
     OpReturn

     %20 = OpLabel
     OpSelectionMerge %49 None
     OpBranchConditional %cond %30 %40

     %49 = OpLabel
     OpBranchConditional %cond %99 %50 ; fallthrough

     %30 = OpLabel
     OpBranch %49

     %40 = OpLabel
     OpBranch %49

     %50 = OpLabel
     OpSelectionMerge %79 None
     OpBranchConditional %cond %60 %70

     %79 = OpLabel
     OpBranch %99

     %60 = OpLabel
     OpBranch %79

     %70 = OpLabel
     OpBranch %79

     OpFunctionEnd
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  fe.ComputeBlockOrderAndPositions();

  EXPECT_THAT(fe.block_order(),
              ElementsAre(10, 20, 30, 40, 49, 50, 60, 70, 79, 99))
      << assembly;
}

TEST_F(SpvParserTest, ComputeBlockOrder_Nest_IfBreak_In_SwitchCase) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpSwitch %selector %50 20 %20 50 %50

     %99 = OpLabel
     OpReturn

     %20 = OpLabel
     OpSelectionMerge %49 None
     OpBranchConditional %cond %99 %40 ; break-if

     %49 = OpLabel
     OpBranch %99

     %40 = OpLabel
     OpBranch %49

     %50 = OpLabel
     OpSelectionMerge %79 None
     OpBranchConditional %cond %60 %99 ; break-unless

     %79 = OpLabel
     OpBranch %99

     %60 = OpLabel
     OpBranch %79

     OpFunctionEnd
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  fe.ComputeBlockOrderAndPositions();

  EXPECT_THAT(fe.block_order(), ElementsAre(10, 20, 40, 49, 50, 60, 79, 99))
      << assembly;
}

TEST_F(SpvParserTest, ComputeBlockOrder_Loop_SingleBlock_Simple) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     ; The entry block can't be the target of a branch
     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %20 None
     OpBranchConditional %cond %20 %99

     %99 = OpLabel
     OpReturn
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  fe.ComputeBlockOrderAndPositions();

  EXPECT_THAT(fe.block_order(), ElementsAre(10, 20, 99)) << assembly;
}

TEST_F(SpvParserTest, ComputeBlockOrder_Loop_SingleBlock_Infinite) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     ; The entry block can't be the target of a branch
     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %20 None
     OpBranch %20

     %99 = OpLabel
     OpReturn
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  fe.ComputeBlockOrderAndPositions();

  EXPECT_THAT(fe.block_order(), ElementsAre(10, 20, 99)) << assembly;
}

TEST_F(SpvParserTest, ComputeBlockOrder_Loop_SingleBlock_DupInfinite) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     ; The entry block can't be the target of a branch
     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %20 None
     OpBranchConditional %cond %20 %20

     %99 = OpLabel
     OpReturn
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  fe.ComputeBlockOrderAndPositions();

  EXPECT_THAT(fe.block_order(), ElementsAre(10, 20, 99)) << assembly;
}

TEST_F(SpvParserTest, ComputeBlockOrder_Loop_HeaderHasBreakIf) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %50 None
     OpBranchConditional %cond %30 %99 ; like While

     %30 = OpLabel ; trivial body
     OpBranch %50

     %50 = OpLabel
     OpBranch %20

     %99 = OpLabel
     OpReturn
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  fe.ComputeBlockOrderAndPositions();

  EXPECT_THAT(fe.block_order(), ElementsAre(10, 20, 30, 50, 99)) << assembly;
}

TEST_F(SpvParserTest, ComputeBlockOrder_Loop_HeaderHasBreakUnless) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %50 None
     OpBranchConditional %cond %99 %30 ; has break-unless

     %30 = OpLabel ; trivial body
     OpBranch %50

     %50 = OpLabel
     OpBranch %20

     %99 = OpLabel
     OpReturn
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  fe.ComputeBlockOrderAndPositions();

  EXPECT_THAT(fe.block_order(), ElementsAre(10, 20, 30, 50, 99)) << assembly;
}

TEST_F(SpvParserTest, ComputeBlockOrder_Loop_BodyHasBreak) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %50 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpBranch %99 ; break

     %50 = OpLabel
     OpBranch %20

     %99 = OpLabel
     OpReturn
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  fe.ComputeBlockOrderAndPositions();

  EXPECT_THAT(fe.block_order(), ElementsAre(10, 20, 30, 50, 99)) << assembly;
}

TEST_F(SpvParserTest, ComputeBlockOrder_Loop_BodyHasBreakIf) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %50 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpBranchConditional %cond2 %99 %40 ; break-if

     %40 = OpLabel
     OpBranch %50

     %50 = OpLabel
     OpBranch %20

     %99 = OpLabel
     OpReturn
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  fe.ComputeBlockOrderAndPositions();

  EXPECT_THAT(fe.block_order(), ElementsAre(10, 20, 30, 40, 50, 99))
      << assembly;
}

TEST_F(SpvParserTest, ComputeBlockOrder_Loop_BodyHasBreakUnless) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %50 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpBranchConditional %cond2 %40 %99 ; break-unless

     %40 = OpLabel
     OpBranch %50

     %50 = OpLabel
     OpBranch %20

     %99 = OpLabel
     OpReturn
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  fe.ComputeBlockOrderAndPositions();

  EXPECT_THAT(fe.block_order(), ElementsAre(10, 20, 30, 40, 50, 99))
      << assembly;
}

TEST_F(SpvParserTest, ComputeBlockOrder_Loop_Body_If) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %50 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpSelectionMerge %49 None
     OpBranchConditional %cond2 %40 %45 ; nested if

     %40 = OpLabel
     OpBranch %49

     %45 = OpLabel
     OpBranch %49

     %49 = OpLabel
     OpBranch %50

     %50 = OpLabel
     OpBranch %20

     %99 = OpLabel
     OpReturn
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  fe.ComputeBlockOrderAndPositions();

  EXPECT_THAT(fe.block_order(), ElementsAre(10, 20, 30, 40, 45, 49, 50, 99))
      << assembly;
}

TEST_F(SpvParserTest, ComputeBlockOrder_Loop_Body_If_Break) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %50 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpSelectionMerge %49 None
     OpBranchConditional %cond2 %40 %49 ; nested if

     %40 = OpLabel
     OpBranch %99   ; break from nested if

     %49 = OpLabel
     OpBranch %50

     %50 = OpLabel
     OpBranch %20

     %99 = OpLabel
     OpReturn
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  fe.ComputeBlockOrderAndPositions();

  EXPECT_THAT(fe.block_order(), ElementsAre(10, 20, 30, 40, 49, 50, 99))
      << assembly;
}

TEST_F(SpvParserTest, ComputeBlockOrder_Loop_BodyHasContinueIf) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %50 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpBranchConditional %cond2 %50 %40 ; continue-if

     %40 = OpLabel
     OpBranch %50

     %50 = OpLabel
     OpBranch %20

     %99 = OpLabel
     OpReturn
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  fe.ComputeBlockOrderAndPositions();

  EXPECT_THAT(fe.block_order(), ElementsAre(10, 20, 30, 40, 50, 99))
      << assembly;
}

TEST_F(SpvParserTest, ComputeBlockOrder_Loop_BodyHasContinueUnless) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %50 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpBranchConditional %cond2 %40 %50 ; continue-unless

     %40 = OpLabel
     OpBranch %50

     %50 = OpLabel
     OpBranch %20

     %99 = OpLabel
     OpReturn
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  fe.ComputeBlockOrderAndPositions();

  EXPECT_THAT(fe.block_order(), ElementsAre(10, 20, 30, 40, 50, 99))
      << assembly;
}

TEST_F(SpvParserTest, ComputeBlockOrder_Loop_Body_If_Continue) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %50 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpSelectionMerge %49 None
     OpBranchConditional %cond2 %40 %49 ; nested if

     %40 = OpLabel
     OpBranch %50   ; continue from nested if

     %49 = OpLabel
     OpBranch %50

     %50 = OpLabel
     OpBranch %20

     %99 = OpLabel
     OpReturn
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  fe.ComputeBlockOrderAndPositions();

  EXPECT_THAT(fe.block_order(), ElementsAre(10, 20, 30, 40, 49, 50, 99))
      << assembly;
}

TEST_F(SpvParserTest, ComputeBlockOrder_Loop_Body_Switch) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %50 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpSelectionMerge %49 None
     OpSwitch %selector %49 40 %40 45 %45 ; fully nested switch

     %40 = OpLabel
     OpBranch %49

     %45 = OpLabel
     OpBranch %49

     %49 = OpLabel
     OpBranch %50

     %50 = OpLabel
     OpBranch %20

     %99 = OpLabel
     OpReturn
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  fe.ComputeBlockOrderAndPositions();

  EXPECT_THAT(fe.block_order(), ElementsAre(10, 20, 30, 45, 40, 49, 50, 99))
      << assembly;
}

TEST_F(SpvParserTest, ComputeBlockOrder_Loop_Body_Switch_CaseBreaks) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %50 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpSelectionMerge %49 None
     OpSwitch %selector %49 40 %40 45 %45

     %40 = OpLabel
     ; This case breaks out of the loop. This is not possible in C
     ; because "break" will escape the switch only.
     OpBranch %99

     %45 = OpLabel
     OpBranch %49

     %49 = OpLabel
     OpBranch %50

     %50 = OpLabel
     OpBranch %20

     %99 = OpLabel
     OpReturn
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  fe.ComputeBlockOrderAndPositions();

  EXPECT_THAT(fe.block_order(), ElementsAre(10, 20, 30, 45, 40, 49, 50, 99))
      << assembly;
}

TEST_F(SpvParserTest, ComputeBlockOrder_Loop_Body_Switch_CaseContinues) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %50 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpSelectionMerge %49 None
     OpSwitch %selector %49 40 %40 45 %45

     %40 = OpLabel
     OpBranch %50   ; continue bypasses switch merge

     %45 = OpLabel
     OpBranch %49

     %49 = OpLabel
     OpBranch %50

     %50 = OpLabel
     OpBranch %20

     %99 = OpLabel
     OpReturn
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  fe.ComputeBlockOrderAndPositions();

  EXPECT_THAT(fe.block_order(), ElementsAre(10, 20, 30, 45, 40, 49, 50, 99))
      << assembly;
}

TEST_F(SpvParserTest, ComputeBlockOrder_Loop_BodyHasSwitchContinueBreak) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %50 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpSwitch %selector %99 50 %50 ; default is break, 50 is continue

     %40 = OpLabel
     OpBranch %50

     %50 = OpLabel
     OpBranch %20

     %99 = OpLabel
     OpReturn
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  fe.ComputeBlockOrderAndPositions();

  EXPECT_THAT(fe.block_order(), ElementsAre(10, 20, 30, 50, 99));
}

TEST_F(SpvParserTest, ComputeBlockOrder_Loop_Continue_Sequence) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %50 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpBranch %50

     %50 = OpLabel
     OpBranch %60

     %60 = OpLabel
     OpBranch %20

     %99 = OpLabel
     OpReturn
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  fe.ComputeBlockOrderAndPositions();

  EXPECT_THAT(fe.block_order(), ElementsAre(10, 20, 30, 50, 60, 99));
}

TEST_F(SpvParserTest, ComputeBlockOrder_Loop_Continue_ContainsIf) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %50 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpBranch %50

     %50 = OpLabel
     OpSelectionMerge %89 None
     OpBranchConditional %cond2 %60 %70

     %89 = OpLabel
     OpBranch %20 ; backedge

     %60 = OpLabel
     OpBranch %89

     %70 = OpLabel
     OpBranch %89

     %99 = OpLabel
     OpReturn
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  fe.ComputeBlockOrderAndPositions();

  EXPECT_THAT(fe.block_order(), ElementsAre(10, 20, 30, 50, 60, 70, 89, 99));
}

TEST_F(SpvParserTest, ComputeBlockOrder_Loop_Continue_HasBreakIf) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %50 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpBranch %50

     %50 = OpLabel
     OpBranchConditional %cond2 %99 %20

     %99 = OpLabel
     OpReturn
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  fe.ComputeBlockOrderAndPositions();

  EXPECT_THAT(fe.block_order(), ElementsAre(10, 20, 30, 50, 99));
}

TEST_F(SpvParserTest, ComputeBlockOrder_Loop_Continue_HasBreakUnless) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %50 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpBranch %50

     %50 = OpLabel
     OpBranchConditional %cond2 %20 %99

     %99 = OpLabel
     OpReturn
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  fe.ComputeBlockOrderAndPositions();

  EXPECT_THAT(fe.block_order(), ElementsAre(10, 20, 30, 50, 99));
}

TEST_F(SpvParserTest, ComputeBlockOrder_Loop_Continue_SwitchBreak) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %50 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpBranch %50

     %50 = OpLabel
     OpSwitch %selector %20 99 %99 ; yes, this is obtuse but valid

     %99 = OpLabel
     OpReturn
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  fe.ComputeBlockOrderAndPositions();

  EXPECT_THAT(fe.block_order(), ElementsAre(10, 20, 30, 50, 99));
}

TEST_F(SpvParserTest, ComputeBlockOrder_Loop_Loop) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %50 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpLoopMerge %49 %40 None
     OpBranchConditional %cond2 %35 %49

     %35 = OpLabel
     OpBranch %37

     %37 = OpLabel
     OpBranch %40

     %40 = OpLabel ; inner loop's continue
     OpBranch %30 ; backedge

     %49 = OpLabel ; inner loop's merge
     OpBranch %50

     %50 = OpLabel ; outer loop's continue
     OpBranch %20 ; outer loop's backege

     %99 = OpLabel
     OpReturn
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  fe.ComputeBlockOrderAndPositions();

  EXPECT_THAT(fe.block_order(),
              ElementsAre(10, 20, 30, 35, 37, 40, 49, 50, 99));
}

TEST_F(SpvParserTest, ComputeBlockOrder_Loop_Loop_InnerBreak) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %50 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpLoopMerge %49 %40 None
     OpBranchConditional %cond2 %35 %49

     %35 = OpLabel
     OpBranchConditional %cond3 %49 %37 ; break to inner merge

     %37 = OpLabel
     OpBranch %40

     %40 = OpLabel ; inner loop's continue
     OpBranch %30 ; backedge

     %49 = OpLabel ; inner loop's merge
     OpBranch %50

     %50 = OpLabel ; outer loop's continue
     OpBranch %20 ; outer loop's backege

     %99 = OpLabel
     OpReturn
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  fe.ComputeBlockOrderAndPositions();

  EXPECT_THAT(fe.block_order(),
              ElementsAre(10, 20, 30, 35, 37, 40, 49, 50, 99));
}

TEST_F(SpvParserTest, ComputeBlockOrder_Loop_Loop_InnerContinue) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %50 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpLoopMerge %49 %40 None
     OpBranchConditional %cond2 %35 %49

     %35 = OpLabel
     OpBranchConditional %cond3 %37 %49 ; continue to inner continue target

     %37 = OpLabel
     OpBranch %40

     %40 = OpLabel ; inner loop's continue
     OpBranch %30 ; backedge

     %49 = OpLabel ; inner loop's merge
     OpBranch %50

     %50 = OpLabel ; outer loop's continue
     OpBranch %20 ; outer loop's backege

     %99 = OpLabel
     OpReturn
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  fe.ComputeBlockOrderAndPositions();

  EXPECT_THAT(fe.block_order(),
              ElementsAre(10, 20, 30, 35, 37, 40, 49, 50, 99));
}

TEST_F(SpvParserTest, ComputeBlockOrder_Loop_Loop_InnerContinueBreaks) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %50 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpLoopMerge %49 %40 None
     OpBranchConditional %cond2 %35 %49

     %35 = OpLabel
     OpBranch %37

     %37 = OpLabel
     OpBranch %40

     %40 = OpLabel ; inner loop's continue
     OpBranchConditional %cond3 %30 %49 ; backedge and inner break

     %49 = OpLabel ; inner loop's merge
     OpBranch %50

     %50 = OpLabel ; outer loop's continue
     OpBranch %20 ; outer loop's backege

     %99 = OpLabel
     OpReturn
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  fe.ComputeBlockOrderAndPositions();

  EXPECT_THAT(fe.block_order(),
              ElementsAre(10, 20, 30, 35, 37, 40, 49, 50, 99));
}

TEST_F(SpvParserTest, ComputeBlockOrder_Loop_Loop_InnerContinueContinues) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %50 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpLoopMerge %49 %40 None
     OpBranchConditional %cond2 %35 %49

     %35 = OpLabel
     OpBranch %37

     %37 = OpLabel
     OpBranch %40

     %40 = OpLabel ; inner loop's continue
     OpBranchConditional %cond3 %30 %50 ; backedge and continue to outer

     %49 = OpLabel ; inner loop's merge
     OpBranch %50

     %50 = OpLabel ; outer loop's continue
     OpBranch %20 ; outer loop's backege

     %99 = OpLabel
     OpReturn
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  fe.ComputeBlockOrderAndPositions();

  EXPECT_THAT(fe.block_order(),
              ElementsAre(10, 20, 30, 35, 37, 40, 49, 50, 99));
}

TEST_F(SpvParserTest, ComputeBlockOrder_Loop_Loop_SwitchBackedgeBreakContinue) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %50 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpLoopMerge %49 %40 None
     OpBranchConditional %cond2 %35 %49

     %35 = OpLabel
     OpBranch %37

     %37 = OpLabel
     OpBranch %40

     %40 = OpLabel ; inner loop's continue
     ; This switch does triple duty:
     ; default -> backedge
     ; 49 -> loop break
     ; 49 -> inner loop break
     ; 50 -> outer loop continue
     OpSwitch %selector %30 49 %49 50 %50

     %49 = OpLabel ; inner loop's merge
     OpBranch %50

     %50 = OpLabel ; outer loop's continue
     OpBranch %20 ; outer loop's backege

     %99 = OpLabel
     OpReturn
  )";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  fe.ComputeBlockOrderAndPositions();

  EXPECT_THAT(fe.block_order(),
              ElementsAre(10, 20, 30, 35, 37, 40, 49, 50, 99));
}

TEST_F(SpvParserTest, VerifyHeaderContinueMergeOrder_Selection_Good) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpBranchConditional %cond %20 %30

     %20 = OpLabel
     OpBranch %99

     %30 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  fe.ComputeBlockOrderAndPositions();
  fe.RegisterMerges();
  EXPECT_TRUE(fe.VerifyHeaderContinueMergeOrder());
}

TEST_F(SpvParserTest, VerifyHeaderContinueMergeOrder_SingleBlockLoop_Good) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %20 None
     OpBranchConditional %cond %20 %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  fe.ComputeBlockOrderAndPositions();
  fe.RegisterMerges();
  EXPECT_TRUE(fe.VerifyHeaderContinueMergeOrder()) << p->error();
}

TEST_F(SpvParserTest, VerifyHeaderContinueMergeOrder_MultiBlockLoop_Good) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %30 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpBranch %20

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  fe.ComputeBlockOrderAndPositions();
  fe.RegisterMerges();
  EXPECT_TRUE(fe.VerifyHeaderContinueMergeOrder());
}

TEST_F(SpvParserTest,
       VerifyHeaderContinueMergeOrder_HeaderDoesNotStrictlyDominateMerge) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpBranch %50

     %50 = OpLabel
     OpSelectionMerge %20 None ; this is backward
     OpBranchConditional %cond2 %60 %99

     %60 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  fe.ComputeBlockOrderAndPositions();
  fe.RegisterMerges();
  EXPECT_FALSE(fe.VerifyHeaderContinueMergeOrder());
  EXPECT_THAT(p->error(),
              Eq("Header 50 does not strictly dominate its merge block 20"))
      << *fe.GetBlockInfo(50) << std::endl
      << *fe.GetBlockInfo(20) << std::endl
      << Dump(fe.block_order());
}

TEST_F(
    SpvParserTest,
    VerifyHeaderContinueMergeOrder_HeaderDoesNotStrictlyDominateContinueTarget) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpBranch %50

     %50 = OpLabel
     OpLoopMerge %99 %20 None ; this is backward
     OpBranchConditional %cond %60 %99

     %60 = OpLabel
     OpBranch %50

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  fe.ComputeBlockOrderAndPositions();
  fe.RegisterMerges();
  EXPECT_FALSE(fe.VerifyHeaderContinueMergeOrder());
  EXPECT_THAT(p->error(),
              Eq("Loop header 50 does not dominate its continue target 20"))
      << *fe.GetBlockInfo(50) << std::endl
      << *fe.GetBlockInfo(20) << std::endl
      << Dump(fe.block_order());
}

TEST_F(SpvParserTest,
       VerifyHeaderContinueMergeOrder_MergeInsideContinueTarget) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %50

     %50 = OpLabel
     OpLoopMerge %60 %70 None
     OpBranchConditional %cond %60 %99

     %60 = OpLabel
     OpBranch %70

     %70 = OpLabel
     OpBranch %50

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  fe.ComputeBlockOrderAndPositions();
  fe.RegisterMerges();
  EXPECT_FALSE(fe.VerifyHeaderContinueMergeOrder());
  EXPECT_THAT(p->error(),
              Eq("Merge block 60 for loop headed at block 50 appears at or "
                 "before the loop's continue construct headed by block 70"))
      << Dump(fe.block_order());
}

TEST_F(SpvParserTest,
       LabelControlFlowConstructs_OuterConstructIsFunction_SingleBlock) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpReturn

     OpFunctionEnd
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  fe.ComputeBlockOrderAndPositions();
  fe.RegisterMerges();
  EXPECT_TRUE(fe.LabelControlFlowConstructs());
  EXPECT_EQ(fe.constructs().size(), 1u);
  auto& c = fe.constructs().front();
  EXPECT_THAT(ToString(c), Eq("Construct{ Function [0,1) begin_id:10 end_id:0 "
                              "depth:0 parent:null }"));
  EXPECT_EQ(fe.GetBlockInfo(10)->construct, c.get());
}

TEST_F(SpvParserTest,
       LabelControlFlowConstructs_OuterConstructIsFunction_MultiBlock) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %5

     %5 = OpLabel
     OpReturn

     OpFunctionEnd
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  fe.ComputeBlockOrderAndPositions();
  fe.RegisterMerges();
  EXPECT_TRUE(fe.LabelControlFlowConstructs());
  EXPECT_EQ(fe.constructs().size(), 1u);
  auto& c = fe.constructs().front();
  EXPECT_THAT(ToString(c), Eq("Construct{ Function [0,2) begin_id:10 end_id:0 "
                              "depth:0 parent:null }"));
  EXPECT_EQ(fe.GetBlockInfo(10)->construct, c.get());
  EXPECT_EQ(fe.GetBlockInfo(5)->construct, c.get());
}

TEST_F(SpvParserTest,
       LabelControlFlowConstructs_FunctionIsOnlyIfSelectionAndItsMerge) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpBranchConditional %cond %20 %30

     %20 = OpLabel
     OpBranch %99

     %30 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  fe.ComputeBlockOrderAndPositions();
  fe.RegisterMerges();
  EXPECT_TRUE(fe.LabelControlFlowConstructs());
  const auto& constructs = fe.constructs();
  EXPECT_EQ(constructs.size(), 2u);
  EXPECT_THAT(ToString(constructs), Eq(R"(ConstructList{
  Construct{ Function [0,4) begin_id:10 end_id:0 depth:0 parent:null }
  Construct{ IfSelection [0,3) begin_id:10 end_id:99 depth:1 parent:Function@10 }
})")) << constructs;
  // The block records the nearest enclosing construct.
  EXPECT_EQ(fe.GetBlockInfo(10)->construct, constructs[1].get());
  EXPECT_EQ(fe.GetBlockInfo(20)->construct, constructs[1].get());
  EXPECT_EQ(fe.GetBlockInfo(30)->construct, constructs[1].get());
  EXPECT_EQ(fe.GetBlockInfo(99)->construct, constructs[0].get());
}

TEST_F(
    SpvParserTest,
    LabelControlFlowConstructs_PaddingBlocksBeforeAndAfterStructuredConstruct) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %5 = OpLabel
     OpBranch %10

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpBranchConditional %cond %20 %30

     %20 = OpLabel
     OpBranch %99

     %30 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpBranch %200

     %200 = OpLabel
     OpReturn

     OpFunctionEnd
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  fe.ComputeBlockOrderAndPositions();
  fe.RegisterMerges();
  EXPECT_TRUE(fe.LabelControlFlowConstructs());
  const auto& constructs = fe.constructs();
  EXPECT_EQ(constructs.size(), 2u);
  EXPECT_THAT(ToString(constructs), Eq(R"(ConstructList{
  Construct{ Function [0,6) begin_id:5 end_id:0 depth:0 parent:null }
  Construct{ IfSelection [1,4) begin_id:10 end_id:99 depth:1 parent:Function@5 }
})")) << constructs;
  // The block records the nearest enclosing construct.
  EXPECT_EQ(fe.GetBlockInfo(5)->construct, constructs[0].get());
  EXPECT_EQ(fe.GetBlockInfo(10)->construct, constructs[1].get());
  EXPECT_EQ(fe.GetBlockInfo(20)->construct, constructs[1].get());
  EXPECT_EQ(fe.GetBlockInfo(30)->construct, constructs[1].get());
  EXPECT_EQ(fe.GetBlockInfo(99)->construct, constructs[0].get());
  EXPECT_EQ(fe.GetBlockInfo(200)->construct, constructs[0].get());
}

TEST_F(SpvParserTest, LabelControlFlowConstructs_SwitchSelection) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpSwitch %selector %40 20 %20 30 %30

     %20 = OpLabel
     OpBranch %99

     %30 = OpLabel
     OpBranch %99

     %40 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  fe.ComputeBlockOrderAndPositions();
  fe.RegisterMerges();
  EXPECT_TRUE(fe.LabelControlFlowConstructs());
  const auto& constructs = fe.constructs();
  EXPECT_EQ(constructs.size(), 2u);
  EXPECT_THAT(ToString(constructs), Eq(R"(ConstructList{
  Construct{ Function [0,5) begin_id:10 end_id:0 depth:0 parent:null }
  Construct{ SwitchSelection [0,4) begin_id:10 end_id:99 depth:1 parent:Function@10 in-c-l-s:SwitchSelection@10 }
})")) << constructs;
  // The block records the nearest enclosing construct.
  EXPECT_EQ(fe.GetBlockInfo(10)->construct, constructs[1].get());
  EXPECT_EQ(fe.GetBlockInfo(20)->construct, constructs[1].get());
  EXPECT_EQ(fe.GetBlockInfo(30)->construct, constructs[1].get());
  EXPECT_EQ(fe.GetBlockInfo(40)->construct, constructs[1].get());
  EXPECT_EQ(fe.GetBlockInfo(99)->construct, constructs[0].get());
}

TEST_F(SpvParserTest, LabelControlFlowConstructs_SingleBlockLoop) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %20 None
     OpBranchConditional %cond %20 %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  fe.ComputeBlockOrderAndPositions();
  fe.RegisterMerges();
  EXPECT_TRUE(fe.LabelControlFlowConstructs());
  const auto& constructs = fe.constructs();
  EXPECT_EQ(constructs.size(), 2u);
  // A single-block loop consists *only* of a continue target with one block in
  // it.
  EXPECT_THAT(ToString(constructs), Eq(R"(ConstructList{
  Construct{ Function [0,3) begin_id:10 end_id:0 depth:0 parent:null }
  Construct{ Continue [1,2) begin_id:20 end_id:99 depth:1 parent:Function@10 in-c:Continue@20 }
})")) << constructs;
  // The block records the nearest enclosing construct.
  EXPECT_EQ(fe.GetBlockInfo(10)->construct, constructs[0].get());
  EXPECT_EQ(fe.GetBlockInfo(20)->construct, constructs[1].get());
  EXPECT_EQ(fe.GetBlockInfo(99)->construct, constructs[0].get());
}

TEST_F(SpvParserTest, LabelControlFlowConstructs_MultiBlockLoop) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %40 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpBranch %40

     %40 = OpLabel
     OpBranch %50

     %50 = OpLabel
     OpBranch %20

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  fe.ComputeBlockOrderAndPositions();
  fe.RegisterMerges();
  EXPECT_TRUE(fe.LabelControlFlowConstructs());
  const auto& constructs = fe.constructs();
  EXPECT_THAT(ToString(constructs), Eq(R"(ConstructList{
  Construct{ Function [0,6) begin_id:10 end_id:0 depth:0 parent:null }
  Construct{ Continue [3,5) begin_id:40 end_id:99 depth:1 parent:Function@10 in-c:Continue@40 }
  Construct{ Loop [1,3) begin_id:20 end_id:40 depth:1 parent:Function@10 in-l:Loop@20 }
})")) << constructs;
  // The block records the nearest enclosing construct.
  EXPECT_EQ(fe.GetBlockInfo(10)->construct, constructs[0].get());
  EXPECT_EQ(fe.GetBlockInfo(20)->construct, constructs[2].get());
  EXPECT_EQ(fe.GetBlockInfo(30)->construct, constructs[2].get());
  EXPECT_EQ(fe.GetBlockInfo(40)->construct, constructs[1].get());
  EXPECT_EQ(fe.GetBlockInfo(50)->construct, constructs[1].get());
  EXPECT_EQ(fe.GetBlockInfo(99)->construct, constructs[0].get());
}

TEST_F(SpvParserTest,
       LabelControlFlowConstructs_MergeBlockIsAlsoSingleBlockLoop) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %50 None
     OpBranchConditional %cond %20 %50

     %20 = OpLabel
     OpBranch %50

     ; %50 is the merge block for the selection starting at 10,
     ; and its own continue target.
     %50 = OpLabel
     OpLoopMerge %99 %50 None
     OpBranchConditional %cond %50 %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  fe.ComputeBlockOrderAndPositions();
  fe.RegisterMerges();
  EXPECT_TRUE(fe.LabelControlFlowConstructs());
  const auto& constructs = fe.constructs();
  EXPECT_EQ(constructs.size(), 3u);
  // A single-block loop consists *only* of a continue target with one block in
  // it.
  EXPECT_THAT(ToString(constructs), Eq(R"(ConstructList{
  Construct{ Function [0,4) begin_id:10 end_id:0 depth:0 parent:null }
  Construct{ IfSelection [0,2) begin_id:10 end_id:50 depth:1 parent:Function@10 }
  Construct{ Continue [2,3) begin_id:50 end_id:99 depth:1 parent:Function@10 in-c:Continue@50 }
})")) << constructs;
  // The block records the nearest enclosing construct.
  EXPECT_EQ(fe.GetBlockInfo(10)->construct, constructs[1].get());
  EXPECT_EQ(fe.GetBlockInfo(20)->construct, constructs[1].get());
  EXPECT_EQ(fe.GetBlockInfo(50)->construct, constructs[2].get());
  EXPECT_EQ(fe.GetBlockInfo(99)->construct, constructs[0].get());
}

TEST_F(SpvParserTest,
       LabelControlFlowConstructs_MergeBlockIsAlsoMultiBlockLoopHeader) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %50 None
     OpBranchConditional %cond %20 %50

     %20 = OpLabel
     OpBranch %50

     ; %50 is the merge block for the selection starting at 10,
     ; and a loop block header but not its own continue target.
     %50 = OpLabel
     OpLoopMerge %99 %60 None
     OpBranchConditional %cond %60 %99

     %60 = OpLabel
     OpBranch %50

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  fe.ComputeBlockOrderAndPositions();
  fe.RegisterMerges();
  EXPECT_TRUE(fe.LabelControlFlowConstructs());
  const auto& constructs = fe.constructs();
  EXPECT_EQ(constructs.size(), 4u);
  EXPECT_THAT(ToString(constructs), Eq(R"(ConstructList{
  Construct{ Function [0,5) begin_id:10 end_id:0 depth:0 parent:null }
  Construct{ IfSelection [0,2) begin_id:10 end_id:50 depth:1 parent:Function@10 }
  Construct{ Continue [3,4) begin_id:60 end_id:99 depth:1 parent:Function@10 in-c:Continue@60 }
  Construct{ Loop [2,3) begin_id:50 end_id:60 depth:1 parent:Function@10 in-l:Loop@50 }
})")) << constructs;
  // The block records the nearest enclosing construct.
  EXPECT_EQ(fe.GetBlockInfo(10)->construct, constructs[1].get());
  EXPECT_EQ(fe.GetBlockInfo(20)->construct, constructs[1].get());
  EXPECT_EQ(fe.GetBlockInfo(50)->construct, constructs[3].get());
  EXPECT_EQ(fe.GetBlockInfo(60)->construct, constructs[2].get());
  EXPECT_EQ(fe.GetBlockInfo(99)->construct, constructs[0].get());
}

TEST_F(SpvParserTest, LabelControlFlowConstructs_Nest_If_If) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpBranchConditional %cond %20 %50

     %20 = OpLabel
     OpSelectionMerge %40 None
     OpBranchConditional %cond %30 %40 ;; true only

     %30 = OpLabel
     OpBranch %40

     %40 = OpLabel ; merge for first inner "if"
     OpBranch %49

     %49 = OpLabel ; an extra padding block
     OpBranch %99

     %50 = OpLabel
     OpSelectionMerge %89 None
     OpBranchConditional %cond %89 %60 ;; false only

     %60 = OpLabel
     OpBranch %89

     %89 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  fe.ComputeBlockOrderAndPositions();
  fe.RegisterMerges();
  EXPECT_TRUE(fe.LabelControlFlowConstructs());
  const auto& constructs = fe.constructs();
  EXPECT_EQ(constructs.size(), 4u);
  EXPECT_THAT(ToString(constructs), Eq(R"(ConstructList{
  Construct{ Function [0,9) begin_id:10 end_id:0 depth:0 parent:null }
  Construct{ IfSelection [0,8) begin_id:10 end_id:99 depth:1 parent:Function@10 }
  Construct{ IfSelection [1,3) begin_id:20 end_id:40 depth:2 parent:IfSelection@10 }
  Construct{ IfSelection [5,7) begin_id:50 end_id:89 depth:2 parent:IfSelection@10 }
})")) << constructs;
  // The block records the nearest enclosing construct.
  EXPECT_EQ(fe.GetBlockInfo(10)->construct, constructs[1].get());
  EXPECT_EQ(fe.GetBlockInfo(20)->construct, constructs[2].get());
  EXPECT_EQ(fe.GetBlockInfo(30)->construct, constructs[2].get());
  EXPECT_EQ(fe.GetBlockInfo(40)->construct, constructs[1].get());
  EXPECT_EQ(fe.GetBlockInfo(49)->construct, constructs[1].get());
  EXPECT_EQ(fe.GetBlockInfo(50)->construct, constructs[3].get());
  EXPECT_EQ(fe.GetBlockInfo(60)->construct, constructs[3].get());
  EXPECT_EQ(fe.GetBlockInfo(89)->construct, constructs[1].get());
  EXPECT_EQ(fe.GetBlockInfo(99)->construct, constructs[0].get());
}

TEST_F(SpvParserTest, LabelControlFlowConstructs_Nest_Switch_If) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpSwitch %selector %99 20 %20 50 %50

     %20 = OpLabel ; if-then nested in case 20
     OpSelectionMerge %49 None
     OpBranchConditional %cond %30 %49

     %30 = OpLabel
     OpBranch %49

     %49 = OpLabel
     OpBranch %99

     %50 = OpLabel ; unles-then nested in case 50
     OpSelectionMerge %89 None
     OpBranchConditional %cond %89 %60

     %60 = OpLabel
     OpBranch %89

     %89 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  fe.ComputeBlockOrderAndPositions();
  fe.RegisterMerges();
  EXPECT_TRUE(fe.LabelControlFlowConstructs());
  const auto& constructs = fe.constructs();
  EXPECT_EQ(constructs.size(), 4u);
  // The ordering among siblings depends on the computed block order.
  EXPECT_THAT(ToString(constructs), Eq(R"(ConstructList{
  Construct{ Function [0,8) begin_id:10 end_id:0 depth:0 parent:null }
  Construct{ SwitchSelection [0,7) begin_id:10 end_id:99 depth:1 parent:Function@10 in-c-l-s:SwitchSelection@10 }
  Construct{ IfSelection [1,3) begin_id:50 end_id:89 depth:2 parent:SwitchSelection@10 in-c-l-s:SwitchSelection@10 }
  Construct{ IfSelection [4,6) begin_id:20 end_id:49 depth:2 parent:SwitchSelection@10 in-c-l-s:SwitchSelection@10 }
})")) << constructs;
  // The block records the nearest enclosing construct.
  EXPECT_EQ(fe.GetBlockInfo(10)->construct, constructs[1].get());
  EXPECT_EQ(fe.GetBlockInfo(20)->construct, constructs[3].get());
  EXPECT_EQ(fe.GetBlockInfo(30)->construct, constructs[3].get());
  EXPECT_EQ(fe.GetBlockInfo(49)->construct, constructs[1].get());
  EXPECT_EQ(fe.GetBlockInfo(50)->construct, constructs[2].get());
  EXPECT_EQ(fe.GetBlockInfo(60)->construct, constructs[2].get());
  EXPECT_EQ(fe.GetBlockInfo(89)->construct, constructs[1].get());
  EXPECT_EQ(fe.GetBlockInfo(99)->construct, constructs[0].get());
}

TEST_F(SpvParserTest, LabelControlFlowConstructs_Nest_If_Switch) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpBranchConditional %cond %20 %99

     %20 = OpLabel
     OpSelectionMerge %89 None
     OpSwitch %selector %89 20 %30

     %30 = OpLabel
     OpBranch %89

     %89 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  fe.ComputeBlockOrderAndPositions();
  fe.RegisterMerges();
  EXPECT_TRUE(fe.LabelControlFlowConstructs());
  const auto& constructs = fe.constructs();
  EXPECT_EQ(constructs.size(), 3u);
  EXPECT_THAT(ToString(constructs), Eq(R"(ConstructList{
  Construct{ Function [0,5) begin_id:10 end_id:0 depth:0 parent:null }
  Construct{ IfSelection [0,4) begin_id:10 end_id:99 depth:1 parent:Function@10 }
  Construct{ SwitchSelection [1,3) begin_id:20 end_id:89 depth:2 parent:IfSelection@10 in-c-l-s:SwitchSelection@20 }
})")) << constructs;
  // The block records the nearest enclosing construct.
  EXPECT_EQ(fe.GetBlockInfo(10)->construct, constructs[1].get());
  EXPECT_EQ(fe.GetBlockInfo(20)->construct, constructs[2].get());
  EXPECT_EQ(fe.GetBlockInfo(30)->construct, constructs[2].get());
  EXPECT_EQ(fe.GetBlockInfo(89)->construct, constructs[1].get());
  EXPECT_EQ(fe.GetBlockInfo(99)->construct, constructs[0].get());
}

TEST_F(SpvParserTest, LabelControlFlowConstructs_Nest_Loop_Loop) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %89 %50 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel ; single block loop
     OpLoopMerge %40 %30 None
     OpBranchConditional %cond2 %30 %40

     %40 = OpLabel ; padding block
     OpBranch %50

     %50 = OpLabel ; outer continue target
     OpBranch %60

     %60 = OpLabel
     OpBranch %20

     %89 = OpLabel ; outer merge
     OpBranch %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  fe.ComputeBlockOrderAndPositions();
  fe.RegisterMerges();
  EXPECT_TRUE(fe.LabelControlFlowConstructs());
  const auto& constructs = fe.constructs();
  EXPECT_EQ(constructs.size(), 4u);
  EXPECT_THAT(ToString(constructs), Eq(R"(ConstructList{
  Construct{ Function [0,8) begin_id:10 end_id:0 depth:0 parent:null }
  Construct{ Continue [4,6) begin_id:50 end_id:89 depth:1 parent:Function@10 in-c:Continue@50 }
  Construct{ Loop [1,4) begin_id:20 end_id:50 depth:1 parent:Function@10 in-l:Loop@20 }
  Construct{ Continue [2,3) begin_id:30 end_id:40 depth:2 parent:Loop@20 in-l:Loop@20 in-c:Continue@30 }
})")) << constructs;
  // The block records the nearest enclosing construct.
  EXPECT_EQ(fe.GetBlockInfo(10)->construct, constructs[0].get());
  EXPECT_EQ(fe.GetBlockInfo(20)->construct, constructs[2].get());
  EXPECT_EQ(fe.GetBlockInfo(30)->construct, constructs[3].get());
  EXPECT_EQ(fe.GetBlockInfo(40)->construct, constructs[2].get());
  EXPECT_EQ(fe.GetBlockInfo(50)->construct, constructs[1].get());
  EXPECT_EQ(fe.GetBlockInfo(60)->construct, constructs[1].get());
  EXPECT_EQ(fe.GetBlockInfo(89)->construct, constructs[0].get());
  EXPECT_EQ(fe.GetBlockInfo(99)->construct, constructs[0].get());
}

TEST_F(SpvParserTest, LabelControlFlowConstructs_Nest_Loop_If) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %80 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel ; If, nested in the loop construct
     OpSelectionMerge %49 None
     OpBranchConditional %cond2 %40 %49

     %40 = OpLabel
     OpBranch %49

     %49 = OpLabel ; merge for inner if
     OpBranch %80

     %80 = OpLabel ; continue target
     OpBranch %20

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  fe.ComputeBlockOrderAndPositions();
  fe.RegisterMerges();
  EXPECT_TRUE(fe.LabelControlFlowConstructs());
  const auto& constructs = fe.constructs();
  EXPECT_EQ(constructs.size(), 4u);
  EXPECT_THAT(ToString(constructs), Eq(R"(ConstructList{
  Construct{ Function [0,7) begin_id:10 end_id:0 depth:0 parent:null }
  Construct{ Continue [5,6) begin_id:80 end_id:99 depth:1 parent:Function@10 in-c:Continue@80 }
  Construct{ Loop [1,5) begin_id:20 end_id:80 depth:1 parent:Function@10 in-l:Loop@20 }
  Construct{ IfSelection [2,4) begin_id:30 end_id:49 depth:2 parent:Loop@20 in-l:Loop@20 }
})")) << constructs;
  // The block records the nearest enclosing construct.
  EXPECT_EQ(fe.GetBlockInfo(10)->construct, constructs[0].get());
  EXPECT_EQ(fe.GetBlockInfo(20)->construct, constructs[2].get());
  EXPECT_EQ(fe.GetBlockInfo(30)->construct, constructs[3].get());
  EXPECT_EQ(fe.GetBlockInfo(40)->construct, constructs[3].get());
  EXPECT_EQ(fe.GetBlockInfo(49)->construct, constructs[2].get());
  EXPECT_EQ(fe.GetBlockInfo(80)->construct, constructs[1].get());
  EXPECT_EQ(fe.GetBlockInfo(99)->construct, constructs[0].get());
}

TEST_F(SpvParserTest, LabelControlFlowConstructs_Nest_LoopContinue_If) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %30 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel ; If, nested at the top of the continue construct head
     OpSelectionMerge %49 None
     OpBranchConditional %cond2 %40 %49

     %40 = OpLabel
     OpBranch %49

     %49 = OpLabel ; merge for inner if, backedge
     OpBranch %20

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  fe.ComputeBlockOrderAndPositions();
  fe.RegisterMerges();
  EXPECT_TRUE(fe.LabelControlFlowConstructs());
  const auto& constructs = fe.constructs();
  EXPECT_EQ(constructs.size(), 4u);
  EXPECT_THAT(ToString(constructs), Eq(R"(ConstructList{
  Construct{ Function [0,6) begin_id:10 end_id:0 depth:0 parent:null }
  Construct{ Continue [2,5) begin_id:30 end_id:99 depth:1 parent:Function@10 in-c:Continue@30 }
  Construct{ Loop [1,2) begin_id:20 end_id:30 depth:1 parent:Function@10 in-l:Loop@20 }
  Construct{ IfSelection [2,4) begin_id:30 end_id:49 depth:2 parent:Continue@30 in-c:Continue@30 }
})")) << constructs;
  // The block records the nearest enclosing construct.
  EXPECT_EQ(fe.GetBlockInfo(10)->construct, constructs[0].get());
  EXPECT_EQ(fe.GetBlockInfo(20)->construct, constructs[2].get());
  EXPECT_EQ(fe.GetBlockInfo(30)->construct, constructs[3].get());
  EXPECT_EQ(fe.GetBlockInfo(40)->construct, constructs[3].get());
  EXPECT_EQ(fe.GetBlockInfo(49)->construct, constructs[1].get());
  EXPECT_EQ(fe.GetBlockInfo(99)->construct, constructs[0].get());
}

TEST_F(SpvParserTest, LabelControlFlowConstructs_Nest_If_SingleBlockLoop) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpBranchConditional %cond %20 %99

     %20 = OpLabel
     OpLoopMerge %89 %20 None
     OpBranchConditional %cond %20 %99

     %89 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  fe.ComputeBlockOrderAndPositions();
  fe.RegisterMerges();
  EXPECT_TRUE(fe.LabelControlFlowConstructs());
  const auto& constructs = fe.constructs();
  EXPECT_EQ(constructs.size(), 3u);
  EXPECT_THAT(ToString(constructs), Eq(R"(ConstructList{
  Construct{ Function [0,4) begin_id:10 end_id:0 depth:0 parent:null }
  Construct{ IfSelection [0,3) begin_id:10 end_id:99 depth:1 parent:Function@10 }
  Construct{ Continue [1,2) begin_id:20 end_id:89 depth:2 parent:IfSelection@10 in-c:Continue@20 }
})")) << constructs;
  // The block records the nearest enclosing construct.
  EXPECT_EQ(fe.GetBlockInfo(10)->construct, constructs[1].get());
  EXPECT_EQ(fe.GetBlockInfo(20)->construct, constructs[2].get());
  EXPECT_EQ(fe.GetBlockInfo(99)->construct, constructs[0].get());
}

TEST_F(SpvParserTest, LabelControlFlowConstructs_Nest_If_MultiBlockLoop) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpBranchConditional %cond %20 %99

     %20 = OpLabel ; start loop body
     OpLoopMerge %89 %40 None
     OpBranchConditional %cond %30 %89

     %30 = OpLabel ; body block
     OpBranch %40

     %40 = OpLabel ; continue target
     OpBranch %50

     %50 = OpLabel ; backedge block
     OpBranch %20

     %89 = OpLabel ; merge for the loop
     OpBranch %20

     %99 = OpLabel ; merge for the if
     OpReturn

     OpFunctionEnd
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  fe.ComputeBlockOrderAndPositions();
  fe.RegisterMerges();
  EXPECT_TRUE(fe.LabelControlFlowConstructs());
  const auto& constructs = fe.constructs();
  EXPECT_EQ(constructs.size(), 4u);
  EXPECT_THAT(ToString(constructs), Eq(R"(ConstructList{
  Construct{ Function [0,7) begin_id:10 end_id:0 depth:0 parent:null }
  Construct{ IfSelection [0,6) begin_id:10 end_id:99 depth:1 parent:Function@10 }
  Construct{ Continue [3,5) begin_id:40 end_id:89 depth:2 parent:IfSelection@10 in-c:Continue@40 }
  Construct{ Loop [1,3) begin_id:20 end_id:40 depth:2 parent:IfSelection@10 in-l:Loop@20 }
})")) << constructs;
  // The block records the nearest enclosing construct.
  EXPECT_EQ(fe.GetBlockInfo(10)->construct, constructs[1].get());
  EXPECT_EQ(fe.GetBlockInfo(20)->construct, constructs[3].get());
  EXPECT_EQ(fe.GetBlockInfo(30)->construct, constructs[3].get());
  EXPECT_EQ(fe.GetBlockInfo(40)->construct, constructs[2].get());
  EXPECT_EQ(fe.GetBlockInfo(50)->construct, constructs[2].get());
  EXPECT_EQ(fe.GetBlockInfo(89)->construct, constructs[1].get());
  EXPECT_EQ(fe.GetBlockInfo(99)->construct, constructs[0].get());
}

TEST_F(SpvParserTest, FindSwitchCaseHeaders_DefaultIsLongRangeBackedge) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpSelectionMerge %99 None
     OpSwitch %selector %10 30 %30

     %30 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  fe.ComputeBlockOrderAndPositions();
  EXPECT_TRUE(fe.VerifyHeaderContinueMergeOrder());
  fe.RegisterMerges();
  fe.LabelControlFlowConstructs();
  EXPECT_FALSE(fe.FindSwitchCaseHeaders());
  EXPECT_THAT(p->error(), Eq("Switch branch from block 20 to default target "
                             "block 10 can't be a back-edge"));
}

TEST_F(SpvParserTest, FindSwitchCaseHeaders_DefaultIsSelfLoop) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpSelectionMerge %99 None
     OpSwitch %selector %20 30 %30

     %30 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  fe.ComputeBlockOrderAndPositions();
  EXPECT_TRUE(fe.VerifyHeaderContinueMergeOrder());
  fe.RegisterMerges();
  fe.LabelControlFlowConstructs();
  EXPECT_FALSE(fe.FindSwitchCaseHeaders());
  // Self-loop that isn't its own continue target is already rejected with a
  // different message.
  EXPECT_THAT(
      p->error(),
      Eq("Block 20 branches to itself but is not its own continue target"));
}

TEST_F(SpvParserTest, FindSwitchCaseHeaders_DefaultCantEscapeSwitch) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %50 None
     OpSwitch %selector %99 30 %30 ; default goes past the merge

     %30 = OpLabel
     OpBranch %50

     %50 = OpLabel ; merge
     OpBranch %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  fe.ComputeBlockOrderAndPositions();
  EXPECT_TRUE(fe.VerifyHeaderContinueMergeOrder());
  fe.RegisterMerges();
  fe.LabelControlFlowConstructs();
  EXPECT_FALSE(fe.FindSwitchCaseHeaders());
  EXPECT_THAT(p->error(), Eq("Switch branch from block 10 to default block 99 "
                             "escapes the selection construct"));
}

TEST_F(SpvParserTest, FindSwitchCaseHeaders_DefaultForTwoSwitches) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpSwitch %selector %89 20 %20

     %20 = OpLabel
     OpBranch %50

     %50 = OpLabel
     OpSelectionMerge %89 None
     OpSwitch %selector %89 60 %60

     %60 = OpLabel
     OpBranch %89

     %89 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  fe.ComputeBlockOrderAndPositions();
  EXPECT_TRUE(fe.VerifyHeaderContinueMergeOrder());
  fe.RegisterMerges();
  fe.LabelControlFlowConstructs();
  EXPECT_FALSE(fe.FindSwitchCaseHeaders());
  EXPECT_THAT(p->error(), Eq("Block 89 is declared as the default target for "
                             "two OpSwitch instructions, at blocks 10 and 50"));
}

TEST_F(SpvParserTest, FindSwitchCaseHeaders_CaseIsLongRangeBackedge) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpSelectionMerge %99 None
     OpSwitch %selector %99 10 %10

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  fe.ComputeBlockOrderAndPositions();
  EXPECT_TRUE(fe.VerifyHeaderContinueMergeOrder());
  fe.RegisterMerges();
  fe.LabelControlFlowConstructs();
  EXPECT_FALSE(fe.FindSwitchCaseHeaders());
  EXPECT_THAT(p->error(), Eq("Switch branch from block 20 to case target "
                             "block 10 can't be a back-edge"));
}

TEST_F(SpvParserTest, FindSwitchCaseHeaders_CaseIsSelfLoop) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpSelectionMerge %99 None
     OpSwitch %selector %99 20 %20

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  fe.ComputeBlockOrderAndPositions();
  EXPECT_TRUE(fe.VerifyHeaderContinueMergeOrder());
  fe.RegisterMerges();
  fe.LabelControlFlowConstructs();
  EXPECT_FALSE(fe.FindSwitchCaseHeaders());
  // The error is caught earlier
  EXPECT_THAT(
      p->error(),
      Eq("Block 20 branches to itself but is not its own continue target"));
}

TEST_F(SpvParserTest, FindSwitchCaseHeaders_CaseCanBeSwitchMerge) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpSelectionMerge %99 None
     OpSwitch %selector %99 20 %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  fe.ComputeBlockOrderAndPositions();
  EXPECT_TRUE(fe.VerifyHeaderContinueMergeOrder());
  fe.RegisterMerges();
  fe.LabelControlFlowConstructs();
  EXPECT_TRUE(fe.FindSwitchCaseHeaders());
}

TEST_F(SpvParserTest, FindSwitchCaseHeaders_CaseCantEscapeSwitch) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None ; force %99 to be very late in block order
     OpBranchConditional %cond %20 %99

     %20 = OpLabel
     OpSelectionMerge %89 None
     OpSwitch %selector %89 20 %99

     %89 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  fe.ComputeBlockOrderAndPositions();
  EXPECT_TRUE(fe.VerifyHeaderContinueMergeOrder());
  fe.RegisterMerges();
  fe.LabelControlFlowConstructs();
  EXPECT_FALSE(fe.FindSwitchCaseHeaders());
  EXPECT_THAT(p->error(), Eq("Switch branch from block 20 to case target block "
                             "99 escapes the selection construct"));
}

TEST_F(SpvParserTest, FindSwitchCaseHeaders_CaseForMoreThanOneSwitch) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpSwitch %selector %99 20 %20 50 %50

     %20 = OpLabel
     OpSelectionMerge %89 None
     OpSwitch %selector %89 50 %50

     %50 = OpLabel
     OpBranch %89

     %89 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  fe.ComputeBlockOrderAndPositions();
  EXPECT_TRUE(fe.VerifyHeaderContinueMergeOrder());
  fe.RegisterMerges();
  fe.LabelControlFlowConstructs();
  EXPECT_FALSE(fe.FindSwitchCaseHeaders());
  EXPECT_THAT(p->error(),
              Eq("Block 50 is declared as the switch case target for two "
                 "OpSwitch instructions, at blocks 10 and 20"));
}

TEST_F(SpvParserTest, FindSwitchCaseHeaders_CaseIsMergeForAnotherConstruct) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %49 None
     OpSwitch %selector %49 20 %20

     %20 = OpLabel
     OpBranch %49

     %49 = OpLabel
     OpBranch %50

     %50 = OpLabel
     OpSelectionMerge %20 None ; points back to the case.
     OpBranchConditional %cond %60 %99

     %60 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  fe.ComputeBlockOrderAndPositions();
  EXPECT_TRUE(fe.VerifyHeaderContinueMergeOrder());
  fe.RegisterMerges();
  fe.LabelControlFlowConstructs();
  EXPECT_FALSE(fe.FindSwitchCaseHeaders());
  EXPECT_THAT(p->error(), Eq("Switch branch from block 10 to case target block "
                             "20 escapes the selection construct"));
}

TEST_F(SpvParserTest, FindSwitchCaseHeaders_NoSwitch) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpReturn

     OpFunctionEnd
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  fe.ComputeBlockOrderAndPositions();
  EXPECT_TRUE(fe.VerifyHeaderContinueMergeOrder());
  fe.RegisterMerges();
  fe.LabelControlFlowConstructs();
  EXPECT_TRUE(fe.FindSwitchCaseHeaders());

  const auto* bi10 = fe.GetBlockInfo(10);
  ASSERT_NE(bi10, nullptr);
  EXPECT_EQ(bi10->case_head_for, nullptr);
  EXPECT_EQ(bi10->default_head_for, nullptr);
  EXPECT_FALSE(bi10->default_is_merge);
  EXPECT_EQ(bi10->case_values.get(), nullptr);
}

TEST_F(SpvParserTest, FindSwitchCaseHeaders_DefaultIsMerge) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpSwitch %selector %99 20 %20

     %20 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  fe.ComputeBlockOrderAndPositions();
  EXPECT_TRUE(fe.VerifyHeaderContinueMergeOrder());
  fe.RegisterMerges();
  fe.LabelControlFlowConstructs();
  EXPECT_TRUE(fe.FindSwitchCaseHeaders());

  const auto* bi99 = fe.GetBlockInfo(99);
  ASSERT_NE(bi99, nullptr);
  EXPECT_EQ(bi99->case_head_for, nullptr);
  ASSERT_NE(bi99->default_head_for, nullptr);
  EXPECT_EQ(bi99->default_head_for->begin_id, 10u);
  EXPECT_TRUE(bi99->default_is_merge);
  EXPECT_EQ(bi99->case_values.get(), nullptr);
}

TEST_F(SpvParserTest, FindSwitchCaseHeaders_DefaultIsNotMerge) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpSwitch %selector %30 20 %20

     %20 = OpLabel
     OpBranch %99

     %30 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  fe.ComputeBlockOrderAndPositions();
  EXPECT_TRUE(fe.VerifyHeaderContinueMergeOrder());
  fe.RegisterMerges();
  fe.LabelControlFlowConstructs();
  EXPECT_TRUE(fe.FindSwitchCaseHeaders());

  const auto* bi30 = fe.GetBlockInfo(30);
  ASSERT_NE(bi30, nullptr);
  EXPECT_EQ(bi30->case_head_for, nullptr);
  ASSERT_NE(bi30->default_head_for, nullptr);
  EXPECT_EQ(bi30->default_head_for->begin_id, 10u);
  EXPECT_FALSE(bi30->default_is_merge);
  EXPECT_EQ(bi30->case_values.get(), nullptr);
}

TEST_F(SpvParserTest, FindSwitchCaseHeaders_CaseIsNotDefault) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpSwitch %selector %30 200 %20

     %20 = OpLabel
     OpBranch %99

     %30 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  fe.ComputeBlockOrderAndPositions();
  EXPECT_TRUE(fe.VerifyHeaderContinueMergeOrder());
  fe.RegisterMerges();
  fe.LabelControlFlowConstructs();
  EXPECT_TRUE(fe.FindSwitchCaseHeaders());

  const auto* bi20 = fe.GetBlockInfo(20);
  ASSERT_NE(bi20, nullptr);
  ASSERT_NE(bi20->case_head_for, nullptr);
  EXPECT_EQ(bi20->case_head_for->begin_id, 10u);
  EXPECT_EQ(bi20->default_head_for, nullptr);
  EXPECT_FALSE(bi20->default_is_merge);
  EXPECT_THAT(*(bi20->case_values.get()), UnorderedElementsAre(200));
}

TEST_F(SpvParserTest, FindSwitchCaseHeaders_CaseIsDefault) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpSwitch %selector %20 200 %20

     %20 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  fe.ComputeBlockOrderAndPositions();
  EXPECT_TRUE(fe.VerifyHeaderContinueMergeOrder());
  fe.RegisterMerges();
  fe.LabelControlFlowConstructs();
  EXPECT_TRUE(fe.FindSwitchCaseHeaders());

  const auto* bi20 = fe.GetBlockInfo(20);
  ASSERT_NE(bi20, nullptr);
  ASSERT_NE(bi20->case_head_for, nullptr);
  EXPECT_EQ(bi20->case_head_for->begin_id, 10u);
  EXPECT_EQ(bi20->default_head_for, bi20->case_head_for);
  EXPECT_FALSE(bi20->default_is_merge);
  EXPECT_THAT(*(bi20->case_values.get()), UnorderedElementsAre(200));
}

TEST_F(SpvParserTest, FindSwitchCaseHeaders_ManyCasesWithSameValue_IsError) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpSwitch %selector %99 200 %20 200 %30

     %20 = OpLabel
     OpBranch %99

     %30 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  fe.ComputeBlockOrderAndPositions();
  EXPECT_TRUE(fe.VerifyHeaderContinueMergeOrder());
  fe.RegisterMerges();
  fe.LabelControlFlowConstructs();
  EXPECT_FALSE(fe.FindSwitchCaseHeaders());

  EXPECT_THAT(p->error(),
              Eq("Duplicate case value 200 in OpSwitch in block 10"));
}

TEST_F(SpvParserTest, FindSwitchCaseHeaders_ManyValuesWithSameCase) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpSwitch %selector %99 200 %20 300 %20

     %20 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpReturn

     OpFunctionEnd
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  fe.ComputeBlockOrderAndPositions();
  EXPECT_TRUE(fe.VerifyHeaderContinueMergeOrder());
  fe.RegisterMerges();
  fe.LabelControlFlowConstructs();
  EXPECT_TRUE(fe.FindSwitchCaseHeaders());

  const auto* bi20 = fe.GetBlockInfo(20);
  ASSERT_NE(bi20, nullptr);
  ASSERT_NE(bi20->case_head_for, nullptr);
  EXPECT_EQ(bi20->case_head_for->begin_id, 10u);
  EXPECT_EQ(bi20->default_head_for, nullptr);
  EXPECT_FALSE(bi20->default_is_merge);
  EXPECT_THAT(*(bi20->case_values.get()), UnorderedElementsAre(200, 300));
}

TEST_F(SpvParserTest, DISABLED_BranchEscapesIfConstruct) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpBranchConditional %cond %20 %99

     %20 = OpLabel
     OpSelectionMerge %50 None
     OpBranchConditional %cond2 %30 %50

     %30 = OpLabel
     OpBranch %80   ; bad exit to %80

     %50 = OpLabel
     OpBranch %80

     %80 = OpLabel  ; bad target
     OpBranch %99

     %99 = OpLabel
     OpReturn
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  fe.ComputeBlockOrderAndPositions();
  EXPECT_TRUE(fe.VerifyHeaderContinueMergeOrder());
  fe.RegisterMerges();
  fe.LabelControlFlowConstructs();
  fe.FindSwitchCaseHeaders();
  // Some further processing
  EXPECT_THAT(p->error(), Eq("something"));
}

TEST_F(SpvParserTest, ClassifyCFGEdges_ReturnInContinueConstruct) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %50 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel ; body
     OpBranch %50

     %50 = OpLabel
     OpReturn

     %99 = OpLabel
     OpReturn
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_FALSE(FlowClassifyCFGEdges(&fe)) << p->error();
  EXPECT_THAT(p->error(), Eq("Invalid function exit at block 50 from continue "
                             "construct starting at 50"));
}

TEST_F(SpvParserTest, ClassifyCFGEdges_KillInContinueConstruct) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %50 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel ; body
     OpBranch %50

     %50 = OpLabel
     OpKill

     %99 = OpLabel
     OpReturn
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_FALSE(FlowClassifyCFGEdges(&fe));
  EXPECT_THAT(p->error(), Eq("Invalid function exit at block 50 from continue "
                             "construct starting at 50"));
}

TEST_F(SpvParserTest, ClassifyCFGEdges_UnreachableInContinueConstruct) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %50 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel ; body
     OpBranch %50

     %50 = OpLabel
     OpUnreachable

     %99 = OpLabel
     OpReturn
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_FALSE(FlowClassifyCFGEdges(&fe));
  EXPECT_THAT(p->error(), Eq("Invalid function exit at block 50 from continue "
                             "construct starting at 50"));
}

TEST_F(SpvParserTest, ClassifyCFGEdges_BackEdge_NotInContinueConstruct) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %50 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel ; body
     OpBranch %20  ; bad backedge

     %50 = OpLabel ; continue target
     OpBranch %99

     %99 = OpLabel
     OpReturn
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_FALSE(FlowClassifyCFGEdges(&fe));
  EXPECT_THAT(
      p->error(),
      Eq("Invalid backedge (30->20): 30 is not in a continue construct"));
}

TEST_F(SpvParserTest,
       ClassifyCFGEdges_BackEdge_NotInLastBlockOfContinueConstruct) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %50 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel ; body
     OpBranch %50

     %50 = OpLabel ; continue target
     OpBranchConditional %cond %20 %60 ; bad branch to %20

     %60 = OpLabel ; end of continue construct
     OpBranch %20

     %99 = OpLabel
     OpReturn
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_FALSE(FlowClassifyCFGEdges(&fe));
  EXPECT_THAT(p->error(),
              Eq("Invalid exit (50->20) from continue construct: 50 is not the "
                 "last block in the continue construct starting at 50 "
                 "(violates post-dominance rule)"));
}

TEST_F(SpvParserTest, ClassifyCFGEdges_BackEdge_ToWrongHeader) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpBranchConditional %cond %20 %99

     %20 = OpLabel
     OpLoopMerge %89 %50 None
     OpBranchConditional %cond %30 %89

     %30 = OpLabel ; loop body
     OpBranch %50

     %50 = OpLabel ; continue target
     OpBranch %10

     %89 = OpLabel ; inner merge
     OpBranch %99

     %99 = OpLabel ; outer merge
     OpReturn
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_FALSE(FlowClassifyCFGEdges(&fe));
  EXPECT_THAT(p->error(), Eq("Invalid backedge (50->10): does not branch to "
                             "the corresponding loop header, expected 20"));
}

TEST_F(SpvParserTest, ClassifyCFGEdges_BackEdge_SingleBlockLoop) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %20 None
     OpBranchConditional %cond %20 %99

     %99 = OpLabel ; outer merge
     OpReturn
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(FlowClassifyCFGEdges(&fe));

  auto* bi20 = fe.GetBlockInfo(20);
  ASSERT_NE(bi20, nullptr);
  EXPECT_EQ(bi20->succ_edge.count(20), 1u);
  EXPECT_EQ(bi20->succ_edge[20], EdgeKind::kBack);
}

TEST_F(SpvParserTest,
       ClassifyCFGEdges_BackEdge_MultiBlockLoop_SingleBlockContinueConstruct) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %40 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpBranch %40

     %40 = OpLabel ; continue target
     OpBranch %20  ; good back edge

     %99 = OpLabel ; outer merge
     OpReturn
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(FlowClassifyCFGEdges(&fe));

  auto* bi40 = fe.GetBlockInfo(40);
  ASSERT_NE(bi40, nullptr);
  EXPECT_EQ(bi40->succ_edge.count(20), 1u);
  EXPECT_EQ(bi40->succ_edge[20], EdgeKind::kBack);
}

TEST_F(SpvParserTest,
       ClassifyCFGEdges_BackEdge_MultiBlockLoop_MultiBlockContinueConstruct) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %40 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpBranch %40

     %40 = OpLabel ; continue target
     OpBranch %50

     %50 = OpLabel
     OpBranch %20  ; good back edge

     %99 = OpLabel ; outer merge
     OpReturn
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(FlowClassifyCFGEdges(&fe));

  auto* bi50 = fe.GetBlockInfo(50);
  ASSERT_NE(bi50, nullptr);
  EXPECT_EQ(bi50->succ_edge.count(20), 1u);
  EXPECT_EQ(bi50->succ_edge[20], EdgeKind::kBack);
}

TEST_F(SpvParserTest, ClassifyCFGEdges_PrematureExitFromContinueConstruct) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %40 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpBranch %40

     %40 = OpLabel ; continue construct
     OpBranchConditional %cond2 %99 %50 ; invalid early exit

     %50 = OpLabel
     OpBranch %20  ; back edge

     %99 = OpLabel ; outer merge
     OpReturn
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_FALSE(FlowClassifyCFGEdges(&fe));
  EXPECT_THAT(p->error(),
              Eq("Invalid exit (40->99) from continue construct: 40 is not the "
                 "last block in the continue construct starting at 40 "
                 "(violates post-dominance rule)"));
}

TEST_F(SpvParserTest,
       ClassifyCFGEdges_LoopBreak_FromLoopHeader_SingleBlockLoop) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel    ; single block loop
     OpLoopMerge %99 %20 None
     OpBranchConditional %cond %20 %99

     %99 = OpLabel ; outer merge
     OpReturn
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(FlowClassifyCFGEdges(&fe));

  auto* bi = fe.GetBlockInfo(20);
  ASSERT_NE(bi, nullptr);
  EXPECT_EQ(bi->succ_edge.count(99), 1u);
  EXPECT_EQ(bi->succ_edge[99], EdgeKind::kLoopBreak);
}

TEST_F(SpvParserTest,
       ClassifyCFGEdges_LoopBreak_FromLoopHeader_MultiBlockLoop) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %30 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpBranch %20

     %99 = OpLabel
     OpReturn
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(FlowClassifyCFGEdges(&fe));

  auto* bi = fe.GetBlockInfo(20);
  ASSERT_NE(bi, nullptr);
  EXPECT_EQ(bi->succ_edge.count(99), 1u);
  EXPECT_EQ(bi->succ_edge[99], EdgeKind::kLoopBreak);
}

TEST_F(SpvParserTest, ClassifyCFGEdges_LoopBreak_FromContinueConstructHeader) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %30 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel ; Single block continue construct
     OpBranchConditional %cond2 %20 %99

     %99 = OpLabel
     OpReturn
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(FlowClassifyCFGEdges(&fe));

  auto* bi = fe.GetBlockInfo(30);
  ASSERT_NE(bi, nullptr);
  EXPECT_EQ(bi->succ_edge.count(99), 1u);
  EXPECT_EQ(bi->succ_edge[99], EdgeKind::kLoopBreak);
}

TEST_F(SpvParserTest, ClassifyCFGEdges_IfBreak_FromIfHeader) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpBranchConditional %cond %20 %99

     %20 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpReturn
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(FlowClassifyCFGEdges(&fe));

  auto* bi = fe.GetBlockInfo(20);
  ASSERT_NE(bi, nullptr);
  EXPECT_EQ(bi->succ_edge.count(99), 1u);
  EXPECT_EQ(bi->succ_edge[99], EdgeKind::kIfBreak);
}

TEST_F(SpvParserTest, ClassifyCFGEdges_IfBreak_FromIfThenElse) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpBranchConditional %cond %20 %50

     %20 = OpLabel
     OpBranch %99

     %50 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpReturn
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(FlowClassifyCFGEdges(&fe));

  // Then clause
  auto* bi20 = fe.GetBlockInfo(20);
  ASSERT_NE(bi20, nullptr);
  EXPECT_EQ(bi20->succ_edge.count(99), 1u);
  EXPECT_EQ(bi20->succ_edge[99], EdgeKind::kIfBreak);

  // Else clause
  auto* bi50 = fe.GetBlockInfo(50);
  ASSERT_NE(bi50, nullptr);
  EXPECT_EQ(bi50->succ_edge.count(99), 1u);
  EXPECT_EQ(bi50->succ_edge[99], EdgeKind::kIfBreak);
}

TEST_F(SpvParserTest, ClassifyCFGEdge_IfBreak_BypassesMerge_IsError) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %50 None
     OpBranchConditional %cond %20 %50

     %20 = OpLabel
     OpBranch %99

     %50 = OpLabel ; merge
     OpBranch %99

     %99 = OpLabel
     OpReturn
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_FALSE(FlowClassifyCFGEdges(&fe));
  EXPECT_THAT(
      p->error(),
      Eq("Branch from block 20 to block 99 is an invalid exit from "
         "construct starting at block 10; branch bypasses merge block 50"));
}

TEST_F(SpvParserTest, ClassifyCFGEdges_SwitchBreak_FromSwitchCaseDirect) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpSwitch %selector %30 20 %99 ; directly to merge

     %30 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpReturn
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(FlowClassifyCFGEdges(&fe));

  auto* bi = fe.GetBlockInfo(10);
  ASSERT_NE(bi, nullptr);
  EXPECT_EQ(bi->succ_edge.count(99), 1u);
  EXPECT_EQ(bi->succ_edge[99], EdgeKind::kSwitchBreak);
}

TEST_F(SpvParserTest, ClassifyCFGEdges_SwitchBreak_FromSwitchCaseBody) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpSwitch %selector %99 20 %20

     %20 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpReturn
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(FlowClassifyCFGEdges(&fe));

  auto* bi = fe.GetBlockInfo(20);
  ASSERT_NE(bi, nullptr);
  EXPECT_EQ(bi->succ_edge.count(99), 1u);
  EXPECT_EQ(bi->succ_edge[99], EdgeKind::kSwitchBreak);
}

TEST_F(SpvParserTest, ClassifyCFGEdges_SwitchBreak_FromSwitchDefaultBody) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpSwitch %selector %30 20 %20

     %20 = OpLabel
     OpBranch %99

     %30 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpReturn
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(FlowClassifyCFGEdges(&fe));

  auto* bi = fe.GetBlockInfo(30);
  ASSERT_NE(bi, nullptr);
  EXPECT_EQ(bi->succ_edge.count(99), 1u);
  EXPECT_EQ(bi->succ_edge[99], EdgeKind::kSwitchBreak);
}

TEST_F(SpvParserTest, ClassifyCFGEdges_SwitchBreak_FromSwitchDefaultIsMerge) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpSwitch %selector %99 20 %20

     %20 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpReturn
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(FlowClassifyCFGEdges(&fe));

  auto* bi = fe.GetBlockInfo(10);
  ASSERT_NE(bi, nullptr);
  EXPECT_EQ(bi->succ_edge.count(99), 1u);
  EXPECT_EQ(bi->succ_edge[99], EdgeKind::kSwitchBreak);
}

TEST_F(SpvParserTest, ClassifyCFGEdges_SwitchBreak_FromNestedIf_Unconditional) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpSwitch %selector %99 20 %20

     %20 = OpLabel
     OpSelectionMerge %80 None
     OpBranchConditional %cond %30 %80

     %30 = OpLabel
     OpBranch %99

     %80 = OpLabel ; inner merge
     OpBranch %99

     %99 = OpLabel ; outer merge
     OpReturn
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(FlowClassifyCFGEdges(&fe));

  auto* bi = fe.GetBlockInfo(30);
  ASSERT_NE(bi, nullptr);
  EXPECT_EQ(bi->succ_edge.count(99), 1u);
  EXPECT_EQ(bi->succ_edge[99], EdgeKind::kSwitchBreak);
}

TEST_F(SpvParserTest, ClassifyCFGEdges_SwitchBreak_FromNestedIf_Conditional) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpSwitch %selector %99 20 %20

     %20 = OpLabel
     OpSelectionMerge %80 None
     OpBranchConditional %cond %30 %80

     %30 = OpLabel
     OpBranchConditional %cond2 %99 %80 ; break-if

     %80 = OpLabel ; inner merge
     OpBranch %99

     %99 = OpLabel ; outer merge
     OpReturn
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(FlowClassifyCFGEdges(&fe));

  auto* bi = fe.GetBlockInfo(30);
  ASSERT_NE(bi, nullptr);
  EXPECT_EQ(bi->succ_edge.count(99), 1u);
  EXPECT_EQ(bi->succ_edge[99], EdgeKind::kSwitchBreak);
}

TEST_F(SpvParserTest, ClassifyCFGEdges_SwitchBreak_BypassesMerge_IsError) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %50 None
     OpSwitch %selector %50 20 %20

     %20 = OpLabel
     OpBranch %99 ; invalid exit

     %50 = OpLabel ; switch merge
     OpBranch %99

     %99 = OpLabel
     OpReturn
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_FALSE(FlowClassifyCFGEdges(&fe));
  EXPECT_THAT(
      p->error(),
      Eq("Branch from block 20 to block 99 is an invalid exit from "
         "construct starting at block 10; branch bypasses merge block 50"));
}

TEST_F(SpvParserTest, ClassifyCFGEdges_SwitchBreak_FromNestedLoop_IsError) {
  // It's an error because the break can only go as far as the loop.
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpSwitch %selector %99 20 %20

     %20 = OpLabel
     OpLoopMerge %80 %70 None
     OpBranchConditional %cond %30 %80

     %30 = OpLabel ; in loop construct
     OpBranch %99 ; break

     %70 = OpLabel
     OpBranch %20

     %80 = OpLabel
     OpBranch %99

     %99 = OpLabel ; outer merge
     OpReturn
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_FALSE(FlowClassifyCFGEdges(&fe));
  EXPECT_THAT(
      p->error(),
      Eq("Branch from block 30 to block 99 is an invalid exit from "
         "construct starting at block 20; branch bypasses merge block 80"));
}

TEST_F(SpvParserTest, ClassifyCFGEdges_SwitchBreak_FromNestedSwitch_IsError) {
  // It's an error because the break can only go as far as inner switch
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpSwitch %selector %99 20 %20

     %20 = OpLabel
     OpSelectionMerge %80 None
     OpSwitch %selector %80 30 %30

     %30 = OpLabel
     OpBranch %99 ; break

     %80 = OpLabel
     OpBranch %99

     %99 = OpLabel ; outer merge
     OpReturn
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_FALSE(FlowClassifyCFGEdges(&fe));
  EXPECT_THAT(
      p->error(),
      Eq("Branch from block 30 to block 99 is an invalid exit from "
         "construct starting at block 20; branch bypasses merge block 80"));
}

TEST_F(SpvParserTest, ClassifyCFGEdges_LoopBreak_FromLoopBody) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %50 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpBranchConditional %cond2 %50 %99 ; break-unless

     %50 = OpLabel
     OpBranch %20

     %99 = OpLabel
     OpReturn
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(FlowClassifyCFGEdges(&fe));

  auto* bi = fe.GetBlockInfo(30);
  ASSERT_NE(bi, nullptr);
  EXPECT_EQ(bi->succ_edge.count(99), 1u);
  EXPECT_EQ(bi->succ_edge[99], EdgeKind::kLoopBreak);
}

TEST_F(SpvParserTest, ClassifyCFGEdges_LoopBreak_FromContinueConstructTail) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %50 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpBranch %50

     %50 = OpLabel ; continue target
     OpBranch %60

     %60 = OpLabel ; continue construct tail
     OpBranchConditional %cond2 %20 %99

     %99 = OpLabel
     OpReturn
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(FlowClassifyCFGEdges(&fe));

  auto* bi = fe.GetBlockInfo(60);
  ASSERT_NE(bi, nullptr);
  EXPECT_EQ(bi->succ_edge.count(99), 1u);
  EXPECT_EQ(bi->succ_edge[99], EdgeKind::kLoopBreak);
}

TEST_F(SpvParserTest, ClassifyCFGEdges_LoopBreak_FromLoopBodyDirect) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %50 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpBranch %99  ; unconditional break

     %50 = OpLabel
     OpBranch %20

     %99 = OpLabel
     OpReturn
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(FlowClassifyCFGEdges(&fe));

  auto* bi = fe.GetBlockInfo(30);
  ASSERT_NE(bi, nullptr);
  EXPECT_EQ(bi->succ_edge.count(99), 1u);
  EXPECT_EQ(bi->succ_edge[99], EdgeKind::kLoopBreak);
}

TEST_F(SpvParserTest,
       ClassifyCFGEdges_LoopBreak_FromLoopBodyNestedSelection_Unconditional) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %80 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpSelectionMerge %50 None
     OpBranchConditional %cond2 %40 %50

     %40 = OpLabel
     OpBranch %99 ; deeply nested break

     %50 = OpLabel ; inner merge
     OpBranch %80

     %80 = OpLabel
     OpBranch %20  ; backedge

     %99 = OpLabel
     OpReturn
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(FlowClassifyCFGEdges(&fe));

  auto* bi = fe.GetBlockInfo(40);
  ASSERT_NE(bi, nullptr);
  EXPECT_EQ(bi->succ_edge.count(99), 1u);
  EXPECT_EQ(bi->succ_edge[99], EdgeKind::kLoopBreak);
}

TEST_F(SpvParserTest,
       ClassifyCFGEdges_LoopBreak_FromLoopBodyNestedSelection_Conditional) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %80 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpSelectionMerge %50 None
     OpBranchConditional %cond2 %40 %50

     %40 = OpLabel
     OpBranchConditional %cond3 %99 %50 ; break-if

     %50 = OpLabel ; inner merge
     OpBranch %80

     %80 = OpLabel
     OpBranch %20  ; backedge

     %99 = OpLabel
     OpReturn
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(FlowClassifyCFGEdges(&fe));

  auto* bi = fe.GetBlockInfo(40);
  ASSERT_NE(bi, nullptr);
  EXPECT_EQ(bi->succ_edge.count(99), 1u);
  EXPECT_EQ(bi->succ_edge[99], EdgeKind::kLoopBreak);
}

TEST_F(SpvParserTest,
       ClassifyCFGEdges_LoopBreak_FromContinueConstructNestedFlow_IsError) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %40 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpBranch %40

     %40 = OpLabel ; continue construct
     OpSelectionMerge %79 None
     OpBranchConditional %cond2 %50 %79

     %50 = OpLabel
     OpBranchConditional %cond3 %99 %79 ; attempt to break to 99 should fail

     %79 = OpLabel
     OpBranch %80  ; inner merge

     %80 = OpLabel
     OpBranch %20  ; backedge

     %99 = OpLabel
     OpReturn
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_FALSE(FlowClassifyCFGEdges(&fe));
  EXPECT_THAT(p->error(),
              Eq("Invalid exit (50->99) from continue construct: 50 is not the "
                 "last block in the continue construct starting at 40 "
                 "(violates post-dominance rule)"));
}

TEST_F(SpvParserTest,
       ClassifyCFGEdges_LoopBreak_FromLoopBypassesMerge_IsError) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %50 %40 None
     OpBranchConditional %cond %30 %50

     %30 = OpLabel
     OpBranch %99 ; bad exit

     %40 = OpLabel ; continue construct
     OpBranch %20

     %50 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpReturn
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_FALSE(FlowClassifyCFGEdges(&fe));
  EXPECT_THAT(
      p->error(),
      Eq("Branch from block 30 to block 99 is an invalid exit from "
         "construct starting at block 20; branch bypasses merge block 50"));
}

TEST_F(SpvParserTest,
       ClassifyCFGEdges_LoopBreak_FromContinueBypassesMerge_IsError) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %50 %40 None
     OpBranchConditional %cond %30 %50

     %30 = OpLabel
     OpBranch %40

     %40 = OpLabel ; continue construct
     OpBranch %45

     %45 = OpLabel
     OpBranchConditional %cond2 %20 %99 ; branch to %99 is bad exit

     %50 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpReturn
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_FALSE(FlowClassifyCFGEdges(&fe));
  EXPECT_THAT(
      p->error(),
      Eq("Branch from block 45 to block 99 is an invalid exit from "
         "construct starting at block 40; branch bypasses merge block 50"));
}

TEST_F(SpvParserTest, ClassifyCFGEdges_LoopContinue_LoopBodyToContinue) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %80 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpBranch %80 ; a forward edge

     %80 = OpLabel
     OpBranch %20

     %99 = OpLabel
     OpReturn
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(FlowClassifyCFGEdges(&fe));

  auto* bi = fe.GetBlockInfo(30);
  ASSERT_NE(bi, nullptr);
  EXPECT_EQ(bi->succ_edge.count(80), 1u);
  EXPECT_EQ(bi->succ_edge[80], EdgeKind::kLoopContinue);
}

TEST_F(SpvParserTest, ClassifyCFGEdges_LoopContinue_FromNestedIf) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %80 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpSelectionMerge %79 None
     OpBranchConditional %cond2 %40 %79

     %40 = OpLabel
     OpBranch %80 ; continue

     %79 = OpLabel ; inner merge
     OpBranch %80

     %80 = OpLabel ; continue target
     OpBranch %20

     %99 = OpLabel
     OpReturn
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(FlowClassifyCFGEdges(&fe));

  auto* bi = fe.GetBlockInfo(40);
  ASSERT_NE(bi, nullptr);
  EXPECT_EQ(bi->succ_edge.count(80), 1u);
  EXPECT_EQ(bi->succ_edge[80], EdgeKind::kLoopContinue);
}

TEST_F(SpvParserTest, ClassifyCFGEdges_LoopContinue_ConditionalFromNestedIf) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %80 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpSelectionMerge %79 None
     OpBranchConditional %cond2 %40 %79

     %40 = OpLabel
     OpBranchConditional %cond2 %80 %79 ; continue-if

     %79 = OpLabel ; inner merge
     OpBranch %80

     %80 = OpLabel ; continue target
     OpBranch %20

     %99 = OpLabel
     OpReturn
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(FlowClassifyCFGEdges(&fe));

  auto* bi = fe.GetBlockInfo(40);
  ASSERT_NE(bi, nullptr);
  EXPECT_EQ(bi->succ_edge.count(80), 1u);
  EXPECT_EQ(bi->succ_edge[80], EdgeKind::kLoopContinue);
}

TEST_F(SpvParserTest,
       ClassifyCFGEdges_LoopContinue_FromNestedSwitchCaseBody_Unconditional) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %80 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpSelectionMerge %79 None
     OpSwitch %selector %79 40 %40

     %40 = OpLabel
     OpBranch %80

     %79 = OpLabel ; inner merge
     OpBranch %80

     %80 = OpLabel ; continue target
     OpBranch %20

     %99 = OpLabel
     OpReturn
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(FlowClassifyCFGEdges(&fe)) << p->error();

  auto* bi = fe.GetBlockInfo(40);
  ASSERT_NE(bi, nullptr);
  EXPECT_EQ(bi->succ_edge.count(80), 1u);
  EXPECT_EQ(bi->succ_edge[80], EdgeKind::kLoopContinue);
}

TEST_F(SpvParserTest,
       ClassifyCFGEdges_LoopContinue_FromNestedSwitchCaseDirect_IsError) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %80 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpSelectionMerge %79 None
     OpSwitch %selector %79 40 %80 ; continue here

     %79 = OpLabel ; inner merge
     OpBranch %80

     %80 = OpLabel ; continue target
     OpBranch %20

     %99 = OpLabel
     OpReturn
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  fe.ComputeBlockOrderAndPositions();
  EXPECT_TRUE(fe.VerifyHeaderContinueMergeOrder());
  EXPECT_TRUE(fe.RegisterMerges());
  EXPECT_TRUE(fe.LabelControlFlowConstructs());
  EXPECT_FALSE(fe.FindSwitchCaseHeaders());
  EXPECT_THAT(p->error(), Eq("Switch branch from block 30 to case target block "
                             "80 escapes the selection construct"));
}

TEST_F(SpvParserTest,
       ClassifyCFGEdges_LoopContinue_FromNestedSwitchDefaultDirect_IsError) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %80 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpSelectionMerge %79 None
     OpSwitch %selector %80 40 %79 ; continue here

     %79 = OpLabel ; inner merge
     OpBranch %80

     %80 = OpLabel ; continue target
     OpBranch %20

     %99 = OpLabel
     OpReturn
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  fe.RegisterBasicBlocks();
  fe.ComputeBlockOrderAndPositions();
  EXPECT_TRUE(fe.VerifyHeaderContinueMergeOrder());
  EXPECT_TRUE(fe.RegisterMerges());
  EXPECT_TRUE(fe.LabelControlFlowConstructs());
  EXPECT_FALSE(fe.FindSwitchCaseHeaders());
  EXPECT_THAT(p->error(), Eq("Switch branch from block 30 to default block 80 "
                             "escapes the selection construct"));
}

TEST_F(SpvParserTest,
       ClassifyCFGEdges_LoopContinue_FromNestedSwitchDefaultBody_Conditional) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %80 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpSelectionMerge %79 None
     OpSwitch %selector %40 79 %79

     %40 = OpLabel
     OpBranchConditional %cond2 %80 %79

     %79 = OpLabel ; inner merge
     OpBranch %80

     %80 = OpLabel ; continue target
     OpBranch %20

     %99 = OpLabel
     OpReturn
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(FlowClassifyCFGEdges(&fe)) << p->error();

  auto* bi = fe.GetBlockInfo(40);
  ASSERT_NE(bi, nullptr);
  EXPECT_EQ(bi->succ_edge.count(80), 1u);
  EXPECT_EQ(bi->succ_edge[80], EdgeKind::kLoopContinue);
}

TEST_F(
    SpvParserTest,
    ClassifyCFGEdges_LoopContinue_FromNestedSwitchDefaultBody_Unconditional) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %80 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpSelectionMerge %79 None
     OpSwitch %selector %40 79 %79

     %40 = OpLabel
     OpBranch %80

     %79 = OpLabel ; inner merge
     OpBranch %80

     %80 = OpLabel ; continue target
     OpBranch %20

     %99 = OpLabel
     OpReturn
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(FlowClassifyCFGEdges(&fe));

  auto* bi = fe.GetBlockInfo(40);
  ASSERT_NE(bi, nullptr);
  EXPECT_EQ(bi->succ_edge.count(80), 1u);
  EXPECT_EQ(bi->succ_edge[80], EdgeKind::kLoopContinue);
}

TEST_F(SpvParserTest, ClassifyCFGEdges_Fallthrough_CaseTailToCase) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpSwitch %selector %99 20 %20 40 %40

     %20 = OpLabel ; case 20
     OpBranch %30

     %30 = OpLabel
     OpBranch %40 ; fallthrough

     %40 = OpLabel ; case 40
     OpBranch %99

     %99 = OpLabel
     OpReturn
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(FlowClassifyCFGEdges(&fe));

  auto* bi = fe.GetBlockInfo(30);
  ASSERT_NE(bi, nullptr);
  EXPECT_EQ(bi->succ_edge.count(40), 1u);
  EXPECT_EQ(bi->succ_edge[40], EdgeKind::kCaseFallThrough);
}

TEST_F(SpvParserTest, ClassifyCFGEdges_Fallthrough_CaseTailToDefaultNotMerge) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpSwitch %selector %40 20 %20

     %20 = OpLabel ; case 20
     OpBranch %30

     %30 = OpLabel
     OpBranch %40 ; fallthrough

     %40 = OpLabel ; case 40
     OpBranch %99

     %99 = OpLabel
     OpReturn
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(FlowClassifyCFGEdges(&fe));

  auto* bi = fe.GetBlockInfo(30);
  ASSERT_NE(bi, nullptr);
  EXPECT_EQ(bi->succ_edge.count(40), 1u);
  EXPECT_EQ(bi->succ_edge[40], EdgeKind::kCaseFallThrough);
}

TEST_F(SpvParserTest, ClassifyCFGEdges_Fallthrough_DefaultToCase) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpSwitch %selector %20 40 %40

     %20 = OpLabel ; default
     OpBranch %30

     %30 = OpLabel
     OpBranch %40 ; fallthrough

     %40 = OpLabel ; case 40
     OpBranch %99

     %99 = OpLabel
     OpReturn
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(FlowClassifyCFGEdges(&fe));

  auto* bi = fe.GetBlockInfo(30);
  ASSERT_NE(bi, nullptr);
  EXPECT_EQ(bi->succ_edge.count(40), 1u);
  EXPECT_EQ(bi->succ_edge[40], EdgeKind::kCaseFallThrough);
}

TEST_F(SpvParserTest,
       ClassifyCFGEdges_Fallthrough_CaseNonTailToCase_TrueBranch) {
  // This is an unusual one, and is an error. Structurally it looks like this:
  //   switch (val) {
  //   case 0: {
  //        if (cond) {
  //          fallthrough;
  //        }
  //        something = 1;
  //      }
  //   case 1: { }
  //   }
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpSwitch %selector %99 20 %20 50 %50

     %20 = OpLabel
     OpSelectionMerge %49 None
     OpBranchConditional %cond %30 %49

     %30 = OpLabel
     OpBranch %50 ; attempt to fallthrough

     %49 = OpLabel
     OpBranch %99

     %50 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpReturn
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_FALSE(FlowClassifyCFGEdges(&fe));
  EXPECT_THAT(
      p->error(),
      Eq("Branch from 10 to 50 bypasses header 20 (dominance rule violated)"));
}

TEST_F(SpvParserTest,
       ClassifyCFGEdges_Fallthrough_CaseNonTailToCase_FalseBranch) {
  // Like previous test, but taking the false branch.

  // This is an unusual one, and is an error. Structurally it looks like this:
  //   switch (val) {
  //   case 0: {
  //        if (cond) {
  //          fallthrough;
  //        }
  //        something = 1;
  //      }
  //   case 1: { }
  //   }
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpSwitch %selector %99 20 %20 50 %50

     %20 = OpLabel
     OpSelectionMerge %49 None
     OpBranchConditional %cond %49 %30 ;; this is the difference

     %30 = OpLabel
     OpBranch %50 ; attempt to fallthrough

     %49 = OpLabel
     OpBranch %99

     %50 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpReturn
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_FALSE(FlowClassifyCFGEdges(&fe));
  EXPECT_THAT(
      p->error(),
      Eq("Branch from 10 to 50 bypasses header 20 (dominance rule violated)"));
}

TEST_F(SpvParserTest, ClassifyCFGEdges_Forward_IfToThen) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpBranchConditional %cond %20 %99

     %20 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpReturn
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(FlowClassifyCFGEdges(&fe));

  auto* bi = fe.GetBlockInfo(10);
  ASSERT_NE(bi, nullptr);
  EXPECT_EQ(bi->succ_edge.count(20), 1u);
  EXPECT_EQ(bi->succ_edge[20], EdgeKind::kForward);
}

TEST_F(SpvParserTest, ClassifyCFGEdges_Forward_IfToElse) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpBranchConditional %cond %99 %30

     %30 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpReturn
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(FlowClassifyCFGEdges(&fe));

  auto* bi = fe.GetBlockInfo(10);
  ASSERT_NE(bi, nullptr);
  EXPECT_EQ(bi->succ_edge.count(30), 1u);
  EXPECT_EQ(bi->succ_edge[30], EdgeKind::kForward);
}

TEST_F(SpvParserTest, ClassifyCFGEdges_Forward_SwitchToCase) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpSwitch %selector %99 20 %20

     %20 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpReturn
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(FlowClassifyCFGEdges(&fe));

  auto* bi = fe.GetBlockInfo(10);
  ASSERT_NE(bi, nullptr);
  EXPECT_EQ(bi->succ_edge.count(20), 1u);
  EXPECT_EQ(bi->succ_edge[20], EdgeKind::kForward);
}

TEST_F(SpvParserTest, ClassifyCFGEdges_Forward_SwitchToDefaultNotMerge) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpSwitch %selector %30 20 %20

     %20 = OpLabel
     OpBranch %99

     %30 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpReturn
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(FlowClassifyCFGEdges(&fe));

  auto* bi = fe.GetBlockInfo(10);
  ASSERT_NE(bi, nullptr);
  EXPECT_EQ(bi->succ_edge.count(30), 1u);
  EXPECT_EQ(bi->succ_edge[30], EdgeKind::kForward);
}

TEST_F(SpvParserTest, ClassifyCFGEdges_Forward_LoopHeadToBody) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %80 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpBranch %80

     %80 = OpLabel
     OpBranch %20

     %99 = OpLabel
     OpReturn
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(FlowClassifyCFGEdges(&fe));

  auto* bi = fe.GetBlockInfo(20);
  ASSERT_NE(bi, nullptr);
  EXPECT_EQ(bi->succ_edge.count(30), 1u);
  EXPECT_EQ(bi->succ_edge[30], EdgeKind::kForward);
}

TEST_F(SpvParserTest,
       ClassifyCFGEdges_DomViolation_BeforeIfToSelectionInterior) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpBranchConditional %cond %20 %50 ;%50 is a bad branch

     %20 = OpLabel
     OpSelectionMerge %89 None
     OpBranchConditional %cond %50 %89

     %50 = OpLabel
     OpBranch %89

     %89 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpReturn
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_FALSE(FlowClassifyCFGEdges(&fe));
  EXPECT_THAT(
      p->error(),
      Eq("Branch from 10 to 50 bypasses header 20 (dominance rule violated)"));
}

TEST_F(SpvParserTest,
       ClassifyCFGEdges_DomViolation_BeforeSwitchToSelectionInterior) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpBranchConditional %cond %20 %50 ;%50 is a bad branch

     %20 = OpLabel
     OpSelectionMerge %89 None
     OpSwitch %selector %89 50 %50

     %50 = OpLabel
     OpBranch %89

     %89 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpReturn
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_FALSE(FlowClassifyCFGEdges(&fe));
  EXPECT_THAT(
      p->error(),
      Eq("Branch from 10 to 50 bypasses header 20 (dominance rule violated)"));
}

TEST_F(SpvParserTest,
       ClassifyCFGEdges_DomViolation_BeforeLoopToLoopBodyInterior) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpBranchConditional %cond %20 %50 ;%50 is a bad branch

     %20 = OpLabel
     OpLoopMerge %89 %80 None
     OpBranchConditional %cond %50 %89

     %50 = OpLabel
     OpBranch %89

     %80 = OpLabel
     OpBranch %20

     %89 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpReturn
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_FALSE(FlowClassifyCFGEdges(&fe));
  EXPECT_THAT(p->error(),
              // Weird error, but still we caught it.
              // Preferred: Eq("Branch from 10 to 50 bypasses header 20
              // (dominance rule violated)"))
              Eq("Branch from 10 to 50 bypasses continue target 80 (dominance "
                 "rule violated)"))
      << Dump(fe.block_order());
}

TEST_F(SpvParserTest,
       ClassifyCFGEdges_DomViolation_BeforeContinueToContinueInterior) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %50 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpBranch %60

     %50 = OpLabel ; continue target
     OpBranch %60

     %60 = OpLabel
     OpBranch %20

     %89 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpReturn
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_FALSE(FlowClassifyCFGEdges(&fe));
  EXPECT_THAT(
      p->error(),
      Eq("Branch from block 30 to block 60 is an invalid exit from "
         "construct starting at block 20; branch bypasses continue target 50"));
}

TEST_F(SpvParserTest,
       ClassifyCFGEdges_DomViolation_AfterContinueToContinueInterior) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %80 %50 None
     OpBranchConditional %cond %30 %80

     %30 = OpLabel
     OpBranch %50

     %50 = OpLabel
     OpBranch %60

     %60 = OpLabel
     OpBranch %20

     %80 = OpLabel
     OpBranch %60 ; bad branch
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_FALSE(FlowClassifyCFGEdges(&fe));
  EXPECT_THAT(
      p->error(),
      Eq("Branch from block 50 to block 60 is an invalid exit from "
         "construct starting at block 50; branch bypasses merge block 80"));
}

TEST_F(SpvParserTest, ClassifyCFGEdges_TooManyBackedges) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %50 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpBranchConditional %cond2 %20 %50

     %50 = OpLabel
     OpBranch %20

     %99 = OpLabel
     OpReturn
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_FALSE(FlowClassifyCFGEdges(&fe));
  EXPECT_THAT(
      p->error(),
      Eq("Invalid backedge (30->20): 30 is not in a continue construct"));
}

TEST_F(SpvParserTest, ClassifyCFGEdges_NeededMerge_BranchConditional) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %20 = OpLabel
     OpBranchConditional %cond %30 %40

     %30 = OpLabel
     OpBranch %99

     %40 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpReturn
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_FALSE(FlowClassifyCFGEdges(&fe));
  EXPECT_THAT(p->error(),
              Eq("Control flow diverges at block 20 (to 30, 40) but it is not "
                 "a structured header (it has no merge instruction)"));
}

TEST_F(SpvParserTest, ClassifyCFGEdges_NeededMerge_Switch) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSwitch %selector %99 20 %20 30 %30

     %20 = OpLabel
     OpBranch %99

     %30 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpReturn
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_FALSE(FlowClassifyCFGEdges(&fe));
  EXPECT_THAT(p->error(),
              Eq("Control flow diverges at block 10 (to 99, 20) but it is not "
                 "a structured header (it has no merge instruction)"));
}

TEST_F(SpvParserTest, ClassifyCFGEdges_Pathological_Forward_LoopHeadSplitBody) {
  // In this case the branch-conditional in the loop header is really also a
  // selection header.
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpBranch %20

     %20 = OpLabel
     OpLoopMerge %99 %80 None
     OpBranchConditional %cond %30 %50 ; what to make of this?

     %30 = OpLabel
     OpBranch %99

     %50 = OpLabel
     OpBranch %99

     %80 = OpLabel
     OpBranch %20

     %99 = OpLabel
     OpReturn
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(FlowClassifyCFGEdges(&fe));

  auto* bi = fe.GetBlockInfo(20);
  ASSERT_NE(bi, nullptr);
  EXPECT_EQ(bi->succ_edge.count(30), 1u);
  EXPECT_EQ(bi->succ_edge[30], EdgeKind::kForward);
  EXPECT_EQ(bi->succ_edge.count(50), 1u);
  EXPECT_EQ(bi->succ_edge[50], EdgeKind::kForward);
}

TEST_F(SpvParserTest, ClassifyCFGEdges_Pathological_Forward_Premerge) {
  // Two arms of an if-selection converge early, before the merge block
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpBranchConditional %cond %20 %30

     %20 = OpLabel
     OpBranch %50

     %30 = OpLabel
     OpBranch %50

     %50 = OpLabel ; this is an early merge!
     OpBranch %60

     %60 = OpLabel ; still early merge
     OpBranch %99

     %99 = OpLabel
     OpReturn
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(FlowClassifyCFGEdges(&fe));

  auto* bi20 = fe.GetBlockInfo(20);
  ASSERT_NE(bi20, nullptr);
  EXPECT_EQ(bi20->succ_edge.count(50), 1u);
  EXPECT_EQ(bi20->succ_edge[50], EdgeKind::kForward);

  auto* bi30 = fe.GetBlockInfo(30);
  ASSERT_NE(bi30, nullptr);
  EXPECT_EQ(bi30->succ_edge.count(50), 1u);
  EXPECT_EQ(bi30->succ_edge[50], EdgeKind::kForward);

  auto* bi50 = fe.GetBlockInfo(50);
  ASSERT_NE(bi50, nullptr);
  EXPECT_EQ(bi50->succ_edge.count(60), 1u);
  EXPECT_EQ(bi50->succ_edge[60], EdgeKind::kForward);

  auto* bi60 = fe.GetBlockInfo(60);
  ASSERT_NE(bi60, nullptr);
  EXPECT_EQ(bi60->succ_edge.count(99), 1u);
  EXPECT_EQ(bi60->succ_edge[99], EdgeKind::kIfBreak);
}

TEST_F(SpvParserTest, ClassifyCFGEdges_Pathological_Forward_Regardless) {
  // Both arms of an OpBranchConditional go to the same target.
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpBranchConditional %cond %20 %20 ; same target!

     %20 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpReturn
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(FlowClassifyCFGEdges(&fe));

  auto* bi10 = fe.GetBlockInfo(10);
  ASSERT_NE(bi10, nullptr);
  EXPECT_EQ(bi10->succ_edge.count(20), 1u);
  EXPECT_EQ(bi10->succ_edge[20], EdgeKind::kForward);

  auto* bi20 = fe.GetBlockInfo(20);
  ASSERT_NE(bi20, nullptr);
  EXPECT_EQ(bi20->succ_edge.count(99), 1u);
  EXPECT_EQ(bi20->succ_edge[99], EdgeKind::kIfBreak);
}

TEST_F(SpvParserTest, FindIfSelectionInternalHeaders_NoIf) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpReturn
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(FlowClassifyCFGEdges(&fe));

  auto* bi = fe.GetBlockInfo(10);
  ASSERT_NE(bi, nullptr);
  EXPECT_EQ(bi->true_head_for, nullptr);
  EXPECT_EQ(bi->false_head_for, nullptr);
  EXPECT_EQ(bi->premerge_head_for, nullptr);
  EXPECT_EQ(bi->premerge_head_for, nullptr);
  EXPECT_EQ(bi->exclusive_false_head_for, nullptr);
}

TEST_F(SpvParserTest, FindIfSelectionInternalHeaders_ThenElse) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpBranchConditional %cond %20 %30

     %20 = OpLabel
     OpBranch %99

     %30 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpReturn
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(FlowFindIfSelectionInternalHeaders(&fe));

  auto* bi20 = fe.GetBlockInfo(20);
  ASSERT_NE(bi20, nullptr);
  ASSERT_NE(bi20->true_head_for, nullptr);
  EXPECT_EQ(bi20->true_head_for->begin_id, 10u);
  EXPECT_EQ(bi20->false_head_for, nullptr);
  EXPECT_EQ(bi20->premerge_head_for, nullptr);
  EXPECT_EQ(bi20->exclusive_false_head_for, nullptr);

  auto* bi30 = fe.GetBlockInfo(30);
  ASSERT_NE(bi30, nullptr);
  EXPECT_EQ(bi30->true_head_for, nullptr);
  ASSERT_NE(bi30->false_head_for, nullptr);
  EXPECT_EQ(bi30->false_head_for->begin_id, 10u);
  EXPECT_EQ(bi30->premerge_head_for, nullptr);
  EXPECT_EQ(bi30->exclusive_false_head_for, nullptr);
}

TEST_F(SpvParserTest, FindIfSelectionInternalHeaders_IfOnly) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpBranchConditional %cond %30 %99

     %30 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpReturn
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(FlowFindIfSelectionInternalHeaders(&fe));

  auto* bi30 = fe.GetBlockInfo(30);
  ASSERT_NE(bi30, nullptr);
  ASSERT_NE(bi30->true_head_for, nullptr);
  EXPECT_EQ(bi30->true_head_for->begin_id, 10u);
  EXPECT_EQ(bi30->false_head_for, nullptr);
  EXPECT_EQ(bi30->premerge_head_for, nullptr);
  EXPECT_EQ(bi30->exclusive_false_head_for, nullptr);
}

TEST_F(SpvParserTest, FindIfSelectionInternalHeaders_ElseOnly) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpBranchConditional %cond %99 %30

     %30 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpReturn
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(FlowFindIfSelectionInternalHeaders(&fe));

  auto* bi30 = fe.GetBlockInfo(30);
  ASSERT_NE(bi30, nullptr);
  EXPECT_EQ(bi30->true_head_for, nullptr);
  ASSERT_NE(bi30->false_head_for, nullptr);
  EXPECT_EQ(bi30->false_head_for->begin_id, 10u);
  EXPECT_EQ(bi30->premerge_head_for, nullptr);
  ASSERT_NE(bi30->exclusive_false_head_for, nullptr);
  EXPECT_EQ(bi30->exclusive_false_head_for->begin_id, 10u);
}

TEST_F(SpvParserTest, FindIfSelectionInternalHeaders_Regardless) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpBranchConditional %cond %20 %20 ; same target

     %20 = OpLabel
     OpBranch %80

     %80 = OpLabel
     OpBranch %99

     %99 = OpLabel
     OpReturn
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(FlowFindIfSelectionInternalHeaders(&fe));

  EXPECT_THAT(fe.block_order(), ElementsAre(10, 20, 80, 99));

  auto* bi20 = fe.GetBlockInfo(20);
  ASSERT_NE(bi20, nullptr);
  ASSERT_NE(bi20->true_head_for, nullptr);
  EXPECT_EQ(bi20->true_head_for->begin_id, 10u);
  ASSERT_NE(bi20->false_head_for, nullptr);
  EXPECT_EQ(bi20->false_head_for->begin_id, 10u);
  EXPECT_EQ(bi20->premerge_head_for, nullptr);
  EXPECT_EQ(bi20->exclusive_false_head_for, nullptr);

  auto* bi80 = fe.GetBlockInfo(80);
  ASSERT_NE(bi80, nullptr);
  EXPECT_EQ(bi80->true_head_for, nullptr);
  EXPECT_EQ(bi80->false_head_for, nullptr);
  EXPECT_EQ(bi80->premerge_head_for, nullptr);
  EXPECT_EQ(bi80->exclusive_false_head_for, nullptr);
}

TEST_F(SpvParserTest, FindIfSelectionInternalHeaders_Premerge_Simple) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpBranchConditional %cond %20 %30

     %20 = OpLabel
     OpBranch %80

     %30 = OpLabel
     OpBranch %80

     %80 = OpLabel ; premerge node
     OpBranch %99

     %99 = OpLabel
     OpReturn
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(FlowFindIfSelectionInternalHeaders(&fe));

  EXPECT_THAT(fe.block_order(), ElementsAre(10, 20, 30, 80, 99));

  auto* bi80 = fe.GetBlockInfo(80);
  ASSERT_NE(bi80, nullptr);
  EXPECT_EQ(bi80->true_head_for, nullptr);
  EXPECT_EQ(bi80->false_head_for, nullptr);
  ASSERT_NE(bi80->premerge_head_for, nullptr);
  EXPECT_EQ(bi80->premerge_head_for->begin_id, 10u);
  EXPECT_EQ(bi80->exclusive_false_head_for, nullptr);
}

TEST_F(SpvParserTest,
       FindIfSelectionInternalHeaders_Premerge_ThenDirectToElse) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpBranchConditional %cond %20 %30

     %20 = OpLabel
     OpBranch %30

     %30 = OpLabel
     OpBranch %80

     %80 = OpLabel ; premerge node
     OpBranch %99

     %99 = OpLabel
     OpReturn
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(FlowFindIfSelectionInternalHeaders(&fe));

  EXPECT_THAT(fe.block_order(), ElementsAre(10, 20, 30, 80, 99));

  auto* bi20 = fe.GetBlockInfo(20);
  ASSERT_NE(bi20, nullptr);
  ASSERT_NE(bi20->true_head_for, nullptr);
  EXPECT_EQ(bi20->true_head_for->begin_id, 10u);
  EXPECT_EQ(bi20->false_head_for, nullptr);
  EXPECT_EQ(bi20->premerge_head_for, nullptr);
  EXPECT_EQ(bi20->exclusive_false_head_for, nullptr);

  auto* bi30 = fe.GetBlockInfo(30);
  ASSERT_NE(bi30, nullptr);
  EXPECT_EQ(bi30->true_head_for, nullptr);
  ASSERT_NE(bi30->false_head_for, nullptr);
  EXPECT_EQ(bi30->false_head_for->begin_id, 10u);
  ASSERT_NE(bi30->premerge_head_for, nullptr);
  EXPECT_EQ(bi30->premerge_head_for->begin_id, 10u);
  EXPECT_EQ(bi30->exclusive_false_head_for, nullptr);

  auto* bi80 = fe.GetBlockInfo(80);
  ASSERT_NE(bi80, nullptr);
  EXPECT_EQ(bi80->true_head_for, nullptr);
  EXPECT_EQ(bi80->false_head_for, nullptr);
  EXPECT_EQ(bi80->premerge_head_for, nullptr);
  EXPECT_EQ(bi80->exclusive_false_head_for, nullptr);
}

TEST_F(SpvParserTest,
       FindIfSelectionInternalHeaders_Premerge_ElseDirectToThen) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpBranchConditional %cond %20 %30

     %20 = OpLabel
     OpBranch %80 ; branches to premerge

     %30 = OpLabel ; else
     OpBranch %20  ; branches to then

     %80 = OpLabel ; premerge node
     OpBranch %99

     %99 = OpLabel
     OpReturn
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(FlowFindIfSelectionInternalHeaders(&fe));

  EXPECT_THAT(fe.block_order(), ElementsAre(10, 30, 20, 80, 99));

  auto* bi20 = fe.GetBlockInfo(20);
  ASSERT_NE(bi20, nullptr);
  ASSERT_NE(bi20->true_head_for, nullptr);
  EXPECT_EQ(bi20->true_head_for->begin_id, 10u);
  EXPECT_EQ(bi20->false_head_for, nullptr);
  ASSERT_NE(bi20->premerge_head_for, nullptr);
  EXPECT_EQ(bi20->premerge_head_for->begin_id, 10u);
  EXPECT_EQ(bi20->exclusive_false_head_for, nullptr);

  auto* bi30 = fe.GetBlockInfo(30);
  ASSERT_NE(bi30, nullptr);
  EXPECT_EQ(bi30->true_head_for, nullptr);
  ASSERT_NE(bi30->false_head_for, nullptr);
  EXPECT_EQ(bi30->false_head_for->begin_id, 10u);
  EXPECT_EQ(bi30->premerge_head_for, nullptr);
  EXPECT_EQ(bi30->exclusive_false_head_for, nullptr);

  auto* bi80 = fe.GetBlockInfo(80);
  ASSERT_NE(bi80, nullptr);
  EXPECT_EQ(bi80->true_head_for, nullptr);
  EXPECT_EQ(bi80->false_head_for, nullptr);
  EXPECT_EQ(bi80->premerge_head_for, nullptr);
  EXPECT_EQ(bi80->exclusive_false_head_for, nullptr);
}

TEST_F(SpvParserTest,
       FindIfSelectionInternalHeaders_Premerge_MultiCandidate_IsError) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpBranchConditional %cond %20 %30

     %20 = OpLabel
     ; Try to force several branches down into "else" territory,
     ; but we error out earlier in the flow due to lack of merge
     ; instruction.
     OpBranchConditional %cond2  %70 %80

     %30 = OpLabel
     OpBranch %70

     %70 = OpLabel ; candidate premerge
     OpBranch %80

     %80 = OpLabel ; canddiate premerge
     OpBranch %99

     %99 = OpLabel
     OpReturn
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  // Error out sooner in the flow
  EXPECT_FALSE(FlowClassifyCFGEdges(&fe));
  EXPECT_THAT(p->error(),
              Eq("Control flow diverges at block 20 (to 70, 80) but it is not "
                 "a structured header (it has no merge instruction)"));
}

TEST_F(SpvParserTest,
       FindIfSelectionInternalHeaders_IfBreak_FromThen_ForwardWithinThen) {
  // TODO(dneto): We can make this case work, if we injected
  //    if (!cond2) { rest-of-then-body }
  // at block 30
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpBranchConditional %cond %20 %99

     %20 = OpLabel
     OpBranchConditional %cond2 %99 %80 ; break with forward edge

     %80 = OpLabel ; still in then clause
     OpBranch %99

     %99 = OpLabel
     OpReturn
     OpFunctionEnd
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(FlowFindIfSelectionInternalHeaders(&fe));
  EXPECT_THAT(fe.block_order(), ElementsAre(10, 20, 80, 99));

  auto* bi20 = fe.GetBlockInfo(20);
  ASSERT_NE(bi20, nullptr);
  ASSERT_NE(bi20->true_head_for, nullptr);
  EXPECT_EQ(bi20->true_head_for->begin_id, 10u);
  EXPECT_EQ(bi20->false_head_for, nullptr);
  EXPECT_EQ(bi20->premerge_head_for, nullptr);
  EXPECT_EQ(bi20->exclusive_false_head_for, nullptr);
  EXPECT_EQ(bi20->succ_edge.count(80), 1u);
  EXPECT_EQ(bi20->succ_edge[80], EdgeKind::kForward);
  EXPECT_EQ(bi20->succ_edge.count(99), 1u);
  EXPECT_EQ(bi20->succ_edge[99], EdgeKind::kIfBreak);

  auto* bi80 = fe.GetBlockInfo(80);
  ASSERT_NE(bi80, nullptr);
  EXPECT_EQ(bi80->true_head_for, nullptr);
  EXPECT_EQ(bi80->false_head_for, nullptr);
  EXPECT_EQ(bi80->premerge_head_for, nullptr);
  EXPECT_EQ(bi80->exclusive_false_head_for, nullptr);
  EXPECT_EQ(bi80->succ_edge.count(99), 1u);
  EXPECT_EQ(bi80->succ_edge[99], EdgeKind::kIfBreak);
}

TEST_F(SpvParserTest,
       FindIfSelectionInternalHeaders_IfBreak_FromElse_ForwardWithinElse) {
  // TODO(dneto): We can make this case work, if we injected
  //    if (!cond2) { rest-of-else-body }
  // at block 30
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpBranchConditional %cond %20 %30

     %20 = OpLabel
     OpBranch %99

     %30 = OpLabel ; else clause
     OpBranchConditional %cond2 %99 %80 ; break with forward edge

     %80 = OpLabel ; still in then clause
     OpBranch %99

     %99 = OpLabel
     OpReturn
     OpFunctionEnd
)";
  auto* p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_TRUE(FlowFindIfSelectionInternalHeaders(&fe));
  EXPECT_THAT(fe.block_order(), ElementsAre(10, 20, 30, 80, 99));

  auto* bi30 = fe.GetBlockInfo(30);
  ASSERT_NE(bi30, nullptr);
  EXPECT_EQ(bi30->true_head_for, nullptr);
  ASSERT_NE(bi30->false_head_for, nullptr);
  EXPECT_EQ(bi30->false_head_for->begin_id, 10u);
  EXPECT_EQ(bi30->premerge_head_for, nullptr);
  EXPECT_EQ(bi30->exclusive_false_head_for, nullptr);
  EXPECT_EQ(bi30->succ_edge.count(80), 1u);
  EXPECT_EQ(bi30->succ_edge[80], EdgeKind::kForward);
  EXPECT_EQ(bi30->succ_edge.count(99), 1u);
  EXPECT_EQ(bi30->succ_edge[99], EdgeKind::kIfBreak);

  auto* bi80 = fe.GetBlockInfo(80);
  ASSERT_NE(bi80, nullptr);
  EXPECT_EQ(bi80->true_head_for, nullptr);
  EXPECT_EQ(bi80->false_head_for, nullptr);
  EXPECT_EQ(bi80->premerge_head_for, nullptr);
  EXPECT_EQ(bi80->exclusive_false_head_for, nullptr);
  EXPECT_EQ(bi80->succ_edge.count(99), 1u);
  EXPECT_EQ(bi80->succ_edge[99], EdgeKind::kIfBreak);
}

TEST_F(SpvParserTest,
       FindIfSelectionInternalHeaders_IfBreak_WithForwardToPremerge_IsError) {
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpBranchConditional %cond %20 %30

     %20 = OpLabel ; then
     OpBranchConditional %cond2 %99 %80 ; break with forward to premerge is error

     %30 = OpLabel ; else
     OpBranch %80

     %80 = OpLabel ; premerge node
     OpBranch %99

     %99 = OpLabel
     OpReturn
     OpFunctionEnd
)";
  auto* p = parser(test::Assemble(assembly));
  std::cout << assembly;
  ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
  FunctionEmitter fe(p, *spirv_function(100));
  EXPECT_FALSE(FlowFindIfSelectionInternalHeaders(&fe));
  EXPECT_THAT(fe.block_order(), ElementsAre(10, 20, 30, 80, 99));
  EXPECT_THAT(
      p->error(),
      Eq("Block 20 in if-selection headed at block 10 branches to both the "
         "merge block 99 and also to block 80 later in the selection"));
}

TEST_F(SpvParserTest, DISABLED_Codegen_IfBreak_FromThen_ForwardWithinThen) {
  // TODO(dneto): We can make this case work, if we injected
  //    if (!cond2) { rest-of-then-body }
  // at block 30
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpBranchConditional %cond %20 %99

     %20 = OpLabel
     OpBranchConditional %cond2 %99 %80 ; break with forward edge

     %80 = OpLabel ; still in then clause
     OpBranch %99

     %99 = OpLabel
     OpReturn
     OpFunctionEnd
)";
}

TEST_F(SpvParserTest, DISABLED_Codegen_IfBreak_FromElse_ForwardWithinElse) {
  // TODO(dneto): We can make this case work, if we injected
  //    if (!cond2) { rest-of-else-body }
  // at block 30
  auto assembly = CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn

     %10 = OpLabel
     OpSelectionMerge %99 None
     OpBranchConditional %cond %20 %30

     %20 = OpLabel
     OpBranch %99

     %30 = OpLabel ; else clause
     OpBranchConditional %cond2 %99 %80 ; break with forward edge

     %80 = OpLabel ; still in then clause
     OpBranch %99

     %99 = OpLabel
     OpReturn
     OpFunctionEnd
)";
}

}  // namespace
}  // namespace spirv
}  // namespace reader
}  // namespace tint
