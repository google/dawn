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

#include "src/tint/lang/spirv/writer/ast_printer/helper_test.h"
#include "src/tint/lang/spirv/writer/common/spv_dump_test.h"

using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::spirv::writer {
namespace {

using SpirvASTPrinterTest = TestHelper;

TEST_F(SpirvASTPrinterTest, Loop_Empty) {
    // loop {
    //   break;
    // }

    auto* loop = Loop(Block(Break()), Block());
    WrapInFunction(loop);

    Builder& b = Build();

    b.PushFunctionForTesting();

    EXPECT_TRUE(b.GenerateLoopStatement(loop)) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(OpBranch %1
%1 = OpLabel
OpLoopMerge %2 %3 None
OpBranch %4
%4 = OpLabel
OpBranch %2
%3 = OpLabel
OpBranch %1
%2 = OpLabel
)");
}

TEST_F(SpirvASTPrinterTest, Loop_WithoutContinuing) {
    // loop {
    //   v = 2i;
    //   break;
    // }

    auto* var = GlobalVar("v", ty.i32(), core::AddressSpace::kPrivate);
    auto* body = Block(Assign("v", 2_i),  //
                       Break());

    auto* loop = Loop(body, Block());
    WrapInFunction(loop);

    Builder& b = Build();

    b.PushFunctionForTesting();
    ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.Diagnostics();

    EXPECT_TRUE(b.GenerateLoopStatement(loop)) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%3 = OpTypeInt 32 1
%2 = OpTypePointer Private %3
%4 = OpConstantNull %3
%1 = OpVariable %2 Private %4
%9 = OpConstant %3 2
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(OpBranch %5
%5 = OpLabel
OpLoopMerge %6 %7 None
OpBranch %8
%8 = OpLabel
OpStore %1 %9
OpBranch %6
%7 = OpLabel
OpBranch %5
%6 = OpLabel
)");
}

TEST_F(SpirvASTPrinterTest, Loop_WithContinuing) {
    // loop {
    //   a = 2i;
    //   break;
    //   continuing {
    //     a = 3i;
    //   }
    // }

    auto* var = GlobalVar("v", ty.i32(), core::AddressSpace::kPrivate);
    auto* body = Block(Assign("v", 2_i),  //
                       Break());
    auto* continuing = Block(Assign("v", 3_i));

    auto* loop = Loop(body, continuing);
    WrapInFunction(loop);

    Builder& b = Build();

    b.PushFunctionForTesting();
    ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.Diagnostics();

    EXPECT_TRUE(b.GenerateLoopStatement(loop)) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%3 = OpTypeInt 32 1
%2 = OpTypePointer Private %3
%4 = OpConstantNull %3
%1 = OpVariable %2 Private %4
%9 = OpConstant %3 2
%10 = OpConstant %3 3
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(OpBranch %5
%5 = OpLabel
OpLoopMerge %6 %7 None
OpBranch %8
%8 = OpLabel
OpStore %1 %9
OpBranch %6
%7 = OpLabel
OpStore %1 %10
OpBranch %5
%6 = OpLabel
)");
}

TEST_F(SpirvASTPrinterTest, Loop_WithBodyVariableAccessInContinuing) {
    // loop {
    //   var a : i32;
    //   break;
    //   continuing {
    //     a = 3i;
    //   }
    // }

    auto* body = Block(Decl(Var("a", ty.i32())),  //
                       Break());
    auto* continuing = Block(Assign("a", 3_i));

    auto* loop = Loop(body, continuing);
    WrapInFunction(loop);

    Builder& b = Build();

    b.PushFunctionForTesting();
    EXPECT_TRUE(b.GenerateLoopStatement(loop)) << b.Diagnostics();

    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%7 = OpTypeInt 32 1
%6 = OpTypePointer Function %7
%8 = OpConstantNull %7
%9 = OpConstant %7 3
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(OpBranch %1
%1 = OpLabel
OpLoopMerge %2 %3 None
OpBranch %4
%4 = OpLabel
OpBranch %2
%3 = OpLabel
OpStore %5 %9
OpBranch %1
%2 = OpLabel
)");
}

TEST_F(SpirvASTPrinterTest, Loop_WithContinue) {
    // loop {
    //   if (false) { break; }
    //   continue;
    // }
    auto* body = Block(If(false, Block(Break())),  //
                       Continue());
    auto* loop = Loop(body, Block());
    WrapInFunction(loop);

    Builder& b = Build();

    b.PushFunctionForTesting();

    EXPECT_TRUE(b.GenerateLoopStatement(loop)) << b.Diagnostics();
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

TEST_F(SpirvASTPrinterTest, Loop_WithBreak) {
    // loop {
    //   break;
    // }
    auto* body = Block(create<ast::BreakStatement>());
    auto* loop = Loop(body, Block());
    WrapInFunction(loop);

    Builder& b = Build();

    b.PushFunctionForTesting();

    EXPECT_TRUE(b.GenerateLoopStatement(loop)) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(OpBranch %1
%1 = OpLabel
OpLoopMerge %2 %3 None
OpBranch %4
%4 = OpLabel
OpBranch %2
%3 = OpLabel
OpBranch %1
%2 = OpLabel
)");
}

TEST_F(SpirvASTPrinterTest, Loop_WithContinuing_BreakIf) {
    // loop {
    //   continuing {
    //     break if (true);
    //   }
    // }

    auto* continuing = Block(BreakIf(true));
    auto* loop = Loop(Block(), continuing);
    WrapInFunction(loop);

    Builder& b = Build();

    b.PushFunctionForTesting();

    EXPECT_TRUE(b.GenerateLoopStatement(loop)) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%5 = OpTypeBool
%6 = OpConstantTrue %5
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(OpBranch %1
%1 = OpLabel
OpLoopMerge %2 %3 None
OpBranch %4
%4 = OpLabel
OpBranch %3
%3 = OpLabel
OpBranchConditional %6 %2 %1
%2 = OpLabel
)");
}

TEST_F(SpirvASTPrinterTest, Loop_WithContinuing_BreakUnless) {
    // loop {
    //   continuing {
    //     break if (false);
    //   }
    // }
    auto* continuing = Block(BreakIf(false));
    auto* loop = Loop(Block(), continuing);
    WrapInFunction(loop);

    Builder& b = Build();

    b.PushFunctionForTesting();

    EXPECT_TRUE(b.GenerateLoopStatement(loop)) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%5 = OpTypeBool
%6 = OpConstantNull %5
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(OpBranch %1
%1 = OpLabel
OpLoopMerge %2 %3 None
OpBranch %4
%4 = OpLabel
OpBranch %3
%3 = OpLabel
OpBranchConditional %6 %2 %1
%2 = OpLabel
)");
}

TEST_F(SpirvASTPrinterTest, Loop_WithContinuing_BreakIf_ConditionIsVar) {
    // loop {
    //   continuing {
    //     var cond = true;
    //     break if (cond);
    //   }
    // }

    auto* cond_var = Decl(Var("cond", Expr(true)));
    auto* continuing = Block(cond_var, BreakIf("cond"));
    auto* loop = Loop(Block(), continuing);
    WrapInFunction(loop);

    Builder& b = Build();

    b.PushFunctionForTesting();

    EXPECT_TRUE(b.GenerateLoopStatement(loop)) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%5 = OpTypeBool
%6 = OpConstantTrue %5
%8 = OpTypePointer Function %5
%9 = OpConstantNull %5
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(OpBranch %1
%1 = OpLabel
OpLoopMerge %2 %3 None
OpBranch %4
%4 = OpLabel
OpBranch %3
%3 = OpLabel
OpStore %7 %6
%10 = OpLoad %5 %7
OpBranchConditional %10 %2 %1
%2 = OpLabel
)");
}

TEST_F(SpirvASTPrinterTest, Loop_WithContinuing_BreakIf_Nested) {
    // Make sure the right backedge and break target are used.
    // loop {
    //   continuing {
    //     loop {
    //       continuing {
    //         break if (true);
    //       }
    //     }
    //     break if (true);
    //   }
    // }

    auto* inner_continuing = Block(BreakIf(true));
    auto* inner_loop = Loop(Block(), inner_continuing);

    auto* outer_continuing = Block(inner_loop, BreakIf(true));
    auto* outer_loop = Loop(Block(), outer_continuing);

    WrapInFunction(outer_loop);

    Builder& b = Build();

    b.PushFunctionForTesting();

    EXPECT_TRUE(b.GenerateLoopStatement(outer_loop)) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%9 = OpTypeBool
%10 = OpConstantTrue %9
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(OpBranch %1
%1 = OpLabel
OpLoopMerge %2 %3 None
OpBranch %4
%4 = OpLabel
OpBranch %3
%3 = OpLabel
OpBranch %5
%5 = OpLabel
OpLoopMerge %6 %7 None
OpBranch %8
%8 = OpLabel
OpBranch %7
%7 = OpLabel
OpBranchConditional %10 %6 %5
%6 = OpLabel
OpBranchConditional %10 %2 %1
%2 = OpLabel
)");
}

}  // namespace
}  // namespace tint::spirv::writer
