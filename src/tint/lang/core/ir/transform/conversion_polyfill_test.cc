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

#include "src/tint/lang/core/ir/transform/conversion_polyfill.h"

#include <utility>

#include "src/tint/lang/core/ir/transform/helper_test.h"

namespace tint::core::ir::transform {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

class IR_ConversionPolyfillTest : public TransformTest {
  protected:
    /// Helper to build a function that executes a convert instruction.
    /// @param src_ty the type of the source
    /// @param res_ty the type of the result
    void Build(const core::type::Type* src_ty, const core::type::Type* res_ty) {
        auto* func = b.Function("foo", res_ty);
        auto* src = b.FunctionParam("src", src_ty);
        func->SetParams({src});
        b.Append(func->Block(), [&] {
            auto* result = b.Convert(res_ty, src);
            b.Return(func, result);
            mod.SetName(result, "result");
        });
    }
};

// No change expected in this direction.
TEST_F(IR_ConversionPolyfillTest, I32_to_F32) {
    Build(ty.i32(), ty.f32());
    auto* src = R"(
%foo = func(%src:i32):f32 -> %b1 {
  %b1 = block {
    %result:f32 = convert %src
    ret %result
  }
}
)";
    auto* expect = src;

    EXPECT_EQ(src, str());

    ConversionPolyfillConfig config;
    config.ftoi = true;
    Run(ConversionPolyfill, config);
    EXPECT_EQ(expect, str());
}

// No change expected in this direction.
TEST_F(IR_ConversionPolyfillTest, U32_to_F32) {
    Build(ty.u32(), ty.f32());
    auto* src = R"(
%foo = func(%src:u32):f32 -> %b1 {
  %b1 = block {
    %result:f32 = convert %src
    ret %result
  }
}
)";
    auto* expect = src;

    EXPECT_EQ(src, str());

    ConversionPolyfillConfig config;
    config.ftoi = true;
    Run(ConversionPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_ConversionPolyfillTest, F32_to_I32_NoPolyfill) {
    Build(ty.f32(), ty.i32());
    auto* src = R"(
%foo = func(%src:f32):i32 -> %b1 {
  %b1 = block {
    %result:i32 = convert %src
    ret %result
  }
}
)";
    auto* expect = src;

    EXPECT_EQ(src, str());

    ConversionPolyfillConfig config;
    config.ftoi = false;
    Run(ConversionPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_ConversionPolyfillTest, F32_to_I32) {
    Build(ty.f32(), ty.i32());
    auto* src = R"(
%foo = func(%src:f32):i32 -> %b1 {
  %b1 = block {
    %result:i32 = convert %src
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%src:f32):i32 -> %b1 {
  %b1 = block {
    %result:i32 = call %tint_f32_to_i32, %src
    ret %result
  }
}
%tint_f32_to_i32 = func(%value:f32):i32 -> %b2 {
  %b2 = block {
    %6:i32 = convert %value
    %7:bool = gte %value, -2147483648.0f
    %8:i32 = select -2147483648i, %6, %7
    %9:bool = lte %value, 2147483520.0f
    %10:i32 = select 2147483647i, %8, %9
    ret %10
  }
}
)";

    EXPECT_EQ(src, str());

    ConversionPolyfillConfig config;
    config.ftoi = true;
    Run(ConversionPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_ConversionPolyfillTest, F32_to_U32) {
    Build(ty.f32(), ty.u32());
    auto* src = R"(
%foo = func(%src:f32):u32 -> %b1 {
  %b1 = block {
    %result:u32 = convert %src
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%src:f32):u32 -> %b1 {
  %b1 = block {
    %result:u32 = call %tint_f32_to_u32, %src
    ret %result
  }
}
%tint_f32_to_u32 = func(%value:f32):u32 -> %b2 {
  %b2 = block {
    %6:u32 = convert %value
    %7:bool = gte %value, 0.0f
    %8:u32 = select 0u, %6, %7
    %9:bool = lte %value, 4294967040.0f
    %10:u32 = select 4294967295u, %8, %9
    ret %10
  }
}
)";

    EXPECT_EQ(src, str());

    ConversionPolyfillConfig config;
    config.ftoi = true;
    Run(ConversionPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_ConversionPolyfillTest, F32_to_I32_Vec2) {
    Build(ty.vec2<f32>(), ty.vec2<i32>());
    auto* src = R"(
%foo = func(%src:vec2<f32>):vec2<i32> -> %b1 {
  %b1 = block {
    %result:vec2<i32> = convert %src
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%src:vec2<f32>):vec2<i32> -> %b1 {
  %b1 = block {
    %result:vec2<i32> = call %tint_v2f32_to_v2i32, %src
    ret %result
  }
}
%tint_v2f32_to_v2i32 = func(%value:vec2<f32>):vec2<i32> -> %b2 {
  %b2 = block {
    %6:vec2<i32> = convert %value
    %7:vec2<bool> = gte %value, vec2<f32>(-2147483648.0f)
    %8:vec2<i32> = select vec2<i32>(-2147483648i), %6, %7
    %9:vec2<bool> = lte %value, vec2<f32>(2147483520.0f)
    %10:vec2<i32> = select vec2<i32>(2147483647i), %8, %9
    ret %10
  }
}
)";

    EXPECT_EQ(src, str());

    ConversionPolyfillConfig config;
    config.ftoi = true;
    Run(ConversionPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_ConversionPolyfillTest, F32_to_U32_Vec3) {
    Build(ty.vec2<f32>(), ty.vec2<u32>());
    auto* src = R"(
%foo = func(%src:vec2<f32>):vec2<u32> -> %b1 {
  %b1 = block {
    %result:vec2<u32> = convert %src
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%src:vec2<f32>):vec2<u32> -> %b1 {
  %b1 = block {
    %result:vec2<u32> = call %tint_v2f32_to_v2u32, %src
    ret %result
  }
}
%tint_v2f32_to_v2u32 = func(%value:vec2<f32>):vec2<u32> -> %b2 {
  %b2 = block {
    %6:vec2<u32> = convert %value
    %7:vec2<bool> = gte %value, vec2<f32>(0.0f)
    %8:vec2<u32> = select vec2<u32>(0u), %6, %7
    %9:vec2<bool> = lte %value, vec2<f32>(4294967040.0f)
    %10:vec2<u32> = select vec2<u32>(4294967295u), %8, %9
    ret %10
  }
}
)";

    EXPECT_EQ(src, str());

    ConversionPolyfillConfig config;
    config.ftoi = true;
    Run(ConversionPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_ConversionPolyfillTest, F16_to_I32) {
    Build(ty.f16(), ty.i32());
    auto* src = R"(
%foo = func(%src:f16):i32 -> %b1 {
  %b1 = block {
    %result:i32 = convert %src
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%src:f16):i32 -> %b1 {
  %b1 = block {
    %result:i32 = call %tint_f16_to_i32, %src
    ret %result
  }
}
%tint_f16_to_i32 = func(%value:f16):i32 -> %b2 {
  %b2 = block {
    %6:i32 = convert %value
    %7:bool = gte %value, -65504.0h
    %8:i32 = select -2147483648i, %6, %7
    %9:bool = lte %value, 65504.0h
    %10:i32 = select 2147483647i, %8, %9
    ret %10
  }
}
)";

    EXPECT_EQ(src, str());

    ConversionPolyfillConfig config;
    config.ftoi = true;
    Run(ConversionPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_ConversionPolyfillTest, F16_to_U32) {
    Build(ty.f16(), ty.u32());
    auto* src = R"(
%foo = func(%src:f16):u32 -> %b1 {
  %b1 = block {
    %result:u32 = convert %src
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%src:f16):u32 -> %b1 {
  %b1 = block {
    %result:u32 = call %tint_f16_to_u32, %src
    ret %result
  }
}
%tint_f16_to_u32 = func(%value:f16):u32 -> %b2 {
  %b2 = block {
    %6:u32 = convert %value
    %7:bool = gte %value, 0.0h
    %8:u32 = select 0u, %6, %7
    %9:bool = lte %value, 65504.0h
    %10:u32 = select 4294967295u, %8, %9
    ret %10
  }
}
)";

    EXPECT_EQ(src, str());

    ConversionPolyfillConfig config;
    config.ftoi = true;
    Run(ConversionPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_ConversionPolyfillTest, F16_to_I32_Vec2) {
    Build(ty.vec2<f16>(), ty.vec2<i32>());
    auto* src = R"(
%foo = func(%src:vec2<f16>):vec2<i32> -> %b1 {
  %b1 = block {
    %result:vec2<i32> = convert %src
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%src:vec2<f16>):vec2<i32> -> %b1 {
  %b1 = block {
    %result:vec2<i32> = call %tint_v2f16_to_v2i32, %src
    ret %result
  }
}
%tint_v2f16_to_v2i32 = func(%value:vec2<f16>):vec2<i32> -> %b2 {
  %b2 = block {
    %6:vec2<i32> = convert %value
    %7:vec2<bool> = gte %value, vec2<f16>(-65504.0h)
    %8:vec2<i32> = select vec2<i32>(-2147483648i), %6, %7
    %9:vec2<bool> = lte %value, vec2<f16>(65504.0h)
    %10:vec2<i32> = select vec2<i32>(2147483647i), %8, %9
    ret %10
  }
}
)";

    EXPECT_EQ(src, str());

    ConversionPolyfillConfig config;
    config.ftoi = true;
    Run(ConversionPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_ConversionPolyfillTest, F16_to_U32_Vec3) {
    Build(ty.vec2<f16>(), ty.vec2<u32>());
    auto* src = R"(
%foo = func(%src:vec2<f16>):vec2<u32> -> %b1 {
  %b1 = block {
    %result:vec2<u32> = convert %src
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%src:vec2<f16>):vec2<u32> -> %b1 {
  %b1 = block {
    %result:vec2<u32> = call %tint_v2f16_to_v2u32, %src
    ret %result
  }
}
%tint_v2f16_to_v2u32 = func(%value:vec2<f16>):vec2<u32> -> %b2 {
  %b2 = block {
    %6:vec2<u32> = convert %value
    %7:vec2<bool> = gte %value, vec2<f16>(0.0h)
    %8:vec2<u32> = select vec2<u32>(0u), %6, %7
    %9:vec2<bool> = lte %value, vec2<f16>(65504.0h)
    %10:vec2<u32> = select vec2<u32>(4294967295u), %8, %9
    ret %10
  }
}
)";

    EXPECT_EQ(src, str());

    ConversionPolyfillConfig config;
    config.ftoi = true;
    Run(ConversionPolyfill, config);
    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::core::ir::transform
