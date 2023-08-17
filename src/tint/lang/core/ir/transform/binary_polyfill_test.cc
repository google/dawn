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

#include "src/tint/lang/core/ir/transform/binary_polyfill.h"
#include "src/tint/lang/core/ir/binary.h"

#include <utility>

#include "src/tint/lang/core/ir/transform/helper_test.h"

namespace tint::core::ir::transform {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

class IR_BinaryPolyfillTest : public TransformTest {
  protected:
    /// Helper to build a function that executes a binary instruction.
    /// @param kind the binary operation
    /// @param result_ty the result type of the builtin call
    /// @param lhs_ty the type of the LHS
    /// @param rhs_ty the type of the RHS
    void Build(enum ir::Binary::Kind kind,
               const core::type::Type* result_ty,
               const core::type::Type* lhs_ty,
               const core::type::Type* rhs_ty) {
        Vector<FunctionParam*, 4> args;
        args.Push(b.FunctionParam("lhs", lhs_ty));
        args.Push(b.FunctionParam("rhs", rhs_ty));
        auto* func = b.Function("foo", result_ty);
        func->SetParams(args);
        b.Append(func->Block(), [&] {
            auto* result = b.Binary(kind, result_ty, args[0], args[1]);
            b.Return(func, result);
            mod.SetName(result, "result");
        });
    }
};

TEST_F(IR_BinaryPolyfillTest, ShiftLeft_NoPolyfill) {
    Build(Binary::Kind::kShiftLeft, ty.i32(), ty.i32(), ty.i32());
    auto* src = R"(
%foo = func(%lhs:i32, %rhs:i32):i32 -> %b1 {
  %b1 = block {
    %result:i32 = shiftl %lhs, %rhs
    ret %result
  }
}
)";
    auto* expect = src;

    EXPECT_EQ(src, str());

    BinaryPolyfillConfig config;
    config.bitshift_modulo = false;
    Run(BinaryPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BinaryPolyfillTest, ShiftRight_NoPolyfill) {
    Build(Binary::Kind::kShiftRight, ty.i32(), ty.i32(), ty.i32());
    auto* src = R"(
%foo = func(%lhs:i32, %rhs:i32):i32 -> %b1 {
  %b1 = block {
    %result:i32 = shiftr %lhs, %rhs
    ret %result
  }
}
)";
    auto* expect = src;

    EXPECT_EQ(src, str());

    BinaryPolyfillConfig config;
    config.bitshift_modulo = false;
    Run(BinaryPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BinaryPolyfillTest, ShiftLeft_I32) {
    Build(Binary::Kind::kShiftLeft, ty.i32(), ty.i32(), ty.i32());
    auto* src = R"(
%foo = func(%lhs:i32, %rhs:i32):i32 -> %b1 {
  %b1 = block {
    %result:i32 = shiftl %lhs, %rhs
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%lhs:i32, %rhs:i32):i32 -> %b1 {
  %b1 = block {
    %4:i32 = and %rhs, 31u
    %result:i32 = shiftl %lhs, %4
    ret %result
  }
}
)";

    EXPECT_EQ(src, str());

    BinaryPolyfillConfig config;
    config.bitshift_modulo = true;
    Run(BinaryPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BinaryPolyfillTest, ShiftLeft_U32) {
    Build(Binary::Kind::kShiftLeft, ty.u32(), ty.u32(), ty.u32());
    auto* src = R"(
%foo = func(%lhs:u32, %rhs:u32):u32 -> %b1 {
  %b1 = block {
    %result:u32 = shiftl %lhs, %rhs
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%lhs:u32, %rhs:u32):u32 -> %b1 {
  %b1 = block {
    %4:u32 = and %rhs, 31u
    %result:u32 = shiftl %lhs, %4
    ret %result
  }
}
)";

    EXPECT_EQ(src, str());

    BinaryPolyfillConfig config;
    config.bitshift_modulo = true;
    Run(BinaryPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BinaryPolyfillTest, ShiftLeft_Vec2I32) {
    Build(Binary::Kind::kShiftLeft, ty.vec2<i32>(), ty.vec2<i32>(), ty.vec2<i32>());
    auto* src = R"(
%foo = func(%lhs:vec2<i32>, %rhs:vec2<i32>):vec2<i32> -> %b1 {
  %b1 = block {
    %result:vec2<i32> = shiftl %lhs, %rhs
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%lhs:vec2<i32>, %rhs:vec2<i32>):vec2<i32> -> %b1 {
  %b1 = block {
    %4:vec2<i32> = and %rhs, vec2<u32>(31u)
    %result:vec2<i32> = shiftl %lhs, %4
    ret %result
  }
}
)";

    EXPECT_EQ(src, str());

    BinaryPolyfillConfig config;
    config.bitshift_modulo = true;
    Run(BinaryPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BinaryPolyfillTest, ShiftLeft_Vec3U32) {
    Build(Binary::Kind::kShiftLeft, ty.vec3<u32>(), ty.vec3<u32>(), ty.vec3<u32>());
    auto* src = R"(
%foo = func(%lhs:vec3<u32>, %rhs:vec3<u32>):vec3<u32> -> %b1 {
  %b1 = block {
    %result:vec3<u32> = shiftl %lhs, %rhs
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%lhs:vec3<u32>, %rhs:vec3<u32>):vec3<u32> -> %b1 {
  %b1 = block {
    %4:vec3<u32> = and %rhs, vec3<u32>(31u)
    %result:vec3<u32> = shiftl %lhs, %4
    ret %result
  }
}
)";

    EXPECT_EQ(src, str());

    BinaryPolyfillConfig config;
    config.bitshift_modulo = true;
    Run(BinaryPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BinaryPolyfillTest, ShiftRight_I32) {
    Build(Binary::Kind::kShiftRight, ty.i32(), ty.i32(), ty.i32());
    auto* src = R"(
%foo = func(%lhs:i32, %rhs:i32):i32 -> %b1 {
  %b1 = block {
    %result:i32 = shiftr %lhs, %rhs
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%lhs:i32, %rhs:i32):i32 -> %b1 {
  %b1 = block {
    %4:i32 = and %rhs, 31u
    %result:i32 = shiftr %lhs, %4
    ret %result
  }
}
)";

    EXPECT_EQ(src, str());

    BinaryPolyfillConfig config;
    config.bitshift_modulo = true;
    Run(BinaryPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BinaryPolyfillTest, ShiftRight_U32) {
    Build(Binary::Kind::kShiftRight, ty.u32(), ty.u32(), ty.u32());
    auto* src = R"(
%foo = func(%lhs:u32, %rhs:u32):u32 -> %b1 {
  %b1 = block {
    %result:u32 = shiftr %lhs, %rhs
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%lhs:u32, %rhs:u32):u32 -> %b1 {
  %b1 = block {
    %4:u32 = and %rhs, 31u
    %result:u32 = shiftr %lhs, %4
    ret %result
  }
}
)";

    EXPECT_EQ(src, str());

    BinaryPolyfillConfig config;
    config.bitshift_modulo = true;
    Run(BinaryPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BinaryPolyfillTest, ShiftRight_Vec2I32) {
    Build(Binary::Kind::kShiftRight, ty.vec2<i32>(), ty.vec2<i32>(), ty.vec2<i32>());
    auto* src = R"(
%foo = func(%lhs:vec2<i32>, %rhs:vec2<i32>):vec2<i32> -> %b1 {
  %b1 = block {
    %result:vec2<i32> = shiftr %lhs, %rhs
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%lhs:vec2<i32>, %rhs:vec2<i32>):vec2<i32> -> %b1 {
  %b1 = block {
    %4:vec2<i32> = and %rhs, vec2<u32>(31u)
    %result:vec2<i32> = shiftr %lhs, %4
    ret %result
  }
}
)";

    EXPECT_EQ(src, str());

    BinaryPolyfillConfig config;
    config.bitshift_modulo = true;
    Run(BinaryPolyfill, config);
    EXPECT_EQ(expect, str());
}

TEST_F(IR_BinaryPolyfillTest, ShiftRight_Vec3U32) {
    Build(Binary::Kind::kShiftRight, ty.vec3<u32>(), ty.vec3<u32>(), ty.vec3<u32>());
    auto* src = R"(
%foo = func(%lhs:vec3<u32>, %rhs:vec3<u32>):vec3<u32> -> %b1 {
  %b1 = block {
    %result:vec3<u32> = shiftr %lhs, %rhs
    ret %result
  }
}
)";
    auto* expect = R"(
%foo = func(%lhs:vec3<u32>, %rhs:vec3<u32>):vec3<u32> -> %b1 {
  %b1 = block {
    %4:vec3<u32> = and %rhs, vec3<u32>(31u)
    %result:vec3<u32> = shiftr %lhs, %4
    ret %result
  }
}
)";

    EXPECT_EQ(src, str());

    BinaryPolyfillConfig config;
    config.bitshift_modulo = true;
    Run(BinaryPolyfill, config);
    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::core::ir::transform
