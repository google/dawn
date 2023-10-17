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

#include "src/tint/lang/core/ir/transform/builtin_polyfill.h"

#include <utility>

#include "src/tint/lang/core/ir/transform/helper_test.h"
#include "src/tint/lang/core/type/sampled_texture.h"

namespace tint::core::ir::transform {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

class IR_BuiltinPolyfillTest : public TransformTest {
  protected:
    /// Helper to build a function that calls a builtin with the given result and argument types.
    /// @param builtin the builtin to call
    /// @param result_ty the result type of the builtin call
    /// @param arg_types the arguments types for the builtin call
    void Build(core::BuiltinFn builtin,
               const core::type::Type* result_ty,
               VectorRef<const core::type::Type*> arg_types) {
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
    Build(core::BuiltinFn::kSaturate, ty.f32(), Vector{ty.f32()});
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
    Build(core::BuiltinFn::kSaturate, ty.f32(), Vector{ty.f32()});
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
    Build(core::BuiltinFn::kSaturate, ty.f16(), Vector{ty.f16()});
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
    Build(core::BuiltinFn::kSaturate, ty.vec2<f32>(), Vector{ty.vec2<f32>()});
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
    Build(core::BuiltinFn::kSaturate, ty.vec4<f16>(), Vector{ty.vec4<f16>()});
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

TEST_F(IR_BuiltinPolyfillTest, CountLeadingZeros_NoPolyfill) {
    Build(core::BuiltinFn::kCountLeadingZeros, ty.u32(), Vector{ty.u32()});
    auto* src = R"(
%foo = func(%arg:u32):u32 -> %b1 {
  %b1 = block {
    %result:u32 = countLeadingZeros %arg
    ret %result
  }
}
)";
    auto* expect = src;

    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.count_leading_zeros = false;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, CountLeadingZeros_U32) {
    Build(core::BuiltinFn::kCountLeadingZeros, ty.u32(), Vector{ty.u32()});
    auto* src = R"(
%foo = func(%arg:u32):u32 -> %b1 {
  %b1 = block {
    %result:u32 = countLeadingZeros %arg
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%arg:u32):u32 -> %b1 {
  %b1 = block {
    %3:bool = lte %arg, 65535u
    %4:u32 = select 0u, 16u, %3
    %5:u32 = shiftl %arg, %4
    %6:bool = lte %5, 16777215u
    %7:u32 = select 0u, 8u, %6
    %8:u32 = shiftl %5, %7
    %9:bool = lte %8, 268435455u
    %10:u32 = select 0u, 4u, %9
    %11:u32 = shiftl %8, %10
    %12:bool = lte %11, 1073741823u
    %13:u32 = select 0u, 2u, %12
    %14:u32 = shiftl %11, %13
    %15:bool = lte %14, 2147483647u
    %16:u32 = select 0u, 1u, %15
    %17:bool = eq %14, 0u
    %18:u32 = select 0u, 1u, %17
    %19:u32 = or %16, %18
    %20:u32 = or %13, %19
    %21:u32 = or %10, %20
    %22:u32 = or %7, %21
    %23:u32 = or %4, %22
    %result:u32 = add %23, %18
    ret %result
  }
}
)";

    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.count_leading_zeros = true;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, CountLeadingZeros_I32) {
    Build(core::BuiltinFn::kCountLeadingZeros, ty.i32(), Vector{ty.i32()});
    auto* src = R"(
%foo = func(%arg:i32):i32 -> %b1 {
  %b1 = block {
    %result:i32 = countLeadingZeros %arg
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%arg:i32):i32 -> %b1 {
  %b1 = block {
    %3:u32 = bitcast %arg
    %4:bool = lte %3, 65535u
    %5:u32 = select 0u, 16u, %4
    %6:u32 = shiftl %3, %5
    %7:bool = lte %6, 16777215u
    %8:u32 = select 0u, 8u, %7
    %9:u32 = shiftl %6, %8
    %10:bool = lte %9, 268435455u
    %11:u32 = select 0u, 4u, %10
    %12:u32 = shiftl %9, %11
    %13:bool = lte %12, 1073741823u
    %14:u32 = select 0u, 2u, %13
    %15:u32 = shiftl %12, %14
    %16:bool = lte %15, 2147483647u
    %17:u32 = select 0u, 1u, %16
    %18:bool = eq %15, 0u
    %19:u32 = select 0u, 1u, %18
    %20:u32 = or %17, %19
    %21:u32 = or %14, %20
    %22:u32 = or %11, %21
    %23:u32 = or %8, %22
    %24:u32 = or %5, %23
    %25:u32 = add %24, %19
    %result:i32 = bitcast %25
    ret %result
  }
}
)";

    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.count_leading_zeros = true;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, CountLeadingZeros_Vec2U32) {
    Build(core::BuiltinFn::kCountLeadingZeros, ty.vec2<u32>(), Vector{ty.vec2<u32>()});
    auto* src = R"(
%foo = func(%arg:vec2<u32>):vec2<u32> -> %b1 {
  %b1 = block {
    %result:vec2<u32> = countLeadingZeros %arg
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%arg:vec2<u32>):vec2<u32> -> %b1 {
  %b1 = block {
    %3:vec2<bool> = lte %arg, vec2<u32>(65535u)
    %4:vec2<u32> = select vec2<u32>(0u), vec2<u32>(16u), %3
    %5:vec2<u32> = shiftl %arg, %4
    %6:vec2<bool> = lte %5, vec2<u32>(16777215u)
    %7:vec2<u32> = select vec2<u32>(0u), vec2<u32>(8u), %6
    %8:vec2<u32> = shiftl %5, %7
    %9:vec2<bool> = lte %8, vec2<u32>(268435455u)
    %10:vec2<u32> = select vec2<u32>(0u), vec2<u32>(4u), %9
    %11:vec2<u32> = shiftl %8, %10
    %12:vec2<bool> = lte %11, vec2<u32>(1073741823u)
    %13:vec2<u32> = select vec2<u32>(0u), vec2<u32>(2u), %12
    %14:vec2<u32> = shiftl %11, %13
    %15:vec2<bool> = lte %14, vec2<u32>(2147483647u)
    %16:vec2<u32> = select vec2<u32>(0u), vec2<u32>(1u), %15
    %17:vec2<bool> = eq %14, vec2<u32>(0u)
    %18:vec2<u32> = select vec2<u32>(0u), vec2<u32>(1u), %17
    %19:vec2<u32> = or %16, %18
    %20:vec2<u32> = or %13, %19
    %21:vec2<u32> = or %10, %20
    %22:vec2<u32> = or %7, %21
    %23:vec2<u32> = or %4, %22
    %result:vec2<u32> = add %23, %18
    ret %result
  }
}
)";

    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.count_leading_zeros = true;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, CountLeadingZeros_Vec4I32) {
    Build(core::BuiltinFn::kCountLeadingZeros, ty.vec4<i32>(), Vector{ty.vec4<i32>()});
    auto* src = R"(
%foo = func(%arg:vec4<i32>):vec4<i32> -> %b1 {
  %b1 = block {
    %result:vec4<i32> = countLeadingZeros %arg
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%arg:vec4<i32>):vec4<i32> -> %b1 {
  %b1 = block {
    %3:vec4<u32> = bitcast %arg
    %4:vec4<bool> = lte %3, vec4<u32>(65535u)
    %5:vec4<u32> = select vec4<u32>(0u), vec4<u32>(16u), %4
    %6:vec4<u32> = shiftl %3, %5
    %7:vec4<bool> = lte %6, vec4<u32>(16777215u)
    %8:vec4<u32> = select vec4<u32>(0u), vec4<u32>(8u), %7
    %9:vec4<u32> = shiftl %6, %8
    %10:vec4<bool> = lte %9, vec4<u32>(268435455u)
    %11:vec4<u32> = select vec4<u32>(0u), vec4<u32>(4u), %10
    %12:vec4<u32> = shiftl %9, %11
    %13:vec4<bool> = lte %12, vec4<u32>(1073741823u)
    %14:vec4<u32> = select vec4<u32>(0u), vec4<u32>(2u), %13
    %15:vec4<u32> = shiftl %12, %14
    %16:vec4<bool> = lte %15, vec4<u32>(2147483647u)
    %17:vec4<u32> = select vec4<u32>(0u), vec4<u32>(1u), %16
    %18:vec4<bool> = eq %15, vec4<u32>(0u)
    %19:vec4<u32> = select vec4<u32>(0u), vec4<u32>(1u), %18
    %20:vec4<u32> = or %17, %19
    %21:vec4<u32> = or %14, %20
    %22:vec4<u32> = or %11, %21
    %23:vec4<u32> = or %8, %22
    %24:vec4<u32> = or %5, %23
    %25:vec4<u32> = add %24, %19
    %result:vec4<i32> = bitcast %25
    ret %result
  }
}
)";

    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.count_leading_zeros = true;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, CountTrailingZeros_NoPolyfill) {
    Build(core::BuiltinFn::kCountTrailingZeros, ty.u32(), Vector{ty.u32()});
    auto* src = R"(
%foo = func(%arg:u32):u32 -> %b1 {
  %b1 = block {
    %result:u32 = countTrailingZeros %arg
    ret %result
  }
}
)";
    auto* expect = src;

    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.count_trailing_zeros = false;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, CountTrailingZeros_U32) {
    Build(core::BuiltinFn::kCountTrailingZeros, ty.u32(), Vector{ty.u32()});
    auto* src = R"(
%foo = func(%arg:u32):u32 -> %b1 {
  %b1 = block {
    %result:u32 = countTrailingZeros %arg
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%arg:u32):u32 -> %b1 {
  %b1 = block {
    %3:u32 = and %arg, 65535u
    %4:bool = eq %3, 0u
    %5:u32 = select 0u, 16u, %4
    %6:u32 = shiftr %arg, %5
    %7:u32 = and %6, 255u
    %8:bool = eq %7, 0u
    %9:u32 = select 0u, 8u, %8
    %10:u32 = shiftr %6, %9
    %11:u32 = and %10, 15u
    %12:bool = eq %11, 0u
    %13:u32 = select 0u, 4u, %12
    %14:u32 = shiftr %10, %13
    %15:u32 = and %14, 3u
    %16:bool = eq %15, 0u
    %17:u32 = select 0u, 2u, %16
    %18:u32 = shiftr %14, %17
    %19:u32 = and %18, 1u
    %20:bool = eq %19, 0u
    %21:u32 = select 0u, 1u, %20
    %22:bool = eq %18, 0u
    %23:u32 = select 0u, 1u, %22
    %24:u32 = or %17, %21
    %25:u32 = or %13, %24
    %26:u32 = or %9, %25
    %27:u32 = or %5, %26
    %result:u32 = add %27, %23
    ret %result
  }
}
)";

    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.count_trailing_zeros = true;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, CountTrailingZeros_I32) {
    Build(core::BuiltinFn::kCountTrailingZeros, ty.i32(), Vector{ty.i32()});
    auto* src = R"(
%foo = func(%arg:i32):i32 -> %b1 {
  %b1 = block {
    %result:i32 = countTrailingZeros %arg
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%arg:i32):i32 -> %b1 {
  %b1 = block {
    %3:u32 = bitcast %arg
    %4:u32 = and %3, 65535u
    %5:bool = eq %4, 0u
    %6:u32 = select 0u, 16u, %5
    %7:u32 = shiftr %3, %6
    %8:u32 = and %7, 255u
    %9:bool = eq %8, 0u
    %10:u32 = select 0u, 8u, %9
    %11:u32 = shiftr %7, %10
    %12:u32 = and %11, 15u
    %13:bool = eq %12, 0u
    %14:u32 = select 0u, 4u, %13
    %15:u32 = shiftr %11, %14
    %16:u32 = and %15, 3u
    %17:bool = eq %16, 0u
    %18:u32 = select 0u, 2u, %17
    %19:u32 = shiftr %15, %18
    %20:u32 = and %19, 1u
    %21:bool = eq %20, 0u
    %22:u32 = select 0u, 1u, %21
    %23:bool = eq %19, 0u
    %24:u32 = select 0u, 1u, %23
    %25:u32 = or %18, %22
    %26:u32 = or %14, %25
    %27:u32 = or %10, %26
    %28:u32 = or %6, %27
    %29:u32 = add %28, %24
    %result:i32 = bitcast %29
    ret %result
  }
}
)";

    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.count_trailing_zeros = true;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, CountTrailingZeros_Vec2U32) {
    Build(core::BuiltinFn::kCountTrailingZeros, ty.vec2<u32>(), Vector{ty.vec2<u32>()});
    auto* src = R"(
%foo = func(%arg:vec2<u32>):vec2<u32> -> %b1 {
  %b1 = block {
    %result:vec2<u32> = countTrailingZeros %arg
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%arg:vec2<u32>):vec2<u32> -> %b1 {
  %b1 = block {
    %3:vec2<u32> = and %arg, vec2<u32>(65535u)
    %4:vec2<bool> = eq %3, vec2<u32>(0u)
    %5:vec2<u32> = select vec2<u32>(0u), vec2<u32>(16u), %4
    %6:vec2<u32> = shiftr %arg, %5
    %7:vec2<u32> = and %6, vec2<u32>(255u)
    %8:vec2<bool> = eq %7, vec2<u32>(0u)
    %9:vec2<u32> = select vec2<u32>(0u), vec2<u32>(8u), %8
    %10:vec2<u32> = shiftr %6, %9
    %11:vec2<u32> = and %10, vec2<u32>(15u)
    %12:vec2<bool> = eq %11, vec2<u32>(0u)
    %13:vec2<u32> = select vec2<u32>(0u), vec2<u32>(4u), %12
    %14:vec2<u32> = shiftr %10, %13
    %15:vec2<u32> = and %14, vec2<u32>(3u)
    %16:vec2<bool> = eq %15, vec2<u32>(0u)
    %17:vec2<u32> = select vec2<u32>(0u), vec2<u32>(2u), %16
    %18:vec2<u32> = shiftr %14, %17
    %19:vec2<u32> = and %18, vec2<u32>(1u)
    %20:vec2<bool> = eq %19, vec2<u32>(0u)
    %21:vec2<u32> = select vec2<u32>(0u), vec2<u32>(1u), %20
    %22:vec2<bool> = eq %18, vec2<u32>(0u)
    %23:vec2<u32> = select vec2<u32>(0u), vec2<u32>(1u), %22
    %24:vec2<u32> = or %17, %21
    %25:vec2<u32> = or %13, %24
    %26:vec2<u32> = or %9, %25
    %27:vec2<u32> = or %5, %26
    %result:vec2<u32> = add %27, %23
    ret %result
  }
}
)";

    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.count_trailing_zeros = true;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, CountTrailingZeros_Vec4I32) {
    Build(core::BuiltinFn::kCountTrailingZeros, ty.vec4<i32>(), Vector{ty.vec4<i32>()});
    auto* src = R"(
%foo = func(%arg:vec4<i32>):vec4<i32> -> %b1 {
  %b1 = block {
    %result:vec4<i32> = countTrailingZeros %arg
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%arg:vec4<i32>):vec4<i32> -> %b1 {
  %b1 = block {
    %result:vec4<i32> = countTrailingZeros %arg
    ret %result
  }
}
)";

    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.first_trailing_bit = true;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, ExtractBits_NoPolyfill) {
    Build(core::BuiltinFn::kExtractBits, ty.u32(), Vector{ty.u32(), ty.u32(), ty.u32()});
    auto* src = R"(
%foo = func(%arg:u32, %arg_1:u32, %arg_2:u32):u32 -> %b1 {  # %arg_1: 'arg', %arg_2: 'arg'
  %b1 = block {
    %result:u32 = extractBits %arg, %arg_1, %arg_2
    ret %result
  }
}
)";
    auto* expect = src;

    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.extract_bits = BuiltinPolyfillLevel::kNone;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, ExtractBits_ClampArgs_U32) {
    Build(core::BuiltinFn::kExtractBits, ty.u32(), Vector{ty.u32(), ty.u32(), ty.u32()});
    auto* src = R"(
%foo = func(%arg:u32, %arg_1:u32, %arg_2:u32):u32 -> %b1 {  # %arg_1: 'arg', %arg_2: 'arg'
  %b1 = block {
    %result:u32 = extractBits %arg, %arg_1, %arg_2
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%arg:u32, %arg_1:u32, %arg_2:u32):u32 -> %b1 {  # %arg_1: 'arg', %arg_2: 'arg'
  %b1 = block {
    %5:u32 = min %arg_1, 32u
    %6:u32 = sub 32u, %5
    %7:u32 = min %arg_2, %6
    %result:u32 = extractBits %arg, %5, %7
    ret %result
  }
}
)";

    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.extract_bits = BuiltinPolyfillLevel::kClampOrRangeCheck;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, ExtractBits_ClampArgs_I32) {
    Build(core::BuiltinFn::kExtractBits, ty.i32(), Vector{ty.i32(), ty.u32(), ty.u32()});
    auto* src = R"(
%foo = func(%arg:i32, %arg_1:u32, %arg_2:u32):i32 -> %b1 {  # %arg_1: 'arg', %arg_2: 'arg'
  %b1 = block {
    %result:i32 = extractBits %arg, %arg_1, %arg_2
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%arg:i32, %arg_1:u32, %arg_2:u32):i32 -> %b1 {  # %arg_1: 'arg', %arg_2: 'arg'
  %b1 = block {
    %5:u32 = min %arg_1, 32u
    %6:u32 = sub 32u, %5
    %7:u32 = min %arg_2, %6
    %result:i32 = extractBits %arg, %5, %7
    ret %result
  }
}
)";

    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.extract_bits = BuiltinPolyfillLevel::kClampOrRangeCheck;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, ExtractBits_ClampArgs_Vec2U32) {
    Build(core::BuiltinFn::kExtractBits, ty.vec2<u32>(),
          Vector{ty.vec2<u32>(), ty.u32(), ty.u32()});
    auto* src = R"(
%foo = func(%arg:vec2<u32>, %arg_1:u32, %arg_2:u32):vec2<u32> -> %b1 {  # %arg_1: 'arg', %arg_2: 'arg'
  %b1 = block {
    %result:vec2<u32> = extractBits %arg, %arg_1, %arg_2
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%arg:vec2<u32>, %arg_1:u32, %arg_2:u32):vec2<u32> -> %b1 {  # %arg_1: 'arg', %arg_2: 'arg'
  %b1 = block {
    %5:u32 = min %arg_1, 32u
    %6:u32 = sub 32u, %5
    %7:u32 = min %arg_2, %6
    %result:vec2<u32> = extractBits %arg, %5, %7
    ret %result
  }
}
)";

    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.extract_bits = BuiltinPolyfillLevel::kClampOrRangeCheck;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, ExtractBits_ClampArgs_Vec4I32) {
    Build(core::BuiltinFn::kExtractBits, ty.vec4<i32>(),
          Vector{ty.vec4<i32>(), ty.u32(), ty.u32()});
    auto* src = R"(
%foo = func(%arg:vec4<i32>, %arg_1:u32, %arg_2:u32):vec4<i32> -> %b1 {  # %arg_1: 'arg', %arg_2: 'arg'
  %b1 = block {
    %result:vec4<i32> = extractBits %arg, %arg_1, %arg_2
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%arg:vec4<i32>, %arg_1:u32, %arg_2:u32):vec4<i32> -> %b1 {  # %arg_1: 'arg', %arg_2: 'arg'
  %b1 = block {
    %5:u32 = min %arg_1, 32u
    %6:u32 = sub 32u, %5
    %7:u32 = min %arg_2, %6
    %result:vec4<i32> = extractBits %arg, %5, %7
    ret %result
  }
}
)";

    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.extract_bits = BuiltinPolyfillLevel::kClampOrRangeCheck;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, FirstLeadingBit_NoPolyfill) {
    Build(core::BuiltinFn::kFirstLeadingBit, ty.u32(), Vector{ty.u32()});
    auto* src = R"(
%foo = func(%arg:u32):u32 -> %b1 {
  %b1 = block {
    %result:u32 = firstLeadingBit %arg
    ret %result
  }
}
)";
    auto* expect = src;

    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.first_leading_bit = false;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, FirstLeadingBit_U32) {
    Build(core::BuiltinFn::kFirstLeadingBit, ty.u32(), Vector{ty.u32()});
    auto* src = R"(
%foo = func(%arg:u32):u32 -> %b1 {
  %b1 = block {
    %result:u32 = firstLeadingBit %arg
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%arg:u32):u32 -> %b1 {
  %b1 = block {
    %3:u32 = and %arg, 4294901760u
    %4:bool = eq %3, 0u
    %5:u32 = select 16u, 0u, %4
    %6:u32 = shiftr %arg, %5
    %7:u32 = and %6, 65280u
    %8:bool = eq %7, 0u
    %9:u32 = select 8u, 0u, %8
    %10:u32 = shiftr %6, %9
    %11:u32 = and %10, 240u
    %12:bool = eq %11, 0u
    %13:u32 = select 4u, 0u, %12
    %14:u32 = shiftr %10, %13
    %15:u32 = and %14, 12u
    %16:bool = eq %15, 0u
    %17:u32 = select 2u, 0u, %16
    %18:u32 = shiftr %14, %17
    %19:u32 = and %18, 2u
    %20:bool = eq %19, 0u
    %21:u32 = select 1u, 0u, %20
    %22:u32 = or %17, %21
    %23:u32 = or %13, %22
    %24:u32 = or %9, %23
    %25:u32 = or %5, %24
    %26:bool = eq %18, 0u
    %result:u32 = select %25, 4294967295u, %26
    ret %result
  }
}
)";

    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.first_leading_bit = true;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, FirstLeadingBit_I32) {
    Build(core::BuiltinFn::kFirstLeadingBit, ty.i32(), Vector{ty.i32()});
    auto* src = R"(
%foo = func(%arg:i32):i32 -> %b1 {
  %b1 = block {
    %result:i32 = firstLeadingBit %arg
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%arg:i32):i32 -> %b1 {
  %b1 = block {
    %3:u32 = bitcast %arg
    %4:u32 = complement %3
    %5:bool = lt %3, 2147483648u
    %6:u32 = select %4, %3, %5
    %7:u32 = and %6, 4294901760u
    %8:bool = eq %7, 0u
    %9:u32 = select 16u, 0u, %8
    %10:u32 = shiftr %6, %9
    %11:u32 = and %10, 65280u
    %12:bool = eq %11, 0u
    %13:u32 = select 8u, 0u, %12
    %14:u32 = shiftr %10, %13
    %15:u32 = and %14, 240u
    %16:bool = eq %15, 0u
    %17:u32 = select 4u, 0u, %16
    %18:u32 = shiftr %14, %17
    %19:u32 = and %18, 12u
    %20:bool = eq %19, 0u
    %21:u32 = select 2u, 0u, %20
    %22:u32 = shiftr %18, %21
    %23:u32 = and %22, 2u
    %24:bool = eq %23, 0u
    %25:u32 = select 1u, 0u, %24
    %26:u32 = or %21, %25
    %27:u32 = or %17, %26
    %28:u32 = or %13, %27
    %29:u32 = or %9, %28
    %30:bool = eq %22, 0u
    %31:u32 = select %29, 4294967295u, %30
    %result:i32 = bitcast %31
    ret %result
  }
}
)";

    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.first_leading_bit = true;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, FirstLeadingBit_Vec2U32) {
    Build(core::BuiltinFn::kFirstLeadingBit, ty.vec2<u32>(), Vector{ty.vec2<u32>()});
    auto* src = R"(
%foo = func(%arg:vec2<u32>):vec2<u32> -> %b1 {
  %b1 = block {
    %result:vec2<u32> = firstLeadingBit %arg
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%arg:vec2<u32>):vec2<u32> -> %b1 {
  %b1 = block {
    %3:vec2<u32> = and %arg, vec2<u32>(4294901760u)
    %4:vec2<bool> = eq %3, vec2<u32>(0u)
    %5:vec2<u32> = select vec2<u32>(16u), vec2<u32>(0u), %4
    %6:vec2<u32> = shiftr %arg, %5
    %7:vec2<u32> = and %6, vec2<u32>(65280u)
    %8:vec2<bool> = eq %7, vec2<u32>(0u)
    %9:vec2<u32> = select vec2<u32>(8u), vec2<u32>(0u), %8
    %10:vec2<u32> = shiftr %6, %9
    %11:vec2<u32> = and %10, vec2<u32>(240u)
    %12:vec2<bool> = eq %11, vec2<u32>(0u)
    %13:vec2<u32> = select vec2<u32>(4u), vec2<u32>(0u), %12
    %14:vec2<u32> = shiftr %10, %13
    %15:vec2<u32> = and %14, vec2<u32>(12u)
    %16:vec2<bool> = eq %15, vec2<u32>(0u)
    %17:vec2<u32> = select vec2<u32>(2u), vec2<u32>(0u), %16
    %18:vec2<u32> = shiftr %14, %17
    %19:vec2<u32> = and %18, vec2<u32>(2u)
    %20:vec2<bool> = eq %19, vec2<u32>(0u)
    %21:vec2<u32> = select vec2<u32>(1u), vec2<u32>(0u), %20
    %22:vec2<u32> = or %17, %21
    %23:vec2<u32> = or %13, %22
    %24:vec2<u32> = or %9, %23
    %25:vec2<u32> = or %5, %24
    %26:vec2<bool> = eq %18, vec2<u32>(0u)
    %result:vec2<u32> = select %25, vec2<u32>(4294967295u), %26
    ret %result
  }
}
)";

    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.first_leading_bit = true;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, FirstLeadingBit_Vec4I32) {
    Build(core::BuiltinFn::kFirstLeadingBit, ty.vec4<i32>(), Vector{ty.vec4<i32>()});
    auto* src = R"(
%foo = func(%arg:vec4<i32>):vec4<i32> -> %b1 {
  %b1 = block {
    %result:vec4<i32> = firstLeadingBit %arg
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%arg:vec4<i32>):vec4<i32> -> %b1 {
  %b1 = block {
    %3:vec4<u32> = bitcast %arg
    %4:vec4<u32> = complement %3
    %5:vec4<bool> = lt %3, vec4<u32>(2147483648u)
    %6:vec4<u32> = select %4, %3, %5
    %7:vec4<u32> = and %6, vec4<u32>(4294901760u)
    %8:vec4<bool> = eq %7, vec4<u32>(0u)
    %9:vec4<u32> = select vec4<u32>(16u), vec4<u32>(0u), %8
    %10:vec4<u32> = shiftr %6, %9
    %11:vec4<u32> = and %10, vec4<u32>(65280u)
    %12:vec4<bool> = eq %11, vec4<u32>(0u)
    %13:vec4<u32> = select vec4<u32>(8u), vec4<u32>(0u), %12
    %14:vec4<u32> = shiftr %10, %13
    %15:vec4<u32> = and %14, vec4<u32>(240u)
    %16:vec4<bool> = eq %15, vec4<u32>(0u)
    %17:vec4<u32> = select vec4<u32>(4u), vec4<u32>(0u), %16
    %18:vec4<u32> = shiftr %14, %17
    %19:vec4<u32> = and %18, vec4<u32>(12u)
    %20:vec4<bool> = eq %19, vec4<u32>(0u)
    %21:vec4<u32> = select vec4<u32>(2u), vec4<u32>(0u), %20
    %22:vec4<u32> = shiftr %18, %21
    %23:vec4<u32> = and %22, vec4<u32>(2u)
    %24:vec4<bool> = eq %23, vec4<u32>(0u)
    %25:vec4<u32> = select vec4<u32>(1u), vec4<u32>(0u), %24
    %26:vec4<u32> = or %21, %25
    %27:vec4<u32> = or %17, %26
    %28:vec4<u32> = or %13, %27
    %29:vec4<u32> = or %9, %28
    %30:vec4<bool> = eq %22, vec4<u32>(0u)
    %31:vec4<u32> = select %29, vec4<u32>(4294967295u), %30
    %result:vec4<i32> = bitcast %31
    ret %result
  }
}
)";

    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.first_leading_bit = true;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, FirstTrailingBit_NoPolyfill) {
    Build(core::BuiltinFn::kFirstTrailingBit, ty.u32(), Vector{ty.u32()});
    auto* src = R"(
%foo = func(%arg:u32):u32 -> %b1 {
  %b1 = block {
    %result:u32 = firstTrailingBit %arg
    ret %result
  }
}
)";
    auto* expect = src;

    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.first_trailing_bit = false;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, FirstTrailingBit_U32) {
    Build(core::BuiltinFn::kFirstTrailingBit, ty.u32(), Vector{ty.u32()});
    auto* src = R"(
%foo = func(%arg:u32):u32 -> %b1 {
  %b1 = block {
    %result:u32 = firstTrailingBit %arg
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%arg:u32):u32 -> %b1 {
  %b1 = block {
    %3:u32 = and %arg, 65535u
    %4:bool = eq %3, 0u
    %5:u32 = select 0u, 16u, %4
    %6:u32 = shiftr %arg, %5
    %7:u32 = and %6, 255u
    %8:bool = eq %7, 0u
    %9:u32 = select 0u, 8u, %8
    %10:u32 = shiftr %6, %9
    %11:u32 = and %10, 15u
    %12:bool = eq %11, 0u
    %13:u32 = select 0u, 4u, %12
    %14:u32 = shiftr %10, %13
    %15:u32 = and %14, 3u
    %16:bool = eq %15, 0u
    %17:u32 = select 0u, 2u, %16
    %18:u32 = shiftr %14, %17
    %19:u32 = and %18, 1u
    %20:bool = eq %19, 0u
    %21:u32 = select 0u, 1u, %20
    %22:u32 = or %17, %21
    %23:u32 = or %13, %22
    %24:u32 = or %9, %23
    %25:u32 = or %5, %24
    %26:bool = eq %18, 0u
    %result:u32 = select %25, 4294967295u, %26
    ret %result
  }
}
)";

    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.first_trailing_bit = true;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, FirstTrailingBit_I32) {
    Build(core::BuiltinFn::kFirstTrailingBit, ty.i32(), Vector{ty.i32()});
    auto* src = R"(
%foo = func(%arg:i32):i32 -> %b1 {
  %b1 = block {
    %result:i32 = firstTrailingBit %arg
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%arg:i32):i32 -> %b1 {
  %b1 = block {
    %3:u32 = bitcast %arg
    %4:u32 = and %3, 65535u
    %5:bool = eq %4, 0u
    %6:u32 = select 0u, 16u, %5
    %7:u32 = shiftr %3, %6
    %8:u32 = and %7, 255u
    %9:bool = eq %8, 0u
    %10:u32 = select 0u, 8u, %9
    %11:u32 = shiftr %7, %10
    %12:u32 = and %11, 15u
    %13:bool = eq %12, 0u
    %14:u32 = select 0u, 4u, %13
    %15:u32 = shiftr %11, %14
    %16:u32 = and %15, 3u
    %17:bool = eq %16, 0u
    %18:u32 = select 0u, 2u, %17
    %19:u32 = shiftr %15, %18
    %20:u32 = and %19, 1u
    %21:bool = eq %20, 0u
    %22:u32 = select 0u, 1u, %21
    %23:u32 = or %18, %22
    %24:u32 = or %14, %23
    %25:u32 = or %10, %24
    %26:u32 = or %6, %25
    %27:bool = eq %19, 0u
    %28:u32 = select %26, 4294967295u, %27
    %result:i32 = bitcast %28
    ret %result
  }
}
)";

    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.first_trailing_bit = true;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, FirstTrailingBit_Vec2U32) {
    Build(core::BuiltinFn::kFirstTrailingBit, ty.vec2<u32>(), Vector{ty.vec2<u32>()});
    auto* src = R"(
%foo = func(%arg:vec2<u32>):vec2<u32> -> %b1 {
  %b1 = block {
    %result:vec2<u32> = firstTrailingBit %arg
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%arg:vec2<u32>):vec2<u32> -> %b1 {
  %b1 = block {
    %3:vec2<u32> = and %arg, vec2<u32>(65535u)
    %4:vec2<bool> = eq %3, vec2<u32>(0u)
    %5:vec2<u32> = select vec2<u32>(0u), vec2<u32>(16u), %4
    %6:vec2<u32> = shiftr %arg, %5
    %7:vec2<u32> = and %6, vec2<u32>(255u)
    %8:vec2<bool> = eq %7, vec2<u32>(0u)
    %9:vec2<u32> = select vec2<u32>(0u), vec2<u32>(8u), %8
    %10:vec2<u32> = shiftr %6, %9
    %11:vec2<u32> = and %10, vec2<u32>(15u)
    %12:vec2<bool> = eq %11, vec2<u32>(0u)
    %13:vec2<u32> = select vec2<u32>(0u), vec2<u32>(4u), %12
    %14:vec2<u32> = shiftr %10, %13
    %15:vec2<u32> = and %14, vec2<u32>(3u)
    %16:vec2<bool> = eq %15, vec2<u32>(0u)
    %17:vec2<u32> = select vec2<u32>(0u), vec2<u32>(2u), %16
    %18:vec2<u32> = shiftr %14, %17
    %19:vec2<u32> = and %18, vec2<u32>(1u)
    %20:vec2<bool> = eq %19, vec2<u32>(0u)
    %21:vec2<u32> = select vec2<u32>(0u), vec2<u32>(1u), %20
    %22:vec2<u32> = or %17, %21
    %23:vec2<u32> = or %13, %22
    %24:vec2<u32> = or %9, %23
    %25:vec2<u32> = or %5, %24
    %26:vec2<bool> = eq %18, vec2<u32>(0u)
    %result:vec2<u32> = select %25, vec2<u32>(4294967295u), %26
    ret %result
  }
}
)";

    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.first_trailing_bit = true;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, FirstTrailingBit_Vec4I32) {
    Build(core::BuiltinFn::kFirstTrailingBit, ty.vec4<i32>(), Vector{ty.vec4<i32>()});
    auto* src = R"(
%foo = func(%arg:vec4<i32>):vec4<i32> -> %b1 {
  %b1 = block {
    %result:vec4<i32> = firstTrailingBit %arg
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%arg:vec4<i32>):vec4<i32> -> %b1 {
  %b1 = block {
    %3:vec4<u32> = bitcast %arg
    %4:vec4<u32> = and %3, vec4<u32>(65535u)
    %5:vec4<bool> = eq %4, vec4<u32>(0u)
    %6:vec4<u32> = select vec4<u32>(0u), vec4<u32>(16u), %5
    %7:vec4<u32> = shiftr %3, %6
    %8:vec4<u32> = and %7, vec4<u32>(255u)
    %9:vec4<bool> = eq %8, vec4<u32>(0u)
    %10:vec4<u32> = select vec4<u32>(0u), vec4<u32>(8u), %9
    %11:vec4<u32> = shiftr %7, %10
    %12:vec4<u32> = and %11, vec4<u32>(15u)
    %13:vec4<bool> = eq %12, vec4<u32>(0u)
    %14:vec4<u32> = select vec4<u32>(0u), vec4<u32>(4u), %13
    %15:vec4<u32> = shiftr %11, %14
    %16:vec4<u32> = and %15, vec4<u32>(3u)
    %17:vec4<bool> = eq %16, vec4<u32>(0u)
    %18:vec4<u32> = select vec4<u32>(0u), vec4<u32>(2u), %17
    %19:vec4<u32> = shiftr %15, %18
    %20:vec4<u32> = and %19, vec4<u32>(1u)
    %21:vec4<bool> = eq %20, vec4<u32>(0u)
    %22:vec4<u32> = select vec4<u32>(0u), vec4<u32>(1u), %21
    %23:vec4<u32> = or %18, %22
    %24:vec4<u32> = or %14, %23
    %25:vec4<u32> = or %10, %24
    %26:vec4<u32> = or %6, %25
    %27:vec4<bool> = eq %19, vec4<u32>(0u)
    %28:vec4<u32> = select %26, vec4<u32>(4294967295u), %27
    %result:vec4<i32> = bitcast %28
    ret %result
  }
}
)";

    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.first_trailing_bit = true;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, InsertBits_NoPolyfill) {
    Build(core::BuiltinFn::kInsertBits, ty.u32(), Vector{ty.u32(), ty.u32(), ty.u32(), ty.u32()});
    auto* src = R"(
%foo = func(%arg:u32, %arg_1:u32, %arg_2:u32, %arg_3:u32):u32 -> %b1 {  # %arg_1: 'arg', %arg_2: 'arg', %arg_3: 'arg'
  %b1 = block {
    %result:u32 = insertBits %arg, %arg_1, %arg_2, %arg_3
    ret %result
  }
}
)";
    auto* expect = src;

    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.insert_bits = BuiltinPolyfillLevel::kNone;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, InsertBits_ClampArgs_U32) {
    Build(core::BuiltinFn::kInsertBits, ty.u32(), Vector{ty.u32(), ty.u32(), ty.u32(), ty.u32()});
    auto* src = R"(
%foo = func(%arg:u32, %arg_1:u32, %arg_2:u32, %arg_3:u32):u32 -> %b1 {  # %arg_1: 'arg', %arg_2: 'arg', %arg_3: 'arg'
  %b1 = block {
    %result:u32 = insertBits %arg, %arg_1, %arg_2, %arg_3
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%arg:u32, %arg_1:u32, %arg_2:u32, %arg_3:u32):u32 -> %b1 {  # %arg_1: 'arg', %arg_2: 'arg', %arg_3: 'arg'
  %b1 = block {
    %6:u32 = min %arg_2, 32u
    %7:u32 = sub 32u, %6
    %8:u32 = min %arg_3, %7
    %result:u32 = insertBits %arg, %arg_1, %6, %8
    ret %result
  }
}
)";

    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.insert_bits = BuiltinPolyfillLevel::kClampOrRangeCheck;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, InsertBits_ClampArgs_I32) {
    Build(core::BuiltinFn::kInsertBits, ty.i32(), Vector{ty.i32(), ty.i32(), ty.u32(), ty.u32()});
    auto* src = R"(
%foo = func(%arg:i32, %arg_1:i32, %arg_2:u32, %arg_3:u32):i32 -> %b1 {  # %arg_1: 'arg', %arg_2: 'arg', %arg_3: 'arg'
  %b1 = block {
    %result:i32 = insertBits %arg, %arg_1, %arg_2, %arg_3
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%arg:i32, %arg_1:i32, %arg_2:u32, %arg_3:u32):i32 -> %b1 {  # %arg_1: 'arg', %arg_2: 'arg', %arg_3: 'arg'
  %b1 = block {
    %6:u32 = min %arg_2, 32u
    %7:u32 = sub 32u, %6
    %8:u32 = min %arg_3, %7
    %result:i32 = insertBits %arg, %arg_1, %6, %8
    ret %result
  }
}
)";

    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.insert_bits = BuiltinPolyfillLevel::kClampOrRangeCheck;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, InsertBits_ClampArgs_Vec2U32) {
    Build(core::BuiltinFn::kInsertBits, ty.vec2<u32>(),
          Vector{ty.vec2<u32>(), ty.vec2<u32>(), ty.u32(), ty.u32()});
    auto* src = R"(
%foo = func(%arg:vec2<u32>, %arg_1:vec2<u32>, %arg_2:u32, %arg_3:u32):vec2<u32> -> %b1 {  # %arg_1: 'arg', %arg_2: 'arg', %arg_3: 'arg'
  %b1 = block {
    %result:vec2<u32> = insertBits %arg, %arg_1, %arg_2, %arg_3
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%arg:vec2<u32>, %arg_1:vec2<u32>, %arg_2:u32, %arg_3:u32):vec2<u32> -> %b1 {  # %arg_1: 'arg', %arg_2: 'arg', %arg_3: 'arg'
  %b1 = block {
    %6:u32 = min %arg_2, 32u
    %7:u32 = sub 32u, %6
    %8:u32 = min %arg_3, %7
    %result:vec2<u32> = insertBits %arg, %arg_1, %6, %8
    ret %result
  }
}
)";

    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.insert_bits = BuiltinPolyfillLevel::kClampOrRangeCheck;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, InsertBits_ClampArgs_Vec4I32) {
    Build(core::BuiltinFn::kInsertBits, ty.vec4<i32>(),
          Vector{ty.vec4<i32>(), ty.vec4<i32>(), ty.u32(), ty.u32()});
    auto* src = R"(
%foo = func(%arg:vec4<i32>, %arg_1:vec4<i32>, %arg_2:u32, %arg_3:u32):vec4<i32> -> %b1 {  # %arg_1: 'arg', %arg_2: 'arg', %arg_3: 'arg'
  %b1 = block {
    %result:vec4<i32> = insertBits %arg, %arg_1, %arg_2, %arg_3
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%arg:vec4<i32>, %arg_1:vec4<i32>, %arg_2:u32, %arg_3:u32):vec4<i32> -> %b1 {  # %arg_1: 'arg', %arg_2: 'arg', %arg_3: 'arg'
  %b1 = block {
    %6:u32 = min %arg_2, 32u
    %7:u32 = sub 32u, %6
    %8:u32 = min %arg_3, %7
    %result:vec4<i32> = insertBits %arg, %arg_1, %6, %8
    ret %result
  }
}
)";

    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.insert_bits = BuiltinPolyfillLevel::kClampOrRangeCheck;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, TextureSampleBaseClampToEdge_2d_f32_NoPolyfill) {
    auto* texture_ty =
        ty.Get<core::type::SampledTexture>(core::type::TextureDimension::k2d, ty.f32());
    Build(core::BuiltinFn::kTextureSampleBaseClampToEdge, ty.vec4<f32>(),
          Vector{texture_ty, ty.sampler(), ty.vec2<f32>()});
    auto* src = R"(
%foo = func(%arg:texture_2d<f32>, %arg_1:sampler, %arg_2:vec2<f32>):vec4<f32> -> %b1 {  # %arg_1: 'arg', %arg_2: 'arg'
  %b1 = block {
    %result:vec4<f32> = textureSampleBaseClampToEdge %arg, %arg_1, %arg_2
    ret %result
  }
}
)";
    auto* expect = src;

    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.texture_sample_base_clamp_to_edge_2d_f32 = false;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BuiltinPolyfillTest, TextureSampleBaseClampToEdge_2d_f32) {
    auto* texture_ty =
        ty.Get<core::type::SampledTexture>(core::type::TextureDimension::k2d, ty.f32());
    Build(core::BuiltinFn::kTextureSampleBaseClampToEdge, ty.vec4<f32>(),
          Vector{texture_ty, ty.sampler(), ty.vec2<f32>()});
    auto* src = R"(
%foo = func(%arg:texture_2d<f32>, %arg_1:sampler, %arg_2:vec2<f32>):vec4<f32> -> %b1 {  # %arg_1: 'arg', %arg_2: 'arg'
  %b1 = block {
    %result:vec4<f32> = textureSampleBaseClampToEdge %arg, %arg_1, %arg_2
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%arg:texture_2d<f32>, %arg_1:sampler, %arg_2:vec2<f32>):vec4<f32> -> %b1 {  # %arg_1: 'arg', %arg_2: 'arg'
  %b1 = block {
    %5:vec2<u32> = textureDimensions %arg
    %6:vec2<f32> = convert %5
    %7:vec2<f32> = div vec2<f32>(0.5f), %6
    %8:vec2<f32> = sub vec2<f32>(1.0f), %7
    %9:vec2<f32> = clamp %arg_2, %7, %8
    %result:vec4<f32> = textureSampleLevel %arg, %arg_1, %9, 0.0f
    ret %result
  }
}
)";

    EXPECT_EQ(src, str());

    BuiltinPolyfillConfig config;
    config.texture_sample_base_clamp_to_edge_2d_f32 = true;
    Run(BuiltinPolyfill, config);
    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::core::ir::transform
