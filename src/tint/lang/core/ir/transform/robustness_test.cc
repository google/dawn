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

#include "src/tint/lang/core/ir/transform/robustness.h"

#include <utility>

#include "src/tint/lang/core/ir/transform/helper_test.h"
#include "src/tint/lang/core/type/array.h"
#include "src/tint/lang/core/type/matrix.h"
#include "src/tint/lang/core/type/pointer.h"
#include "src/tint/lang/core/type/struct.h"
#include "src/tint/lang/core/type/vector.h"

namespace tint::core::ir::transform {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

using IR_RobustnessTest = TransformTestWithParam<bool>;

////////////////////////////////////////////////////////////////
// These tests use the function address space.
// Test clamping of vectors, matrices, and fixed-size arrays.
// Test indices that are const, const-via-let, and dynamic.
// Test signed vs unsigned indices.
////////////////////////////////////////////////////////////////

TEST_P(IR_RobustnessTest, VectorLoad_ConstIndex) {
    auto* func = b.Function("foo", ty.u32());
    b.Append(func->Block(), [&] {
        auto* vec = b.Var("vec", ty.ptr(function, ty.vec4<u32>()));
        auto* load = b.LoadVectorElement(vec, b.Constant(5_u));
        b.Return(func, load);
    });

    auto* src = R"(
%foo = func():u32 -> %b1 {
  %b1 = block {
    %vec:ptr<function, vec4<u32>, read_write> = var
    %3:u32 = load_vector_element %vec, 5u
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func():u32 -> %b1 {
  %b1 = block {
    %vec:ptr<function, vec4<u32>, read_write> = var
    %3:u32 = load_vector_element %vec, 3u
    ret %3
  }
}
)";

    RobustnessConfig cfg;
    cfg.clamp_function = GetParam();
    Run(Robustness, cfg);

    EXPECT_EQ(GetParam() ? expect : src, str());
}

TEST_P(IR_RobustnessTest, VectorLoad_ConstIndexViaLet) {
    auto* func = b.Function("foo", ty.u32());
    b.Append(func->Block(), [&] {
        auto* vec = b.Var("vec", ty.ptr(function, ty.vec4<u32>()));
        auto* idx = b.Let("idx", b.Constant(5_u));
        auto* load = b.LoadVectorElement(vec, idx);
        b.Return(func, load);
    });

    auto* src = R"(
%foo = func():u32 -> %b1 {
  %b1 = block {
    %vec:ptr<function, vec4<u32>, read_write> = var
    %idx:u32 = let 5u
    %4:u32 = load_vector_element %vec, %idx
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func():u32 -> %b1 {
  %b1 = block {
    %vec:ptr<function, vec4<u32>, read_write> = var
    %idx:u32 = let 5u
    %4:u32 = min %idx, 3u
    %5:u32 = load_vector_element %vec, %4
    ret %5
  }
}
)";

    RobustnessConfig cfg;
    cfg.clamp_function = GetParam();
    Run(Robustness, cfg);

    EXPECT_EQ(GetParam() ? expect : src, str());
}

TEST_P(IR_RobustnessTest, VectorLoad_DynamicIndex) {
    auto* func = b.Function("foo", ty.u32());
    auto* idx = b.FunctionParam("idx", ty.u32());
    func->SetParams({idx});
    b.Append(func->Block(), [&] {
        auto* vec = b.Var("vec", ty.ptr(function, ty.vec4<u32>()));
        auto* load = b.LoadVectorElement(vec, idx);
        b.Return(func, load);
    });

    auto* src = R"(
%foo = func(%idx:u32):u32 -> %b1 {
  %b1 = block {
    %vec:ptr<function, vec4<u32>, read_write> = var
    %4:u32 = load_vector_element %vec, %idx
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%idx:u32):u32 -> %b1 {
  %b1 = block {
    %vec:ptr<function, vec4<u32>, read_write> = var
    %4:u32 = min %idx, 3u
    %5:u32 = load_vector_element %vec, %4
    ret %5
  }
}
)";

    RobustnessConfig cfg;
    cfg.clamp_function = GetParam();
    Run(Robustness, cfg);

    EXPECT_EQ(GetParam() ? expect : src, str());
}

TEST_P(IR_RobustnessTest, VectorLoad_DynamicIndex_Signed) {
    auto* func = b.Function("foo", ty.u32());
    auto* idx = b.FunctionParam("idx", ty.i32());
    func->SetParams({idx});
    b.Append(func->Block(), [&] {
        auto* vec = b.Var("vec", ty.ptr(function, ty.vec4<u32>()));
        auto* load = b.LoadVectorElement(vec, idx);
        b.Return(func, load);
    });

    auto* src = R"(
%foo = func(%idx:i32):u32 -> %b1 {
  %b1 = block {
    %vec:ptr<function, vec4<u32>, read_write> = var
    %4:u32 = load_vector_element %vec, %idx
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%idx:i32):u32 -> %b1 {
  %b1 = block {
    %vec:ptr<function, vec4<u32>, read_write> = var
    %4:u32 = convert %idx
    %5:u32 = min %4, 3u
    %6:u32 = load_vector_element %vec, %5
    ret %6
  }
}
)";

    RobustnessConfig cfg;
    cfg.clamp_function = GetParam();
    Run(Robustness, cfg);

    EXPECT_EQ(GetParam() ? expect : src, str());
}

TEST_P(IR_RobustnessTest, VectorStore_ConstIndex) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* vec = b.Var("vec", ty.ptr(function, ty.vec4<u32>()));
        b.StoreVectorElement(vec, b.Constant(5_u), b.Constant(0_u));
        b.Return(func);
    });

    auto* src = R"(
%foo = func():void -> %b1 {
  %b1 = block {
    %vec:ptr<function, vec4<u32>, read_write> = var
    store_vector_element %vec, 5u, 0u
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func():void -> %b1 {
  %b1 = block {
    %vec:ptr<function, vec4<u32>, read_write> = var
    store_vector_element %vec, 3u, 0u
    ret
  }
}
)";

    RobustnessConfig cfg;
    cfg.clamp_function = GetParam();
    Run(Robustness, cfg);

    EXPECT_EQ(GetParam() ? expect : src, str());
}

TEST_P(IR_RobustnessTest, VectorStore_ConstIndexViaLet) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* vec = b.Var("vec", ty.ptr(function, ty.vec4<u32>()));
        auto* idx = b.Let("idx", b.Constant(5_u));
        b.StoreVectorElement(vec, idx, b.Constant(0_u));
        b.Return(func);
    });

    auto* src = R"(
%foo = func():void -> %b1 {
  %b1 = block {
    %vec:ptr<function, vec4<u32>, read_write> = var
    %idx:u32 = let 5u
    store_vector_element %vec, %idx, 0u
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func():void -> %b1 {
  %b1 = block {
    %vec:ptr<function, vec4<u32>, read_write> = var
    %idx:u32 = let 5u
    %4:u32 = min %idx, 3u
    store_vector_element %vec, %4, 0u
    ret
  }
}
)";

    RobustnessConfig cfg;
    cfg.clamp_function = GetParam();
    Run(Robustness, cfg);

    EXPECT_EQ(GetParam() ? expect : src, str());
}

TEST_P(IR_RobustnessTest, VectorStore_DynamicIndex) {
    auto* func = b.Function("foo", ty.void_());
    auto* idx = b.FunctionParam("idx", ty.u32());
    func->SetParams({idx});
    b.Append(func->Block(), [&] {
        auto* vec = b.Var("vec", ty.ptr(function, ty.vec4<u32>()));
        b.StoreVectorElement(vec, idx, b.Constant(0_u));
        b.Return(func);
    });

    auto* src = R"(
%foo = func(%idx:u32):void -> %b1 {
  %b1 = block {
    %vec:ptr<function, vec4<u32>, read_write> = var
    store_vector_element %vec, %idx, 0u
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%idx:u32):void -> %b1 {
  %b1 = block {
    %vec:ptr<function, vec4<u32>, read_write> = var
    %4:u32 = min %idx, 3u
    store_vector_element %vec, %4, 0u
    ret
  }
}
)";

    RobustnessConfig cfg;
    cfg.clamp_function = GetParam();
    Run(Robustness, cfg);

    EXPECT_EQ(GetParam() ? expect : src, str());
}

TEST_P(IR_RobustnessTest, VectorStore_DynamicIndex_Signed) {
    auto* func = b.Function("foo", ty.void_());
    auto* idx = b.FunctionParam("idx", ty.i32());
    func->SetParams({idx});
    b.Append(func->Block(), [&] {
        auto* vec = b.Var("vec", ty.ptr(function, ty.vec4<u32>()));
        b.StoreVectorElement(vec, idx, b.Constant(0_u));
        b.Return(func);
    });

    auto* src = R"(
%foo = func(%idx:i32):void -> %b1 {
  %b1 = block {
    %vec:ptr<function, vec4<u32>, read_write> = var
    store_vector_element %vec, %idx, 0u
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%idx:i32):void -> %b1 {
  %b1 = block {
    %vec:ptr<function, vec4<u32>, read_write> = var
    %4:u32 = convert %idx
    %5:u32 = min %4, 3u
    store_vector_element %vec, %5, 0u
    ret
  }
}
)";

    RobustnessConfig cfg;
    cfg.clamp_function = GetParam();
    Run(Robustness, cfg);

    EXPECT_EQ(GetParam() ? expect : src, str());
}

TEST_P(IR_RobustnessTest, Matrix_ConstIndex) {
    auto* func = b.Function("foo", ty.vec4<f32>());
    b.Append(func->Block(), [&] {
        auto* mat = b.Var("mat", ty.ptr(function, ty.mat4x4<f32>()));
        auto* access = b.Access(ty.ptr(function, ty.vec4<f32>()), mat, b.Constant(2_u));
        auto* load = b.Load(access);
        b.Return(func, load);
    });

    auto* src = R"(
%foo = func():vec4<f32> -> %b1 {
  %b1 = block {
    %mat:ptr<function, mat4x4<f32>, read_write> = var
    %3:ptr<function, vec4<f32>, read_write> = access %mat, 2u
    %4:vec4<f32> = load %3
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    RobustnessConfig cfg;
    cfg.clamp_function = GetParam();
    Run(Robustness, cfg);

    EXPECT_EQ(expect, str());
}

TEST_P(IR_RobustnessTest, Matrix_ConstIndexViaLet) {
    auto* func = b.Function("foo", ty.vec4<f32>());
    b.Append(func->Block(), [&] {
        auto* mat = b.Var("mat", ty.ptr(function, ty.mat4x4<f32>()));
        auto* idx = b.Let("idx", b.Constant(2_u));
        auto* access = b.Access(ty.ptr(function, ty.vec4<f32>()), mat, idx);
        auto* load = b.Load(access);
        b.Return(func, load);
    });

    auto* src = R"(
%foo = func():vec4<f32> -> %b1 {
  %b1 = block {
    %mat:ptr<function, mat4x4<f32>, read_write> = var
    %idx:u32 = let 2u
    %4:ptr<function, vec4<f32>, read_write> = access %mat, %idx
    %5:vec4<f32> = load %4
    ret %5
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func():vec4<f32> -> %b1 {
  %b1 = block {
    %mat:ptr<function, mat4x4<f32>, read_write> = var
    %idx:u32 = let 2u
    %4:u32 = min %idx, 3u
    %5:ptr<function, vec4<f32>, read_write> = access %mat, %4
    %6:vec4<f32> = load %5
    ret %6
  }
}
)";

    RobustnessConfig cfg;
    cfg.clamp_function = GetParam();
    Run(Robustness, cfg);

    EXPECT_EQ(GetParam() ? expect : src, str());
}

TEST_P(IR_RobustnessTest, Matrix_DynamicIndex) {
    auto* func = b.Function("foo", ty.vec4<f32>());
    auto* idx = b.FunctionParam("idx", ty.u32());
    func->SetParams({idx});
    b.Append(func->Block(), [&] {
        auto* mat = b.Var("mat", ty.ptr(function, ty.mat4x4<f32>()));
        auto* access = b.Access(ty.ptr(function, ty.vec4<f32>()), mat, idx);
        auto* load = b.Load(access);
        b.Return(func, load);
    });

    auto* src = R"(
%foo = func(%idx:u32):vec4<f32> -> %b1 {
  %b1 = block {
    %mat:ptr<function, mat4x4<f32>, read_write> = var
    %4:ptr<function, vec4<f32>, read_write> = access %mat, %idx
    %5:vec4<f32> = load %4
    ret %5
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%idx:u32):vec4<f32> -> %b1 {
  %b1 = block {
    %mat:ptr<function, mat4x4<f32>, read_write> = var
    %4:u32 = min %idx, 3u
    %5:ptr<function, vec4<f32>, read_write> = access %mat, %4
    %6:vec4<f32> = load %5
    ret %6
  }
}
)";

    RobustnessConfig cfg;
    cfg.clamp_function = GetParam();
    Run(Robustness, cfg);

    EXPECT_EQ(GetParam() ? expect : src, str());
}

TEST_P(IR_RobustnessTest, Matrix_DynamicIndex_Signed) {
    auto* func = b.Function("foo", ty.vec4<f32>());
    auto* idx = b.FunctionParam("idx", ty.i32());
    func->SetParams({idx});
    b.Append(func->Block(), [&] {
        auto* mat = b.Var("mat", ty.ptr(function, ty.mat4x4<f32>()));
        auto* access = b.Access(ty.ptr(function, ty.vec4<f32>()), mat, idx);
        auto* load = b.Load(access);
        b.Return(func, load);
    });

    auto* src = R"(
%foo = func(%idx:i32):vec4<f32> -> %b1 {
  %b1 = block {
    %mat:ptr<function, mat4x4<f32>, read_write> = var
    %4:ptr<function, vec4<f32>, read_write> = access %mat, %idx
    %5:vec4<f32> = load %4
    ret %5
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%idx:i32):vec4<f32> -> %b1 {
  %b1 = block {
    %mat:ptr<function, mat4x4<f32>, read_write> = var
    %4:u32 = convert %idx
    %5:u32 = min %4, 3u
    %6:ptr<function, vec4<f32>, read_write> = access %mat, %5
    %7:vec4<f32> = load %6
    ret %7
  }
}
)";

    RobustnessConfig cfg;
    cfg.clamp_function = GetParam();
    Run(Robustness, cfg);

    EXPECT_EQ(GetParam() ? expect : src, str());
}

TEST_P(IR_RobustnessTest, Array_ConstSize_ConstIndex) {
    auto* func = b.Function("foo", ty.u32());
    b.Append(func->Block(), [&] {
        auto* arr = b.Var("arr", ty.ptr(function, ty.array<u32, 4>()));
        auto* access = b.Access(ty.ptr<function, u32>(), arr, b.Constant(2_u));
        auto* load = b.Load(access);
        b.Return(func, load);
    });

    auto* src = R"(
%foo = func():u32 -> %b1 {
  %b1 = block {
    %arr:ptr<function, array<u32, 4>, read_write> = var
    %3:ptr<function, u32, read_write> = access %arr, 2u
    %4:u32 = load %3
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    RobustnessConfig cfg;
    cfg.clamp_function = GetParam();
    Run(Robustness, cfg);

    EXPECT_EQ(expect, str());
}

TEST_P(IR_RobustnessTest, Array_ConstSize_ConstIndexViaLet) {
    auto* func = b.Function("foo", ty.u32());
    b.Append(func->Block(), [&] {
        auto* arr = b.Var("arr", ty.ptr(function, ty.array<u32, 4>()));
        auto* idx = b.Let("idx", b.Constant(2_u));
        auto* access = b.Access(ty.ptr<function, u32>(), arr, idx);
        auto* load = b.Load(access);
        b.Return(func, load);
    });

    auto* src = R"(
%foo = func():u32 -> %b1 {
  %b1 = block {
    %arr:ptr<function, array<u32, 4>, read_write> = var
    %idx:u32 = let 2u
    %4:ptr<function, u32, read_write> = access %arr, %idx
    %5:u32 = load %4
    ret %5
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func():u32 -> %b1 {
  %b1 = block {
    %arr:ptr<function, array<u32, 4>, read_write> = var
    %idx:u32 = let 2u
    %4:u32 = min %idx, 3u
    %5:ptr<function, u32, read_write> = access %arr, %4
    %6:u32 = load %5
    ret %6
  }
}
)";

    RobustnessConfig cfg;
    cfg.clamp_function = GetParam();
    Run(Robustness, cfg);

    EXPECT_EQ(GetParam() ? expect : src, str());
}

TEST_P(IR_RobustnessTest, Array_ConstSize_DynamicIndex) {
    auto* func = b.Function("foo", ty.u32());
    auto* idx = b.FunctionParam("idx", ty.u32());
    func->SetParams({idx});
    b.Append(func->Block(), [&] {
        auto* arr = b.Var("arr", ty.ptr(function, ty.array<u32, 4>()));
        auto* access = b.Access(ty.ptr<function, u32>(), arr, idx);
        auto* load = b.Load(access);
        b.Return(func, load);
    });

    auto* src = R"(
%foo = func(%idx:u32):u32 -> %b1 {
  %b1 = block {
    %arr:ptr<function, array<u32, 4>, read_write> = var
    %4:ptr<function, u32, read_write> = access %arr, %idx
    %5:u32 = load %4
    ret %5
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%idx:u32):u32 -> %b1 {
  %b1 = block {
    %arr:ptr<function, array<u32, 4>, read_write> = var
    %4:u32 = min %idx, 3u
    %5:ptr<function, u32, read_write> = access %arr, %4
    %6:u32 = load %5
    ret %6
  }
}
)";

    RobustnessConfig cfg;
    cfg.clamp_function = GetParam();
    Run(Robustness, cfg);

    EXPECT_EQ(GetParam() ? expect : src, str());
}

TEST_P(IR_RobustnessTest, Array_ConstSize_DynamicIndex_Signed) {
    auto* func = b.Function("foo", ty.u32());
    auto* idx = b.FunctionParam("idx", ty.i32());
    func->SetParams({idx});
    b.Append(func->Block(), [&] {
        auto* arr = b.Var("arr", ty.ptr(function, ty.array<u32, 4>()));
        auto* access = b.Access(ty.ptr<function, u32>(), arr, idx);
        auto* load = b.Load(access);
        b.Return(func, load);
    });

    auto* src = R"(
%foo = func(%idx:i32):u32 -> %b1 {
  %b1 = block {
    %arr:ptr<function, array<u32, 4>, read_write> = var
    %4:ptr<function, u32, read_write> = access %arr, %idx
    %5:u32 = load %4
    ret %5
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%idx:i32):u32 -> %b1 {
  %b1 = block {
    %arr:ptr<function, array<u32, 4>, read_write> = var
    %4:u32 = convert %idx
    %5:u32 = min %4, 3u
    %6:ptr<function, u32, read_write> = access %arr, %5
    %7:u32 = load %6
    ret %7
  }
}
)";

    RobustnessConfig cfg;
    cfg.clamp_function = GetParam();
    Run(Robustness, cfg);

    EXPECT_EQ(GetParam() ? expect : src, str());
}

TEST_P(IR_RobustnessTest, NestedArrays) {
    auto* func = b.Function("foo", ty.u32());
    auto* idx1 = b.FunctionParam("idx1", ty.u32());
    auto* idx2 = b.FunctionParam("idx2", ty.u32());
    auto* idx3 = b.FunctionParam("idx3", ty.u32());
    auto* idx4 = b.FunctionParam("idx4", ty.u32());
    func->SetParams({idx1, idx2, idx3, idx4});
    b.Append(func->Block(), [&] {
        auto* arr = b.Var(
            "arr", ty.ptr(function, ty.array(ty.array(ty.array(ty.array(ty.u32(), 4), 5), 6), 7)));
        auto* access = b.Access(ty.ptr<function, u32>(), arr, idx1, idx2, idx3, idx4);
        auto* load = b.Load(access);
        b.Return(func, load);
    });

    auto* src = R"(
%foo = func(%idx1:u32, %idx2:u32, %idx3:u32, %idx4:u32):u32 -> %b1 {
  %b1 = block {
    %arr:ptr<function, array<array<array<array<u32, 4>, 5>, 6>, 7>, read_write> = var
    %7:ptr<function, u32, read_write> = access %arr, %idx1, %idx2, %idx3, %idx4
    %8:u32 = load %7
    ret %8
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%idx1:u32, %idx2:u32, %idx3:u32, %idx4:u32):u32 -> %b1 {
  %b1 = block {
    %arr:ptr<function, array<array<array<array<u32, 4>, 5>, 6>, 7>, read_write> = var
    %7:u32 = min %idx1, 6u
    %8:u32 = min %idx2, 5u
    %9:u32 = min %idx3, 4u
    %10:u32 = min %idx4, 3u
    %11:ptr<function, u32, read_write> = access %arr, %7, %8, %9, %10
    %12:u32 = load %11
    ret %12
  }
}
)";

    RobustnessConfig cfg;
    cfg.clamp_function = GetParam();
    Run(Robustness, cfg);

    EXPECT_EQ(GetParam() ? expect : src, str());
}

TEST_P(IR_RobustnessTest, NestedMixedTypes) {
    auto* structure = ty.Struct(mod.symbols.Register("structure"),
                                {
                                    {mod.symbols.Register("arr"), ty.array(ty.mat3x4<f32>(), 4)},
                                });
    auto* func = b.Function("foo", ty.vec4<f32>());
    auto* idx1 = b.FunctionParam("idx1", ty.u32());
    auto* idx2 = b.FunctionParam("idx2", ty.u32());
    auto* idx3 = b.FunctionParam("idx3", ty.u32());
    func->SetParams({idx1, idx2, idx3});
    b.Append(func->Block(), [&] {
        auto* arr = b.Var("arr", ty.ptr(function, ty.array(structure, 8)));
        auto* access =
            b.Access(ty.ptr<function, vec4<f32>>(), arr, idx1, b.Constant(0_u), idx2, idx3);
        auto* load = b.Load(access);
        b.Return(func, load);
    });

    auto* src = R"(
structure = struct @align(16) {
  arr:array<mat3x4<f32>, 4> @offset(0)
}

%foo = func(%idx1:u32, %idx2:u32, %idx3:u32):vec4<f32> -> %b1 {
  %b1 = block {
    %arr:ptr<function, array<structure, 8>, read_write> = var
    %6:ptr<function, vec4<f32>, read_write> = access %arr, %idx1, 0u, %idx2, %idx3
    %7:vec4<f32> = load %6
    ret %7
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
structure = struct @align(16) {
  arr:array<mat3x4<f32>, 4> @offset(0)
}

%foo = func(%idx1:u32, %idx2:u32, %idx3:u32):vec4<f32> -> %b1 {
  %b1 = block {
    %arr:ptr<function, array<structure, 8>, read_write> = var
    %6:u32 = min %idx1, 7u
    %7:u32 = min %idx2, 3u
    %8:u32 = min %idx3, 2u
    %9:ptr<function, vec4<f32>, read_write> = access %arr, %6, 0u, %7, %8
    %10:vec4<f32> = load %9
    ret %10
  }
}
)";

    RobustnessConfig cfg;
    cfg.clamp_function = GetParam();
    Run(Robustness, cfg);

    EXPECT_EQ(GetParam() ? expect : src, str());
}

////////////////////////////////////////////////////////////////
// Test the clamp toggles for every other address space.
////////////////////////////////////////////////////////////////

TEST_P(IR_RobustnessTest, Private_LoadVectorElement) {
    auto* vec = b.Var("vec", ty.ptr(private_, ty.vec4<u32>()));
    b.RootBlock()->Append(vec);

    auto* func = b.Function("foo", ty.u32());
    auto* idx = b.FunctionParam("idx", ty.u32());
    func->SetParams({idx});
    b.Append(func->Block(), [&] {
        auto* load = b.LoadVectorElement(vec, idx);
        b.Return(func, load);
    });

    auto* src = R"(
%b1 = block {  # root
  %vec:ptr<private, vec4<u32>, read_write> = var
}

%foo = func(%idx:u32):u32 -> %b2 {
  %b2 = block {
    %4:u32 = load_vector_element %vec, %idx
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %vec:ptr<private, vec4<u32>, read_write> = var
}

%foo = func(%idx:u32):u32 -> %b2 {
  %b2 = block {
    %4:u32 = min %idx, 3u
    %5:u32 = load_vector_element %vec, %4
    ret %5
  }
}
)";

    RobustnessConfig cfg;
    cfg.clamp_private = GetParam();
    Run(Robustness, cfg);

    EXPECT_EQ(GetParam() ? expect : src, str());
}

TEST_P(IR_RobustnessTest, Private_StoreVectorElement) {
    auto* vec = b.Var("vec", ty.ptr(private_, ty.vec4<u32>()));
    b.RootBlock()->Append(vec);

    auto* func = b.Function("foo", ty.void_());
    auto* idx = b.FunctionParam("idx", ty.u32());
    func->SetParams({idx});
    b.Append(func->Block(), [&] {
        b.StoreVectorElement(vec, idx, b.Constant(0_u));
        b.Return(func);
    });

    auto* src = R"(
%b1 = block {  # root
  %vec:ptr<private, vec4<u32>, read_write> = var
}

%foo = func(%idx:u32):void -> %b2 {
  %b2 = block {
    store_vector_element %vec, %idx, 0u
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %vec:ptr<private, vec4<u32>, read_write> = var
}

%foo = func(%idx:u32):void -> %b2 {
  %b2 = block {
    %4:u32 = min %idx, 3u
    store_vector_element %vec, %4, 0u
    ret
  }
}
)";

    RobustnessConfig cfg;
    cfg.clamp_private = GetParam();
    Run(Robustness, cfg);

    EXPECT_EQ(GetParam() ? expect : src, str());
}

TEST_P(IR_RobustnessTest, Private_Access) {
    auto* arr = b.Var("arr", ty.ptr(private_, ty.array<u32, 4>()));
    b.RootBlock()->Append(arr);

    auto* func = b.Function("foo", ty.u32());
    auto* idx = b.FunctionParam("idx", ty.u32());
    func->SetParams({idx});
    b.Append(func->Block(), [&] {
        auto* access = b.Access(ty.ptr<private_, u32>(), arr, idx);
        auto* load = b.Load(access);
        b.Return(func, load);
    });

    auto* src = R"(
%b1 = block {  # root
  %arr:ptr<private, array<u32, 4>, read_write> = var
}

%foo = func(%idx:u32):u32 -> %b2 {
  %b2 = block {
    %4:ptr<private, u32, read_write> = access %arr, %idx
    %5:u32 = load %4
    ret %5
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %arr:ptr<private, array<u32, 4>, read_write> = var
}

%foo = func(%idx:u32):u32 -> %b2 {
  %b2 = block {
    %4:u32 = min %idx, 3u
    %5:ptr<private, u32, read_write> = access %arr, %4
    %6:u32 = load %5
    ret %6
  }
}
)";

    RobustnessConfig cfg;
    cfg.clamp_private = GetParam();
    Run(Robustness, cfg);

    EXPECT_EQ(GetParam() ? expect : src, str());
}

TEST_P(IR_RobustnessTest, PushConstant_LoadVectorElement) {
    auto* vec = b.Var("vec", ty.ptr(push_constant, ty.vec4<u32>()));
    b.RootBlock()->Append(vec);

    auto* func = b.Function("foo", ty.u32());
    auto* idx = b.FunctionParam("idx", ty.u32());
    func->SetParams({idx});
    b.Append(func->Block(), [&] {
        auto* load = b.LoadVectorElement(vec, idx);
        b.Return(func, load);
    });

    auto* src = R"(
%b1 = block {  # root
  %vec:ptr<push_constant, vec4<u32>, read_write> = var
}

%foo = func(%idx:u32):u32 -> %b2 {
  %b2 = block {
    %4:u32 = load_vector_element %vec, %idx
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %vec:ptr<push_constant, vec4<u32>, read_write> = var
}

%foo = func(%idx:u32):u32 -> %b2 {
  %b2 = block {
    %4:u32 = min %idx, 3u
    %5:u32 = load_vector_element %vec, %4
    ret %5
  }
}
)";

    RobustnessConfig cfg;
    cfg.clamp_push_constant = GetParam();
    Run(Robustness, cfg);

    EXPECT_EQ(GetParam() ? expect : src, str());
}

TEST_P(IR_RobustnessTest, PushConstant_StoreVectorElement) {
    auto* vec = b.Var("vec", ty.ptr(push_constant, ty.vec4<u32>()));
    b.RootBlock()->Append(vec);

    auto* func = b.Function("foo", ty.void_());
    auto* idx = b.FunctionParam("idx", ty.u32());
    func->SetParams({idx});
    b.Append(func->Block(), [&] {
        b.StoreVectorElement(vec, idx, b.Constant(0_u));
        b.Return(func);
    });

    auto* src = R"(
%b1 = block {  # root
  %vec:ptr<push_constant, vec4<u32>, read_write> = var
}

%foo = func(%idx:u32):void -> %b2 {
  %b2 = block {
    store_vector_element %vec, %idx, 0u
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %vec:ptr<push_constant, vec4<u32>, read_write> = var
}

%foo = func(%idx:u32):void -> %b2 {
  %b2 = block {
    %4:u32 = min %idx, 3u
    store_vector_element %vec, %4, 0u
    ret
  }
}
)";

    RobustnessConfig cfg;
    cfg.clamp_push_constant = GetParam();
    Run(Robustness, cfg);

    EXPECT_EQ(GetParam() ? expect : src, str());
}

TEST_P(IR_RobustnessTest, PushConstant_Access) {
    auto* arr = b.Var("arr", ty.ptr(push_constant, ty.array<u32, 4>()));
    b.RootBlock()->Append(arr);

    auto* func = b.Function("foo", ty.u32());
    auto* idx = b.FunctionParam("idx", ty.u32());
    func->SetParams({idx});
    b.Append(func->Block(), [&] {
        auto* access = b.Access(ty.ptr<push_constant, u32>(), arr, idx);
        auto* load = b.Load(access);
        b.Return(func, load);
    });

    auto* src = R"(
%b1 = block {  # root
  %arr:ptr<push_constant, array<u32, 4>, read_write> = var
}

%foo = func(%idx:u32):u32 -> %b2 {
  %b2 = block {
    %4:ptr<push_constant, u32, read_write> = access %arr, %idx
    %5:u32 = load %4
    ret %5
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %arr:ptr<push_constant, array<u32, 4>, read_write> = var
}

%foo = func(%idx:u32):u32 -> %b2 {
  %b2 = block {
    %4:u32 = min %idx, 3u
    %5:ptr<push_constant, u32, read_write> = access %arr, %4
    %6:u32 = load %5
    ret %6
  }
}
)";

    RobustnessConfig cfg;
    cfg.clamp_push_constant = GetParam();
    Run(Robustness, cfg);

    EXPECT_EQ(GetParam() ? expect : src, str());
}

TEST_P(IR_RobustnessTest, Storage_LoadVectorElement) {
    auto* vec = b.Var("vec", ty.ptr(storage, ty.vec4<u32>()));
    vec->SetBindingPoint(0, 0);
    b.RootBlock()->Append(vec);

    auto* func = b.Function("foo", ty.u32());
    auto* idx = b.FunctionParam("idx", ty.u32());
    func->SetParams({idx});
    b.Append(func->Block(), [&] {
        auto* load = b.LoadVectorElement(vec, idx);
        b.Return(func, load);
    });

    auto* src = R"(
%b1 = block {  # root
  %vec:ptr<storage, vec4<u32>, read_write> = var @binding_point(0, 0)
}

%foo = func(%idx:u32):u32 -> %b2 {
  %b2 = block {
    %4:u32 = load_vector_element %vec, %idx
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %vec:ptr<storage, vec4<u32>, read_write> = var @binding_point(0, 0)
}

%foo = func(%idx:u32):u32 -> %b2 {
  %b2 = block {
    %4:u32 = min %idx, 3u
    %5:u32 = load_vector_element %vec, %4
    ret %5
  }
}
)";

    RobustnessConfig cfg;
    cfg.clamp_storage = GetParam();
    Run(Robustness, cfg);

    EXPECT_EQ(GetParam() ? expect : src, str());
}

TEST_P(IR_RobustnessTest, Storage_StoreVectorElement) {
    auto* vec = b.Var("vec", ty.ptr(storage, ty.vec4<u32>()));
    vec->SetBindingPoint(0, 0);
    b.RootBlock()->Append(vec);

    auto* func = b.Function("foo", ty.void_());
    auto* idx = b.FunctionParam("idx", ty.u32());
    func->SetParams({idx});
    b.Append(func->Block(), [&] {
        b.StoreVectorElement(vec, idx, b.Constant(0_u));
        b.Return(func);
    });

    auto* src = R"(
%b1 = block {  # root
  %vec:ptr<storage, vec4<u32>, read_write> = var @binding_point(0, 0)
}

%foo = func(%idx:u32):void -> %b2 {
  %b2 = block {
    store_vector_element %vec, %idx, 0u
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %vec:ptr<storage, vec4<u32>, read_write> = var @binding_point(0, 0)
}

%foo = func(%idx:u32):void -> %b2 {
  %b2 = block {
    %4:u32 = min %idx, 3u
    store_vector_element %vec, %4, 0u
    ret
  }
}
)";

    RobustnessConfig cfg;
    cfg.clamp_storage = GetParam();
    Run(Robustness, cfg);

    EXPECT_EQ(GetParam() ? expect : src, str());
}

TEST_P(IR_RobustnessTest, Storage_Access) {
    auto* arr = b.Var("arr", ty.ptr(storage, ty.array<u32, 4>()));
    arr->SetBindingPoint(0, 0);
    b.RootBlock()->Append(arr);

    auto* func = b.Function("foo", ty.u32());
    auto* idx = b.FunctionParam("idx", ty.u32());
    func->SetParams({idx});
    b.Append(func->Block(), [&] {
        auto* access = b.Access(ty.ptr<storage, u32>(), arr, idx);
        auto* load = b.Load(access);
        b.Return(func, load);
    });

    auto* src = R"(
%b1 = block {  # root
  %arr:ptr<storage, array<u32, 4>, read_write> = var @binding_point(0, 0)
}

%foo = func(%idx:u32):u32 -> %b2 {
  %b2 = block {
    %4:ptr<storage, u32, read_write> = access %arr, %idx
    %5:u32 = load %4
    ret %5
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %arr:ptr<storage, array<u32, 4>, read_write> = var @binding_point(0, 0)
}

%foo = func(%idx:u32):u32 -> %b2 {
  %b2 = block {
    %4:u32 = min %idx, 3u
    %5:ptr<storage, u32, read_write> = access %arr, %4
    %6:u32 = load %5
    ret %6
  }
}
)";

    RobustnessConfig cfg;
    cfg.clamp_storage = GetParam();
    Run(Robustness, cfg);

    EXPECT_EQ(GetParam() ? expect : src, str());
}

TEST_P(IR_RobustnessTest, Unifom_LoadVectorElement) {
    auto* vec = b.Var("vec", ty.ptr(uniform, ty.vec4<u32>()));
    vec->SetBindingPoint(0, 0);
    b.RootBlock()->Append(vec);

    auto* func = b.Function("foo", ty.u32());
    auto* idx = b.FunctionParam("idx", ty.u32());
    func->SetParams({idx});
    b.Append(func->Block(), [&] {
        auto* load = b.LoadVectorElement(vec, idx);
        b.Return(func, load);
    });

    auto* src = R"(
%b1 = block {  # root
  %vec:ptr<uniform, vec4<u32>, read_write> = var @binding_point(0, 0)
}

%foo = func(%idx:u32):u32 -> %b2 {
  %b2 = block {
    %4:u32 = load_vector_element %vec, %idx
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %vec:ptr<uniform, vec4<u32>, read_write> = var @binding_point(0, 0)
}

%foo = func(%idx:u32):u32 -> %b2 {
  %b2 = block {
    %4:u32 = min %idx, 3u
    %5:u32 = load_vector_element %vec, %4
    ret %5
  }
}
)";

    RobustnessConfig cfg;
    cfg.clamp_uniform = GetParam();
    Run(Robustness, cfg);

    EXPECT_EQ(GetParam() ? expect : src, str());
}

TEST_P(IR_RobustnessTest, Unifom_StoreVectorElement) {
    auto* vec = b.Var("vec", ty.ptr(uniform, ty.vec4<u32>()));
    vec->SetBindingPoint(0, 0);
    b.RootBlock()->Append(vec);

    auto* func = b.Function("foo", ty.void_());
    auto* idx = b.FunctionParam("idx", ty.u32());
    func->SetParams({idx});
    b.Append(func->Block(), [&] {
        b.StoreVectorElement(vec, idx, b.Constant(0_u));
        b.Return(func);
    });

    auto* src = R"(
%b1 = block {  # root
  %vec:ptr<uniform, vec4<u32>, read_write> = var @binding_point(0, 0)
}

%foo = func(%idx:u32):void -> %b2 {
  %b2 = block {
    store_vector_element %vec, %idx, 0u
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %vec:ptr<uniform, vec4<u32>, read_write> = var @binding_point(0, 0)
}

%foo = func(%idx:u32):void -> %b2 {
  %b2 = block {
    %4:u32 = min %idx, 3u
    store_vector_element %vec, %4, 0u
    ret
  }
}
)";

    RobustnessConfig cfg;
    cfg.clamp_uniform = GetParam();
    Run(Robustness, cfg);

    EXPECT_EQ(GetParam() ? expect : src, str());
}

TEST_P(IR_RobustnessTest, Unifom_Access) {
    auto* arr = b.Var("arr", ty.ptr(uniform, ty.array<u32, 4>()));
    arr->SetBindingPoint(0, 0);
    b.RootBlock()->Append(arr);

    auto* func = b.Function("foo", ty.u32());
    auto* idx = b.FunctionParam("idx", ty.u32());
    func->SetParams({idx});
    b.Append(func->Block(), [&] {
        auto* access = b.Access(ty.ptr<uniform, u32>(), arr, idx);
        auto* load = b.Load(access);
        b.Return(func, load);
    });

    auto* src = R"(
%b1 = block {  # root
  %arr:ptr<uniform, array<u32, 4>, read_write> = var @binding_point(0, 0)
}

%foo = func(%idx:u32):u32 -> %b2 {
  %b2 = block {
    %4:ptr<uniform, u32, read_write> = access %arr, %idx
    %5:u32 = load %4
    ret %5
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %arr:ptr<uniform, array<u32, 4>, read_write> = var @binding_point(0, 0)
}

%foo = func(%idx:u32):u32 -> %b2 {
  %b2 = block {
    %4:u32 = min %idx, 3u
    %5:ptr<uniform, u32, read_write> = access %arr, %4
    %6:u32 = load %5
    ret %6
  }
}
)";

    RobustnessConfig cfg;
    cfg.clamp_uniform = GetParam();
    Run(Robustness, cfg);

    EXPECT_EQ(GetParam() ? expect : src, str());
}

TEST_P(IR_RobustnessTest, Workgroup_LoadVectorElement) {
    auto* vec = b.Var("vec", ty.ptr(workgroup, ty.vec4<u32>()));
    b.RootBlock()->Append(vec);

    auto* func = b.Function("foo", ty.u32());
    auto* idx = b.FunctionParam("idx", ty.u32());
    func->SetParams({idx});
    b.Append(func->Block(), [&] {
        auto* load = b.LoadVectorElement(vec, idx);
        b.Return(func, load);
    });

    auto* src = R"(
%b1 = block {  # root
  %vec:ptr<workgroup, vec4<u32>, read_write> = var
}

%foo = func(%idx:u32):u32 -> %b2 {
  %b2 = block {
    %4:u32 = load_vector_element %vec, %idx
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %vec:ptr<workgroup, vec4<u32>, read_write> = var
}

%foo = func(%idx:u32):u32 -> %b2 {
  %b2 = block {
    %4:u32 = min %idx, 3u
    %5:u32 = load_vector_element %vec, %4
    ret %5
  }
}
)";

    RobustnessConfig cfg;
    cfg.clamp_workgroup = GetParam();
    Run(Robustness, cfg);

    EXPECT_EQ(GetParam() ? expect : src, str());
}

TEST_P(IR_RobustnessTest, Workgroup_StoreVectorElement) {
    auto* vec = b.Var("vec", ty.ptr(workgroup, ty.vec4<u32>()));
    b.RootBlock()->Append(vec);

    auto* func = b.Function("foo", ty.void_());
    auto* idx = b.FunctionParam("idx", ty.u32());
    func->SetParams({idx});
    b.Append(func->Block(), [&] {
        b.StoreVectorElement(vec, idx, b.Constant(0_u));
        b.Return(func);
    });

    auto* src = R"(
%b1 = block {  # root
  %vec:ptr<workgroup, vec4<u32>, read_write> = var
}

%foo = func(%idx:u32):void -> %b2 {
  %b2 = block {
    store_vector_element %vec, %idx, 0u
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %vec:ptr<workgroup, vec4<u32>, read_write> = var
}

%foo = func(%idx:u32):void -> %b2 {
  %b2 = block {
    %4:u32 = min %idx, 3u
    store_vector_element %vec, %4, 0u
    ret
  }
}
)";

    RobustnessConfig cfg;
    cfg.clamp_workgroup = GetParam();
    Run(Robustness, cfg);

    EXPECT_EQ(GetParam() ? expect : src, str());
}

TEST_P(IR_RobustnessTest, Workgroup_Access) {
    auto* arr = b.Var("arr", ty.ptr(workgroup, ty.array<u32, 4>()));
    b.RootBlock()->Append(arr);

    auto* func = b.Function("foo", ty.u32());
    auto* idx = b.FunctionParam("idx", ty.u32());
    func->SetParams({idx});
    b.Append(func->Block(), [&] {
        auto* access = b.Access(ty.ptr<workgroup, u32>(), arr, idx);
        auto* load = b.Load(access);
        b.Return(func, load);
    });

    auto* src = R"(
%b1 = block {  # root
  %arr:ptr<workgroup, array<u32, 4>, read_write> = var
}

%foo = func(%idx:u32):u32 -> %b2 {
  %b2 = block {
    %4:ptr<workgroup, u32, read_write> = access %arr, %idx
    %5:u32 = load %4
    ret %5
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %arr:ptr<workgroup, array<u32, 4>, read_write> = var
}

%foo = func(%idx:u32):u32 -> %b2 {
  %b2 = block {
    %4:u32 = min %idx, 3u
    %5:ptr<workgroup, u32, read_write> = access %arr, %4
    %6:u32 = load %5
    ret %6
  }
}
)";

    RobustnessConfig cfg;
    cfg.clamp_workgroup = GetParam();
    Run(Robustness, cfg);

    EXPECT_EQ(GetParam() ? expect : src, str());
}

////////////////////////////////////////////////////////////////
// Test clamping non-pointer values.
////////////////////////////////////////////////////////////////

TEST_P(IR_RobustnessTest, ConstantVector_DynamicIndex) {
    auto* func = b.Function("foo", ty.u32());
    auto* idx = b.FunctionParam("idx", ty.u32());
    func->SetParams({idx});
    b.Append(func->Block(), [&] {
        auto* vec = mod.constant_values.Composite(ty.vec4<u32>(), Vector{
                                                                      mod.constant_values.Get(1_u),
                                                                      mod.constant_values.Get(2_u),
                                                                      mod.constant_values.Get(3_u),
                                                                      mod.constant_values.Get(4_u),
                                                                  });
        auto* element = b.Access(ty.u32(), b.Constant(vec), idx);
        b.Return(func, element);
    });

    auto* src = R"(
%foo = func(%idx:u32):u32 -> %b1 {
  %b1 = block {
    %3:u32 = access vec4<u32>(1u, 2u, 3u, 4u), %idx
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%idx:u32):u32 -> %b1 {
  %b1 = block {
    %3:u32 = min %idx, 3u
    %4:u32 = access vec4<u32>(1u, 2u, 3u, 4u), %3
    ret %4
  }
}
)";

    RobustnessConfig cfg;
    cfg.clamp_value = GetParam();
    Run(Robustness, cfg);

    EXPECT_EQ(GetParam() ? expect : src, str());
}

TEST_P(IR_RobustnessTest, ConstantArray_DynamicIndex) {
    auto* func = b.Function("foo", ty.u32());
    auto* idx = b.FunctionParam("idx", ty.u32());
    func->SetParams({idx});
    b.Append(func->Block(), [&] {
        auto* arr =
            mod.constant_values.Composite(ty.array<u32, 4>(), Vector{
                                                                  mod.constant_values.Get(1_u),
                                                                  mod.constant_values.Get(2_u),
                                                                  mod.constant_values.Get(3_u),
                                                                  mod.constant_values.Get(4_u),
                                                              });
        auto* element = b.Access(ty.u32(), b.Constant(arr), idx);
        b.Return(func, element);
    });

    auto* src = R"(
%foo = func(%idx:u32):u32 -> %b1 {
  %b1 = block {
    %3:u32 = access array<u32, 4>(1u, 2u, 3u, 4u), %idx
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%idx:u32):u32 -> %b1 {
  %b1 = block {
    %3:u32 = min %idx, 3u
    %4:u32 = access array<u32, 4>(1u, 2u, 3u, 4u), %3
    ret %4
  }
}
)";

    RobustnessConfig cfg;
    cfg.clamp_value = GetParam();
    Run(Robustness, cfg);

    EXPECT_EQ(GetParam() ? expect : src, str());
}

TEST_P(IR_RobustnessTest, ParamValueArray_DynamicIndex) {
    auto* func = b.Function("foo", ty.u32());
    auto* arr = b.FunctionParam("arr", ty.array<u32, 4>());
    auto* idx = b.FunctionParam("idx", ty.u32());
    func->SetParams({arr, idx});
    b.Append(func->Block(), [&] {
        auto* element = b.Access(ty.u32(), arr, idx);
        b.Return(func, element);
    });

    auto* src = R"(
%foo = func(%arr:array<u32, 4>, %idx:u32):u32 -> %b1 {
  %b1 = block {
    %4:u32 = access %arr, %idx
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%arr:array<u32, 4>, %idx:u32):u32 -> %b1 {
  %b1 = block {
    %4:u32 = min %idx, 3u
    %5:u32 = access %arr, %4
    ret %5
  }
}
)";

    RobustnessConfig cfg;
    cfg.clamp_value = GetParam();
    Run(Robustness, cfg);

    EXPECT_EQ(GetParam() ? expect : src, str());
}

INSTANTIATE_TEST_SUITE_P(, IR_RobustnessTest, testing::Values(false, true));

////////////////////////////////////////////////////////////////
// Test clamping non-pointer arrays.
////////////////////////////////////////////////////////////////

TEST_P(IR_RobustnessTest, RuntimeSizedArray_ConstIndex) {
    auto* arr = b.Var("arr", ty.ptr(storage, ty.array<u32>()));
    arr->SetBindingPoint(0, 0);
    b.RootBlock()->Append(arr);

    auto* func = b.Function("foo", ty.u32());
    b.Append(func->Block(), [&] {
        auto* access = b.Access(ty.ptr<storage, u32>(), arr, b.Constant(42_u));
        auto* load = b.Load(access);
        b.Return(func, load);
    });

    auto* src = R"(
%b1 = block {  # root
  %arr:ptr<storage, array<u32>, read_write> = var @binding_point(0, 0)
}

%foo = func():u32 -> %b2 {
  %b2 = block {
    %3:ptr<storage, u32, read_write> = access %arr, 42u
    %4:u32 = load %3
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %arr:ptr<storage, array<u32>, read_write> = var @binding_point(0, 0)
}

%foo = func():u32 -> %b2 {
  %b2 = block {
    %3:u32 = arrayLength %arr
    %4:u32 = sub %3, 1u
    %5:u32 = min 42u, %4
    %6:ptr<storage, u32, read_write> = access %arr, %5
    %7:u32 = load %6
    ret %7
  }
}
)";

    RobustnessConfig cfg;
    cfg.clamp_storage = GetParam();
    Run(Robustness, cfg);

    EXPECT_EQ(GetParam() ? expect : src, str());
}

TEST_P(IR_RobustnessTest, RuntimeSizedArray_DynamicIndex) {
    auto* arr = b.Var("arr", ty.ptr(storage, ty.array<u32>()));
    arr->SetBindingPoint(0, 0);
    b.RootBlock()->Append(arr);

    auto* func = b.Function("foo", ty.u32());
    auto* idx = b.FunctionParam("idx", ty.u32());
    func->SetParams({idx});
    b.Append(func->Block(), [&] {
        auto* access = b.Access(ty.ptr<storage, u32>(), arr, idx);
        auto* load = b.Load(access);
        b.Return(func, load);
    });

    auto* src = R"(
%b1 = block {  # root
  %arr:ptr<storage, array<u32>, read_write> = var @binding_point(0, 0)
}

%foo = func(%idx:u32):u32 -> %b2 {
  %b2 = block {
    %4:ptr<storage, u32, read_write> = access %arr, %idx
    %5:u32 = load %4
    ret %5
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %arr:ptr<storage, array<u32>, read_write> = var @binding_point(0, 0)
}

%foo = func(%idx:u32):u32 -> %b2 {
  %b2 = block {
    %4:u32 = arrayLength %arr
    %5:u32 = sub %4, 1u
    %6:u32 = min %idx, %5
    %7:ptr<storage, u32, read_write> = access %arr, %6
    %8:u32 = load %7
    ret %8
  }
}
)";

    RobustnessConfig cfg;
    cfg.clamp_storage = GetParam();
    Run(Robustness, cfg);

    EXPECT_EQ(GetParam() ? expect : src, str());
}

TEST_P(IR_RobustnessTest, RuntimeSizedArray_InStruct_ConstIndex) {
    auto* structure = ty.Struct(mod.symbols.Register("structure"),
                                {
                                    {mod.symbols.Register("arr"), ty.array<u32>()},
                                });

    auto* buffer = b.Var("buffer", ty.ptr(storage, structure));
    buffer->SetBindingPoint(0, 0);
    b.RootBlock()->Append(buffer);

    auto* func = b.Function("foo", ty.u32());
    b.Append(func->Block(), [&] {
        auto* access = b.Access(ty.ptr<storage, u32>(), buffer, b.Constant(0_u), b.Constant(42_u));
        auto* load = b.Load(access);
        b.Return(func, load);
    });

    auto* src = R"(
structure = struct @align(4) {
  arr:array<u32> @offset(0)
}

%b1 = block {  # root
  %buffer:ptr<storage, structure, read_write> = var @binding_point(0, 0)
}

%foo = func():u32 -> %b2 {
  %b2 = block {
    %3:ptr<storage, u32, read_write> = access %buffer, 0u, 42u
    %4:u32 = load %3
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
structure = struct @align(4) {
  arr:array<u32> @offset(0)
}

%b1 = block {  # root
  %buffer:ptr<storage, structure, read_write> = var @binding_point(0, 0)
}

%foo = func():u32 -> %b2 {
  %b2 = block {
    %3:ptr<storage, array<u32>, read_write> = access %buffer, 0u
    %4:u32 = arrayLength %3
    %5:u32 = sub %4, 1u
    %6:u32 = min 42u, %5
    %7:ptr<storage, u32, read_write> = access %buffer, 0u, %6
    %8:u32 = load %7
    ret %8
  }
}
)";

    RobustnessConfig cfg;
    cfg.clamp_storage = GetParam();
    Run(Robustness, cfg);

    EXPECT_EQ(GetParam() ? expect : src, str());
}

TEST_P(IR_RobustnessTest, RuntimeSizedArray_InStruct_DynamicIndex) {
    auto* structure = ty.Struct(mod.symbols.Register("structure"),
                                {
                                    {mod.symbols.Register("arr"), ty.array<u32>()},
                                });

    auto* buffer = b.Var("buffer", ty.ptr(storage, structure));
    buffer->SetBindingPoint(0, 0);
    b.RootBlock()->Append(buffer);

    auto* func = b.Function("foo", ty.u32());
    auto* idx = b.FunctionParam("idx", ty.u32());
    func->SetParams({idx});
    b.Append(func->Block(), [&] {
        auto* access = b.Access(ty.ptr<storage, u32>(), buffer, b.Constant(0_u), idx);
        auto* load = b.Load(access);
        b.Return(func, load);
    });

    auto* src = R"(
structure = struct @align(4) {
  arr:array<u32> @offset(0)
}

%b1 = block {  # root
  %buffer:ptr<storage, structure, read_write> = var @binding_point(0, 0)
}

%foo = func(%idx:u32):u32 -> %b2 {
  %b2 = block {
    %4:ptr<storage, u32, read_write> = access %buffer, 0u, %idx
    %5:u32 = load %4
    ret %5
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
structure = struct @align(4) {
  arr:array<u32> @offset(0)
}

%b1 = block {  # root
  %buffer:ptr<storage, structure, read_write> = var @binding_point(0, 0)
}

%foo = func(%idx:u32):u32 -> %b2 {
  %b2 = block {
    %4:ptr<storage, array<u32>, read_write> = access %buffer, 0u
    %5:u32 = arrayLength %4
    %6:u32 = sub %5, 1u
    %7:u32 = min %idx, %6
    %8:ptr<storage, u32, read_write> = access %buffer, 0u, %7
    %9:u32 = load %8
    ret %9
  }
}
)";

    RobustnessConfig cfg;
    cfg.clamp_storage = GetParam();
    Run(Robustness, cfg);

    EXPECT_EQ(GetParam() ? expect : src, str());
}

}  // namespace
}  // namespace tint::core::ir::transform
