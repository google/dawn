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

#include "src/tint/lang/msl/writer/raise/cooperative_tensors.h"

#include "gtest/gtest.h"
#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/ir/transform/helper_test.h"
#include "src/tint/lang/core/number.h"
#include "src/tint/lang/msl/builtin_fn.h"
#include "src/tint/lang/msl/ir/builtin_call.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::msl::writer::raise {
namespace {

struct MslWriter_CooperativeTensorsTest : public core::ir::transform::TransformTest {
  protected:
    void SetUp() override {
        mod.properties.Add(core::ir::Property::kAllow16BitFloats);
        mod.properties.Add(core::ir::Property::kAllow8BitIntegers);
    }
};

TEST_F(MslWriter_CooperativeTensorsTest, Construct_ZeroValue) {
    auto* ep = b.ComputeFunction("entry");
    b.Append(ep->Block(), [&] {
        b.Construct(ty.subgroup_matrix_result(ty.f16(), 32, 32));
        b.Construct(ty.subgroup_matrix_result(ty.f32(), 16, 32));
        b.Construct(ty.subgroup_matrix_result(ty.u8(), 32, 16));
        b.Return(ep);
    });

    auto* src = R"(
%entry = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:subgroup_matrix_result<f16, 32, 32> = construct
    %3:subgroup_matrix_result<f32, 16, 32> = construct
    %4:subgroup_matrix_result<u8, 32, 16> = construct
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%entry = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, msl.cooperative_tensor_result<32, 32, 32, f16, f16>, read_write> = var undef
    %3:void = msl.fill_cooperative_tensor %2, 0.0h
    %4:ptr<function, msl.cooperative_tensor_result<32, 16, 32, f32, f32>, read_write> = var undef
    %5:void = msl.fill_cooperative_tensor %4, 0.0f
    %6:ptr<function, msl.cooperative_tensor_result<16, 32, 32, u8, u8>, read_write> = var undef
    %7:void = msl.fill_cooperative_tensor %6, 0u8
    ret
  }
}
)";

    Run(CooperativeTensors);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_CooperativeTensorsTest, Construct_WithValue) {
    auto* ep = b.ComputeFunction("entry");
    b.Append(ep->Block(), [&] {
        b.Construct(ty.subgroup_matrix_result(ty.f16(), 32, 32), 1.0_h);
        b.Construct(ty.subgroup_matrix_result(ty.f32(), 16, 32), 2.0_f);
        b.Construct(ty.subgroup_matrix_result(ty.u8(), 32, 16), u8(3));
        b.Return(ep);
    });

    auto* src = R"(
%entry = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:subgroup_matrix_result<f16, 32, 32> = construct 1.0h
    %3:subgroup_matrix_result<f32, 16, 32> = construct 2.0f
    %4:subgroup_matrix_result<u8, 32, 16> = construct 3u8
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%entry = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, msl.cooperative_tensor_result<32, 32, 32, f16, f16>, read_write> = var undef
    %3:void = msl.fill_cooperative_tensor %2, 1.0h
    %4:ptr<function, msl.cooperative_tensor_result<32, 16, 32, f32, f32>, read_write> = var undef
    %5:void = msl.fill_cooperative_tensor %4, 2.0f
    %6:ptr<function, msl.cooperative_tensor_result<16, 32, 32, u8, u8>, read_write> = var undef
    %7:void = msl.fill_cooperative_tensor %6, 3u8
    ret
  }
}
)";

    Run(CooperativeTensors);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_CooperativeTensorsTest, Construct_Array_ZeroValue) {
    auto* ep = b.ComputeFunction("entry");
    b.Append(ep->Block(), [&] {
        b.Construct(ty.array(ty.subgroup_matrix_result(ty.f32(), 16, 32), 2));
        b.Return(ep);
    });

    auto* src = R"(
%entry = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:array<subgroup_matrix_result<f32, 16, 32>, 2> = construct
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%entry = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, msl.cooperative_tensor_result<32, 16, 32, f32, f32>, read_write> = var undef
    %3:ptr<function, msl.cooperative_tensor_result<32, 16, 32, f32, f32>, read_write> = var undef
    %4:array<ptr<function, msl.cooperative_tensor_result<32, 16, 32, f32, f32>, read_write>, 2> = construct %2, %3
    %5:ptr<function, msl.cooperative_tensor_result<32, 16, 32, f32, f32>, read_write> = access %4, 0u
    %6:void = msl.fill_cooperative_tensor %5, 0.0f
    %7:ptr<function, msl.cooperative_tensor_result<32, 16, 32, f32, f32>, read_write> = access %4, 1u
    %8:void = msl.fill_cooperative_tensor %7, 0.0f
    ret
  }
}
)";

    Run(CooperativeTensors);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_CooperativeTensorsTest, Construct_Array_WithValues) {
    auto* ep = b.ComputeFunction("entry");
    b.Append(ep->Block(), [&] {
        auto* v1 = b.Construct(ty.subgroup_matrix_result(ty.f32(), 16, 32), 1_f);
        auto* v2 = b.Construct(ty.subgroup_matrix_result(ty.f32(), 16, 32), 2_f);
        b.Construct(ty.array(ty.subgroup_matrix_result(ty.f32(), 16, 32), 3), v1, v2, v2);
        b.Return(ep);
    });

    auto* src = R"(
%entry = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:subgroup_matrix_result<f32, 16, 32> = construct 1.0f
    %3:subgroup_matrix_result<f32, 16, 32> = construct 2.0f
    %4:array<subgroup_matrix_result<f32, 16, 32>, 3> = construct %2, %3, %3
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%entry = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, msl.cooperative_tensor_result<32, 16, 32, f32, f32>, read_write> = var undef
    %3:void = msl.fill_cooperative_tensor %2, 1.0f
    %4:ptr<function, msl.cooperative_tensor_result<32, 16, 32, f32, f32>, read_write> = var undef
    %5:void = msl.fill_cooperative_tensor %4, 2.0f
    %6:ptr<function, msl.cooperative_tensor_result<32, 16, 32, f32, f32>, read_write> = var undef
    %7:void = msl.copy_cooperative_tensor %6, %4
    %8:ptr<function, msl.cooperative_tensor_result<32, 16, 32, f32, f32>, read_write> = var undef
    %9:void = msl.copy_cooperative_tensor %8, %4
    %10:array<ptr<function, msl.cooperative_tensor_result<32, 16, 32, f32, f32>, read_write>, 3> = construct %2, %6, %8
    ret
  }
}
)";

    Run(CooperativeTensors);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_CooperativeTensorsTest, Construct_ArrayOfArray_ZeroValue) {
    auto* ep = b.ComputeFunction("entry");
    b.Append(ep->Block(), [&] {
        b.Construct(ty.array(ty.array(ty.subgroup_matrix_result(ty.f32(), 16, 32), 2), 3));
        b.Return(ep);
    });

    auto* src = R"(
%entry = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:array<array<subgroup_matrix_result<f32, 16, 32>, 2>, 3> = construct
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%entry = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, msl.cooperative_tensor_result<32, 16, 32, f32, f32>, read_write> = var undef
    %3:ptr<function, msl.cooperative_tensor_result<32, 16, 32, f32, f32>, read_write> = var undef
    %4:array<ptr<function, msl.cooperative_tensor_result<32, 16, 32, f32, f32>, read_write>, 2> = construct %2, %3
    %5:ptr<function, msl.cooperative_tensor_result<32, 16, 32, f32, f32>, read_write> = var undef
    %6:ptr<function, msl.cooperative_tensor_result<32, 16, 32, f32, f32>, read_write> = var undef
    %7:array<ptr<function, msl.cooperative_tensor_result<32, 16, 32, f32, f32>, read_write>, 2> = construct %5, %6
    %8:ptr<function, msl.cooperative_tensor_result<32, 16, 32, f32, f32>, read_write> = var undef
    %9:ptr<function, msl.cooperative_tensor_result<32, 16, 32, f32, f32>, read_write> = var undef
    %10:array<ptr<function, msl.cooperative_tensor_result<32, 16, 32, f32, f32>, read_write>, 2> = construct %8, %9
    %11:array<array<ptr<function, msl.cooperative_tensor_result<32, 16, 32, f32, f32>, read_write>, 2>, 3> = construct %4, %7, %10
    %12:array<ptr<function, msl.cooperative_tensor_result<32, 16, 32, f32, f32>, read_write>, 2> = access %11, 0u
    %13:ptr<function, msl.cooperative_tensor_result<32, 16, 32, f32, f32>, read_write> = access %12, 0u
    %14:void = msl.fill_cooperative_tensor %13, 0.0f
    %15:ptr<function, msl.cooperative_tensor_result<32, 16, 32, f32, f32>, read_write> = access %12, 1u
    %16:void = msl.fill_cooperative_tensor %15, 0.0f
    %17:array<ptr<function, msl.cooperative_tensor_result<32, 16, 32, f32, f32>, read_write>, 2> = access %11, 1u
    %18:ptr<function, msl.cooperative_tensor_result<32, 16, 32, f32, f32>, read_write> = access %17, 0u
    %19:void = msl.fill_cooperative_tensor %18, 0.0f
    %20:ptr<function, msl.cooperative_tensor_result<32, 16, 32, f32, f32>, read_write> = access %17, 1u
    %21:void = msl.fill_cooperative_tensor %20, 0.0f
    %22:array<ptr<function, msl.cooperative_tensor_result<32, 16, 32, f32, f32>, read_write>, 2> = access %11, 2u
    %23:ptr<function, msl.cooperative_tensor_result<32, 16, 32, f32, f32>, read_write> = access %22, 0u
    %24:void = msl.fill_cooperative_tensor %23, 0.0f
    %25:ptr<function, msl.cooperative_tensor_result<32, 16, 32, f32, f32>, read_write> = access %22, 1u
    %26:void = msl.fill_cooperative_tensor %25, 0.0f
    ret
  }
}
)";

    Run(CooperativeTensors);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_CooperativeTensorsTest, Construct_ArrayOfArray_WithValues) {
    auto* ep = b.ComputeFunction("entry");
    b.Append(ep->Block(), [&] {
        auto* v1 = b.Construct(ty.array(ty.subgroup_matrix_result(ty.f32(), 16, 32), 2));
        auto* v2 = b.Construct(ty.array(ty.subgroup_matrix_result(ty.f32(), 16, 32), 2));
        b.Construct(ty.array(ty.array(ty.subgroup_matrix_result(ty.f32(), 16, 32), 2), 3), v1, v2,
                    v2);
        b.Return(ep);
    });

    auto* src = R"(
%entry = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:array<subgroup_matrix_result<f32, 16, 32>, 2> = construct
    %3:array<subgroup_matrix_result<f32, 16, 32>, 2> = construct
    %4:array<array<subgroup_matrix_result<f32, 16, 32>, 2>, 3> = construct %2, %3, %3
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%entry = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, msl.cooperative_tensor_result<32, 16, 32, f32, f32>, read_write> = var undef
    %3:ptr<function, msl.cooperative_tensor_result<32, 16, 32, f32, f32>, read_write> = var undef
    %4:array<ptr<function, msl.cooperative_tensor_result<32, 16, 32, f32, f32>, read_write>, 2> = construct %2, %3
    %5:ptr<function, msl.cooperative_tensor_result<32, 16, 32, f32, f32>, read_write> = access %4, 0u
    %6:void = msl.fill_cooperative_tensor %5, 0.0f
    %7:ptr<function, msl.cooperative_tensor_result<32, 16, 32, f32, f32>, read_write> = access %4, 1u
    %8:void = msl.fill_cooperative_tensor %7, 0.0f
    %9:ptr<function, msl.cooperative_tensor_result<32, 16, 32, f32, f32>, read_write> = var undef
    %10:ptr<function, msl.cooperative_tensor_result<32, 16, 32, f32, f32>, read_write> = var undef
    %11:array<ptr<function, msl.cooperative_tensor_result<32, 16, 32, f32, f32>, read_write>, 2> = construct %9, %10
    %12:ptr<function, msl.cooperative_tensor_result<32, 16, 32, f32, f32>, read_write> = access %11, 0u
    %13:void = msl.fill_cooperative_tensor %12, 0.0f
    %14:ptr<function, msl.cooperative_tensor_result<32, 16, 32, f32, f32>, read_write> = access %11, 1u
    %15:void = msl.fill_cooperative_tensor %14, 0.0f
    %16:ptr<function, msl.cooperative_tensor_result<32, 16, 32, f32, f32>, read_write> = var undef
    %17:ptr<function, msl.cooperative_tensor_result<32, 16, 32, f32, f32>, read_write> = var undef
    %18:array<ptr<function, msl.cooperative_tensor_result<32, 16, 32, f32, f32>, read_write>, 2> = construct %16, %17
    %19:ptr<function, msl.cooperative_tensor_result<32, 16, 32, f32, f32>, read_write> = access %18, 0u
    %20:ptr<function, msl.cooperative_tensor_result<32, 16, 32, f32, f32>, read_write> = access %11, 0u
    %21:void = msl.copy_cooperative_tensor %19, %20
    %22:ptr<function, msl.cooperative_tensor_result<32, 16, 32, f32, f32>, read_write> = access %18, 1u
    %23:ptr<function, msl.cooperative_tensor_result<32, 16, 32, f32, f32>, read_write> = access %11, 1u
    %24:void = msl.copy_cooperative_tensor %22, %23
    %25:ptr<function, msl.cooperative_tensor_result<32, 16, 32, f32, f32>, read_write> = var undef
    %26:ptr<function, msl.cooperative_tensor_result<32, 16, 32, f32, f32>, read_write> = var undef
    %27:array<ptr<function, msl.cooperative_tensor_result<32, 16, 32, f32, f32>, read_write>, 2> = construct %25, %26
    %28:ptr<function, msl.cooperative_tensor_result<32, 16, 32, f32, f32>, read_write> = access %27, 0u
    %29:ptr<function, msl.cooperative_tensor_result<32, 16, 32, f32, f32>, read_write> = access %11, 0u
    %30:void = msl.copy_cooperative_tensor %28, %29
    %31:ptr<function, msl.cooperative_tensor_result<32, 16, 32, f32, f32>, read_write> = access %27, 1u
    %32:ptr<function, msl.cooperative_tensor_result<32, 16, 32, f32, f32>, read_write> = access %11, 1u
    %33:void = msl.copy_cooperative_tensor %31, %32
    %34:array<array<ptr<function, msl.cooperative_tensor_result<32, 16, 32, f32, f32>, read_write>, 2>, 3> = construct %4, %18, %27
    ret
  }
}
)";

    Run(CooperativeTensors);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_CooperativeTensorsTest, VarWithNoInitializer) {
    auto* ep = b.ComputeFunction("entry");
    b.Append(ep->Block(), [&] {
        b.Var("lhs", ty.ptr(function, ty.subgroup_matrix_left(ty.f16(), 32, 32)));
        b.Var("rhs", ty.ptr(function, ty.subgroup_matrix_right(ty.f32(), 16, 32)));
        b.Var("acc", ty.ptr(function, ty.subgroup_matrix_result(ty.f16(), 32, 16)));
        b.Return(ep);
    });

    auto* src = R"(
%entry = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %lhs:ptr<function, subgroup_matrix_left<f16, 32, 32>, read_write> = var undef
    %rhs:ptr<function, subgroup_matrix_right<f32, 16, 32>, read_write> = var undef
    %acc:ptr<function, subgroup_matrix_result<f16, 32, 16>, read_write> = var undef
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%entry = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %lhs:ptr<function, msl.cooperative_tensor_left<32, 32, 32, f16, f16>, read_write> = var undef
    %3:void = msl.fill_cooperative_tensor %lhs, 0.0h
    %rhs:ptr<function, msl.cooperative_tensor_right<32, 16, 32, f32, f32>, read_write> = var undef
    %5:void = msl.fill_cooperative_tensor %rhs, 0.0f
    %acc:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %7:void = msl.fill_cooperative_tensor %acc, 0.0h
    ret
  }
}
)";

    Run(CooperativeTensors);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_CooperativeTensorsTest, VarWithNoInitializer_Array) {
    auto* ep = b.ComputeFunction("entry");
    b.Append(ep->Block(), [&] {
        b.Var("acc", ty.ptr(function, ty.array(ty.subgroup_matrix_result(ty.f16(), 32, 16), 2)));
        b.Return(ep);
    });

    auto* src = R"(
%entry = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %acc:ptr<function, array<subgroup_matrix_result<f16, 32, 16>, 2>, read_write> = var undef
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%entry = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %3:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %4:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = construct %2, %3
    %acc:ptr<function, array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2>, read_write> = var %4
    %6:ptr<function, ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, read_write> = access %acc, 0u
    %7:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = load %6
    %8:void = msl.fill_cooperative_tensor %7, 0.0h
    %9:ptr<function, ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, read_write> = access %acc, 1u
    %10:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = load %9
    %11:void = msl.fill_cooperative_tensor %10, 0.0h
    ret
  }
}
)";

    Run(CooperativeTensors);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_CooperativeTensorsTest, VarWithNoInitializer_ArrayOfArray) {
    auto* ep = b.ComputeFunction("entry");
    b.Append(ep->Block(), [&] {
        b.Var("acc", ty.ptr(function,
                            ty.array(ty.array(ty.subgroup_matrix_result(ty.f16(), 32, 16), 2), 3)));
        b.Return(ep);
    });

    auto* src = R"(
%entry = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %acc:ptr<function, array<array<subgroup_matrix_result<f16, 32, 16>, 2>, 3>, read_write> = var undef
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%entry = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %3:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %4:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = construct %2, %3
    %5:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %6:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %7:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = construct %5, %6
    %8:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %9:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %10:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = construct %8, %9
    %11:array<array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2>, 3> = construct %4, %7, %10
    %acc:ptr<function, array<array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2>, 3>, read_write> = var %11
    %13:ptr<function, array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2>, read_write> = access %acc, 0u
    %14:ptr<function, ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, read_write> = access %13, 0u
    %15:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = load %14
    %16:void = msl.fill_cooperative_tensor %15, 0.0h
    %17:ptr<function, ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, read_write> = access %13, 1u
    %18:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = load %17
    %19:void = msl.fill_cooperative_tensor %18, 0.0h
    %20:ptr<function, array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2>, read_write> = access %acc, 1u
    %21:ptr<function, ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, read_write> = access %20, 0u
    %22:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = load %21
    %23:void = msl.fill_cooperative_tensor %22, 0.0h
    %24:ptr<function, ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, read_write> = access %20, 1u
    %25:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = load %24
    %26:void = msl.fill_cooperative_tensor %25, 0.0h
    %27:ptr<function, array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2>, read_write> = access %acc, 2u
    %28:ptr<function, ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, read_write> = access %27, 0u
    %29:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = load %28
    %30:void = msl.fill_cooperative_tensor %29, 0.0h
    %31:ptr<function, ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, read_write> = access %27, 1u
    %32:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = load %31
    %33:void = msl.fill_cooperative_tensor %32, 0.0h
    ret
  }
}
)";

    Run(CooperativeTensors);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_CooperativeTensorsTest, VarWithSingleUseInitializer) {
    auto* ep = b.ComputeFunction("entry");
    b.Append(ep->Block(), [&] {
        b.Var("acc", b.Construct(ty.subgroup_matrix_result(ty.f16(), 32, 16)));
        b.Return(ep);
    });

    auto* src = R"(
%entry = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:subgroup_matrix_result<f16, 32, 16> = construct
    %acc:ptr<function, subgroup_matrix_result<f16, 32, 16>, read_write> = var %2
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%entry = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %acc:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %3:void = msl.fill_cooperative_tensor %acc, 0.0h
    ret
  }
}
)";

    Run(CooperativeTensors);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_CooperativeTensorsTest, VarWithSingleUseInitializer_Array) {
    auto* ep = b.ComputeFunction("entry");
    b.Append(ep->Block(), [&] {
        b.Var("acc", b.Construct(ty.array(ty.subgroup_matrix_result(ty.f16(), 32, 16), 2)));
        b.Return(ep);
    });

    auto* src = R"(
%entry = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:array<subgroup_matrix_result<f16, 32, 16>, 2> = construct
    %acc:ptr<function, array<subgroup_matrix_result<f16, 32, 16>, 2>, read_write> = var %2
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%entry = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %3:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %4:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = construct %2, %3
    %5:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %4, 0u
    %6:void = msl.fill_cooperative_tensor %5, 0.0h
    %7:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %4, 1u
    %8:void = msl.fill_cooperative_tensor %7, 0.0h
    %acc:ptr<function, array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2>, read_write> = var %4
    ret
  }
}
)";

    Run(CooperativeTensors);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_CooperativeTensorsTest, VarWithSingleUseInitializer_ArrayOfArray) {
    auto* ep = b.ComputeFunction("entry");
    b.Append(ep->Block(), [&] {
        b.Var("acc",
              b.Construct(ty.array(ty.array(ty.subgroup_matrix_result(ty.f16(), 32, 16), 2), 3)));
        b.Return(ep);
    });

    auto* src = R"(
%entry = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:array<array<subgroup_matrix_result<f16, 32, 16>, 2>, 3> = construct
    %acc:ptr<function, array<array<subgroup_matrix_result<f16, 32, 16>, 2>, 3>, read_write> = var %2
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%entry = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %3:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %4:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = construct %2, %3
    %5:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %6:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %7:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = construct %5, %6
    %8:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %9:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %10:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = construct %8, %9
    %11:array<array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2>, 3> = construct %4, %7, %10
    %12:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = access %11, 0u
    %13:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %12, 0u
    %14:void = msl.fill_cooperative_tensor %13, 0.0h
    %15:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %12, 1u
    %16:void = msl.fill_cooperative_tensor %15, 0.0h
    %17:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = access %11, 1u
    %18:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %17, 0u
    %19:void = msl.fill_cooperative_tensor %18, 0.0h
    %20:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %17, 1u
    %21:void = msl.fill_cooperative_tensor %20, 0.0h
    %22:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = access %11, 2u
    %23:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %22, 0u
    %24:void = msl.fill_cooperative_tensor %23, 0.0h
    %25:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %22, 1u
    %26:void = msl.fill_cooperative_tensor %25, 0.0h
    %acc:ptr<function, array<array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2>, 3>, read_write> = var %11
    ret
  }
}
)";

    Run(CooperativeTensors);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_CooperativeTensorsTest, VarWithMultipleUseInitializer) {
    auto* ep = b.ComputeFunction("entry");
    b.Append(ep->Block(), [&] {
        auto* init = b.Construct(ty.subgroup_matrix_result(ty.f16(), 32, 16));
        b.Var("acc1", init);
        b.Var("acc2", init);
        b.Return(ep);
    });

    auto* src = R"(
%entry = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:subgroup_matrix_result<f16, 32, 16> = construct
    %acc1:ptr<function, subgroup_matrix_result<f16, 32, 16>, read_write> = var %2
    %acc2:ptr<function, subgroup_matrix_result<f16, 32, 16>, read_write> = var %2
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%entry = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %3:void = msl.fill_cooperative_tensor %2, 0.0h
    %acc1:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %5:void = msl.copy_cooperative_tensor %acc1, %2
    %acc2:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %7:void = msl.copy_cooperative_tensor %acc2, %2
    ret
  }
}
)";

    Run(CooperativeTensors);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_CooperativeTensorsTest, VarWithMultipleUseInitializer_Array) {
    auto* ep = b.ComputeFunction("entry");
    b.Append(ep->Block(), [&] {
        auto* init = b.Construct(ty.array(ty.subgroup_matrix_result(ty.f16(), 32, 16), 2));
        b.Var("acc1", init);
        b.Var("acc2", init);
        b.Return(ep);
    });

    auto* src = R"(
%entry = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:array<subgroup_matrix_result<f16, 32, 16>, 2> = construct
    %acc1:ptr<function, array<subgroup_matrix_result<f16, 32, 16>, 2>, read_write> = var %2
    %acc2:ptr<function, array<subgroup_matrix_result<f16, 32, 16>, 2>, read_write> = var %2
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%entry = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %3:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %4:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = construct %2, %3
    %5:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %4, 0u
    %6:void = msl.fill_cooperative_tensor %5, 0.0h
    %7:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %4, 1u
    %8:void = msl.fill_cooperative_tensor %7, 0.0h
    %acc1:ptr<function, array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2>, read_write> = var undef
    %10:ptr<function, ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, read_write> = access %acc1, 0u
    %11:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %4, 0u
    %12:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = load %10
    %13:void = msl.copy_cooperative_tensor %12, %11
    %14:ptr<function, ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, read_write> = access %acc1, 1u
    %15:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %4, 1u
    %16:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = load %14
    %17:void = msl.copy_cooperative_tensor %16, %15
    %acc2:ptr<function, array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2>, read_write> = var undef
    %19:ptr<function, ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, read_write> = access %acc2, 0u
    %20:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %4, 0u
    %21:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = load %19
    %22:void = msl.copy_cooperative_tensor %21, %20
    %23:ptr<function, ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, read_write> = access %acc2, 1u
    %24:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %4, 1u
    %25:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = load %23
    %26:void = msl.copy_cooperative_tensor %25, %24
    ret
  }
}
)";

    Run(CooperativeTensors);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_CooperativeTensorsTest, VarWithMultipleUseInitializer_ArrayOfArray) {
    auto* ep = b.ComputeFunction("entry");
    b.Append(ep->Block(), [&] {
        auto* init =
            b.Construct(ty.array(ty.array(ty.subgroup_matrix_result(ty.f16(), 32, 16), 2), 3));
        b.Var("acc1", init);
        b.Var("acc2", init);
        b.Return(ep);
    });

    auto* src = R"(
%entry = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:array<array<subgroup_matrix_result<f16, 32, 16>, 2>, 3> = construct
    %acc1:ptr<function, array<array<subgroup_matrix_result<f16, 32, 16>, 2>, 3>, read_write> = var %2
    %acc2:ptr<function, array<array<subgroup_matrix_result<f16, 32, 16>, 2>, 3>, read_write> = var %2
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%entry = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %3:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %4:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = construct %2, %3
    %5:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %6:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %7:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = construct %5, %6
    %8:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %9:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %10:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = construct %8, %9
    %11:array<array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2>, 3> = construct %4, %7, %10
    %12:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = access %11, 0u
    %13:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %12, 0u
    %14:void = msl.fill_cooperative_tensor %13, 0.0h
    %15:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %12, 1u
    %16:void = msl.fill_cooperative_tensor %15, 0.0h
    %17:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = access %11, 1u
    %18:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %17, 0u
    %19:void = msl.fill_cooperative_tensor %18, 0.0h
    %20:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %17, 1u
    %21:void = msl.fill_cooperative_tensor %20, 0.0h
    %22:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = access %11, 2u
    %23:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %22, 0u
    %24:void = msl.fill_cooperative_tensor %23, 0.0h
    %25:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %22, 1u
    %26:void = msl.fill_cooperative_tensor %25, 0.0h
    %acc1:ptr<function, array<array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2>, 3>, read_write> = var undef
    %28:ptr<function, array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2>, read_write> = access %acc1, 0u
    %29:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = access %11, 0u
    %30:ptr<function, ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, read_write> = access %28, 0u
    %31:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %29, 0u
    %32:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = load %30
    %33:void = msl.copy_cooperative_tensor %32, %31
    %34:ptr<function, ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, read_write> = access %28, 1u
    %35:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %29, 1u
    %36:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = load %34
    %37:void = msl.copy_cooperative_tensor %36, %35
    %38:ptr<function, array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2>, read_write> = access %acc1, 1u
    %39:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = access %11, 1u
    %40:ptr<function, ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, read_write> = access %38, 0u
    %41:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %39, 0u
    %42:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = load %40
    %43:void = msl.copy_cooperative_tensor %42, %41
    %44:ptr<function, ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, read_write> = access %38, 1u
    %45:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %39, 1u
    %46:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = load %44
    %47:void = msl.copy_cooperative_tensor %46, %45
    %48:ptr<function, array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2>, read_write> = access %acc1, 2u
    %49:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = access %11, 2u
    %50:ptr<function, ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, read_write> = access %48, 0u
    %51:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %49, 0u
    %52:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = load %50
    %53:void = msl.copy_cooperative_tensor %52, %51
    %54:ptr<function, ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, read_write> = access %48, 1u
    %55:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %49, 1u
    %56:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = load %54
    %57:void = msl.copy_cooperative_tensor %56, %55
    %acc2:ptr<function, array<array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2>, 3>, read_write> = var undef
    %59:ptr<function, array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2>, read_write> = access %acc2, 0u
    %60:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = access %11, 0u
    %61:ptr<function, ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, read_write> = access %59, 0u
    %62:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %60, 0u
    %63:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = load %61
    %64:void = msl.copy_cooperative_tensor %63, %62
    %65:ptr<function, ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, read_write> = access %59, 1u
    %66:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %60, 1u
    %67:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = load %65
    %68:void = msl.copy_cooperative_tensor %67, %66
    %69:ptr<function, array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2>, read_write> = access %acc2, 1u
    %70:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = access %11, 1u
    %71:ptr<function, ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, read_write> = access %69, 0u
    %72:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %70, 0u
    %73:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = load %71
    %74:void = msl.copy_cooperative_tensor %73, %72
    %75:ptr<function, ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, read_write> = access %69, 1u
    %76:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %70, 1u
    %77:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = load %75
    %78:void = msl.copy_cooperative_tensor %77, %76
    %79:ptr<function, array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2>, read_write> = access %acc2, 2u
    %80:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = access %11, 2u
    %81:ptr<function, ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, read_write> = access %79, 0u
    %82:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %80, 0u
    %83:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = load %81
    %84:void = msl.copy_cooperative_tensor %83, %82
    %85:ptr<function, ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, read_write> = access %79, 1u
    %86:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %80, 1u
    %87:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = load %85
    %88:void = msl.copy_cooperative_tensor %87, %86
    ret
  }
}
)";

    Run(CooperativeTensors);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_CooperativeTensorsTest, LetWithSingleUseInitializer) {
    auto* ep = b.ComputeFunction("entry");
    b.Append(ep->Block(), [&] {
        b.Let("acc", b.Construct(ty.subgroup_matrix_result(ty.f16(), 32, 16)));
        b.Return(ep);
    });

    auto* src = R"(
%entry = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:subgroup_matrix_result<f16, 32, 16> = construct
    %acc:subgroup_matrix_result<f16, 32, 16> = let %2
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%entry = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %acc:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %3:void = msl.fill_cooperative_tensor %acc, 0.0h
    ret
  }
}
)";

    Run(CooperativeTensors);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_CooperativeTensorsTest, LetWithSingleUseInitializer_Array) {
    auto* ep = b.ComputeFunction("entry");
    b.Append(ep->Block(), [&] {
        b.Let("acc", b.Construct(ty.array(ty.subgroup_matrix_result(ty.f16(), 32, 16), 2)));
        b.Return(ep);
    });

    auto* src = R"(
%entry = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:array<subgroup_matrix_result<f16, 32, 16>, 2> = construct
    %acc:array<subgroup_matrix_result<f16, 32, 16>, 2> = let %2
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%entry = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %3:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %acc:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = construct %2, %3
    %5:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %acc, 0u
    %6:void = msl.fill_cooperative_tensor %5, 0.0h
    %7:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %acc, 1u
    %8:void = msl.fill_cooperative_tensor %7, 0.0h
    ret
  }
}
)";

    Run(CooperativeTensors);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_CooperativeTensorsTest, LetWithSingleUseInitializer_ArrayOfArray) {
    auto* ep = b.ComputeFunction("entry");
    b.Append(ep->Block(), [&] {
        b.Let("acc",
              b.Construct(ty.array(ty.array(ty.subgroup_matrix_result(ty.f16(), 32, 16), 2), 3)));
        b.Return(ep);
    });

    auto* src = R"(
%entry = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:array<array<subgroup_matrix_result<f16, 32, 16>, 2>, 3> = construct
    %acc:array<array<subgroup_matrix_result<f16, 32, 16>, 2>, 3> = let %2
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%entry = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %3:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %4:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = construct %2, %3
    %5:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %6:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %7:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = construct %5, %6
    %8:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %9:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %10:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = construct %8, %9
    %acc:array<array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2>, 3> = construct %4, %7, %10
    %12:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = access %acc, 0u
    %13:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %12, 0u
    %14:void = msl.fill_cooperative_tensor %13, 0.0h
    %15:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %12, 1u
    %16:void = msl.fill_cooperative_tensor %15, 0.0h
    %17:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = access %acc, 1u
    %18:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %17, 0u
    %19:void = msl.fill_cooperative_tensor %18, 0.0h
    %20:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %17, 1u
    %21:void = msl.fill_cooperative_tensor %20, 0.0h
    %22:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = access %acc, 2u
    %23:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %22, 0u
    %24:void = msl.fill_cooperative_tensor %23, 0.0h
    %25:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %22, 1u
    %26:void = msl.fill_cooperative_tensor %25, 0.0h
    ret
  }
}
)";

    Run(CooperativeTensors);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_CooperativeTensorsTest, LetWithMultipleUseInitializer) {
    auto* ep = b.ComputeFunction("entry");
    b.Append(ep->Block(), [&] {
        auto* init = b.Construct(ty.subgroup_matrix_result(ty.f16(), 32, 16));
        b.Let("acc1", init);
        b.Let("acc2", init);
        b.Return(ep);
    });

    auto* src = R"(
%entry = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:subgroup_matrix_result<f16, 32, 16> = construct
    %acc1:subgroup_matrix_result<f16, 32, 16> = let %2
    %acc2:subgroup_matrix_result<f16, 32, 16> = let %2
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%entry = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %3:void = msl.fill_cooperative_tensor %2, 0.0h
    %acc1:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %5:void = msl.copy_cooperative_tensor %acc1, %2
    %acc2:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %7:void = msl.copy_cooperative_tensor %acc2, %2
    ret
  }
}
)";

    Run(CooperativeTensors);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_CooperativeTensorsTest, LetWithMultipleUseInitializer_Array) {
    auto* ep = b.ComputeFunction("entry");
    b.Append(ep->Block(), [&] {
        auto* init = b.Construct(ty.array(ty.subgroup_matrix_result(ty.f16(), 32, 16), 2));
        b.Let("acc1", init);
        b.Let("acc2", init);
        b.Return(ep);
    });

    auto* src = R"(
%entry = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:array<subgroup_matrix_result<f16, 32, 16>, 2> = construct
    %acc1:array<subgroup_matrix_result<f16, 32, 16>, 2> = let %2
    %acc2:array<subgroup_matrix_result<f16, 32, 16>, 2> = let %2
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%entry = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %3:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %4:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = construct %2, %3
    %5:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %4, 0u
    %6:void = msl.fill_cooperative_tensor %5, 0.0h
    %7:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %4, 1u
    %8:void = msl.fill_cooperative_tensor %7, 0.0h
    %9:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %10:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %acc1:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = construct %9, %10
    %12:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %acc1, 0u
    %13:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %4, 0u
    %14:void = msl.copy_cooperative_tensor %12, %13
    %15:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %acc1, 1u
    %16:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %4, 1u
    %17:void = msl.copy_cooperative_tensor %15, %16
    %18:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %19:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %acc2:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = construct %18, %19
    %21:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %acc2, 0u
    %22:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %4, 0u
    %23:void = msl.copy_cooperative_tensor %21, %22
    %24:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %acc2, 1u
    %25:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %4, 1u
    %26:void = msl.copy_cooperative_tensor %24, %25
    ret
  }
}
)";

    Run(CooperativeTensors);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_CooperativeTensorsTest, LetWithMultipleUseInitializer_ArrayOfArray) {
    auto* ep = b.ComputeFunction("entry");
    b.Append(ep->Block(), [&] {
        auto* init =
            b.Construct(ty.array(ty.array(ty.subgroup_matrix_result(ty.f16(), 32, 16), 2), 3));
        b.Let("acc1", init);
        b.Let("acc2", init);
        b.Return(ep);
    });

    auto* src = R"(
%entry = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:array<array<subgroup_matrix_result<f16, 32, 16>, 2>, 3> = construct
    %acc1:array<array<subgroup_matrix_result<f16, 32, 16>, 2>, 3> = let %2
    %acc2:array<array<subgroup_matrix_result<f16, 32, 16>, 2>, 3> = let %2
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%entry = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %3:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %4:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = construct %2, %3
    %5:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %6:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %7:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = construct %5, %6
    %8:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %9:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %10:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = construct %8, %9
    %11:array<array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2>, 3> = construct %4, %7, %10
    %12:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = access %11, 0u
    %13:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %12, 0u
    %14:void = msl.fill_cooperative_tensor %13, 0.0h
    %15:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %12, 1u
    %16:void = msl.fill_cooperative_tensor %15, 0.0h
    %17:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = access %11, 1u
    %18:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %17, 0u
    %19:void = msl.fill_cooperative_tensor %18, 0.0h
    %20:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %17, 1u
    %21:void = msl.fill_cooperative_tensor %20, 0.0h
    %22:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = access %11, 2u
    %23:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %22, 0u
    %24:void = msl.fill_cooperative_tensor %23, 0.0h
    %25:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %22, 1u
    %26:void = msl.fill_cooperative_tensor %25, 0.0h
    %27:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %28:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %29:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = construct %27, %28
    %30:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %31:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %32:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = construct %30, %31
    %33:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %34:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %35:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = construct %33, %34
    %acc1:array<array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2>, 3> = construct %29, %32, %35
    %37:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = access %acc1, 0u
    %38:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = access %11, 0u
    %39:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %37, 0u
    %40:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %38, 0u
    %41:void = msl.copy_cooperative_tensor %39, %40
    %42:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %37, 1u
    %43:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %38, 1u
    %44:void = msl.copy_cooperative_tensor %42, %43
    %45:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = access %acc1, 1u
    %46:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = access %11, 1u
    %47:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %45, 0u
    %48:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %46, 0u
    %49:void = msl.copy_cooperative_tensor %47, %48
    %50:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %45, 1u
    %51:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %46, 1u
    %52:void = msl.copy_cooperative_tensor %50, %51
    %53:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = access %acc1, 2u
    %54:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = access %11, 2u
    %55:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %53, 0u
    %56:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %54, 0u
    %57:void = msl.copy_cooperative_tensor %55, %56
    %58:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %53, 1u
    %59:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %54, 1u
    %60:void = msl.copy_cooperative_tensor %58, %59
    %61:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %62:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %63:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = construct %61, %62
    %64:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %65:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %66:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = construct %64, %65
    %67:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %68:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %69:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = construct %67, %68
    %acc2:array<array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2>, 3> = construct %63, %66, %69
    %71:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = access %acc2, 0u
    %72:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = access %11, 0u
    %73:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %71, 0u
    %74:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %72, 0u
    %75:void = msl.copy_cooperative_tensor %73, %74
    %76:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %71, 1u
    %77:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %72, 1u
    %78:void = msl.copy_cooperative_tensor %76, %77
    %79:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = access %acc2, 1u
    %80:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = access %11, 1u
    %81:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %79, 0u
    %82:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %80, 0u
    %83:void = msl.copy_cooperative_tensor %81, %82
    %84:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %79, 1u
    %85:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %80, 1u
    %86:void = msl.copy_cooperative_tensor %84, %85
    %87:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = access %acc2, 2u
    %88:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = access %11, 2u
    %89:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %87, 0u
    %90:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %88, 0u
    %91:void = msl.copy_cooperative_tensor %89, %90
    %92:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %87, 1u
    %93:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %88, 1u
    %94:void = msl.copy_cooperative_tensor %92, %93
    ret
  }
}
)";

    Run(CooperativeTensors);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_CooperativeTensorsTest, LetChains) {
    auto* ep = b.ComputeFunction("entry");
    b.Append(ep->Block(), [&] {
        auto* init = b.Construct(ty.subgroup_matrix_result(ty.f16(), 32, 16));
        auto* acc1 = b.Let("acc1", init);
        b.Let("acc2", acc1);
        auto* acc3 = b.Let("acc3", acc1);
        b.Let("acc4", acc1);
        b.Let("acc5", acc3);
        b.Let("acc6", acc3);
        b.Return(ep);
    });

    auto* src = R"(
%entry = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:subgroup_matrix_result<f16, 32, 16> = construct
    %acc1:subgroup_matrix_result<f16, 32, 16> = let %2
    %acc2:subgroup_matrix_result<f16, 32, 16> = let %acc1
    %acc3:subgroup_matrix_result<f16, 32, 16> = let %acc1
    %acc4:subgroup_matrix_result<f16, 32, 16> = let %acc1
    %acc5:subgroup_matrix_result<f16, 32, 16> = let %acc3
    %acc6:subgroup_matrix_result<f16, 32, 16> = let %acc3
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%entry = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %acc1:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %3:void = msl.fill_cooperative_tensor %acc1, 0.0h
    %acc2:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %5:void = msl.copy_cooperative_tensor %acc2, %acc1
    %acc3:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %7:void = msl.copy_cooperative_tensor %acc3, %acc1
    %acc4:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %9:void = msl.copy_cooperative_tensor %acc4, %acc1
    %acc5:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %11:void = msl.copy_cooperative_tensor %acc5, %acc3
    %acc6:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %13:void = msl.copy_cooperative_tensor %acc6, %acc3
    ret
  }
}
)";

    Run(CooperativeTensors);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_CooperativeTensorsTest, LetVar_SingleUseInit_AcrossBlocks) {
    auto* ep = b.ComputeFunction("entry");
    b.Append(ep->Block(), [&] {
        auto* init1 = b.Construct(ty.subgroup_matrix_result(ty.f16(), 32, 16));
        auto* init2 = b.Construct(ty.subgroup_matrix_result(ty.f16(), 32, 16));
        auto* acc1 = b.Let("acc1", init1);
        auto* acc2 = b.Let("acc2", init2);
        auto* loop = b.Loop();
        b.Append(loop->Body(), [&] {
            b.Let("inner_let", acc1);
            b.Var("inner_var", acc2);
            b.ExitLoop(loop);
        });
        b.Return(ep);
    });

    auto* src = R"(
%entry = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:subgroup_matrix_result<f16, 32, 16> = construct
    %3:subgroup_matrix_result<f16, 32, 16> = construct
    %acc1:subgroup_matrix_result<f16, 32, 16> = let %2
    %acc2:subgroup_matrix_result<f16, 32, 16> = let %3
    loop [b: $B2] {  # loop_1
      $B2: {  # body
        %inner_let:subgroup_matrix_result<f16, 32, 16> = let %acc1
        %inner_var:ptr<function, subgroup_matrix_result<f16, 32, 16>, read_write> = var %acc2
        exit_loop  # loop_1
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%entry = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %acc1:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %3:void = msl.fill_cooperative_tensor %acc1, 0.0h
    %acc2:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %5:void = msl.fill_cooperative_tensor %acc2, 0.0h
    loop [b: $B2] {  # loop_1
      $B2: {  # body
        %inner_let:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
        %7:void = msl.copy_cooperative_tensor %inner_let, %acc1
        %inner_var:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
        %9:void = msl.copy_cooperative_tensor %inner_var, %acc2
        exit_loop  # loop_1
      }
    }
    ret
  }
}
)";

    Run(CooperativeTensors);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_CooperativeTensorsTest, LetPointer) {
    auto* ep = b.ComputeFunction("entry");
    b.Append(ep->Block(), [&] {
        auto* matrix_ty = ty.subgroup_matrix_result(ty.f16(), 32, 16);
        auto* var = b.Var("acc", ty.ptr<function>(ty.array(matrix_ty, 2)));
        b.Let("whole", var);
        auto* el = b.Let("el", b.Access(ty.ptr<function>(matrix_ty), var, 1_u));
        b.Let("el2", el);
        b.Return(ep);
    });

    auto* src = R"(
%entry = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %acc:ptr<function, array<subgroup_matrix_result<f16, 32, 16>, 2>, read_write> = var undef
    %whole:ptr<function, array<subgroup_matrix_result<f16, 32, 16>, 2>, read_write> = let %acc
    %4:ptr<function, subgroup_matrix_result<f16, 32, 16>, read_write> = access %acc, 1u
    %el:ptr<function, subgroup_matrix_result<f16, 32, 16>, read_write> = let %4
    %el2:ptr<function, subgroup_matrix_result<f16, 32, 16>, read_write> = let %el
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%entry = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %3:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %4:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = construct %2, %3
    %acc:ptr<function, array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2>, read_write> = var %4
    %6:ptr<function, ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, read_write> = access %acc, 0u
    %7:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = load %6
    %8:void = msl.fill_cooperative_tensor %7, 0.0h
    %9:ptr<function, ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, read_write> = access %acc, 1u
    %10:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = load %9
    %11:void = msl.fill_cooperative_tensor %10, 0.0h
    %whole:ptr<function, array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2>, read_write> = let %acc
    %13:ptr<function, ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, read_write> = access %acc, 1u
    %14:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = load %13
    %el:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = let %14
    %el2:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = let %el
    ret
  }
}
)";

    Run(CooperativeTensors);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_CooperativeTensorsTest, Access_Array_Element) {
    auto* ep = b.ComputeFunction("entry");
    b.Append(ep->Block(), [&] {
        auto* let =
            b.Let("acc", b.Construct(ty.array(ty.subgroup_matrix_result(ty.f16(), 32, 16), 2)));
        auto* access = b.Access(ty.subgroup_matrix_result(ty.f16(), 32, 16), let, 1_u);
        b.Let("x", access);
        b.Return(ep);
    });

    auto* src = R"(
%entry = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:array<subgroup_matrix_result<f16, 32, 16>, 2> = construct
    %acc:array<subgroup_matrix_result<f16, 32, 16>, 2> = let %2
    %4:subgroup_matrix_result<f16, 32, 16> = access %acc, 1u
    %x:subgroup_matrix_result<f16, 32, 16> = let %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%entry = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %3:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %acc:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = construct %2, %3
    %5:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %acc, 0u
    %6:void = msl.fill_cooperative_tensor %5, 0.0h
    %7:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %acc, 1u
    %8:void = msl.fill_cooperative_tensor %7, 0.0h
    %9:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %acc, 1u
    %x:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %11:void = msl.copy_cooperative_tensor %x, %9
    ret
  }
}
)";

    Run(CooperativeTensors);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_CooperativeTensorsTest, Access_ArrayOfArray_Inner) {
    auto* ep = b.ComputeFunction("entry");
    b.Append(ep->Block(), [&] {
        auto* let = b.Let("acc", b.Construct(ty.array(
                                     ty.array(ty.subgroup_matrix_result(ty.f16(), 32, 16), 2), 3)));
        auto* access = b.Access(ty.array(ty.subgroup_matrix_result(ty.f16(), 32, 16), 2), let, 2_u);
        b.Let("x", access);
        b.Return(ep);
    });

    auto* src = R"(
%entry = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:array<array<subgroup_matrix_result<f16, 32, 16>, 2>, 3> = construct
    %acc:array<array<subgroup_matrix_result<f16, 32, 16>, 2>, 3> = let %2
    %4:array<subgroup_matrix_result<f16, 32, 16>, 2> = access %acc, 2u
    %x:array<subgroup_matrix_result<f16, 32, 16>, 2> = let %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%entry = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %3:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %4:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = construct %2, %3
    %5:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %6:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %7:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = construct %5, %6
    %8:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %9:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %10:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = construct %8, %9
    %acc:array<array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2>, 3> = construct %4, %7, %10
    %12:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = access %acc, 0u
    %13:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %12, 0u
    %14:void = msl.fill_cooperative_tensor %13, 0.0h
    %15:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %12, 1u
    %16:void = msl.fill_cooperative_tensor %15, 0.0h
    %17:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = access %acc, 1u
    %18:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %17, 0u
    %19:void = msl.fill_cooperative_tensor %18, 0.0h
    %20:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %17, 1u
    %21:void = msl.fill_cooperative_tensor %20, 0.0h
    %22:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = access %acc, 2u
    %23:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %22, 0u
    %24:void = msl.fill_cooperative_tensor %23, 0.0h
    %25:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %22, 1u
    %26:void = msl.fill_cooperative_tensor %25, 0.0h
    %27:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = access %acc, 2u
    %28:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %29:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %x:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = construct %28, %29
    %31:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %x, 0u
    %32:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %27, 0u
    %33:void = msl.copy_cooperative_tensor %31, %32
    %34:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %x, 1u
    %35:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %27, 1u
    %36:void = msl.copy_cooperative_tensor %34, %35
    ret
  }
}
)";

    Run(CooperativeTensors);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_CooperativeTensorsTest, Access_ArrayOfArray_Element) {
    auto* ep = b.ComputeFunction("entry");
    b.Append(ep->Block(), [&] {
        auto* let = b.Let("acc", b.Construct(ty.array(
                                     ty.array(ty.subgroup_matrix_result(ty.f16(), 32, 16), 2), 3)));
        auto* access = b.Access(ty.subgroup_matrix_result(ty.f16(), 32, 16), let, 2_u, 1_u);
        b.Let("x", access);
        b.Return(ep);
    });

    auto* src = R"(
%entry = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:array<array<subgroup_matrix_result<f16, 32, 16>, 2>, 3> = construct
    %acc:array<array<subgroup_matrix_result<f16, 32, 16>, 2>, 3> = let %2
    %4:subgroup_matrix_result<f16, 32, 16> = access %acc, 2u, 1u
    %x:subgroup_matrix_result<f16, 32, 16> = let %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%entry = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %3:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %4:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = construct %2, %3
    %5:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %6:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %7:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = construct %5, %6
    %8:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %9:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %10:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = construct %8, %9
    %acc:array<array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2>, 3> = construct %4, %7, %10
    %12:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = access %acc, 0u
    %13:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %12, 0u
    %14:void = msl.fill_cooperative_tensor %13, 0.0h
    %15:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %12, 1u
    %16:void = msl.fill_cooperative_tensor %15, 0.0h
    %17:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = access %acc, 1u
    %18:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %17, 0u
    %19:void = msl.fill_cooperative_tensor %18, 0.0h
    %20:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %17, 1u
    %21:void = msl.fill_cooperative_tensor %20, 0.0h
    %22:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = access %acc, 2u
    %23:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %22, 0u
    %24:void = msl.fill_cooperative_tensor %23, 0.0h
    %25:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %22, 1u
    %26:void = msl.fill_cooperative_tensor %25, 0.0h
    %27:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %acc, 2u, 1u
    %x:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %29:void = msl.copy_cooperative_tensor %x, %27
    ret
  }
}
)";

    Run(CooperativeTensors);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_CooperativeTensorsTest, Load) {
    auto* ep = b.ComputeFunction("entry");
    b.Append(ep->Block(), [&] {
        auto* v = b.Var("acc", ty.ptr(function, ty.subgroup_matrix_result(ty.f16(), 32, 16)));
        auto* load = b.Load(v);
        b.Let("x", load);
        b.Return(ep);
    });

    auto* src = R"(
%entry = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %acc:ptr<function, subgroup_matrix_result<f16, 32, 16>, read_write> = var undef
    %3:subgroup_matrix_result<f16, 32, 16> = load %acc
    %x:subgroup_matrix_result<f16, 32, 16> = let %3
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%entry = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %acc:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %3:void = msl.fill_cooperative_tensor %acc, 0.0h
    %x:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %5:void = msl.copy_cooperative_tensor %x, %acc
    ret
  }
}
)";

    Run(CooperativeTensors);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_CooperativeTensorsTest, Load_Array_Whole) {
    auto* ep = b.ComputeFunction("entry");
    b.Append(ep->Block(), [&] {
        auto* v = b.Var("acc",
                        ty.ptr(function, ty.array(ty.subgroup_matrix_result(ty.f16(), 32, 16), 2)));
        auto* load = b.Load(v);
        b.Let("x", load);
        b.Return(ep);
    });

    auto* src = R"(
%entry = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %acc:ptr<function, array<subgroup_matrix_result<f16, 32, 16>, 2>, read_write> = var undef
    %3:array<subgroup_matrix_result<f16, 32, 16>, 2> = load %acc
    %x:array<subgroup_matrix_result<f16, 32, 16>, 2> = let %3
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%entry = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %3:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %4:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = construct %2, %3
    %acc:ptr<function, array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2>, read_write> = var %4
    %6:ptr<function, ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, read_write> = access %acc, 0u
    %7:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = load %6
    %8:void = msl.fill_cooperative_tensor %7, 0.0h
    %9:ptr<function, ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, read_write> = access %acc, 1u
    %10:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = load %9
    %11:void = msl.fill_cooperative_tensor %10, 0.0h
    %12:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %13:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %x:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = construct %12, %13
    %15:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %x, 0u
    %16:ptr<function, ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, read_write> = access %acc, 0u
    %17:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = load %16
    %18:void = msl.copy_cooperative_tensor %15, %17
    %19:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %x, 1u
    %20:ptr<function, ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, read_write> = access %acc, 1u
    %21:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = load %20
    %22:void = msl.copy_cooperative_tensor %19, %21
    ret
  }
}
)";

    Run(CooperativeTensors);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_CooperativeTensorsTest, Load_Array_Element) {
    auto* ep = b.ComputeFunction("entry");
    b.Append(ep->Block(), [&] {
        auto* v = b.Var("acc",
                        ty.ptr(function, ty.array(ty.subgroup_matrix_result(ty.f16(), 32, 16), 2)));
        auto* load =
            b.Load(b.Access(ty.ptr(function, ty.subgroup_matrix_result(ty.f16(), 32, 16)), v, 1_u));
        b.Let("x", load);
        b.Return(ep);
    });

    auto* src = R"(
%entry = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %acc:ptr<function, array<subgroup_matrix_result<f16, 32, 16>, 2>, read_write> = var undef
    %3:ptr<function, subgroup_matrix_result<f16, 32, 16>, read_write> = access %acc, 1u
    %4:subgroup_matrix_result<f16, 32, 16> = load %3
    %x:subgroup_matrix_result<f16, 32, 16> = let %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%entry = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %3:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %4:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = construct %2, %3
    %acc:ptr<function, array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2>, read_write> = var %4
    %6:ptr<function, ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, read_write> = access %acc, 0u
    %7:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = load %6
    %8:void = msl.fill_cooperative_tensor %7, 0.0h
    %9:ptr<function, ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, read_write> = access %acc, 1u
    %10:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = load %9
    %11:void = msl.fill_cooperative_tensor %10, 0.0h
    %12:ptr<function, ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, read_write> = access %acc, 1u
    %13:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = load %12
    %x:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %15:void = msl.copy_cooperative_tensor %x, %13
    ret
  }
}
)";

    Run(CooperativeTensors);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_CooperativeTensorsTest, Load_ArrayOfArray_Whole) {
    auto* ep = b.ComputeFunction("entry");
    b.Append(ep->Block(), [&] {
        auto* v = b.Var(
            "acc", ty.ptr(function,
                          ty.array(ty.array(ty.subgroup_matrix_result(ty.f16(), 32, 16), 2), 3)));
        auto* load = b.Load(v);
        b.Let("x", load);
        b.Return(ep);
    });

    auto* src = R"(
%entry = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %acc:ptr<function, array<array<subgroup_matrix_result<f16, 32, 16>, 2>, 3>, read_write> = var undef
    %3:array<array<subgroup_matrix_result<f16, 32, 16>, 2>, 3> = load %acc
    %x:array<array<subgroup_matrix_result<f16, 32, 16>, 2>, 3> = let %3
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%entry = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %3:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %4:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = construct %2, %3
    %5:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %6:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %7:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = construct %5, %6
    %8:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %9:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %10:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = construct %8, %9
    %11:array<array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2>, 3> = construct %4, %7, %10
    %acc:ptr<function, array<array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2>, 3>, read_write> = var %11
    %13:ptr<function, array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2>, read_write> = access %acc, 0u
    %14:ptr<function, ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, read_write> = access %13, 0u
    %15:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = load %14
    %16:void = msl.fill_cooperative_tensor %15, 0.0h
    %17:ptr<function, ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, read_write> = access %13, 1u
    %18:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = load %17
    %19:void = msl.fill_cooperative_tensor %18, 0.0h
    %20:ptr<function, array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2>, read_write> = access %acc, 1u
    %21:ptr<function, ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, read_write> = access %20, 0u
    %22:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = load %21
    %23:void = msl.fill_cooperative_tensor %22, 0.0h
    %24:ptr<function, ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, read_write> = access %20, 1u
    %25:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = load %24
    %26:void = msl.fill_cooperative_tensor %25, 0.0h
    %27:ptr<function, array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2>, read_write> = access %acc, 2u
    %28:ptr<function, ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, read_write> = access %27, 0u
    %29:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = load %28
    %30:void = msl.fill_cooperative_tensor %29, 0.0h
    %31:ptr<function, ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, read_write> = access %27, 1u
    %32:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = load %31
    %33:void = msl.fill_cooperative_tensor %32, 0.0h
    %34:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %35:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %36:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = construct %34, %35
    %37:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %38:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %39:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = construct %37, %38
    %40:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %41:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %42:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = construct %40, %41
    %x:array<array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2>, 3> = construct %36, %39, %42
    %44:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = access %x, 0u
    %45:ptr<function, array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2>, read_write> = access %acc, 0u
    %46:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %44, 0u
    %47:ptr<function, ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, read_write> = access %45, 0u
    %48:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = load %47
    %49:void = msl.copy_cooperative_tensor %46, %48
    %50:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %44, 1u
    %51:ptr<function, ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, read_write> = access %45, 1u
    %52:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = load %51
    %53:void = msl.copy_cooperative_tensor %50, %52
    %54:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = access %x, 1u
    %55:ptr<function, array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2>, read_write> = access %acc, 1u
    %56:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %54, 0u
    %57:ptr<function, ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, read_write> = access %55, 0u
    %58:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = load %57
    %59:void = msl.copy_cooperative_tensor %56, %58
    %60:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %54, 1u
    %61:ptr<function, ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, read_write> = access %55, 1u
    %62:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = load %61
    %63:void = msl.copy_cooperative_tensor %60, %62
    %64:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = access %x, 2u
    %65:ptr<function, array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2>, read_write> = access %acc, 2u
    %66:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %64, 0u
    %67:ptr<function, ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, read_write> = access %65, 0u
    %68:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = load %67
    %69:void = msl.copy_cooperative_tensor %66, %68
    %70:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %64, 1u
    %71:ptr<function, ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, read_write> = access %65, 1u
    %72:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = load %71
    %73:void = msl.copy_cooperative_tensor %70, %72
    ret
  }
}
)";

    Run(CooperativeTensors);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_CooperativeTensorsTest, Load_ArrayOfArray_Inner) {
    auto* ep = b.ComputeFunction("entry");
    b.Append(ep->Block(), [&] {
        auto* v = b.Var(
            "acc", ty.ptr(function,
                          ty.array(ty.array(ty.subgroup_matrix_result(ty.f16(), 32, 16), 2), 3)));
        auto* load = b.Load(b.Access(
            ty.ptr(function, ty.array(ty.subgroup_matrix_result(ty.f16(), 32, 16), 2)), v, 2_u));
        b.Let("x", load);
        b.Return(ep);
    });

    auto* src = R"(
%entry = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %acc:ptr<function, array<array<subgroup_matrix_result<f16, 32, 16>, 2>, 3>, read_write> = var undef
    %3:ptr<function, array<subgroup_matrix_result<f16, 32, 16>, 2>, read_write> = access %acc, 2u
    %4:array<subgroup_matrix_result<f16, 32, 16>, 2> = load %3
    %x:array<subgroup_matrix_result<f16, 32, 16>, 2> = let %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%entry = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %3:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %4:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = construct %2, %3
    %5:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %6:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %7:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = construct %5, %6
    %8:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %9:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %10:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = construct %8, %9
    %11:array<array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2>, 3> = construct %4, %7, %10
    %acc:ptr<function, array<array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2>, 3>, read_write> = var %11
    %13:ptr<function, array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2>, read_write> = access %acc, 0u
    %14:ptr<function, ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, read_write> = access %13, 0u
    %15:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = load %14
    %16:void = msl.fill_cooperative_tensor %15, 0.0h
    %17:ptr<function, ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, read_write> = access %13, 1u
    %18:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = load %17
    %19:void = msl.fill_cooperative_tensor %18, 0.0h
    %20:ptr<function, array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2>, read_write> = access %acc, 1u
    %21:ptr<function, ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, read_write> = access %20, 0u
    %22:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = load %21
    %23:void = msl.fill_cooperative_tensor %22, 0.0h
    %24:ptr<function, ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, read_write> = access %20, 1u
    %25:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = load %24
    %26:void = msl.fill_cooperative_tensor %25, 0.0h
    %27:ptr<function, array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2>, read_write> = access %acc, 2u
    %28:ptr<function, ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, read_write> = access %27, 0u
    %29:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = load %28
    %30:void = msl.fill_cooperative_tensor %29, 0.0h
    %31:ptr<function, ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, read_write> = access %27, 1u
    %32:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = load %31
    %33:void = msl.fill_cooperative_tensor %32, 0.0h
    %34:ptr<function, array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2>, read_write> = access %acc, 2u
    %35:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %36:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %x:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = construct %35, %36
    %38:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %x, 0u
    %39:ptr<function, ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, read_write> = access %34, 0u
    %40:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = load %39
    %41:void = msl.copy_cooperative_tensor %38, %40
    %42:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %x, 1u
    %43:ptr<function, ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, read_write> = access %34, 1u
    %44:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = load %43
    %45:void = msl.copy_cooperative_tensor %42, %44
    ret
  }
}
)";

    Run(CooperativeTensors);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_CooperativeTensorsTest, Load_ArrayOfArray_Element) {
    auto* ep = b.ComputeFunction("entry");
    b.Append(ep->Block(), [&] {
        auto* v = b.Var(
            "acc", ty.ptr(function,
                          ty.array(ty.array(ty.subgroup_matrix_result(ty.f16(), 32, 16), 2), 3)));
        auto* load = b.Load(
            b.Access(ty.ptr(function, ty.subgroup_matrix_result(ty.f16(), 32, 16)), v, 2_u, 1_u));
        b.Let("x", load);
        b.Return(ep);
    });

    auto* src = R"(
%entry = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %acc:ptr<function, array<array<subgroup_matrix_result<f16, 32, 16>, 2>, 3>, read_write> = var undef
    %3:ptr<function, subgroup_matrix_result<f16, 32, 16>, read_write> = access %acc, 2u, 1u
    %4:subgroup_matrix_result<f16, 32, 16> = load %3
    %x:subgroup_matrix_result<f16, 32, 16> = let %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%entry = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %3:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %4:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = construct %2, %3
    %5:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %6:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %7:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = construct %5, %6
    %8:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %9:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %10:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = construct %8, %9
    %11:array<array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2>, 3> = construct %4, %7, %10
    %acc:ptr<function, array<array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2>, 3>, read_write> = var %11
    %13:ptr<function, array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2>, read_write> = access %acc, 0u
    %14:ptr<function, ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, read_write> = access %13, 0u
    %15:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = load %14
    %16:void = msl.fill_cooperative_tensor %15, 0.0h
    %17:ptr<function, ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, read_write> = access %13, 1u
    %18:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = load %17
    %19:void = msl.fill_cooperative_tensor %18, 0.0h
    %20:ptr<function, array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2>, read_write> = access %acc, 1u
    %21:ptr<function, ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, read_write> = access %20, 0u
    %22:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = load %21
    %23:void = msl.fill_cooperative_tensor %22, 0.0h
    %24:ptr<function, ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, read_write> = access %20, 1u
    %25:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = load %24
    %26:void = msl.fill_cooperative_tensor %25, 0.0h
    %27:ptr<function, array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2>, read_write> = access %acc, 2u
    %28:ptr<function, ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, read_write> = access %27, 0u
    %29:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = load %28
    %30:void = msl.fill_cooperative_tensor %29, 0.0h
    %31:ptr<function, ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, read_write> = access %27, 1u
    %32:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = load %31
    %33:void = msl.fill_cooperative_tensor %32, 0.0h
    %34:ptr<function, ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, read_write> = access %acc, 2u, 1u
    %35:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = load %34
    %x:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %37:void = msl.copy_cooperative_tensor %x, %35
    ret
  }
}
)";

    Run(CooperativeTensors);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_CooperativeTensorsTest, LoadWithMultipleUses) {
    auto* ep = b.ComputeFunction("entry");
    b.Append(ep->Block(), [&] {
        auto* v = b.Var("acc", ty.ptr(function, ty.subgroup_matrix_result(ty.f16(), 32, 16)));
        auto* load = b.Load(v);
        b.Let("x", load);
        b.Let("y", load);
        b.Return(ep);
    });

    auto* src = R"(
%entry = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %acc:ptr<function, subgroup_matrix_result<f16, 32, 16>, read_write> = var undef
    %3:subgroup_matrix_result<f16, 32, 16> = load %acc
    %x:subgroup_matrix_result<f16, 32, 16> = let %3
    %y:subgroup_matrix_result<f16, 32, 16> = let %3
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%entry = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %acc:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %3:void = msl.fill_cooperative_tensor %acc, 0.0h
    %4:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %5:void = msl.copy_cooperative_tensor %4, %acc
    %x:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %7:void = msl.copy_cooperative_tensor %x, %4
    %y:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %9:void = msl.copy_cooperative_tensor %y, %4
    ret
  }
}
)";

    Run(CooperativeTensors);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_CooperativeTensorsTest, Store) {
    auto* ep = b.ComputeFunction("entry");
    b.Append(ep->Block(), [&] {
        auto* v = b.Var("acc", ty.ptr(function, ty.subgroup_matrix_result(ty.f16(), 32, 16)));
        auto* val = b.Construct(ty.subgroup_matrix_result(ty.f16(), 32, 16), 1.0_h);
        b.Store(v, val);
        b.Return(ep);
    });

    auto* src = R"(
%entry = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %acc:ptr<function, subgroup_matrix_result<f16, 32, 16>, read_write> = var undef
    %3:subgroup_matrix_result<f16, 32, 16> = construct 1.0h
    store %acc, %3
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%entry = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %acc:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %3:void = msl.fill_cooperative_tensor %acc, 0.0h
    %4:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %5:void = msl.fill_cooperative_tensor %4, 1.0h
    %6:void = msl.copy_cooperative_tensor %acc, %4
    ret
  }
}
)";

    Run(CooperativeTensors);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_CooperativeTensorsTest, Store_Array_Whole) {
    auto* ep = b.ComputeFunction("entry");
    b.Append(ep->Block(), [&] {
        auto* v = b.Var("acc",
                        ty.ptr(function, ty.array(ty.subgroup_matrix_result(ty.f16(), 32, 16), 2)));
        auto* val = b.Construct(ty.array(ty.subgroup_matrix_result(ty.f16(), 32, 16), 2));
        b.Store(v, val);
        b.Return(ep);
    });

    auto* src = R"(
%entry = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %acc:ptr<function, array<subgroup_matrix_result<f16, 32, 16>, 2>, read_write> = var undef
    %3:array<subgroup_matrix_result<f16, 32, 16>, 2> = construct
    store %acc, %3
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%entry = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %3:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %4:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = construct %2, %3
    %acc:ptr<function, array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2>, read_write> = var %4
    %6:ptr<function, ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, read_write> = access %acc, 0u
    %7:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = load %6
    %8:void = msl.fill_cooperative_tensor %7, 0.0h
    %9:ptr<function, ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, read_write> = access %acc, 1u
    %10:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = load %9
    %11:void = msl.fill_cooperative_tensor %10, 0.0h
    %12:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %13:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %14:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = construct %12, %13
    %15:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %14, 0u
    %16:void = msl.fill_cooperative_tensor %15, 0.0h
    %17:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %14, 1u
    %18:void = msl.fill_cooperative_tensor %17, 0.0h
    %19:ptr<function, ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, read_write> = access %acc, 0u
    %20:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %14, 0u
    %21:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = load %19
    %22:void = msl.copy_cooperative_tensor %21, %20
    %23:ptr<function, ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, read_write> = access %acc, 1u
    %24:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %14, 1u
    %25:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = load %23
    %26:void = msl.copy_cooperative_tensor %25, %24
    ret
  }
}
)";

    Run(CooperativeTensors);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_CooperativeTensorsTest, Store_Array_Element) {
    auto* ep = b.ComputeFunction("entry");
    b.Append(ep->Block(), [&] {
        auto* v = b.Var("acc",
                        ty.ptr(function, ty.array(ty.subgroup_matrix_result(ty.f16(), 32, 16), 2)));
        auto* val = b.Construct(ty.subgroup_matrix_result(ty.f16(), 32, 16), 1.0_h);
        b.Store(b.Access(ty.ptr(function, ty.subgroup_matrix_result(ty.f16(), 32, 16)), v, 1_u),
                val);
        b.Return(ep);
    });

    auto* src = R"(
%entry = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %acc:ptr<function, array<subgroup_matrix_result<f16, 32, 16>, 2>, read_write> = var undef
    %3:subgroup_matrix_result<f16, 32, 16> = construct 1.0h
    %4:ptr<function, subgroup_matrix_result<f16, 32, 16>, read_write> = access %acc, 1u
    store %4, %3
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%entry = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %3:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %4:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = construct %2, %3
    %acc:ptr<function, array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2>, read_write> = var %4
    %6:ptr<function, ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, read_write> = access %acc, 0u
    %7:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = load %6
    %8:void = msl.fill_cooperative_tensor %7, 0.0h
    %9:ptr<function, ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, read_write> = access %acc, 1u
    %10:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = load %9
    %11:void = msl.fill_cooperative_tensor %10, 0.0h
    %12:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %13:void = msl.fill_cooperative_tensor %12, 1.0h
    %14:ptr<function, ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, read_write> = access %acc, 1u
    %15:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = load %14
    %16:void = msl.copy_cooperative_tensor %15, %12
    ret
  }
}
)";

    Run(CooperativeTensors);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_CooperativeTensorsTest, Store_ArrayOfArray_Whole) {
    auto* ep = b.ComputeFunction("entry");
    b.Append(ep->Block(), [&] {
        auto* v = b.Var(
            "acc", ty.ptr(function,
                          ty.array(ty.array(ty.subgroup_matrix_result(ty.f16(), 32, 16), 2), 3)));
        auto* val =
            b.Construct(ty.array(ty.array(ty.subgroup_matrix_result(ty.f16(), 32, 16), 2), 3));
        b.Store(v, val);
        b.Return(ep);
    });

    auto* src = R"(
%entry = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %acc:ptr<function, array<array<subgroup_matrix_result<f16, 32, 16>, 2>, 3>, read_write> = var undef
    %3:array<array<subgroup_matrix_result<f16, 32, 16>, 2>, 3> = construct
    store %acc, %3
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%entry = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %3:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %4:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = construct %2, %3
    %5:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %6:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %7:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = construct %5, %6
    %8:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %9:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %10:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = construct %8, %9
    %11:array<array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2>, 3> = construct %4, %7, %10
    %acc:ptr<function, array<array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2>, 3>, read_write> = var %11
    %13:ptr<function, array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2>, read_write> = access %acc, 0u
    %14:ptr<function, ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, read_write> = access %13, 0u
    %15:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = load %14
    %16:void = msl.fill_cooperative_tensor %15, 0.0h
    %17:ptr<function, ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, read_write> = access %13, 1u
    %18:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = load %17
    %19:void = msl.fill_cooperative_tensor %18, 0.0h
    %20:ptr<function, array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2>, read_write> = access %acc, 1u
    %21:ptr<function, ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, read_write> = access %20, 0u
    %22:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = load %21
    %23:void = msl.fill_cooperative_tensor %22, 0.0h
    %24:ptr<function, ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, read_write> = access %20, 1u
    %25:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = load %24
    %26:void = msl.fill_cooperative_tensor %25, 0.0h
    %27:ptr<function, array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2>, read_write> = access %acc, 2u
    %28:ptr<function, ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, read_write> = access %27, 0u
    %29:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = load %28
    %30:void = msl.fill_cooperative_tensor %29, 0.0h
    %31:ptr<function, ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, read_write> = access %27, 1u
    %32:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = load %31
    %33:void = msl.fill_cooperative_tensor %32, 0.0h
    %34:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %35:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %36:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = construct %34, %35
    %37:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %38:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %39:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = construct %37, %38
    %40:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %41:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %42:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = construct %40, %41
    %43:array<array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2>, 3> = construct %36, %39, %42
    %44:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = access %43, 0u
    %45:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %44, 0u
    %46:void = msl.fill_cooperative_tensor %45, 0.0h
    %47:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %44, 1u
    %48:void = msl.fill_cooperative_tensor %47, 0.0h
    %49:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = access %43, 1u
    %50:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %49, 0u
    %51:void = msl.fill_cooperative_tensor %50, 0.0h
    %52:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %49, 1u
    %53:void = msl.fill_cooperative_tensor %52, 0.0h
    %54:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = access %43, 2u
    %55:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %54, 0u
    %56:void = msl.fill_cooperative_tensor %55, 0.0h
    %57:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %54, 1u
    %58:void = msl.fill_cooperative_tensor %57, 0.0h
    %59:ptr<function, array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2>, read_write> = access %acc, 0u
    %60:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = access %43, 0u
    %61:ptr<function, ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, read_write> = access %59, 0u
    %62:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %60, 0u
    %63:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = load %61
    %64:void = msl.copy_cooperative_tensor %63, %62
    %65:ptr<function, ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, read_write> = access %59, 1u
    %66:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %60, 1u
    %67:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = load %65
    %68:void = msl.copy_cooperative_tensor %67, %66
    %69:ptr<function, array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2>, read_write> = access %acc, 1u
    %70:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = access %43, 1u
    %71:ptr<function, ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, read_write> = access %69, 0u
    %72:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %70, 0u
    %73:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = load %71
    %74:void = msl.copy_cooperative_tensor %73, %72
    %75:ptr<function, ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, read_write> = access %69, 1u
    %76:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %70, 1u
    %77:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = load %75
    %78:void = msl.copy_cooperative_tensor %77, %76
    %79:ptr<function, array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2>, read_write> = access %acc, 2u
    %80:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = access %43, 2u
    %81:ptr<function, ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, read_write> = access %79, 0u
    %82:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %80, 0u
    %83:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = load %81
    %84:void = msl.copy_cooperative_tensor %83, %82
    %85:ptr<function, ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, read_write> = access %79, 1u
    %86:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %80, 1u
    %87:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = load %85
    %88:void = msl.copy_cooperative_tensor %87, %86
    ret
  }
}
)";

    Run(CooperativeTensors);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_CooperativeTensorsTest, Store_ArrayOfArray_Inner) {
    auto* ep = b.ComputeFunction("entry");
    b.Append(ep->Block(), [&] {
        auto* v = b.Var(
            "acc", ty.ptr(function,
                          ty.array(ty.array(ty.subgroup_matrix_result(ty.f16(), 32, 16), 2), 3)));
        auto* val = b.Construct(ty.array(ty.subgroup_matrix_result(ty.f16(), 32, 16), 2));
        b.Store(b.Access(ty.ptr(function, ty.array(ty.subgroup_matrix_result(ty.f16(), 32, 16), 2)),
                         v, 2_u),
                val);
        b.Return(ep);
    });

    auto* src = R"(
%entry = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %acc:ptr<function, array<array<subgroup_matrix_result<f16, 32, 16>, 2>, 3>, read_write> = var undef
    %3:array<subgroup_matrix_result<f16, 32, 16>, 2> = construct
    %4:ptr<function, array<subgroup_matrix_result<f16, 32, 16>, 2>, read_write> = access %acc, 2u
    store %4, %3
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%entry = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %3:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %4:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = construct %2, %3
    %5:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %6:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %7:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = construct %5, %6
    %8:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %9:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %10:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = construct %8, %9
    %11:array<array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2>, 3> = construct %4, %7, %10
    %acc:ptr<function, array<array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2>, 3>, read_write> = var %11
    %13:ptr<function, array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2>, read_write> = access %acc, 0u
    %14:ptr<function, ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, read_write> = access %13, 0u
    %15:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = load %14
    %16:void = msl.fill_cooperative_tensor %15, 0.0h
    %17:ptr<function, ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, read_write> = access %13, 1u
    %18:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = load %17
    %19:void = msl.fill_cooperative_tensor %18, 0.0h
    %20:ptr<function, array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2>, read_write> = access %acc, 1u
    %21:ptr<function, ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, read_write> = access %20, 0u
    %22:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = load %21
    %23:void = msl.fill_cooperative_tensor %22, 0.0h
    %24:ptr<function, ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, read_write> = access %20, 1u
    %25:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = load %24
    %26:void = msl.fill_cooperative_tensor %25, 0.0h
    %27:ptr<function, array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2>, read_write> = access %acc, 2u
    %28:ptr<function, ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, read_write> = access %27, 0u
    %29:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = load %28
    %30:void = msl.fill_cooperative_tensor %29, 0.0h
    %31:ptr<function, ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, read_write> = access %27, 1u
    %32:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = load %31
    %33:void = msl.fill_cooperative_tensor %32, 0.0h
    %34:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %35:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %36:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = construct %34, %35
    %37:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %36, 0u
    %38:void = msl.fill_cooperative_tensor %37, 0.0h
    %39:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %36, 1u
    %40:void = msl.fill_cooperative_tensor %39, 0.0h
    %41:ptr<function, array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2>, read_write> = access %acc, 2u
    %42:ptr<function, ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, read_write> = access %41, 0u
    %43:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %36, 0u
    %44:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = load %42
    %45:void = msl.copy_cooperative_tensor %44, %43
    %46:ptr<function, ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, read_write> = access %41, 1u
    %47:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = access %36, 1u
    %48:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = load %46
    %49:void = msl.copy_cooperative_tensor %48, %47
    ret
  }
}
)";

    Run(CooperativeTensors);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_CooperativeTensorsTest, Store_ArrayOfArray_Element) {
    auto* ep = b.ComputeFunction("entry");
    b.Append(ep->Block(), [&] {
        auto* v = b.Var(
            "acc", ty.ptr(function,
                          ty.array(ty.array(ty.subgroup_matrix_result(ty.f16(), 32, 16), 2), 3)));
        auto* val = b.Construct(ty.subgroup_matrix_result(ty.f16(), 32, 16), 1.0_h);
        b.Store(
            b.Access(ty.ptr(function, ty.subgroup_matrix_result(ty.f16(), 32, 16)), v, 2_u, 1_u),
            val);
        b.Return(ep);
    });

    auto* src = R"(
%entry = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %acc:ptr<function, array<array<subgroup_matrix_result<f16, 32, 16>, 2>, 3>, read_write> = var undef
    %3:subgroup_matrix_result<f16, 32, 16> = construct 1.0h
    %4:ptr<function, subgroup_matrix_result<f16, 32, 16>, read_write> = access %acc, 2u, 1u
    store %4, %3
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%entry = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %3:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %4:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = construct %2, %3
    %5:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %6:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %7:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = construct %5, %6
    %8:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %9:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %10:array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2> = construct %8, %9
    %11:array<array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2>, 3> = construct %4, %7, %10
    %acc:ptr<function, array<array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2>, 3>, read_write> = var %11
    %13:ptr<function, array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2>, read_write> = access %acc, 0u
    %14:ptr<function, ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, read_write> = access %13, 0u
    %15:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = load %14
    %16:void = msl.fill_cooperative_tensor %15, 0.0h
    %17:ptr<function, ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, read_write> = access %13, 1u
    %18:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = load %17
    %19:void = msl.fill_cooperative_tensor %18, 0.0h
    %20:ptr<function, array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2>, read_write> = access %acc, 1u
    %21:ptr<function, ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, read_write> = access %20, 0u
    %22:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = load %21
    %23:void = msl.fill_cooperative_tensor %22, 0.0h
    %24:ptr<function, ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, read_write> = access %20, 1u
    %25:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = load %24
    %26:void = msl.fill_cooperative_tensor %25, 0.0h
    %27:ptr<function, array<ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, 2>, read_write> = access %acc, 2u
    %28:ptr<function, ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, read_write> = access %27, 0u
    %29:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = load %28
    %30:void = msl.fill_cooperative_tensor %29, 0.0h
    %31:ptr<function, ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, read_write> = access %27, 1u
    %32:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = load %31
    %33:void = msl.fill_cooperative_tensor %32, 0.0h
    %34:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = var undef
    %35:void = msl.fill_cooperative_tensor %34, 1.0h
    %36:ptr<function, ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write>, read_write> = access %acc, 2u, 1u
    %37:ptr<function, msl.cooperative_tensor_result<16, 32, 32, f16, f16>, read_write> = load %36
    %38:void = msl.copy_cooperative_tensor %37, %34
    ret
  }
}
)";

    Run(CooperativeTensors);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_CooperativeTensorsTest, SubgroupMatrixStore_RowMajor) {
    auto* buffer = b.Var("buffer", ty.ptr<storage, array<f32>, read_write>());
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* ep = b.ComputeFunction("entry");
    b.Append(ep->Block(), [&] {
        auto* mat = b.Construct(ty.subgroup_matrix_left(ty.f32(), 8, 8));
        b.CallExplicit(ty.void_(), core::BuiltinFn::kSubgroupMatrixStore,
                       Vector<core::ir::TemplateParameter, 1>{core::Majorness::kRowMajor}, buffer,
                       0_u, mat, 64_u);
        b.Return(ep);
    });

    auto* src = R"(
$B1: {  # root
  %buffer:ptr<storage, array<f32>, read_write> = var undef @binding_point(0, 0)
}

%entry = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:subgroup_matrix_left<f32, 8, 8> = construct
    %4:void = subgroupMatrixStore<row_major> %buffer, 0u, %3, 64u
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %buffer:ptr<storage, array<f32>, read_write> = var undef @binding_point(0, 0)
}

%entry = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<function, msl.cooperative_tensor_left<8, 32, 8, f32, f32>, read_write> = var undef
    %4:void = msl.fill_cooperative_tensor %3, 0.0f
    %5:ptr<storage, f32, read_write> = access %buffer, 0u
    %6:msl.tensor_inline = msl.make_tensor_inline %5, vec2<u32>(8u), 64u
    %tint_dst_tensor:msl.tensor_inline = let %6
    %8:msl.cooperative_tensor_left<8, 32, 8, f32, f32> = load %3
    %9:void = %8.store %tint_dst_tensor
    ret
  }
}
)";

    Run(CooperativeTensors);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_CooperativeTensorsTest, DISABLED_SubgroupMatrixStore_ColMajor) {
    auto* buffer = b.Var("buffer", ty.ptr<storage, array<f32>, read_write>());
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* ep = b.ComputeFunction("entry");
    b.Append(ep->Block(), [&] {
        auto* mat = b.Construct(ty.subgroup_matrix_left(ty.f32(), 8, 8));
        b.CallExplicit(ty.void_(), core::BuiltinFn::kSubgroupMatrixStore,
                       Vector<core::ir::TemplateParameter, 1>{core::Majorness::kColMajor}, buffer,
                       0_u, mat, 64_u);
        b.Return(ep);
    });

    auto* src = R"(
$B1: {  # root
  %buffer:ptr<storage, array<f32>, read_write> = var undef @binding_point(0, 0)
}

%entry = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:subgroup_matrix_left<f32, 8, 8> = construct
    %4:void = subgroupMatrixStore<col_major> %buffer, 0u, %3, 64u
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
      // TODO(556210460): implement polyfill for column-major layout.
)";

    Run(CooperativeTensors);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_CooperativeTensorsTest, SubgroupMatrixStore_ElementTypeMismatch) {
    auto* buffer = b.Var("buffer", ty.ptr<storage, array<vec4<f32>>, read_write>());
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* ep = b.ComputeFunction("entry");
    b.Append(ep->Block(), [&] {
        auto* mat = b.Construct(ty.subgroup_matrix_result(ty.f32(), 16, 16));
        b.CallExplicit(ty.void_(), core::BuiltinFn::kSubgroupMatrixStore,
                       Vector<core::ir::TemplateParameter, 1>{core::Majorness::kRowMajor}, buffer,
                       2_u, mat, 64_u);
        b.Return(ep);
    });

    auto* src = R"(
$B1: {  # root
  %buffer:ptr<storage, array<vec4<f32>>, read_write> = var undef @binding_point(0, 0)
}

%entry = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:subgroup_matrix_result<f32, 16, 16> = construct
    %4:void = subgroupMatrixStore<row_major> %buffer, 2u, %3, 64u
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %buffer:ptr<storage, array<vec4<f32>>, read_write> = var undef @binding_point(0, 0)
}

%entry = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<function, msl.cooperative_tensor_result<16, 16, 32, f32, f32>, read_write> = var undef
    %4:void = msl.fill_cooperative_tensor %3, 0.0f
    %5:ptr<storage, f32, read_write> = msl.pointer_offset<f32> %buffer, 32u
    %6:msl.tensor_inline = msl.make_tensor_inline %5, vec2<u32>(16u), 256u
    %tint_dst_tensor:msl.tensor_inline = let %6
    %8:msl.cooperative_tensor_result<16, 16, 32, f32, f32> = load %3
    %9:void = %8.store %tint_dst_tensor
    ret
  }
}
)";

    Run(CooperativeTensors);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_CooperativeTensorsTest, SubgroupMatrixStore_RedundantPointerOffset) {
    auto* buffer = b.Var("buffer", ty.ptr<storage, array<vec4<f32>>, read_write>());
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* ep = b.ComputeFunction("entry");
    b.Append(ep->Block(), [&] {
        auto* pre_cast = b.CallExplicit<msl::ir::BuiltinCall>(
            ty.ptr<storage, array<vec4<f32>>, read_write>(), msl::BuiltinFn::kPointerOffset,
            Vector<core::ir::TemplateParameter, 1>{ty.array<vec4<f32>>()}, buffer, 0_u);
        auto* mat = b.Construct(ty.subgroup_matrix_result(ty.f32(), 16, 16));
        b.CallExplicit(ty.void_(), core::BuiltinFn::kSubgroupMatrixStore,
                       Vector<core::ir::TemplateParameter, 1>{core::Majorness::kRowMajor}, pre_cast,
                       2_u, mat, 64_u);
        b.Return(ep);
    });

    auto* src = R"(
$B1: {  # root
  %buffer:ptr<storage, array<vec4<f32>>, read_write> = var undef @binding_point(0, 0)
}

%entry = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<storage, array<vec4<f32>>, read_write> = msl.pointer_offset<array<vec4<f32>>> %buffer, 0u
    %4:subgroup_matrix_result<f32, 16, 16> = construct
    %5:void = subgroupMatrixStore<row_major> %3, 2u, %4, 64u
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %buffer:ptr<storage, array<vec4<f32>>, read_write> = var undef @binding_point(0, 0)
}

%entry = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<function, msl.cooperative_tensor_result<16, 16, 32, f32, f32>, read_write> = var undef
    %4:void = msl.fill_cooperative_tensor %3, 0.0f
    %5:ptr<storage, f32, read_write> = msl.pointer_offset<f32> %buffer, 32u
    %6:msl.tensor_inline = msl.make_tensor_inline %5, vec2<u32>(16u), 256u
    %tint_dst_tensor:msl.tensor_inline = let %6
    %8:msl.cooperative_tensor_result<16, 16, 32, f32, f32> = load %3
    %9:void = %8.store %tint_dst_tensor
    ret
  }
}
)";

    Run(CooperativeTensors);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_CooperativeTensorsTest, SubgroupMatrixStore_SignedOffsetAndStride) {
    auto* buffer = b.Var("buffer", ty.ptr<storage, array<f32>, read_write>());
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* ep = b.ComputeFunction("entry");
    b.Append(ep->Block(), [&] {
        auto* mat = b.Construct(ty.subgroup_matrix_right(ty.f32(), 8, 8));
        b.CallExplicit(ty.void_(), core::BuiltinFn::kSubgroupMatrixStore,
                       Vector<core::ir::TemplateParameter, 1>{core::Majorness::kRowMajor}, buffer,
                       4_i, mat, 32_i);
        b.Return(ep);
    });

    auto* src = R"(
$B1: {  # root
  %buffer:ptr<storage, array<f32>, read_write> = var undef @binding_point(0, 0)
}

%entry = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:subgroup_matrix_right<f32, 8, 8> = construct
    %4:void = subgroupMatrixStore<row_major> %buffer, 4i, %3, 32i
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %buffer:ptr<storage, array<f32>, read_write> = var undef @binding_point(0, 0)
}

%entry = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<function, msl.cooperative_tensor_right<32, 8, 8, f32, f32>, read_write> = var undef
    %4:void = msl.fill_cooperative_tensor %3, 0.0f
    %5:ptr<storage, f32, read_write> = access %buffer, 4i
    %6:msl.tensor_inline = msl.make_tensor_inline %5, vec2<u32>(8u), 32u
    %tint_dst_tensor:msl.tensor_inline = let %6
    %8:msl.cooperative_tensor_right<32, 8, 8, f32, f32> = load %3
    %9:void = %8.store %tint_dst_tensor
    ret
  }
}
)";

    Run(CooperativeTensors);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_CooperativeTensorsTest, SubgroupMatrixStore_Workgroup) {
    auto* buffer = b.Var("buffer", ty.ptr<workgroup, array<f16, 256>, read_write>());
    mod.root_block->Append(buffer);

    auto* ep = b.ComputeFunction("entry");
    b.Append(ep->Block(), [&] {
        auto* mat = b.Construct(ty.subgroup_matrix_left(ty.f16(), 16, 16));
        b.CallExplicit(ty.void_(), core::BuiltinFn::kSubgroupMatrixStore,
                       Vector<core::ir::TemplateParameter, 1>{core::Majorness::kRowMajor}, buffer,
                       0_u, mat, 16_u);
        b.Return(ep);
    });

    auto* src = R"(
$B1: {  # root
  %buffer:ptr<workgroup, array<f16, 256>, read_write> = var undef
}

%entry = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:subgroup_matrix_left<f16, 16, 16> = construct
    %4:void = subgroupMatrixStore<row_major> %buffer, 0u, %3, 16u
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %buffer:ptr<workgroup, array<f16, 256>, read_write> = var undef
}

%entry = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<function, msl.cooperative_tensor_left<16, 32, 16, f16, f16>, read_write> = var undef
    %4:void = msl.fill_cooperative_tensor %3, 0.0h
    %5:ptr<workgroup, f16, read_write> = access %buffer, 0u
    %6:msl.tensor_inline = msl.make_tensor_inline %5, vec2<u32>(16u), 16u
    %tint_dst_tensor:msl.tensor_inline = let %6
    %8:msl.cooperative_tensor_left<16, 32, 16, f16, f16> = load %3
    %9:void = %8.store %tint_dst_tensor
    ret
  }
}
)";

    Run(CooperativeTensors);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_CooperativeTensorsTest, SubgroupMatrixLoad_RowMajor) {
    auto* buffer = b.Var("buffer", ty.ptr<storage, array<f32>, core::Access::kRead>());
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* ep = b.ComputeFunction("entry");
    b.Append(ep->Block(), [&] {
        auto* mat_ty = ty.subgroup_matrix_left(ty.f32(), 8, 8);
        auto* mat = b.CallExplicit(
            mat_ty, core::BuiltinFn::kSubgroupMatrixLoad,
            Vector<core::ir::TemplateParameter, 2>{mat_ty, core::Majorness::kRowMajor}, buffer, 0_u,
            64_u);
        b.Let("x", mat);
        b.Return(ep);
    });

    auto* src = R"(
$B1: {  # root
  %buffer:ptr<storage, array<f32>, read> = var undef @binding_point(0, 0)
}

%entry = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:subgroup_matrix_left<f32, 8, 8> = subgroupMatrixLoad<subgroup_matrix_left<f32, 8, 8>, row_major> %buffer, 0u, 64u
    %x:subgroup_matrix_left<f32, 8, 8> = let %3
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %buffer:ptr<storage, array<f32>, read> = var undef @binding_point(0, 0)
}

%entry = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<storage, f32, read> = access %buffer, 0u
    %4:msl.tensor_inline = msl.make_tensor_inline %3, vec2<u32>(8u), 64u
    %tint_src_tensor:msl.tensor_inline = let %4
    %x:ptr<function, msl.cooperative_tensor_left<8, 32, 8, f32, f32>, read_write> = var undef
    %7:msl.cooperative_tensor_left<8, 32, 8, f32, f32> = load %x
    %8:void = %7.load %tint_src_tensor
    ret
  }
}
)";

    Run(CooperativeTensors);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_CooperativeTensorsTest, DISABLED_SubgroupMatrixLoad_ColMajor) {
    auto* buffer = b.Var("buffer", ty.ptr<storage, array<f32>, core::Access::kRead>());
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* ep = b.ComputeFunction("entry");
    b.Append(ep->Block(), [&] {
        auto* mat_ty = ty.subgroup_matrix_left(ty.f32(), 8, 8);
        auto* mat = b.CallExplicit(
            mat_ty, core::BuiltinFn::kSubgroupMatrixLoad,
            Vector<core::ir::TemplateParameter, 2>{mat_ty, core::Majorness::kColMajor}, buffer, 0_u,
            64_u);
        b.Let("x", mat);
        b.Return(ep);
    });

    auto* src = R"(
$B1: {  # root
  %buffer:ptr<storage, array<f32>, read> = var undef @binding_point(0, 0)
}

%entry = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:subgroup_matrix_left<f32, 8, 8> = subgroupMatrixLoad<subgroup_matrix_left<f32, 8, 8>, col_major> %buffer, 0u, 64u
    %x:subgroup_matrix_left<f32, 8, 8> = let %3
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
    // TODO(556210460): implement polyfill for column-major layout.
)";

    Run(CooperativeTensors);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_CooperativeTensorsTest, SubgroupMatrixLoad_ElementTypeMismatch) {
    auto* buffer = b.Var("buffer", ty.ptr<storage, array<vec4<f32>>, core::Access::kRead>());
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* ep = b.ComputeFunction("entry");
    b.Append(ep->Block(), [&] {
        auto* mat_ty = ty.subgroup_matrix_left(ty.f32(), 8, 8);
        auto* mat = b.CallExplicit(
            mat_ty, core::BuiltinFn::kSubgroupMatrixLoad,
            Vector<core::ir::TemplateParameter, 2>{mat_ty, core::Majorness::kRowMajor}, buffer, 1_u,
            16_u);
        b.Let("x", mat);
        b.Return(ep);
    });

    auto* src = R"(
$B1: {  # root
  %buffer:ptr<storage, array<vec4<f32>>, read> = var undef @binding_point(0, 0)
}

%entry = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:subgroup_matrix_left<f32, 8, 8> = subgroupMatrixLoad<subgroup_matrix_left<f32, 8, 8>, row_major> %buffer, 1u, 16u
    %x:subgroup_matrix_left<f32, 8, 8> = let %3
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %buffer:ptr<storage, array<vec4<f32>>, read> = var undef @binding_point(0, 0)
}

%entry = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<storage, f32, read> = msl.pointer_offset<f32> %buffer, 16u
    %4:msl.tensor_inline = msl.make_tensor_inline %3, vec2<u32>(8u), 64u
    %tint_src_tensor:msl.tensor_inline = let %4
    %x:ptr<function, msl.cooperative_tensor_left<8, 32, 8, f32, f32>, read_write> = var undef
    %7:msl.cooperative_tensor_left<8, 32, 8, f32, f32> = load %x
    %8:void = %7.load %tint_src_tensor
    ret
  }
}
)";

    Run(CooperativeTensors);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_CooperativeTensorsTest, SubgroupMatrixLoad_RedundantPointerOffset) {
    auto* buffer = b.Var("buffer", ty.ptr<storage, array<f32>, core::Access::kRead>());
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* ep = b.ComputeFunction("entry");
    b.Append(ep->Block(), [&] {
        auto* pre_cast = b.CallExplicit<msl::ir::BuiltinCall>(
            ty.ptr<storage, array<f32>, core::Access::kRead>(), msl::BuiltinFn::kPointerOffset,
            Vector<core::ir::TemplateParameter, 1>{ty.array<f32>()}, buffer, 0_u);
        auto* mat_ty = ty.subgroup_matrix_left(ty.f32(), 8, 8);
        auto* mat = b.CallExplicit(
            mat_ty, core::BuiltinFn::kSubgroupMatrixLoad,
            Vector<core::ir::TemplateParameter, 2>{mat_ty, core::Majorness::kRowMajor}, pre_cast,
            0_u, 64_u);
        b.Let("x", mat);
        b.Return(ep);
    });

    auto* src = R"(
$B1: {  # root
  %buffer:ptr<storage, array<f32>, read> = var undef @binding_point(0, 0)
}

%entry = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<storage, array<f32>, read> = msl.pointer_offset<array<f32>> %buffer, 0u
    %4:subgroup_matrix_left<f32, 8, 8> = subgroupMatrixLoad<subgroup_matrix_left<f32, 8, 8>, row_major> %3, 0u, 64u
    %x:subgroup_matrix_left<f32, 8, 8> = let %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %buffer:ptr<storage, array<f32>, read> = var undef @binding_point(0, 0)
}

%entry = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<storage, f32, read> = access %buffer, 0u
    %4:msl.tensor_inline = msl.make_tensor_inline %3, vec2<u32>(8u), 64u
    %tint_src_tensor:msl.tensor_inline = let %4
    %x:ptr<function, msl.cooperative_tensor_left<8, 32, 8, f32, f32>, read_write> = var undef
    %7:msl.cooperative_tensor_left<8, 32, 8, f32, f32> = load %x
    %8:void = %7.load %tint_src_tensor
    ret
  }
}
)";

    Run(CooperativeTensors);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_CooperativeTensorsTest, SubgroupMatrixLoad_SignedOffsetAndStride) {
    auto* buffer = b.Var("buffer", ty.ptr<storage, array<f32>, core::Access::kRead>());
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* ep = b.ComputeFunction("entry");
    b.Append(ep->Block(), [&] {
        auto* mat_ty = ty.subgroup_matrix_right(ty.f32(), 8, 8);
        auto* mat = b.CallExplicit(
            mat_ty, core::BuiltinFn::kSubgroupMatrixLoad,
            Vector<core::ir::TemplateParameter, 2>{mat_ty, core::Majorness::kRowMajor}, buffer, 4_i,
            32_i);
        b.Let("x", mat);
        b.Return(ep);
    });

    auto* src = R"(
$B1: {  # root
  %buffer:ptr<storage, array<f32>, read> = var undef @binding_point(0, 0)
}

%entry = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:subgroup_matrix_right<f32, 8, 8> = subgroupMatrixLoad<subgroup_matrix_right<f32, 8, 8>, row_major> %buffer, 4i, 32i
    %x:subgroup_matrix_right<f32, 8, 8> = let %3
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %buffer:ptr<storage, array<f32>, read> = var undef @binding_point(0, 0)
}

%entry = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<storage, f32, read> = access %buffer, 4i
    %4:msl.tensor_inline = msl.make_tensor_inline %3, vec2<u32>(8u), 32u
    %tint_src_tensor:msl.tensor_inline = let %4
    %x:ptr<function, msl.cooperative_tensor_right<32, 8, 8, f32, f32>, read_write> = var undef
    %7:msl.cooperative_tensor_right<32, 8, 8, f32, f32> = load %x
    %8:void = %7.load %tint_src_tensor
    ret
  }
}
)";

    Run(CooperativeTensors);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_CooperativeTensorsTest, SubgroupMatrixLoad_Workgroup) {
    auto* buffer = b.Var("buffer", ty.ptr<workgroup, array<f16, 256>, read_write>());
    mod.root_block->Append(buffer);

    auto* ep = b.ComputeFunction("entry");
    b.Append(ep->Block(), [&] {
        auto* mat_ty = ty.subgroup_matrix_left(ty.f16(), 16, 16);
        auto* mat = b.CallExplicit(
            mat_ty, core::BuiltinFn::kSubgroupMatrixLoad,
            Vector<core::ir::TemplateParameter, 2>{mat_ty, core::Majorness::kRowMajor}, buffer, 0_u,
            16_u);
        b.Let("x", mat);
        b.Return(ep);
    });

    auto* src = R"(
$B1: {  # root
  %buffer:ptr<workgroup, array<f16, 256>, read_write> = var undef
}

%entry = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:subgroup_matrix_left<f16, 16, 16> = subgroupMatrixLoad<subgroup_matrix_left<f16, 16, 16>, row_major> %buffer, 0u, 16u
    %x:subgroup_matrix_left<f16, 16, 16> = let %3
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %buffer:ptr<workgroup, array<f16, 256>, read_write> = var undef
}

%entry = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<workgroup, f16, read_write> = access %buffer, 0u
    %4:msl.tensor_inline = msl.make_tensor_inline %3, vec2<u32>(16u), 16u
    %tint_src_tensor:msl.tensor_inline = let %4
    %x:ptr<function, msl.cooperative_tensor_left<16, 32, 16, f16, f16>, read_write> = var undef
    %7:msl.cooperative_tensor_left<16, 32, 16, f16, f16> = load %x
    %8:void = %7.load %tint_src_tensor
    ret
  }
}
)";

    Run(CooperativeTensors);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_CooperativeTensorsTest, SubgroupMatrixMultiply) {
    auto* ep = b.ComputeFunction("entry");
    b.Append(ep->Block(), [&] {
        auto* lhs = b.Construct(ty.subgroup_matrix_left(ty.f32(), 32, 32));
        auto* rhs = b.Construct(ty.subgroup_matrix_right(ty.f32(), 32, 32));
        auto* mat = b.CallExplicit(ty.subgroup_matrix_result(ty.f32(), 32, 32),
                                   core::BuiltinFn::kSubgroupMatrixMultiply,
                                   Vector<core::ir::TemplateParameter, 1>{ty.f32()}, lhs, rhs);
        b.Let("x", mat);
        b.Return(ep);
    });

    auto* src = R"(
%entry = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:subgroup_matrix_left<f32, 32, 32> = construct
    %3:subgroup_matrix_right<f32, 32, 32> = construct
    %4:subgroup_matrix_result<f32, 32, 32> = subgroupMatrixMultiply<f32> %2, %3
    %x:subgroup_matrix_result<f32, 32, 32> = let %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%entry = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, msl.cooperative_tensor_left<32, 32, 32, f32, f32>, read_write> = var undef
    %3:void = msl.fill_cooperative_tensor %2, 0.0f
    %4:ptr<function, msl.cooperative_tensor_right<32, 32, 32, f32, f32>, read_write> = var undef
    %5:void = msl.fill_cooperative_tensor %4, 0.0f
    %x:ptr<function, msl.cooperative_tensor_result<32, 32, 32, f32, f32>, read_write> = var undef
    %7:msl.cooperative_tensor_left<32, 32, 32, f32, f32> = load %2
    %8:msl.cooperative_tensor_right<32, 32, 32, f32, f32> = load %4
    %9:msl.cooperative_tensor_result<32, 32, 32, f32, f32> = load %x
    %10:void = msl.run_tensor_multiply %7, %8, %9
    ret
  }
}
)";

    Run(CooperativeTensors);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_CooperativeTensorsTest, SubgroupMatrixMultiplyAccumulate) {
    auto* ep = b.ComputeFunction("entry");
    b.Append(ep->Block(), [&] {
        auto* lhs = b.Construct(ty.subgroup_matrix_left(ty.f32(), 32, 32));
        auto* rhs = b.Construct(ty.subgroup_matrix_right(ty.f32(), 32, 32));
        auto* acc = b.Construct(ty.subgroup_matrix_result(ty.f32(), 32, 32));
        auto* mat = b.Call(ty.subgroup_matrix_result(ty.f32(), 32, 32),
                           core::BuiltinFn::kSubgroupMatrixMultiplyAccumulate, lhs, rhs, acc);
        b.Let("x", mat);
        b.Return(ep);
    });

    auto* src = R"(
%entry = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:subgroup_matrix_left<f32, 32, 32> = construct
    %3:subgroup_matrix_right<f32, 32, 32> = construct
    %4:subgroup_matrix_result<f32, 32, 32> = construct
    %5:subgroup_matrix_result<f32, 32, 32> = subgroupMatrixMultiplyAccumulate %2, %3, %4
    %x:subgroup_matrix_result<f32, 32, 32> = let %5
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%entry = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, msl.cooperative_tensor_left<32, 32, 32, f32, f32>, read_write> = var undef
    %3:void = msl.fill_cooperative_tensor %2, 0.0f
    %4:ptr<function, msl.cooperative_tensor_right<32, 32, 32, f32, f32>, read_write> = var undef
    %5:void = msl.fill_cooperative_tensor %4, 0.0f
    %6:ptr<function, msl.cooperative_tensor_result<32, 32, 32, f32, f32>, read_write> = var undef
    %7:void = msl.fill_cooperative_tensor %6, 0.0f
    %x:ptr<function, msl.cooperative_tensor_result<32, 32, 32, f32, f32>, read_write> = var undef
    %9:void = msl.copy_cooperative_tensor %x, %6
    %10:msl.cooperative_tensor_left<32, 32, 32, f32, f32> = load %2
    %11:msl.cooperative_tensor_right<32, 32, 32, f32, f32> = load %4
    %12:msl.cooperative_tensor_result<32, 32, 32, f32, f32> = load %x
    %13:void = msl.run_tensor_multiply_accumulate %10, %11, %12
    ret
  }
}
)";

    Run(CooperativeTensors);

    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::msl::writer::raise
