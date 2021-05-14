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

#include "src/sem/struct.h"
#include "src/sem/test_helper.h"
#include "src/sem/texture_type.h"

namespace tint {
namespace sem {
namespace {

using StructTest = TestHelper;

TEST_F(StructTest, Creation) {
  auto name = Sym("S");
  auto* impl =
      create<ast::Struct>(name, ast::StructMemberList{}, ast::DecorationList{});
  auto* ptr = impl;
  auto* s = create<sem::Struct>(impl, StructMemberList{}, 4 /* align */,
                                8 /* size */, 16 /* size_no_padding */);
  EXPECT_EQ(s->Declaration(), ptr);
  EXPECT_EQ(s->Align(), 4u);
  EXPECT_EQ(s->Size(), 8u);
  EXPECT_EQ(s->SizeNoPadding(), 16u);
}

TEST_F(StructTest, TypeName) {
  auto name = Sym("my_struct");
  auto* impl =
      create<ast::Struct>(name, ast::StructMemberList{}, ast::DecorationList{});
  auto* s = create<sem::Struct>(impl, StructMemberList{}, 4 /* align */,
                                4 /* size */, 4 /* size_no_padding */);
  EXPECT_EQ(s->type_name(), "__struct_$1");
}

TEST_F(StructTest, FriendlyName) {
  auto name = Sym("my_struct");
  auto* impl =
      create<ast::Struct>(name, ast::StructMemberList{}, ast::DecorationList{});
  auto* s = create<sem::Struct>(impl, StructMemberList{}, 4 /* align */,
                                4 /* size */, 4 /* size_no_padding */);
  EXPECT_EQ(s->FriendlyName(Symbols()), "my_struct");
}

}  // namespace
}  // namespace sem
}  // namespace tint
