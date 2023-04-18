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

#include "src/tint/type/reference.h"
#include "src/tint/builtin/address_space.h"
#include "src/tint/type/test_helper.h"

namespace tint::type {
namespace {

using ReferenceTest = TestHelper;

TEST_F(ReferenceTest, Creation) {
    auto* a = create<Reference>(create<I32>(), builtin::AddressSpace::kStorage,
                                builtin::Access::kReadWrite);
    auto* b = create<Reference>(create<I32>(), builtin::AddressSpace::kStorage,
                                builtin::Access::kReadWrite);
    auto* c = create<Reference>(create<F32>(), builtin::AddressSpace::kStorage,
                                builtin::Access::kReadWrite);
    auto* d = create<Reference>(create<I32>(), builtin::AddressSpace::kPrivate,
                                builtin::Access::kReadWrite);
    auto* e =
        create<Reference>(create<I32>(), builtin::AddressSpace::kStorage, builtin::Access::kRead);

    EXPECT_TRUE(a->StoreType()->Is<I32>());
    EXPECT_EQ(a->AddressSpace(), builtin::AddressSpace::kStorage);
    EXPECT_EQ(a->Access(), builtin::Access::kReadWrite);

    EXPECT_EQ(a, b);
    EXPECT_NE(a, c);
    EXPECT_NE(a, d);
    EXPECT_NE(a, e);
}

TEST_F(ReferenceTest, Hash) {
    auto* a = create<Reference>(create<I32>(), builtin::AddressSpace::kStorage,
                                builtin::Access::kReadWrite);
    auto* b = create<Reference>(create<I32>(), builtin::AddressSpace::kStorage,
                                builtin::Access::kReadWrite);

    EXPECT_EQ(a->unique_hash, b->unique_hash);
}

TEST_F(ReferenceTest, Equals) {
    auto* a = create<Reference>(create<I32>(), builtin::AddressSpace::kStorage,
                                builtin::Access::kReadWrite);
    auto* b = create<Reference>(create<I32>(), builtin::AddressSpace::kStorage,
                                builtin::Access::kReadWrite);
    auto* c = create<Reference>(create<F32>(), builtin::AddressSpace::kStorage,
                                builtin::Access::kReadWrite);
    auto* d = create<Reference>(create<I32>(), builtin::AddressSpace::kPrivate,
                                builtin::Access::kReadWrite);
    auto* e =
        create<Reference>(create<I32>(), builtin::AddressSpace::kStorage, builtin::Access::kRead);

    EXPECT_TRUE(a->Equals(*b));
    EXPECT_FALSE(a->Equals(*c));
    EXPECT_FALSE(a->Equals(*d));
    EXPECT_FALSE(a->Equals(*e));
    EXPECT_FALSE(a->Equals(Void{}));
}

TEST_F(ReferenceTest, FriendlyName) {
    auto* r =
        create<Reference>(create<I32>(), builtin::AddressSpace::kUndefined, builtin::Access::kRead);
    EXPECT_EQ(r->FriendlyName(), "ref<i32, read>");
}

TEST_F(ReferenceTest, FriendlyNameWithAddressSpace) {
    auto* r =
        create<Reference>(create<I32>(), builtin::AddressSpace::kWorkgroup, builtin::Access::kRead);
    EXPECT_EQ(r->FriendlyName(), "ref<workgroup, i32, read>");
}

TEST_F(ReferenceTest, Clone) {
    auto* a = create<Reference>(create<I32>(), builtin::AddressSpace::kStorage,
                                builtin::Access::kReadWrite);

    type::Manager mgr;
    type::CloneContext ctx{{nullptr}, {nullptr, &mgr}};

    auto* ref = a->Clone(ctx);
    EXPECT_TRUE(ref->StoreType()->Is<I32>());
    EXPECT_EQ(ref->AddressSpace(), builtin::AddressSpace::kStorage);
    EXPECT_EQ(ref->Access(), builtin::Access::kReadWrite);
}

}  // namespace
}  // namespace tint::type
