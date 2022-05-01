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

#include "src/tint/ast/alias.h"
#include "src/tint/ast/array.h"
#include "src/tint/ast/matrix.h"
#include "src/tint/ast/sampler.h"
#include "src/tint/reader/wgsl/parser_impl_test_helper.h"
#include "src/tint/sem/sampled_texture.h"

namespace tint::reader::wgsl {
namespace {

TEST_F(ParserImplTest, TypeDecl_Invalid) {
    auto p = parser("1234");
    auto t = p->type_decl();
    EXPECT_EQ(t.errored, false);
    EXPECT_EQ(t.matched, false);
    EXPECT_EQ(t.value, nullptr);
    EXPECT_FALSE(p->has_error());
}

TEST_F(ParserImplTest, TypeDecl_Identifier) {
    auto p = parser("A");

    auto t = p->type_decl();
    EXPECT_TRUE(t.matched);
    EXPECT_FALSE(t.errored);
    ASSERT_NE(t.value, nullptr) << p->error();
    auto* type_name = t.value->As<ast::TypeName>();
    ASSERT_NE(type_name, nullptr);
    EXPECT_EQ(p->builder().Symbols().Get("A"), type_name->name);
    EXPECT_EQ(type_name->source.range, (Source::Range{{1u, 1u}, {1u, 2u}}));
}

TEST_F(ParserImplTest, TypeDecl_Bool) {
    auto p = parser("bool");

    auto t = p->type_decl();
    EXPECT_TRUE(t.matched);
    EXPECT_FALSE(t.errored);
    ASSERT_NE(t.value, nullptr) << p->error();
    ASSERT_TRUE(t.value->Is<ast::Bool>());
    EXPECT_EQ(t.value->source.range, (Source::Range{{1u, 1u}, {1u, 5u}}));
}

TEST_F(ParserImplTest, TypeDecl_F32) {
    auto p = parser("f32");

    auto t = p->type_decl();
    EXPECT_TRUE(t.matched);
    EXPECT_FALSE(t.errored);
    ASSERT_NE(t.value, nullptr) << p->error();
    ASSERT_TRUE(t.value->Is<ast::F32>());
    EXPECT_EQ(t.value->source.range, (Source::Range{{1u, 1u}, {1u, 4u}}));
}

TEST_F(ParserImplTest, TypeDecl_I32) {
    auto p = parser("i32");

    auto t = p->type_decl();
    EXPECT_TRUE(t.matched);
    EXPECT_FALSE(t.errored);
    ASSERT_NE(t.value, nullptr) << p->error();
    ASSERT_TRUE(t.value->Is<ast::I32>());
    EXPECT_EQ(t.value->source.range, (Source::Range{{1u, 1u}, {1u, 4u}}));
}

TEST_F(ParserImplTest, TypeDecl_U32) {
    auto p = parser("u32");

    auto t = p->type_decl();
    EXPECT_TRUE(t.matched);
    EXPECT_FALSE(t.errored);
    ASSERT_NE(t.value, nullptr) << p->error();
    ASSERT_TRUE(t.value->Is<ast::U32>());
    EXPECT_EQ(t.value->source.range, (Source::Range{{1u, 1u}, {1u, 4u}}));
}

struct VecData {
    const char* input;
    size_t count;
    Source::Range range;
};
inline std::ostream& operator<<(std::ostream& out, VecData data) {
    out << std::string(data.input);
    return out;
}

class VecTest : public ParserImplTestWithParam<VecData> {};

TEST_P(VecTest, Parse) {
    auto params = GetParam();
    auto p = parser(params.input);
    auto t = p->type_decl();
    EXPECT_TRUE(t.matched);
    EXPECT_FALSE(t.errored);
    ASSERT_NE(t.value, nullptr) << p->error();
    ASSERT_FALSE(p->has_error());
    EXPECT_TRUE(t.value->Is<ast::Vector>());
    EXPECT_EQ(t.value->As<ast::Vector>()->width, params.count);
    EXPECT_EQ(t.value->source.range, params.range);
}
INSTANTIATE_TEST_SUITE_P(ParserImplTest,
                         VecTest,
                         testing::Values(VecData{"vec2<f32>", 2, {{1u, 1u}, {1u, 10u}}},
                                         VecData{"vec3<f32>", 3, {{1u, 1u}, {1u, 10u}}},
                                         VecData{"vec4<f32>", 4, {{1u, 1u}, {1u, 10u}}}));

class VecMissingGreaterThanTest : public ParserImplTestWithParam<VecData> {};

TEST_P(VecMissingGreaterThanTest, Handles_Missing_GreaterThan) {
    auto params = GetParam();
    auto p = parser(params.input);
    auto t = p->type_decl();
    EXPECT_TRUE(t.errored);
    EXPECT_FALSE(t.matched);
    ASSERT_EQ(t.value, nullptr);
    ASSERT_TRUE(p->has_error());
    ASSERT_EQ(p->error(), "1:9: expected '>' for vector");
}
INSTANTIATE_TEST_SUITE_P(ParserImplTest,
                         VecMissingGreaterThanTest,
                         testing::Values(VecData{"vec2<f32", 2, {}},
                                         VecData{"vec3<f32", 3, {}},
                                         VecData{"vec4<f32", 4, {}}));

class VecMissingType : public ParserImplTestWithParam<VecData> {};

TEST_P(VecMissingType, Handles_Missing_Type) {
    auto params = GetParam();
    auto p = parser(params.input);
    auto t = p->type_decl();
    EXPECT_TRUE(t.errored);
    EXPECT_FALSE(t.matched);
    ASSERT_EQ(t.value, nullptr);
    ASSERT_TRUE(p->has_error());
    ASSERT_EQ(p->error(), "1:6: invalid type for vector");
}
INSTANTIATE_TEST_SUITE_P(ParserImplTest,
                         VecMissingType,
                         testing::Values(VecData{"vec2<>", 2, {}},
                                         VecData{"vec3<>", 3, {}},
                                         VecData{"vec4<>", 4, {}}));

TEST_F(ParserImplTest, TypeDecl_Ptr) {
    auto p = parser("ptr<function, f32>");
    auto t = p->type_decl();
    EXPECT_TRUE(t.matched);
    EXPECT_FALSE(t.errored);
    ASSERT_NE(t.value, nullptr) << p->error();
    ASSERT_FALSE(p->has_error());
    ASSERT_TRUE(t.value->Is<ast::Pointer>());

    auto* ptr = t.value->As<ast::Pointer>();
    ASSERT_TRUE(ptr->type->Is<ast::F32>());
    ASSERT_EQ(ptr->storage_class, ast::StorageClass::kFunction);
    EXPECT_EQ(t.value->source.range, (Source::Range{{1u, 1u}, {1u, 19u}}));
}

TEST_F(ParserImplTest, TypeDecl_Ptr_WithAccess) {
    auto p = parser("ptr<function, f32, read>");
    auto t = p->type_decl();
    EXPECT_TRUE(t.matched);
    EXPECT_FALSE(t.errored);
    ASSERT_NE(t.value, nullptr) << p->error();
    ASSERT_FALSE(p->has_error());
    ASSERT_TRUE(t.value->Is<ast::Pointer>());

    auto* ptr = t.value->As<ast::Pointer>();
    ASSERT_TRUE(ptr->type->Is<ast::F32>());
    ASSERT_EQ(ptr->storage_class, ast::StorageClass::kFunction);
    ASSERT_EQ(ptr->access, ast::Access::kRead);
    EXPECT_EQ(t.value->source.range, (Source::Range{{1u, 1u}, {1u, 25u}}));
}

TEST_F(ParserImplTest, TypeDecl_Ptr_ToVec) {
    auto p = parser("ptr<function, vec2<f32>>");
    auto t = p->type_decl();
    EXPECT_TRUE(t.matched);
    EXPECT_FALSE(t.errored);
    ASSERT_NE(t.value, nullptr) << p->error();
    ASSERT_FALSE(p->has_error());
    ASSERT_TRUE(t.value->Is<ast::Pointer>());

    auto* ptr = t.value->As<ast::Pointer>();
    ASSERT_TRUE(ptr->type->Is<ast::Vector>());
    ASSERT_EQ(ptr->storage_class, ast::StorageClass::kFunction);

    auto* vec = ptr->type->As<ast::Vector>();
    ASSERT_EQ(vec->width, 2u);
    ASSERT_TRUE(vec->type->Is<ast::F32>());
    EXPECT_EQ(t.value->source.range, (Source::Range{{1u, 1u}, {1u, 25}}));
}

TEST_F(ParserImplTest, TypeDecl_Ptr_MissingLessThan) {
    auto p = parser("ptr private, f32>");
    auto t = p->type_decl();
    EXPECT_TRUE(t.errored);
    EXPECT_FALSE(t.matched);
    ASSERT_EQ(t.value, nullptr);
    ASSERT_TRUE(p->has_error());
    ASSERT_EQ(p->error(), "1:5: expected '<' for ptr declaration");
}

TEST_F(ParserImplTest, TypeDecl_Ptr_MissingGreaterThanAfterType) {
    auto p = parser("ptr<function, f32");
    auto t = p->type_decl();
    EXPECT_TRUE(t.errored);
    EXPECT_FALSE(t.matched);
    ASSERT_EQ(t.value, nullptr);
    ASSERT_TRUE(p->has_error());
    ASSERT_EQ(p->error(), "1:18: expected '>' for ptr declaration");
}

TEST_F(ParserImplTest, TypeDecl_Ptr_MissingGreaterThanAfterAccess) {
    auto p = parser("ptr<function, f32, read");
    auto t = p->type_decl();
    EXPECT_TRUE(t.errored);
    EXPECT_FALSE(t.matched);
    ASSERT_EQ(t.value, nullptr);
    ASSERT_TRUE(p->has_error());
    ASSERT_EQ(p->error(), "1:24: expected '>' for ptr declaration");
}

TEST_F(ParserImplTest, TypeDecl_Ptr_MissingCommaAfterStorageClass) {
    auto p = parser("ptr<function f32>");
    auto t = p->type_decl();
    EXPECT_TRUE(t.errored);
    EXPECT_FALSE(t.matched);
    ASSERT_EQ(t.value, nullptr);
    ASSERT_TRUE(p->has_error());
    ASSERT_EQ(p->error(), "1:14: expected ',' for ptr declaration");
}

TEST_F(ParserImplTest, TypeDecl_Ptr_MissingCommaAfterAccess) {
    auto p = parser("ptr<function, f32 read>");
    auto t = p->type_decl();
    EXPECT_TRUE(t.errored);
    EXPECT_FALSE(t.matched);
    ASSERT_EQ(t.value, nullptr);
    ASSERT_TRUE(p->has_error());
    ASSERT_EQ(p->error(), "1:19: expected '>' for ptr declaration");
}

TEST_F(ParserImplTest, TypeDecl_Ptr_MissingStorageClass) {
    auto p = parser("ptr<, f32>");
    auto t = p->type_decl();
    EXPECT_TRUE(t.errored);
    EXPECT_FALSE(t.matched);
    ASSERT_EQ(t.value, nullptr);
    ASSERT_TRUE(p->has_error());
    ASSERT_EQ(p->error(), "1:5: invalid storage class for ptr declaration");
}

TEST_F(ParserImplTest, TypeDecl_Ptr_MissingType) {
    auto p = parser("ptr<function,>");
    auto t = p->type_decl();
    EXPECT_TRUE(t.errored);
    EXPECT_FALSE(t.matched);
    ASSERT_EQ(t.value, nullptr);
    ASSERT_TRUE(p->has_error());
    ASSERT_EQ(p->error(), "1:14: invalid type for ptr declaration");
}

TEST_F(ParserImplTest, TypeDecl_Ptr_MissingAccess) {
    auto p = parser("ptr<function, i32, >");
    auto t = p->type_decl();
    EXPECT_TRUE(t.errored);
    EXPECT_FALSE(t.matched);
    ASSERT_EQ(t.value, nullptr);
    ASSERT_TRUE(p->has_error());
    ASSERT_EQ(p->error(), "1:20: expected identifier for access control");
}

TEST_F(ParserImplTest, TypeDecl_Ptr_MissingParams) {
    auto p = parser("ptr<>");
    auto t = p->type_decl();
    EXPECT_TRUE(t.errored);
    EXPECT_FALSE(t.matched);
    ASSERT_EQ(t.value, nullptr);
    ASSERT_TRUE(p->has_error());
    ASSERT_EQ(p->error(), "1:5: invalid storage class for ptr declaration");
}

TEST_F(ParserImplTest, TypeDecl_Ptr_BadStorageClass) {
    auto p = parser("ptr<unknown, f32>");
    auto t = p->type_decl();
    EXPECT_TRUE(t.errored);
    EXPECT_FALSE(t.matched);
    ASSERT_EQ(t.value, nullptr);
    ASSERT_TRUE(p->has_error());
    ASSERT_EQ(p->error(), "1:5: invalid storage class for ptr declaration");
}

TEST_F(ParserImplTest, TypeDecl_Ptr_BadAccess) {
    auto p = parser("ptr<function, i32, unknown>");
    auto t = p->type_decl();
    EXPECT_TRUE(t.errored);
    EXPECT_FALSE(t.matched);
    ASSERT_EQ(t.value, nullptr);
    ASSERT_TRUE(p->has_error());
    ASSERT_EQ(p->error(), "1:20: invalid value for access control");
}

TEST_F(ParserImplTest, TypeDecl_Atomic) {
    auto p = parser("atomic<f32>");
    auto t = p->type_decl();
    EXPECT_TRUE(t.matched);
    EXPECT_FALSE(t.errored);
    ASSERT_NE(t.value, nullptr) << p->error();
    ASSERT_FALSE(p->has_error());
    ASSERT_TRUE(t.value->Is<ast::Atomic>());

    auto* atomic = t.value->As<ast::Atomic>();
    ASSERT_TRUE(atomic->type->Is<ast::F32>());
    EXPECT_EQ(t.value->source.range, (Source::Range{{1u, 1u}, {1u, 12u}}));
}

TEST_F(ParserImplTest, TypeDecl_Atomic_ToVec) {
    auto p = parser("atomic<vec2<f32>>");
    auto t = p->type_decl();
    EXPECT_TRUE(t.matched);
    EXPECT_FALSE(t.errored);
    ASSERT_NE(t.value, nullptr) << p->error();
    ASSERT_FALSE(p->has_error());
    ASSERT_TRUE(t.value->Is<ast::Atomic>());

    auto* atomic = t.value->As<ast::Atomic>();
    ASSERT_TRUE(atomic->type->Is<ast::Vector>());

    auto* vec = atomic->type->As<ast::Vector>();
    ASSERT_EQ(vec->width, 2u);
    ASSERT_TRUE(vec->type->Is<ast::F32>());
    EXPECT_EQ(t.value->source.range, (Source::Range{{1u, 1u}, {1u, 18u}}));
}

TEST_F(ParserImplTest, TypeDecl_Atomic_MissingLessThan) {
    auto p = parser("atomic f32>");
    auto t = p->type_decl();
    EXPECT_TRUE(t.errored);
    EXPECT_FALSE(t.matched);
    ASSERT_EQ(t.value, nullptr);
    ASSERT_TRUE(p->has_error());
    ASSERT_EQ(p->error(), "1:8: expected '<' for atomic declaration");
}

TEST_F(ParserImplTest, TypeDecl_Atomic_MissingGreaterThan) {
    auto p = parser("atomic<f32");
    auto t = p->type_decl();
    EXPECT_TRUE(t.errored);
    EXPECT_FALSE(t.matched);
    ASSERT_EQ(t.value, nullptr);
    ASSERT_TRUE(p->has_error());
    ASSERT_EQ(p->error(), "1:11: expected '>' for atomic declaration");
}

TEST_F(ParserImplTest, TypeDecl_Atomic_MissingType) {
    auto p = parser("atomic<>");
    auto t = p->type_decl();
    EXPECT_TRUE(t.errored);
    EXPECT_FALSE(t.matched);
    ASSERT_EQ(t.value, nullptr);
    ASSERT_TRUE(p->has_error());
    ASSERT_EQ(p->error(), "1:8: invalid type for atomic declaration");
}

TEST_F(ParserImplTest, TypeDecl_Array_SintLiteralSize) {
    auto p = parser("array<f32, 5>");
    auto t = p->type_decl();
    EXPECT_TRUE(t.matched);
    EXPECT_FALSE(t.errored);
    ASSERT_NE(t.value, nullptr) << p->error();
    ASSERT_FALSE(p->has_error());
    ASSERT_TRUE(t.value->Is<ast::Array>());

    auto* a = t.value->As<ast::Array>();
    ASSERT_FALSE(a->IsRuntimeArray());
    ASSERT_TRUE(a->type->Is<ast::F32>());
    EXPECT_EQ(a->attributes.size(), 0u);
    EXPECT_EQ(t.value->source.range, (Source::Range{{1u, 1u}, {1u, 14u}}));

    auto* size = a->count->As<ast::SintLiteralExpression>();
    ASSERT_NE(size, nullptr);
    EXPECT_EQ(size->ValueAsI32(), 5);
}

TEST_F(ParserImplTest, TypeDecl_Array_UintLiteralSize) {
    auto p = parser("array<f32, 5u>");
    auto t = p->type_decl();
    EXPECT_TRUE(t.matched);
    EXPECT_FALSE(t.errored);
    ASSERT_NE(t.value, nullptr) << p->error();
    ASSERT_FALSE(p->has_error());
    ASSERT_TRUE(t.value->Is<ast::Array>());

    auto* a = t.value->As<ast::Array>();
    ASSERT_FALSE(a->IsRuntimeArray());
    ASSERT_TRUE(a->type->Is<ast::F32>());
    EXPECT_EQ(a->attributes.size(), 0u);
    EXPECT_EQ(t.value->source.range, (Source::Range{{1u, 1u}, {1u, 15u}}));

    auto* size = a->count->As<ast::UintLiteralExpression>();
    ASSERT_NE(size, nullptr);
    EXPECT_EQ(size->ValueAsU32(), 5u);
}

TEST_F(ParserImplTest, TypeDecl_Array_ConstantSize) {
    auto p = parser("array<f32, size>");
    auto t = p->type_decl();
    EXPECT_TRUE(t.matched);
    EXPECT_FALSE(t.errored);
    ASSERT_NE(t.value, nullptr) << p->error();
    ASSERT_FALSE(p->has_error());
    ASSERT_TRUE(t.value->Is<ast::Array>());

    auto* a = t.value->As<ast::Array>();
    ASSERT_FALSE(a->IsRuntimeArray());
    ASSERT_TRUE(a->type->Is<ast::F32>());
    EXPECT_EQ(a->attributes.size(), 0u);
    EXPECT_EQ(t.value->source.range, (Source::Range{{1u, 1u}, {1u, 17u}}));

    auto* count_expr = a->count->As<ast::IdentifierExpression>();
    ASSERT_NE(count_expr, nullptr);
    EXPECT_EQ(p->builder().Symbols().NameFor(count_expr->symbol), "size");
}

TEST_F(ParserImplTest, TypeDecl_Array_Runtime) {
    auto p = parser("array<u32>");
    auto t = p->type_decl();
    EXPECT_TRUE(t.matched);
    EXPECT_FALSE(t.errored);
    ASSERT_NE(t.value, nullptr) << p->error();
    ASSERT_FALSE(p->has_error());
    ASSERT_TRUE(t.value->Is<ast::Array>());

    auto* a = t.value->As<ast::Array>();
    ASSERT_TRUE(a->IsRuntimeArray());
    ASSERT_TRUE(a->type->Is<ast::U32>());
    EXPECT_EQ(t.value->source.range, (Source::Range{{1u, 1u}, {1u, 11u}}));
}

TEST_F(ParserImplTest, TypeDecl_Array_Runtime_Vec) {
    auto p = parser("array<vec4<u32>>");
    auto t = p->type_decl();
    EXPECT_TRUE(t.matched);
    EXPECT_FALSE(t.errored);
    ASSERT_NE(t.value, nullptr) << p->error();
    ASSERT_FALSE(p->has_error());
    ASSERT_TRUE(t.value->Is<ast::Array>());

    auto* a = t.value->As<ast::Array>();
    ASSERT_TRUE(a->IsRuntimeArray());
    ASSERT_TRUE(a->type->Is<ast::Vector>());
    EXPECT_EQ(a->type->As<ast::Vector>()->width, 4u);
    EXPECT_TRUE(a->type->As<ast::Vector>()->type->Is<ast::U32>());
    EXPECT_EQ(t.value->source.range, (Source::Range{{1u, 1u}, {1u, 17u}}));
}

TEST_F(ParserImplTest, TypeDecl_Array_BadSize) {
    auto p = parser("array<f32, !>");
    auto t = p->type_decl();
    EXPECT_TRUE(t.errored);
    EXPECT_FALSE(t.matched);
    ASSERT_EQ(t.value, nullptr);
    ASSERT_TRUE(p->has_error());
    ASSERT_EQ(p->error(), "1:12: expected array size expression");
}

TEST_F(ParserImplTest, TypeDecl_Array_MissingSize) {
    auto p = parser("array<f32,>");
    auto t = p->type_decl();
    EXPECT_TRUE(t.errored);
    EXPECT_FALSE(t.matched);
    ASSERT_EQ(t.value, nullptr);
    ASSERT_TRUE(p->has_error());
    ASSERT_EQ(p->error(), "1:11: expected array size expression");
}

TEST_F(ParserImplTest, TypeDecl_Array_MissingLessThan) {
    auto p = parser("array f32>");
    auto t = p->type_decl();
    EXPECT_TRUE(t.errored);
    EXPECT_FALSE(t.matched);
    ASSERT_EQ(t.value, nullptr);
    ASSERT_TRUE(p->has_error());
    ASSERT_EQ(p->error(), "1:7: expected '<' for array declaration");
}

TEST_F(ParserImplTest, TypeDecl_Array_MissingGreaterThan) {
    auto p = parser("array<f32");
    auto t = p->type_decl();
    EXPECT_TRUE(t.errored);
    EXPECT_FALSE(t.matched);
    ASSERT_EQ(t.value, nullptr);
    ASSERT_TRUE(p->has_error());
    ASSERT_EQ(p->error(), "1:10: expected '>' for array declaration");
}

TEST_F(ParserImplTest, TypeDecl_Array_MissingComma) {
    auto p = parser("array<f32 3>");
    auto t = p->type_decl();
    EXPECT_TRUE(t.errored);
    EXPECT_FALSE(t.matched);
    ASSERT_EQ(t.value, nullptr);
    ASSERT_TRUE(p->has_error());
    ASSERT_EQ(p->error(), "1:11: expected '>' for array declaration");
}

struct MatrixData {
    const char* input;
    size_t columns;
    size_t rows;
    Source::Range range;
};
inline std::ostream& operator<<(std::ostream& out, MatrixData data) {
    out << std::string(data.input);
    return out;
}

class MatrixTest : public ParserImplTestWithParam<MatrixData> {};

TEST_P(MatrixTest, Parse) {
    auto params = GetParam();
    auto p = parser(params.input);
    auto t = p->type_decl();
    EXPECT_TRUE(t.matched);
    EXPECT_FALSE(t.errored);
    ASSERT_NE(t.value, nullptr) << p->error();
    ASSERT_FALSE(p->has_error());
    EXPECT_TRUE(t.value->Is<ast::Matrix>());
    auto* mat = t.value->As<ast::Matrix>();
    EXPECT_EQ(mat->rows, params.rows);
    EXPECT_EQ(mat->columns, params.columns);
    EXPECT_EQ(t.value->source.range, params.range);
}
INSTANTIATE_TEST_SUITE_P(ParserImplTest,
                         MatrixTest,
                         testing::Values(MatrixData{"mat2x2<f32>", 2, 2, {{1u, 1u}, {1u, 12u}}},
                                         MatrixData{"mat2x3<f32>", 2, 3, {{1u, 1u}, {1u, 12u}}},
                                         MatrixData{"mat2x4<f32>", 2, 4, {{1u, 1u}, {1u, 12u}}},
                                         MatrixData{"mat3x2<f32>", 3, 2, {{1u, 1u}, {1u, 12u}}},
                                         MatrixData{"mat3x3<f32>", 3, 3, {{1u, 1u}, {1u, 12u}}},
                                         MatrixData{"mat3x4<f32>", 3, 4, {{1u, 1u}, {1u, 12u}}},
                                         MatrixData{"mat4x2<f32>", 4, 2, {{1u, 1u}, {1u, 12u}}},
                                         MatrixData{"mat4x3<f32>", 4, 3, {{1u, 1u}, {1u, 12u}}},
                                         MatrixData{"mat4x4<f32>", 4, 4, {{1u, 1u}, {1u, 12u}}}));

class MatrixMissingGreaterThanTest : public ParserImplTestWithParam<MatrixData> {};

TEST_P(MatrixMissingGreaterThanTest, Handles_Missing_GreaterThan) {
    auto params = GetParam();
    auto p = parser(params.input);
    auto t = p->type_decl();
    EXPECT_TRUE(t.errored);
    EXPECT_FALSE(t.matched);
    ASSERT_EQ(t.value, nullptr);
    ASSERT_TRUE(p->has_error());
    ASSERT_EQ(p->error(), "1:11: expected '>' for matrix");
}
INSTANTIATE_TEST_SUITE_P(ParserImplTest,
                         MatrixMissingGreaterThanTest,
                         testing::Values(MatrixData{"mat2x2<f32", 2, 2, {}},
                                         MatrixData{"mat2x3<f32", 2, 3, {}},
                                         MatrixData{"mat2x4<f32", 2, 4, {}},
                                         MatrixData{"mat3x2<f32", 3, 2, {}},
                                         MatrixData{"mat3x3<f32", 3, 3, {}},
                                         MatrixData{"mat3x4<f32", 3, 4, {}},
                                         MatrixData{"mat4x2<f32", 4, 2, {}},
                                         MatrixData{"mat4x3<f32", 4, 3, {}},
                                         MatrixData{"mat4x4<f32", 4, 4, {}}));

class MatrixMissingType : public ParserImplTestWithParam<MatrixData> {};

TEST_P(MatrixMissingType, Handles_Missing_Type) {
    auto params = GetParam();
    auto p = parser(params.input);
    auto t = p->type_decl();
    EXPECT_TRUE(t.errored);
    EXPECT_FALSE(t.matched);
    ASSERT_EQ(t.value, nullptr);
    ASSERT_TRUE(p->has_error());
    ASSERT_EQ(p->error(), "1:8: invalid type for matrix");
}
INSTANTIATE_TEST_SUITE_P(ParserImplTest,
                         MatrixMissingType,
                         testing::Values(MatrixData{"mat2x2<>", 2, 2, {}},
                                         MatrixData{"mat2x3<>", 2, 3, {}},
                                         MatrixData{"mat2x4<>", 2, 4, {}},
                                         MatrixData{"mat3x2<>", 3, 2, {}},
                                         MatrixData{"mat3x3<>", 3, 3, {}},
                                         MatrixData{"mat3x4<>", 3, 4, {}},
                                         MatrixData{"mat4x2<>", 4, 2, {}},
                                         MatrixData{"mat4x3<>", 4, 3, {}},
                                         MatrixData{"mat4x4<>", 4, 4, {}}));

TEST_F(ParserImplTest, TypeDecl_Sampler) {
    auto p = parser("sampler");

    auto t = p->type_decl();
    EXPECT_TRUE(t.matched);
    EXPECT_FALSE(t.errored);
    ASSERT_NE(t.value, nullptr) << p->error();
    ASSERT_TRUE(t.value->Is<ast::Sampler>());
    ASSERT_FALSE(t.value->As<ast::Sampler>()->IsComparison());
    EXPECT_EQ(t.value->source.range, (Source::Range{{1u, 1u}, {1u, 8u}}));
}

TEST_F(ParserImplTest, TypeDecl_Texture) {
    auto p = parser("texture_cube<f32>");

    auto t = p->type_decl();
    EXPECT_TRUE(t.matched);
    EXPECT_FALSE(t.errored);
    ASSERT_NE(t.value, nullptr);
    ASSERT_TRUE(t.value->Is<ast::Texture>());
    ASSERT_TRUE(t.value->Is<ast::SampledTexture>());
    ASSERT_TRUE(t.value->As<ast::SampledTexture>()->type->Is<ast::F32>());
    EXPECT_EQ(t.value->source.range, (Source::Range{{1u, 1u}, {1u, 18u}}));
}

}  // namespace
}  // namespace tint::reader::wgsl
