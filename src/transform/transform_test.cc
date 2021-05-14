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

#include "src/transform/transform.h"
#include "src/clone_context.h"
#include "src/program_builder.h"

#include "gtest/gtest.h"

namespace tint {
namespace transform {
namespace {

// Inherit from Transform so we have access to protected methods
struct CreateASTTypeForTest : public testing::Test, public Transform {
  Output Run(const Program*, const DataMap&) override { return {}; }

  ast::Type* create(
      std::function<sem::Type*(ProgramBuilder&)> create_sem_type) {
    ProgramBuilder sem_type_builder;
    auto* sem_type = create_sem_type(sem_type_builder);
    Program program(std::move(sem_type_builder));
    CloneContext ctx(&ast_type_builder, &program, false);
    return CreateASTTypeFor(&ctx, sem_type);
  }

  ProgramBuilder ast_type_builder;
};

TEST_F(CreateASTTypeForTest, Basic) {
  EXPECT_TRUE(create([](ProgramBuilder& b) {
                return b.create<sem::I32>();
              })->Is<ast::I32>());
  EXPECT_TRUE(create([](ProgramBuilder& b) {
                return b.create<sem::U32>();
              })->Is<ast::U32>());
  EXPECT_TRUE(create([](ProgramBuilder& b) {
                return b.create<sem::F32>();
              })->Is<ast::F32>());
  EXPECT_TRUE(create([](ProgramBuilder& b) {
                return b.create<sem::Bool>();
              })->Is<ast::Bool>());
  EXPECT_TRUE(create([](ProgramBuilder& b) {
                return b.create<sem::Void>();
              })->Is<ast::Void>());
}

TEST_F(CreateASTTypeForTest, Matrix) {
  auto* mat = create([](ProgramBuilder& b) {
    auto* column_type = b.create<sem::Vector>(b.create<sem::F32>(), 2u);
    return b.create<sem::Matrix>(column_type, 3u);
  });
  ASSERT_TRUE(mat->Is<ast::Matrix>());
  ASSERT_TRUE(mat->As<ast::Matrix>()->type()->Is<ast::F32>());
  ASSERT_EQ(mat->As<ast::Matrix>()->columns(), 3u);
  ASSERT_EQ(mat->As<ast::Matrix>()->rows(), 2u);
}

TEST_F(CreateASTTypeForTest, Vector) {
  auto* vec = create([](ProgramBuilder& b) {
    return b.create<sem::Vector>(b.create<sem::F32>(), 2);
  });
  ASSERT_TRUE(vec->Is<ast::Vector>());
  ASSERT_TRUE(vec->As<ast::Vector>()->type()->Is<ast::F32>());
  ASSERT_EQ(vec->As<ast::Vector>()->size(), 2u);
}

TEST_F(CreateASTTypeForTest, ArrayImplicitStride) {
  auto* arr = create([](ProgramBuilder& b) {
    return b.create<sem::Array>(b.create<sem::F32>(), 2, 4, 4, 32u, true);
  });
  ASSERT_TRUE(arr->Is<ast::Array>());
  ASSERT_TRUE(arr->As<ast::Array>()->type()->Is<ast::F32>());
  ASSERT_EQ(arr->As<ast::Array>()->size(), 2u);
  ASSERT_EQ(arr->As<ast::Array>()->decorations().size(), 0u);
}

TEST_F(CreateASTTypeForTest, ArrayNonImplicitStride) {
  auto* arr = create([](ProgramBuilder& b) {
    return b.create<sem::Array>(b.create<sem::F32>(), 2, 4, 4, 32u, false);
  });
  ASSERT_TRUE(arr->Is<ast::Array>());
  ASSERT_TRUE(arr->As<ast::Array>()->type()->Is<ast::F32>());
  ASSERT_EQ(arr->As<ast::Array>()->size(), 2u);
  ASSERT_EQ(arr->As<ast::Array>()->decorations().size(), 1u);
  ASSERT_TRUE(
      arr->As<ast::Array>()->decorations()[0]->Is<ast::StrideDecoration>());
  ASSERT_EQ(arr->As<ast::Array>()
                ->decorations()[0]
                ->As<ast::StrideDecoration>()
                ->stride(),
            32u);
}

TEST_F(CreateASTTypeForTest, Struct) {
  auto* str = create([](ProgramBuilder& b) {
    auto* decl = b.Structure("S", {}, {});
    return b.create<sem::Struct>(decl, sem::StructMemberList{}, 4 /* align */,
                                 4 /* size */, 4 /* size_no_padding */);
  });
  ASSERT_TRUE(str->Is<ast::TypeName>());
  EXPECT_EQ(
      ast_type_builder.Symbols().NameFor(str->As<ast::TypeName>()->name()),
      "S");
}

}  // namespace
}  // namespace transform
}  // namespace tint
