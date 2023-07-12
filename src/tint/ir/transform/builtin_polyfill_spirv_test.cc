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

#include "src/tint/ir/transform/builtin_polyfill_spirv.h"

#include <utility>

#include "src/tint/ir/transform/test_helper.h"

namespace tint::ir::transform {
namespace {

using namespace tint::builtin::fluent_types;  // NOLINT
using namespace tint::number_suffixes;        // NOLINT

using IR_BuiltinPolyfillSpirvTest = TransformTest;

TEST_F(IR_BuiltinPolyfillSpirvTest, Dot_Vec4f) {
    auto* arg1 = b.FunctionParam("arg1", ty.vec4<f32>());
    auto* arg2 = b.FunctionParam("arg2", ty.vec4<f32>());
    auto* func = b.Function("foo", ty.f32());
    func->SetParams({arg1, arg2});

    b.With(func->Block(), [&] {
        auto* result = b.Call(ty.f32(), builtin::Function::kDot, arg1, arg2);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%arg1:vec4<f32>, %arg2:vec4<f32>):f32 -> %b1 {
  %b1 = block {
    %4:f32 = dot %arg1, %arg2
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%arg1:vec4<f32>, %arg2:vec4<f32>):f32 -> %b1 {
  %b1 = block {
    %4:f32 = spirv.dot %arg1, %arg2
    ret %4
  }
}
)";

    Run<BuiltinPolyfillSpirv>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillSpirvTest, Dot_Vec2i) {
    auto* arg1 = b.FunctionParam("arg1", ty.vec2<i32>());
    auto* arg2 = b.FunctionParam("arg2", ty.vec2<i32>());
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({arg1, arg2});

    b.With(func->Block(), [&] {
        auto* result = b.Call(ty.i32(), builtin::Function::kDot, arg1, arg2);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%arg1:vec2<i32>, %arg2:vec2<i32>):i32 -> %b1 {
  %b1 = block {
    %4:i32 = dot %arg1, %arg2
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%arg1:vec2<i32>, %arg2:vec2<i32>):i32 -> %b1 {
  %b1 = block {
    %4:i32 = access %arg1, 0u
    %5:i32 = access %arg2, 0u
    %6:i32 = mul %4, %5
    %7:i32 = access %arg1, 1u
    %8:i32 = access %arg2, 1u
    %9:i32 = mul %7, %8
    %10:i32 = add %6, %9
    ret %10
  }
}
)";

    Run<BuiltinPolyfillSpirv>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillSpirvTest, Dot_Vec4u) {
    auto* arg1 = b.FunctionParam("arg1", ty.vec4<u32>());
    auto* arg2 = b.FunctionParam("arg2", ty.vec4<u32>());
    auto* func = b.Function("foo", ty.u32());
    func->SetParams({arg1, arg2});

    b.With(func->Block(), [&] {
        auto* result = b.Call(ty.u32(), builtin::Function::kDot, arg1, arg2);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%arg1:vec4<u32>, %arg2:vec4<u32>):u32 -> %b1 {
  %b1 = block {
    %4:u32 = dot %arg1, %arg2
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%arg1:vec4<u32>, %arg2:vec4<u32>):u32 -> %b1 {
  %b1 = block {
    %4:u32 = access %arg1, 0u
    %5:u32 = access %arg2, 0u
    %6:u32 = mul %4, %5
    %7:u32 = access %arg1, 1u
    %8:u32 = access %arg2, 1u
    %9:u32 = mul %7, %8
    %10:u32 = add %6, %9
    %11:u32 = access %arg1, 2u
    %12:u32 = access %arg2, 2u
    %13:u32 = mul %11, %12
    %14:u32 = add %10, %13
    %15:u32 = access %arg1, 3u
    %16:u32 = access %arg2, 3u
    %17:u32 = mul %15, %16
    %18:u32 = add %14, %17
    ret %18
  }
}
)";

    Run<BuiltinPolyfillSpirv>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillSpirvTest, Select_ScalarCondition_ScalarOperands) {
    auto* argf = b.FunctionParam("argf", ty.i32());
    auto* argt = b.FunctionParam("argt", ty.i32());
    auto* cond = b.FunctionParam("cond", ty.bool_());
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({argf, argt, cond});

    b.With(func->Block(), [&] {
        auto* result = b.Call(ty.i32(), builtin::Function::kSelect, argf, argt, cond);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%argf:i32, %argt:i32, %cond:bool):i32 -> %b1 {
  %b1 = block {
    %5:i32 = select %argf, %argt, %cond
    ret %5
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%argf:i32, %argt:i32, %cond:bool):i32 -> %b1 {
  %b1 = block {
    %5:i32 = spirv.select %cond, %argt, %argf
    ret %5
  }
}
)";

    Run<BuiltinPolyfillSpirv>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillSpirvTest, Select_VectorCondition_VectorOperands) {
    auto* argf = b.FunctionParam("argf", ty.vec4<i32>());
    auto* argt = b.FunctionParam("argt", ty.vec4<i32>());
    auto* cond = b.FunctionParam("cond", ty.vec4<bool>());
    auto* func = b.Function("foo", ty.vec4<i32>());
    func->SetParams({argf, argt, cond});

    b.With(func->Block(), [&] {
        auto* result = b.Call(ty.vec4<i32>(), builtin::Function::kSelect, argf, argt, cond);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%argf:vec4<i32>, %argt:vec4<i32>, %cond:vec4<bool>):vec4<i32> -> %b1 {
  %b1 = block {
    %5:vec4<i32> = select %argf, %argt, %cond
    ret %5
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%argf:vec4<i32>, %argt:vec4<i32>, %cond:vec4<bool>):vec4<i32> -> %b1 {
  %b1 = block {
    %5:vec4<i32> = spirv.select %cond, %argt, %argf
    ret %5
  }
}
)";

    Run<BuiltinPolyfillSpirv>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillSpirvTest, Select_ScalarCondition_VectorOperands) {
    auto* argf = b.FunctionParam("argf", ty.vec4<i32>());
    auto* argt = b.FunctionParam("argt", ty.vec4<i32>());
    auto* cond = b.FunctionParam("cond", ty.bool_());
    auto* func = b.Function("foo", ty.vec4<i32>());
    func->SetParams({argf, argt, cond});

    b.With(func->Block(), [&] {
        auto* result = b.Call(ty.vec4<i32>(), builtin::Function::kSelect, argf, argt, cond);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%argf:vec4<i32>, %argt:vec4<i32>, %cond:bool):vec4<i32> -> %b1 {
  %b1 = block {
    %5:vec4<i32> = select %argf, %argt, %cond
    ret %5
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%argf:vec4<i32>, %argt:vec4<i32>, %cond:bool):vec4<i32> -> %b1 {
  %b1 = block {
    %5:vec4<bool> = construct %cond, %cond, %cond, %cond
    %6:vec4<i32> = spirv.select %5, %argt, %argf
    ret %6
  }
}
)";

    Run<BuiltinPolyfillSpirv>();

    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::ir::transform
