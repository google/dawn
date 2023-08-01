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

#include "src/tint/lang/core/ir/transform/expand_implicit_splats.h"

#include <utility>

#include "src/tint/lang/core/ir/transform/helper_test.h"

namespace tint::ir::transform {
namespace {

using namespace tint::builtin::fluent_types;  // NOLINT
using namespace tint::number_suffixes;        // NOLINT

using IR_ExpandImplicitSplatsTest = TransformTest;

TEST_F(IR_ExpandImplicitSplatsTest, NoModify_Construct_VectorIdentity) {
    auto* vector = b.FunctionParam("vector", ty.vec2<i32>());
    auto* func = b.Function("foo", ty.vec2<i32>());
    func->SetParams({vector});

    b.Append(func->Block(), [&] {
        auto* result = b.Construct(ty.vec2<i32>(), vector);
        b.Return(func, result);
    });

    auto* expect = R"(
%foo = func(%vector:vec2<i32>):vec2<i32> -> %b1 {
  %b1 = block {
    %3:vec2<i32> = construct %vector
    ret %3
  }
}
)";

    Run(ExpandImplicitSplats);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_ExpandImplicitSplatsTest, NoModify_Construct_MixedScalarVector) {
    auto* scalar = b.FunctionParam("scalar", ty.i32());
    auto* vector = b.FunctionParam("vector", ty.vec2<i32>());
    auto* func = b.Function("foo", ty.vec3<i32>());
    func->SetParams({scalar, vector});

    b.Append(func->Block(), [&] {
        auto* result = b.Construct(ty.vec3<i32>(), scalar, vector);
        b.Return(func, result);
    });

    auto* expect = R"(
%foo = func(%scalar:i32, %vector:vec2<i32>):vec3<i32> -> %b1 {
  %b1 = block {
    %4:vec3<i32> = construct %scalar, %vector
    ret %4
  }
}
)";

    Run(ExpandImplicitSplats);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_ExpandImplicitSplatsTest, NoModify_Construct_AllScalars) {
    auto* scalar = b.FunctionParam("scalar", ty.i32());
    auto* func = b.Function("foo", ty.vec3<i32>());
    func->SetParams({scalar});

    b.Append(func->Block(), [&] {
        auto* result = b.Construct(ty.vec3<i32>(), scalar, scalar, scalar);
        b.Return(func, result);
    });

    auto* expect = R"(
%foo = func(%scalar:i32):vec3<i32> -> %b1 {
  %b1 = block {
    %3:vec3<i32> = construct %scalar, %scalar, %scalar
    ret %3
  }
}
)";

    Run(ExpandImplicitSplats);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_ExpandImplicitSplatsTest, Construct_Splat_Vec2i) {
    auto* scalar = b.FunctionParam("scalar", ty.i32());
    auto* func = b.Function("foo", ty.vec2<i32>());
    func->SetParams({scalar});

    b.Append(func->Block(), [&] {
        auto* result = b.Construct(ty.vec2<i32>(), scalar);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%scalar:i32):vec2<i32> -> %b1 {
  %b1 = block {
    %3:vec2<i32> = construct %scalar
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%scalar:i32):vec2<i32> -> %b1 {
  %b1 = block {
    %3:vec2<i32> = construct %scalar, %scalar
    ret %3
  }
}
)";

    Run(ExpandImplicitSplats);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_ExpandImplicitSplatsTest, Construct_Splat_Vec3u) {
    auto* scalar = b.FunctionParam("scalar", ty.u32());
    auto* func = b.Function("foo", ty.vec3<u32>());
    func->SetParams({scalar});

    b.Append(func->Block(), [&] {
        auto* result = b.Construct(ty.vec3<u32>(), scalar);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%scalar:u32):vec3<u32> -> %b1 {
  %b1 = block {
    %3:vec3<u32> = construct %scalar
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%scalar:u32):vec3<u32> -> %b1 {
  %b1 = block {
    %3:vec3<u32> = construct %scalar, %scalar, %scalar
    ret %3
  }
}
)";

    Run(ExpandImplicitSplats);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_ExpandImplicitSplatsTest, Construct_Splat_Vec4f) {
    auto* scalar = b.FunctionParam("scalar", ty.f32());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({scalar});

    b.Append(func->Block(), [&] {
        auto* result = b.Construct(ty.vec4<f32>(), scalar);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%scalar:f32):vec4<f32> -> %b1 {
  %b1 = block {
    %3:vec4<f32> = construct %scalar
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%scalar:f32):vec4<f32> -> %b1 {
  %b1 = block {
    %3:vec4<f32> = construct %scalar, %scalar, %scalar, %scalar
    ret %3
  }
}
)";

    Run(ExpandImplicitSplats);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_ExpandImplicitSplatsTest, BinaryAdd_VectorScalar_Vec4f) {
    auto* scalar = b.FunctionParam("scalar", ty.f32());
    auto* vector = b.FunctionParam("vector", ty.vec4<f32>());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({scalar, vector});

    b.Append(func->Block(), [&] {
        auto* result = b.Add(ty.vec4<f32>(), vector, scalar);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%scalar:f32, %vector:vec4<f32>):vec4<f32> -> %b1 {
  %b1 = block {
    %4:vec4<f32> = add %vector, %scalar
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%scalar:f32, %vector:vec4<f32>):vec4<f32> -> %b1 {
  %b1 = block {
    %4:vec4<f32> = construct %scalar, %scalar, %scalar, %scalar
    %5:vec4<f32> = add %vector, %4
    ret %5
  }
}
)";

    Run(ExpandImplicitSplats);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_ExpandImplicitSplatsTest, BinaryAdd_ScalarVector_Vec4f) {
    auto* scalar = b.FunctionParam("scalar", ty.f32());
    auto* vector = b.FunctionParam("vector", ty.vec4<f32>());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({scalar, vector});

    b.Append(func->Block(), [&] {
        auto* result = b.Add(ty.vec4<f32>(), scalar, vector);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%scalar:f32, %vector:vec4<f32>):vec4<f32> -> %b1 {
  %b1 = block {
    %4:vec4<f32> = add %scalar, %vector
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%scalar:f32, %vector:vec4<f32>):vec4<f32> -> %b1 {
  %b1 = block {
    %4:vec4<f32> = construct %scalar, %scalar, %scalar, %scalar
    %5:vec4<f32> = add %4, %vector
    ret %5
  }
}
)";

    Run(ExpandImplicitSplats);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_ExpandImplicitSplatsTest, BinarySubtract_VectorScalar_Vec4f) {
    auto* scalar = b.FunctionParam("scalar", ty.f32());
    auto* vector = b.FunctionParam("vector", ty.vec4<f32>());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({scalar, vector});

    b.Append(func->Block(), [&] {
        auto* result = b.Subtract(ty.vec4<f32>(), vector, scalar);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%scalar:f32, %vector:vec4<f32>):vec4<f32> -> %b1 {
  %b1 = block {
    %4:vec4<f32> = sub %vector, %scalar
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%scalar:f32, %vector:vec4<f32>):vec4<f32> -> %b1 {
  %b1 = block {
    %4:vec4<f32> = construct %scalar, %scalar, %scalar, %scalar
    %5:vec4<f32> = sub %vector, %4
    ret %5
  }
}
)";

    Run(ExpandImplicitSplats);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_ExpandImplicitSplatsTest, BinarySubtract_ScalarVector_Vec4f) {
    auto* scalar = b.FunctionParam("scalar", ty.f32());
    auto* vector = b.FunctionParam("vector", ty.vec4<f32>());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({scalar, vector});

    b.Append(func->Block(), [&] {
        auto* result = b.Subtract(ty.vec4<f32>(), scalar, vector);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%scalar:f32, %vector:vec4<f32>):vec4<f32> -> %b1 {
  %b1 = block {
    %4:vec4<f32> = sub %scalar, %vector
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%scalar:f32, %vector:vec4<f32>):vec4<f32> -> %b1 {
  %b1 = block {
    %4:vec4<f32> = construct %scalar, %scalar, %scalar, %scalar
    %5:vec4<f32> = sub %4, %vector
    ret %5
  }
}
)";

    Run(ExpandImplicitSplats);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_ExpandImplicitSplatsTest, BinaryDivide_VectorScalar_Vec4f) {
    auto* scalar = b.FunctionParam("scalar", ty.f32());
    auto* vector = b.FunctionParam("vector", ty.vec4<f32>());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({scalar, vector});

    b.Append(func->Block(), [&] {
        auto* result = b.Divide(ty.vec4<f32>(), vector, scalar);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%scalar:f32, %vector:vec4<f32>):vec4<f32> -> %b1 {
  %b1 = block {
    %4:vec4<f32> = div %vector, %scalar
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%scalar:f32, %vector:vec4<f32>):vec4<f32> -> %b1 {
  %b1 = block {
    %4:vec4<f32> = construct %scalar, %scalar, %scalar, %scalar
    %5:vec4<f32> = div %vector, %4
    ret %5
  }
}
)";

    Run(ExpandImplicitSplats);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_ExpandImplicitSplatsTest, BinaryDivide_ScalarVector_Vec4f) {
    auto* scalar = b.FunctionParam("scalar", ty.f32());
    auto* vector = b.FunctionParam("vector", ty.vec4<f32>());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({scalar, vector});

    b.Append(func->Block(), [&] {
        auto* result = b.Divide(ty.vec4<f32>(), scalar, vector);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%scalar:f32, %vector:vec4<f32>):vec4<f32> -> %b1 {
  %b1 = block {
    %4:vec4<f32> = div %scalar, %vector
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%scalar:f32, %vector:vec4<f32>):vec4<f32> -> %b1 {
  %b1 = block {
    %4:vec4<f32> = construct %scalar, %scalar, %scalar, %scalar
    %5:vec4<f32> = div %4, %vector
    ret %5
  }
}
)";

    Run(ExpandImplicitSplats);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_ExpandImplicitSplatsTest, BinaryModulo_VectorScalar_Vec4f) {
    auto* scalar = b.FunctionParam("scalar", ty.f32());
    auto* vector = b.FunctionParam("vector", ty.vec4<f32>());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({scalar, vector});

    b.Append(func->Block(), [&] {
        auto* result = b.Modulo(ty.vec4<f32>(), vector, scalar);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%scalar:f32, %vector:vec4<f32>):vec4<f32> -> %b1 {
  %b1 = block {
    %4:vec4<f32> = mod %vector, %scalar
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%scalar:f32, %vector:vec4<f32>):vec4<f32> -> %b1 {
  %b1 = block {
    %4:vec4<f32> = construct %scalar, %scalar, %scalar, %scalar
    %5:vec4<f32> = mod %vector, %4
    ret %5
  }
}
)";

    Run(ExpandImplicitSplats);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_ExpandImplicitSplatsTest, BinaryModulo_ScalarVector_Vec4f) {
    auto* scalar = b.FunctionParam("scalar", ty.f32());
    auto* vector = b.FunctionParam("vector", ty.vec4<f32>());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({scalar, vector});

    b.Append(func->Block(), [&] {
        auto* result = b.Modulo(ty.vec4<f32>(), scalar, vector);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%scalar:f32, %vector:vec4<f32>):vec4<f32> -> %b1 {
  %b1 = block {
    %4:vec4<f32> = mod %scalar, %vector
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%scalar:f32, %vector:vec4<f32>):vec4<f32> -> %b1 {
  %b1 = block {
    %4:vec4<f32> = construct %scalar, %scalar, %scalar, %scalar
    %5:vec4<f32> = mod %4, %vector
    ret %5
  }
}
)";

    Run(ExpandImplicitSplats);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_ExpandImplicitSplatsTest, BinaryMultiply_VectorScalar_Vec4f) {
    auto* scalar = b.FunctionParam("scalar", ty.f32());
    auto* vector = b.FunctionParam("vector", ty.vec4<f32>());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({scalar, vector});

    b.Append(func->Block(), [&] {
        auto* result = b.Multiply(ty.vec4<f32>(), vector, scalar);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%scalar:f32, %vector:vec4<f32>):vec4<f32> -> %b1 {
  %b1 = block {
    %4:vec4<f32> = mul %vector, %scalar
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%scalar:f32, %vector:vec4<f32>):vec4<f32> -> %b1 {
  %b1 = block {
    %4:vec4<f32> = spirv.vector_times_scalar %vector, %scalar
    ret %4
  }
}
)";

    Run(ExpandImplicitSplats);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_ExpandImplicitSplatsTest, BinaryMultiply_ScalarVector_Vec4f) {
    auto* scalar = b.FunctionParam("scalar", ty.f32());
    auto* vector = b.FunctionParam("vector", ty.vec4<f32>());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({scalar, vector});

    b.Append(func->Block(), [&] {
        auto* result = b.Multiply(ty.vec4<f32>(), scalar, vector);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%scalar:f32, %vector:vec4<f32>):vec4<f32> -> %b1 {
  %b1 = block {
    %4:vec4<f32> = mul %scalar, %vector
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%scalar:f32, %vector:vec4<f32>):vec4<f32> -> %b1 {
  %b1 = block {
    %4:vec4<f32> = spirv.vector_times_scalar %vector, %scalar
    ret %4
  }
}
)";

    Run(ExpandImplicitSplats);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_ExpandImplicitSplatsTest, BinaryMultiply_VectorScalar_Vec4i) {
    auto* scalar = b.FunctionParam("scalar", ty.i32());
    auto* vector = b.FunctionParam("vector", ty.vec4<i32>());
    auto* func = b.Function("foo", ty.vec4<i32>());
    func->SetParams({scalar, vector});

    b.Append(func->Block(), [&] {
        auto* result = b.Multiply(ty.vec4<i32>(), vector, scalar);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%scalar:i32, %vector:vec4<i32>):vec4<i32> -> %b1 {
  %b1 = block {
    %4:vec4<i32> = mul %vector, %scalar
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%scalar:i32, %vector:vec4<i32>):vec4<i32> -> %b1 {
  %b1 = block {
    %4:vec4<i32> = construct %scalar, %scalar, %scalar, %scalar
    %5:vec4<i32> = mul %vector, %4
    ret %5
  }
}
)";

    Run(ExpandImplicitSplats);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_ExpandImplicitSplatsTest, BinaryMultiply_ScalarVector_Vec4i) {
    auto* scalar = b.FunctionParam("scalar", ty.i32());
    auto* vector = b.FunctionParam("vector", ty.vec4<i32>());
    auto* func = b.Function("foo", ty.vec4<i32>());
    func->SetParams({scalar, vector});

    b.Append(func->Block(), [&] {
        auto* result = b.Multiply(ty.vec4<i32>(), scalar, vector);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%scalar:i32, %vector:vec4<i32>):vec4<i32> -> %b1 {
  %b1 = block {
    %4:vec4<i32> = mul %scalar, %vector
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%scalar:i32, %vector:vec4<i32>):vec4<i32> -> %b1 {
  %b1 = block {
    %4:vec4<i32> = construct %scalar, %scalar, %scalar, %scalar
    %5:vec4<i32> = mul %4, %vector
    ret %5
  }
}
)";

    Run(ExpandImplicitSplats);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_ExpandImplicitSplatsTest, Mix_VectorOperands_ScalarFactor) {
    auto* arg1 = b.FunctionParam("arg1", ty.vec4<f32>());
    auto* arg2 = b.FunctionParam("arg2", ty.vec4<f32>());
    auto* factor = b.FunctionParam("factor", ty.f32());
    auto* func = b.Function("foo", ty.vec4<f32>());
    func->SetParams({arg1, arg2, factor});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.vec4<f32>(), builtin::Function::kMix, arg1, arg2, factor);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%arg1:vec4<f32>, %arg2:vec4<f32>, %factor:f32):vec4<f32> -> %b1 {
  %b1 = block {
    %5:vec4<f32> = mix %arg1, %arg2, %factor
    ret %5
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%arg1:vec4<f32>, %arg2:vec4<f32>, %factor:f32):vec4<f32> -> %b1 {
  %b1 = block {
    %5:vec4<f32> = construct %factor, %factor, %factor, %factor
    %6:vec4<f32> = mix %arg1, %arg2, %5
    ret %6
  }
}
)";

    Run(ExpandImplicitSplats);

    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::ir::transform
