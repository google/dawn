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

#include "src/ast/array.h"
#include "src/ast/access_control.h"
#include "src/ast/alias.h"
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

using AstArrayTest = TestHelper;

TEST_F(AstArrayTest, CreateSizedArray) {
  auto* u32 = create<U32>();
  auto* arr = create<Array>(u32, 3, DecorationList{});
  EXPECT_EQ(arr->type(), u32);
  EXPECT_EQ(arr->size(), 3u);
  EXPECT_TRUE(arr->Is<Array>());
  EXPECT_FALSE(arr->IsRuntimeArray());
}

TEST_F(AstArrayTest, CreateRuntimeArray) {
  auto* u32 = create<U32>();
  auto* arr = create<Array>(u32, 0, DecorationList{});
  EXPECT_EQ(arr->type(), u32);
  EXPECT_EQ(arr->size(), 0u);
  EXPECT_TRUE(arr->Is<Array>());
  EXPECT_TRUE(arr->IsRuntimeArray());
}

TEST_F(AstArrayTest, TypeName) {
  auto* i32 = create<I32>();
  auto* arr = create<Array>(i32, 0, DecorationList{});
  EXPECT_EQ(arr->type_name(), "__array__i32");
}

TEST_F(AstArrayTest, FriendlyNameRuntimeSized) {
  auto* i32 = create<I32>();
  auto* arr = create<Array>(i32, 0, DecorationList{});
  EXPECT_EQ(arr->FriendlyName(Symbols()), "array<i32>");
}

TEST_F(AstArrayTest, FriendlyNameStaticSized) {
  auto* i32 = create<I32>();
  auto* arr = create<Array>(i32, 5, DecorationList{});
  EXPECT_EQ(arr->FriendlyName(Symbols()), "array<i32, 5>");
}

TEST_F(AstArrayTest, FriendlyNameWithStride) {
  auto* i32 = create<I32>();
  auto* arr =
      create<Array>(i32, 5, DecorationList{create<StrideDecoration>(32)});
  EXPECT_EQ(arr->FriendlyName(Symbols()), "[[stride(32)]] array<i32, 5>");
}

TEST_F(AstArrayTest, TypeName_RuntimeArray) {
  auto* i32 = create<I32>();
  auto* arr = create<Array>(i32, 3, DecorationList{});
  EXPECT_EQ(arr->type_name(), "__array__i32_3");
}

TEST_F(AstArrayTest, TypeName_WithStride) {
  auto* i32 = create<I32>();
  auto* arr =
      create<Array>(i32, 3, DecorationList{create<StrideDecoration>(16)});
  EXPECT_EQ(arr->type_name(), "__array__i32_3_stride_16");
}

}  // namespace
}  // namespace ast
}  // namespace tint
