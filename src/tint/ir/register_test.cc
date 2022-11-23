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

#include "src/tint/ir/register.h"
#include "src/tint/ir/test_helper.h"

namespace tint::ir {
namespace {

using namespace tint::number_suffixes;  // NOLINT

using IR_RegisterTest = TestHelper;

TEST_F(IR_RegisterTest, f32) {
    std::stringstream str;

    Register r(1.2_f);
    EXPECT_EQ(1.2_f, r.AsF32());

    str << r;
    EXPECT_EQ("1.200000", str.str());

    EXPECT_TRUE(r.IsF32());
    EXPECT_FALSE(r.IsF16());
    EXPECT_FALSE(r.IsI32());
    EXPECT_FALSE(r.IsU32());
    EXPECT_FALSE(r.IsTemp());
    EXPECT_FALSE(r.IsVar());
    EXPECT_FALSE(r.IsBool());
}

TEST_F(IR_RegisterTest, f16) {
    std::stringstream str;

    Register r(1.1_h);
    EXPECT_EQ(1.1_h, r.AsF16());

    str << r;
    EXPECT_EQ("1.099609", str.str());

    EXPECT_FALSE(r.IsF32());
    EXPECT_TRUE(r.IsF16());
    EXPECT_FALSE(r.IsI32());
    EXPECT_FALSE(r.IsU32());
    EXPECT_FALSE(r.IsTemp());
    EXPECT_FALSE(r.IsVar());
    EXPECT_FALSE(r.IsBool());
}

TEST_F(IR_RegisterTest, i32) {
    std::stringstream str;

    Register r(1_i);
    EXPECT_EQ(1_i, r.AsI32());

    str << r;
    EXPECT_EQ("1", str.str());

    EXPECT_FALSE(r.IsF32());
    EXPECT_FALSE(r.IsF16());
    EXPECT_TRUE(r.IsI32());
    EXPECT_FALSE(r.IsU32());
    EXPECT_FALSE(r.IsTemp());
    EXPECT_FALSE(r.IsVar());
    EXPECT_FALSE(r.IsBool());
}

TEST_F(IR_RegisterTest, u32) {
    std::stringstream str;

    Register r(2_u);
    EXPECT_EQ(2_u, r.AsU32());

    str << r;
    EXPECT_EQ("2", str.str());

    EXPECT_FALSE(r.IsF32());
    EXPECT_FALSE(r.IsF16());
    EXPECT_FALSE(r.IsI32());
    EXPECT_TRUE(r.IsU32());
    EXPECT_FALSE(r.IsTemp());
    EXPECT_FALSE(r.IsVar());
    EXPECT_FALSE(r.IsBool());
}

TEST_F(IR_RegisterTest, id) {
    std::stringstream str;

    Register r(Register::Id(4));
    EXPECT_EQ(4u, r.AsId());

    str << r;
    EXPECT_EQ("%4", str.str());

    EXPECT_FALSE(r.IsF32());
    EXPECT_FALSE(r.IsF16());
    EXPECT_FALSE(r.IsI32());
    EXPECT_FALSE(r.IsU32());
    EXPECT_TRUE(r.IsTemp());
    EXPECT_FALSE(r.IsVar());
    EXPECT_FALSE(r.IsBool());
}

TEST_F(IR_RegisterTest, bool) {
    std::stringstream str;

    Register r(false);
    EXPECT_FALSE(r.AsBool());

    str << r;
    EXPECT_EQ("false", str.str());

    str.str("");
    r = Register(true);
    EXPECT_TRUE(r.AsBool());

    str << r;
    EXPECT_EQ("true", str.str());

    EXPECT_FALSE(r.IsF32());
    EXPECT_FALSE(r.IsF16());
    EXPECT_FALSE(r.IsI32());
    EXPECT_FALSE(r.IsU32());
    EXPECT_FALSE(r.IsTemp());
    EXPECT_FALSE(r.IsVar());
    EXPECT_TRUE(r.IsBool());
}

TEST_F(IR_RegisterTest, var) {
    std::stringstream str;

    Symbol s;
    Register r(s, 2);
    EXPECT_EQ(2u, r.AsVarData().id);
    EXPECT_EQ(s, r.AsVarData().sym);

    str << r;
    EXPECT_EQ("%v2", str.str());
    str.str("");

    r = Register(s, 4);
    EXPECT_EQ(4u, r.AsVarData().id);
    EXPECT_EQ(s, r.AsVarData().sym);

    str << r;
    EXPECT_EQ("%v4", str.str());

    EXPECT_FALSE(r.IsF32());
    EXPECT_FALSE(r.IsF16());
    EXPECT_FALSE(r.IsI32());
    EXPECT_FALSE(r.IsU32());
    EXPECT_FALSE(r.IsTemp());
    EXPECT_TRUE(r.IsVar());
    EXPECT_FALSE(r.IsBool());
}

TEST_F(IR_RegisterTest, uninitialized) {
    Register r;

    EXPECT_FALSE(r.IsF32());
    EXPECT_FALSE(r.IsF16());
    EXPECT_FALSE(r.IsI32());
    EXPECT_FALSE(r.IsU32());
    EXPECT_FALSE(r.IsTemp());
    EXPECT_FALSE(r.IsVar());
    EXPECT_FALSE(r.IsBool());
}

}  // namespace
}  // namespace tint::ir
