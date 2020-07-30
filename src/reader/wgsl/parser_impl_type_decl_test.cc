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

#include "gtest/gtest.h"
#include "src/ast/type/alias_type.h"
#include "src/ast/type/array_type.h"
#include "src/ast/type/bool_type.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/i32_type.h"
#include "src/ast/type/matrix_type.h"
#include "src/ast/type/pointer_type.h"
#include "src/ast/type/struct_type.h"
#include "src/ast/type/u32_type.h"
#include "src/ast/type/vector_type.h"
#include "src/reader/wgsl/parser_impl.h"
#include "src/reader/wgsl/parser_impl_test_helper.h"
#include "src/type_manager.h"

namespace tint {
namespace reader {
namespace wgsl {
namespace {

TEST_F(ParserImplTest, TypeDecl_Invalid) {
  auto* p = parser("1234");
  auto* t = p->type_decl();
  EXPECT_EQ(t, nullptr);
  EXPECT_FALSE(p->has_error());
}

TEST_F(ParserImplTest, TypeDecl_Identifier) {
  auto* p = parser("A");

  auto* int_type = tm()->Get(std::make_unique<ast::type::I32Type>());
  // Pre-register to make sure that it's the same type.
  auto* alias_type =
      tm()->Get(std::make_unique<ast::type::AliasType>("A", int_type));

  p->register_alias("A", alias_type);

  auto* t = p->type_decl();
  ASSERT_NE(t, nullptr);
  EXPECT_EQ(t, alias_type);
  ASSERT_TRUE(t->IsAlias());

  auto* alias = t->AsAlias();
  EXPECT_EQ(alias->name(), "A");
  EXPECT_EQ(alias->type(), int_type);
}

TEST_F(ParserImplTest, TypeDecl_Identifier_NotFound) {
  auto* p = parser("B");

  auto* t = p->type_decl();
  ASSERT_EQ(t, nullptr);
  EXPECT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:1: unknown type alias 'B'");
}

TEST_F(ParserImplTest, TypeDecl_Bool) {
  auto* p = parser("bool");

  auto* bool_type = tm()->Get(std::make_unique<ast::type::BoolType>());

  auto* t = p->type_decl();
  ASSERT_NE(t, nullptr);
  EXPECT_EQ(t, bool_type);
  ASSERT_TRUE(t->IsBool());
}

TEST_F(ParserImplTest, TypeDecl_F32) {
  auto* p = parser("f32");

  auto* float_type = tm()->Get(std::make_unique<ast::type::F32Type>());

  auto* t = p->type_decl();
  ASSERT_NE(t, nullptr);
  EXPECT_EQ(t, float_type);
  ASSERT_TRUE(t->IsF32());
}

TEST_F(ParserImplTest, TypeDecl_I32) {
  auto* p = parser("i32");

  auto* int_type = tm()->Get(std::make_unique<ast::type::I32Type>());

  auto* t = p->type_decl();
  ASSERT_NE(t, nullptr);
  EXPECT_EQ(t, int_type);
  ASSERT_TRUE(t->IsI32());
}

TEST_F(ParserImplTest, TypeDecl_U32) {
  auto* p = parser("u32");

  auto* uint_type = tm()->Get(std::make_unique<ast::type::U32Type>());

  auto* t = p->type_decl();
  ASSERT_NE(t, nullptr);
  EXPECT_EQ(t, uint_type);
  ASSERT_TRUE(t->IsU32());
}

struct VecData {
  const char* input;
  size_t count;
};
inline std::ostream& operator<<(std::ostream& out, VecData data) {
  out << std::string(data.input);
  return out;
}
class VecTest : public testing::TestWithParam<VecData> {
 public:
  VecTest() = default;
  ~VecTest() override = default;

  void SetUp() override { ctx_.Reset(); }

  void TearDown() override { impl_ = nullptr; }

  ParserImpl* parser(const std::string& str) {
    impl_ = std::make_unique<ParserImpl>(&ctx_, str);
    return impl_.get();
  }

 private:
  std::unique_ptr<ParserImpl> impl_;
  Context ctx_;
};

TEST_P(VecTest, Parse) {
  auto params = GetParam();
  auto* p = parser(params.input);
  auto* t = p->type_decl();
  ASSERT_NE(t, nullptr);
  ASSERT_FALSE(p->has_error());
  EXPECT_TRUE(t->IsVector());
  EXPECT_EQ(t->AsVector()->size(), params.count);
}
INSTANTIATE_TEST_SUITE_P(ParserImplTest,
                         VecTest,
                         testing::Values(VecData{"vec2<f32>", 2},
                                         VecData{"vec3<f32>", 3},
                                         VecData{"vec4<f32>", 4}));

class VecMissingGreaterThanTest : public testing::TestWithParam<VecData> {
 public:
  VecMissingGreaterThanTest() = default;
  ~VecMissingGreaterThanTest() override = default;

  void SetUp() override { ctx_.Reset(); }

  void TearDown() override { impl_ = nullptr; }

  ParserImpl* parser(const std::string& str) {
    impl_ = std::make_unique<ParserImpl>(&ctx_, str);
    return impl_.get();
  }

 private:
  std::unique_ptr<ParserImpl> impl_;
  Context ctx_;
};

TEST_P(VecMissingGreaterThanTest, Handles_Missing_GreaterThan) {
  auto params = GetParam();
  auto* p = parser(params.input);
  auto* t = p->type_decl();
  ASSERT_EQ(t, nullptr);
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(p->error(), "1:9: missing > for vector");
}
INSTANTIATE_TEST_SUITE_P(ParserImplTest,
                         VecMissingGreaterThanTest,
                         testing::Values(VecData{"vec2<f32", 2},
                                         VecData{"vec3<f32", 3},
                                         VecData{"vec4<f32", 4}));

class VecMissingLessThanTest : public testing::TestWithParam<VecData> {
 public:
  VecMissingLessThanTest() = default;
  ~VecMissingLessThanTest() override = default;

  void SetUp() override { ctx_.Reset(); }

  void TearDown() override { impl_ = nullptr; }

  ParserImpl* parser(const std::string& str) {
    impl_ = std::make_unique<ParserImpl>(&ctx_, str);
    return impl_.get();
  }

 private:
  std::unique_ptr<ParserImpl> impl_;
  Context ctx_;
};

TEST_P(VecMissingLessThanTest, Handles_Missing_GreaterThan) {
  auto params = GetParam();
  auto* p = parser(params.input);
  auto* t = p->type_decl();
  ASSERT_EQ(t, nullptr);
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(p->error(), "1:5: missing < for vector");
}
INSTANTIATE_TEST_SUITE_P(ParserImplTest,
                         VecMissingLessThanTest,
                         testing::Values(VecData{"vec2", 2},
                                         VecData{"vec3", 3},
                                         VecData{"vec4", 4}));

class VecBadType : public testing::TestWithParam<VecData> {
 public:
  VecBadType() = default;
  ~VecBadType() override = default;

  void SetUp() override { ctx_.Reset(); }

  void TearDown() override { impl_ = nullptr; }

  ParserImpl* parser(const std::string& str) {
    impl_ = std::make_unique<ParserImpl>(&ctx_, str);
    return impl_.get();
  }

 private:
  std::unique_ptr<ParserImpl> impl_;
  Context ctx_;
};

TEST_P(VecBadType, Handles_Unknown_Type) {
  auto params = GetParam();
  auto* p = parser(params.input);
  auto* t = p->type_decl();
  ASSERT_EQ(t, nullptr);
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(p->error(), "1:6: unknown type alias 'unknown'");
}
INSTANTIATE_TEST_SUITE_P(ParserImplTest,
                         VecBadType,
                         testing::Values(VecData{"vec2<unknown", 2},
                                         VecData{"vec3<unknown", 3},
                                         VecData{"vec4<unknown", 4}));

class VecMissingType : public testing::TestWithParam<VecData> {
 public:
  VecMissingType() = default;
  ~VecMissingType() override = default;

  void SetUp() override { ctx_.Reset(); }

  void TearDown() override { impl_ = nullptr; }

  ParserImpl* parser(const std::string& str) {
    impl_ = std::make_unique<ParserImpl>(&ctx_, str);
    return impl_.get();
  }

 private:
  std::unique_ptr<ParserImpl> impl_;
  Context ctx_;
};

TEST_P(VecMissingType, Handles_Missing_Type) {
  auto params = GetParam();
  auto* p = parser(params.input);
  auto* t = p->type_decl();
  ASSERT_EQ(t, nullptr);
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(p->error(), "1:6: unable to determine subtype for vector");
}
INSTANTIATE_TEST_SUITE_P(ParserImplTest,
                         VecMissingType,
                         testing::Values(VecData{"vec2<>", 2},
                                         VecData{"vec3<>", 3},
                                         VecData{"vec4<>", 4}));

TEST_F(ParserImplTest, TypeDecl_Ptr) {
  auto* p = parser("ptr<function, f32>");
  auto* t = p->type_decl();
  ASSERT_NE(t, nullptr) << p->error();
  ASSERT_FALSE(p->has_error());
  ASSERT_TRUE(t->IsPointer());

  auto* ptr = t->AsPointer();
  ASSERT_TRUE(ptr->type()->IsF32());
  ASSERT_EQ(ptr->storage_class(), ast::StorageClass::kFunction);
}

TEST_F(ParserImplTest, TypeDecl_Ptr_ToVec) {
  auto* p = parser("ptr<function, vec2<f32>>");
  auto* t = p->type_decl();
  ASSERT_NE(t, nullptr) << p->error();
  ASSERT_FALSE(p->has_error());
  ASSERT_TRUE(t->IsPointer());

  auto* ptr = t->AsPointer();
  ASSERT_TRUE(ptr->type()->IsVector());
  ASSERT_EQ(ptr->storage_class(), ast::StorageClass::kFunction);

  auto* vec = ptr->type()->AsVector();
  ASSERT_EQ(vec->size(), 2u);
  ASSERT_TRUE(vec->type()->IsF32());
}

TEST_F(ParserImplTest, TypeDecl_Ptr_MissingLessThan) {
  auto* p = parser("ptr private, f32>");
  auto* t = p->type_decl();
  ASSERT_EQ(t, nullptr);
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(p->error(), "1:5: missing < for ptr declaration");
}

TEST_F(ParserImplTest, TypeDecl_Ptr_MissingGreaterThan) {
  auto* p = parser("ptr<function, f32");
  auto* t = p->type_decl();
  ASSERT_EQ(t, nullptr);
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(p->error(), "1:18: missing > for ptr declaration");
}

TEST_F(ParserImplTest, TypeDecl_Ptr_MissingComma) {
  auto* p = parser("ptr<function f32>");
  auto* t = p->type_decl();
  ASSERT_EQ(t, nullptr);
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(p->error(), "1:14: missing , for ptr declaration");
}

TEST_F(ParserImplTest, TypeDecl_Ptr_MissingStorageClass) {
  auto* p = parser("ptr<, f32>");
  auto* t = p->type_decl();
  ASSERT_EQ(t, nullptr);
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(p->error(), "1:5: missing storage class for ptr declaration");
}

TEST_F(ParserImplTest, TypeDecl_Ptr_MissingParams) {
  auto* p = parser("ptr<>");
  auto* t = p->type_decl();
  ASSERT_EQ(t, nullptr);
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(p->error(), "1:5: missing storage class for ptr declaration");
}

TEST_F(ParserImplTest, TypeDecl_Ptr_MissingType) {
  auto* p = parser("ptr<function,>");
  auto* t = p->type_decl();
  ASSERT_EQ(t, nullptr);
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(p->error(), "1:14: missing type for ptr declaration");
}

TEST_F(ParserImplTest, TypeDecl_Ptr_BadStorageClass) {
  auto* p = parser("ptr<unknown, f32>");
  auto* t = p->type_decl();
  ASSERT_EQ(t, nullptr);
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(p->error(), "1:5: missing storage class for ptr declaration");
}

TEST_F(ParserImplTest, TypeDecl_Ptr_BadType) {
  auto* p = parser("ptr<function, unknown>");
  auto* t = p->type_decl();
  ASSERT_EQ(t, nullptr);
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(p->error(), "1:15: unknown type alias 'unknown'");
}

TEST_F(ParserImplTest, TypeDecl_Array) {
  auto* p = parser("array<f32, 5>");
  auto* t = p->type_decl();
  ASSERT_NE(t, nullptr);
  ASSERT_FALSE(p->has_error());
  ASSERT_TRUE(t->IsArray());

  auto* a = t->AsArray();
  ASSERT_FALSE(a->IsRuntimeArray());
  ASSERT_EQ(a->size(), 5u);
  ASSERT_TRUE(a->type()->IsF32());
  ASSERT_FALSE(a->has_array_stride());
}

TEST_F(ParserImplTest, TypeDecl_Array_Stride) {
  auto* p = parser("[[stride 16]] array<f32, 5>");
  auto* t = p->type_decl();
  ASSERT_NE(t, nullptr);
  ASSERT_FALSE(p->has_error());
  ASSERT_TRUE(t->IsArray());

  auto* a = t->AsArray();
  ASSERT_FALSE(a->IsRuntimeArray());
  ASSERT_EQ(a->size(), 5u);
  ASSERT_TRUE(a->type()->IsF32());
  ASSERT_TRUE(a->has_array_stride());
  EXPECT_EQ(a->array_stride(), 16u);
}

TEST_F(ParserImplTest, TypeDecl_Array_Runtime_Stride) {
  auto* p = parser("[[stride 16]] array<f32>");
  auto* t = p->type_decl();
  ASSERT_NE(t, nullptr);
  ASSERT_FALSE(p->has_error());
  ASSERT_TRUE(t->IsArray());

  auto* a = t->AsArray();
  ASSERT_TRUE(a->IsRuntimeArray());
  ASSERT_TRUE(a->type()->IsF32());
  ASSERT_TRUE(a->has_array_stride());
  EXPECT_EQ(a->array_stride(), 16u);
}

TEST_F(ParserImplTest, TypeDecl_Array_Decoration_MissingArray) {
  auto* p = parser("[[stride 16]] f32");
  auto* t = p->type_decl();
  ASSERT_EQ(t, nullptr);
  ASSERT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:15: found array decoration but no array");
}

TEST_F(ParserImplTest, TypeDecl_Array_Decoration_MissingClosingAttr) {
  auto* p = parser("[[stride 16 array<f32, 5>");
  auto* t = p->type_decl();
  ASSERT_EQ(t, nullptr);
  ASSERT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:13: missing ]] for array decoration");
}

// Note, this isn't an error because it could be a struct decoration, we just
// don't have an array ...
TEST_F(ParserImplTest, TypeDecl_Array_Decoration_UnknownDecoration) {
  auto* p = parser("[[unknown 16]] array<f32, 5>");
  auto* t = p->type_decl();
  ASSERT_EQ(t, nullptr);
  ASSERT_FALSE(p->has_error());
}

TEST_F(ParserImplTest, TypeDecl_Array_Stride_MissingValue) {
  auto* p = parser("[[stride]] array<f32, 5>");
  auto* t = p->type_decl();
  ASSERT_EQ(t, nullptr);
  ASSERT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:9: missing value for stride decoration");
}

TEST_F(ParserImplTest, TypeDecl_Array_Stride_InvalidValue) {
  auto* p = parser("[[stride invalid]] array<f32, 5>");
  auto* t = p->type_decl();
  ASSERT_EQ(t, nullptr);
  ASSERT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:10: missing value for stride decoration");
}

TEST_F(ParserImplTest, TypeDecl_Array_Stride_InvalidValue_Negative) {
  auto* p = parser("[[stride -1]] array<f32, 5>");
  auto* t = p->type_decl();
  ASSERT_EQ(t, nullptr);
  ASSERT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:10: invalid stride value: -1");
}

TEST_F(ParserImplTest, TypeDecl_Array_Runtime) {
  auto* p = parser("array<u32>");
  auto* t = p->type_decl();
  ASSERT_NE(t, nullptr);
  ASSERT_FALSE(p->has_error());
  ASSERT_TRUE(t->IsArray());

  auto* a = t->AsArray();
  ASSERT_TRUE(a->IsRuntimeArray());
  ASSERT_TRUE(a->type()->IsU32());
}

TEST_F(ParserImplTest, TypeDecl_Array_BadType) {
  auto* p = parser("array<unknown, 3>");
  auto* t = p->type_decl();
  ASSERT_EQ(t, nullptr);
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(p->error(), "1:7: unknown type alias 'unknown'");
}

TEST_F(ParserImplTest, TypeDecl_Array_ZeroSize) {
  auto* p = parser("array<f32, 0>");
  auto* t = p->type_decl();
  ASSERT_EQ(t, nullptr);
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(p->error(), "1:12: invalid size for array declaration");
}

TEST_F(ParserImplTest, TypeDecl_Array_NegativeSize) {
  auto* p = parser("array<f32, -1>");
  auto* t = p->type_decl();
  ASSERT_EQ(t, nullptr);
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(p->error(), "1:12: invalid size for array declaration");
}

TEST_F(ParserImplTest, TypeDecl_Array_BadSize) {
  auto* p = parser("array<f32, invalid>");
  auto* t = p->type_decl();
  ASSERT_EQ(t, nullptr);
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(p->error(), "1:12: missing size of array declaration");
}

TEST_F(ParserImplTest, TypeDecl_Array_MissingLessThan) {
  auto* p = parser("array f32>");
  auto* t = p->type_decl();
  ASSERT_EQ(t, nullptr);
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(p->error(), "1:7: missing < for array declaration");
}

TEST_F(ParserImplTest, TypeDecl_Array_MissingGreaterThan) {
  auto* p = parser("array<f32");
  auto* t = p->type_decl();
  ASSERT_EQ(t, nullptr);
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(p->error(), "1:10: missing > for array declaration");
}

TEST_F(ParserImplTest, TypeDecl_Array_MissingComma) {
  auto* p = parser("array<f32 3>");
  auto* t = p->type_decl();
  ASSERT_EQ(t, nullptr);
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(p->error(), "1:11: missing > for array declaration");
}

struct MatrixData {
  const char* input;
  size_t rows;
  size_t columns;
};
inline std::ostream& operator<<(std::ostream& out, MatrixData data) {
  out << std::string(data.input);
  return out;
}
class MatrixTest : public testing::TestWithParam<MatrixData> {
 public:
  MatrixTest() = default;
  ~MatrixTest() override = default;

  void SetUp() override { ctx_.Reset(); }

  void TearDown() override { impl_ = nullptr; }

  ParserImpl* parser(const std::string& str) {
    impl_ = std::make_unique<ParserImpl>(&ctx_, str);
    return impl_.get();
  }

 private:
  std::unique_ptr<ParserImpl> impl_;
  Context ctx_;
};

TEST_P(MatrixTest, Parse) {
  auto params = GetParam();
  auto* p = parser(params.input);
  auto* t = p->type_decl();
  ASSERT_NE(t, nullptr);
  ASSERT_FALSE(p->has_error());
  EXPECT_TRUE(t->IsMatrix());
  auto* mat = t->AsMatrix();
  EXPECT_EQ(mat->rows(), params.rows);
  EXPECT_EQ(mat->columns(), params.columns);
}
INSTANTIATE_TEST_SUITE_P(ParserImplTest,
                         MatrixTest,
                         testing::Values(MatrixData{"mat2x2<f32>", 2, 2},
                                         MatrixData{"mat2x3<f32>", 2, 3},
                                         MatrixData{"mat2x4<f32>", 2, 4},
                                         MatrixData{"mat3x2<f32>", 3, 2},
                                         MatrixData{"mat3x3<f32>", 3, 3},
                                         MatrixData{"mat3x4<f32>", 3, 4},
                                         MatrixData{"mat4x2<f32>", 4, 2},
                                         MatrixData{"mat4x3<f32>", 4, 3},
                                         MatrixData{"mat4x4<f32>", 4, 4}));

class MatrixMissingGreaterThanTest : public testing::TestWithParam<MatrixData> {
 public:
  MatrixMissingGreaterThanTest() = default;
  ~MatrixMissingGreaterThanTest() override = default;

  void SetUp() override { ctx_.Reset(); }

  void TearDown() override { impl_ = nullptr; }

  ParserImpl* parser(const std::string& str) {
    impl_ = std::make_unique<ParserImpl>(&ctx_, str);
    return impl_.get();
  }

 private:
  std::unique_ptr<ParserImpl> impl_;
  Context ctx_;
};
TEST_P(MatrixMissingGreaterThanTest, Handles_Missing_GreaterThan) {
  auto params = GetParam();
  auto* p = parser(params.input);
  auto* t = p->type_decl();
  ASSERT_EQ(t, nullptr);
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(p->error(), "1:11: missing > for matrix");
}
INSTANTIATE_TEST_SUITE_P(ParserImplTest,
                         MatrixMissingGreaterThanTest,
                         testing::Values(MatrixData{"mat2x2<f32", 2, 2},
                                         MatrixData{"mat2x3<f32", 2, 3},
                                         MatrixData{"mat2x4<f32", 2, 4},
                                         MatrixData{"mat3x2<f32", 3, 2},
                                         MatrixData{"mat3x3<f32", 3, 3},
                                         MatrixData{"mat3x4<f32", 3, 4},
                                         MatrixData{"mat4x2<f32", 4, 2},
                                         MatrixData{"mat4x3<f32", 4, 3},
                                         MatrixData{"mat4x4<f32", 4, 4}));

class MatrixMissingLessThanTest : public testing::TestWithParam<MatrixData> {
 public:
  MatrixMissingLessThanTest() = default;
  ~MatrixMissingLessThanTest() override = default;

  void SetUp() override { ctx_.Reset(); }

  void TearDown() override { impl_ = nullptr; }

  ParserImpl* parser(const std::string& str) {
    impl_ = std::make_unique<ParserImpl>(&ctx_, str);
    return impl_.get();
  }

 private:
  std::unique_ptr<ParserImpl> impl_;
  Context ctx_;
};
TEST_P(MatrixMissingLessThanTest, Handles_Missing_GreaterThan) {
  auto params = GetParam();
  auto* p = parser(params.input);
  auto* t = p->type_decl();
  ASSERT_EQ(t, nullptr);
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(p->error(), "1:8: missing < for matrix");
}
INSTANTIATE_TEST_SUITE_P(ParserImplTest,
                         MatrixMissingLessThanTest,
                         testing::Values(MatrixData{"mat2x2 f32>", 2, 2},
                                         MatrixData{"mat2x3 f32>", 2, 3},
                                         MatrixData{"mat2x4 f32>", 2, 4},
                                         MatrixData{"mat3x2 f32>", 3, 2},
                                         MatrixData{"mat3x3 f32>", 3, 3},
                                         MatrixData{"mat3x4 f32>", 3, 4},
                                         MatrixData{"mat4x2 f32>", 4, 2},
                                         MatrixData{"mat4x3 f32>", 4, 3},
                                         MatrixData{"mat4x4 f32>", 4, 4}));

class MatrixBadType : public testing::TestWithParam<MatrixData> {
 public:
  MatrixBadType() = default;
  ~MatrixBadType() override = default;

  void SetUp() override { ctx_.Reset(); }

  void TearDown() override { impl_ = nullptr; }

  ParserImpl* parser(const std::string& str) {
    impl_ = std::make_unique<ParserImpl>(&ctx_, str);
    return impl_.get();
  }

 private:
  std::unique_ptr<ParserImpl> impl_;
  Context ctx_;
};
TEST_P(MatrixBadType, Handles_Unknown_Type) {
  auto params = GetParam();
  auto* p = parser(params.input);
  auto* t = p->type_decl();
  ASSERT_EQ(t, nullptr);
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(p->error(), "1:8: unknown type alias 'unknown'");
}
INSTANTIATE_TEST_SUITE_P(ParserImplTest,
                         MatrixBadType,
                         testing::Values(MatrixData{"mat2x2<unknown>", 2, 2},
                                         MatrixData{"mat2x3<unknown>", 2, 3},
                                         MatrixData{"mat2x4<unknown>", 2, 4},
                                         MatrixData{"mat3x2<unknown>", 3, 2},
                                         MatrixData{"mat3x3<unknown>", 3, 3},
                                         MatrixData{"mat3x4<unknown>", 3, 4},
                                         MatrixData{"mat4x2<unknown>", 4, 2},
                                         MatrixData{"mat4x3<unknown>", 4, 3},
                                         MatrixData{"mat4x4<unknown>", 4, 4}));

class MatrixMissingType : public testing::TestWithParam<MatrixData> {
 public:
  MatrixMissingType() = default;
  ~MatrixMissingType() override = default;

  void SetUp() override { ctx_.Reset(); }

  void TearDown() override { impl_ = nullptr; }

  ParserImpl* parser(const std::string& str) {
    impl_ = std::make_unique<ParserImpl>(&ctx_, str);
    return impl_.get();
  }

 private:
  std::unique_ptr<ParserImpl> impl_;
  Context ctx_;
};
TEST_P(MatrixMissingType, Handles_Missing_Type) {
  auto params = GetParam();
  auto* p = parser(params.input);
  auto* t = p->type_decl();
  ASSERT_EQ(t, nullptr);
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(p->error(), "1:8: unable to determine subtype for matrix");
}
INSTANTIATE_TEST_SUITE_P(ParserImplTest,
                         MatrixMissingType,
                         testing::Values(MatrixData{"mat2x2<>", 2, 2},
                                         MatrixData{"mat2x3<>", 2, 3},
                                         MatrixData{"mat2x4<>", 2, 4},
                                         MatrixData{"mat3x2<>", 3, 2},
                                         MatrixData{"mat3x3<>", 3, 3},
                                         MatrixData{"mat3x4<>", 3, 4},
                                         MatrixData{"mat4x2<>", 4, 2},
                                         MatrixData{"mat4x3<>", 4, 3},
                                         MatrixData{"mat4x4<>", 4, 4}));

}  // namespace
}  // namespace wgsl
}  // namespace reader
}  // namespace tint
