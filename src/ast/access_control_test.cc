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

#include "src/ast/access_control.h"

#include "src/ast/alias.h"
#include "src/ast/array.h"
#include "src/ast/bool.h"
#include "src/ast/f32.h"
#include "src/ast/i32.h"
#include "src/ast/matrix.h"
#include "src/ast/pointer.h"
#include "src/ast/sampler.h"
#include "src/ast/struct.h"
#include "src/ast/test_helper.h"
#include "src/ast/texture.h"
#include "src/ast/u32.h"
#include "src/ast/vector.h"

namespace tint {
namespace ast {
namespace {

using AstAccessControlTest = TestHelper;

TEST_F(AstAccessControlTest, Create) {
  auto* u32 = create<U32>();
  auto* a = create<AccessControl>(AccessControl::kReadWrite, u32);
  EXPECT_TRUE(a->IsReadWrite());
  EXPECT_EQ(a->type(), u32);
}

TEST_F(AstAccessControlTest, AccessRead) {
  auto* i32 = create<I32>();
  auto* ac = create<AccessControl>(AccessControl::kReadOnly, i32);
  EXPECT_TRUE(ac->IsReadOnly());
  EXPECT_FALSE(ac->IsWriteOnly());
  EXPECT_FALSE(ac->IsReadWrite());

  EXPECT_EQ(ac->type_name(), "__access_control_read_only__i32");
}

TEST_F(AstAccessControlTest, AccessWrite) {
  auto* i32 = create<I32>();
  auto* ac = create<AccessControl>(AccessControl::kWriteOnly, i32);
  EXPECT_FALSE(ac->IsReadOnly());
  EXPECT_TRUE(ac->IsWriteOnly());
  EXPECT_FALSE(ac->IsReadWrite());

  EXPECT_EQ(ac->type_name(), "__access_control_write_only__i32");
}

TEST_F(AstAccessControlTest, AccessReadWrite) {
  auto* i32 = create<I32>();
  auto* ac = create<AccessControl>(AccessControl::kReadWrite, i32);
  EXPECT_FALSE(ac->IsReadOnly());
  EXPECT_FALSE(ac->IsWriteOnly());
  EXPECT_TRUE(ac->IsReadWrite());

  EXPECT_EQ(ac->type_name(), "__access_control_read_write__i32");
}

TEST_F(AstAccessControlTest, FriendlyNameReadOnly) {
  auto* i32 = create<I32>();
  auto* ac = create<AccessControl>(AccessControl::kReadOnly, i32);
  EXPECT_EQ(ac->FriendlyName(Symbols()), "[[access(read)]] i32");
}

TEST_F(AstAccessControlTest, FriendlyNameWriteOnly) {
  auto* i32 = create<I32>();
  auto* ac = create<AccessControl>(AccessControl::kWriteOnly, i32);
  EXPECT_EQ(ac->FriendlyName(Symbols()), "[[access(write)]] i32");
}

TEST_F(AstAccessControlTest, FriendlyNameReadWrite) {
  auto* i32 = create<I32>();
  auto* ac = create<AccessControl>(AccessControl::kReadWrite, i32);
  EXPECT_EQ(ac->FriendlyName(Symbols()), "[[access(read_write)]] i32");
}

}  // namespace
}  // namespace ast
}  // namespace tint
