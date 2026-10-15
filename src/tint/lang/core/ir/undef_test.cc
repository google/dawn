// Copyright 2026 The Dawn & Tint Authors
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

#include "src/tint/lang/core/ir/undef.h"

#include "src/tint/lang/core/ir/ir_helper_test.h"

namespace tint::core::ir {
namespace {

using IR_UndefTest = IRTestHelper;
using IR_UndefDeathTest = IR_UndefTest;

TEST_F(IR_UndefDeathTest, Fail_NullType) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            Module mod;
            mod.CreateValue<Undef>(nullptr);
        },
        "internal compiler error");
}

TEST_F(IR_UndefTest, Type) {
    auto* undef = mod.CreateValue<Undef>(ty.i32());
    EXPECT_EQ(ty.i32(), undef->Type());
    EXPECT_TRUE(undef->Is<Undef>());
    EXPECT_TRUE(undef->Is<Value>());
}

TEST_F(IR_UndefTest, Clone) {
    auto* undef = mod.CreateValue<Undef>(ty.i32());
    auto* new_undef = clone_ctx.Clone(undef);

    EXPECT_NE(undef, new_undef);
    EXPECT_EQ(ty.i32(), new_undef->Type());
    EXPECT_TRUE(new_undef->Is<Undef>());
}

TEST_F(IR_UndefTest, Usages) {
    auto* undef = mod.CreateValue<Undef>(ty.i32());
    EXPECT_FALSE(undef->IsUsed());
    EXPECT_EQ(0u, undef->NumUsages());

    auto* let = b.Let("l", undef);
    EXPECT_TRUE(undef->IsUsed());
    EXPECT_EQ(1u, undef->NumUsages());
    EXPECT_TRUE(undef->HasUsage(let, 0u));
}

TEST_F(IR_UndefTest, Destroy) {
    auto* undef = mod.CreateValue<Undef>(ty.i32());
    EXPECT_TRUE(undef->Alive());
    undef->Destroy();
    EXPECT_FALSE(undef->Alive());
}

}  // namespace
}  // namespace tint::core::ir
