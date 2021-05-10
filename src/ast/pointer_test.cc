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

#include "src/ast/pointer.h"
#include "src/ast/access_control.h"
#include "src/ast/alias.h"
#include "src/ast/array.h"
#include "src/ast/bool.h"
#include "src/ast/f32.h"
#include "src/ast/i32.h"
#include "src/ast/matrix.h"
#include "src/ast/sampler.h"
#include "src/ast/struct.h"
#include "src/ast/test_helper.h"
#include "src/ast/texture.h"
#include "src/ast/u32.h"
#include "src/ast/vector.h"

namespace tint {
namespace ast {
namespace {

using AstPointerTest = TestHelper;

TEST_F(AstPointerTest, Creation) {
  auto* i32 = create<I32>();
  auto* p = create<Pointer>(i32, ast::StorageClass::kStorage);
  EXPECT_EQ(p->type(), i32);
  EXPECT_EQ(p->storage_class(), ast::StorageClass::kStorage);
}

TEST_F(AstPointerTest, TypeName) {
  auto* i32 = create<I32>();
  auto* p = create<Pointer>(i32, ast::StorageClass::kWorkgroup);
  EXPECT_EQ(p->type_name(), "__ptr_workgroup__i32");
}

TEST_F(AstPointerTest, FriendlyNameWithStorageClass) {
  auto* i32 = create<I32>();
  auto* p = create<Pointer>(i32, ast::StorageClass::kWorkgroup);
  EXPECT_EQ(p->FriendlyName(Symbols()), "ptr<workgroup, i32>");
}

TEST_F(AstPointerTest, FriendlyNameWithoutStorageClass) {
  auto* i32 = create<I32>();
  auto* p = create<Pointer>(i32, ast::StorageClass::kNone);
  EXPECT_EQ(p->FriendlyName(Symbols()), "ptr<i32>");
}

}  // namespace
}  // namespace ast
}  // namespace tint
