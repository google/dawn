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

TEST_F(SpirvWriterTest, Swizzle_TwoElements) {
    auto* vec = b.FunctionParam("vec", ty.vec4<i32>());
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({vec});
    b.Append(func->Block(), [&] {
        auto* result = b.Swizzle(ty.vec2<i32>(), vec, {3_u, 2_u});
        b.Return(func);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpVectorShuffle %v2int %vec %vec 3 2");
}

TEST_F(SpirvWriterTest, Swizzle_ThreeElements) {
    auto* vec = b.FunctionParam("vec", ty.vec4<i32>());
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({vec});
    b.Append(func->Block(), [&] {
        auto* result = b.Swizzle(ty.vec3<i32>(), vec, {3_u, 2_u, 1_u});
        b.Return(func);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpVectorShuffle %v3int %vec %vec 3 2 1");
}

TEST_F(SpirvWriterTest, Swizzle_FourElements) {
    auto* vec = b.FunctionParam("vec", ty.vec4<i32>());
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({vec});
    b.Append(func->Block(), [&] {
        auto* result = b.Swizzle(ty.vec4<i32>(), vec, {3_u, 2_u, 1_u, 0u});
        b.Return(func);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpVectorShuffle %v4int %vec %vec 3 2 1 0");
}

TEST_F(SpirvWriterTest, Swizzle_RepeatedElements) {
    auto* vec = b.FunctionParam("vec", ty.vec2<i32>());
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({vec});
    b.Append(func->Block(), [&] {
        auto* result = b.Swizzle(ty.vec4<i32>(), vec, {1_u, 3_u, 1_u, 3_u});
        b.Return(func);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpVectorShuffle %v4int %vec %vec 1 3 1 3");
}

}  // namespace
}  // namespace tint::spirv::writer
