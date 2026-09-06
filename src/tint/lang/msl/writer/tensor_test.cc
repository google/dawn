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

}  // namespace
}  // namespace tint::msl::writer
