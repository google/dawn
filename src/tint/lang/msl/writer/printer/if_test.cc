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

#include "src/tint/lang/msl/writer/printer/helper_test.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::msl::writer {
namespace {

TEST_F(MslPrinterTest, If) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* if_ = b.If(true);
        b.Append(if_->True(), [&] { b.ExitIf(if_); });
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_;
    EXPECT_EQ(output_, MetalHeader() + R"(
void foo() {
  if (true) {
  }
}
)");
}

TEST_F(MslPrinterTest, IfWithElseIf) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* if_ = b.If(true);
        b.Append(if_->True(), [&] { b.ExitIf(if_); });
        b.Append(if_->False(), [&] {
            auto* false_ = b.If(false);
            b.Append(false_->True(), [&] { b.ExitIf(false_); });
            b.ExitIf(if_);
        });
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_;
    EXPECT_EQ(output_, MetalHeader() + R"(
void foo() {
  if (true) {
  } else {
    if (false) {
    }
  }
}
)");
}

TEST_F(MslPrinterTest, IfWithElse) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* if_ = b.If(true);
        b.Append(if_->True(), [&] { b.ExitIf(if_); });
        b.Append(if_->False(), [&] { b.Return(func); });
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_;
    EXPECT_EQ(output_, MetalHeader() + R"(
void foo() {
  if (true) {
  } else {
    return;
  }
}
)");
}

TEST_F(MslPrinterTest, IfBothBranchesReturn) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* if_ = b.If(true);
        b.Append(if_->True(), [&] { b.Return(func); });
        b.Append(if_->False(), [&] { b.Return(func); });
        b.Unreachable();
    });

    ASSERT_TRUE(Generate()) << err_ << output_;
    EXPECT_EQ(output_, MetalHeader() + R"(
void foo() {
  if (true) {
    return;
  } else {
    return;
  }
  /* unreachable */
}
)");
}

TEST_F(MslPrinterTest, IfWithSinglePhi) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* i = b.If(true);
        i->SetResults(b.InstructionResult(ty.i32()));
        b.Append(i->True(), [&] {  //
            b.ExitIf(i, 10_i);
        });
        b.Append(i->False(), [&] {  //
            b.ExitIf(i, 20_i);
        });
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_;
    EXPECT_EQ(output_, MetalHeader() + R"(
void foo() {
  int tint_symbol;
  if (true) {
    tint_symbol = 10;
  } else {
    tint_symbol = 20;
  }
}
)");
}

TEST_F(MslPrinterTest, IfWithMultiPhi) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* i = b.If(true);
        i->SetResults(b.InstructionResult(ty.i32()), b.InstructionResult(ty.bool_()));
        b.Append(i->True(), [&] {  //
            b.ExitIf(i, 10_i, true);
        });
        b.Append(i->False(), [&] {  //
            b.ExitIf(i, 20_i, false);
        });
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_;
    EXPECT_EQ(output_, MetalHeader() + R"(
void foo() {
  int tint_symbol;
  bool tint_symbol_1;
  if (true) {
    tint_symbol = 10;
    tint_symbol_1 = true;
  } else {
    tint_symbol = 20;
    tint_symbol_1 = false;
  }
}
)");
}

TEST_F(MslPrinterTest, IfWithMultiPhiReturn1) {
    auto* func = b.Function("foo", ty.i32());
    b.Append(func->Block(), [&] {
        auto* i = b.If(true);
        i->SetResults(b.InstructionResult(ty.i32()), b.InstructionResult(ty.bool_()));
        b.Append(i->True(), [&] {  //
            b.ExitIf(i, 10_i, true);
        });
        b.Append(i->False(), [&] {  //
            b.ExitIf(i, 20_i, false);
        });
        b.Return(func, i->Result(0));
    });

    ASSERT_TRUE(Generate()) << err_ << output_;
    EXPECT_EQ(output_, MetalHeader() + R"(
int foo() {
  int tint_symbol;
  bool tint_symbol_1;
  if (true) {
    tint_symbol = 10;
    tint_symbol_1 = true;
  } else {
    tint_symbol = 20;
    tint_symbol_1 = false;
  }
  return tint_symbol;
}
)");
}

TEST_F(MslPrinterTest, IfWithMultiPhiReturn2) {
    auto* func = b.Function("foo", ty.bool_());
    b.Append(func->Block(), [&] {
        auto* i = b.If(true);
        i->SetResults(b.InstructionResult(ty.i32()), b.InstructionResult(ty.bool_()));
        b.Append(i->True(), [&] {  //
            b.ExitIf(i, 10_i, true);
        });
        b.Append(i->False(), [&] {  //
            b.ExitIf(i, 20_i, false);
        });
        b.Return(func, i->Result(1));
    });

    ASSERT_TRUE(Generate()) << err_ << output_;
    EXPECT_EQ(output_, MetalHeader() + R"(
bool foo() {
  int tint_symbol;
  bool tint_symbol_1;
  if (true) {
    tint_symbol = 10;
    tint_symbol_1 = true;
  } else {
    tint_symbol = 20;
    tint_symbol_1 = false;
  }
  return tint_symbol_1;
}
)");
}

}  // namespace
}  // namespace tint::msl::writer
