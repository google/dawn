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
#include "src/ast/type/i32_type.h"
#include "src/ast/type/u32_type.h"

namespace tint {
namespace ast {
namespace type {
namespace {

using ArrayTypeTest = TestHelper;

TEST_F(ArrayTypeTest, CreateSizedArray) {
  U32Type u32;
  ArrayType arr{&u32, 3};
  EXPECT_EQ(arr.type(), &u32);
  EXPECT_EQ(arr.size(), 3u);
  EXPECT_TRUE(arr.IsArray());
  EXPECT_FALSE(arr.IsRuntimeArray());
}

TEST_F(ArrayTypeTest, CreateRuntimeArray) {
  U32Type u32;
  ArrayType arr{&u32};
  EXPECT_EQ(arr.type(), &u32);
  EXPECT_EQ(arr.size(), 0u);
  EXPECT_TRUE(arr.IsArray());
  EXPECT_TRUE(arr.IsRuntimeArray());
}

TEST_F(ArrayTypeTest, Is) {
  I32Type i32;

  ArrayType arr{&i32, 3};
  EXPECT_FALSE(arr.IsAccessControl());
  EXPECT_FALSE(arr.IsAlias());
  EXPECT_TRUE(arr.IsArray());
  EXPECT_FALSE(arr.IsBool());
  EXPECT_FALSE(arr.IsF32());
  EXPECT_FALSE(arr.IsI32());
  EXPECT_FALSE(arr.IsMatrix());
  EXPECT_FALSE(arr.IsPointer());
  EXPECT_FALSE(arr.IsSampler());
  EXPECT_FALSE(arr.IsStruct());
  EXPECT_FALSE(arr.IsTexture());
  EXPECT_FALSE(arr.IsU32());
  EXPECT_FALSE(arr.IsVector());
}

TEST_F(ArrayTypeTest, TypeName) {
  I32Type i32;
  ArrayType arr{&i32};
  EXPECT_EQ(arr.type_name(), "__array__i32");
}

TEST_F(ArrayTypeTest, TypeName_RuntimeArray) {
  I32Type i32;
  ArrayType arr{&i32, 3};
  EXPECT_EQ(arr.type_name(), "__array__i32_3");
}

TEST_F(ArrayTypeTest, TypeName_WithStride) {
  I32Type i32;
  ArrayDecorationList decos;
  decos.push_back(std::make_unique<StrideDecoration>(16, Source{}));

  ArrayType arr{&i32, 3};
  arr.set_decorations(std::move(decos));
  EXPECT_EQ(arr.type_name(), "__array__i32_3_stride_16");
}

TEST_F(ArrayTypeTest, MinBufferBindingSizeNoStride) {
  U32Type u32;
  ArrayType arr(&u32, 4);
  EXPECT_EQ(0u, arr.MinBufferBindingSize(MemoryLayout::kUniformBuffer));
}

TEST_F(ArrayTypeTest, MinBufferBindingSizeArray) {
  U32Type u32;
  ArrayDecorationList decos;
  decos.push_back(std::make_unique<StrideDecoration>(4, Source{}));

  ArrayType arr(&u32, 4);
  arr.set_decorations(std::move(decos));
  EXPECT_EQ(16u, arr.MinBufferBindingSize(MemoryLayout::kUniformBuffer));
}

TEST_F(ArrayTypeTest, MinBufferBindingSizeRuntimeArray) {
  U32Type u32;
  ArrayDecorationList decos;
  decos.push_back(std::make_unique<StrideDecoration>(4, Source{}));

  ArrayType arr(&u32);
  arr.set_decorations(std::move(decos));
  EXPECT_EQ(4u, arr.MinBufferBindingSize(MemoryLayout::kUniformBuffer));
}

TEST_F(ArrayTypeTest, BaseAlignmentArray) {
  U32Type u32;
  ArrayDecorationList decos;
  decos.push_back(std::make_unique<StrideDecoration>(4, Source{}));

  ArrayType arr(&u32, 4);
  arr.set_decorations(std::move(decos));
  EXPECT_EQ(16u, arr.BaseAlignment(MemoryLayout::kUniformBuffer));
  EXPECT_EQ(4u, arr.BaseAlignment(MemoryLayout::kStorageBuffer));
}

TEST_F(ArrayTypeTest, BaseAlignmentRuntimeArray) {
  U32Type u32;
  ArrayDecorationList decos;
  decos.push_back(std::make_unique<StrideDecoration>(4, Source{}));

  ArrayType arr(&u32);
  arr.set_decorations(std::move(decos));
  EXPECT_EQ(16u, arr.BaseAlignment(MemoryLayout::kUniformBuffer));
  EXPECT_EQ(4u, arr.BaseAlignment(MemoryLayout::kStorageBuffer));
}

}  // namespace
}  // namespace type
}  // namespace ast
}  // namespace tint
