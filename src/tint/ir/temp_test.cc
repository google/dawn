// Copyright 2022 The Tint Authors.
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

#include "src/tint/ir/temp.h"
#include "src/tint/ir/test_helper.h"

namespace tint::ir {
namespace {

using namespace tint::number_suffixes;  // NOLINT

using IR_TempTest = TestHelper;

TEST_F(IR_TempTest, id) {
    auto& b = CreateEmptyBuilder();

    std::stringstream str;

    b.builder.next_temp_id = Temp::Id(4);
    auto* val = b.builder.Temp(b.builder.ir.types.Get<type::I32>());
    EXPECT_EQ(4u, val->AsId());

    val->ToString(str, b.builder.ir.symbols);
    EXPECT_EQ("%4 (i32)", str.str());
}

}  // namespace
}  // namespace tint::ir
