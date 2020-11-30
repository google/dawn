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

#include "src/ast/type/u32_type.h"

#include "src/ast/test_helper.h"
#include "src/ast/type/access_control_type.h"
#include "src/ast/type/array_type.h"
#include "src/ast/type/bool_type.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/i32_type.h"
#include "src/ast/type/matrix_type.h"
#include "src/ast/type/pointer_type.h"
#include "src/ast/type/sampler_type.h"

namespace tint {
namespace ast {
namespace type {
namespace {

using U32TypeTest = TestHelper;

TEST_F(U32TypeTest, Is) {
  U32Type u;
  Type* ty = &u;
  EXPECT_FALSE(ty->Is<AccessControlType>());
  EXPECT_FALSE(ty->Is<AliasType>());
  EXPECT_FALSE(ty->Is<ArrayType>());
  EXPECT_FALSE(ty->Is<BoolType>());
  EXPECT_FALSE(ty->Is<F32Type>());
  EXPECT_FALSE(ty->Is<I32Type>());
  EXPECT_FALSE(ty->Is<MatrixType>());
  EXPECT_FALSE(ty->Is<PointerType>());
  EXPECT_FALSE(ty->Is<SamplerType>());
  EXPECT_FALSE(ty->IsStruct());
  EXPECT_FALSE(ty->IsTexture());
  EXPECT_TRUE(ty->IsU32());
  EXPECT_FALSE(ty->IsVector());
}

TEST_F(U32TypeTest, TypeName) {
  U32Type u;
  EXPECT_EQ(u.type_name(), "__u32");
}

TEST_F(U32TypeTest, MinBufferBindingSize) {
  U32Type u;
  EXPECT_EQ(4u, u.MinBufferBindingSize(MemoryLayout::kUniformBuffer));
}

TEST_F(U32TypeTest, BaseAlignment) {
  U32Type u;
  EXPECT_EQ(4u, u.BaseAlignment(MemoryLayout::kUniformBuffer));
}

}  // namespace
}  // namespace type
}  // namespace ast
}  // namespace tint
