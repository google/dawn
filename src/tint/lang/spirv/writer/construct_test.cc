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

#include "src/tint/lang/spirv/writer/common/helper_test.h"

namespace tint::spirv::writer {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

TEST_F(SpirvWriterTest, Construct_Vector) {
    auto* func = b.Function("foo", ty.vec4<i32>());
    func->SetParams({
        b.FunctionParam("a", ty.i32()),
        b.FunctionParam("b", ty.i32()),
        b.FunctionParam("c", ty.i32()),
        b.FunctionParam("d", ty.i32()),
    });
    b.Append(func->Block(), [&] {
        auto* result = b.Construct(ty.vec4<i32>(), func->Params());
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpCompositeConstruct %v4int %a %b %c %d");
}

TEST_F(SpirvWriterTest, Construct_Matrix) {
    auto* func = b.Function("foo", ty.mat3x4<f32>());
    func->SetParams({
        b.FunctionParam("a", ty.vec4<f32>()),
        b.FunctionParam("b", ty.vec4<f32>()),
        b.FunctionParam("c", ty.vec4<f32>()),
    });
    b.Append(func->Block(), [&] {
        auto* result = b.Construct(ty.mat3x4<f32>(), func->Params());
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpCompositeConstruct %mat3v4float %a %b %c");
}

TEST_F(SpirvWriterTest, Construct_Array) {
    auto* func = b.Function("foo", ty.array<f32, 4>());
    func->SetParams({
        b.FunctionParam("a", ty.f32()),
        b.FunctionParam("b", ty.f32()),
        b.FunctionParam("c", ty.f32()),
        b.FunctionParam("d", ty.f32()),
    });
    b.Append(func->Block(), [&] {
        auto* result = b.Construct(ty.array<f32, 4>(), func->Params());
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpCompositeConstruct %_arr_float_uint_4 %a %b %c %d");
}

TEST_F(SpirvWriterTest, Construct_Struct) {
    auto* str =
        ty.Struct(mod.symbols.New("MyStruct"), {
                                                   {mod.symbols.Register("a"), ty.i32()},
                                                   {mod.symbols.Register("b"), ty.u32()},
                                                   {mod.symbols.Register("c"), ty.vec4<f32>()},
                                               });
    auto* func = b.Function("foo", str);
    func->SetParams({
        b.FunctionParam("a", ty.i32()),
        b.FunctionParam("b", ty.u32()),
        b.FunctionParam("c", ty.vec4<f32>()),
    });
    b.Append(func->Block(), [&] {
        auto* result = b.Construct(str, func->Params());
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpCompositeConstruct %MyStruct %a %b %c");
}

TEST_F(SpirvWriterTest, Construct_Scalar_Identity) {
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({b.FunctionParam("arg", ty.i32())});
    b.Append(func->Block(), [&] {
        auto* result = b.Construct(ty.i32(), func->Params()[0]);
        b.Return(func, result);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("OpReturnValue %arg");
}

TEST_F(SpirvWriterTest, Construct_Vector_Identity) {
    auto* func = b.Function("foo", ty.vec4<i32>());
    func->SetParams({b.FunctionParam("arg", ty.vec4<i32>())});
    b.Append(func->Block(), [&] {
        auto* result = b.Construct(ty.vec4<i32>(), func->Params()[0]);
        b.Return(func, result);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("OpReturnValue %arg");
}

}  // namespace
}  // namespace tint::spirv::writer
