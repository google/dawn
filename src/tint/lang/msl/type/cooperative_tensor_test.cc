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

#include "src/tint/lang/msl/type/cooperative_tensor.h"

#include "src/tint/lang/core/type/f32.h"
#include "src/tint/lang/core/type/helper_test.h"
#include "src/tint/lang/core/type/i32.h"
#include "src/tint/lang/core/type/i8.h"
#include "src/tint/lang/core/type/manager.h"
#include "src/tint/lang/core/type/u32.h"

namespace tint::msl::type {
namespace {

using CooperativeTensorTest = core::type::TestHelper;

TEST_F(CooperativeTensorTest, Creation) {
    core::type::Manager ty;
    auto* f32 = ty.f32();
    auto* u32 = ty.u32();

    auto* l1 = ty.Get<CooperativeTensor>(core::SubgroupMatrixKind::kLeft, 3u, 4u, 5u, f32, u32);

    EXPECT_EQ(l1->InputType(), f32);
    EXPECT_EQ(l1->ResultType(), u32);
    EXPECT_EQ(l1->Kind(), core::SubgroupMatrixKind::kLeft);
    EXPECT_EQ(l1->M(), 3u);
    EXPECT_EQ(l1->N(), 4u);
    EXPECT_EQ(l1->K(), 5u);
}

TEST_F(CooperativeTensorTest, Hash) {
    core::type::Manager ty;
    auto* a =
        ty.Get<CooperativeTensor>(core::SubgroupMatrixKind::kRight, 3u, 4u, 5u, ty.i32(), ty.u32());
    auto* b =
        ty.Get<CooperativeTensor>(core::SubgroupMatrixKind::kRight, 3u, 4u, 5u, ty.i32(), ty.u32());

    EXPECT_EQ(a->unique_hash, b->unique_hash);
}

TEST_F(CooperativeTensorTest, Equals) {
    core::type::Manager ty;
    auto* f32 = ty.f32();
    auto* i8 = ty.i8();
    auto* u32 = ty.u32();

    auto* l1 = ty.Get<CooperativeTensor>(core::SubgroupMatrixKind::kLeft, 3u, 4u, 5u, f32, u32);
    auto* l2 = ty.Get<CooperativeTensor>(core::SubgroupMatrixKind::kLeft, 3u, 4u, 5u, f32, u32);
    auto* l3 = ty.Get<CooperativeTensor>(core::SubgroupMatrixKind::kLeft, 3u, 4u, 5u, i8, u32);
    auto* l4 = ty.Get<CooperativeTensor>(core::SubgroupMatrixKind::kLeft, 4u, 3u, 5u, f32, u32);

    auto* r1 = ty.Get<CooperativeTensor>(core::SubgroupMatrixKind::kRight, 3u, 4u, 5u, f32, u32);
    auto* r2 = ty.Get<CooperativeTensor>(core::SubgroupMatrixKind::kRight, 3u, 4u, 5u, f32, u32);
    auto* res1 = ty.Get<CooperativeTensor>(core::SubgroupMatrixKind::kResult, 3u, 4u, 5u, f32, u32);
    auto* res2 = ty.Get<CooperativeTensor>(core::SubgroupMatrixKind::kResult, 3u, 4u, 5u, f32, u32);

    EXPECT_EQ(l1, l2);
    EXPECT_NE(l1, l3);
    EXPECT_NE(l1, l4);
    EXPECT_NE(l1, r1);
    EXPECT_NE(l1, res1);

    EXPECT_EQ(r1, r2);
    EXPECT_NE(r1, res1);

    EXPECT_EQ(res1, res2);
}

TEST_F(CooperativeTensorTest, FriendlyName_Left) {
    core::type::I8 i8;
    core::type::F32 f32;
    CooperativeTensor m{core::SubgroupMatrixKind::kLeft, 2, 4, 16, &i8, &f32};
    EXPECT_EQ(m.FriendlyName(), "msl.cooperative_tensor_left<2, 4, 16, i8, f32>");
}

TEST_F(CooperativeTensorTest, FriendlyName_Right) {
    core::type::F32 f32;
    core::type::U32 u32;
    CooperativeTensor m{core::SubgroupMatrixKind::kRight, 8, 8, 16, &f32, &u32};
    EXPECT_EQ(m.FriendlyName(), "msl.cooperative_tensor_right<8, 8, 16, f32, u32>");
}

TEST_F(CooperativeTensorTest, FriendlyName_Result) {
    core::type::U32 u32;
    core::type::I8 i8;
    CooperativeTensor m{core::SubgroupMatrixKind::kResult, 32, 32, 16, &u32, &i8};
    EXPECT_EQ(m.FriendlyName(), "msl.cooperative_tensor_result<32, 32, 16, u32, i8>");
}

TEST_F(CooperativeTensorTest, IdentifierName_Left) {
    core::type::I8 i8;
    core::type::F32 f32;
    CooperativeTensor m{core::SubgroupMatrixKind::kLeft, 2, 4, 16, &i8, &f32};
    EXPECT_EQ(m.IdentifierName(), "msl_cooperative_tensor_left_2_4_16_i8_f32");
}

TEST_F(CooperativeTensorTest, IdentifierName_Right) {
    core::type::F32 f32;
    core::type::U32 u32;
    CooperativeTensor m{core::SubgroupMatrixKind::kRight, 8, 8, 16, &f32, &u32};
    EXPECT_EQ(m.IdentifierName(), "msl_cooperative_tensor_right_8_8_16_f32_u32");
}

TEST_F(CooperativeTensorTest, IdentifierName_Result) {
    core::type::U32 u32;
    core::type::I8 i8;
    CooperativeTensor m{core::SubgroupMatrixKind::kResult, 32, 32, 16, &u32, &i8};
    EXPECT_EQ(m.IdentifierName(), "msl_cooperative_tensor_result_32_32_16_u32_i8");
}

TEST_F(CooperativeTensorTest, Clone) {
    core::type::Manager ty;
    auto* a = ty.Get<CooperativeTensor>(core::SubgroupMatrixKind::kResult, 3u, 4u, 5u, ty.i32(),
                                        ty.u32());

    core::type::Manager mgr;
    core::type::CloneContext ctx{{nullptr}, {nullptr, &mgr}};

    auto* s = a->Clone(ctx);
    EXPECT_EQ(core::SubgroupMatrixKind::kResult, s->Kind());
    EXPECT_TRUE(s->InputType()->Is<core::type::I32>());
    EXPECT_TRUE(s->ResultType()->Is<core::type::U32>());
    EXPECT_EQ(s->M(), 3u);
    EXPECT_EQ(s->N(), 4u);
    EXPECT_EQ(s->K(), 5u);
}

}  // namespace
}  // namespace tint::msl::type
