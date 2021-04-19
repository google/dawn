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

#include "src/type/access_control_type.h"
#include "src/type/test_helper.h"
#include "src/type/texture_type.h"

namespace tint {
namespace type {
namespace {

using ArrayTest = TestHelper;

TEST_F(ArrayTest, CreateSizedArray) {
  U32 u32;
  ArrayType arr{&u32, 3, ast::DecorationList{}};
  EXPECT_EQ(arr.type(), &u32);
  EXPECT_EQ(arr.size(), 3u);
  EXPECT_TRUE(arr.Is<ArrayType>());
  EXPECT_FALSE(arr.IsRuntimeArray());
}

TEST_F(ArrayTest, CreateRuntimeArray) {
  U32 u32;
  ArrayType arr{&u32, 0, ast::DecorationList{}};
  EXPECT_EQ(arr.type(), &u32);
  EXPECT_EQ(arr.size(), 0u);
  EXPECT_TRUE(arr.Is<ArrayType>());
  EXPECT_TRUE(arr.IsRuntimeArray());
}

TEST_F(ArrayTest, Is) {
  I32 i32;

  ArrayType arr{&i32, 3, ast::DecorationList{}};
  Type* ty = &arr;
  EXPECT_FALSE(ty->Is<AccessControl>());
  EXPECT_FALSE(ty->Is<Alias>());
  EXPECT_TRUE(ty->Is<ArrayType>());
  EXPECT_FALSE(ty->Is<Bool>());
  EXPECT_FALSE(ty->Is<F32>());
  EXPECT_FALSE(ty->Is<I32>());
  EXPECT_FALSE(ty->Is<Matrix>());
  EXPECT_FALSE(ty->Is<Pointer>());
  EXPECT_FALSE(ty->Is<Sampler>());
  EXPECT_FALSE(ty->Is<Struct>());
  EXPECT_FALSE(ty->Is<Texture>());
  EXPECT_FALSE(ty->Is<U32>());
  EXPECT_FALSE(ty->Is<Vector>());
}

TEST_F(ArrayTest, TypeName) {
  I32 i32;
  ArrayType arr{&i32, 0, ast::DecorationList{}};
  EXPECT_EQ(arr.type_name(), "__array__i32");
}

TEST_F(ArrayTest, FriendlyNameRuntimeSized) {
  ArrayType arr{ty.i32(), 0, ast::DecorationList{}};
  EXPECT_EQ(arr.FriendlyName(Symbols()), "array<i32>");
}

TEST_F(ArrayTest, FriendlyNameStaticSized) {
  ArrayType arr{ty.i32(), 5, ast::DecorationList{}};
  EXPECT_EQ(arr.FriendlyName(Symbols()), "array<i32, 5>");
}

TEST_F(ArrayTest, FriendlyNameWithStride) {
  ArrayType arr{ty.i32(), 5,
                ast::DecorationList{create<ast::StrideDecoration>(32)}};
  EXPECT_EQ(arr.FriendlyName(Symbols()), "[[stride(32)]] array<i32, 5>");
}

TEST_F(ArrayTest, TypeName_RuntimeArray) {
  I32 i32;
  ArrayType arr{&i32, 3, ast::DecorationList{}};
  EXPECT_EQ(arr.type_name(), "__array__i32_3");
}

TEST_F(ArrayTest, TypeName_WithStride) {
  I32 i32;
  ArrayType arr{&i32, 3,
                ast::DecorationList{create<ast::StrideDecoration>(16)}};
  EXPECT_EQ(arr.type_name(), "__array__i32_3_stride_16");
}

}  // namespace
}  // namespace type
}  // namespace tint
