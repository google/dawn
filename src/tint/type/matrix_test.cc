// Copyright 2022 The Tint Authors.
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

#include "src/tint/type/test_helper.h"
#include "src/tint/type/texture.h"

namespace tint::type {
namespace {

using MatrixTest = TestHelper;

TEST_F(MatrixTest, Creation) {
    auto* a = create<Matrix>(create<Vector>(create<I32>(), 3u), 4u);
    auto* b = create<Matrix>(create<Vector>(create<I32>(), 3u), 4u);
    auto* c = create<Matrix>(create<Vector>(create<F32>(), 3u), 4u);
    auto* d = create<Matrix>(create<Vector>(create<I32>(), 2u), 4u);
    auto* e = create<Matrix>(create<Vector>(create<I32>(), 3u), 2u);

    EXPECT_EQ(a->type(), create<I32>());
    EXPECT_EQ(a->rows(), 3u);
    EXPECT_EQ(a->columns(), 4u);

    EXPECT_EQ(a, b);
    EXPECT_NE(a, c);
    EXPECT_NE(a, d);
    EXPECT_NE(a, e);
}

TEST_F(MatrixTest, Hash) {
    auto* a = create<Matrix>(create<Vector>(create<I32>(), 3u), 4u);
    auto* b = create<Matrix>(create<Vector>(create<I32>(), 3u), 4u);

    EXPECT_EQ(a->unique_hash, b->unique_hash);
}

TEST_F(MatrixTest, Equals) {
    auto* a = create<Matrix>(create<Vector>(create<I32>(), 3u), 4u);
    auto* b = create<Matrix>(create<Vector>(create<I32>(), 3u), 4u);
    auto* c = create<Matrix>(create<Vector>(create<F32>(), 3u), 4u);
    auto* d = create<Matrix>(create<Vector>(create<I32>(), 2u), 4u);
    auto* e = create<Matrix>(create<Vector>(create<I32>(), 3u), 2u);

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
    auto* a = create<Matrix>(create<Vector>(create<I32>(), 3u), 4u);

    type::Manager mgr;
    type::CloneContext ctx{{nullptr}, {nullptr, &mgr}};

    auto* mat = a->Clone(ctx);
    EXPECT_TRUE(mat->type()->Is<I32>());
    EXPECT_EQ(mat->rows(), 3u);
    EXPECT_EQ(mat->columns(), 4u);
}

}  // namespace
}  // namespace tint::type
