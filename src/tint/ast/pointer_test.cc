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

#include "src/tint/ast/pointer.h"

#include "src/tint/ast/i32.h"
#include "src/tint/ast/test_helper.h"

namespace tint::ast {
namespace {

using AstPointerTest = TestHelper;

TEST_F(AstPointerTest, Creation) {
    auto* i32 = create<I32>();
    auto* p = create<Pointer>(i32, ast::StorageClass::kStorage, Access::kRead);
    EXPECT_EQ(p->type, i32);
    EXPECT_EQ(p->storage_class, ast::StorageClass::kStorage);
    EXPECT_EQ(p->access, Access::kRead);
}

TEST_F(AstPointerTest, FriendlyName) {
    auto* i32 = create<I32>();
    auto* p = create<Pointer>(i32, ast::StorageClass::kWorkgroup, Access::kUndefined);
    EXPECT_EQ(p->FriendlyName(Symbols()), "ptr<workgroup, i32>");
}

TEST_F(AstPointerTest, FriendlyNameWithAccess) {
    auto* i32 = create<I32>();
    auto* p = create<Pointer>(i32, ast::StorageClass::kStorage, Access::kReadWrite);
    EXPECT_EQ(p->FriendlyName(Symbols()), "ptr<storage, i32, read_write>");
}

}  // namespace
}  // namespace tint::ast
