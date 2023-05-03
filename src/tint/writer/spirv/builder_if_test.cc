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

#include "gmock/gmock.h"
#include "src/tint/writer/spirv/spv_dump.h"
#include "src/tint/writer/spirv/test_helper.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::writer::spirv {
namespace {

using BuilderTest = TestHelper;

TEST_F(BuilderTest, If_Empty) {
    // if (true) {
    // }
    auto* expr = If(true, Block());
    WrapInFunction(expr);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();

    EXPECT_TRUE(b.GenerateIfStatement(expr)) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%1 = OpTypeBool
%2 = OpConstantTrue %1
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(OpSelectionMerge %3 None
OpBranchConditional %2 %4 %3
%4 = OpLabel
OpBranch %3
%3 = OpLabel
)");
}

TEST_F(BuilderTest, If_Empty_OutsideFunction_IsError) {
    // Outside a function.
    // if (true) {
    // }

    auto* block = Block();
    auto* expr = If(true, block);
    WrapInFunction(expr);

    spirv::Builder& b = Build();

    tint::SetInternalCompilerErrorReporter(nullptr);

    EXPECT_FALSE(b.GenerateIfStatement(expr)) << b.Diagnostics();
    EXPECT_TRUE(b.has_error());
    EXPECT_THAT(b.Diagnostics().str(),
                ::testing::HasSubstr(
                    "Internal error: trying to add SPIR-V instruction 247 outside a function"));
}

TEST_F(BuilderTest, If_WithStatements) {
    // if (true) {
    //   v = 2;
    // }

    auto* var = GlobalVar("v", ty.i32(), builtin::AddressSpace::kPrivate);
    auto* body = Block(Assign("v", 2_i));
    auto* expr = If(true, body);
    WrapInFunction(expr);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.Diagnostics();

    EXPECT_TRUE(b.GenerateIfStatement(expr)) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%3 = OpTypeInt 32 1
%2 = OpTypePointer Private %3
%4 = OpConstantNull %3
%1 = OpVariable %2 Private %4
%5 = OpTypeBool
%6 = OpConstantTrue %5
%9 = OpConstant %3 2
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(OpSelectionMerge %7 None
OpBranchConditional %6 %8 %7
%8 = OpLabel
OpStore %1 %9
OpBranch %7
%7 = OpLabel
)");
}

TEST_F(BuilderTest, If_WithElse) {
    // if (true) {
    //   v = 2i;
    // } else {
    //   v = 3i;
    // }

    auto* var = GlobalVar("v", ty.i32(), builtin::AddressSpace::kPrivate);
    auto* body = Block(Assign("v", 2_i));
    auto* else_body = Block(Assign("v", 3_i));

    auto* expr = If(true, body, Else(else_body));
    WrapInFunction(expr);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.Diagnostics();

    EXPECT_TRUE(b.GenerateIfStatement(expr)) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%3 = OpTypeInt 32 1
%2 = OpTypePointer Private %3
%4 = OpConstantNull %3
%1 = OpVariable %2 Private %4
%5 = OpTypeBool
%6 = OpConstantTrue %5
%10 = OpConstant %3 2
%11 = OpConstant %3 3
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(OpSelectionMerge %7 None
OpBranchConditional %6 %8 %9
%8 = OpLabel
OpStore %1 %10
OpBranch %7
%9 = OpLabel
OpStore %1 %11
OpBranch %7
%7 = OpLabel
)");
}

TEST_F(BuilderTest, If_WithElseIf) {
    // if (true) {
    //   v = 2i;
    // } else if (true) {
    //   v = 3i;
    // }

    auto* var = GlobalVar("v", ty.i32(), builtin::AddressSpace::kPrivate);
    auto* body = Block(Assign("v", 2_i));
    auto* else_body = Block(Assign("v", 3_i));

    auto* expr = If(true, body, Else(If(true, else_body)));
    WrapInFunction(expr);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.Diagnostics();

    EXPECT_TRUE(b.GenerateIfStatement(expr)) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%3 = OpTypeInt 32 1
%2 = OpTypePointer Private %3
%4 = OpConstantNull %3
%1 = OpVariable %2 Private %4
%5 = OpTypeBool
%6 = OpConstantTrue %5
%10 = OpConstant %3 2
%13 = OpConstant %3 3
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(OpSelectionMerge %7 None
OpBranchConditional %6 %8 %9
%8 = OpLabel
OpStore %1 %10
OpBranch %7
%9 = OpLabel
OpSelectionMerge %11 None
OpBranchConditional %6 %12 %11
%12 = OpLabel
OpStore %1 %13
OpBranch %11
%11 = OpLabel
OpBranch %7
%7 = OpLabel
)");
}

TEST_F(BuilderTest, If_WithMultiple) {
    // if (true) {
    //   v = 2i;
    // } else if (true) {
    //   v = 3i;
    // } else if (false) {
    //   v = 4i;
    // } else {
    //   v = 5i;
    // }

    auto* var = GlobalVar("v", ty.i32(), builtin::AddressSpace::kPrivate);
    auto* body = Block(Assign("v", 2_i));
    auto* elseif_1_body = Block(Assign("v", 3_i));
    auto* elseif_2_body = Block(Assign("v", 4_i));
    auto* else_body = Block(Assign("v", 5_i));

    auto* expr = If(true, body,                            //
                    Else(If(true, elseif_1_body,           //
                            Else(If(false, elseif_2_body,  //
                                    Else(else_body))))));
    WrapInFunction(expr);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();
    ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.Diagnostics();

    EXPECT_TRUE(b.GenerateIfStatement(expr)) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%3 = OpTypeInt 32 1
%2 = OpTypePointer Private %3
%4 = OpConstantNull %3
%1 = OpVariable %2 Private %4
%5 = OpTypeBool
%6 = OpConstantTrue %5
%10 = OpConstant %3 2
%14 = OpConstant %3 3
%15 = OpConstantNull %5
%19 = OpConstant %3 4
%20 = OpConstant %3 5
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(OpSelectionMerge %7 None
OpBranchConditional %6 %8 %9
%8 = OpLabel
OpStore %1 %10
OpBranch %7
%9 = OpLabel
OpSelectionMerge %11 None
OpBranchConditional %6 %12 %13
%12 = OpLabel
OpStore %1 %14
OpBranch %11
%13 = OpLabel
OpSelectionMerge %16 None
OpBranchConditional %15 %17 %18
%17 = OpLabel
OpStore %1 %19
OpBranch %16
%18 = OpLabel
OpStore %1 %20
OpBranch %16
%16 = OpLabel
OpBranch %11
%11 = OpLabel
OpBranch %7
%7 = OpLabel
)");
}

TEST_F(BuilderTest, If_WithBreak) {
    // loop {
    //   if (true) {
    //     break;
    //   }
    // }

    auto* if_body = Block(Break());

    auto* if_stmt = If(true, if_body);

    auto* loop_body = Block(if_stmt);

    auto* expr = Loop(loop_body, Block());
    WrapInFunction(expr);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();

    EXPECT_TRUE(b.GenerateLoopStatement(expr)) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%5 = OpTypeBool
%6 = OpConstantTrue %5
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(OpBranch %1
%1 = OpLabel
OpLoopMerge %2 %3 None
OpBranch %4
%4 = OpLabel
OpSelectionMerge %7 None
OpBranchConditional %6 %8 %7
%8 = OpLabel
OpBranch %2
%7 = OpLabel
OpBranch %3
%3 = OpLabel
OpBranch %1
%2 = OpLabel
)");
}

TEST_F(BuilderTest, If_WithElseBreak) {
    // loop {
    //   if (true) {
    //   } else {
    //     break;
    //   }
    // }
    auto* else_body = Block(Break());

    auto* if_stmt = If(true, Block(), Else(else_body));

    auto* loop_body = Block(if_stmt);

    auto* expr = Loop(loop_body, Block());
    WrapInFunction(expr);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();

    EXPECT_TRUE(b.GenerateLoopStatement(expr)) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%5 = OpTypeBool
%6 = OpConstantTrue %5
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(OpBranch %1
%1 = OpLabel
OpLoopMerge %2 %3 None
OpBranch %4
%4 = OpLabel
OpSelectionMerge %7 None
OpBranchConditional %6 %8 %9
%8 = OpLabel
OpBranch %7
%9 = OpLabel
OpBranch %2
%7 = OpLabel
OpBranch %3
%3 = OpLabel
OpBranch %1
%2 = OpLabel
)");
}

TEST_F(BuilderTest, If_WithContinueAndBreak) {
    // loop {
    //   if (true) {
    //     continue;
    //   } else {
    //     break;
    //   }
    // }

    auto* if_stmt = If(true, Block(Continue()), Else(Block(Break())));

    auto* expr = Loop(Block(if_stmt), Block());
    WrapInFunction(expr);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();

    EXPECT_TRUE(b.GenerateLoopStatement(expr)) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%5 = OpTypeBool
%6 = OpConstantTrue %5
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(OpBranch %1
%1 = OpLabel
OpLoopMerge %2 %3 None
OpBranch %4
%4 = OpLabel
OpSelectionMerge %7 None
OpBranchConditional %6 %8 %9
%8 = OpLabel
OpBranch %3
%9 = OpLabel
OpBranch %2
%7 = OpLabel
OpBranch %3
%3 = OpLabel
OpBranch %1
%2 = OpLabel
)");
}

TEST_F(BuilderTest, If_WithElseContinue) {
    // loop {
    //   if (true) {
    //   } else {
    //     continue;
    //   }
    //   break;
    // }
    auto* else_body = Block(create<ast::ContinueStatement>());

    auto* if_stmt = If(true, Block(), Else(else_body));

    auto* loop_body = Block(if_stmt, Break());

    auto* expr = Loop(loop_body, Block());
    WrapInFunction(expr);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();

    EXPECT_TRUE(b.GenerateLoopStatement(expr)) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%5 = OpTypeBool
%6 = OpConstantTrue %5
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(OpBranch %1
%1 = OpLabel
OpLoopMerge %2 %3 None
OpBranch %4
%4 = OpLabel
OpSelectionMerge %7 None
OpBranchConditional %6 %8 %9
%8 = OpLabel
OpBranch %7
%9 = OpLabel
OpBranch %3
%7 = OpLabel
OpBranch %2
%3 = OpLabel
OpBranch %1
%2 = OpLabel
)");
}

TEST_F(BuilderTest, If_WithReturn) {
    // if (true) {
    //   return;
    // }

    auto* fn = Func("f", utils::Empty, ty.void_(),
                    utils::Vector{
                        If(true, Block(Return())),
                    });

    spirv::Builder& b = Build();

    EXPECT_TRUE(b.GenerateFunction(fn)) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeVoid
%1 = OpTypeFunction %2
%5 = OpTypeBool
%6 = OpConstantTrue %5
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].instructions()),
              R"(OpSelectionMerge %7 None
OpBranchConditional %6 %8 %7
%8 = OpLabel
OpReturn
%7 = OpLabel
OpReturn
)");
}

TEST_F(BuilderTest, If_WithReturnValue) {
    // if (true) {
    //   return false;
    // }
    // return true;

    auto* fn = Func("f", utils::Empty, ty.bool_(),
                    utils::Vector{
                        If(true, Block(Return(false))),
                        Return(true),
                    });

    spirv::Builder& b = Build();

    EXPECT_TRUE(b.GenerateFunction(fn)) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeBool
%1 = OpTypeFunction %2
%5 = OpConstantTrue %2
%8 = OpConstantNull %2
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].instructions()),
              R"(OpSelectionMerge %6 None
OpBranchConditional %5 %7 %6
%7 = OpLabel
OpReturnValue %8
%6 = OpLabel
OpReturnValue %5
)");
}

TEST_F(BuilderTest, IfElse_BothReturn) {
    // if (true) {
    //   return true;
    // } else {
    //   return true;
    // }

    auto* fn = Func("f", utils::Empty, ty.bool_(),
                    utils::Vector{
                        If(true,                 //
                           Block(Return(true)),  //
                           Else(Block(Return(true)))),
                    });

    spirv::Builder& b = Build();

    EXPECT_TRUE(b.GenerateFunction(fn)) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeBool
%1 = OpTypeFunction %2
%5 = OpConstantTrue %2
%9 = OpConstantNull %2
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].instructions()),
              R"(OpSelectionMerge %6 None
OpBranchConditional %5 %7 %8
%7 = OpLabel
OpReturnValue %5
%8 = OpLabel
OpReturnValue %5
%6 = OpLabel
OpReturnValue %9
)");
}

TEST_F(BuilderTest, If_WithNestedBlockReturnValue) {
    // if (true) {
    //  {
    //    {
    //      {
    //        return false;
    //      }
    //    }
    //  }
    // }
    // return true;

    auto* fn = Func("f", utils::Empty, ty.bool_(),
                    utils::Vector{
                        If(true, Block(Block(Block(Block(Return(false)))))),
                        Return(true),
                    });

    spirv::Builder& b = Build();

    EXPECT_TRUE(b.GenerateFunction(fn)) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeBool
%1 = OpTypeFunction %2
%5 = OpConstantTrue %2
%8 = OpConstantNull %2
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].instructions()),
              R"(OpSelectionMerge %6 None
OpBranchConditional %5 %7 %6
%7 = OpLabel
OpReturnValue %8
%6 = OpLabel
OpReturnValue %5
)");
}

TEST_F(BuilderTest, If_WithLoad_Bug327) {
    // var a : bool;
    // if (a) {
    // }

    auto* var = GlobalVar("a", ty.bool_(), builtin::AddressSpace::kPrivate);
    auto* fn = Func("f", utils::Empty, ty.void_(),
                    utils::Vector{
                        If("a", Block()),
                    });

    spirv::Builder& b = Build();

    ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.Diagnostics();
    EXPECT_TRUE(b.GenerateFunction(fn)) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%3 = OpTypeBool
%2 = OpTypePointer Private %3
%4 = OpConstantNull %3
%1 = OpVariable %2 Private %4
%6 = OpTypeVoid
%5 = OpTypeFunction %6
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].instructions()),
              R"(%9 = OpLoad %3 %1
OpSelectionMerge %10 None
OpBranchConditional %9 %11 %10
%11 = OpLabel
OpBranch %10
%10 = OpLabel
OpReturn
)");
}

TEST_F(BuilderTest, If_ElseIf_WithReturn) {
    // crbug.com/tint/1315
    // if (false) {
    // } else if (true) {
    //   return;
    // }

    auto* if_stmt = If(false, Block(), Else(If(true, Block(Return()))));
    auto* fn = Func("f", utils::Empty, ty.void_(), utils::Vector{if_stmt});

    spirv::Builder& b = Build();

    EXPECT_TRUE(b.GenerateFunction(fn)) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeVoid
%1 = OpTypeFunction %2
%5 = OpTypeBool
%6 = OpConstantNull %5
%10 = OpConstantTrue %5
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].instructions()),
              R"(OpSelectionMerge %7 None
OpBranchConditional %6 %8 %9
%8 = OpLabel
OpBranch %7
%9 = OpLabel
OpSelectionMerge %11 None
OpBranchConditional %10 %12 %11
%12 = OpLabel
OpReturn
%11 = OpLabel
OpBranch %7
%7 = OpLabel
OpReturn
)");
}

TEST_F(BuilderTest, Loop_If_ElseIf_WithBreak) {
    // crbug.com/tint/1315
    // loop {
    //   if (false) {
    //   } else if (true) {
    //     break;
    //   }
    // }

    auto* if_stmt = If(false, Block(), Else(If(true, Block(Break()))));
    auto* fn = Func("f", utils::Empty, ty.void_(),
                    utils::Vector{
                        Loop(Block(if_stmt)),
                    });

    spirv::Builder& b = Build();

    EXPECT_TRUE(b.GenerateFunction(fn)) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeVoid
%1 = OpTypeFunction %2
%9 = OpTypeBool
%10 = OpConstantNull %9
%14 = OpConstantTrue %9
)");
    EXPECT_EQ(DumpInstructions(b.Module().Functions()[0].instructions()),
              R"(OpBranch %5
%5 = OpLabel
OpLoopMerge %6 %7 None
OpBranch %8
%8 = OpLabel
OpSelectionMerge %11 None
OpBranchConditional %10 %12 %13
%12 = OpLabel
OpBranch %11
%13 = OpLabel
OpSelectionMerge %15 None
OpBranchConditional %14 %16 %15
%16 = OpLabel
OpBranch %6
%15 = OpLabel
OpBranch %11
%11 = OpLabel
OpBranch %7
%7 = OpLabel
OpBranch %5
%6 = OpLabel
OpReturn
)");
}

}  // namespace
}  // namespace tint::writer::spirv
