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

#include "src/tint/writer/spirv/spv_dump.h"
#include "src/tint/writer/spirv/test_helper.h"

namespace tint::writer::spirv {
namespace {

using BuilderTest = TestHelper;

TEST_F(BuilderTest, Loop_Empty) {
    // loop {
    //   break;
    // }

    auto* loop = Loop(Block(Break()), Block());
    WrapInFunction(loop);

    spirv::Builder& b = Build();

    b.push_function(Function{});

    EXPECT_TRUE(b.GenerateLoopStatement(loop)) << b.error();
    EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
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

TEST_F(BuilderTest, Loop_WithoutContinuing) {
    // loop {
    //   v = 2;
    //   break;
    // }

    auto* var = Global("v", ty.i32(), ast::StorageClass::kPrivate);
    auto* body = Block(Assign("v", 2),  //
                       Break());

    auto* loop = Loop(body, Block());
    WrapInFunction(loop);

    spirv::Builder& b = Build();

    b.push_function(Function{});
    ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.error();

    EXPECT_TRUE(b.GenerateLoopStatement(loop)) << b.error();
    EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeInt 32 1
%2 = OpTypePointer Private %3
%4 = OpConstantNull %3
%1 = OpVariable %2 Private %4
%9 = OpConstant %3 2
)");
    EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
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

TEST_F(BuilderTest, Loop_WithContinuing) {
    // loop {
    //   a = 2;
    //   break;
    //   continuing {
    //     a = 3;
    //   }
    // }

    auto* var = Global("v", ty.i32(), ast::StorageClass::kPrivate);
    auto* body = Block(Assign("v", 2),  //
                       Break());
    auto* continuing = Block(Assign("v", 3));

    auto* loop = Loop(body, continuing);
    WrapInFunction(loop);

    spirv::Builder& b = Build();

    b.push_function(Function{});
    ASSERT_TRUE(b.GenerateGlobalVariable(var)) << b.error();

    EXPECT_TRUE(b.GenerateLoopStatement(loop)) << b.error();
    EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeInt 32 1
%2 = OpTypePointer Private %3
%4 = OpConstantNull %3
%1 = OpVariable %2 Private %4
%9 = OpConstant %3 2
%10 = OpConstant %3 3
)");
    EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
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

TEST_F(BuilderTest, Loop_WithBodyVariableAccessInContinuing) {
    // loop {
    //   var a : i32;
    //   break;
    //   continuing {
    //     a = 3;
    //   }
    // }

    auto* body = Block(Decl(Var("a", ty.i32())),  //
                       Break());
    auto* continuing = Block(Assign("a", 3));

    auto* loop = Loop(body, continuing);
    WrapInFunction(loop);

    spirv::Builder& b = Build();

    b.push_function(Function{});
    EXPECT_TRUE(b.GenerateLoopStatement(loop)) << b.error();

    EXPECT_EQ(DumpInstructions(b.types()), R"(%7 = OpTypeInt 32 1
%6 = OpTypePointer Function %7
%8 = OpConstantNull %7
%9 = OpConstant %7 3
)");
    EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
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

TEST_F(BuilderTest, Loop_WithContinue) {
    // loop {
    //   if (false) { break; }
    //   continue;
    // }
    auto* body = Block(If(false, Block(Break())),  //
                       Continue());
    auto* loop = Loop(body, Block());
    WrapInFunction(loop);

    spirv::Builder& b = Build();

    b.push_function(Function{});

    EXPECT_TRUE(b.GenerateLoopStatement(loop)) << b.error();
    EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
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

TEST_F(BuilderTest, Loop_WithBreak) {
    // loop {
    //   break;
    // }
    auto* body = Block(create<ast::BreakStatement>());
    auto* loop = Loop(body, Block());
    WrapInFunction(loop);

    spirv::Builder& b = Build();

    b.push_function(Function{});

    EXPECT_TRUE(b.GenerateLoopStatement(loop)) << b.error();
    EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
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

TEST_F(BuilderTest, Loop_WithContinuing_BreakIf) {
    // loop {
    //   continuing {
    //     if (true) { break; }
    //   }
    // }

    auto* if_stmt = If(Expr(true), Block(Break()));
    auto* continuing = Block(if_stmt);
    auto* loop = Loop(Block(), continuing);
    WrapInFunction(loop);

    spirv::Builder& b = Build();

    b.push_function(Function{});

    EXPECT_TRUE(b.GenerateLoopStatement(loop)) << b.error();
    EXPECT_EQ(DumpInstructions(b.types()), R"(%5 = OpTypeBool
%6 = OpConstantTrue %5
)");
    EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
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

TEST_F(BuilderTest, Loop_WithContinuing_BreakUnless) {
    // loop {
    //   continuing {
    //     if (true) {} else { break; }
    //   }
    // }
    auto* if_stmt = If(Expr(true), Block(), Block(Break()));
    auto* continuing = Block(if_stmt);
    auto* loop = Loop(Block(), continuing);
    WrapInFunction(loop);

    spirv::Builder& b = Build();

    b.push_function(Function{});

    EXPECT_TRUE(b.GenerateLoopStatement(loop)) << b.error();
    EXPECT_EQ(DumpInstructions(b.types()), R"(%5 = OpTypeBool
%6 = OpConstantTrue %5
)");
    EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
              R"(OpBranch %1
%1 = OpLabel
OpLoopMerge %2 %3 None
OpBranch %4
%4 = OpLabel
OpBranch %3
%3 = OpLabel
OpBranchConditional %6 %1 %2
%2 = OpLabel
)");
}

TEST_F(BuilderTest, Loop_WithContinuing_BreakIf_ConditionIsVar) {
    // loop {
    //   continuing {
    //     var cond = true;
    //     if (cond) { break; }
    //   }
    // }

    auto* cond_var = Decl(Var("cond", nullptr, Expr(true)));
    auto* if_stmt = If(Expr("cond"), Block(Break()));
    auto* continuing = Block(cond_var, if_stmt);
    auto* loop = Loop(Block(), continuing);
    WrapInFunction(loop);

    spirv::Builder& b = Build();

    b.push_function(Function{});

    EXPECT_TRUE(b.GenerateLoopStatement(loop)) << b.error();
    EXPECT_EQ(DumpInstructions(b.types()), R"(%5 = OpTypeBool
%6 = OpConstantTrue %5
%8 = OpTypePointer Function %5
%9 = OpConstantNull %5
)");
    EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
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

TEST_F(BuilderTest, Loop_WithContinuing_BreakUnless_ConditionIsVar) {
    // loop {
    //   continuing {
    //     var cond = true;
    //     if (cond) {} else { break; }
    //   }
    // }
    auto* cond_var = Decl(Var("cond", nullptr, Expr(true)));
    auto* if_stmt = If(Expr("cond"), Block(), Block(Break()));
    auto* continuing = Block(cond_var, if_stmt);
    auto* loop = Loop(Block(), continuing);
    WrapInFunction(loop);

    spirv::Builder& b = Build();

    b.push_function(Function{});

    EXPECT_TRUE(b.GenerateLoopStatement(loop)) << b.error();
    EXPECT_EQ(DumpInstructions(b.types()), R"(%5 = OpTypeBool
%6 = OpConstantTrue %5
%8 = OpTypePointer Function %5
%9 = OpConstantNull %5
)");
    EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
              R"(OpBranch %1
%1 = OpLabel
OpLoopMerge %2 %3 None
OpBranch %4
%4 = OpLabel
OpBranch %3
%3 = OpLabel
OpStore %7 %6
%10 = OpLoad %5 %7
OpBranchConditional %10 %1 %2
%2 = OpLabel
)");
}

TEST_F(BuilderTest, Loop_WithContinuing_BreakIf_Nested) {
    // Make sure the right backedge and break target are used.
    // loop {
    //   continuing {
    //     loop {
    //       continuing {
    //         if (true) { break; }
    //       }
    //     }
    //     if (true) { break; }
    //   }
    // }

    auto* inner_if_stmt = If(Expr(true), Block(Break()));
    auto* inner_continuing = Block(inner_if_stmt);
    auto* inner_loop = Loop(Block(), inner_continuing);

    auto* outer_if_stmt = If(Expr(true), Block(Break()));
    auto* outer_continuing = Block(inner_loop, outer_if_stmt);
    auto* outer_loop = Loop(Block(), outer_continuing);

    WrapInFunction(outer_loop);

    spirv::Builder& b = Build();

    b.push_function(Function{});

    EXPECT_TRUE(b.GenerateLoopStatement(outer_loop)) << b.error();
    EXPECT_EQ(DumpInstructions(b.types()), R"(%9 = OpTypeBool
%10 = OpConstantTrue %9
)");
    EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
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

TEST_F(BuilderTest, Loop_WithContinuing_BreakUnless_Nested) {
    // Make sure the right backedge and break target are used.
    // loop {
    //   continuing {
    //     loop {
    //       continuing {
    //         if (true) {} else { break; }
    //       }
    //     }
    //     if (true) {} else { break; }
    //   }
    // }

    auto* inner_if_stmt = If(Expr(true), Block(), Block(Break()));
    auto* inner_continuing = Block(inner_if_stmt);
    auto* inner_loop = Loop(Block(), inner_continuing);

    auto* outer_if_stmt = If(Expr(true), Block(), Block(Break()));
    auto* outer_continuing = Block(inner_loop, outer_if_stmt);
    auto* outer_loop = Loop(Block(), outer_continuing);

    WrapInFunction(outer_loop);

    spirv::Builder& b = Build();

    b.push_function(Function{});

    EXPECT_TRUE(b.GenerateLoopStatement(outer_loop)) << b.error();
    EXPECT_EQ(DumpInstructions(b.types()), R"(%9 = OpTypeBool
%10 = OpConstantTrue %9
)");
    EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
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
OpBranchConditional %10 %5 %6
%6 = OpLabel
OpBranchConditional %10 %1 %2
%2 = OpLabel
)");
}

}  // namespace
}  // namespace tint::writer::spirv
