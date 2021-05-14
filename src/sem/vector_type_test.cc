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

#include "src/sem/test_helper.h"
#include "src/sem/texture_type.h"

namespace tint {
namespace sem {
namespace {

using VectorTest = TestHelper;

TEST_F(VectorTest, Creation) {
  I32 i32;
  Vector v{&i32, 2};
  EXPECT_EQ(v.type(), &i32);
  EXPECT_EQ(v.size(), 2u);
}

TEST_F(VectorTest, TypeName) {
  auto* i32 = create<I32>();
  auto* v = create<Vector>(i32, 3);
  EXPECT_EQ(v->type_name(), "__vec_3__i32");
}

TEST_F(VectorTest, FriendlyName) {
  auto* f32 = create<F32>();
  auto* v = create<Vector>(f32, 3);
  EXPECT_EQ(v->FriendlyName(Symbols()), "vec3<f32>");
}

}  // namespace
}  // namespace sem
}  // namespace tint
