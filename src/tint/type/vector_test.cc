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

using VectorTest = TestHelper;

TEST_F(VectorTest, Creation) {
    auto* a = create<Vector>(create<I32>(), 2u);
    auto* b = create<Vector>(create<I32>(), 2u);
    auto* c = create<Vector>(create<F32>(), 2u);
    auto* d = create<Vector>(create<F32>(), 3u);

    EXPECT_EQ(a->type(), create<I32>());
    EXPECT_EQ(a->Width(), 2u);

    EXPECT_EQ(a, b);
    EXPECT_NE(a, c);
    EXPECT_NE(a, d);
}

TEST_F(VectorTest, Creation_Packed) {
    auto* v = create<Vector>(create<F32>(), 3u);
    auto* p1 = create<Vector>(create<F32>(), 3u, true);
    auto* p2 = create<Vector>(create<F32>(), 3u, true);

    EXPECT_FALSE(v->Packed());

    EXPECT_EQ(p1->type(), create<F32>());
    EXPECT_EQ(p1->Width(), 3u);
    EXPECT_TRUE(p1->Packed());

    EXPECT_NE(v, p1);
    EXPECT_EQ(p1, p2);
}

TEST_F(VectorTest, Hash) {
    auto* a = create<Vector>(create<I32>(), 2u);
    auto* b = create<Vector>(create<I32>(), 2u);

    EXPECT_EQ(a->unique_hash, b->unique_hash);
}

TEST_F(VectorTest, Equals) {
    auto* a = create<Vector>(create<I32>(), 2u);
    auto* b = create<Vector>(create<I32>(), 2u);
    auto* c = create<Vector>(create<F32>(), 2u);
    auto* d = create<Vector>(create<F32>(), 3u);

    EXPECT_TRUE(a->Equals(*b));
    EXPECT_FALSE(a->Equals(*c));
    EXPECT_FALSE(a->Equals(*d));
    EXPECT_FALSE(a->Equals(Void{}));
}

TEST_F(VectorTest, FriendlyName) {
    auto* f32 = create<F32>();
    auto* v = create<Vector>(f32, 3u);
    EXPECT_EQ(v->FriendlyName(), "vec3<f32>");
}

TEST_F(VectorTest, FriendlyName_Packed) {
    auto* f32 = create<F32>();
    auto* v = create<Vector>(f32, 3u, true);
    EXPECT_EQ(v->FriendlyName(), "__packed_vec3<f32>");
}

TEST_F(VectorTest, Clone) {
    auto* a = create<Vector>(create<I32>(), 2u);

    type::Manager mgr;
    type::CloneContext ctx{{nullptr}, {nullptr, &mgr}};

    auto* vec = a->Clone(ctx);
    EXPECT_TRUE(vec->type()->Is<I32>());
    EXPECT_EQ(vec->Width(), 2u);
    EXPECT_FALSE(vec->Packed());
}

TEST_F(VectorTest, Clone_Packed) {
    auto* a = create<Vector>(create<I32>(), 3u, true);

    type::Manager mgr;
    type::CloneContext ctx{{nullptr}, {nullptr, &mgr}};

    auto* vec = a->Clone(ctx);
    EXPECT_TRUE(vec->type()->Is<I32>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TRUE(vec->Packed());
}

}  // namespace
}  // namespace tint::type
