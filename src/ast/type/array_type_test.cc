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

#include "src/ast/type/array_type.h"

#include <memory>
#include <utility>

#include "src/ast/stride_decoration.h"
#include "src/ast/test_helper.h"
#include "src/ast/type/access_control_type.h"
#include "src/ast/type/bool_type.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/i32_type.h"
#include "src/ast/type/matrix_type.h"
#include "src/ast/type/pointer_type.h"
#include "src/ast/type/struct_type.h"
#include "src/ast/type/texture_type.h"
#include "src/ast/type/u32_type.h"
#include "src/ast/type/vector_type.h"

namespace tint {
namespace ast {
namespace type {
namespace {

using ArrayTest = TestHelper;

TEST_F(ArrayTest, CreateSizedArray) {
  U32 u32;
  Array arr{&u32, 3};
  EXPECT_EQ(arr.type(), &u32);
  EXPECT_EQ(arr.size(), 3u);
  EXPECT_TRUE(arr.Is<Array>());
  EXPECT_FALSE(arr.IsRuntimeArray());
}

TEST_F(ArrayTest, CreateRuntimeArray) {
  U32 u32;
  Array arr{&u32};
  EXPECT_EQ(arr.type(), &u32);
  EXPECT_EQ(arr.size(), 0u);
  EXPECT_TRUE(arr.Is<Array>());
  EXPECT_TRUE(arr.IsRuntimeArray());
}

TEST_F(ArrayTest, Is) {
  I32 i32;

  Array arr{&i32, 3};
  Type* ty = &arr;
  EXPECT_FALSE(ty->Is<AccessControl>());
  EXPECT_FALSE(ty->Is<Alias>());
  EXPECT_TRUE(ty->Is<Array>());
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
  Array arr{&i32};
  EXPECT_EQ(arr.type_name(), "__array__i32");
}

TEST_F(ArrayTest, TypeName_RuntimeArray) {
  I32 i32;
  Array arr{&i32, 3};
  EXPECT_EQ(arr.type_name(), "__array__i32_3");
}

TEST_F(ArrayTest, TypeName_WithStride) {
  I32 i32;
  ArrayDecorationList decos;
  decos.push_back(create<StrideDecoration>(16, Source{}));

  Array arr{&i32, 3};
  arr.set_decorations(decos);
  EXPECT_EQ(arr.type_name(), "__array__i32_3_stride_16");
}

TEST_F(ArrayTest, MinBufferBindingSizeNoStride) {
  U32 u32;
  Array arr(&u32, 4);
  EXPECT_EQ(0u, arr.MinBufferBindingSize(MemoryLayout::kUniformBuffer));
}

TEST_F(ArrayTest, MinBufferBindingSizeArray) {
  U32 u32;
  ArrayDecorationList decos;
  decos.push_back(create<StrideDecoration>(4, Source{}));

  Array arr(&u32, 4);
  arr.set_decorations(decos);
  EXPECT_EQ(16u, arr.MinBufferBindingSize(MemoryLayout::kUniformBuffer));
}

TEST_F(ArrayTest, MinBufferBindingSizeRuntimeArray) {
  U32 u32;
  ArrayDecorationList decos;
  decos.push_back(create<StrideDecoration>(4, Source{}));

  Array arr(&u32);
  arr.set_decorations(decos);
  EXPECT_EQ(4u, arr.MinBufferBindingSize(MemoryLayout::kUniformBuffer));
}

TEST_F(ArrayTest, BaseAlignmentArray) {
  U32 u32;
  ArrayDecorationList decos;
  decos.push_back(create<StrideDecoration>(4, Source{}));

  Array arr(&u32, 4);
  arr.set_decorations(decos);
  EXPECT_EQ(16u, arr.BaseAlignment(MemoryLayout::kUniformBuffer));
  EXPECT_EQ(4u, arr.BaseAlignment(MemoryLayout::kStorageBuffer));
}

TEST_F(ArrayTest, BaseAlignmentRuntimeArray) {
  U32 u32;
  ArrayDecorationList decos;
  decos.push_back(create<StrideDecoration>(4, Source{}));

  Array arr(&u32);
  arr.set_decorations(decos);
  EXPECT_EQ(16u, arr.BaseAlignment(MemoryLayout::kUniformBuffer));
  EXPECT_EQ(4u, arr.BaseAlignment(MemoryLayout::kStorageBuffer));
}

}  // namespace
}  // namespace type
}  // namespace ast
}  // namespace tint
