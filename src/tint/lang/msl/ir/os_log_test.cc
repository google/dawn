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

#include "src/tint/lang/msl/ir/os_log.h"

#include "gmock/gmock.h"
#include "src/tint/lang/core/ir/ir_helper_test.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::msl::ir {
namespace {

using MslIROsLogTest = core::ir::IRTestHelper;

TEST_F(MslIROsLogTest, SetsUsage) {
    auto* fmt = b.Constant("test %u %i");
    auto* arg1 = b.Constant(1_u);
    auto* arg2 = b.Constant(2_i);

    Vector<core::ir::Value*, 3> args = {fmt, arg1, arg2};
    auto* inst = b.ir.CreateInstruction<OsLog>(b.InstructionResult(ty.void_()), args);

    EXPECT_THAT(fmt->UsagesUnsorted(), testing::UnorderedElementsAre(core::ir::Usage{inst, 0u}));
    EXPECT_THAT(arg1->UsagesUnsorted(), testing::UnorderedElementsAre(core::ir::Usage{inst, 1u}));
    EXPECT_THAT(arg2->UsagesUnsorted(), testing::UnorderedElementsAre(core::ir::Usage{inst, 2u}));
}

TEST_F(MslIROsLogTest, Result) {
    auto* fmt = b.Constant("test");
    Vector<core::ir::Value*, 1> args = {fmt};
    auto* inst = b.ir.CreateInstruction<OsLog>(b.InstructionResult(ty.void_()), args);

    EXPECT_EQ(inst->Results().Length(), 1u);
    EXPECT_TRUE(inst->Result()->Is<core::ir::InstructionResult>());
    EXPECT_EQ(inst, inst->Result()->Instruction());
    EXPECT_EQ(ty.void_(), inst->Result()->Type());
}

TEST_F(MslIROsLogTest, Clone) {
    auto* fmt = b.Constant("test %u");
    auto* arg = b.Constant(42_u);

    Vector<core::ir::Value*, 2> args = {fmt, arg};
    auto* inst = b.ir.CreateInstruction<OsLog>(b.InstructionResult(ty.void_()), args);

    auto* cloned = clone_ctx.Clone(inst);

    EXPECT_NE(inst, cloned);
    EXPECT_NE(inst->Result(), cloned->Result());
    EXPECT_EQ(ty.void_(), cloned->Result()->Type());

    ASSERT_EQ(2u, cloned->Args().size());
    EXPECT_EQ(inst->Args()[0], cloned->Args()[0]);
    EXPECT_EQ(inst->Args()[1], cloned->Args()[1]);
}

}  // namespace
}  // namespace tint::msl::ir
