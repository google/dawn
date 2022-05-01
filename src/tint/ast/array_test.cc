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

#include "src/tint/ast/array.h"

#include "src/tint/ast/test_helper.h"

namespace tint::ast {
namespace {

using AstArrayTest = TestHelper;

TEST_F(AstArrayTest, CreateSizedArray) {
    auto* u32 = create<U32>();
    auto* count = Expr(3);
    auto* arr = create<Array>(u32, count, AttributeList{});
    EXPECT_EQ(arr->type, u32);
    EXPECT_EQ(arr->count, count);
    EXPECT_TRUE(arr->Is<Array>());
    EXPECT_FALSE(arr->IsRuntimeArray());
}

TEST_F(AstArrayTest, CreateRuntimeArray) {
    auto* u32 = create<U32>();
    auto* arr = create<Array>(u32, nullptr, AttributeList{});
    EXPECT_EQ(arr->type, u32);
    EXPECT_EQ(arr->count, nullptr);
    EXPECT_TRUE(arr->Is<Array>());
    EXPECT_TRUE(arr->IsRuntimeArray());
}

TEST_F(AstArrayTest, FriendlyName_RuntimeSized) {
    auto* i32 = create<I32>();
    auto* arr = create<Array>(i32, nullptr, AttributeList{});
    EXPECT_EQ(arr->FriendlyName(Symbols()), "array<i32>");
}

TEST_F(AstArrayTest, FriendlyName_LiteralSized) {
    auto* i32 = create<I32>();
    auto* arr = create<Array>(i32, Expr(5), AttributeList{});
    EXPECT_EQ(arr->FriendlyName(Symbols()), "array<i32, 5>");
}

TEST_F(AstArrayTest, FriendlyName_ConstantSized) {
    auto* i32 = create<I32>();
    auto* arr = create<Array>(i32, Expr("size"), AttributeList{});
    EXPECT_EQ(arr->FriendlyName(Symbols()), "array<i32, size>");
}

TEST_F(AstArrayTest, FriendlyName_WithStride) {
    auto* i32 = create<I32>();
    auto* arr = create<Array>(i32, Expr(5), AttributeList{create<StrideAttribute>(32)});
    EXPECT_EQ(arr->FriendlyName(Symbols()), "@stride(32) array<i32, 5>");
}

}  // namespace
}  // namespace tint::ast
