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
