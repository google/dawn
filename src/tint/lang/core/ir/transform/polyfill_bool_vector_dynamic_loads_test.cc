// Copyright 2026 The Dawn & Tint Authors
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

#include "src/tint/lang/core/ir/transform/polyfill_bool_vector_dynamic_loads.h"

#include "src/tint/lang/core/ir/transform/helper_test.h"

namespace tint::core::ir::transform {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

using IR_PolyfillBoolVectorDynamicLoadsTest = TransformTest;

TEST_F(IR_PolyfillBoolVectorDynamicLoadsTest, Vec2Bool_U32Index) {
    auto* func = b.Function("foo", ty.bool_());
    auto* vec = b.FunctionParam("vec", ty.vec2<bool>());
    auto* idx = b.FunctionParam("idx", ty.u32());
    func->SetParams({vec, idx});
    b.Append(func->Block(), [&] { b.Return(func, b.Access(ty.bool_(), vec, idx)); });

    auto* src = R"(
%foo = func(%vec:vec2<bool>, %idx:u32):bool {
  $B1: {
    %4:bool = access %vec, %idx
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%vec:vec2<bool>, %idx:u32):bool {
  $B1: {
    %4:bool = access %vec, 0u
    %5:bool = access %vec, 1u
    %6:bool = eq %idx, 1u
    %7:bool = select %4, %5, %6
    ret %7
  }
}
)";

    Run(PolyfillBoolVectorDynamicLoads);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_PolyfillBoolVectorDynamicLoadsTest, Vec4Bool_I32Index) {
    auto* func = b.Function("foo", ty.bool_());
    auto* vec = b.FunctionParam("vec", ty.vec4<bool>());
    auto* idx = b.FunctionParam("idx", ty.i32());
    func->SetParams({vec, idx});
    b.Append(func->Block(), [&] { b.Return(func, b.Access(ty.bool_(), vec, idx)); });

    auto* src = R"(
%foo = func(%vec:vec4<bool>, %idx:i32):bool {
  $B1: {
    %4:bool = access %vec, %idx
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%vec:vec4<bool>, %idx:i32):bool {
  $B1: {
    %4:u32 = convert %idx
    %5:bool = access %vec, 0u
    %6:bool = access %vec, 1u
    %7:bool = eq %4, 1u
    %8:bool = select %5, %6, %7
    %9:bool = access %vec, 2u
    %10:bool = eq %4, 2u
    %11:bool = select %8, %9, %10
    %12:bool = access %vec, 3u
    %13:bool = eq %4, 3u
    %14:bool = select %11, %12, %13
    ret %14
  }
}
)";

    Run(PolyfillBoolVectorDynamicLoads);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_PolyfillBoolVectorDynamicLoadsTest, ConstantVector) {
    auto* func = b.Function("foo", ty.bool_());
    auto* idx = b.FunctionParam("idx", ty.u32());
    func->SetParams({idx});
    b.Append(func->Block(), [&] {
        b.Return(func, b.Access(ty.bool_(), b.Composite(ty.vec3<bool>(), true, false, true), idx));
    });

    auto* src = R"(
%foo = func(%idx:u32):bool {
  $B1: {
    %3:bool = access vec3<bool>(true, false, true), %idx
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%idx:u32):bool {
  $B1: {
    %3:bool = eq %idx, 1u
    %4:bool = select true, false, %3
    %5:bool = eq %idx, 2u
    %6:bool = select %4, true, %5
    ret %6
  }
}
)";

    Run(PolyfillBoolVectorDynamicLoads);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_PolyfillBoolVectorDynamicLoadsTest, NoModify_ConstantIndexOrNonBoolVector) {
    auto* func = b.Function("foo", ty.bool_());
    auto* bvec = b.FunctionParam("bvec", ty.vec3<bool>());
    auto* ivec = b.FunctionParam("ivec", ty.vec3<i32>());
    auto* barr = b.FunctionParam("barr", ty.array<bool, 3>());
    auto* idx = b.FunctionParam("idx", ty.u32());
    func->SetParams({bvec, ivec, barr, idx});
    b.Append(func->Block(), [&] {
        auto* constant_index = b.Access(ty.bool_(), bvec, 1_u);
        auto* int_vector = b.Access(ty.i32(), ivec, idx);
        auto* bool_array = b.Access(ty.bool_(), barr, idx);
        auto* cmp = b.Equal(int_vector, 0_i);
        auto* sel = b.Call(ty.bool_(), core::BuiltinFn::kSelect, constant_index, bool_array, cmp);
        b.Return(func, sel);
    });

    auto* src = R"(
%foo = func(%bvec:vec3<bool>, %ivec:vec3<i32>, %barr:array<bool, 3>, %idx:u32):bool {
  $B1: {
    %6:bool = access %bvec, 1u
    %7:i32 = access %ivec, %idx
    %8:bool = access %barr, %idx
    %9:bool = eq %7, 0i
    %10:bool = select %6, %8, %9
    ret %10
  }
}
)";
    EXPECT_EQ(src, str());

    Run(PolyfillBoolVectorDynamicLoads);

    EXPECT_EQ(src, str());
}

}  // namespace
}  // namespace tint::core::ir::transform
