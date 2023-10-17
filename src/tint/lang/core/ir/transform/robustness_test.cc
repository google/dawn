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

#include "src/tint/lang/core/ir/transform/robustness.h"

#include <utility>

#include "src/tint/lang/core/ir/transform/helper_test.h"
#include "src/tint/lang/core/type/array.h"
#include "src/tint/lang/core/type/depth_multisampled_texture.h"
#include "src/tint/lang/core/type/depth_texture.h"
#include "src/tint/lang/core/type/external_texture.h"
#include "src/tint/lang/core/type/matrix.h"
#include "src/tint/lang/core/type/multisampled_texture.h"
#include "src/tint/lang/core/type/pointer.h"
#include "src/tint/lang/core/type/sampled_texture.h"
#include "src/tint/lang/core/type/storage_texture.h"
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
    mod.root_block->Append(vec);

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
    mod.root_block->Append(vec);

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
    mod.root_block->Append(arr);

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
    mod.root_block->Append(vec);

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
    mod.root_block->Append(vec);

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
    mod.root_block->Append(arr);

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
    mod.root_block->Append(vec);

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
    mod.root_block->Append(vec);

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
    mod.root_block->Append(arr);

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
    mod.root_block->Append(vec);

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
    mod.root_block->Append(vec);

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
    mod.root_block->Append(arr);

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
    mod.root_block->Append(vec);

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
    mod.root_block->Append(vec);

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
    mod.root_block->Append(arr);

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
// Test clamping runtime-sized arrays.
////////////////////////////////////////////////////////////////

TEST_P(IR_RobustnessTest, RuntimeSizedArray_ConstIndex) {
    auto* arr = b.Var("arr", ty.ptr(storage, ty.array<u32>()));
    arr->SetBindingPoint(0, 0);
    mod.root_block->Append(arr);

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
    mod.root_block->Append(arr);

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
    mod.root_block->Append(buffer);

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
    mod.root_block->Append(buffer);

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

TEST_P(IR_RobustnessTest, RuntimeSizedArray_DisableClamping) {
    auto* arr = b.Var("arr", ty.ptr(storage, ty.array<u32>()));
    arr->SetBindingPoint(0, 0);
    mod.root_block->Append(arr);

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
    cfg.clamp_storage = true;
    cfg.disable_runtime_sized_array_index_clamping = !GetParam();
    Run(Robustness, cfg);

    EXPECT_EQ(GetParam() ? expect : src, str());
}

////////////////////////////////////////////////////////////////
// Test clamping texture builtin calls.
////////////////////////////////////////////////////////////////

TEST_P(IR_RobustnessTest, TextureDimensions) {
    auto* texture = b.Var(
        "texture",
        ty.ptr(handle, ty.Get<type::SampledTexture>(type::TextureDimension::k2d, ty.f32()), read));
    texture->SetBindingPoint(0, 0);
    mod.root_block->Append(texture);

    auto* func = b.Function("foo", ty.vec2<u32>());
    b.Append(func->Block(), [&] {
        auto* handle = b.Load(texture);
        auto* dims = b.Call(ty.vec2<u32>(), core::BuiltinFn::kTextureDimensions, handle);
        b.Return(func, dims);
    });

    auto* src = R"(
%b1 = block {  # root
  %texture:ptr<handle, texture_2d<f32>, read> = var @binding_point(0, 0)
}

%foo = func():vec2<u32> -> %b2 {
  %b2 = block {
    %3:texture_2d<f32> = load %texture
    %4:vec2<u32> = textureDimensions %3
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    RobustnessConfig cfg;
    cfg.clamp_texture = GetParam();
    Run(Robustness, cfg);

    EXPECT_EQ(GetParam() ? expect : src, str());
}

TEST_P(IR_RobustnessTest, TextureDimensions_WithLevel) {
    auto* texture = b.Var(
        "texture",
        ty.ptr(handle, ty.Get<type::SampledTexture>(type::TextureDimension::k2d, ty.f32()), read));
    texture->SetBindingPoint(0, 0);
    mod.root_block->Append(texture);

    auto* func = b.Function("foo", ty.vec2<u32>());
    auto* level = b.FunctionParam("level", ty.u32());
    func->SetParams({level});
    b.Append(func->Block(), [&] {
        auto* handle = b.Load(texture);
        auto* dims = b.Call(ty.vec2<u32>(), core::BuiltinFn::kTextureDimensions, handle, level);
        b.Return(func, dims);
    });

    auto* src = R"(
%b1 = block {  # root
  %texture:ptr<handle, texture_2d<f32>, read> = var @binding_point(0, 0)
}

%foo = func(%level:u32):vec2<u32> -> %b2 {
  %b2 = block {
    %4:texture_2d<f32> = load %texture
    %5:vec2<u32> = textureDimensions %4, %level
    ret %5
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %texture:ptr<handle, texture_2d<f32>, read> = var @binding_point(0, 0)
}

%foo = func(%level:u32):vec2<u32> -> %b2 {
  %b2 = block {
    %4:texture_2d<f32> = load %texture
    %5:u32 = textureNumLevels %4
    %6:u32 = sub %5, 1u
    %7:u32 = min %level, %6
    %8:vec2<u32> = textureDimensions %4, %7
    ret %8
  }
}
)";

    RobustnessConfig cfg;
    cfg.clamp_texture = GetParam();
    Run(Robustness, cfg);

    EXPECT_EQ(GetParam() ? expect : src, str());
}

TEST_P(IR_RobustnessTest, TextureLoad_Sampled1D) {
    auto* texture = b.Var(
        "texture",
        ty.ptr(handle, ty.Get<type::SampledTexture>(type::TextureDimension::k1d, ty.f32()), read));
    texture->SetBindingPoint(0, 0);
    mod.root_block->Append(texture);

    {
        auto* func = b.Function("load_signed", ty.vec4<f32>());
        auto* coords = b.FunctionParam("coords", ty.i32());
        auto* level = b.FunctionParam("level", ty.i32());
        func->SetParams({coords, level});
        b.Append(func->Block(), [&] {
            auto* handle = b.Load(texture);
            auto* texel =
                b.Call(ty.vec4<f32>(), core::BuiltinFn::kTextureLoad, handle, coords, level);
            b.Return(func, texel);
        });
    }

    {
        auto* func = b.Function("load_unsigned", ty.vec4<f32>());
        auto* coords = b.FunctionParam("coords", ty.u32());
        auto* level = b.FunctionParam("level", ty.u32());
        func->SetParams({coords, level});
        b.Append(func->Block(), [&] {
            auto* handle = b.Load(texture);
            auto* texel =
                b.Call(ty.vec4<f32>(), core::BuiltinFn::kTextureLoad, handle, coords, level);
            b.Return(func, texel);
        });
    }

    auto* src = R"(
%b1 = block {  # root
  %texture:ptr<handle, texture_1d<f32>, read> = var @binding_point(0, 0)
}

%load_signed = func(%coords:i32, %level:i32):vec4<f32> -> %b2 {
  %b2 = block {
    %5:texture_1d<f32> = load %texture
    %6:vec4<f32> = textureLoad %5, %coords, %level
    ret %6
  }
}
%load_unsigned = func(%coords_1:u32, %level_1:u32):vec4<f32> -> %b3 {  # %coords_1: 'coords', %level_1: 'level'
  %b3 = block {
    %10:texture_1d<f32> = load %texture
    %11:vec4<f32> = textureLoad %10, %coords_1, %level_1
    ret %11
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %texture:ptr<handle, texture_1d<f32>, read> = var @binding_point(0, 0)
}

%load_signed = func(%coords:i32, %level:i32):vec4<f32> -> %b2 {
  %b2 = block {
    %5:texture_1d<f32> = load %texture
    %6:u32 = textureDimensions %5
    %7:u32 = sub %6, 1u
    %8:u32 = convert %coords
    %9:u32 = min %8, %7
    %10:u32 = textureNumLevels %5
    %11:u32 = sub %10, 1u
    %12:u32 = convert %level
    %13:u32 = min %12, %11
    %14:vec4<f32> = textureLoad %5, %9, %13
    ret %14
  }
}
%load_unsigned = func(%coords_1:u32, %level_1:u32):vec4<f32> -> %b3 {  # %coords_1: 'coords', %level_1: 'level'
  %b3 = block {
    %18:texture_1d<f32> = load %texture
    %19:u32 = textureDimensions %18
    %20:u32 = sub %19, 1u
    %21:u32 = min %coords_1, %20
    %22:u32 = textureNumLevels %18
    %23:u32 = sub %22, 1u
    %24:u32 = min %level_1, %23
    %25:vec4<f32> = textureLoad %18, %21, %24
    ret %25
  }
}
)";

    RobustnessConfig cfg;
    cfg.clamp_texture = GetParam();
    Run(Robustness, cfg);

    EXPECT_EQ(GetParam() ? expect : src, str());
}

TEST_P(IR_RobustnessTest, TextureLoad_Sampled2D) {
    auto* texture = b.Var(
        "texture",
        ty.ptr(handle, ty.Get<type::SampledTexture>(type::TextureDimension::k2d, ty.f32()), read));
    texture->SetBindingPoint(0, 0);
    mod.root_block->Append(texture);

    {
        auto* func = b.Function("load_signed", ty.vec4<f32>());
        auto* coords = b.FunctionParam("coords", ty.vec2<i32>());
        auto* level = b.FunctionParam("level", ty.i32());
        func->SetParams({coords, level});
        b.Append(func->Block(), [&] {
            auto* handle = b.Load(texture);
            auto* texel =
                b.Call(ty.vec4<f32>(), core::BuiltinFn::kTextureLoad, handle, coords, level);
            b.Return(func, texel);
        });
    }

    {
        auto* func = b.Function("load_unsigned", ty.vec4<f32>());
        auto* coords = b.FunctionParam("coords", ty.vec2<u32>());
        auto* level = b.FunctionParam("level", ty.u32());
        func->SetParams({coords, level});
        b.Append(func->Block(), [&] {
            auto* handle = b.Load(texture);
            auto* texel =
                b.Call(ty.vec4<f32>(), core::BuiltinFn::kTextureLoad, handle, coords, level);
            b.Return(func, texel);
        });
    }

    auto* src = R"(
%b1 = block {  # root
  %texture:ptr<handle, texture_2d<f32>, read> = var @binding_point(0, 0)
}

%load_signed = func(%coords:vec2<i32>, %level:i32):vec4<f32> -> %b2 {
  %b2 = block {
    %5:texture_2d<f32> = load %texture
    %6:vec4<f32> = textureLoad %5, %coords, %level
    ret %6
  }
}
%load_unsigned = func(%coords_1:vec2<u32>, %level_1:u32):vec4<f32> -> %b3 {  # %coords_1: 'coords', %level_1: 'level'
  %b3 = block {
    %10:texture_2d<f32> = load %texture
    %11:vec4<f32> = textureLoad %10, %coords_1, %level_1
    ret %11
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %texture:ptr<handle, texture_2d<f32>, read> = var @binding_point(0, 0)
}

%load_signed = func(%coords:vec2<i32>, %level:i32):vec4<f32> -> %b2 {
  %b2 = block {
    %5:texture_2d<f32> = load %texture
    %6:vec2<u32> = textureDimensions %5
    %7:vec2<u32> = sub %6, vec2<u32>(1u)
    %8:vec2<u32> = convert %coords
    %9:vec2<u32> = min %8, %7
    %10:u32 = textureNumLevels %5
    %11:u32 = sub %10, 1u
    %12:u32 = convert %level
    %13:u32 = min %12, %11
    %14:vec4<f32> = textureLoad %5, %9, %13
    ret %14
  }
}
%load_unsigned = func(%coords_1:vec2<u32>, %level_1:u32):vec4<f32> -> %b3 {  # %coords_1: 'coords', %level_1: 'level'
  %b3 = block {
    %18:texture_2d<f32> = load %texture
    %19:vec2<u32> = textureDimensions %18
    %20:vec2<u32> = sub %19, vec2<u32>(1u)
    %21:vec2<u32> = min %coords_1, %20
    %22:u32 = textureNumLevels %18
    %23:u32 = sub %22, 1u
    %24:u32 = min %level_1, %23
    %25:vec4<f32> = textureLoad %18, %21, %24
    ret %25
  }
}
)";

    RobustnessConfig cfg;
    cfg.clamp_texture = GetParam();
    Run(Robustness, cfg);

    EXPECT_EQ(GetParam() ? expect : src, str());
}

TEST_P(IR_RobustnessTest, TextureLoad_Sampled2DArray) {
    auto* texture = b.Var(
        "texture",
        ty.ptr(handle, ty.Get<type::SampledTexture>(type::TextureDimension::k2dArray, ty.f32()),
               read));
    texture->SetBindingPoint(0, 0);
    mod.root_block->Append(texture);

    {
        auto* func = b.Function("load_signed", ty.vec4<f32>());
        auto* coords = b.FunctionParam("coords", ty.vec2<i32>());
        auto* layer = b.FunctionParam("layer", ty.i32());
        auto* level = b.FunctionParam("level", ty.i32());
        func->SetParams({coords, layer, level});
        b.Append(func->Block(), [&] {
            auto* handle = b.Load(texture);
            auto* texel =
                b.Call(ty.vec4<f32>(), core::BuiltinFn::kTextureLoad, handle, coords, layer, level);
            b.Return(func, texel);
        });
    }

    {
        auto* func = b.Function("load_unsigned", ty.vec4<f32>());
        auto* coords = b.FunctionParam("coords", ty.vec2<u32>());
        auto* layer = b.FunctionParam("layer", ty.u32());
        auto* level = b.FunctionParam("level", ty.u32());
        func->SetParams({coords, layer, level});
        b.Append(func->Block(), [&] {
            auto* handle = b.Load(texture);
            auto* texel =
                b.Call(ty.vec4<f32>(), core::BuiltinFn::kTextureLoad, handle, coords, layer, level);
            b.Return(func, texel);
        });
    }

    auto* src = R"(
%b1 = block {  # root
  %texture:ptr<handle, texture_2d_array<f32>, read> = var @binding_point(0, 0)
}

%load_signed = func(%coords:vec2<i32>, %layer:i32, %level:i32):vec4<f32> -> %b2 {
  %b2 = block {
    %6:texture_2d_array<f32> = load %texture
    %7:vec4<f32> = textureLoad %6, %coords, %layer, %level
    ret %7
  }
}
%load_unsigned = func(%coords_1:vec2<u32>, %layer_1:u32, %level_1:u32):vec4<f32> -> %b3 {  # %coords_1: 'coords', %layer_1: 'layer', %level_1: 'level'
  %b3 = block {
    %12:texture_2d_array<f32> = load %texture
    %13:vec4<f32> = textureLoad %12, %coords_1, %layer_1, %level_1
    ret %13
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %texture:ptr<handle, texture_2d_array<f32>, read> = var @binding_point(0, 0)
}

%load_signed = func(%coords:vec2<i32>, %layer:i32, %level:i32):vec4<f32> -> %b2 {
  %b2 = block {
    %6:texture_2d_array<f32> = load %texture
    %7:vec2<u32> = textureDimensions %6
    %8:vec2<u32> = sub %7, vec2<u32>(1u)
    %9:vec2<u32> = convert %coords
    %10:vec2<u32> = min %9, %8
    %11:u32 = textureNumLayers %6
    %12:u32 = sub %11, 1u
    %13:u32 = convert %layer
    %14:u32 = min %13, %12
    %15:u32 = textureNumLevels %6
    %16:u32 = sub %15, 1u
    %17:u32 = convert %level
    %18:u32 = min %17, %16
    %19:vec4<f32> = textureLoad %6, %10, %14, %18
    ret %19
  }
}
%load_unsigned = func(%coords_1:vec2<u32>, %layer_1:u32, %level_1:u32):vec4<f32> -> %b3 {  # %coords_1: 'coords', %layer_1: 'layer', %level_1: 'level'
  %b3 = block {
    %24:texture_2d_array<f32> = load %texture
    %25:vec2<u32> = textureDimensions %24
    %26:vec2<u32> = sub %25, vec2<u32>(1u)
    %27:vec2<u32> = min %coords_1, %26
    %28:u32 = textureNumLayers %24
    %29:u32 = sub %28, 1u
    %30:u32 = min %layer_1, %29
    %31:u32 = textureNumLevels %24
    %32:u32 = sub %31, 1u
    %33:u32 = min %level_1, %32
    %34:vec4<f32> = textureLoad %24, %27, %30, %33
    ret %34
  }
}
)";

    RobustnessConfig cfg;
    cfg.clamp_texture = GetParam();
    Run(Robustness, cfg);

    EXPECT_EQ(GetParam() ? expect : src, str());
}

TEST_P(IR_RobustnessTest, TextureLoad_Sampled3D) {
    auto* texture = b.Var(
        "texture",
        ty.ptr(handle, ty.Get<type::SampledTexture>(type::TextureDimension::k3d, ty.f32()), read));
    texture->SetBindingPoint(0, 0);
    mod.root_block->Append(texture);

    {
        auto* func = b.Function("load_signed", ty.vec4<f32>());
        auto* coords = b.FunctionParam("coords", ty.vec3<i32>());
        auto* level = b.FunctionParam("level", ty.i32());
        func->SetParams({coords, level});
        b.Append(func->Block(), [&] {
            auto* handle = b.Load(texture);
            auto* texel =
                b.Call(ty.vec4<f32>(), core::BuiltinFn::kTextureLoad, handle, coords, level);
            b.Return(func, texel);
        });
    }

    {
        auto* func = b.Function("load_unsigned", ty.vec4<f32>());
        auto* coords = b.FunctionParam("coords", ty.vec3<u32>());
        auto* level = b.FunctionParam("level", ty.u32());
        func->SetParams({coords, level});
        b.Append(func->Block(), [&] {
            auto* handle = b.Load(texture);
            auto* texel =
                b.Call(ty.vec4<f32>(), core::BuiltinFn::kTextureLoad, handle, coords, level);
            b.Return(func, texel);
        });
    }

    auto* src = R"(
%b1 = block {  # root
  %texture:ptr<handle, texture_3d<f32>, read> = var @binding_point(0, 0)
}

%load_signed = func(%coords:vec3<i32>, %level:i32):vec4<f32> -> %b2 {
  %b2 = block {
    %5:texture_3d<f32> = load %texture
    %6:vec4<f32> = textureLoad %5, %coords, %level
    ret %6
  }
}
%load_unsigned = func(%coords_1:vec3<u32>, %level_1:u32):vec4<f32> -> %b3 {  # %coords_1: 'coords', %level_1: 'level'
  %b3 = block {
    %10:texture_3d<f32> = load %texture
    %11:vec4<f32> = textureLoad %10, %coords_1, %level_1
    ret %11
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %texture:ptr<handle, texture_3d<f32>, read> = var @binding_point(0, 0)
}

%load_signed = func(%coords:vec3<i32>, %level:i32):vec4<f32> -> %b2 {
  %b2 = block {
    %5:texture_3d<f32> = load %texture
    %6:vec3<u32> = textureDimensions %5
    %7:vec3<u32> = sub %6, vec3<u32>(1u)
    %8:vec3<u32> = convert %coords
    %9:vec3<u32> = min %8, %7
    %10:u32 = textureNumLevels %5
    %11:u32 = sub %10, 1u
    %12:u32 = convert %level
    %13:u32 = min %12, %11
    %14:vec4<f32> = textureLoad %5, %9, %13
    ret %14
  }
}
%load_unsigned = func(%coords_1:vec3<u32>, %level_1:u32):vec4<f32> -> %b3 {  # %coords_1: 'coords', %level_1: 'level'
  %b3 = block {
    %18:texture_3d<f32> = load %texture
    %19:vec3<u32> = textureDimensions %18
    %20:vec3<u32> = sub %19, vec3<u32>(1u)
    %21:vec3<u32> = min %coords_1, %20
    %22:u32 = textureNumLevels %18
    %23:u32 = sub %22, 1u
    %24:u32 = min %level_1, %23
    %25:vec4<f32> = textureLoad %18, %21, %24
    ret %25
  }
}
)";

    RobustnessConfig cfg;
    cfg.clamp_texture = GetParam();
    Run(Robustness, cfg);

    EXPECT_EQ(GetParam() ? expect : src, str());
}

TEST_P(IR_RobustnessTest, TextureLoad_Multisampled2D) {
    auto* texture = b.Var(
        "texture",
        ty.ptr(handle, ty.Get<type::MultisampledTexture>(type::TextureDimension::k2d, ty.f32()),
               read));
    texture->SetBindingPoint(0, 0);
    mod.root_block->Append(texture);

    {
        auto* func = b.Function("load_signed", ty.vec4<f32>());
        auto* coords = b.FunctionParam("coords", ty.vec2<i32>());
        auto* level = b.FunctionParam("level", ty.i32());
        func->SetParams({coords, level});
        b.Append(func->Block(), [&] {
            auto* handle = b.Load(texture);
            auto* texel =
                b.Call(ty.vec4<f32>(), core::BuiltinFn::kTextureLoad, handle, coords, level);
            b.Return(func, texel);
        });
    }

    {
        auto* func = b.Function("load_unsigned", ty.vec4<f32>());
        auto* coords = b.FunctionParam("coords", ty.vec2<u32>());
        auto* level = b.FunctionParam("level", ty.u32());
        func->SetParams({coords, level});
        b.Append(func->Block(), [&] {
            auto* handle = b.Load(texture);
            auto* texel =
                b.Call(ty.vec4<f32>(), core::BuiltinFn::kTextureLoad, handle, coords, level);
            b.Return(func, texel);
        });
    }

    auto* src = R"(
%b1 = block {  # root
  %texture:ptr<handle, texture_multisampled_2d<f32>, read> = var @binding_point(0, 0)
}

%load_signed = func(%coords:vec2<i32>, %level:i32):vec4<f32> -> %b2 {
  %b2 = block {
    %5:texture_multisampled_2d<f32> = load %texture
    %6:vec4<f32> = textureLoad %5, %coords, %level
    ret %6
  }
}
%load_unsigned = func(%coords_1:vec2<u32>, %level_1:u32):vec4<f32> -> %b3 {  # %coords_1: 'coords', %level_1: 'level'
  %b3 = block {
    %10:texture_multisampled_2d<f32> = load %texture
    %11:vec4<f32> = textureLoad %10, %coords_1, %level_1
    ret %11
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %texture:ptr<handle, texture_multisampled_2d<f32>, read> = var @binding_point(0, 0)
}

%load_signed = func(%coords:vec2<i32>, %level:i32):vec4<f32> -> %b2 {
  %b2 = block {
    %5:texture_multisampled_2d<f32> = load %texture
    %6:vec2<u32> = textureDimensions %5
    %7:vec2<u32> = sub %6, vec2<u32>(1u)
    %8:vec2<u32> = convert %coords
    %9:vec2<u32> = min %8, %7
    %10:vec4<f32> = textureLoad %5, %9, %level
    ret %10
  }
}
%load_unsigned = func(%coords_1:vec2<u32>, %level_1:u32):vec4<f32> -> %b3 {  # %coords_1: 'coords', %level_1: 'level'
  %b3 = block {
    %14:texture_multisampled_2d<f32> = load %texture
    %15:vec2<u32> = textureDimensions %14
    %16:vec2<u32> = sub %15, vec2<u32>(1u)
    %17:vec2<u32> = min %coords_1, %16
    %18:vec4<f32> = textureLoad %14, %17, %level_1
    ret %18
  }
}
)";

    RobustnessConfig cfg;
    cfg.clamp_texture = GetParam();
    Run(Robustness, cfg);

    EXPECT_EQ(GetParam() ? expect : src, str());
}

TEST_P(IR_RobustnessTest, TextureLoad_Depth2D) {
    auto* texture = b.Var(
        "texture", ty.ptr(handle, ty.Get<type::DepthTexture>(type::TextureDimension::k2d), read));
    texture->SetBindingPoint(0, 0);
    mod.root_block->Append(texture);

    {
        auto* func = b.Function("load_signed", ty.f32());
        auto* coords = b.FunctionParam("coords", ty.vec2<i32>());
        auto* level = b.FunctionParam("level", ty.i32());
        func->SetParams({coords, level});
        b.Append(func->Block(), [&] {
            auto* handle = b.Load(texture);
            auto* texel = b.Call(ty.f32(), core::BuiltinFn::kTextureLoad, handle, coords, level);
            b.Return(func, texel);
        });
    }

    {
        auto* func = b.Function("load_unsigned", ty.f32());
        auto* coords = b.FunctionParam("coords", ty.vec2<u32>());
        auto* level = b.FunctionParam("level", ty.u32());
        func->SetParams({coords, level});
        b.Append(func->Block(), [&] {
            auto* handle = b.Load(texture);
            auto* texel = b.Call(ty.f32(), core::BuiltinFn::kTextureLoad, handle, coords, level);
            b.Return(func, texel);
        });
    }

    auto* src = R"(
%b1 = block {  # root
  %texture:ptr<handle, texture_depth_2d, read> = var @binding_point(0, 0)
}

%load_signed = func(%coords:vec2<i32>, %level:i32):f32 -> %b2 {
  %b2 = block {
    %5:texture_depth_2d = load %texture
    %6:f32 = textureLoad %5, %coords, %level
    ret %6
  }
}
%load_unsigned = func(%coords_1:vec2<u32>, %level_1:u32):f32 -> %b3 {  # %coords_1: 'coords', %level_1: 'level'
  %b3 = block {
    %10:texture_depth_2d = load %texture
    %11:f32 = textureLoad %10, %coords_1, %level_1
    ret %11
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %texture:ptr<handle, texture_depth_2d, read> = var @binding_point(0, 0)
}

%load_signed = func(%coords:vec2<i32>, %level:i32):f32 -> %b2 {
  %b2 = block {
    %5:texture_depth_2d = load %texture
    %6:vec2<u32> = textureDimensions %5
    %7:vec2<u32> = sub %6, vec2<u32>(1u)
    %8:vec2<u32> = convert %coords
    %9:vec2<u32> = min %8, %7
    %10:u32 = textureNumLevels %5
    %11:u32 = sub %10, 1u
    %12:u32 = convert %level
    %13:u32 = min %12, %11
    %14:f32 = textureLoad %5, %9, %13
    ret %14
  }
}
%load_unsigned = func(%coords_1:vec2<u32>, %level_1:u32):f32 -> %b3 {  # %coords_1: 'coords', %level_1: 'level'
  %b3 = block {
    %18:texture_depth_2d = load %texture
    %19:vec2<u32> = textureDimensions %18
    %20:vec2<u32> = sub %19, vec2<u32>(1u)
    %21:vec2<u32> = min %coords_1, %20
    %22:u32 = textureNumLevels %18
    %23:u32 = sub %22, 1u
    %24:u32 = min %level_1, %23
    %25:f32 = textureLoad %18, %21, %24
    ret %25
  }
}
)";

    RobustnessConfig cfg;
    cfg.clamp_texture = GetParam();
    Run(Robustness, cfg);

    EXPECT_EQ(GetParam() ? expect : src, str());
}

TEST_P(IR_RobustnessTest, TextureLoad_Depth2DArray) {
    auto* texture =
        b.Var("texture",
              ty.ptr(handle, ty.Get<type::DepthTexture>(type::TextureDimension::k2dArray), read));
    texture->SetBindingPoint(0, 0);
    mod.root_block->Append(texture);

    {
        auto* func = b.Function("load_signed", ty.f32());
        auto* coords = b.FunctionParam("coords", ty.vec2<i32>());
        auto* layer = b.FunctionParam("layer", ty.i32());
        auto* level = b.FunctionParam("level", ty.i32());
        func->SetParams({coords, layer, level});
        b.Append(func->Block(), [&] {
            auto* handle = b.Load(texture);
            auto* texel =
                b.Call(ty.f32(), core::BuiltinFn::kTextureLoad, handle, coords, layer, level);
            b.Return(func, texel);
        });
    }

    {
        auto* func = b.Function("load_unsigned", ty.f32());
        auto* coords = b.FunctionParam("coords", ty.vec2<u32>());
        auto* layer = b.FunctionParam("layer", ty.u32());
        auto* level = b.FunctionParam("level", ty.u32());
        func->SetParams({coords, layer, level});
        b.Append(func->Block(), [&] {
            auto* handle = b.Load(texture);
            auto* texel =
                b.Call(ty.f32(), core::BuiltinFn::kTextureLoad, handle, coords, layer, level);
            b.Return(func, texel);
        });
    }

    auto* src = R"(
%b1 = block {  # root
  %texture:ptr<handle, texture_depth_2d_array, read> = var @binding_point(0, 0)
}

%load_signed = func(%coords:vec2<i32>, %layer:i32, %level:i32):f32 -> %b2 {
  %b2 = block {
    %6:texture_depth_2d_array = load %texture
    %7:f32 = textureLoad %6, %coords, %layer, %level
    ret %7
  }
}
%load_unsigned = func(%coords_1:vec2<u32>, %layer_1:u32, %level_1:u32):f32 -> %b3 {  # %coords_1: 'coords', %layer_1: 'layer', %level_1: 'level'
  %b3 = block {
    %12:texture_depth_2d_array = load %texture
    %13:f32 = textureLoad %12, %coords_1, %layer_1, %level_1
    ret %13
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %texture:ptr<handle, texture_depth_2d_array, read> = var @binding_point(0, 0)
}

%load_signed = func(%coords:vec2<i32>, %layer:i32, %level:i32):f32 -> %b2 {
  %b2 = block {
    %6:texture_depth_2d_array = load %texture
    %7:vec2<u32> = textureDimensions %6
    %8:vec2<u32> = sub %7, vec2<u32>(1u)
    %9:vec2<u32> = convert %coords
    %10:vec2<u32> = min %9, %8
    %11:u32 = textureNumLayers %6
    %12:u32 = sub %11, 1u
    %13:u32 = convert %layer
    %14:u32 = min %13, %12
    %15:u32 = textureNumLevels %6
    %16:u32 = sub %15, 1u
    %17:u32 = convert %level
    %18:u32 = min %17, %16
    %19:f32 = textureLoad %6, %10, %14, %18
    ret %19
  }
}
%load_unsigned = func(%coords_1:vec2<u32>, %layer_1:u32, %level_1:u32):f32 -> %b3 {  # %coords_1: 'coords', %layer_1: 'layer', %level_1: 'level'
  %b3 = block {
    %24:texture_depth_2d_array = load %texture
    %25:vec2<u32> = textureDimensions %24
    %26:vec2<u32> = sub %25, vec2<u32>(1u)
    %27:vec2<u32> = min %coords_1, %26
    %28:u32 = textureNumLayers %24
    %29:u32 = sub %28, 1u
    %30:u32 = min %layer_1, %29
    %31:u32 = textureNumLevels %24
    %32:u32 = sub %31, 1u
    %33:u32 = min %level_1, %32
    %34:f32 = textureLoad %24, %27, %30, %33
    ret %34
  }
}
)";

    RobustnessConfig cfg;
    cfg.clamp_texture = GetParam();
    Run(Robustness, cfg);

    EXPECT_EQ(GetParam() ? expect : src, str());
}

TEST_P(IR_RobustnessTest, TextureLoad_DepthMultisampled2D) {
    auto* texture = b.Var(
        "texture",
        ty.ptr(handle, ty.Get<type::DepthMultisampledTexture>(type::TextureDimension::k2d), read));
    texture->SetBindingPoint(0, 0);
    mod.root_block->Append(texture);

    {
        auto* func = b.Function("load_signed", ty.f32());
        auto* coords = b.FunctionParam("coords", ty.vec2<i32>());
        auto* index = b.FunctionParam("index", ty.i32());
        func->SetParams({coords, index});
        b.Append(func->Block(), [&] {
            auto* handle = b.Load(texture);
            auto* texel = b.Call(ty.f32(), core::BuiltinFn::kTextureLoad, handle, coords, index);
            b.Return(func, texel);
        });
    }

    {
        auto* func = b.Function("load_unsigned", ty.f32());
        auto* coords = b.FunctionParam("coords", ty.vec2<u32>());
        auto* index = b.FunctionParam("index", ty.u32());
        func->SetParams({coords, index});
        b.Append(func->Block(), [&] {
            auto* handle = b.Load(texture);
            auto* texel = b.Call(ty.f32(), core::BuiltinFn::kTextureLoad, handle, coords, index);
            b.Return(func, texel);
        });
    }

    auto* src = R"(
%b1 = block {  # root
  %texture:ptr<handle, texture_depth_multisampled_2d, read> = var @binding_point(0, 0)
}

%load_signed = func(%coords:vec2<i32>, %index:i32):f32 -> %b2 {
  %b2 = block {
    %5:texture_depth_multisampled_2d = load %texture
    %6:f32 = textureLoad %5, %coords, %index
    ret %6
  }
}
%load_unsigned = func(%coords_1:vec2<u32>, %index_1:u32):f32 -> %b3 {  # %coords_1: 'coords', %index_1: 'index'
  %b3 = block {
    %10:texture_depth_multisampled_2d = load %texture
    %11:f32 = textureLoad %10, %coords_1, %index_1
    ret %11
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %texture:ptr<handle, texture_depth_multisampled_2d, read> = var @binding_point(0, 0)
}

%load_signed = func(%coords:vec2<i32>, %index:i32):f32 -> %b2 {
  %b2 = block {
    %5:texture_depth_multisampled_2d = load %texture
    %6:vec2<u32> = textureDimensions %5
    %7:vec2<u32> = sub %6, vec2<u32>(1u)
    %8:vec2<u32> = convert %coords
    %9:vec2<u32> = min %8, %7
    %10:f32 = textureLoad %5, %9, %index
    ret %10
  }
}
%load_unsigned = func(%coords_1:vec2<u32>, %index_1:u32):f32 -> %b3 {  # %coords_1: 'coords', %index_1: 'index'
  %b3 = block {
    %14:texture_depth_multisampled_2d = load %texture
    %15:vec2<u32> = textureDimensions %14
    %16:vec2<u32> = sub %15, vec2<u32>(1u)
    %17:vec2<u32> = min %coords_1, %16
    %18:f32 = textureLoad %14, %17, %index_1
    ret %18
  }
}
)";

    RobustnessConfig cfg;
    cfg.clamp_texture = GetParam();
    Run(Robustness, cfg);

    EXPECT_EQ(GetParam() ? expect : src, str());
}

TEST_P(IR_RobustnessTest, TextureLoad_External) {
    auto* texture = b.Var("texture", ty.ptr(handle, ty.Get<type::ExternalTexture>(), read));
    texture->SetBindingPoint(0, 0);
    mod.root_block->Append(texture);

    {
        auto* func = b.Function("load_signed", ty.vec4<f32>());
        auto* coords = b.FunctionParam("coords", ty.vec2<i32>());
        func->SetParams({coords});
        b.Append(func->Block(), [&] {
            auto* handle = b.Load(texture);
            auto* texel = b.Call(ty.vec4<f32>(), core::BuiltinFn::kTextureLoad, handle, coords);
            b.Return(func, texel);
        });
    }

    {
        auto* func = b.Function("load_unsigned", ty.vec4<f32>());
        auto* coords = b.FunctionParam("coords", ty.vec2<u32>());
        func->SetParams({coords});
        b.Append(func->Block(), [&] {
            auto* handle = b.Load(texture);
            auto* texel = b.Call(ty.vec4<f32>(), core::BuiltinFn::kTextureLoad, handle, coords);
            b.Return(func, texel);
        });
    }

    auto* src = R"(
%b1 = block {  # root
  %texture:ptr<handle, texture_external, read> = var @binding_point(0, 0)
}

%load_signed = func(%coords:vec2<i32>):vec4<f32> -> %b2 {
  %b2 = block {
    %4:texture_external = load %texture
    %5:vec4<f32> = textureLoad %4, %coords
    ret %5
  }
}
%load_unsigned = func(%coords_1:vec2<u32>):vec4<f32> -> %b3 {  # %coords_1: 'coords'
  %b3 = block {
    %8:texture_external = load %texture
    %9:vec4<f32> = textureLoad %8, %coords_1
    ret %9
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %texture:ptr<handle, texture_external, read> = var @binding_point(0, 0)
}

%load_signed = func(%coords:vec2<i32>):vec4<f32> -> %b2 {
  %b2 = block {
    %4:texture_external = load %texture
    %5:vec2<u32> = textureDimensions %4
    %6:vec2<u32> = sub %5, vec2<u32>(1u)
    %7:vec2<u32> = convert %coords
    %8:vec2<u32> = min %7, %6
    %9:vec4<f32> = textureLoad %4, %8
    ret %9
  }
}
%load_unsigned = func(%coords_1:vec2<u32>):vec4<f32> -> %b3 {  # %coords_1: 'coords'
  %b3 = block {
    %12:texture_external = load %texture
    %13:vec2<u32> = textureDimensions %12
    %14:vec2<u32> = sub %13, vec2<u32>(1u)
    %15:vec2<u32> = min %coords_1, %14
    %16:vec4<f32> = textureLoad %12, %15
    ret %16
  }
}
)";

    RobustnessConfig cfg;
    cfg.clamp_texture = GetParam();
    Run(Robustness, cfg);

    EXPECT_EQ(GetParam() ? expect : src, str());
}

TEST_P(IR_RobustnessTest, TextureLoad_Storage1D) {
    auto format = core::TexelFormat::kRgba8Unorm;
    auto* texture =
        b.Var("texture",
              ty.ptr(handle,
                     ty.Get<type::StorageTexture>(type::TextureDimension::k1d, format, read_write,
                                                  type::StorageTexture::SubtypeFor(format, ty)),
                     read));
    texture->SetBindingPoint(0, 0);
    mod.root_block->Append(texture);

    {
        auto* func = b.Function("load_signed", ty.vec4<f32>());
        auto* coords = b.FunctionParam("coords", ty.i32());
        func->SetParams({coords});
        b.Append(func->Block(), [&] {
            auto* handle = b.Load(texture);
            auto* texel = b.Call(ty.vec4<f32>(), core::BuiltinFn::kTextureLoad, handle, coords);
            b.Return(func, texel);
        });
    }

    {
        auto* func = b.Function("load_unsigned", ty.vec4<f32>());
        auto* coords = b.FunctionParam("coords", ty.u32());
        func->SetParams({coords});
        b.Append(func->Block(), [&] {
            auto* handle = b.Load(texture);
            auto* texel = b.Call(ty.vec4<f32>(), core::BuiltinFn::kTextureLoad, handle, coords);
            b.Return(func, texel);
        });
    }

    auto* src = R"(
%b1 = block {  # root
  %texture:ptr<handle, texture_storage_1d<rgba8unorm, read_write>, read> = var @binding_point(0, 0)
}

%load_signed = func(%coords:i32):vec4<f32> -> %b2 {
  %b2 = block {
    %4:texture_storage_1d<rgba8unorm, read_write> = load %texture
    %5:vec4<f32> = textureLoad %4, %coords
    ret %5
  }
}
%load_unsigned = func(%coords_1:u32):vec4<f32> -> %b3 {  # %coords_1: 'coords'
  %b3 = block {
    %8:texture_storage_1d<rgba8unorm, read_write> = load %texture
    %9:vec4<f32> = textureLoad %8, %coords_1
    ret %9
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %texture:ptr<handle, texture_storage_1d<rgba8unorm, read_write>, read> = var @binding_point(0, 0)
}

%load_signed = func(%coords:i32):vec4<f32> -> %b2 {
  %b2 = block {
    %4:texture_storage_1d<rgba8unorm, read_write> = load %texture
    %5:u32 = textureDimensions %4
    %6:u32 = sub %5, 1u
    %7:u32 = convert %coords
    %8:u32 = min %7, %6
    %9:vec4<f32> = textureLoad %4, %8
    ret %9
  }
}
%load_unsigned = func(%coords_1:u32):vec4<f32> -> %b3 {  # %coords_1: 'coords'
  %b3 = block {
    %12:texture_storage_1d<rgba8unorm, read_write> = load %texture
    %13:u32 = textureDimensions %12
    %14:u32 = sub %13, 1u
    %15:u32 = min %coords_1, %14
    %16:vec4<f32> = textureLoad %12, %15
    ret %16
  }
}
)";

    RobustnessConfig cfg;
    cfg.clamp_texture = GetParam();
    Run(Robustness, cfg);

    EXPECT_EQ(GetParam() ? expect : src, str());
}

TEST_P(IR_RobustnessTest, TextureLoad_Storage2D) {
    auto format = core::TexelFormat::kRgba8Unorm;
    auto* texture =
        b.Var("texture",
              ty.ptr(handle,
                     ty.Get<type::StorageTexture>(type::TextureDimension::k2d, format, read_write,
                                                  type::StorageTexture::SubtypeFor(format, ty)),
                     read));
    texture->SetBindingPoint(0, 0);
    mod.root_block->Append(texture);

    {
        auto* func = b.Function("load_signed", ty.vec4<f32>());
        auto* coords = b.FunctionParam("coords", ty.vec2<i32>());
        func->SetParams({coords});
        b.Append(func->Block(), [&] {
            auto* handle = b.Load(texture);
            auto* texel = b.Call(ty.vec4<f32>(), core::BuiltinFn::kTextureLoad, handle, coords);
            b.Return(func, texel);
        });
    }

    {
        auto* func = b.Function("load_unsigned", ty.vec4<f32>());
        auto* coords = b.FunctionParam("coords", ty.vec2<u32>());
        func->SetParams({coords});
        b.Append(func->Block(), [&] {
            auto* handle = b.Load(texture);
            auto* texel = b.Call(ty.vec4<f32>(), core::BuiltinFn::kTextureLoad, handle, coords);
            b.Return(func, texel);
        });
    }

    auto* src = R"(
%b1 = block {  # root
  %texture:ptr<handle, texture_storage_2d<rgba8unorm, read_write>, read> = var @binding_point(0, 0)
}

%load_signed = func(%coords:vec2<i32>):vec4<f32> -> %b2 {
  %b2 = block {
    %4:texture_storage_2d<rgba8unorm, read_write> = load %texture
    %5:vec4<f32> = textureLoad %4, %coords
    ret %5
  }
}
%load_unsigned = func(%coords_1:vec2<u32>):vec4<f32> -> %b3 {  # %coords_1: 'coords'
  %b3 = block {
    %8:texture_storage_2d<rgba8unorm, read_write> = load %texture
    %9:vec4<f32> = textureLoad %8, %coords_1
    ret %9
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %texture:ptr<handle, texture_storage_2d<rgba8unorm, read_write>, read> = var @binding_point(0, 0)
}

%load_signed = func(%coords:vec2<i32>):vec4<f32> -> %b2 {
  %b2 = block {
    %4:texture_storage_2d<rgba8unorm, read_write> = load %texture
    %5:vec2<u32> = textureDimensions %4
    %6:vec2<u32> = sub %5, vec2<u32>(1u)
    %7:vec2<u32> = convert %coords
    %8:vec2<u32> = min %7, %6
    %9:vec4<f32> = textureLoad %4, %8
    ret %9
  }
}
%load_unsigned = func(%coords_1:vec2<u32>):vec4<f32> -> %b3 {  # %coords_1: 'coords'
  %b3 = block {
    %12:texture_storage_2d<rgba8unorm, read_write> = load %texture
    %13:vec2<u32> = textureDimensions %12
    %14:vec2<u32> = sub %13, vec2<u32>(1u)
    %15:vec2<u32> = min %coords_1, %14
    %16:vec4<f32> = textureLoad %12, %15
    ret %16
  }
}
)";

    RobustnessConfig cfg;
    cfg.clamp_texture = GetParam();
    Run(Robustness, cfg);

    EXPECT_EQ(GetParam() ? expect : src, str());
}

TEST_P(IR_RobustnessTest, TextureLoad_Storage2DArray) {
    auto format = core::TexelFormat::kRgba8Unorm;
    auto* texture = b.Var(
        "texture",
        ty.ptr(handle,
               ty.Get<type::StorageTexture>(type::TextureDimension::k2dArray, format, read_write,
                                            type::StorageTexture::SubtypeFor(format, ty)),
               read));
    texture->SetBindingPoint(0, 0);
    mod.root_block->Append(texture);

    {
        auto* func = b.Function("load_signed", ty.vec4<f32>());
        auto* coords = b.FunctionParam("coords", ty.vec2<i32>());
        auto* layer = b.FunctionParam("layer", ty.i32());
        func->SetParams({coords, layer});
        b.Append(func->Block(), [&] {
            auto* handle = b.Load(texture);
            auto* texel =
                b.Call(ty.vec4<f32>(), core::BuiltinFn::kTextureLoad, handle, coords, layer);
            b.Return(func, texel);
        });
    }

    {
        auto* func = b.Function("load_unsigned", ty.vec4<f32>());
        auto* coords = b.FunctionParam("coords", ty.vec2<u32>());
        auto* layer = b.FunctionParam("layer", ty.u32());
        func->SetParams({coords, layer});
        b.Append(func->Block(), [&] {
            auto* handle = b.Load(texture);
            auto* texel =
                b.Call(ty.vec4<f32>(), core::BuiltinFn::kTextureLoad, handle, coords, layer);
            b.Return(func, texel);
        });
    }

    auto* src = R"(
%b1 = block {  # root
  %texture:ptr<handle, texture_storage_2d_array<rgba8unorm, read_write>, read> = var @binding_point(0, 0)
}

%load_signed = func(%coords:vec2<i32>, %layer:i32):vec4<f32> -> %b2 {
  %b2 = block {
    %5:texture_storage_2d_array<rgba8unorm, read_write> = load %texture
    %6:vec4<f32> = textureLoad %5, %coords, %layer
    ret %6
  }
}
%load_unsigned = func(%coords_1:vec2<u32>, %layer_1:u32):vec4<f32> -> %b3 {  # %coords_1: 'coords', %layer_1: 'layer'
  %b3 = block {
    %10:texture_storage_2d_array<rgba8unorm, read_write> = load %texture
    %11:vec4<f32> = textureLoad %10, %coords_1, %layer_1
    ret %11
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %texture:ptr<handle, texture_storage_2d_array<rgba8unorm, read_write>, read> = var @binding_point(0, 0)
}

%load_signed = func(%coords:vec2<i32>, %layer:i32):vec4<f32> -> %b2 {
  %b2 = block {
    %5:texture_storage_2d_array<rgba8unorm, read_write> = load %texture
    %6:vec2<u32> = textureDimensions %5
    %7:vec2<u32> = sub %6, vec2<u32>(1u)
    %8:vec2<u32> = convert %coords
    %9:vec2<u32> = min %8, %7
    %10:u32 = textureNumLayers %5
    %11:u32 = sub %10, 1u
    %12:u32 = convert %layer
    %13:u32 = min %12, %11
    %14:vec4<f32> = textureLoad %5, %9, %13
    ret %14
  }
}
%load_unsigned = func(%coords_1:vec2<u32>, %layer_1:u32):vec4<f32> -> %b3 {  # %coords_1: 'coords', %layer_1: 'layer'
  %b3 = block {
    %18:texture_storage_2d_array<rgba8unorm, read_write> = load %texture
    %19:vec2<u32> = textureDimensions %18
    %20:vec2<u32> = sub %19, vec2<u32>(1u)
    %21:vec2<u32> = min %coords_1, %20
    %22:u32 = textureNumLayers %18
    %23:u32 = sub %22, 1u
    %24:u32 = min %layer_1, %23
    %25:vec4<f32> = textureLoad %18, %21, %24
    ret %25
  }
}
)";

    RobustnessConfig cfg;
    cfg.clamp_texture = GetParam();
    Run(Robustness, cfg);

    EXPECT_EQ(GetParam() ? expect : src, str());
}

TEST_P(IR_RobustnessTest, TextureLoad_Storage3D) {
    auto format = core::TexelFormat::kRgba8Unorm;
    auto* texture =
        b.Var("texture",
              ty.ptr(handle,
                     ty.Get<type::StorageTexture>(type::TextureDimension::k3d, format, read_write,
                                                  type::StorageTexture::SubtypeFor(format, ty)),
                     read));
    texture->SetBindingPoint(0, 0);
    mod.root_block->Append(texture);

    {
        auto* func = b.Function("load_signed", ty.vec4<f32>());
        auto* coords = b.FunctionParam("coords", ty.vec3<i32>());
        func->SetParams({coords});
        b.Append(func->Block(), [&] {
            auto* handle = b.Load(texture);
            auto* texel = b.Call(ty.vec4<f32>(), core::BuiltinFn::kTextureLoad, handle, coords);
            b.Return(func, texel);
        });
    }

    {
        auto* func = b.Function("load_unsigned", ty.vec4<f32>());
        auto* coords = b.FunctionParam("coords", ty.vec3<u32>());
        func->SetParams({coords});
        b.Append(func->Block(), [&] {
            auto* handle = b.Load(texture);
            auto* texel = b.Call(ty.vec4<f32>(), core::BuiltinFn::kTextureLoad, handle, coords);
            b.Return(func, texel);
        });
    }

    auto* src = R"(
%b1 = block {  # root
  %texture:ptr<handle, texture_storage_3d<rgba8unorm, read_write>, read> = var @binding_point(0, 0)
}

%load_signed = func(%coords:vec3<i32>):vec4<f32> -> %b2 {
  %b2 = block {
    %4:texture_storage_3d<rgba8unorm, read_write> = load %texture
    %5:vec4<f32> = textureLoad %4, %coords
    ret %5
  }
}
%load_unsigned = func(%coords_1:vec3<u32>):vec4<f32> -> %b3 {  # %coords_1: 'coords'
  %b3 = block {
    %8:texture_storage_3d<rgba8unorm, read_write> = load %texture
    %9:vec4<f32> = textureLoad %8, %coords_1
    ret %9
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %texture:ptr<handle, texture_storage_3d<rgba8unorm, read_write>, read> = var @binding_point(0, 0)
}

%load_signed = func(%coords:vec3<i32>):vec4<f32> -> %b2 {
  %b2 = block {
    %4:texture_storage_3d<rgba8unorm, read_write> = load %texture
    %5:vec3<u32> = textureDimensions %4
    %6:vec3<u32> = sub %5, vec3<u32>(1u)
    %7:vec3<u32> = convert %coords
    %8:vec3<u32> = min %7, %6
    %9:vec4<f32> = textureLoad %4, %8
    ret %9
  }
}
%load_unsigned = func(%coords_1:vec3<u32>):vec4<f32> -> %b3 {  # %coords_1: 'coords'
  %b3 = block {
    %12:texture_storage_3d<rgba8unorm, read_write> = load %texture
    %13:vec3<u32> = textureDimensions %12
    %14:vec3<u32> = sub %13, vec3<u32>(1u)
    %15:vec3<u32> = min %coords_1, %14
    %16:vec4<f32> = textureLoad %12, %15
    ret %16
  }
}
)";

    RobustnessConfig cfg;
    cfg.clamp_texture = GetParam();
    Run(Robustness, cfg);

    EXPECT_EQ(GetParam() ? expect : src, str());
}

TEST_P(IR_RobustnessTest, TextureStore_Storage1D) {
    auto format = core::TexelFormat::kRgba8Unorm;
    auto* texture =
        b.Var("texture",
              ty.ptr(handle,
                     ty.Get<type::StorageTexture>(type::TextureDimension::k1d, format, write,
                                                  type::StorageTexture::SubtypeFor(format, ty)),
                     read));
    texture->SetBindingPoint(0, 0);
    mod.root_block->Append(texture);

    {
        auto* func = b.Function("load_signed", ty.void_());
        auto* coords = b.FunctionParam("coords", ty.i32());
        auto* value = b.FunctionParam("value", ty.vec4<f32>());
        func->SetParams({coords, value});
        b.Append(func->Block(), [&] {
            auto* handle = b.Load(texture);
            b.Call(ty.void_(), core::BuiltinFn::kTextureStore, handle, coords, value);
            b.Return(func);
        });
    }

    {
        auto* func = b.Function("load_unsigned", ty.void_());
        auto* coords = b.FunctionParam("coords", ty.u32());
        auto* value = b.FunctionParam("value", ty.vec4<f32>());
        func->SetParams({coords, value});
        b.Append(func->Block(), [&] {
            auto* handle = b.Load(texture);
            b.Call(ty.void_(), core::BuiltinFn::kTextureStore, handle, coords, value);
            b.Return(func);
        });
    }

    auto* src = R"(
%b1 = block {  # root
  %texture:ptr<handle, texture_storage_1d<rgba8unorm, write>, read> = var @binding_point(0, 0)
}

%load_signed = func(%coords:i32, %value:vec4<f32>):void -> %b2 {
  %b2 = block {
    %5:texture_storage_1d<rgba8unorm, write> = load %texture
    %6:void = textureStore %5, %coords, %value
    ret
  }
}
%load_unsigned = func(%coords_1:u32, %value_1:vec4<f32>):void -> %b3 {  # %coords_1: 'coords', %value_1: 'value'
  %b3 = block {
    %10:texture_storage_1d<rgba8unorm, write> = load %texture
    %11:void = textureStore %10, %coords_1, %value_1
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %texture:ptr<handle, texture_storage_1d<rgba8unorm, write>, read> = var @binding_point(0, 0)
}

%load_signed = func(%coords:i32, %value:vec4<f32>):void -> %b2 {
  %b2 = block {
    %5:texture_storage_1d<rgba8unorm, write> = load %texture
    %6:u32 = textureDimensions %5
    %7:u32 = sub %6, 1u
    %8:u32 = convert %coords
    %9:u32 = min %8, %7
    %10:void = textureStore %5, %9, %value
    ret
  }
}
%load_unsigned = func(%coords_1:u32, %value_1:vec4<f32>):void -> %b3 {  # %coords_1: 'coords', %value_1: 'value'
  %b3 = block {
    %14:texture_storage_1d<rgba8unorm, write> = load %texture
    %15:u32 = textureDimensions %14
    %16:u32 = sub %15, 1u
    %17:u32 = min %coords_1, %16
    %18:void = textureStore %14, %17, %value_1
    ret
  }
}
)";

    RobustnessConfig cfg;
    cfg.clamp_texture = GetParam();
    Run(Robustness, cfg);

    EXPECT_EQ(GetParam() ? expect : src, str());
}

TEST_P(IR_RobustnessTest, TextureStore_Storage2D) {
    auto format = core::TexelFormat::kRgba8Unorm;
    auto* texture =
        b.Var("texture",
              ty.ptr(handle,
                     ty.Get<type::StorageTexture>(type::TextureDimension::k2d, format, write,
                                                  type::StorageTexture::SubtypeFor(format, ty)),
                     read));
    texture->SetBindingPoint(0, 0);
    mod.root_block->Append(texture);

    {
        auto* func = b.Function("load_signed", ty.void_());
        auto* coords = b.FunctionParam("coords", ty.vec2<i32>());
        auto* value = b.FunctionParam("value", ty.vec4<f32>());
        func->SetParams({coords, value});
        b.Append(func->Block(), [&] {
            auto* handle = b.Load(texture);
            b.Call(ty.void_(), core::BuiltinFn::kTextureStore, handle, coords, value);
            b.Return(func);
        });
    }

    {
        auto* func = b.Function("load_unsigned", ty.void_());
        auto* coords = b.FunctionParam("coords", ty.vec2<u32>());
        auto* value = b.FunctionParam("value", ty.vec4<f32>());
        func->SetParams({coords, value});
        b.Append(func->Block(), [&] {
            auto* handle = b.Load(texture);
            b.Call(ty.void_(), core::BuiltinFn::kTextureStore, handle, coords, value);
            b.Return(func);
        });
    }

    auto* src = R"(
%b1 = block {  # root
  %texture:ptr<handle, texture_storage_2d<rgba8unorm, write>, read> = var @binding_point(0, 0)
}

%load_signed = func(%coords:vec2<i32>, %value:vec4<f32>):void -> %b2 {
  %b2 = block {
    %5:texture_storage_2d<rgba8unorm, write> = load %texture
    %6:void = textureStore %5, %coords, %value
    ret
  }
}
%load_unsigned = func(%coords_1:vec2<u32>, %value_1:vec4<f32>):void -> %b3 {  # %coords_1: 'coords', %value_1: 'value'
  %b3 = block {
    %10:texture_storage_2d<rgba8unorm, write> = load %texture
    %11:void = textureStore %10, %coords_1, %value_1
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %texture:ptr<handle, texture_storage_2d<rgba8unorm, write>, read> = var @binding_point(0, 0)
}

%load_signed = func(%coords:vec2<i32>, %value:vec4<f32>):void -> %b2 {
  %b2 = block {
    %5:texture_storage_2d<rgba8unorm, write> = load %texture
    %6:vec2<u32> = textureDimensions %5
    %7:vec2<u32> = sub %6, vec2<u32>(1u)
    %8:vec2<u32> = convert %coords
    %9:vec2<u32> = min %8, %7
    %10:void = textureStore %5, %9, %value
    ret
  }
}
%load_unsigned = func(%coords_1:vec2<u32>, %value_1:vec4<f32>):void -> %b3 {  # %coords_1: 'coords', %value_1: 'value'
  %b3 = block {
    %14:texture_storage_2d<rgba8unorm, write> = load %texture
    %15:vec2<u32> = textureDimensions %14
    %16:vec2<u32> = sub %15, vec2<u32>(1u)
    %17:vec2<u32> = min %coords_1, %16
    %18:void = textureStore %14, %17, %value_1
    ret
  }
}
)";

    RobustnessConfig cfg;
    cfg.clamp_texture = GetParam();
    Run(Robustness, cfg);

    EXPECT_EQ(GetParam() ? expect : src, str());
}

TEST_P(IR_RobustnessTest, TextureStore_Storage2DArray) {
    auto format = core::TexelFormat::kRgba8Unorm;
    auto* texture =
        b.Var("texture",
              ty.ptr(handle,
                     ty.Get<type::StorageTexture>(type::TextureDimension::k2dArray, format, write,
                                                  type::StorageTexture::SubtypeFor(format, ty)),
                     read));
    texture->SetBindingPoint(0, 0);
    mod.root_block->Append(texture);

    {
        auto* func = b.Function("load_signed", ty.void_());
        auto* coords = b.FunctionParam("coords", ty.vec2<i32>());
        auto* layer = b.FunctionParam("layer", ty.i32());
        auto* value = b.FunctionParam("value", ty.vec4<f32>());
        func->SetParams({coords, layer, value});
        b.Append(func->Block(), [&] {
            auto* handle = b.Load(texture);
            b.Call(ty.void_(), core::BuiltinFn::kTextureStore, handle, coords, layer, value);
            b.Return(func);
        });
    }

    {
        auto* func = b.Function("load_unsigned", ty.void_());
        auto* coords = b.FunctionParam("coords", ty.vec2<u32>());
        auto* layer = b.FunctionParam("layer", ty.u32());
        auto* value = b.FunctionParam("value", ty.vec4<f32>());
        func->SetParams({coords, layer, value});
        b.Append(func->Block(), [&] {
            auto* handle = b.Load(texture);
            b.Call(ty.void_(), core::BuiltinFn::kTextureStore, handle, coords, layer, value);
            b.Return(func);
        });
    }

    auto* src = R"(
%b1 = block {  # root
  %texture:ptr<handle, texture_storage_2d_array<rgba8unorm, write>, read> = var @binding_point(0, 0)
}

%load_signed = func(%coords:vec2<i32>, %layer:i32, %value:vec4<f32>):void -> %b2 {
  %b2 = block {
    %6:texture_storage_2d_array<rgba8unorm, write> = load %texture
    %7:void = textureStore %6, %coords, %layer, %value
    ret
  }
}
%load_unsigned = func(%coords_1:vec2<u32>, %layer_1:u32, %value_1:vec4<f32>):void -> %b3 {  # %coords_1: 'coords', %layer_1: 'layer', %value_1: 'value'
  %b3 = block {
    %12:texture_storage_2d_array<rgba8unorm, write> = load %texture
    %13:void = textureStore %12, %coords_1, %layer_1, %value_1
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %texture:ptr<handle, texture_storage_2d_array<rgba8unorm, write>, read> = var @binding_point(0, 0)
}

%load_signed = func(%coords:vec2<i32>, %layer:i32, %value:vec4<f32>):void -> %b2 {
  %b2 = block {
    %6:texture_storage_2d_array<rgba8unorm, write> = load %texture
    %7:vec2<u32> = textureDimensions %6
    %8:vec2<u32> = sub %7, vec2<u32>(1u)
    %9:vec2<u32> = convert %coords
    %10:vec2<u32> = min %9, %8
    %11:u32 = textureNumLayers %6
    %12:u32 = sub %11, 1u
    %13:u32 = convert %layer
    %14:u32 = min %13, %12
    %15:void = textureStore %6, %10, %14, %value
    ret
  }
}
%load_unsigned = func(%coords_1:vec2<u32>, %layer_1:u32, %value_1:vec4<f32>):void -> %b3 {  # %coords_1: 'coords', %layer_1: 'layer', %value_1: 'value'
  %b3 = block {
    %20:texture_storage_2d_array<rgba8unorm, write> = load %texture
    %21:vec2<u32> = textureDimensions %20
    %22:vec2<u32> = sub %21, vec2<u32>(1u)
    %23:vec2<u32> = min %coords_1, %22
    %24:u32 = textureNumLayers %20
    %25:u32 = sub %24, 1u
    %26:u32 = min %layer_1, %25
    %27:void = textureStore %20, %23, %26, %value_1
    ret
  }
}
)";

    RobustnessConfig cfg;
    cfg.clamp_texture = GetParam();
    Run(Robustness, cfg);

    EXPECT_EQ(GetParam() ? expect : src, str());
}

TEST_P(IR_RobustnessTest, TextureStore_Storage3D) {
    auto format = core::TexelFormat::kRgba8Unorm;
    auto* texture =
        b.Var("texture",
              ty.ptr(handle,
                     ty.Get<type::StorageTexture>(type::TextureDimension::k3d, format, write,
                                                  type::StorageTexture::SubtypeFor(format, ty)),
                     read));
    texture->SetBindingPoint(0, 0);
    mod.root_block->Append(texture);

    {
        auto* func = b.Function("load_signed", ty.void_());
        auto* coords = b.FunctionParam("coords", ty.vec3<i32>());
        auto* value = b.FunctionParam("value", ty.vec4<f32>());
        func->SetParams({coords, value});
        b.Append(func->Block(), [&] {
            auto* handle = b.Load(texture);
            b.Call(ty.void_(), core::BuiltinFn::kTextureStore, handle, coords, value);
            b.Return(func);
        });
    }

    {
        auto* func = b.Function("load_unsigned", ty.void_());
        auto* coords = b.FunctionParam("coords", ty.vec3<u32>());
        auto* value = b.FunctionParam("value", ty.vec4<f32>());
        func->SetParams({coords, value});
        b.Append(func->Block(), [&] {
            auto* handle = b.Load(texture);
            b.Call(ty.void_(), core::BuiltinFn::kTextureStore, handle, coords, value);
            b.Return(func);
        });
    }

    auto* src = R"(
%b1 = block {  # root
  %texture:ptr<handle, texture_storage_3d<rgba8unorm, write>, read> = var @binding_point(0, 0)
}

%load_signed = func(%coords:vec3<i32>, %value:vec4<f32>):void -> %b2 {
  %b2 = block {
    %5:texture_storage_3d<rgba8unorm, write> = load %texture
    %6:void = textureStore %5, %coords, %value
    ret
  }
}
%load_unsigned = func(%coords_1:vec3<u32>, %value_1:vec4<f32>):void -> %b3 {  # %coords_1: 'coords', %value_1: 'value'
  %b3 = block {
    %10:texture_storage_3d<rgba8unorm, write> = load %texture
    %11:void = textureStore %10, %coords_1, %value_1
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %texture:ptr<handle, texture_storage_3d<rgba8unorm, write>, read> = var @binding_point(0, 0)
}

%load_signed = func(%coords:vec3<i32>, %value:vec4<f32>):void -> %b2 {
  %b2 = block {
    %5:texture_storage_3d<rgba8unorm, write> = load %texture
    %6:vec3<u32> = textureDimensions %5
    %7:vec3<u32> = sub %6, vec3<u32>(1u)
    %8:vec3<u32> = convert %coords
    %9:vec3<u32> = min %8, %7
    %10:void = textureStore %5, %9, %value
    ret
  }
}
%load_unsigned = func(%coords_1:vec3<u32>, %value_1:vec4<f32>):void -> %b3 {  # %coords_1: 'coords', %value_1: 'value'
  %b3 = block {
    %14:texture_storage_3d<rgba8unorm, write> = load %texture
    %15:vec3<u32> = textureDimensions %14
    %16:vec3<u32> = sub %15, vec3<u32>(1u)
    %17:vec3<u32> = min %coords_1, %16
    %18:void = textureStore %14, %17, %value_1
    ret
  }
}
)";

    RobustnessConfig cfg;
    cfg.clamp_texture = GetParam();
    Run(Robustness, cfg);

    EXPECT_EQ(GetParam() ? expect : src, str());
}

}  // namespace
}  // namespace tint::core::ir::transform
