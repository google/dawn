// Copyright 2020 The Dawn & Tint Authors
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

#include "gmock/gmock.h"
#include "gtest/gtest-spi.h"
#include "src/tint/lang/spirv/writer/ast_printer/helper_test.h"
#include "src/tint/lang/spirv/writer/common/spv_dump_test.h"

using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::spirv::writer {
namespace {

using SpirvASTPrinterTest = TestHelper;

TEST_F(SpirvASTPrinterTest, If_Empty) {
    // if (true) {
    // }
    auto* expr = If(true, Block());
    WrapInFunction(expr);

    Builder& b = Build();

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

TEST_F(SpirvASTPrinterTest, If_Empty_OutsideFunction_IsError) {
    // Outside a function.
    // if (true) {
    // }

    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder pb;

            auto* block = pb.Block();
            auto* expr = pb.If(true, block);
            pb.WrapInFunction(expr);

            auto program = resolver::Resolve(pb);
            Builder b(program);

            b.GenerateIfStatement(expr);
        },
        "Internal error: trying to add SPIR-V instruction 247 outside a function");
}

TEST_F(SpirvASTPrinterTest, If_WithStatements) {
    // if (true) {
    //   v = 2;
    // }

    auto* var = GlobalVar("v", ty.i32(), core::AddressSpace::kPrivate);
    auto* body = Block(Assign("v", 2_i));
    auto* expr = If(true, body);
    WrapInFunction(expr);

    Builder& b = Build();

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

TEST_F(SpirvASTPrinterTest, If_WithElse) {
    // if (true) {
    //   v = 2i;
    // } else {
    //   v = 3i;
    // }

    auto* var = GlobalVar("v", ty.i32(), core::AddressSpace::kPrivate);
    auto* body = Block(Assign("v", 2_i));
    auto* else_body = Block(Assign("v", 3_i));

    auto* expr = If(true, body, Else(else_body));
    WrapInFunction(expr);

    Builder& b = Build();

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

TEST_F(SpirvASTPrinterTest, If_WithElseIf) {
    // if (true) {
    //   v = 2i;
    // } else if (true) {
    //   v = 3i;
    // }

    auto* var = GlobalVar("v", ty.i32(), core::AddressSpace::kPrivate);
    auto* body = Block(Assign("v", 2_i));
    auto* else_body = Block(Assign("v", 3_i));

    auto* expr = If(true, body, Else(If(true, else_body)));
    WrapInFunction(expr);

    Builder& b = Build();

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

TEST_F(SpirvASTPrinterTest, If_WithMultiple) {
    // if (true) {
    //   v = 2i;
    // } else if (true) {
    //   v = 3i;
    // } else if (false) {
    //   v = 4i;
    // } else {
    //   v = 5i;
    // }

    auto* var = GlobalVar("v", ty.i32(), core::AddressSpace::kPrivate);
    auto* body = Block(Assign("v", 2_i));
    auto* elseif_1_body = Block(Assign("v", 3_i));
    auto* elseif_2_body = Block(Assign("v", 4_i));
    auto* else_body = Block(Assign("v", 5_i));

    auto* expr = If(true, body,                            //
                    Else(If(true, elseif_1_body,           //
                            Else(If(false, elseif_2_body,  //
                                    Else(else_body))))));
    WrapInFunction(expr);

    Builder& b = Build();

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

TEST_F(SpirvASTPrinterTest, If_WithBreak) {
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

    Builder& b = Build();

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

TEST_F(SpirvASTPrinterTest, If_WithElseBreak) {
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

    Builder& b = Build();

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

TEST_F(SpirvASTPrinterTest, If_WithContinueAndBreak) {
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

    Builder& b = Build();

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

TEST_F(SpirvASTPrinterTest, If_WithElseContinue) {
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

    Builder& b = Build();

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

TEST_F(SpirvASTPrinterTest, If_WithReturn) {
    // if (true) {
    //   return;
    // }

    auto* fn = Func("f", tint::Empty, ty.void_(),
                    Vector{
                        If(true, Block(Return())),
                    });

    Builder& b = Build();

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

TEST_F(SpirvASTPrinterTest, If_WithReturnValue) {
    // if (true) {
    //   return false;
    // }
    // return true;

    auto* fn = Func("f", tint::Empty, ty.bool_(),
                    Vector{
                        If(true, Block(Return(false))),
                        Return(true),
                    });

    Builder& b = Build();

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

TEST_F(SpirvASTPrinterTest, IfElse_BothReturn) {
    // if (true) {
    //   return true;
    // } else {
    //   return true;
    // }

    auto* fn = Func("f", tint::Empty, ty.bool_(),
                    Vector{
                        If(true,                 //
                           Block(Return(true)),  //
                           Else(Block(Return(true)))),
                    });

    Builder& b = Build();

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

TEST_F(SpirvASTPrinterTest, If_WithNestedBlockReturnValue) {
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

    auto* fn = Func("f", tint::Empty, ty.bool_(),
                    Vector{
                        If(true, Block(Block(Block(Block(Return(false)))))),
                        Return(true),
                    });

    Builder& b = Build();

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

TEST_F(SpirvASTPrinterTest, If_WithLoad_Bug327) {
    // var a : bool;
    // if (a) {
    // }

    auto* var = GlobalVar("a", ty.bool_(), core::AddressSpace::kPrivate);
    auto* fn = Func("f", tint::Empty, ty.void_(),
                    Vector{
                        If("a", Block()),
                    });

    Builder& b = Build();

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

TEST_F(SpirvASTPrinterTest, If_ElseIf_WithReturn) {
    // crbug.com/tint/1315
    // if (false) {
    // } else if (true) {
    //   return;
    // }

    auto* if_stmt = If(false, Block(), Else(If(true, Block(Return()))));
    auto* fn = Func("f", tint::Empty, ty.void_(), Vector{if_stmt});

    Builder& b = Build();

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

TEST_F(SpirvASTPrinterTest, Loop_If_ElseIf_WithBreak) {
    // crbug.com/tint/1315
    // loop {
    //   if (false) {
    //   } else if (true) {
    //     break;
    //   }
    // }

    auto* if_stmt = If(false, Block(), Else(If(true, Block(Break()))));
    auto* fn = Func("f", tint::Empty, ty.void_(),
                    Vector{
                        Loop(Block(if_stmt)),
                    });

    Builder& b = Build();

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
}  // namespace tint::spirv::writer
