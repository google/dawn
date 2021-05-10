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

#include "src/ast/matrix.h"
#include "src/ast/access_control.h"
#include "src/ast/alias.h"
#include "src/ast/array.h"
#include "src/ast/bool.h"
#include "src/ast/f32.h"
#include "src/ast/i32.h"
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

using AstMatrixTest = TestHelper;

TEST_F(AstMatrixTest, Creation) {
  auto* i32 = create<I32>();
  auto* m = create<Matrix>(i32, 2, 4);
  EXPECT_EQ(m->type(), i32);
  EXPECT_EQ(m->rows(), 2u);
  EXPECT_EQ(m->columns(), 4u);
}

TEST_F(AstMatrixTest, TypeName) {
  auto* i32 = create<I32>();
  auto* m = create<Matrix>(i32, 2, 3);
  EXPECT_EQ(m->type_name(), "__mat_2_3__i32");
}

TEST_F(AstMatrixTest, FriendlyName) {
  auto* i32 = create<I32>();
  auto* m = create<Matrix>(i32, 3, 2);
  EXPECT_EQ(m->FriendlyName(Symbols()), "mat2x3<i32>");
}

}  // namespace
}  // namespace ast
}  // namespace tint
