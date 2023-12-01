// Copyright 2023 The Dawn & Tint Authors
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

#include "src/tint/lang/core/ir/ir_helper_test.h"

#include "src/tint/lang/core/ir/binary/decode.h"
#include "src/tint/lang/core/ir/binary/encode.h"
#include "src/tint/lang/core/ir/disassembler.h"

namespace tint::core::ir::binary {
namespace {

using namespace tint::core::number_suffixes;  // NOLINT

template <typename T = testing::Test>
class IRBinaryRoundtripTestBase : public IRTestParamHelper<T> {
  public:
    std::pair<std::string, std::string> Roundtrip() {
        auto pre = Disassemble(this->mod);
        auto encoded = Encode(this->mod);
        if (!encoded) {
            return {pre, encoded.Failure().reason.str()};
        }
        auto decoded = Decode(encoded->Slice());
        if (!decoded) {
            return {pre, decoded.Failure().reason.str()};
        }
        auto post = Disassemble(decoded.Get());
        return {pre, post};
    }
};

#define RUN_TEST()                      \
    {                                   \
        auto [pre, post] = Roundtrip(); \
        EXPECT_EQ(pre, post);           \
    }                                   \
    TINT_REQUIRE_SEMICOLON

using IRBinaryRoundtripTest = IRBinaryRoundtripTestBase<>;
TEST_F(IRBinaryRoundtripTest, EmptyModule) {
    RUN_TEST();
}

////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////
TEST_F(IRBinaryRoundtripTest, Fn_i32_ret) {
    b.Function("Function", ty.i32());
    RUN_TEST();
}

using IRBinaryRoundtripTest_FnPipelineStage = IRBinaryRoundtripTestBase<Function::PipelineStage>;
TEST_P(IRBinaryRoundtripTest_FnPipelineStage, Test) {
    b.Function("Function", ty.i32(), GetParam());
    RUN_TEST();
}
INSTANTIATE_TEST_SUITE_P(,
                         IRBinaryRoundtripTest_FnPipelineStage,
                         testing::Values(Function::PipelineStage::kCompute,
                                         Function::PipelineStage::kFragment,
                                         Function::PipelineStage::kVertex));

TEST_F(IRBinaryRoundtripTest, Fn_WorkgroupSize) {
    b.Function("Function", ty.i32(), Function::PipelineStage::kCompute,
               std::array<uint32_t, 3>{1, 2, 3});
    RUN_TEST();
}

TEST_F(IRBinaryRoundtripTest, Fn_Parameters) {
    auto* fn = b.Function("Function", ty.void_());
    auto* p0 = b.FunctionParam(ty.i32());
    auto* p1 = b.FunctionParam(ty.u32());
    auto* p2 = b.FunctionParam(ty.f32());
    b.ir.SetName(p1, "p1");
    fn->SetParams({p0, p1, p2});
    RUN_TEST();
}

////////////////////////////////////////////////////////////////////////////////
// Instructions
////////////////////////////////////////////////////////////////////////////////
TEST_F(IRBinaryRoundtripTest, Return) {
    auto* fn = b.Function("Function", ty.void_());
    b.Append(fn->Block(), [&] { b.Return(fn); });
    RUN_TEST();
}

}  // namespace
}  // namespace tint::core::ir::binary
