
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
#include "src/tint/lang/wgsl/ast/call_statement.h"

using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::spirv::writer {
namespace {

using SpirvASTPrinterTest = TestHelper;

TEST_F(SpirvASTPrinterTest, Expression_Call) {
    auto* a_func = Func("a_func",
                        Vector{
                            Param("a", ty.f32()),
                            Param("b", ty.f32()),
                        },
                        ty.f32(), Vector{Return(Add("a", "b"))});
    auto* func =
        Func("main", tint::Empty, ty.void_(), Vector{Assign(Phony(), Call("a_func", 1_f, 1_f))});

    Builder& b = Build();

    ASSERT_TRUE(b.GenerateFunction(a_func)) << b.Diagnostics();
    ASSERT_TRUE(b.GenerateFunction(func)) << b.Diagnostics();

    EXPECT_EQ(DumpModule(b.Module()), R"(OpName %3 "a_func"
OpName %4 "a"
OpName %5 "b"
OpName %10 "main"
%2 = OpTypeFloat 32
%1 = OpTypeFunction %2 %2 %2
%9 = OpTypeVoid
%8 = OpTypeFunction %9
%13 = OpConstant %2 1
%3 = OpFunction %2 None %1
%4 = OpFunctionParameter %2
%5 = OpFunctionParameter %2
%6 = OpLabel
%7 = OpFAdd %2 %4 %5
OpReturnValue %7
OpFunctionEnd
%10 = OpFunction %9 None %8
%11 = OpLabel
%12 = OpFunctionCall %2 %3 %13 %13
OpReturn
OpFunctionEnd
)");
}

TEST_F(SpirvASTPrinterTest, Statement_Call) {
    auto* a_func = Func("a_func",
                        Vector{
                            Param("a", ty.f32()),
                            Param("b", ty.f32()),
                        },
                        ty.f32(), Vector{Return(Add("a", "b"))});

    auto* func = Func("main", tint::Empty, ty.void_(), Vector{CallStmt(Call("a_func", 1_f, 1_f))});

    Builder& b = Build();

    ASSERT_TRUE(b.GenerateFunction(a_func)) << b.Diagnostics();
    ASSERT_TRUE(b.GenerateFunction(func)) << b.Diagnostics();

    EXPECT_EQ(DumpModule(b.Module()), R"(OpName %3 "a_func"
OpName %4 "a"
OpName %5 "b"
OpName %10 "main"
%2 = OpTypeFloat 32
%1 = OpTypeFunction %2 %2 %2
%9 = OpTypeVoid
%8 = OpTypeFunction %9
%13 = OpConstant %2 1
%3 = OpFunction %2 None %1
%4 = OpFunctionParameter %2
%5 = OpFunctionParameter %2
%6 = OpLabel
%7 = OpFAdd %2 %4 %5
OpReturnValue %7
OpFunctionEnd
%10 = OpFunction %9 None %8
%11 = OpLabel
%12 = OpFunctionCall %2 %3 %13 %13
OpReturn
OpFunctionEnd
)");
}

}  // namespace
}  // namespace tint::spirv::writer
