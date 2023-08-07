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

#include "src/tint/lang/core/ir/transform/builtin_polyfill.h"

#include <utility>

#include "src/tint/lang/core/ir/transform/helper_test.h"

namespace tint::ir::transform {
namespace {

using namespace tint::builtin::fluent_types;  // NOLINT
using namespace tint::number_suffixes;        // NOLINT

class IR_BuiltinPolyfillTest : public TransformTest {
  protected:
    /// Helper to build a function that calls a builtin with the given result and argument types.
    /// @param builtin the builtin to call
    /// @param result_ty the result type of the builtin call
    /// @param arg_types the arguments types for the builtin call
    void Build(builtin::Function builtin,
               const type::Type* result_ty,
               VectorRef<const type::Type*> arg_types) {
        Vector<FunctionParam*, 4> args;
        for (auto* arg_ty : arg_types) {
            args.Push(b.FunctionParam("arg", arg_ty));
        }
        auto* func = b.Function("foo", result_ty);
        func->SetParams(args);
        b.Append(func->Block(), [&] {
            auto* result = b.Call(result_ty, builtin, args);
            b.Return(func, result);
            mod.SetName(result, "result");
        });
    }
};

TEST_F(IR_BuiltinPolyfillTest, Saturate_NoPolyfill) {
    Build(builtin::Function::kSaturate, ty.f32(), Vector{ty.f32()});
    auto* src = R"(
%foo = func(%arg:f32):f32 -> %b1 {
  %b1 = block {
    %result:f32 = saturate %arg
    ret %result
  }
}
)";
    auto* expect = src;

    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.saturate = false;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, Saturate_F32) {
    Build(builtin::Function::kSaturate, ty.f32(), Vector{ty.f32()});
    auto* src = R"(
%foo = func(%arg:f32):f32 -> %b1 {
  %b1 = block {
    %result:f32 = saturate %arg
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%arg:f32):f32 -> %b1 {
  %b1 = block {
    %result:f32 = clamp %arg, 0.0f, 1.0f
    ret %result
  }
}
)";

    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.saturate = true;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, Saturate_F16) {
    Build(builtin::Function::kSaturate, ty.f16(), Vector{ty.f16()});
    auto* src = R"(
%foo = func(%arg:f16):f16 -> %b1 {
  %b1 = block {
    %result:f16 = saturate %arg
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%arg:f16):f16 -> %b1 {
  %b1 = block {
    %result:f16 = clamp %arg, 0.0h, 1.0h
    ret %result
  }
}
)";
    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.saturate = true;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, Saturate_Vec2F32) {
    Build(builtin::Function::kSaturate, ty.vec2<f32>(), Vector{ty.vec2<f32>()});
    auto* src = R"(
%foo = func(%arg:vec2<f32>):vec2<f32> -> %b1 {
  %b1 = block {
    %result:vec2<f32> = saturate %arg
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%arg:vec2<f32>):vec2<f32> -> %b1 {
  %b1 = block {
    %result:vec2<f32> = clamp %arg, vec2<f32>(0.0f), vec2<f32>(1.0f)
    ret %result
  }
}
)";

    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.saturate = true;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, Saturate_Vec4F16) {
    Build(builtin::Function::kSaturate, ty.vec4<f16>(), Vector{ty.vec4<f16>()});
    auto* src = R"(
%foo = func(%arg:vec4<f16>):vec4<f16> -> %b1 {
  %b1 = block {
    %result:vec4<f16> = saturate %arg
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%arg:vec4<f16>):vec4<f16> -> %b1 {
  %b1 = block {
    %result:vec4<f16> = clamp %arg, vec4<f16>(0.0h), vec4<f16>(1.0h)
    ret %result
  }
}
)";

    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.saturate = true;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::ir::transform
