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

#include "gmock/gmock.h"
#include "src/tint/lang/core/type/subgroup_matrix.h"
#include "src/tint/lang/msl/writer/helper_test.h"

namespace tint::msl::writer {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

class MslWriterTensorTest : public MslWriterTest {
  protected:
    Result<SuccessType> Generate() {
        Options options;
        options.extensions.enable_tensors = true;
        return MslWriterTest::Generate(options, validate::MslVersion::kMsl_4_0);
    }
};

TEST_F(MslWriterTensorTest, VarWithNoInitializer) {
    auto* ep = b.ComputeFunction("entry");
    b.Append(ep->Block(), [&] {
        b.Var("lhs", ty.ptr(function, ty.subgroup_matrix_left(ty.f16(), 32, 32)));
        b.Var("rhs", ty.ptr(function, ty.subgroup_matrix_right(ty.f16(), 32, 32)));
        b.Var("acc", ty.ptr(function, ty.subgroup_matrix_result(ty.f16(), 32, 32)));
        b.Return(ep);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure() << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
#include <MetalPerformancePrimitives/MetalPerformancePrimitives.h>

template<uint M, uint N, uint K,
         mpp::tensor_ops::matmul2d_descriptor::mode O>
constant constexpr auto tint_matmul2d_descriptor =
  mpp::tensor_ops::matmul2d_descriptor(M, N, K, false, false, false, O);

template<uint M, uint N, uint K,
         mpp::tensor_ops::matmul2d_descriptor::mode O = mpp::tensor_ops::matmul2d_descriptor::mode::multiply>
using tint_matmul2d_operation =
  mpp::tensor_ops::matmul2d<tint_matmul2d_descriptor<M, N, K, O>, execution_simdgroup>;

using tint_left_input_32_32_32_half_half =
  decltype(declval<tint_matmul2d_operation<32, 32, 32>>()
             .get_left_input_cooperative_tensor<half, half, half>());
using tint_right_input_32_32_32_half_half =
  decltype(declval<tint_matmul2d_operation<32, 32, 32>>()
             .get_right_input_cooperative_tensor<half, half, half>());
using tint_destination_32_32_32_half_half =
  decltype(declval<tint_matmul2d_operation<32, 32, 32>>()
             .get_destination_cooperative_tensor<tint_left_input_32_32_32_half_half, tint_right_input_32_32_32_half_half, half>());

template<typename T, typename V>
void tint_fill_cooperative_tensor(thread T* dst, V value) {
  for (uint i = 0; i < dst->get_capacity(); i++) {
    dst->set(i, value);
  }
}

[[max_total_threads_per_threadgroup(1)]]
kernel void entry() {
  tint_left_input_32_32_32_half_half lhs;
  (tint_fill_cooperative_tensor((&lhs), 0.0h));
  tint_right_input_32_32_32_half_half rhs;
  (tint_fill_cooperative_tensor((&rhs), 0.0h));
  tint_destination_32_32_32_half_half acc;
  (tint_fill_cooperative_tensor((&acc), 0.0h));
}
)");
}

TEST_F(MslWriterTensorTest, VarWithZeroConstruct) {
    auto* ep = b.ComputeFunction("entry");
    b.Append(ep->Block(), [&] {
        b.Var<function>("lhs", b.Construct(ty.subgroup_matrix_left(ty.f16(), 32, 32)));
        b.Var<function>("rhs", b.Construct(ty.subgroup_matrix_right(ty.f16(), 32, 32)));
        b.Var<function>("acc", b.Construct(ty.subgroup_matrix_result(ty.f16(), 32, 32)));
        b.Return(ep);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure() << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
#include <MetalPerformancePrimitives/MetalPerformancePrimitives.h>

template<uint M, uint N, uint K,
         mpp::tensor_ops::matmul2d_descriptor::mode O>
constant constexpr auto tint_matmul2d_descriptor =
  mpp::tensor_ops::matmul2d_descriptor(M, N, K, false, false, false, O);

template<uint M, uint N, uint K,
         mpp::tensor_ops::matmul2d_descriptor::mode O = mpp::tensor_ops::matmul2d_descriptor::mode::multiply>
using tint_matmul2d_operation =
  mpp::tensor_ops::matmul2d<tint_matmul2d_descriptor<M, N, K, O>, execution_simdgroup>;

using tint_left_input_32_32_32_half_half =
  decltype(declval<tint_matmul2d_operation<32, 32, 32>>()
             .get_left_input_cooperative_tensor<half, half, half>());
using tint_right_input_32_32_32_half_half =
  decltype(declval<tint_matmul2d_operation<32, 32, 32>>()
             .get_right_input_cooperative_tensor<half, half, half>());
using tint_destination_32_32_32_half_half =
  decltype(declval<tint_matmul2d_operation<32, 32, 32>>()
             .get_destination_cooperative_tensor<tint_left_input_32_32_32_half_half, tint_right_input_32_32_32_half_half, half>());

template<typename T, typename V>
void tint_fill_cooperative_tensor(thread T* dst, V value) {
  for (uint i = 0; i < dst->get_capacity(); i++) {
    dst->set(i, value);
  }
}

[[max_total_threads_per_threadgroup(1)]]
kernel void entry() {
  tint_left_input_32_32_32_half_half lhs;
  (tint_fill_cooperative_tensor((&lhs), 0.0h));
  tint_right_input_32_32_32_half_half rhs;
  (tint_fill_cooperative_tensor((&rhs), 0.0h));
  tint_destination_32_32_32_half_half acc;
  (tint_fill_cooperative_tensor((&acc), 0.0h));
}
)");
}

TEST_F(MslWriterTensorTest, VarWithValueConstruct) {
    auto* ep = b.ComputeFunction("entry");
    b.Append(ep->Block(), [&] {
        b.Var<function>("lhs", b.Construct(ty.subgroup_matrix_left(ty.f16(), 32, 32), 1.0_h));
        b.Var<function>("rhs", b.Construct(ty.subgroup_matrix_right(ty.f16(), 32, 32), 2.0_h));
        b.Var<function>("acc", b.Construct(ty.subgroup_matrix_result(ty.f16(), 32, 32), 3.0_h));
        b.Return(ep);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure() << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
#include <MetalPerformancePrimitives/MetalPerformancePrimitives.h>

template<uint M, uint N, uint K,
         mpp::tensor_ops::matmul2d_descriptor::mode O>
constant constexpr auto tint_matmul2d_descriptor =
  mpp::tensor_ops::matmul2d_descriptor(M, N, K, false, false, false, O);

template<uint M, uint N, uint K,
         mpp::tensor_ops::matmul2d_descriptor::mode O = mpp::tensor_ops::matmul2d_descriptor::mode::multiply>
using tint_matmul2d_operation =
  mpp::tensor_ops::matmul2d<tint_matmul2d_descriptor<M, N, K, O>, execution_simdgroup>;

using tint_left_input_32_32_32_half_half =
  decltype(declval<tint_matmul2d_operation<32, 32, 32>>()
             .get_left_input_cooperative_tensor<half, half, half>());
using tint_right_input_32_32_32_half_half =
  decltype(declval<tint_matmul2d_operation<32, 32, 32>>()
             .get_right_input_cooperative_tensor<half, half, half>());
using tint_destination_32_32_32_half_half =
  decltype(declval<tint_matmul2d_operation<32, 32, 32>>()
             .get_destination_cooperative_tensor<tint_left_input_32_32_32_half_half, tint_right_input_32_32_32_half_half, half>());

template<typename T, typename V>
void tint_fill_cooperative_tensor(thread T* dst, V value) {
  for (uint i = 0; i < dst->get_capacity(); i++) {
    dst->set(i, value);
  }
}

[[max_total_threads_per_threadgroup(1)]]
kernel void entry() {
  tint_left_input_32_32_32_half_half lhs;
  (tint_fill_cooperative_tensor((&lhs), 1.0h));
  tint_right_input_32_32_32_half_half rhs;
  (tint_fill_cooperative_tensor((&rhs), 2.0h));
  tint_destination_32_32_32_half_half acc;
  (tint_fill_cooperative_tensor((&acc), 3.0h));
}
)");
}

TEST_F(MslWriterTensorTest, MultipleGeometries) {
    auto* ep = b.ComputeFunction("entry");
    b.Append(ep->Block(), [&] {
        b.Var("lhs_32x32x32", ty.ptr(function, ty.subgroup_matrix_left(ty.f16(), 32, 32)));
        b.Var("rhs_32x32x32", ty.ptr(function, ty.subgroup_matrix_right(ty.f16(), 32, 32)));
        b.Var("acc_32x32x32", ty.ptr(function, ty.subgroup_matrix_result(ty.f16(), 32, 32)));
        b.Var("lhs_32x16x16", ty.ptr(function, ty.subgroup_matrix_left(ty.f16(), 16, 32)));
        b.Var("rhs_32x16x16", ty.ptr(function, ty.subgroup_matrix_right(ty.f16(), 16, 16)));
        b.Var("acc_32x16x16", ty.ptr(function, ty.subgroup_matrix_result(ty.f16(), 16, 32)));
        b.Return(ep);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure() << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
#include <MetalPerformancePrimitives/MetalPerformancePrimitives.h>

template<uint M, uint N, uint K,
         mpp::tensor_ops::matmul2d_descriptor::mode O>
constant constexpr auto tint_matmul2d_descriptor =
  mpp::tensor_ops::matmul2d_descriptor(M, N, K, false, false, false, O);

template<uint M, uint N, uint K,
         mpp::tensor_ops::matmul2d_descriptor::mode O = mpp::tensor_ops::matmul2d_descriptor::mode::multiply>
using tint_matmul2d_operation =
  mpp::tensor_ops::matmul2d<tint_matmul2d_descriptor<M, N, K, O>, execution_simdgroup>;

using tint_left_input_32_32_32_half_half =
  decltype(declval<tint_matmul2d_operation<32, 32, 32>>()
             .get_left_input_cooperative_tensor<half, half, half>());
using tint_right_input_32_32_32_half_half =
  decltype(declval<tint_matmul2d_operation<32, 32, 32>>()
             .get_right_input_cooperative_tensor<half, half, half>());
using tint_destination_32_32_32_half_half =
  decltype(declval<tint_matmul2d_operation<32, 32, 32>>()
             .get_destination_cooperative_tensor<tint_left_input_32_32_32_half_half, tint_right_input_32_32_32_half_half, half>());

template<typename T, typename V>
void tint_fill_cooperative_tensor(thread T* dst, V value) {
  for (uint i = 0; i < dst->get_capacity(); i++) {
    dst->set(i, value);
  }
}

using tint_left_input_32_32_16_half_half =
  decltype(declval<tint_matmul2d_operation<32, 32, 16>>()
             .get_left_input_cooperative_tensor<half, half, half>());
using tint_right_input_32_32_16_half_half =
  decltype(declval<tint_matmul2d_operation<32, 32, 16>>()
             .get_right_input_cooperative_tensor<half, half, half>());
using tint_destination_32_32_16_half_half =
  decltype(declval<tint_matmul2d_operation<32, 32, 16>>()
             .get_destination_cooperative_tensor<tint_left_input_32_32_16_half_half, tint_right_input_32_32_16_half_half, half>());

using tint_left_input_32_16_16_half_half =
  decltype(declval<tint_matmul2d_operation<32, 16, 16>>()
             .get_left_input_cooperative_tensor<half, half, half>());
using tint_right_input_32_16_16_half_half =
  decltype(declval<tint_matmul2d_operation<32, 16, 16>>()
             .get_right_input_cooperative_tensor<half, half, half>());
using tint_destination_32_16_16_half_half =
  decltype(declval<tint_matmul2d_operation<32, 16, 16>>()
             .get_destination_cooperative_tensor<tint_left_input_32_16_16_half_half, tint_right_input_32_16_16_half_half, half>());

using tint_left_input_32_16_32_half_half =
  decltype(declval<tint_matmul2d_operation<32, 16, 32>>()
             .get_left_input_cooperative_tensor<half, half, half>());
using tint_right_input_32_16_32_half_half =
  decltype(declval<tint_matmul2d_operation<32, 16, 32>>()
             .get_right_input_cooperative_tensor<half, half, half>());
using tint_destination_32_16_32_half_half =
  decltype(declval<tint_matmul2d_operation<32, 16, 32>>()
             .get_destination_cooperative_tensor<tint_left_input_32_16_32_half_half, tint_right_input_32_16_32_half_half, half>());

[[max_total_threads_per_threadgroup(1)]]
kernel void entry() {
  tint_left_input_32_32_32_half_half lhs_32x32x32;
  (tint_fill_cooperative_tensor((&lhs_32x32x32), 0.0h));
  tint_right_input_32_32_32_half_half rhs_32x32x32;
  (tint_fill_cooperative_tensor((&rhs_32x32x32), 0.0h));
  tint_destination_32_32_32_half_half acc_32x32x32;
  (tint_fill_cooperative_tensor((&acc_32x32x32), 0.0h));
  tint_left_input_32_32_16_half_half lhs_32x16x16;
  (tint_fill_cooperative_tensor((&lhs_32x16x16), 0.0h));
  tint_right_input_32_16_16_half_half rhs_32x16x16;
  (tint_fill_cooperative_tensor((&rhs_32x16x16), 0.0h));
  tint_destination_32_16_32_half_half acc_32x16x16;
  (tint_fill_cooperative_tensor((&acc_32x16x16), 0.0h));
}
)");
}

TEST_F(MslWriterTensorTest, SubgroupMatrixStore_RowMajor) {
    auto* buffer = b.Var("buffer", ty.ptr<storage, array<f16, 8192>, read_write>());
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* ep = b.ComputeFunction("entry");
    b.Append(ep->Block(), [&] {
        auto* mat = b.Construct(ty.subgroup_matrix_left(ty.f16(), 32, 32));
        b.CallExplicit(ty.void_(), core::BuiltinFn::kSubgroupMatrixStore,
                       Vector<core::ir::TemplateParameter, 1>{core::Majorness::kRowMajor}, buffer,
                       0_u, mat, 64_u);
        b.Return(ep);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure() << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
template<typename T, size_t N>
struct tint_array {
  const constant T& operator[](size_t i) const constant { return elements[i]; }
  device T& operator[](size_t i) device { return elements[i]; }
  const device T& operator[](size_t i) const device { return elements[i]; }
  thread T& operator[](size_t i) thread { return elements[i]; }
  const thread T& operator[](size_t i) const thread { return elements[i]; }
  threadgroup T& operator[](size_t i) threadgroup { return elements[i]; }
  const threadgroup T& operator[](size_t i) const threadgroup { return elements[i]; }
  T elements[N];
};

struct tint_module_vars_struct {
  device tint_array<half, 8192>* tint_member;
};

#include <MetalPerformancePrimitives/MetalPerformancePrimitives.h>

template<uint M, uint N, uint K,
         mpp::tensor_ops::matmul2d_descriptor::mode O>
constant constexpr auto tint_matmul2d_descriptor =
  mpp::tensor_ops::matmul2d_descriptor(M, N, K, false, false, false, O);

template<uint M, uint N, uint K,
         mpp::tensor_ops::matmul2d_descriptor::mode O = mpp::tensor_ops::matmul2d_descriptor::mode::multiply>
using tint_matmul2d_operation =
  mpp::tensor_ops::matmul2d<tint_matmul2d_descriptor<M, N, K, O>, execution_simdgroup>;

using tint_left_input_32_32_32_half_half =
  decltype(declval<tint_matmul2d_operation<32, 32, 32>>()
             .get_left_input_cooperative_tensor<half, half, half>());
using tint_right_input_32_32_32_half_half =
  decltype(declval<tint_matmul2d_operation<32, 32, 32>>()
             .get_right_input_cooperative_tensor<half, half, half>());
using tint_destination_32_32_32_half_half =
  decltype(declval<tint_matmul2d_operation<32, 32, 32>>()
             .get_destination_cooperative_tensor<tint_left_input_32_32_32_half_half, tint_right_input_32_32_32_half_half, half>());

template<typename T, typename V>
void tint_fill_cooperative_tensor(thread T* dst, V value) {
  for (uint i = 0; i < dst->get_capacity(); i++) {
    dst->set(i, value);
  }
}

[[max_total_threads_per_threadgroup(1)]]
kernel void entry(device tint_array<half, 8192>* v [[buffer(0)]]) {
  tint_module_vars_struct const tint_module_vars = tint_module_vars_struct{.tint_member=v};
  tint_left_input_32_32_32_half_half v_1;
  (tint_fill_cooperative_tensor((&v_1), 0.0h));
  auto const tint_dst_tensor = tensor<device half, dextents<uint, 2>, tensor_inline>((&(*tint_module_vars.tint_member)[0u]), dextents<uint, 2>(32, 32), array<uint, 2>({1u, 64u}));
  (v_1.store(tint_dst_tensor));
}
)");
}

TEST_F(MslWriterTensorTest, DISABLED_SubgroupMatrixStore_ColMajor) {
    auto* buffer = b.Var("buffer", ty.ptr<storage, array<f16, 8192>, read_write>());
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* ep = b.ComputeFunction("entry");
    b.Append(ep->Block(), [&] {
        auto* mat = b.Construct(ty.subgroup_matrix_left(ty.f16(), 32, 32));
        b.CallExplicit(ty.void_(), core::BuiltinFn::kSubgroupMatrixStore,
                       Vector<core::ir::TemplateParameter, 1>{core::Majorness::kColMajor}, buffer,
                       0_u, mat, 64_u);
        b.Return(ep);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure() << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
      // TODO(556210460): implement polyfill for column-major layout.
)");
}

TEST_F(MslWriterTensorTest, SubgroupMatrixStore_Workgroup) {
    auto* buffer = b.Var("buffer", ty.ptr<workgroup, array<f16, 1024>, read_write>());
    mod.root_block->Append(buffer);

    auto* ep = b.ComputeFunction("entry");
    b.Append(ep->Block(), [&] {
        auto* mat = b.Construct(ty.subgroup_matrix_left(ty.f16(), 32, 32));
        b.CallExplicit(ty.void_(), core::BuiltinFn::kSubgroupMatrixStore,
                       Vector<core::ir::TemplateParameter, 1>{core::Majorness::kRowMajor}, buffer,
                       0_u, mat, 32_u);
        b.Return(ep);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure() << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
template<typename T, size_t N>
struct tint_array {
  const constant T& operator[](size_t i) const constant { return elements[i]; }
  device T& operator[](size_t i) device { return elements[i]; }
  const device T& operator[](size_t i) const device { return elements[i]; }
  thread T& operator[](size_t i) thread { return elements[i]; }
  const thread T& operator[](size_t i) const thread { return elements[i]; }
  threadgroup T& operator[](size_t i) threadgroup { return elements[i]; }
  const threadgroup T& operator[](size_t i) const threadgroup { return elements[i]; }
  T elements[N];
};

struct tint_module_vars_struct {
  threadgroup tint_array<half, 1024>* tint_member;
};

#include <MetalPerformancePrimitives/MetalPerformancePrimitives.h>

template<uint M, uint N, uint K,
         mpp::tensor_ops::matmul2d_descriptor::mode O>
constant constexpr auto tint_matmul2d_descriptor =
  mpp::tensor_ops::matmul2d_descriptor(M, N, K, false, false, false, O);

template<uint M, uint N, uint K,
         mpp::tensor_ops::matmul2d_descriptor::mode O = mpp::tensor_ops::matmul2d_descriptor::mode::multiply>
using tint_matmul2d_operation =
  mpp::tensor_ops::matmul2d<tint_matmul2d_descriptor<M, N, K, O>, execution_simdgroup>;

using tint_left_input_32_32_32_half_half =
  decltype(declval<tint_matmul2d_operation<32, 32, 32>>()
             .get_left_input_cooperative_tensor<half, half, half>());
using tint_right_input_32_32_32_half_half =
  decltype(declval<tint_matmul2d_operation<32, 32, 32>>()
             .get_right_input_cooperative_tensor<half, half, half>());
using tint_destination_32_32_32_half_half =
  decltype(declval<tint_matmul2d_operation<32, 32, 32>>()
             .get_destination_cooperative_tensor<tint_left_input_32_32_32_half_half, tint_right_input_32_32_32_half_half, half>());

template<typename T, typename V>
void tint_fill_cooperative_tensor(thread T* dst, V value) {
  for (uint i = 0; i < dst->get_capacity(); i++) {
    dst->set(i, value);
  }
}

struct tint_symbol_1 {
  tint_array<half, 1024> tint_symbol;
};

void entry_inner(uint tint_local_index, tint_module_vars_struct tint_module_vars) {
  {
    uint v = 0u;
    v = tint_local_index;
    while(true) {
      uint const v_1 = v;
      if ((v_1 >= 1024u)) {
        break;
      }
      (*tint_module_vars.tint_member)[v_1] = 0.0h;
      {
        v = (v_1 + 1u);
      }
    }
  }
  (threadgroup_barrier(mem_flags::mem_threadgroup));
  tint_left_input_32_32_32_half_half v_2;
  (tint_fill_cooperative_tensor((&v_2), 0.0h));
  auto const tint_dst_tensor = tensor<threadgroup half, dextents<uint, 2>, tensor_inline>((&(*tint_module_vars.tint_member)[0u]), dextents<uint, 2>(32, 32), array<uint, 2>({1u, 32u}));
  (v_2.store(tint_dst_tensor));
}

[[max_total_threads_per_threadgroup(1)]]
kernel void entry(uint tint_local_index [[thread_index_in_threadgroup]], threadgroup tint_symbol_1* v_3 [[threadgroup(0)]]) {
  tint_module_vars_struct const tint_module_vars = tint_module_vars_struct{.tint_member=(&(*v_3).tint_symbol)};
  (entry_inner(tint_local_index, tint_module_vars));
}
)");
}

TEST_F(MslWriterTensorTest, SubgroupMatrixLoad_RowMajor) {
    auto* buffer = b.Var("buffer", ty.ptr<storage, array<f16, 8192>, core::Access::kRead>());
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* ep = b.ComputeFunction("entry");
    b.Append(ep->Block(), [&] {
        auto* mat_ty = ty.subgroup_matrix_left(ty.f16(), 32, 32);
        auto* mat = b.CallExplicit(
            mat_ty, core::BuiltinFn::kSubgroupMatrixLoad,
            Vector<core::ir::TemplateParameter, 2>{mat_ty, core::Majorness::kRowMajor}, buffer, 0_u,
            64_u);
        b.Let("x", mat);
        b.Return(ep);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure() << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
template<typename T, size_t N>
struct tint_array {
  const constant T& operator[](size_t i) const constant { return elements[i]; }
  device T& operator[](size_t i) device { return elements[i]; }
  const device T& operator[](size_t i) const device { return elements[i]; }
  thread T& operator[](size_t i) thread { return elements[i]; }
  const thread T& operator[](size_t i) const thread { return elements[i]; }
  threadgroup T& operator[](size_t i) threadgroup { return elements[i]; }
  const threadgroup T& operator[](size_t i) const threadgroup { return elements[i]; }
  T elements[N];
};

struct tint_module_vars_struct {
  const device tint_array<half, 8192>* tint_member;
};

#include <MetalPerformancePrimitives/MetalPerformancePrimitives.h>

template<uint M, uint N, uint K,
         mpp::tensor_ops::matmul2d_descriptor::mode O>
constant constexpr auto tint_matmul2d_descriptor =
  mpp::tensor_ops::matmul2d_descriptor(M, N, K, false, false, false, O);

template<uint M, uint N, uint K,
         mpp::tensor_ops::matmul2d_descriptor::mode O = mpp::tensor_ops::matmul2d_descriptor::mode::multiply>
using tint_matmul2d_operation =
  mpp::tensor_ops::matmul2d<tint_matmul2d_descriptor<M, N, K, O>, execution_simdgroup>;

using tint_left_input_32_32_32_half_half =
  decltype(declval<tint_matmul2d_operation<32, 32, 32>>()
             .get_left_input_cooperative_tensor<half, half, half>());
using tint_right_input_32_32_32_half_half =
  decltype(declval<tint_matmul2d_operation<32, 32, 32>>()
             .get_right_input_cooperative_tensor<half, half, half>());
using tint_destination_32_32_32_half_half =
  decltype(declval<tint_matmul2d_operation<32, 32, 32>>()
             .get_destination_cooperative_tensor<tint_left_input_32_32_32_half_half, tint_right_input_32_32_32_half_half, half>());

[[max_total_threads_per_threadgroup(1)]]
kernel void entry(const device tint_array<half, 8192>* v [[buffer(0)]]) {
  tint_module_vars_struct const tint_module_vars = tint_module_vars_struct{.tint_member=v};
  auto const tint_src_tensor = tensor<device half, dextents<uint, 2>, tensor_inline>(const_cast<device half*>((&(*tint_module_vars.tint_member)[0u])), dextents<uint, 2>(32, 32), array<uint, 2>({1u, 64u}));
  tint_left_input_32_32_32_half_half x;
  (x.load(tint_src_tensor));
}
)");
}

TEST_F(MslWriterTensorTest, DISABLED_SubgroupMatrixLoad_ColMajor) {
    auto* buffer = b.Var("buffer", ty.ptr<storage, array<f16, 8192>, core::Access::kRead>());
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* ep = b.ComputeFunction("entry");
    b.Append(ep->Block(), [&] {
        auto* mat_ty = ty.subgroup_matrix_left(ty.f16(), 32, 32);
        auto* mat = b.CallExplicit(
            mat_ty, core::BuiltinFn::kSubgroupMatrixLoad,
            Vector<core::ir::TemplateParameter, 2>{mat_ty, core::Majorness::kColMajor}, buffer, 0_u,
            64_u);
        b.Let("x", mat);
        b.Return(ep);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure() << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
    // TODO(556210460): implement polyfill for column-major layout.
)");
}

TEST_F(MslWriterTensorTest, SubgroupMatrixLoad_Workgroup) {
    auto* buffer = b.Var("buffer", ty.ptr<workgroup, array<f16, 1024>, read_write>());
    mod.root_block->Append(buffer);

    auto* ep = b.ComputeFunction("entry");
    b.Append(ep->Block(), [&] {
        auto* mat_ty = ty.subgroup_matrix_left(ty.f16(), 32, 32);
        auto* mat = b.CallExplicit(
            mat_ty, core::BuiltinFn::kSubgroupMatrixLoad,
            Vector<core::ir::TemplateParameter, 2>{mat_ty, core::Majorness::kRowMajor}, buffer, 0_u,
            32_u);
        b.Let("x", mat);
        b.Return(ep);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure() << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
template<typename T, size_t N>
struct tint_array {
  const constant T& operator[](size_t i) const constant { return elements[i]; }
  device T& operator[](size_t i) device { return elements[i]; }
  const device T& operator[](size_t i) const device { return elements[i]; }
  thread T& operator[](size_t i) thread { return elements[i]; }
  const thread T& operator[](size_t i) const thread { return elements[i]; }
  threadgroup T& operator[](size_t i) threadgroup { return elements[i]; }
  const threadgroup T& operator[](size_t i) const threadgroup { return elements[i]; }
  T elements[N];
};

struct tint_module_vars_struct {
  threadgroup tint_array<half, 1024>* tint_member;
};

#include <MetalPerformancePrimitives/MetalPerformancePrimitives.h>

template<uint M, uint N, uint K,
         mpp::tensor_ops::matmul2d_descriptor::mode O>
constant constexpr auto tint_matmul2d_descriptor =
  mpp::tensor_ops::matmul2d_descriptor(M, N, K, false, false, false, O);

template<uint M, uint N, uint K,
         mpp::tensor_ops::matmul2d_descriptor::mode O = mpp::tensor_ops::matmul2d_descriptor::mode::multiply>
using tint_matmul2d_operation =
  mpp::tensor_ops::matmul2d<tint_matmul2d_descriptor<M, N, K, O>, execution_simdgroup>;

using tint_left_input_32_32_32_half_half =
  decltype(declval<tint_matmul2d_operation<32, 32, 32>>()
             .get_left_input_cooperative_tensor<half, half, half>());
using tint_right_input_32_32_32_half_half =
  decltype(declval<tint_matmul2d_operation<32, 32, 32>>()
             .get_right_input_cooperative_tensor<half, half, half>());
using tint_destination_32_32_32_half_half =
  decltype(declval<tint_matmul2d_operation<32, 32, 32>>()
             .get_destination_cooperative_tensor<tint_left_input_32_32_32_half_half, tint_right_input_32_32_32_half_half, half>());

struct tint_symbol_1 {
  tint_array<half, 1024> tint_symbol;
};

void entry_inner(uint tint_local_index, tint_module_vars_struct tint_module_vars) {
  {
    uint v = 0u;
    v = tint_local_index;
    while(true) {
      uint const v_1 = v;
      if ((v_1 >= 1024u)) {
        break;
      }
      (*tint_module_vars.tint_member)[v_1] = 0.0h;
      {
        v = (v_1 + 1u);
      }
    }
  }
  (threadgroup_barrier(mem_flags::mem_threadgroup));
  auto const tint_src_tensor = tensor<threadgroup half, dextents<uint, 2>, tensor_inline>((&(*tint_module_vars.tint_member)[0u]), dextents<uint, 2>(32, 32), array<uint, 2>({1u, 32u}));
  tint_left_input_32_32_32_half_half x;
  (x.load(tint_src_tensor));
}

[[max_total_threads_per_threadgroup(1)]]
kernel void entry(uint tint_local_index [[thread_index_in_threadgroup]], threadgroup tint_symbol_1* v_2 [[threadgroup(0)]]) {
  tint_module_vars_struct const tint_module_vars = tint_module_vars_struct{.tint_member=(&(*v_2).tint_symbol)};
  (entry_inner(tint_local_index, tint_module_vars));
}
)");
}

TEST_F(MslWriterTensorTest, SubgroupMatrixMultiply) {
    auto* ep = b.ComputeFunction("entry");
    b.Append(ep->Block(), [&] {
        auto* lhs = b.Construct(ty.subgroup_matrix_left(ty.f16(), 32, 32));
        auto* rhs = b.Construct(ty.subgroup_matrix_right(ty.f16(), 32, 32));
        auto* mat = b.CallExplicit(ty.subgroup_matrix_result(ty.f16(), 32, 32),
                                   core::BuiltinFn::kSubgroupMatrixMultiply,
                                   Vector<core::ir::TemplateParameter, 1>{ty.f16()}, lhs, rhs);
        b.Let("x", mat);
        b.Return(ep);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure() << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
#include <MetalPerformancePrimitives/MetalPerformancePrimitives.h>

template<uint M, uint N, uint K,
         mpp::tensor_ops::matmul2d_descriptor::mode O>
constant constexpr auto tint_matmul2d_descriptor =
  mpp::tensor_ops::matmul2d_descriptor(M, N, K, false, false, false, O);

template<uint M, uint N, uint K,
         mpp::tensor_ops::matmul2d_descriptor::mode O = mpp::tensor_ops::matmul2d_descriptor::mode::multiply>
using tint_matmul2d_operation =
  mpp::tensor_ops::matmul2d<tint_matmul2d_descriptor<M, N, K, O>, execution_simdgroup>;

using tint_left_input_32_32_32_half_half =
  decltype(declval<tint_matmul2d_operation<32, 32, 32>>()
             .get_left_input_cooperative_tensor<half, half, half>());
using tint_right_input_32_32_32_half_half =
  decltype(declval<tint_matmul2d_operation<32, 32, 32>>()
             .get_right_input_cooperative_tensor<half, half, half>());
using tint_destination_32_32_32_half_half =
  decltype(declval<tint_matmul2d_operation<32, 32, 32>>()
             .get_destination_cooperative_tensor<tint_left_input_32_32_32_half_half, tint_right_input_32_32_32_half_half, half>());

template<typename T, typename V>
void tint_fill_cooperative_tensor(thread T* dst, V value) {
  for (uint i = 0; i < dst->get_capacity(); i++) {
    dst->set(i, value);
  }
}

[[max_total_threads_per_threadgroup(1)]]
kernel void entry() {
  tint_left_input_32_32_32_half_half v;
  (tint_fill_cooperative_tensor((&v), 0.0h));
  tint_right_input_32_32_32_half_half v_1;
  (tint_fill_cooperative_tensor((&v_1), 0.0h));
  tint_destination_32_32_32_half_half x;
  (tint_matmul2d_operation<32, 32, 32>().run(v, v_1, x));
}
)");
}

TEST_F(MslWriterTensorTest, SubgroupMatrixMultiplyAccumulate) {
    auto* ep = b.ComputeFunction("entry");
    b.Append(ep->Block(), [&] {
        auto* lhs = b.Construct(ty.subgroup_matrix_left(ty.f16(), 32, 32));
        auto* rhs = b.Construct(ty.subgroup_matrix_right(ty.f16(), 32, 32));
        auto* acc = b.Construct(ty.subgroup_matrix_result(ty.f16(), 32, 32));
        auto* mat = b.Call(ty.subgroup_matrix_result(ty.f16(), 32, 32),
                           core::BuiltinFn::kSubgroupMatrixMultiplyAccumulate, lhs, rhs, acc);
        b.Let("x", mat);
        b.Return(ep);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure() << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
#include <MetalPerformancePrimitives/MetalPerformancePrimitives.h>

template<uint M, uint N, uint K,
         mpp::tensor_ops::matmul2d_descriptor::mode O>
constant constexpr auto tint_matmul2d_descriptor =
  mpp::tensor_ops::matmul2d_descriptor(M, N, K, false, false, false, O);

template<uint M, uint N, uint K,
         mpp::tensor_ops::matmul2d_descriptor::mode O = mpp::tensor_ops::matmul2d_descriptor::mode::multiply>
using tint_matmul2d_operation =
  mpp::tensor_ops::matmul2d<tint_matmul2d_descriptor<M, N, K, O>, execution_simdgroup>;

using tint_left_input_32_32_32_half_half =
  decltype(declval<tint_matmul2d_operation<32, 32, 32>>()
             .get_left_input_cooperative_tensor<half, half, half>());
using tint_right_input_32_32_32_half_half =
  decltype(declval<tint_matmul2d_operation<32, 32, 32>>()
             .get_right_input_cooperative_tensor<half, half, half>());
using tint_destination_32_32_32_half_half =
  decltype(declval<tint_matmul2d_operation<32, 32, 32>>()
             .get_destination_cooperative_tensor<tint_left_input_32_32_32_half_half, tint_right_input_32_32_32_half_half, half>());

template<typename T, typename V>
void tint_fill_cooperative_tensor(thread T* dst, V value) {
  for (uint i = 0; i < dst->get_capacity(); i++) {
    dst->set(i, value);
  }
}

template<typename T>
void tint_copy_cooperative_tensor(thread T* dst, const thread T* src) {
  for (uint i = 0; i < dst->get_capacity(); i++) {
    dst->set(i, src->get(i));
  }
}

[[max_total_threads_per_threadgroup(1)]]
kernel void entry() {
  tint_left_input_32_32_32_half_half v;
  (tint_fill_cooperative_tensor((&v), 0.0h));
  tint_right_input_32_32_32_half_half v_1;
  (tint_fill_cooperative_tensor((&v_1), 0.0h));
  tint_destination_32_32_32_half_half v_2;
  (tint_fill_cooperative_tensor((&v_2), 0.0h));
  tint_destination_32_32_32_half_half x;
  (tint_copy_cooperative_tensor((&x), (&v_2)));
  (tint_matmul2d_operation<32, 32, 32, mpp::tensor_ops::matmul2d_descriptor::mode::multiply_accumulate>().run(v, v_1, x));
}
)");
}

TEST_F(MslWriterTensorTest, Load) {
    auto* ep = b.ComputeFunction("entry");
    b.Append(ep->Block(), [&] {
        auto* v = b.Var("acc", ty.ptr(function, ty.subgroup_matrix_result(ty.f16(), 32, 32)));
        b.Let("x", b.Load(v));
        b.Return(ep);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure() << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
#include <MetalPerformancePrimitives/MetalPerformancePrimitives.h>

template<uint M, uint N, uint K,
         mpp::tensor_ops::matmul2d_descriptor::mode O>
constant constexpr auto tint_matmul2d_descriptor =
  mpp::tensor_ops::matmul2d_descriptor(M, N, K, false, false, false, O);

template<uint M, uint N, uint K,
         mpp::tensor_ops::matmul2d_descriptor::mode O = mpp::tensor_ops::matmul2d_descriptor::mode::multiply>
using tint_matmul2d_operation =
  mpp::tensor_ops::matmul2d<tint_matmul2d_descriptor<M, N, K, O>, execution_simdgroup>;

using tint_left_input_32_32_32_half_half =
  decltype(declval<tint_matmul2d_operation<32, 32, 32>>()
             .get_left_input_cooperative_tensor<half, half, half>());
using tint_right_input_32_32_32_half_half =
  decltype(declval<tint_matmul2d_operation<32, 32, 32>>()
             .get_right_input_cooperative_tensor<half, half, half>());
using tint_destination_32_32_32_half_half =
  decltype(declval<tint_matmul2d_operation<32, 32, 32>>()
             .get_destination_cooperative_tensor<tint_left_input_32_32_32_half_half, tint_right_input_32_32_32_half_half, half>());

template<typename T, typename V>
void tint_fill_cooperative_tensor(thread T* dst, V value) {
  for (uint i = 0; i < dst->get_capacity(); i++) {
    dst->set(i, value);
  }
}

template<typename T>
void tint_copy_cooperative_tensor(thread T* dst, const thread T* src) {
  for (uint i = 0; i < dst->get_capacity(); i++) {
    dst->set(i, src->get(i));
  }
}

[[max_total_threads_per_threadgroup(1)]]
kernel void entry() {
  tint_destination_32_32_32_half_half acc;
  (tint_fill_cooperative_tensor((&acc), 0.0h));
  tint_destination_32_32_32_half_half x;
  (tint_copy_cooperative_tensor((&x), (&acc)));
}
)");
}

TEST_F(MslWriterTensorTest, Store) {
    auto* ep = b.ComputeFunction("entry");
    b.Append(ep->Block(), [&] {
        auto* v = b.Var("acc", ty.ptr(function, ty.subgroup_matrix_result(ty.f16(), 32, 32)));
        auto* val = b.Construct(ty.subgroup_matrix_result(ty.f16(), 32, 32), 1.0_h);
        b.Store(v, val);
        b.Return(ep);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure() << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
#include <MetalPerformancePrimitives/MetalPerformancePrimitives.h>

template<uint M, uint N, uint K,
         mpp::tensor_ops::matmul2d_descriptor::mode O>
constant constexpr auto tint_matmul2d_descriptor =
  mpp::tensor_ops::matmul2d_descriptor(M, N, K, false, false, false, O);

template<uint M, uint N, uint K,
         mpp::tensor_ops::matmul2d_descriptor::mode O = mpp::tensor_ops::matmul2d_descriptor::mode::multiply>
using tint_matmul2d_operation =
  mpp::tensor_ops::matmul2d<tint_matmul2d_descriptor<M, N, K, O>, execution_simdgroup>;

using tint_left_input_32_32_32_half_half =
  decltype(declval<tint_matmul2d_operation<32, 32, 32>>()
             .get_left_input_cooperative_tensor<half, half, half>());
using tint_right_input_32_32_32_half_half =
  decltype(declval<tint_matmul2d_operation<32, 32, 32>>()
             .get_right_input_cooperative_tensor<half, half, half>());
using tint_destination_32_32_32_half_half =
  decltype(declval<tint_matmul2d_operation<32, 32, 32>>()
             .get_destination_cooperative_tensor<tint_left_input_32_32_32_half_half, tint_right_input_32_32_32_half_half, half>());

template<typename T, typename V>
void tint_fill_cooperative_tensor(thread T* dst, V value) {
  for (uint i = 0; i < dst->get_capacity(); i++) {
    dst->set(i, value);
  }
}

template<typename T>
void tint_copy_cooperative_tensor(thread T* dst, const thread T* src) {
  for (uint i = 0; i < dst->get_capacity(); i++) {
    dst->set(i, src->get(i));
  }
}

[[max_total_threads_per_threadgroup(1)]]
kernel void entry() {
  tint_destination_32_32_32_half_half acc;
  (tint_fill_cooperative_tensor((&acc), 0.0h));
  tint_destination_32_32_32_half_half v;
  (tint_fill_cooperative_tensor((&v), 1.0h));
  (tint_copy_cooperative_tensor((&acc), (&v)));
}
)");
}

TEST_F(MslWriterTensorTest, Array) {
    auto* ep = b.ComputeFunction("entry");
    b.Append(ep->Block(), [&] {
        auto* matrix_ty = ty.subgroup_matrix_result(ty.f16(), 32, 32);
        auto* matrix_ptr = ty.ptr<function>(matrix_ty);
        auto* v = b.Var("acc", ty.ptr(function, ty.array(matrix_ty, 2)));

        // Copy the whole array to a temporary and back again.
        auto* copy = b.Let("copy", b.Load(v));
        b.Store(v, copy);

        // Copy an element of the array to a temporary and back to another element.
        auto* el = b.Let("el", b.Load(b.Access(matrix_ptr, v, 1_u)));
        b.Store(b.Access(matrix_ptr, v, 0_u), el);

        b.Return(ep);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure() << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
#include <MetalPerformancePrimitives/MetalPerformancePrimitives.h>

template<uint M, uint N, uint K,
         mpp::tensor_ops::matmul2d_descriptor::mode O>
constant constexpr auto tint_matmul2d_descriptor =
  mpp::tensor_ops::matmul2d_descriptor(M, N, K, false, false, false, O);

template<uint M, uint N, uint K,
         mpp::tensor_ops::matmul2d_descriptor::mode O = mpp::tensor_ops::matmul2d_descriptor::mode::multiply>
using tint_matmul2d_operation =
  mpp::tensor_ops::matmul2d<tint_matmul2d_descriptor<M, N, K, O>, execution_simdgroup>;

using tint_left_input_32_32_32_half_half =
  decltype(declval<tint_matmul2d_operation<32, 32, 32>>()
             .get_left_input_cooperative_tensor<half, half, half>());
using tint_right_input_32_32_32_half_half =
  decltype(declval<tint_matmul2d_operation<32, 32, 32>>()
             .get_right_input_cooperative_tensor<half, half, half>());
using tint_destination_32_32_32_half_half =
  decltype(declval<tint_matmul2d_operation<32, 32, 32>>()
             .get_destination_cooperative_tensor<tint_left_input_32_32_32_half_half, tint_right_input_32_32_32_half_half, half>());

template<typename T, size_t N>
struct tint_array {
  const constant T& operator[](size_t i) const constant { return elements[i]; }
  device T& operator[](size_t i) device { return elements[i]; }
  const device T& operator[](size_t i) const device { return elements[i]; }
  thread T& operator[](size_t i) thread { return elements[i]; }
  const thread T& operator[](size_t i) const thread { return elements[i]; }
  threadgroup T& operator[](size_t i) threadgroup { return elements[i]; }
  const threadgroup T& operator[](size_t i) const threadgroup { return elements[i]; }
  T elements[N];
};

template<typename T, typename V>
void tint_fill_cooperative_tensor(thread T* dst, V value) {
  for (uint i = 0; i < dst->get_capacity(); i++) {
    dst->set(i, value);
  }
}

template<typename T>
void tint_copy_cooperative_tensor(thread T* dst, const thread T* src) {
  for (uint i = 0; i < dst->get_capacity(); i++) {
    dst->set(i, src->get(i));
  }
}

[[max_total_threads_per_threadgroup(1)]]
kernel void entry() {
  tint_destination_32_32_32_half_half v;
  tint_destination_32_32_32_half_half v_1;
  tint_array<thread tint_destination_32_32_32_half_half*, 2> acc = tint_array<thread tint_destination_32_32_32_half_half*, 2>{(&v), (&v_1)};
  (tint_fill_cooperative_tensor(acc[0u], 0.0h));
  (tint_fill_cooperative_tensor(acc[1u], 0.0h));
  tint_destination_32_32_32_half_half v_2;
  tint_destination_32_32_32_half_half v_3;
  tint_array<thread tint_destination_32_32_32_half_half*, 2> const copy = tint_array<thread tint_destination_32_32_32_half_half*, 2>{(&v_2), (&v_3)};
  (tint_copy_cooperative_tensor(copy[0u], acc[0u]));
  (tint_copy_cooperative_tensor(copy[1u], acc[1u]));
  (tint_copy_cooperative_tensor(acc[0u], copy[0u]));
  (tint_copy_cooperative_tensor(acc[1u], copy[1u]));
  tint_destination_32_32_32_half_half el;
  (tint_copy_cooperative_tensor((&el), acc[1u]));
  (tint_copy_cooperative_tensor(acc[0u], (&el)));
}
)");
}

TEST_F(MslWriterTensorTest, ArrayOfArray) {
    auto* ep = b.ComputeFunction("entry");
    b.Append(ep->Block(), [&] {
        auto* matrix_ty = ty.subgroup_matrix_result(ty.f16(), 32, 32);
        auto* matrix_ptr = ty.ptr<function>(matrix_ty);
        auto* inner_array_ptr = ty.ptr<function>(ty.array(matrix_ty, 2));
        auto* v = b.Var("acc", ty.ptr(function, ty.array(ty.array(matrix_ty, 2), 3)));

        // Copy the whole array to a temporary and back again.
        auto* copy = b.Let("copy", b.Load(v));
        b.Store(v, copy);

        // Copy the inner array to a temporary and back to another element.
        auto* inner = b.Let("inner", b.Load(b.Access(inner_array_ptr, v, 2_u)));
        b.Store(b.Access(inner_array_ptr, v, 1_u), inner);

        // Copy an element of the array to a temporary and back to another element.
        auto* el = b.Let("el", b.Load(b.Access(matrix_ptr, v, 2_u, 1_u)));
        b.Store(b.Access(matrix_ptr, v, 1_u, 0_u), el);

        b.Return(ep);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure() << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
#include <MetalPerformancePrimitives/MetalPerformancePrimitives.h>

template<uint M, uint N, uint K,
         mpp::tensor_ops::matmul2d_descriptor::mode O>
constant constexpr auto tint_matmul2d_descriptor =
  mpp::tensor_ops::matmul2d_descriptor(M, N, K, false, false, false, O);

template<uint M, uint N, uint K,
         mpp::tensor_ops::matmul2d_descriptor::mode O = mpp::tensor_ops::matmul2d_descriptor::mode::multiply>
using tint_matmul2d_operation =
  mpp::tensor_ops::matmul2d<tint_matmul2d_descriptor<M, N, K, O>, execution_simdgroup>;

using tint_left_input_32_32_32_half_half =
  decltype(declval<tint_matmul2d_operation<32, 32, 32>>()
             .get_left_input_cooperative_tensor<half, half, half>());
using tint_right_input_32_32_32_half_half =
  decltype(declval<tint_matmul2d_operation<32, 32, 32>>()
             .get_right_input_cooperative_tensor<half, half, half>());
using tint_destination_32_32_32_half_half =
  decltype(declval<tint_matmul2d_operation<32, 32, 32>>()
             .get_destination_cooperative_tensor<tint_left_input_32_32_32_half_half, tint_right_input_32_32_32_half_half, half>());

template<typename T, size_t N>
struct tint_array {
  const constant T& operator[](size_t i) const constant { return elements[i]; }
  device T& operator[](size_t i) device { return elements[i]; }
  const device T& operator[](size_t i) const device { return elements[i]; }
  thread T& operator[](size_t i) thread { return elements[i]; }
  const thread T& operator[](size_t i) const thread { return elements[i]; }
  threadgroup T& operator[](size_t i) threadgroup { return elements[i]; }
  const threadgroup T& operator[](size_t i) const threadgroup { return elements[i]; }
  T elements[N];
};

template<typename T, typename V>
void tint_fill_cooperative_tensor(thread T* dst, V value) {
  for (uint i = 0; i < dst->get_capacity(); i++) {
    dst->set(i, value);
  }
}

template<typename T>
void tint_copy_cooperative_tensor(thread T* dst, const thread T* src) {
  for (uint i = 0; i < dst->get_capacity(); i++) {
    dst->set(i, src->get(i));
  }
}

[[max_total_threads_per_threadgroup(1)]]
kernel void entry() {
  tint_destination_32_32_32_half_half v;
  tint_destination_32_32_32_half_half v_1;
  tint_array<thread tint_destination_32_32_32_half_half*, 2> const v_2 = tint_array<thread tint_destination_32_32_32_half_half*, 2>{(&v), (&v_1)};
  tint_destination_32_32_32_half_half v_3;
  tint_destination_32_32_32_half_half v_4;
  tint_array<thread tint_destination_32_32_32_half_half*, 2> const v_5 = tint_array<thread tint_destination_32_32_32_half_half*, 2>{(&v_3), (&v_4)};
  tint_destination_32_32_32_half_half v_6;
  tint_destination_32_32_32_half_half v_7;
  tint_array<tint_array<thread tint_destination_32_32_32_half_half*, 2>, 3> acc = tint_array<tint_array<thread tint_destination_32_32_32_half_half*, 2>, 3>{v_2, v_5, tint_array<thread tint_destination_32_32_32_half_half*, 2>{(&v_6), (&v_7)}};
  (tint_fill_cooperative_tensor(acc[0u][0u], 0.0h));
  (tint_fill_cooperative_tensor(acc[0u][1u], 0.0h));
  (tint_fill_cooperative_tensor(acc[1u][0u], 0.0h));
  (tint_fill_cooperative_tensor(acc[1u][1u], 0.0h));
  (tint_fill_cooperative_tensor(acc[2u][0u], 0.0h));
  (tint_fill_cooperative_tensor(acc[2u][1u], 0.0h));
  tint_destination_32_32_32_half_half v_8;
  tint_destination_32_32_32_half_half v_9;
  tint_array<thread tint_destination_32_32_32_half_half*, 2> const v_10 = tint_array<thread tint_destination_32_32_32_half_half*, 2>{(&v_8), (&v_9)};
  tint_destination_32_32_32_half_half v_11;
  tint_destination_32_32_32_half_half v_12;
  tint_array<thread tint_destination_32_32_32_half_half*, 2> const v_13 = tint_array<thread tint_destination_32_32_32_half_half*, 2>{(&v_11), (&v_12)};
  tint_destination_32_32_32_half_half v_14;
  tint_destination_32_32_32_half_half v_15;
  tint_array<tint_array<thread tint_destination_32_32_32_half_half*, 2>, 3> const copy = tint_array<tint_array<thread tint_destination_32_32_32_half_half*, 2>, 3>{v_10, v_13, tint_array<thread tint_destination_32_32_32_half_half*, 2>{(&v_14), (&v_15)}};
  (tint_copy_cooperative_tensor(copy[0u][0u], acc[0u][0u]));
  (tint_copy_cooperative_tensor(copy[0u][1u], acc[0u][1u]));
  (tint_copy_cooperative_tensor(copy[1u][0u], acc[1u][0u]));
  (tint_copy_cooperative_tensor(copy[1u][1u], acc[1u][1u]));
  (tint_copy_cooperative_tensor(copy[2u][0u], acc[2u][0u]));
  (tint_copy_cooperative_tensor(copy[2u][1u], acc[2u][1u]));
  (tint_copy_cooperative_tensor(acc[0u][0u], copy[0u][0u]));
  (tint_copy_cooperative_tensor(acc[0u][1u], copy[0u][1u]));
  (tint_copy_cooperative_tensor(acc[1u][0u], copy[1u][0u]));
  (tint_copy_cooperative_tensor(acc[1u][1u], copy[1u][1u]));
  (tint_copy_cooperative_tensor(acc[2u][0u], copy[2u][0u]));
  (tint_copy_cooperative_tensor(acc[2u][1u], copy[2u][1u]));
  tint_destination_32_32_32_half_half v_16;
  tint_destination_32_32_32_half_half v_17;
  tint_array<thread tint_destination_32_32_32_half_half*, 2> const inner = tint_array<thread tint_destination_32_32_32_half_half*, 2>{(&v_16), (&v_17)};
  (tint_copy_cooperative_tensor(inner[0u], acc[2u][0u]));
  (tint_copy_cooperative_tensor(inner[1u], acc[2u][1u]));
  (tint_copy_cooperative_tensor(acc[1u][0u], inner[0u]));
  (tint_copy_cooperative_tensor(acc[1u][1u], inner[1u]));
  tint_destination_32_32_32_half_half el;
  (tint_copy_cooperative_tensor((&el), acc[2u][1u]));
  (tint_copy_cooperative_tensor(acc[1u][0u], (&el)));
}
)");
}

TEST_F(MslWriterTensorTest, PointerAliases) {
    auto* ep = b.ComputeFunction("entry");
    b.Append(ep->Block(), [&] {
        auto* matrix_ty = ty.subgroup_matrix_result(ty.f16(), 32, 16);
        auto* var = b.Var("acc", ty.ptr<function>(ty.array(matrix_ty, 2)));
        b.Let("whole", var);
        auto* el = b.Let("el", b.Access(ty.ptr<function>(matrix_ty), var, 1_u));
        auto* el2 = b.Let("el2", el);
        auto* load = b.Load(el2);
        b.Store(b.Access(ty.ptr<function>(matrix_ty), var, 0_u), load);
        b.Return(ep);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure() << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
#include <MetalPerformancePrimitives/MetalPerformancePrimitives.h>

template<uint M, uint N, uint K,
         mpp::tensor_ops::matmul2d_descriptor::mode O>
constant constexpr auto tint_matmul2d_descriptor =
  mpp::tensor_ops::matmul2d_descriptor(M, N, K, false, false, false, O);

template<uint M, uint N, uint K,
         mpp::tensor_ops::matmul2d_descriptor::mode O = mpp::tensor_ops::matmul2d_descriptor::mode::multiply>
using tint_matmul2d_operation =
  mpp::tensor_ops::matmul2d<tint_matmul2d_descriptor<M, N, K, O>, execution_simdgroup>;

using tint_left_input_16_32_32_half_half =
  decltype(declval<tint_matmul2d_operation<16, 32, 32>>()
             .get_left_input_cooperative_tensor<half, half, half>());
using tint_right_input_16_32_32_half_half =
  decltype(declval<tint_matmul2d_operation<16, 32, 32>>()
             .get_right_input_cooperative_tensor<half, half, half>());
using tint_destination_16_32_32_half_half =
  decltype(declval<tint_matmul2d_operation<16, 32, 32>>()
             .get_destination_cooperative_tensor<tint_left_input_16_32_32_half_half, tint_right_input_16_32_32_half_half, half>());

template<typename T, size_t N>
struct tint_array {
  const constant T& operator[](size_t i) const constant { return elements[i]; }
  device T& operator[](size_t i) device { return elements[i]; }
  const device T& operator[](size_t i) const device { return elements[i]; }
  thread T& operator[](size_t i) thread { return elements[i]; }
  const thread T& operator[](size_t i) const thread { return elements[i]; }
  threadgroup T& operator[](size_t i) threadgroup { return elements[i]; }
  const threadgroup T& operator[](size_t i) const threadgroup { return elements[i]; }
  T elements[N];
};

template<typename T, typename V>
void tint_fill_cooperative_tensor(thread T* dst, V value) {
  for (uint i = 0; i < dst->get_capacity(); i++) {
    dst->set(i, value);
  }
}

template<typename T>
void tint_copy_cooperative_tensor(thread T* dst, const thread T* src) {
  for (uint i = 0; i < dst->get_capacity(); i++) {
    dst->set(i, src->get(i));
  }
}

[[max_total_threads_per_threadgroup(1)]]
kernel void entry() {
  tint_destination_16_32_32_half_half v;
  tint_destination_16_32_32_half_half v_1;
  tint_array<thread tint_destination_16_32_32_half_half*, 2> acc = tint_array<thread tint_destination_16_32_32_half_half*, 2>{(&v), (&v_1)};
  (tint_fill_cooperative_tensor(acc[0u], 0.0h));
  (tint_fill_cooperative_tensor(acc[1u], 0.0h));
  thread tint_array<thread tint_destination_16_32_32_half_half*, 2>* const whole = (&acc);
  thread tint_destination_16_32_32_half_half* const el = acc[1u];
  thread tint_destination_16_32_32_half_half* const el2 = el;
  tint_destination_16_32_32_half_half v_2;
  (tint_copy_cooperative_tensor((&v_2), el2));
  (tint_copy_cooperative_tensor(acc[0u], (&v_2)));
}
)");
}

}  // namespace
}  // namespace tint::msl::writer
