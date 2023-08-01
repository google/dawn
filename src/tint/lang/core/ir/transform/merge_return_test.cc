// Copyright 2023 The Tint Authors.
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

#include "src/tint/lang/core/ir/transform/merge_return.h"

#include <utility>

#include "src/tint/lang/core/ir/transform/helper_test.h"

namespace tint::ir::transform {
namespace {

using namespace tint::builtin::fluent_types;  // NOLINT
using namespace tint::number_suffixes;        // NOLINT

using IR_MergeReturnTest = TransformTest;

TEST_F(IR_MergeReturnTest, NoModify_SingleReturnInRootBlock) {
    auto* in = b.FunctionParam(ty.i32());
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({in});

    b.Append(func->Block(), [&] { b.Return(func, b.Add(ty.i32(), in, 1_i)); });

    auto* src = R"(
%foo = func(%2:i32):i32 -> %b1 {
  %b1 = block {
    %3:i32 = add %2, 1i
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    Run<MergeReturn>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_MergeReturnTest, NoModify_SingleReturnInMergeBlock) {
    auto* in = b.FunctionParam(ty.i32());
    auto* cond = b.FunctionParam(ty.bool_());
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({in});

    b.Append(func->Block(), [&] {
        auto* ifelse = b.If(cond);
        ifelse->SetResults(b.InstructionResult(ty.i32()));
        b.Append(ifelse->True(), [&] { b.ExitIf(ifelse, b.Add(ty.i32(), in, 1_i)); });
        b.Append(ifelse->False(), [&] { b.ExitIf(ifelse, b.Add(ty.i32(), in, 2_i)); });

        b.Return(func, ifelse->Result(0));
    });
    auto* src = R"(
%foo = func(%2:i32):i32 -> %b1 {
  %b1 = block {
    %3:i32 = if %4 [t: %b2, f: %b3] {  # if_1
      %b2 = block {  # true
        %5:i32 = add %2, 1i
        exit_if %5  # if_1
      }
      %b3 = block {  # false
        %6:i32 = add %2, 2i
        exit_if %6  # if_1
      }
    }
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    Run<MergeReturn>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_MergeReturnTest, NoModify_SingleReturnInNestedMergeBlock) {
    auto* in = b.FunctionParam(ty.i32());
    auto* cond = b.FunctionParam(ty.bool_());
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({in});

    b.Append(func->Block(), [&] {
        auto* swtch = b.Switch(in);
        b.Append(b.Case(swtch, {Switch::CaseSelector{}}), [&] { b.ExitSwitch(swtch); });

        auto* l = b.Loop();
        b.Append(l->Body(), [&] { b.ExitLoop(l); });

        auto* ifelse = b.If(cond);
        ifelse->SetResults(b.InstructionResult(ty.i32()));
        b.Append(ifelse->True(), [&] { b.ExitIf(ifelse, b.Add(ty.i32(), in, 1_i)); });
        b.Append(ifelse->False(), [&] { b.ExitIf(ifelse, b.Add(ty.i32(), in, 2_i)); });

        b.Return(func, ifelse->Result(0));
    });

    auto* src = R"(
%foo = func(%2:i32):i32 -> %b1 {
  %b1 = block {
    switch %2 [c: (default, %b2)] {  # switch_1
      %b2 = block {  # case
        exit_switch  # switch_1
      }
    }
    loop [b: %b3] {  # loop_1
      %b3 = block {  # body
        exit_loop  # loop_1
      }
    }
    %3:i32 = if %4 [t: %b4, f: %b5] {  # if_1
      %b4 = block {  # true
        %5:i32 = add %2, 1i
        exit_if %5  # if_1
      }
      %b5 = block {  # false
        %6:i32 = add %2, 2i
        exit_if %6  # if_1
      }
    }
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    Run<MergeReturn>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_MergeReturnTest, IfElse_OneSideReturns) {
    auto* cond = b.FunctionParam(ty.bool_());
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({cond});

    b.Append(func->Block(), [&] {
        auto* ifelse = b.If(cond);
        b.Append(ifelse->True(), [&] { b.Return(func); });
        b.Append(ifelse->False(), [&] { b.ExitIf(ifelse); });

        b.Return(func);
    });

    auto* src = R"(
%foo = func(%2:bool):void -> %b1 {
  %b1 = block {
    if %2 [t: %b2, f: %b3] {  # if_1
      %b2 = block {  # true
        ret
      }
      %b3 = block {  # false
        exit_if  # if_1
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%2:bool):void -> %b1 {
  %b1 = block {
    if %2 [t: %b2, f: %b3] {  # if_1
      %b2 = block {  # true
        exit_if  # if_1
      }
      %b3 = block {  # false
        exit_if  # if_1
      }
    }
    ret
  }
}
)";

    Run<MergeReturn>();

    EXPECT_EQ(expect, str());
}

// This is the same as the above tests, but we create the return instructions in a different order
// to make sure that creation order doesn't matter.
TEST_F(IR_MergeReturnTest, IfElse_OneSideReturns_ReturnsCreatedInDifferentOrder) {
    auto* cond = b.FunctionParam(ty.bool_());
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({cond});

    b.Append(func->Block(), [&] {
        auto* ifelse = b.If(cond);
        b.Return(func);

        b.Append(ifelse->True(), [&] { b.Return(func); });
        b.Append(ifelse->False(), [&] { b.ExitIf(ifelse); });
    });

    auto* src = R"(
%foo = func(%2:bool):void -> %b1 {
  %b1 = block {
    if %2 [t: %b2, f: %b3] {  # if_1
      %b2 = block {  # true
        ret
      }
      %b3 = block {  # false
        exit_if  # if_1
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%2:bool):void -> %b1 {
  %b1 = block {
    if %2 [t: %b2, f: %b3] {  # if_1
      %b2 = block {  # true
        exit_if  # if_1
      }
      %b3 = block {  # false
        exit_if  # if_1
      }
    }
    ret
  }
}
)";

    Run<MergeReturn>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_MergeReturnTest, IfElse_OneSideReturns_WithValue) {
    auto* cond = b.FunctionParam(ty.bool_());
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({cond});

    b.Append(func->Block(), [&] {
        auto* ifelse = b.If(cond);
        b.Append(ifelse->True(), [&] { b.Return(func, 1_i); });
        b.Append(ifelse->False(), [&] { b.ExitIf(ifelse); });

        b.Return(func, 2_i);
    });

    auto* src = R"(
%foo = func(%2:bool):i32 -> %b1 {
  %b1 = block {
    if %2 [t: %b2, f: %b3] {  # if_1
      %b2 = block {  # true
        ret 1i
      }
      %b3 = block {  # false
        exit_if  # if_1
      }
    }
    ret 2i
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%2:bool):i32 -> %b1 {
  %b1 = block {
    %return_value:ptr<function, i32, read_write> = var
    %continue_execution:ptr<function, bool, read_write> = var, true
    if %2 [t: %b2, f: %b3] {  # if_1
      %b2 = block {  # true
        store %continue_execution, false
        store %return_value, 1i
        exit_if  # if_1
      }
      %b3 = block {  # false
        exit_if  # if_1
      }
    }
    %5:bool = load %continue_execution
    if %5 [t: %b4] {  # if_2
      %b4 = block {  # true
        store %return_value, 2i
        exit_if  # if_2
      }
    }
    %6:i32 = load %return_value
    ret %6
  }
}
)";

    Run<MergeReturn>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_MergeReturnTest, IfElse_OneSideReturns_WithValue_MergeHasBasicBlockArguments) {
    auto* cond = b.FunctionParam(ty.bool_());
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({cond});

    b.Append(func->Block(), [&] {
        auto* ifelse = b.If(cond);
        ifelse->SetResults(b.InstructionResult(ty.i32()));
        b.Append(ifelse->True(), [&] { b.Return(func, 1_i); });
        b.Append(ifelse->False(), [&] { b.ExitIf(ifelse, 2_i); });

        b.Return(func, ifelse->Result(0));
    });

    auto* src = R"(
%foo = func(%2:bool):i32 -> %b1 {
  %b1 = block {
    %3:i32 = if %2 [t: %b2, f: %b3] {  # if_1
      %b2 = block {  # true
        ret 1i
      }
      %b3 = block {  # false
        exit_if 2i  # if_1
      }
    }
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%2:bool):i32 -> %b1 {
  %b1 = block {
    %return_value:ptr<function, i32, read_write> = var
    %continue_execution:ptr<function, bool, read_write> = var, true
    %5:i32 = if %2 [t: %b2, f: %b3] {  # if_1
      %b2 = block {  # true
        store %continue_execution, false
        store %return_value, 1i
        exit_if undef  # if_1
      }
      %b3 = block {  # false
        exit_if 2i  # if_1
      }
    }
    %6:bool = load %continue_execution
    if %6 [t: %b4] {  # if_2
      %b4 = block {  # true
        store %return_value, %5
        exit_if  # if_2
      }
    }
    %7:i32 = load %return_value
    ret %7
  }
}
)";

    Run<MergeReturn>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_MergeReturnTest, IfElse_OneSideReturns_WithValue_MergeHasUndefBasicBlockArguments) {
    auto* cond = b.FunctionParam(ty.bool_());
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({cond});

    b.Append(func->Block(), [&] {
        auto* ifelse = b.If(cond);
        ifelse->SetResults(b.InstructionResult(ty.i32()));
        b.Append(ifelse->True(), [&] { b.Return(func, 1_i); });
        b.Append(ifelse->False(), [&] { b.ExitIf(ifelse, nullptr); });

        b.Return(func, ifelse->Result(0));
    });

    auto* src = R"(
%foo = func(%2:bool):i32 -> %b1 {
  %b1 = block {
    %3:i32 = if %2 [t: %b2, f: %b3] {  # if_1
      %b2 = block {  # true
        ret 1i
      }
      %b3 = block {  # false
        exit_if undef  # if_1
      }
    }
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%2:bool):i32 -> %b1 {
  %b1 = block {
    %return_value:ptr<function, i32, read_write> = var
    %continue_execution:ptr<function, bool, read_write> = var, true
    %5:i32 = if %2 [t: %b2, f: %b3] {  # if_1
      %b2 = block {  # true
        store %continue_execution, false
        store %return_value, 1i
        exit_if undef  # if_1
      }
      %b3 = block {  # false
        exit_if undef  # if_1
      }
    }
    %6:bool = load %continue_execution
    if %6 [t: %b4] {  # if_2
      %b4 = block {  # true
        store %return_value, %5
        exit_if  # if_2
      }
    }
    %7:i32 = load %return_value
    ret %7
  }
}
)";

    Run<MergeReturn>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_MergeReturnTest, IfElse_BothSidesReturn) {
    auto* cond = b.FunctionParam(ty.bool_());
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({cond});

    b.Append(func->Block(), [&] {
        auto* ifelse = b.If(cond);
        b.Append(ifelse->True(), [&] { b.Return(func); });
        b.Append(ifelse->False(), [&] { b.Return(func); });

        b.Unreachable();
    });

    auto* src = R"(
%foo = func(%2:bool):void -> %b1 {
  %b1 = block {
    if %2 [t: %b2, f: %b3] {  # if_1
      %b2 = block {  # true
        ret
      }
      %b3 = block {  # false
        ret
      }
    }
    unreachable
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%2:bool):void -> %b1 {
  %b1 = block {
    if %2 [t: %b2, f: %b3] {  # if_1
      %b2 = block {  # true
        exit_if  # if_1
      }
      %b3 = block {  # false
        exit_if  # if_1
      }
    }
    ret
  }
}
)";

    Run<MergeReturn>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_MergeReturnTest, IfElse_ThenStatements) {
    auto* global = b.Var(ty.ptr<private_, i32>());
    b.RootBlock()->Append(global);

    auto* cond = b.FunctionParam(ty.bool_());
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({cond});

    b.Append(func->Block(), [&] {
        auto* ifelse = b.If(cond);
        b.Append(ifelse->True(), [&] { b.Return(func); });
        b.Append(ifelse->False(), [&] { b.ExitIf(ifelse); });

        b.Store(global, 42_i);
        b.Return(func);
    });

    auto* src = R"(
%b1 = block {  # root
  %1:ptr<private, i32, read_write> = var
}

%foo = func(%3:bool):void -> %b2 {
  %b2 = block {
    if %3 [t: %b3, f: %b4] {  # if_1
      %b3 = block {  # true
        ret
      }
      %b4 = block {  # false
        exit_if  # if_1
      }
    }
    store %1, 42i
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %1:ptr<private, i32, read_write> = var
}

%foo = func(%3:bool):void -> %b2 {
  %b2 = block {
    %continue_execution:ptr<function, bool, read_write> = var, true
    if %3 [t: %b3, f: %b4] {  # if_1
      %b3 = block {  # true
        store %continue_execution, false
        exit_if  # if_1
      }
      %b4 = block {  # false
        exit_if  # if_1
      }
    }
    %5:bool = load %continue_execution
    if %5 [t: %b5] {  # if_2
      %b5 = block {  # true
        store %1, 42i
        exit_if  # if_2
      }
    }
    ret
  }
}
)";

    Run<MergeReturn>();

    EXPECT_EQ(expect, str());
}

// This is the same as the above tests, but we create the return instructions in a different order
// to make sure that creation order doesn't matter.
TEST_F(IR_MergeReturnTest, IfElse_ThenStatements_ReturnsCreatedInDifferentOrder) {
    auto* global = b.Var(ty.ptr<private_, i32>());
    b.RootBlock()->Append(global);

    auto* cond = b.FunctionParam(ty.bool_());
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({cond});

    b.Append(func->Block(), [&] {
        auto* ifelse = b.If(cond);
        b.Store(global, 42_i);
        b.Return(func);

        b.Append(ifelse->True(), [&] { b.Return(func); });
        b.Append(ifelse->False(), [&] { b.ExitIf(ifelse); });
    });

    auto* src = R"(
%b1 = block {  # root
  %1:ptr<private, i32, read_write> = var
}

%foo = func(%3:bool):void -> %b2 {
  %b2 = block {
    if %3 [t: %b3, f: %b4] {  # if_1
      %b3 = block {  # true
        ret
      }
      %b4 = block {  # false
        exit_if  # if_1
      }
    }
    store %1, 42i
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %1:ptr<private, i32, read_write> = var
}

%foo = func(%3:bool):void -> %b2 {
  %b2 = block {
    %continue_execution:ptr<function, bool, read_write> = var, true
    if %3 [t: %b3, f: %b4] {  # if_1
      %b3 = block {  # true
        store %continue_execution, false
        exit_if  # if_1
      }
      %b4 = block {  # false
        exit_if  # if_1
      }
    }
    %5:bool = load %continue_execution
    if %5 [t: %b5] {  # if_2
      %b5 = block {  # true
        store %1, 42i
        exit_if  # if_2
      }
    }
    ret
  }
}
)";

    Run<MergeReturn>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_MergeReturnTest, IfElse_Nested) {
    auto* global = b.Var(ty.ptr<private_, i32>());
    b.RootBlock()->Append(global);

    auto* func = b.Function("foo", ty.i32());
    auto* condA = b.FunctionParam("condA", ty.bool_());
    auto* condB = b.FunctionParam("condB", ty.bool_());
    auto* condC = b.FunctionParam("condC", ty.bool_());
    func->SetParams({condA, condB, condC});

    b.Append(func->Block(), [&] {
        auto* ifelse_outer = b.If(condA);
        b.Append(ifelse_outer->True(), [&] { b.Return(func, 3_i); });
        b.Append(ifelse_outer->False(), [&] {
            auto* ifelse_middle = b.If(condB);
            b.Append(ifelse_middle->True(), [&] {
                auto* ifelse_inner = b.If(condC);
                b.Append(ifelse_inner->True(), [&] { b.Return(func, 1_i); });
                b.Append(ifelse_inner->False(), [&] { b.ExitIf(ifelse_inner); });

                b.Store(global, 1_i);
                b.Return(func, 2_i);
            });
            b.Append(ifelse_middle->False(), [&] { b.ExitIf(ifelse_middle); });
            b.Store(global, 2_i);
            b.ExitIf(ifelse_outer);
        });
        b.Store(global, 3_i);
        b.Return(func, b.Add(ty.i32(), 5_i, 6_i));
    });

    auto* src = R"(
%b1 = block {  # root
  %1:ptr<private, i32, read_write> = var
}

%foo = func(%condA:bool, %condB:bool, %condC:bool):i32 -> %b2 {
  %b2 = block {
    if %condA [t: %b3, f: %b4] {  # if_1
      %b3 = block {  # true
        ret 3i
      }
      %b4 = block {  # false
        if %condB [t: %b5, f: %b6] {  # if_2
          %b5 = block {  # true
            if %condC [t: %b7, f: %b8] {  # if_3
              %b7 = block {  # true
                ret 1i
              }
              %b8 = block {  # false
                exit_if  # if_3
              }
            }
            store %1, 1i
            ret 2i
          }
          %b6 = block {  # false
            exit_if  # if_2
          }
        }
        store %1, 2i
        exit_if  # if_1
      }
    }
    store %1, 3i
    %6:i32 = add 5i, 6i
    ret %6
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %1:ptr<private, i32, read_write> = var
}

%foo = func(%condA:bool, %condB:bool, %condC:bool):i32 -> %b2 {
  %b2 = block {
    %return_value:ptr<function, i32, read_write> = var
    %continue_execution:ptr<function, bool, read_write> = var, true
    if %condA [t: %b3, f: %b4] {  # if_1
      %b3 = block {  # true
        store %continue_execution, false
        store %return_value, 3i
        exit_if  # if_1
      }
      %b4 = block {  # false
        if %condB [t: %b5, f: %b6] {  # if_2
          %b5 = block {  # true
            if %condC [t: %b7, f: %b8] {  # if_3
              %b7 = block {  # true
                store %continue_execution, false
                store %return_value, 1i
                exit_if  # if_3
              }
              %b8 = block {  # false
                exit_if  # if_3
              }
            }
            %8:bool = load %continue_execution
            if %8 [t: %b9] {  # if_4
              %b9 = block {  # true
                store %1, 1i
                store %continue_execution, false
                store %return_value, 2i
                exit_if  # if_4
              }
            }
            exit_if  # if_2
          }
          %b6 = block {  # false
            exit_if  # if_2
          }
        }
        %9:bool = load %continue_execution
        if %9 [t: %b10] {  # if_5
          %b10 = block {  # true
            store %1, 2i
            exit_if  # if_5
          }
        }
        exit_if  # if_1
      }
    }
    %10:bool = load %continue_execution
    if %10 [t: %b11] {  # if_6
      %b11 = block {  # true
        store %1, 3i
        %11:i32 = add 5i, 6i
        store %return_value, %11
        exit_if  # if_6
      }
    }
    %12:i32 = load %return_value
    ret %12
  }
}
)";

    Run<MergeReturn>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_MergeReturnTest, IfElse_Nested_TrivialMerge) {
    auto* global = b.Var(ty.ptr<private_, i32>());
    b.RootBlock()->Append(global);

    auto* func = b.Function("foo", ty.i32());
    auto* condA = b.FunctionParam("condA", ty.bool_());
    auto* condB = b.FunctionParam("condB", ty.bool_());
    auto* condC = b.FunctionParam("condC", ty.bool_());
    func->SetParams({condA, condB, condC});

    b.Append(func->Block(), [&] {
        auto* ifelse_outer = b.If(condA);
        b.Append(ifelse_outer->True(), [&] { b.Return(func, 3_i); });
        b.Append(ifelse_outer->False(), [&] {
            auto* ifelse_middle = b.If(condB);
            b.Append(ifelse_middle->True(), [&] {
                auto* ifelse_inner = b.If(condC);
                b.Append(ifelse_inner->True(), [&] { b.Return(func, 1_i); });
                b.Append(ifelse_inner->False(), [&] { b.ExitIf(ifelse_inner); });

                b.ExitIf(ifelse_middle);
            });
            b.Append(ifelse_middle->False(), [&] { b.ExitIf(ifelse_middle); });

            b.ExitIf(ifelse_outer);
        });
        b.Return(func, 3_i);
    });
    auto* src = R"(
%b1 = block {  # root
  %1:ptr<private, i32, read_write> = var
}

%foo = func(%condA:bool, %condB:bool, %condC:bool):i32 -> %b2 {
  %b2 = block {
    if %condA [t: %b3, f: %b4] {  # if_1
      %b3 = block {  # true
        ret 3i
      }
      %b4 = block {  # false
        if %condB [t: %b5, f: %b6] {  # if_2
          %b5 = block {  # true
            if %condC [t: %b7, f: %b8] {  # if_3
              %b7 = block {  # true
                ret 1i
              }
              %b8 = block {  # false
                exit_if  # if_3
              }
            }
            exit_if  # if_2
          }
          %b6 = block {  # false
            exit_if  # if_2
          }
        }
        exit_if  # if_1
      }
    }
    ret 3i
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %1:ptr<private, i32, read_write> = var
}

%foo = func(%condA:bool, %condB:bool, %condC:bool):i32 -> %b2 {
  %b2 = block {
    %return_value:ptr<function, i32, read_write> = var
    %continue_execution:ptr<function, bool, read_write> = var, true
    if %condA [t: %b3, f: %b4] {  # if_1
      %b3 = block {  # true
        store %continue_execution, false
        store %return_value, 3i
        exit_if  # if_1
      }
      %b4 = block {  # false
        if %condB [t: %b5, f: %b6] {  # if_2
          %b5 = block {  # true
            if %condC [t: %b7, f: %b8] {  # if_3
              %b7 = block {  # true
                store %continue_execution, false
                store %return_value, 1i
                exit_if  # if_3
              }
              %b8 = block {  # false
                exit_if  # if_3
              }
            }
            exit_if  # if_2
          }
          %b6 = block {  # false
            exit_if  # if_2
          }
        }
        exit_if  # if_1
      }
    }
    %8:bool = load %continue_execution
    if %8 [t: %b9] {  # if_4
      %b9 = block {  # true
        store %return_value, 3i
        exit_if  # if_4
      }
    }
    %9:i32 = load %return_value
    ret %9
  }
}
)";

    Run<MergeReturn>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_MergeReturnTest, IfElse_Nested_WithBasicBlockArguments) {
    auto* global = b.Var(ty.ptr<private_, i32>());
    b.RootBlock()->Append(global);

    auto* func = b.Function("foo", ty.i32());
    auto* condA = b.FunctionParam("condA", ty.bool_());
    auto* condB = b.FunctionParam("condB", ty.bool_());
    auto* condC = b.FunctionParam("condC", ty.bool_());
    func->SetParams({condA, condB, condC});

    b.Append(func->Block(), [&] {
        auto* ifelse_outer = b.If(condA);
        ifelse_outer->SetResults(b.InstructionResult(ty.i32()));
        b.Append(ifelse_outer->True(), [&] { b.Return(func, 3_i); });
        b.Append(ifelse_outer->False(), [&] {
            auto* ifelse_middle = b.If(condB);
            ifelse_middle->SetResults(b.InstructionResult(ty.i32()));
            b.Append(ifelse_middle->True(), [&] {
                auto* ifelse_inner = b.If(condC);

                b.Append(ifelse_inner->True(), [&] { b.Return(func, 1_i); });
                b.Append(ifelse_inner->False(), [&] { b.ExitIf(ifelse_inner); });

                b.ExitIf(ifelse_middle, b.Add(ty.i32(), 42_i, 1_i));
            });
            b.Append(ifelse_middle->False(),
                     [&] { b.ExitIf(ifelse_middle, b.Add(ty.i32(), 43_i, 2_i)); });
            b.ExitIf(ifelse_outer, b.Add(ty.i32(), ifelse_middle->Result(0), 1_i));
        });

        b.Return(func, b.Add(ty.i32(), ifelse_outer->Result(0), 1_i));
    });

    auto* src = R"(
%b1 = block {  # root
  %1:ptr<private, i32, read_write> = var
}

%foo = func(%condA:bool, %condB:bool, %condC:bool):i32 -> %b2 {
  %b2 = block {
    %6:i32 = if %condA [t: %b3, f: %b4] {  # if_1
      %b3 = block {  # true
        ret 3i
      }
      %b4 = block {  # false
        %7:i32 = if %condB [t: %b5, f: %b6] {  # if_2
          %b5 = block {  # true
            if %condC [t: %b7, f: %b8] {  # if_3
              %b7 = block {  # true
                ret 1i
              }
              %b8 = block {  # false
                exit_if  # if_3
              }
            }
            %8:i32 = add 42i, 1i
            exit_if %8  # if_2
          }
          %b6 = block {  # false
            %9:i32 = add 43i, 2i
            exit_if %9  # if_2
          }
        }
        %10:i32 = add %7, 1i
        exit_if %10  # if_1
      }
    }
    %11:i32 = add %6, 1i
    ret %11
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %1:ptr<private, i32, read_write> = var
}

%foo = func(%condA:bool, %condB:bool, %condC:bool):i32 -> %b2 {
  %b2 = block {
    %return_value:ptr<function, i32, read_write> = var
    %continue_execution:ptr<function, bool, read_write> = var, true
    %8:i32 = if %condA [t: %b3, f: %b4] {  # if_1
      %b3 = block {  # true
        store %continue_execution, false
        store %return_value, 3i
        exit_if undef  # if_1
      }
      %b4 = block {  # false
        %9:i32 = if %condB [t: %b5, f: %b6] {  # if_2
          %b5 = block {  # true
            if %condC [t: %b7, f: %b8] {  # if_3
              %b7 = block {  # true
                store %continue_execution, false
                store %return_value, 1i
                exit_if  # if_3
              }
              %b8 = block {  # false
                exit_if  # if_3
              }
            }
            %10:bool = load %continue_execution
            %11:i32 = if %10 [t: %b9] {  # if_4
              %b9 = block {  # true
                %12:i32 = add 42i, 1i
                exit_if %12  # if_4
              }
              # implicit false block: exit_if undef
            }
            exit_if %11  # if_2
          }
          %b6 = block {  # false
            %13:i32 = add 43i, 2i
            exit_if %13  # if_2
          }
        }
        %14:bool = load %continue_execution
        %15:i32 = if %14 [t: %b10] {  # if_5
          %b10 = block {  # true
            %16:i32 = add %9, 1i
            exit_if %16  # if_5
          }
          # implicit false block: exit_if undef
        }
        exit_if %15  # if_1
      }
    }
    %17:bool = load %continue_execution
    if %17 [t: %b11] {  # if_6
      %b11 = block {  # true
        %18:i32 = add %8, 1i
        store %return_value, %18
        exit_if  # if_6
      }
    }
    %19:i32 = load %return_value
    ret %19
  }
}
)";

    Run<MergeReturn>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_MergeReturnTest, Loop_UnconditionalReturnInBody) {
    auto* func = b.Function("foo", ty.i32());

    b.Append(func->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Body(), [&] { b.Return(func, 42_i); });

        b.Unreachable();
    });
    auto* src = R"(
%foo = func():i32 -> %b1 {
  %b1 = block {
    loop [b: %b2] {  # loop_1
      %b2 = block {  # body
        ret 42i
      }
    }
    unreachable
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func():i32 -> %b1 {
  %b1 = block {
    %return_value:ptr<function, i32, read_write> = var
    loop [b: %b2] {  # loop_1
      %b2 = block {  # body
        store %return_value, 42i
        exit_loop  # loop_1
      }
    }
    %3:i32 = load %return_value
    ret %3
  }
}
)";

    Run<MergeReturn>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_MergeReturnTest, Loop_ConditionalReturnInBody) {
    auto* global = b.Var(ty.ptr<private_, i32>());
    b.RootBlock()->Append(global);

    auto* cond = b.FunctionParam(ty.bool_());
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({cond});

    b.Append(func->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Body(), [&] {
            auto* ifelse = b.If(cond);
            b.Append(ifelse->True(), [&] { b.Return(func, 42_i); });
            b.Append(ifelse->False(), [&] { b.ExitIf(ifelse); });

            b.Store(global, 2_i);
            b.Continue(loop);
        });

        b.Append(loop->Continuing(), [&] {
            b.Store(global, 1_i);
            b.BreakIf(loop, true);
        });

        b.Store(global, 3_i);
        b.Return(func, 43_i);
    });

    auto* src = R"(
%b1 = block {  # root
  %1:ptr<private, i32, read_write> = var
}

%foo = func(%3:bool):i32 -> %b2 {
  %b2 = block {
    loop [b: %b3, c: %b4] {  # loop_1
      %b3 = block {  # body
        if %3 [t: %b5, f: %b6] {  # if_1
          %b5 = block {  # true
            ret 42i
          }
          %b6 = block {  # false
            exit_if  # if_1
          }
        }
        store %1, 2i
        continue %b4
      }
      %b4 = block {  # continuing
        store %1, 1i
        break_if true %b3
      }
    }
    store %1, 3i
    ret 43i
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %1:ptr<private, i32, read_write> = var
}

%foo = func(%3:bool):i32 -> %b2 {
  %b2 = block {
    %return_value:ptr<function, i32, read_write> = var
    %continue_execution:ptr<function, bool, read_write> = var, true
    loop [b: %b3, c: %b4] {  # loop_1
      %b3 = block {  # body
        if %3 [t: %b5, f: %b6] {  # if_1
          %b5 = block {  # true
            store %continue_execution, false
            store %return_value, 42i
            exit_if  # if_1
          }
          %b6 = block {  # false
            exit_if  # if_1
          }
        }
        %6:bool = load %continue_execution
        if %6 [t: %b7] {  # if_2
          %b7 = block {  # true
            store %1, 2i
            continue %b4
          }
        }
        exit_loop  # loop_1
      }
      %b4 = block {  # continuing
        store %1, 1i
        break_if true %b3
      }
    }
    %7:bool = load %continue_execution
    if %7 [t: %b8] {  # if_3
      %b8 = block {  # true
        store %1, 3i
        store %return_value, 43i
        exit_if  # if_3
      }
    }
    %8:i32 = load %return_value
    ret %8
  }
}
)";

    Run<MergeReturn>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_MergeReturnTest, Loop_ConditionalReturnInBody_UnreachableMerge) {
    auto* global = b.Var(ty.ptr<private_, i32>());
    b.RootBlock()->Append(global);

    auto* cond = b.FunctionParam(ty.bool_());
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({cond});

    b.Append(func->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Body(), [&] {
            auto* ifelse = b.If(cond);

            b.Append(ifelse->True(), [&] { b.Return(func, 42_i); });
            b.Append(ifelse->False(), [&] { b.ExitIf(ifelse); });

            b.Store(global, 2_i);
            b.Continue(loop);
        });

        b.Append(loop->Continuing(), [&] {
            b.Store(global, 1_i);
            b.NextIteration(loop);
        });

        b.Unreachable();
    });

    auto* src = R"(
%b1 = block {  # root
  %1:ptr<private, i32, read_write> = var
}

%foo = func(%3:bool):i32 -> %b2 {
  %b2 = block {
    loop [b: %b3, c: %b4] {  # loop_1
      %b3 = block {  # body
        if %3 [t: %b5, f: %b6] {  # if_1
          %b5 = block {  # true
            ret 42i
          }
          %b6 = block {  # false
            exit_if  # if_1
          }
        }
        store %1, 2i
        continue %b4
      }
      %b4 = block {  # continuing
        store %1, 1i
        next_iteration %b3
      }
    }
    unreachable
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %1:ptr<private, i32, read_write> = var
}

%foo = func(%3:bool):i32 -> %b2 {
  %b2 = block {
    %return_value:ptr<function, i32, read_write> = var
    %continue_execution:ptr<function, bool, read_write> = var, true
    loop [b: %b3, c: %b4] {  # loop_1
      %b3 = block {  # body
        if %3 [t: %b5, f: %b6] {  # if_1
          %b5 = block {  # true
            store %continue_execution, false
            store %return_value, 42i
            exit_if  # if_1
          }
          %b6 = block {  # false
            exit_if  # if_1
          }
        }
        %6:bool = load %continue_execution
        if %6 [t: %b7] {  # if_2
          %b7 = block {  # true
            store %1, 2i
            continue %b4
          }
        }
        exit_loop  # loop_1
      }
      %b4 = block {  # continuing
        store %1, 1i
        next_iteration %b3
      }
    }
    %7:i32 = load %return_value
    ret %7
  }
}
)";

    Run<MergeReturn>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_MergeReturnTest, DISABLED_Loop_WithBasicBlockArgumentsOnMerge) {
    auto* global = b.Var(ty.ptr<private_, i32>());
    b.RootBlock()->Append(global);

    auto* cond = b.FunctionParam(ty.bool_());
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({cond});

    b.Append(func->Block(), [&] {
        auto* loop = b.Loop();
        loop->SetResults(b.InstructionResult(ty.i32()));
        b.Append(loop->Body(), [&] {
            auto* ifelse = b.If(cond);
            b.Append(ifelse->True(), [&] { b.Return(func, 42_i); });
            b.Append(ifelse->False(), [&] { b.ExitIf(ifelse); });

            b.Store(global, 2_i);
            b.Continue(loop);
        });

        b.Append(loop->Continuing(), [&] {
            b.Store(global, 1_i);
            b.BreakIf(loop, true, 4_i);
        });

        b.Store(global, 3_i);
        b.Return(func, loop->Result(0));
    });
    auto* src = R"(
%b1 = block {  # root
  %1:ptr<private, i32, read_write> = var
}

%foo = func(%3:bool):i32 -> %b2 {
  %b2 = block {
    %4:i32 = loop [b: %b3, c: %b4] {  # loop_1
      %b3 = block {  # body
        if %3 [t: %b5, f: %b6] {  # if_1
          %b5 = block {  # true
            ret 42i
          }
          %b6 = block {  # false
            exit_if  # if_1
          }
        }
        store %1, 2i
        continue %b4
      }
      %b4 = block {  # continuing
        store %1, 1i
        break_if true %b3 4i
      }
    }
    store %1, 3i
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %1:ptr<private, i32, read_write> = var
}

%foo = func(%3:bool):i32 -> %b2 {
  %b2 = block {
    %return_value:ptr<function, i32, read_write> = var
    %continue_execution:ptr<function, bool, read_write> = var, true
    %6:i32 = loop [b: %b3, c: %b4] {  # loop_1
      %b3 = block {  # body
        if %3 [t: %b5, f: %b6] {  # if_1
          %b5 = block {  # true
            store %continue_execution, false
            store %return_value, 42i
            exit_if  # if_1
          }
          %b6 = block {  # false
            exit_if  # if_1
          }
        }
        %7:bool = load %continue_execution
        if %7 [t: %b7] {  # if_2
          %b7 = block {  # true
            store %1, 2i
            continue %b4
          }
        }
        exit_loop  # loop_1
      }
      %b4 = block {  # continuing
        store %1, 1i
        break_if true %b3 4i
      }
    }
    %8:bool = load %continue_execution
    if %8 [t: %b8] {  # if_3
      %b8 = block {  # true
        store %1, 3i
        store %return_value, %6
        exit_if  # if_3
      }
    }
    %9:i32 = load %return_value
    ret %9
  }
}
)";

    Run<MergeReturn>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_MergeReturnTest, Switch_UnconditionalReturnInCase) {
    auto* cond = b.FunctionParam(ty.i32());
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({cond});

    b.Append(func->Block(), [&] {
        auto* sw = b.Switch(cond);
        b.Append(b.Case(sw, {Switch::CaseSelector{b.Constant(1_i)}}),
                 [&] { b.Return(func, 42_i); });
        b.Append(b.Case(sw, {Switch::CaseSelector{}}), [&] { b.ExitSwitch(sw); });

        b.Return(func, 0_i);
    });

    auto* src = R"(
%foo = func(%2:i32):i32 -> %b1 {
  %b1 = block {
    switch %2 [c: (1i, %b2), c: (default, %b3)] {  # switch_1
      %b2 = block {  # case
        ret 42i
      }
      %b3 = block {  # case
        exit_switch  # switch_1
      }
    }
    ret 0i
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%2:i32):i32 -> %b1 {
  %b1 = block {
    %return_value:ptr<function, i32, read_write> = var
    %continue_execution:ptr<function, bool, read_write> = var, true
    switch %2 [c: (1i, %b2), c: (default, %b3)] {  # switch_1
      %b2 = block {  # case
        store %continue_execution, false
        store %return_value, 42i
        exit_switch  # switch_1
      }
      %b3 = block {  # case
        exit_switch  # switch_1
      }
    }
    %5:bool = load %continue_execution
    if %5 [t: %b4] {  # if_1
      %b4 = block {  # true
        store %return_value, 0i
        exit_if  # if_1
      }
    }
    %6:i32 = load %return_value
    ret %6
  }
}
)";

    Run<MergeReturn>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_MergeReturnTest, Switch_ConditionalReturnInBody) {
    auto* global = b.Var(ty.ptr<private_, i32>());
    b.RootBlock()->Append(global);

    auto* cond = b.FunctionParam(ty.i32());
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({cond});

    b.Append(func->Block(), [&] {
        auto* sw = b.Switch(cond);
        b.Append(b.Case(sw, {Switch::CaseSelector{b.Constant(1_i)}}), [&] {
            auto* ifcond = b.Equal(ty.bool_(), cond, 1_i);
            auto* ifelse = b.If(ifcond);
            b.Append(ifelse->True(), [&] { b.Return(func, 42_i); });
            b.Append(ifelse->False(), [&] { b.ExitIf(ifelse); });

            b.Store(global, 2_i);
            b.ExitSwitch(sw);
        });

        b.Append(b.Case(sw, {Switch::CaseSelector{}}), [&] { b.ExitSwitch(sw); });

        b.Return(func, 0_i);
    });

    auto* src = R"(
%b1 = block {  # root
  %1:ptr<private, i32, read_write> = var
}

%foo = func(%3:i32):i32 -> %b2 {
  %b2 = block {
    switch %3 [c: (1i, %b3), c: (default, %b4)] {  # switch_1
      %b3 = block {  # case
        %4:bool = eq %3, 1i
        if %4 [t: %b5, f: %b6] {  # if_1
          %b5 = block {  # true
            ret 42i
          }
          %b6 = block {  # false
            exit_if  # if_1
          }
        }
        store %1, 2i
        exit_switch  # switch_1
      }
      %b4 = block {  # case
        exit_switch  # switch_1
      }
    }
    ret 0i
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %1:ptr<private, i32, read_write> = var
}

%foo = func(%3:i32):i32 -> %b2 {
  %b2 = block {
    %return_value:ptr<function, i32, read_write> = var
    %continue_execution:ptr<function, bool, read_write> = var, true
    switch %3 [c: (1i, %b3), c: (default, %b4)] {  # switch_1
      %b3 = block {  # case
        %6:bool = eq %3, 1i
        if %6 [t: %b5, f: %b6] {  # if_1
          %b5 = block {  # true
            store %continue_execution, false
            store %return_value, 42i
            exit_if  # if_1
          }
          %b6 = block {  # false
            exit_if  # if_1
          }
        }
        %7:bool = load %continue_execution
        if %7 [t: %b7] {  # if_2
          %b7 = block {  # true
            store %1, 2i
            exit_switch  # switch_1
          }
        }
        exit_switch  # switch_1
      }
      %b4 = block {  # case
        exit_switch  # switch_1
      }
    }
    %8:bool = load %continue_execution
    if %8 [t: %b8] {  # if_3
      %b8 = block {  # true
        store %return_value, 0i
        exit_if  # if_3
      }
    }
    %9:i32 = load %return_value
    ret %9
  }
}
)";

    Run<MergeReturn>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_MergeReturnTest, Switch_WithBasicBlockArgumentsOnMerge) {
    auto* cond = b.FunctionParam(ty.i32());
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({cond});

    b.Append(func->Block(), [&] {
        auto* sw = b.Switch(cond);
        sw->SetResults(b.InstructionResult(ty.i32()));  // NOLINT: false detection of std::tuple
        b.Append(b.Case(sw, {Switch::CaseSelector{b.Constant(1_i)}}),
                 [&] { b.Return(func, 42_i); });
        b.Append(b.Case(sw, {Switch::CaseSelector{b.Constant(2_i)}}),
                 [&] { b.Return(func, 99_i); });
        b.Append(b.Case(sw, {Switch::CaseSelector{b.Constant(3_i)}}),
                 [&] { b.ExitSwitch(sw, 1_i); });
        b.Append(b.Case(sw, {Switch::CaseSelector{}}), [&] { b.ExitSwitch(sw, 0_i); });

        b.Return(func, sw->Result(0));
    });

    auto* src = R"(
%foo = func(%2:i32):i32 -> %b1 {
  %b1 = block {
    %3:i32 = switch %2 [c: (1i, %b2), c: (2i, %b3), c: (3i, %b4), c: (default, %b5)] {  # switch_1
      %b2 = block {  # case
        ret 42i
      }
      %b3 = block {  # case
        ret 99i
      }
      %b4 = block {  # case
        exit_switch 1i  # switch_1
      }
      %b5 = block {  # case
        exit_switch 0i  # switch_1
      }
    }
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%2:i32):i32 -> %b1 {
  %b1 = block {
    %return_value:ptr<function, i32, read_write> = var
    %continue_execution:ptr<function, bool, read_write> = var, true
    %5:i32 = switch %2 [c: (1i, %b2), c: (2i, %b3), c: (3i, %b4), c: (default, %b5)] {  # switch_1
      %b2 = block {  # case
        store %continue_execution, false
        store %return_value, 42i
        exit_switch undef  # switch_1
      }
      %b3 = block {  # case
        store %continue_execution, false
        store %return_value, 99i
        exit_switch undef  # switch_1
      }
      %b4 = block {  # case
        exit_switch 1i  # switch_1
      }
      %b5 = block {  # case
        exit_switch 0i  # switch_1
      }
    }
    %6:bool = load %continue_execution
    if %6 [t: %b6] {  # if_1
      %b6 = block {  # true
        store %return_value, %5
        exit_if  # if_1
      }
    }
    %7:i32 = load %return_value
    ret %7
  }
}
)";

    Run<MergeReturn>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_MergeReturnTest, LoopIfReturnThenContinue) {
    auto* func = b.Function("foo", ty.void_());

    b.Append(func->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Body(), [&] {
            b.Append(b.If(true)->True(), [&] { b.Return(func); });
            b.Continue(loop);
        });
        b.Unreachable();
    });

    auto* src = R"(
%foo = func():void -> %b1 {
  %b1 = block {
    loop [b: %b2] {  # loop_1
      %b2 = block {  # body
        if true [t: %b3] {  # if_1
          %b3 = block {  # true
            ret
          }
        }
        continue %b4
      }
    }
    unreachable
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func():void -> %b1 {
  %b1 = block {
    %continue_execution:ptr<function, bool, read_write> = var, true
    loop [b: %b2] {  # loop_1
      %b2 = block {  # body
        if true [t: %b3] {  # if_1
          %b3 = block {  # true
            store %continue_execution, false
            exit_if  # if_1
          }
        }
        %3:bool = load %continue_execution
        if %3 [t: %b4] {  # if_2
          %b4 = block {  # true
            continue %b5
          }
        }
        exit_loop  # loop_1
      }
    }
    ret
  }
}
)";

    Run<MergeReturn>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_MergeReturnTest, NestedIfsWithReturns) {
    auto* func = b.Function("foo", ty.i32());

    b.Append(func->Block(), [&] {
        b.Append(b.If(true)->True(), [&] {
            b.Append(b.If(true)->True(), [&] { b.Return(func, 1_i); });
            b.Return(func, 2_i);
        });
        b.Return(func, 3_i);
    });

    auto* src = R"(
%foo = func():i32 -> %b1 {
  %b1 = block {
    if true [t: %b2] {  # if_1
      %b2 = block {  # true
        if true [t: %b3] {  # if_2
          %b3 = block {  # true
            ret 1i
          }
        }
        ret 2i
      }
    }
    ret 3i
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func():i32 -> %b1 {
  %b1 = block {
    %return_value:ptr<function, i32, read_write> = var
    %continue_execution:ptr<function, bool, read_write> = var, true
    if true [t: %b2] {  # if_1
      %b2 = block {  # true
        if true [t: %b3] {  # if_2
          %b3 = block {  # true
            store %continue_execution, false
            store %return_value, 1i
            exit_if  # if_2
          }
        }
        %4:bool = load %continue_execution
        if %4 [t: %b4] {  # if_3
          %b4 = block {  # true
            store %continue_execution, false
            store %return_value, 2i
            exit_if  # if_3
          }
        }
        exit_if  # if_1
      }
    }
    %5:bool = load %continue_execution
    if %5 [t: %b5] {  # if_4
      %b5 = block {  # true
        store %return_value, 3i
        exit_if  # if_4
      }
    }
    %6:i32 = load %return_value
    ret %6
  }
}
)";

    Run<MergeReturn>();

    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::ir::transform
