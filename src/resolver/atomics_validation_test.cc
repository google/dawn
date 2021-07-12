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

#include "src/resolver/resolver.h"
#include "src/resolver/resolver_test_helper.h"
#include "src/sem/atomic_type.h"
#include "src/sem/reference_type.h"

#include "gmock/gmock.h"

namespace tint {
namespace resolver {
namespace {

struct ResolverAtomicValidationTest : public resolver::TestHelper,
                                      public testing::Test {};

TEST_F(ResolverAtomicValidationTest, GlobalOfInvalidType) {
  Global("a", ty.atomic(ty.f32(Source{{12, 34}})),
         ast::StorageClass::kWorkgroup);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(), "12:34 error: atomic only supports i32 or u32 types");
}

TEST_F(ResolverAtomicValidationTest, GlobalOfInvalidStorageClass) {
  Global("a", ty.atomic(Source{{12, 34}}, ty.i32()),
         ast::StorageClass::kPrivate);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(), "12:34 error: atomic var requires workgroup storage");
}

TEST_F(ResolverAtomicValidationTest, GlobalPrivateStruct) {
  auto* s =
      Structure("s", {Member("a", ty.atomic(Source{{12, 34}}, ty.i32()))});
  Global("g", ty.Of(s), ast::StorageClass::kPrivate);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: atomic types can only be used in storage classes "
            "workgroup or storage, but was used by storage class private");
}

// TODO(crbug.com/tint/901): Validate that storage buffer access mode is
// read_write.

TEST_F(ResolverAtomicValidationTest, Local) {
  WrapInFunction(Var("a", ty.atomic(Source{{12, 34}}, ty.i32())));

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(), "12:34 error: atomic var requires workgroup storage");
}

TEST_F(ResolverAtomicValidationTest, NoAtomicExpr) {
  WrapInFunction(Construct(Source{{12, 34}}, ty.atomic<u32>()));

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: an expression must not evaluate to an atomic type");
}

}  // namespace
}  // namespace resolver
}  // namespace tint
