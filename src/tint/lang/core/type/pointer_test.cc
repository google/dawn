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

#include "src/tint/lang/core/address_space.h"
#include "src/tint/lang/core/type/helper_test.h"
#include "src/tint/lang/core/type/texture.h"

namespace tint::type {
namespace {

using PointerTest = TestHelper;

TEST_F(PointerTest, Creation) {
    auto* a =
        create<Pointer>(core::AddressSpace::kStorage, create<I32>(), core::Access::kReadWrite);
    auto* b =
        create<Pointer>(core::AddressSpace::kStorage, create<I32>(), core::Access::kReadWrite);
    auto* c =
        create<Pointer>(core::AddressSpace::kStorage, create<F32>(), core::Access::kReadWrite);
    auto* d =
        create<Pointer>(core::AddressSpace::kPrivate, create<I32>(), core::Access::kReadWrite);
    auto* e = create<Pointer>(core::AddressSpace::kStorage, create<I32>(), core::Access::kRead);

    EXPECT_TRUE(a->StoreType()->Is<I32>());
    EXPECT_EQ(a->AddressSpace(), core::AddressSpace::kStorage);
    EXPECT_EQ(a->Access(), core::Access::kReadWrite);

    EXPECT_EQ(a, b);
    EXPECT_NE(a, c);
    EXPECT_NE(a, d);
    EXPECT_NE(a, e);
}

TEST_F(PointerTest, Hash) {
    auto* a =
        create<Pointer>(core::AddressSpace::kStorage, create<I32>(), core::Access::kReadWrite);
    auto* b =
        create<Pointer>(core::AddressSpace::kStorage, create<I32>(), core::Access::kReadWrite);

    EXPECT_EQ(a->unique_hash, b->unique_hash);
}

TEST_F(PointerTest, Equals) {
    auto* a =
        create<Pointer>(core::AddressSpace::kStorage, create<I32>(), core::Access::kReadWrite);
    auto* b =
        create<Pointer>(core::AddressSpace::kStorage, create<I32>(), core::Access::kReadWrite);
    auto* c =
        create<Pointer>(core::AddressSpace::kStorage, create<F32>(), core::Access::kReadWrite);
    auto* d =
        create<Pointer>(core::AddressSpace::kPrivate, create<I32>(), core::Access::kReadWrite);
    auto* e = create<Pointer>(core::AddressSpace::kStorage, create<I32>(), core::Access::kRead);

    EXPECT_TRUE(a->Equals(*b));
    EXPECT_FALSE(a->Equals(*c));
    EXPECT_FALSE(a->Equals(*d));
    EXPECT_FALSE(a->Equals(*e));
    EXPECT_FALSE(a->Equals(Void{}));
}

TEST_F(PointerTest, FriendlyName) {
    auto* r = create<Pointer>(core::AddressSpace::kUndefined, create<I32>(), core::Access::kRead);
    EXPECT_EQ(r->FriendlyName(), "ptr<i32, read>");
}

TEST_F(PointerTest, FriendlyNameWithAddressSpace) {
    auto* r = create<Pointer>(core::AddressSpace::kWorkgroup, create<I32>(), core::Access::kRead);
    EXPECT_EQ(r->FriendlyName(), "ptr<workgroup, i32, read>");
}

TEST_F(PointerTest, Clone) {
    auto* a =
        create<Pointer>(core::AddressSpace::kStorage, create<I32>(), core::Access::kReadWrite);

    type::Manager mgr;
    type::CloneContext ctx{{nullptr}, {nullptr, &mgr}};

    auto* ptr = a->Clone(ctx);
    EXPECT_TRUE(ptr->StoreType()->Is<I32>());
    EXPECT_EQ(ptr->AddressSpace(), core::AddressSpace::kStorage);
    EXPECT_EQ(ptr->Access(), core::Access::kReadWrite);
}

}  // namespace
}  // namespace tint::type
