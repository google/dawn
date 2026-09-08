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

}  // namespace
}  // namespace tint::msl::writer::raise
