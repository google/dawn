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

TEST_F(SpirvASTPrinterTest, Switch_Empty) {
    // switch (1i) {
    //   default: {}
    // }

    auto* expr = Switch(1_i, DefaultCase());
    WrapInFunction(expr);

    Builder& b = Build();

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

TEST_F(SpirvASTPrinterTest, Switch_WithCase) {
    // switch(a) {
    //   case 1i:
    //     v = 1i;
    //   case 2i:
    //     v = 2i;
    //   default: {}
    // }

    auto* v = GlobalVar("v", ty.i32(), core::AddressSpace::kPrivate);
    auto* a = GlobalVar("a", ty.i32(), core::AddressSpace::kPrivate);

    auto* func = Func("a_func", tint::Empty, ty.void_(),
                      Vector{
                          Switch("a",                                               //
                                 Case(CaseSelector(1_i), Block(Assign("v", 1_i))),  //
                                 Case(CaseSelector(2_i), Block(Assign("v", 2_i))),  //
                                 DefaultCase()),
                      });

    Builder& b = Build();

    ASSERT_TRUE(b.GenerateGlobalVariable(v)) << b.Diagnostics();
    ASSERT_TRUE(b.GenerateGlobalVariable(a)) << b.Diagnostics();
    ASSERT_TRUE(b.GenerateFunction(func)) << b.Diagnostics();

    EXPECT_EQ(DumpModule(b.Module()), R"(OpName %1 "v"
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

TEST_F(SpirvASTPrinterTest, Switch_WithCase_Unsigned) {
    // switch(a) {
    //   case 1u:
    //     v = 1i;
    //   case 2u:
    //     v = 2i;
    //   default: {}
    // }

    auto* v = GlobalVar("v", ty.i32(), core::AddressSpace::kPrivate);
    auto* a = GlobalVar("a", ty.u32(), core::AddressSpace::kPrivate);

    auto* func = Func("a_func", tint::Empty, ty.void_(),
                      Vector{
                          Switch("a",                                               //
                                 Case(CaseSelector(1_u), Block(Assign("v", 1_i))),  //
                                 Case(CaseSelector(2_u), Block(Assign("v", 2_i))),  //
                                 DefaultCase()),
                      });

    Builder& b = Build();

    ASSERT_TRUE(b.GenerateGlobalVariable(v)) << b.Diagnostics();
    ASSERT_TRUE(b.GenerateGlobalVariable(a)) << b.Diagnostics();
    ASSERT_TRUE(b.GenerateFunction(func)) << b.Diagnostics();

    EXPECT_EQ(DumpModule(b.Module()), R"(OpName %1 "v"
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

TEST_F(SpirvASTPrinterTest, Switch_WithDefault) {
    // switch(true) {
    //   default: {}
    //     v = 1i;
    //  }

    auto* v = GlobalVar("v", ty.i32(), core::AddressSpace::kPrivate);
    auto* a = GlobalVar("a", ty.i32(), core::AddressSpace::kPrivate);

    auto* func = Func("a_func", tint::Empty, ty.void_(),
                      Vector{
                          Switch("a",                                    //
                                 DefaultCase(Block(Assign("v", 1_i)))),  //
                      });

    Builder& b = Build();

    ASSERT_TRUE(b.GenerateGlobalVariable(v)) << b.Diagnostics();
    ASSERT_TRUE(b.GenerateGlobalVariable(a)) << b.Diagnostics();
    ASSERT_TRUE(b.GenerateFunction(func)) << b.Diagnostics();

    EXPECT_EQ(DumpModule(b.Module()), R"(OpName %1 "v"
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

TEST_F(SpirvASTPrinterTest, Switch_WithCaseAndDefault) {
    // switch(a) {
    //   case 1i:
    //      v = 1i;
    //   case 2i, 3i:
    //      v = 2i;
    //   default: {}
    //      v = 3i;
    //  }

    auto* v = GlobalVar("v", ty.i32(), core::AddressSpace::kPrivate);
    auto* a = GlobalVar("a", ty.i32(), core::AddressSpace::kPrivate);

    auto* func = Func("a_func", tint::Empty, ty.void_(),
                      Vector{
                          Switch(Expr("a"),                                          //
                                 Case(CaseSelector(1_i),                             //
                                      Block(Assign("v", 1_i))),                      //
                                 Case(Vector{CaseSelector(2_i), CaseSelector(3_i)},  //
                                      Block(Assign("v", 2_i))),                      //
                                 DefaultCase(Block(Assign("v", 3_i)))),
                      });

    Builder& b = Build();

    ASSERT_TRUE(b.GenerateGlobalVariable(v)) << b.Diagnostics();
    ASSERT_TRUE(b.GenerateGlobalVariable(a)) << b.Diagnostics();
    ASSERT_TRUE(b.GenerateFunction(func)) << b.Diagnostics();

    EXPECT_EQ(DumpModule(b.Module()), R"(OpName %1 "v"
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

TEST_F(SpirvASTPrinterTest, Switch_WithCaseAndMixedDefault) {
    // switch(a) {
    //   case 1i:
    //      v = 1i;
    //   case 2i, 3i, default:
    //      v = 2i;
    //  }

    auto* v = GlobalVar("v", ty.i32(), core::AddressSpace::kPrivate);
    auto* a = GlobalVar("a", ty.i32(), core::AddressSpace::kPrivate);

    auto* func = Func(
        "a_func", tint::Empty, ty.void_(),
        Vector{Switch(Expr("a"),                                                                 //
                      Case(CaseSelector(1_i),                                                    //
                           Block(Assign("v", 1_i))),                                             //
                      Case(Vector{CaseSelector(2_i), CaseSelector(3_i), DefaultCaseSelector()},  //
                           Block(Assign("v", 2_i)))                                              //
                      )});

    Builder& b = Build();

    ASSERT_TRUE(b.GenerateGlobalVariable(v)) << b.Diagnostics();
    ASSERT_TRUE(b.GenerateGlobalVariable(a)) << b.Diagnostics();
    ASSERT_TRUE(b.GenerateFunction(func)) << b.Diagnostics();

    EXPECT_EQ(DumpModule(b.Module()), R"(OpName %1 "v"
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

TEST_F(SpirvASTPrinterTest, Switch_WithNestedBreak) {
    // switch (a) {
    //   case 1:
    //     if (true) {
    //       break;
    //     }
    //     v = 1i;
    //   default: {}
    // }

    auto* v = GlobalVar("v", ty.i32(), core::AddressSpace::kPrivate);
    auto* a = GlobalVar("a", ty.i32(), core::AddressSpace::kPrivate);

    auto* func = Func("a_func", tint::Empty, ty.void_(),
                      Vector{
                          Switch("a",                     //
                                 Case(CaseSelector(1_i),  //
                                      Block(              //
                                          If(Expr(true), Block(create<ast::BreakStatement>())),
                                          Assign("v", 1_i))),
                                 DefaultCase()),
                      });

    Builder& b = Build();

    ASSERT_TRUE(b.GenerateGlobalVariable(v)) << b.Diagnostics();
    ASSERT_TRUE(b.GenerateGlobalVariable(a)) << b.Diagnostics();
    ASSERT_TRUE(b.GenerateFunction(func)) << b.Diagnostics();

    EXPECT_EQ(DumpModule(b.Module()), R"(OpName %1 "v"
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

TEST_F(SpirvASTPrinterTest, Switch_AllReturn) {
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

    auto* fn = Func("f", tint::Empty, ty.i32(),
                    Vector{
                        Switch(1_i,                                          //
                               Case(CaseSelector(1_i), Block(Return(1_i))),  //
                               Case(CaseSelector(2_i), Block(Return(1_i))),  //
                               DefaultCase(Block(Return(3_i)))),
                    });

    Builder& b = Build();

    EXPECT_TRUE(b.GenerateFunction(fn)) << b.Diagnostics();
    EXPECT_EQ(DumpModule(b.Module()), R"(OpName %3 "f"
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
}  // namespace tint::spirv::writer
