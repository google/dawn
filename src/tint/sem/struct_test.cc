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

#include "src/tint/sem/struct.h"
#include "src/tint/sem/test_helper.h"
#include "src/tint/type/texture.h"

namespace tint::sem {
namespace {

using namespace tint::number_suffixes;  // NOLINT
using SemStructTest = TestHelper;

TEST_F(SemStructTest, Creation) {
    auto name = Sym("S");
    auto* impl = create<ast::Struct>(Ident(name), utils::Empty, utils::Empty);
    auto* ptr = impl;
    auto* s = create<sem::Struct>(impl, impl->name->symbol, utils::Empty, 4u /* align */,
                                  8u /* size */, 16u /* size_no_padding */);
    EXPECT_EQ(s->Declaration(), ptr);
    EXPECT_EQ(s->Align(), 4u);
    EXPECT_EQ(s->Size(), 8u);
    EXPECT_EQ(s->SizeNoPadding(), 16u);
}

TEST_F(SemStructTest, Equals) {
    auto* a_impl = create<ast::Struct>(Ident("a"), utils::Empty, utils::Empty);
    auto* a = create<sem::Struct>(a_impl, a_impl->name->symbol, utils::Empty, 4u /* align */,
                                  4u /* size */, 4u /* size_no_padding */);
    auto* b_impl = create<ast::Struct>(Ident("b"), utils::Empty, utils::Empty);
    auto* b = create<sem::Struct>(b_impl, b_impl->name->symbol, utils::Empty, 4u /* align */,
                                  4u /* size */, 4u /* size_no_padding */);

    EXPECT_TRUE(a->Equals(*a));
    EXPECT_FALSE(a->Equals(*b));
    EXPECT_FALSE(a->Equals(type::Void{}));
}

TEST_F(SemStructTest, FriendlyName) {
    auto name = Sym("my_struct");
    auto* impl = create<ast::Struct>(Ident(name), utils::Empty, utils::Empty);
    auto* s = create<sem::Struct>(impl, impl->name->symbol, utils::Empty, 4u /* align */,
                                  4u /* size */, 4u /* size_no_padding */);
    EXPECT_EQ(s->FriendlyName(), "my_struct");
}

}  // namespace
}  // namespace tint::sem
