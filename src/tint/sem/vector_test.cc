// Copyright 2020 The Tint Authors.
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

#include "src/tint/sem/test_helper.h"
#include "src/tint/sem/texture.h"

namespace tint::sem {
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

TEST_F(VectorTest, Hash) {
    auto* a = create<Vector>(create<I32>(), 2u);
    auto* b = create<Vector>(create<I32>(), 2u);
    auto* c = create<Vector>(create<F32>(), 2u);
    auto* d = create<Vector>(create<F32>(), 3u);

    EXPECT_EQ(a->Hash(), b->Hash());
    EXPECT_NE(a->Hash(), c->Hash());
    EXPECT_NE(a->Hash(), d->Hash());
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
    EXPECT_EQ(v->FriendlyName(Symbols()), "vec3<f32>");
}

}  // namespace
}  // namespace tint::sem
