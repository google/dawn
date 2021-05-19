// Copyright 2021 The Tint Authors.
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

#include "src/sem/reference_type.h"
#include "src/sem/test_helper.h"

namespace tint {
namespace sem {
namespace {

using ReferenceTest = TestHelper;

TEST_F(ReferenceTest, Creation) {
  auto* r = create<Reference>(create<I32>(), ast::StorageClass::kStorage);
  EXPECT_TRUE(r->StoreType()->Is<sem::I32>());
  EXPECT_EQ(r->StorageClass(), ast::StorageClass::kStorage);
}

TEST_F(ReferenceTest, TypeName) {
  auto* r = create<Reference>(create<I32>(), ast::StorageClass::kWorkgroup);
  EXPECT_EQ(r->type_name(), "__ref_workgroup__i32");
}

TEST_F(ReferenceTest, FriendlyNameWithStorageClass) {
  auto* r = create<Reference>(create<I32>(), ast::StorageClass::kWorkgroup);
  EXPECT_EQ(r->FriendlyName(Symbols()), "ref<workgroup, i32>");
}

TEST_F(ReferenceTest, FriendlyNameWithoutStorageClass) {
  auto* r = create<Reference>(create<I32>(), ast::StorageClass::kNone);
  EXPECT_EQ(r->FriendlyName(Symbols()), "ref<i32>");
}

}  // namespace
}  // namespace sem
}  // namespace tint
