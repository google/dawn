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

#include "src/tint/builtin/address_space.h"
#include "src/tint/type/test_helper.h"
#include "src/tint/type/texture.h"

namespace tint::type {
namespace {

using PointerTest = TestHelper;

TEST_F(PointerTest, Creation) {
    auto* a = create<Pointer>(create<I32>(), builtin::AddressSpace::kStorage,
                              builtin::Access::kReadWrite);
    auto* b = create<Pointer>(create<I32>(), builtin::AddressSpace::kStorage,
                              builtin::Access::kReadWrite);
    auto* c = create<Pointer>(create<F32>(), builtin::AddressSpace::kStorage,
                              builtin::Access::kReadWrite);
    auto* d = create<Pointer>(create<I32>(), builtin::AddressSpace::kPrivate,
                              builtin::Access::kReadWrite);
    auto* e =
        create<Pointer>(create<I32>(), builtin::AddressSpace::kStorage, builtin::Access::kRead);

    EXPECT_TRUE(a->StoreType()->Is<I32>());
    EXPECT_EQ(a->AddressSpace(), builtin::AddressSpace::kStorage);
    EXPECT_EQ(a->Access(), builtin::Access::kReadWrite);

    EXPECT_EQ(a, b);
    EXPECT_NE(a, c);
    EXPECT_NE(a, d);
    EXPECT_NE(a, e);
}

TEST_F(PointerTest, Hash) {
    auto* a = create<Pointer>(create<I32>(), builtin::AddressSpace::kStorage,
                              builtin::Access::kReadWrite);
    auto* b = create<Pointer>(create<I32>(), builtin::AddressSpace::kStorage,
                              builtin::Access::kReadWrite);

    EXPECT_EQ(a->unique_hash, b->unique_hash);
}

TEST_F(PointerTest, Equals) {
    auto* a = create<Pointer>(create<I32>(), builtin::AddressSpace::kStorage,
                              builtin::Access::kReadWrite);
    auto* b = create<Pointer>(create<I32>(), builtin::AddressSpace::kStorage,
                              builtin::Access::kReadWrite);
    auto* c = create<Pointer>(create<F32>(), builtin::AddressSpace::kStorage,
                              builtin::Access::kReadWrite);
    auto* d = create<Pointer>(create<I32>(), builtin::AddressSpace::kPrivate,
                              builtin::Access::kReadWrite);
    auto* e =
        create<Pointer>(create<I32>(), builtin::AddressSpace::kStorage, builtin::Access::kRead);

    EXPECT_TRUE(a->Equals(*b));
    EXPECT_FALSE(a->Equals(*c));
    EXPECT_FALSE(a->Equals(*d));
    EXPECT_FALSE(a->Equals(*e));
    EXPECT_FALSE(a->Equals(Void{}));
}

TEST_F(PointerTest, FriendlyName) {
    auto* r =
        create<Pointer>(create<I32>(), builtin::AddressSpace::kUndefined, builtin::Access::kRead);
    EXPECT_EQ(r->FriendlyName(), "ptr<i32, read>");
}

TEST_F(PointerTest, FriendlyNameWithAddressSpace) {
    auto* r =
        create<Pointer>(create<I32>(), builtin::AddressSpace::kWorkgroup, builtin::Access::kRead);
    EXPECT_EQ(r->FriendlyName(), "ptr<workgroup, i32, read>");
}

TEST_F(PointerTest, Clone) {
    auto* a = create<Pointer>(create<I32>(), builtin::AddressSpace::kStorage,
                              builtin::Access::kReadWrite);

    type::Manager mgr;
    type::CloneContext ctx{{nullptr}, {nullptr, &mgr}};

    auto* ptr = a->Clone(ctx);
    EXPECT_TRUE(ptr->StoreType()->Is<I32>());
    EXPECT_EQ(ptr->AddressSpace(), builtin::AddressSpace::kStorage);
    EXPECT_EQ(ptr->Access(), builtin::Access::kReadWrite);
}

}  // namespace
}  // namespace tint::type
