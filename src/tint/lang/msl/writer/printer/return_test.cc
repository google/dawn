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

TEST_F(MslPrinterTest, Return) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* if_ = b.If(true);
        b.Append(if_->True(), [&] { b.Return(func); });
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_;
    EXPECT_EQ(output_, MetalHeader() + R"(
void foo() {
  if (true) {
    return;
  }
}
)");
}

TEST_F(MslPrinterTest, ReturnAtEndOfVoidDropped) {
    auto* func = b.Function("foo", ty.void_());
    func->Block()->Append(b.Return(func));

    ASSERT_TRUE(Generate()) << err_ << output_;
    EXPECT_EQ(output_, MetalHeader() + R"(
void foo() {
}
)");
}

TEST_F(MslPrinterTest, ReturnWithValue) {
    auto* func = b.Function("foo", ty.i32());
    func->Block()->Append(b.Return(func, 123_i));

    ASSERT_TRUE(Generate()) << err_ << output_;
    EXPECT_EQ(output_, MetalHeader() + R"(
int foo() {
  return 123;
}
)");
}

}  // namespace
}  // namespace tint::msl::writer
