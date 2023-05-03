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

using namespace tint::number_suffixes;  // NOLINT

namespace tint::writer::spirv {
namespace {

using BuilderTest = TestHelper;

TEST_F(BuilderTest, Switch_Empty) {
    // switch (1i) {
    //   default: {}
    // }

    auto* expr = Switch(1_i, DefaultCase());
    WrapInFunction(expr);

    spirv::Builder& b = Build();

    b.PushFunctionForTesting();

    EXPECT_TRUE(b.GenerateSwitchStatement(expr)) << b.Diagnostics();
    EXPECT_EQ(DumpInstructions(b.Module().Types()), R"(%2 = OpTypeInt 32 1
%3 = OpConstant %2 1
)");
    EXPECT_EQ(DumpInstructions(b.CurrentFunction().instructions()),
              R"(OpSelectionMerge %1 None
OpSwitch %3 %4
%4 = OpLabel
OpBranch %1
%1 = OpLabel
)");
}

TEST_F(BuilderTest, Switch_WithCase) {
    // switch(a) {
    //   case 1i:
    //     v = 1i;
    //   case 2i:
    //     v = 2i;
    //   default: {}
    // }

    auto* v = GlobalVar("v", ty.i32(), builtin::AddressSpace::kPrivate);
    auto* a = GlobalVar("a", ty.i32(), builtin::AddressSpace::kPrivate);

    auto* func = Func("a_func", utils::Empty, ty.void_(),
                      utils::Vector{
                          Switch("a",                                               //
                                 Case(CaseSelector(1_i), Block(Assign("v", 1_i))),  //
                                 Case(CaseSelector(2_i), Block(Assign("v", 2_i))),  //
                                 DefaultCase()),
                      });

    spirv::Builder& b = Build();

    ASSERT_TRUE(b.GenerateGlobalVariable(v)) << b.Diagnostics();
    ASSERT_TRUE(b.GenerateGlobalVariable(a)) << b.Diagnostics();
    ASSERT_TRUE(b.GenerateFunction(func)) << b.Diagnostics();

    EXPECT_EQ(DumpBuilder(b), R"(OpName %1 "v"
OpName %5 "a"
OpName %8 "a_func"
%3 = OpTypeInt 32 1
%2 = OpTypePointer Private %3
%4 = OpConstantNull %3
%1 = OpVariable %2 Private %4
%5 = OpVariable %2 Private %4
%7 = OpTypeVoid
%6 = OpTypeFunction %7
%15 = OpConstant %3 1
%16 = OpConstant %3 2
%8 = OpFunction %7 None %6
%9 = OpLabel
%11 = OpLoad %3 %5
OpSelectionMerge %10 None
OpSwitch %11 %12 1 %13 2 %14
%13 = OpLabel
OpStore %1 %15
OpBranch %10
%14 = OpLabel
OpStore %1 %16
OpBranch %10
%12 = OpLabel
OpBranch %10
%10 = OpLabel
OpReturn
OpFunctionEnd
)");
}

TEST_F(BuilderTest, Switch_WithCase_Unsigned) {
    // switch(a) {
    //   case 1u:
    //     v = 1i;
    //   case 2u:
    //     v = 2i;
    //   default: {}
    // }

    auto* v = GlobalVar("v", ty.i32(), builtin::AddressSpace::kPrivate);
    auto* a = GlobalVar("a", ty.u32(), builtin::AddressSpace::kPrivate);

    auto* func = Func("a_func", utils::Empty, ty.void_(),
                      utils::Vector{
                          Switch("a",                                               //
                                 Case(CaseSelector(1_u), Block(Assign("v", 1_i))),  //
                                 Case(CaseSelector(2_u), Block(Assign("v", 2_i))),  //
                                 DefaultCase()),
                      });

    spirv::Builder& b = Build();

    ASSERT_TRUE(b.GenerateGlobalVariable(v)) << b.Diagnostics();
    ASSERT_TRUE(b.GenerateGlobalVariable(a)) << b.Diagnostics();
    ASSERT_TRUE(b.GenerateFunction(func)) << b.Diagnostics();

    EXPECT_EQ(DumpBuilder(b), R"(OpName %1 "v"
OpName %5 "a"
OpName %11 "a_func"
%3 = OpTypeInt 32 1
%2 = OpTypePointer Private %3
%4 = OpConstantNull %3
%1 = OpVariable %2 Private %4
%7 = OpTypeInt 32 0
%6 = OpTypePointer Private %7
%8 = OpConstantNull %7
%5 = OpVariable %6 Private %8
%10 = OpTypeVoid
%9 = OpTypeFunction %10
%18 = OpConstant %3 1
%19 = OpConstant %3 2
%11 = OpFunction %10 None %9
%12 = OpLabel
%14 = OpLoad %7 %5
OpSelectionMerge %13 None
OpSwitch %14 %15 1 %16 2 %17
%16 = OpLabel
OpStore %1 %18
OpBranch %13
%17 = OpLabel
OpStore %1 %19
OpBranch %13
%15 = OpLabel
OpBranch %13
%13 = OpLabel
OpReturn
OpFunctionEnd
)");
}

TEST_F(BuilderTest, Switch_WithDefault) {
    // switch(true) {
    //   default: {}
    //     v = 1i;
    //  }

    auto* v = GlobalVar("v", ty.i32(), builtin::AddressSpace::kPrivate);
    auto* a = GlobalVar("a", ty.i32(), builtin::AddressSpace::kPrivate);

    auto* func = Func("a_func", utils::Empty, ty.void_(),
                      utils::Vector{
                          Switch("a",                                    //
                                 DefaultCase(Block(Assign("v", 1_i)))),  //
                      });

    spirv::Builder& b = Build();

    ASSERT_TRUE(b.GenerateGlobalVariable(v)) << b.Diagnostics();
    ASSERT_TRUE(b.GenerateGlobalVariable(a)) << b.Diagnostics();
    ASSERT_TRUE(b.GenerateFunction(func)) << b.Diagnostics();

    EXPECT_EQ(DumpBuilder(b), R"(OpName %1 "v"
OpName %5 "a"
OpName %8 "a_func"
%3 = OpTypeInt 32 1
%2 = OpTypePointer Private %3
%4 = OpConstantNull %3
%1 = OpVariable %2 Private %4
%5 = OpVariable %2 Private %4
%7 = OpTypeVoid
%6 = OpTypeFunction %7
%13 = OpConstant %3 1
%8 = OpFunction %7 None %6
%9 = OpLabel
%11 = OpLoad %3 %5
OpSelectionMerge %10 None
OpSwitch %11 %12
%12 = OpLabel
OpStore %1 %13
OpBranch %10
%10 = OpLabel
OpReturn
OpFunctionEnd
)");
}

TEST_F(BuilderTest, Switch_WithCaseAndDefault) {
    // switch(a) {
    //   case 1i:
    //      v = 1i;
    //   case 2i, 3i:
    //      v = 2i;
    //   default: {}
    //      v = 3i;
    //  }

    auto* v = GlobalVar("v", ty.i32(), builtin::AddressSpace::kPrivate);
    auto* a = GlobalVar("a", ty.i32(), builtin::AddressSpace::kPrivate);

    auto* func = Func("a_func", utils::Empty, ty.void_(),
                      utils::Vector{
                          Switch(Expr("a"),                                                 //
                                 Case(CaseSelector(1_i),                                    //
                                      Block(Assign("v", 1_i))),                             //
                                 Case(utils::Vector{CaseSelector(2_i), CaseSelector(3_i)},  //
                                      Block(Assign("v", 2_i))),                             //
                                 DefaultCase(Block(Assign("v", 3_i)))),
                      });

    spirv::Builder& b = Build();

    ASSERT_TRUE(b.GenerateGlobalVariable(v)) << b.Diagnostics();
    ASSERT_TRUE(b.GenerateGlobalVariable(a)) << b.Diagnostics();
    ASSERT_TRUE(b.GenerateFunction(func)) << b.Diagnostics();

    EXPECT_EQ(DumpBuilder(b), R"(OpName %1 "v"
OpName %5 "a"
OpName %8 "a_func"
%3 = OpTypeInt 32 1
%2 = OpTypePointer Private %3
%4 = OpConstantNull %3
%1 = OpVariable %2 Private %4
%5 = OpVariable %2 Private %4
%7 = OpTypeVoid
%6 = OpTypeFunction %7
%15 = OpConstant %3 1
%16 = OpConstant %3 2
%17 = OpConstant %3 3
%8 = OpFunction %7 None %6
%9 = OpLabel
%11 = OpLoad %3 %5
OpSelectionMerge %10 None
OpSwitch %11 %12 1 %13 2 %14 3 %14
%13 = OpLabel
OpStore %1 %15
OpBranch %10
%14 = OpLabel
OpStore %1 %16
OpBranch %10
%12 = OpLabel
OpStore %1 %17
OpBranch %10
%10 = OpLabel
OpReturn
OpFunctionEnd
)");
}

TEST_F(BuilderTest, Switch_WithCaseAndMixedDefault) {
    // switch(a) {
    //   case 1i:
    //      v = 1i;
    //   case 2i, 3i, default:
    //      v = 2i;
    //  }

    auto* v = GlobalVar("v", ty.i32(), builtin::AddressSpace::kPrivate);
    auto* a = GlobalVar("a", ty.i32(), builtin::AddressSpace::kPrivate);

    auto* func = Func("a_func", utils::Empty, ty.void_(),
                      utils::Vector{Switch(Expr("a"),                      //
                                           Case(CaseSelector(1_i),         //
                                                Block(Assign("v", 1_i))),  //
                                           Case(utils::Vector{CaseSelector(2_i), CaseSelector(3_i),
                                                              DefaultCaseSelector()},  //
                                                Block(Assign("v", 2_i)))               //
                                           )});

    spirv::Builder& b = Build();

    ASSERT_TRUE(b.GenerateGlobalVariable(v)) << b.Diagnostics();
    ASSERT_TRUE(b.GenerateGlobalVariable(a)) << b.Diagnostics();
    ASSERT_TRUE(b.GenerateFunction(func)) << b.Diagnostics();

    EXPECT_EQ(DumpBuilder(b), R"(OpName %1 "v"
OpName %5 "a"
OpName %8 "a_func"
%3 = OpTypeInt 32 1
%2 = OpTypePointer Private %3
%4 = OpConstantNull %3
%1 = OpVariable %2 Private %4
%5 = OpVariable %2 Private %4
%7 = OpTypeVoid
%6 = OpTypeFunction %7
%14 = OpConstant %3 1
%15 = OpConstant %3 2
%8 = OpFunction %7 None %6
%9 = OpLabel
%11 = OpLoad %3 %5
OpSelectionMerge %10 None
OpSwitch %11 %12 1 %13 2 %12 3 %12
%13 = OpLabel
OpStore %1 %14
OpBranch %10
%12 = OpLabel
OpStore %1 %15
OpBranch %10
%10 = OpLabel
OpReturn
OpFunctionEnd
)");
}

TEST_F(BuilderTest, Switch_WithNestedBreak) {
    // switch (a) {
    //   case 1:
    //     if (true) {
    //       break;
    //     }
    //     v = 1i;
    //   default: {}
    // }

    auto* v = GlobalVar("v", ty.i32(), builtin::AddressSpace::kPrivate);
    auto* a = GlobalVar("a", ty.i32(), builtin::AddressSpace::kPrivate);

    auto* func = Func("a_func", utils::Empty, ty.void_(),
                      utils::Vector{
                          Switch("a",                     //
                                 Case(CaseSelector(1_i),  //
                                      Block(              //
                                          If(Expr(true), Block(create<ast::BreakStatement>())),
                                          Assign("v", 1_i))),
                                 DefaultCase()),
                      });

    spirv::Builder& b = Build();

    ASSERT_TRUE(b.GenerateGlobalVariable(v)) << b.Diagnostics();
    ASSERT_TRUE(b.GenerateGlobalVariable(a)) << b.Diagnostics();
    ASSERT_TRUE(b.GenerateFunction(func)) << b.Diagnostics();

    EXPECT_EQ(DumpBuilder(b), R"(OpName %1 "v"
OpName %5 "a"
OpName %8 "a_func"
%3 = OpTypeInt 32 1
%2 = OpTypePointer Private %3
%4 = OpConstantNull %3
%1 = OpVariable %2 Private %4
%5 = OpVariable %2 Private %4
%7 = OpTypeVoid
%6 = OpTypeFunction %7
%14 = OpTypeBool
%15 = OpConstantTrue %14
%18 = OpConstant %3 1
%8 = OpFunction %7 None %6
%9 = OpLabel
%11 = OpLoad %3 %5
OpSelectionMerge %10 None
OpSwitch %11 %12 1 %13
%13 = OpLabel
OpSelectionMerge %16 None
OpBranchConditional %15 %17 %16
%17 = OpLabel
OpBranch %10
%16 = OpLabel
OpStore %1 %18
OpBranch %10
%12 = OpLabel
OpBranch %10
%10 = OpLabel
OpReturn
OpFunctionEnd
)");
}

TEST_F(BuilderTest, Switch_AllReturn) {
    // switch (1i) {
    //   case 1i: {
    //     return 1i;
    //   }
    //   case 2i: {
    //     return 1i;
    //   }
    //   default: {
    //     return 3i;
    //   }
    // }

    auto* fn = Func("f", utils::Empty, ty.i32(),
                    utils::Vector{
                        Switch(1_i,                                          //
                               Case(CaseSelector(1_i), Block(Return(1_i))),  //
                               Case(CaseSelector(2_i), Block(Return(1_i))),  //
                               DefaultCase(Block(Return(3_i)))),
                    });

    spirv::Builder& b = Build();

    EXPECT_TRUE(b.GenerateFunction(fn)) << b.Diagnostics();
    EXPECT_EQ(DumpBuilder(b), R"(OpName %3 "f"
%2 = OpTypeInt 32 1
%1 = OpTypeFunction %2
%6 = OpConstant %2 1
%10 = OpConstant %2 3
%11 = OpConstantNull %2
%3 = OpFunction %2 None %1
%4 = OpLabel
OpSelectionMerge %5 None
OpSwitch %6 %7 1 %8 2 %9
%8 = OpLabel
OpReturnValue %6
%9 = OpLabel
OpReturnValue %6
%7 = OpLabel
OpReturnValue %10
%5 = OpLabel
OpReturnValue %11
OpFunctionEnd
)");
}

}  // namespace
}  // namespace tint::writer::spirv
