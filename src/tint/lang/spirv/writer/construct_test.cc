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

#include "src/tint/lang/spirv/writer/common/helper_test.h"

namespace tint::spirv::writer {
namespace {

using namespace tint::core::fluent_types;  // NOLINT
using namespace tint::number_suffixes;     // NOLINT

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
