// Copyright 2022 The Dawn & Tint Authors
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

#include "src/tint/lang/core/type/matrix.h"
#include "src/tint/lang/core/type/f32.h"
#include "src/tint/lang/core/type/helper_test.h"
#include "src/tint/lang/core/type/i32.h"
#include "src/tint/lang/core/type/manager.h"
#include "src/tint/lang/core/type/void.h"

namespace tint::core::type {
namespace {

using MatrixTest = TestHelper;

TEST_F(MatrixTest, Creation) {
    Manager ty;
    auto* a = ty.mat4x3(ty.i32());
    auto* b = ty.mat4x3(ty.i32());
    auto* c = ty.mat4x3(ty.f32());
    auto* d = ty.mat4x2(ty.i32());
    auto* e = ty.mat2x3(ty.i32());

    EXPECT_EQ(a->Type(), ty.i32());
    EXPECT_EQ(a->Rows(), 3u);
    EXPECT_EQ(a->Columns(), 4u);

    EXPECT_EQ(a, b);
    EXPECT_NE(a, c);
    EXPECT_NE(a, d);
    EXPECT_NE(a, e);
}

TEST_F(MatrixTest, Hash) {
    Manager ty;
    auto* a = ty.mat4x3(ty.i32());
    auto* b = ty.mat4x3(ty.i32());

    EXPECT_EQ(a->unique_hash, b->unique_hash);
}

TEST_F(MatrixTest, Equals) {
    Manager ty;
    auto* a = ty.mat4x3(ty.i32());
    auto* b = ty.mat4x3(ty.i32());
    auto* c = ty.mat4x3(ty.f32());
    auto* d = ty.mat4x2(ty.i32());
    auto* e = ty.mat2x3(ty.i32());

    EXPECT_TRUE(a->Equals(*b));
    EXPECT_FALSE(a->Equals(*c));
    EXPECT_FALSE(a->Equals(*d));
    EXPECT_FALSE(a->Equals(*e));
    EXPECT_FALSE(a->Equals(Void{}));
}

TEST_F(MatrixTest, FriendlyName) {
    I32 i32;
    Vector c{&i32, 3};
    Matrix m{&c, 2};
    EXPECT_EQ(m.FriendlyName(), "mat2x3<i32>");
}

TEST_F(MatrixTest, Clone) {
    Manager ty;
    auto* a = ty.mat4x3(ty.i32());

    core::type::Manager mgr;
    core::type::CloneContext ctx{{nullptr}, {nullptr, &mgr}};

    auto* mat = a->Clone(ctx);
    EXPECT_TRUE(mat->Type()->Is<I32>());
    EXPECT_EQ(mat->Rows(), 3u);
    EXPECT_EQ(mat->Columns(), 4u);
}

}  // namespace
}  // namespace tint::core::type
