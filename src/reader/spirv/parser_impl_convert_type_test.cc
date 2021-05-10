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

#include "gmock/gmock.h"
#include "src/reader/spirv/parser_impl_test_helper.h"
#include "src/reader/spirv/spirv_tools_helpers_test.h"

namespace tint {
namespace reader {
namespace spirv {
namespace {

using ::testing::Eq;

TEST_F(SpvParserTest, ConvertType_PreservesExistingFailure) {
  auto p = parser(std::vector<uint32_t>{});
  p->Fail() << "boing";
  auto* type = p->ConvertType(10);
  EXPECT_EQ(type, nullptr);
  EXPECT_THAT(p->error(), Eq("boing"));
}

TEST_F(SpvParserTest, ConvertType_RequiresInternalRepresntation) {
  auto p = parser(std::vector<uint32_t>{});
  auto* type = p->ConvertType(10);
  EXPECT_EQ(type, nullptr);
  EXPECT_THAT(
      p->error(),
      Eq("ConvertType called when the internal module has not been built"));
}

TEST_F(SpvParserTest, ConvertType_NotAnId) {
  auto p = parser(test::Assemble("%1 = OpExtInstImport \"GLSL.std.450\""));
  EXPECT_TRUE(p->BuildInternalModule());

  auto* type = p->ConvertType(10);
  EXPECT_EQ(type, nullptr);
  EXPECT_EQ(nullptr, type);
  EXPECT_THAT(p->error(), Eq("ID is not a SPIR-V type: 10"));
}

TEST_F(SpvParserTest, ConvertType_IdExistsButIsNotAType) {
  auto p = parser(test::Assemble("%1 = OpExtInstImport \"GLSL.std.450\""));
  EXPECT_TRUE(p->BuildInternalModule());

  auto* type = p->ConvertType(1);
  EXPECT_EQ(nullptr, type);
  EXPECT_THAT(p->error(), Eq("ID is not a SPIR-V type: 1"));
}

TEST_F(SpvParserTest, ConvertType_UnhandledType) {
  // Pipes are an OpenCL type. Tint doesn't support them.
  auto p = parser(test::Assemble("%70 = OpTypePipe WriteOnly"));
  EXPECT_TRUE(p->BuildInternalModule());

  auto* type = p->ConvertType(70);
  EXPECT_EQ(nullptr, type);
  EXPECT_THAT(p->error(),
              Eq("unknown SPIR-V type with ID 70: %70 = OpTypePipe WriteOnly"));
}

TEST_F(SpvParserTest, ConvertType_Void) {
  auto p = parser(test::Assemble("%1 = OpTypeVoid"));
  EXPECT_TRUE(p->BuildInternalModule());

  auto* type = p->ConvertType(1);
  EXPECT_TRUE(type->Is<Void>());
  EXPECT_TRUE(p->error().empty());
}

TEST_F(SpvParserTest, ConvertType_Bool) {
  auto p = parser(test::Assemble("%100 = OpTypeBool"));
  EXPECT_TRUE(p->BuildInternalModule());

  auto* type = p->ConvertType(100);
  EXPECT_TRUE(type->Is<Bool>());
  EXPECT_TRUE(p->error().empty());
}

TEST_F(SpvParserTest, ConvertType_I32) {
  auto p = parser(test::Assemble("%2 = OpTypeInt 32 1"));
  EXPECT_TRUE(p->BuildInternalModule());

  auto* type = p->ConvertType(2);
  EXPECT_TRUE(type->Is<I32>());
  EXPECT_TRUE(p->error().empty());
}

TEST_F(SpvParserTest, ConvertType_U32) {
  auto p = parser(test::Assemble("%3 = OpTypeInt 32 0"));
  EXPECT_TRUE(p->BuildInternalModule());

  auto* type = p->ConvertType(3);
  EXPECT_TRUE(type->Is<U32>());
  EXPECT_TRUE(p->error().empty());
}

TEST_F(SpvParserTest, ConvertType_F32) {
  auto p = parser(test::Assemble("%4 = OpTypeFloat 32"));
  EXPECT_TRUE(p->BuildInternalModule());

  auto* type = p->ConvertType(4);
  EXPECT_TRUE(type->Is<F32>());
  EXPECT_TRUE(p->error().empty());
}

TEST_F(SpvParserTest, ConvertType_BadIntWidth) {
  auto p = parser(test::Assemble("%5 = OpTypeInt 17 1"));
  EXPECT_TRUE(p->BuildInternalModule());

  auto* type = p->ConvertType(5);
  EXPECT_EQ(type, nullptr);
  EXPECT_THAT(p->error(), Eq("unhandled integer width: 17"));
}

TEST_F(SpvParserTest, ConvertType_BadFloatWidth) {
  auto p = parser(test::Assemble("%6 = OpTypeFloat 19"));
  EXPECT_TRUE(p->BuildInternalModule());

  auto* type = p->ConvertType(6);
  EXPECT_EQ(type, nullptr);
  EXPECT_THAT(p->error(), Eq("unhandled float width: 19"));
}

TEST_F(SpvParserTest, DISABLED_ConvertType_InvalidVectorElement) {
  auto p = parser(test::Assemble(R"(
    %5 = OpTypePipe ReadOnly
    %20 = OpTypeVector %5 2
  )"));
  EXPECT_TRUE(p->BuildInternalModule());

  auto* type = p->ConvertType(20);
  EXPECT_EQ(type, nullptr);
  EXPECT_THAT(p->error(), Eq("unknown SPIR-V type: 5"));
}

TEST_F(SpvParserTest, ConvertType_VecOverF32) {
  auto p = parser(test::Assemble(R"(
    %float = OpTypeFloat 32
    %20 = OpTypeVector %float 2
    %30 = OpTypeVector %float 3
    %40 = OpTypeVector %float 4
  )"));
  EXPECT_TRUE(p->BuildInternalModule());

  auto* v2xf32 = p->ConvertType(20);
  EXPECT_TRUE(v2xf32->Is<Vector>());
  EXPECT_TRUE(v2xf32->As<Vector>()->type->Is<F32>());
  EXPECT_EQ(v2xf32->As<Vector>()->size, 2u);

  auto* v3xf32 = p->ConvertType(30);
  EXPECT_TRUE(v3xf32->Is<Vector>());
  EXPECT_TRUE(v3xf32->As<Vector>()->type->Is<F32>());
  EXPECT_EQ(v3xf32->As<Vector>()->size, 3u);

  auto* v4xf32 = p->ConvertType(40);
  EXPECT_TRUE(v4xf32->Is<Vector>());
  EXPECT_TRUE(v4xf32->As<Vector>()->type->Is<F32>());
  EXPECT_EQ(v4xf32->As<Vector>()->size, 4u);

  EXPECT_TRUE(p->error().empty());
}

TEST_F(SpvParserTest, ConvertType_VecOverI32) {
  auto p = parser(test::Assemble(R"(
    %int = OpTypeInt 32 1
    %20 = OpTypeVector %int 2
    %30 = OpTypeVector %int 3
    %40 = OpTypeVector %int 4
  )"));
  EXPECT_TRUE(p->BuildInternalModule());

  auto* v2xi32 = p->ConvertType(20);
  EXPECT_TRUE(v2xi32->Is<Vector>());
  EXPECT_TRUE(v2xi32->As<Vector>()->type->Is<I32>());
  EXPECT_EQ(v2xi32->As<Vector>()->size, 2u);

  auto* v3xi32 = p->ConvertType(30);
  EXPECT_TRUE(v3xi32->Is<Vector>());
  EXPECT_TRUE(v3xi32->As<Vector>()->type->Is<I32>());
  EXPECT_EQ(v3xi32->As<Vector>()->size, 3u);

  auto* v4xi32 = p->ConvertType(40);
  EXPECT_TRUE(v4xi32->Is<Vector>());
  EXPECT_TRUE(v4xi32->As<Vector>()->type->Is<I32>());
  EXPECT_EQ(v4xi32->As<Vector>()->size, 4u);

  EXPECT_TRUE(p->error().empty());
}

TEST_F(SpvParserTest, ConvertType_VecOverU32) {
  auto p = parser(test::Assemble(R"(
    %uint = OpTypeInt 32 0
    %20 = OpTypeVector %uint 2
    %30 = OpTypeVector %uint 3
    %40 = OpTypeVector %uint 4
  )"));
  EXPECT_TRUE(p->BuildInternalModule());

  auto* v2xu32 = p->ConvertType(20);
  EXPECT_TRUE(v2xu32->Is<Vector>());
  EXPECT_TRUE(v2xu32->As<Vector>()->type->Is<U32>());
  EXPECT_EQ(v2xu32->As<Vector>()->size, 2u);

  auto* v3xu32 = p->ConvertType(30);
  EXPECT_TRUE(v3xu32->Is<Vector>());
  EXPECT_TRUE(v3xu32->As<Vector>()->type->Is<U32>());
  EXPECT_EQ(v3xu32->As<Vector>()->size, 3u);

  auto* v4xu32 = p->ConvertType(40);
  EXPECT_TRUE(v4xu32->Is<Vector>());
  EXPECT_TRUE(v4xu32->As<Vector>()->type->Is<U32>());
  EXPECT_EQ(v4xu32->As<Vector>()->size, 4u);

  EXPECT_TRUE(p->error().empty());
}

TEST_F(SpvParserTest, DISABLED_ConvertType_InvalidMatrixElement) {
  auto p = parser(test::Assemble(R"(
    %5 = OpTypePipe ReadOnly
    %10 = OpTypeVector %5 2
    %20 = OpTypeMatrix %10 2
  )"));
  EXPECT_TRUE(p->BuildInternalModule());

  auto* type = p->ConvertType(20);
  EXPECT_EQ(type, nullptr);
  EXPECT_THAT(p->error(), Eq("unknown SPIR-V type: 5"));
}

TEST_F(SpvParserTest, ConvertType_MatrixOverF32) {
  // Matrices are only defined over floats.
  auto p = parser(test::Assemble(R"(
    %float = OpTypeFloat 32
    %v2 = OpTypeVector %float 2
    %v3 = OpTypeVector %float 3
    %v4 = OpTypeVector %float 4
    ; First digit is rows
    ; Second digit is columns
    %22 = OpTypeMatrix %v2 2
    %23 = OpTypeMatrix %v2 3
    %24 = OpTypeMatrix %v2 4
    %32 = OpTypeMatrix %v3 2
    %33 = OpTypeMatrix %v3 3
    %34 = OpTypeMatrix %v3 4
    %42 = OpTypeMatrix %v4 2
    %43 = OpTypeMatrix %v4 3
    %44 = OpTypeMatrix %v4 4
  )"));
  EXPECT_TRUE(p->BuildInternalModule());

  auto* m22 = p->ConvertType(22);
  EXPECT_TRUE(m22->Is<Matrix>());
  EXPECT_TRUE(m22->As<Matrix>()->type->Is<F32>());
  EXPECT_EQ(m22->As<Matrix>()->rows, 2u);
  EXPECT_EQ(m22->As<Matrix>()->columns, 2u);

  auto* m23 = p->ConvertType(23);
  EXPECT_TRUE(m23->Is<Matrix>());
  EXPECT_TRUE(m23->As<Matrix>()->type->Is<F32>());
  EXPECT_EQ(m23->As<Matrix>()->rows, 2u);
  EXPECT_EQ(m23->As<Matrix>()->columns, 3u);

  auto* m24 = p->ConvertType(24);
  EXPECT_TRUE(m24->Is<Matrix>());
  EXPECT_TRUE(m24->As<Matrix>()->type->Is<F32>());
  EXPECT_EQ(m24->As<Matrix>()->rows, 2u);
  EXPECT_EQ(m24->As<Matrix>()->columns, 4u);

  auto* m32 = p->ConvertType(32);
  EXPECT_TRUE(m32->Is<Matrix>());
  EXPECT_TRUE(m32->As<Matrix>()->type->Is<F32>());
  EXPECT_EQ(m32->As<Matrix>()->rows, 3u);
  EXPECT_EQ(m32->As<Matrix>()->columns, 2u);

  auto* m33 = p->ConvertType(33);
  EXPECT_TRUE(m33->Is<Matrix>());
  EXPECT_TRUE(m33->As<Matrix>()->type->Is<F32>());
  EXPECT_EQ(m33->As<Matrix>()->rows, 3u);
  EXPECT_EQ(m33->As<Matrix>()->columns, 3u);

  auto* m34 = p->ConvertType(34);
  EXPECT_TRUE(m34->Is<Matrix>());
  EXPECT_TRUE(m34->As<Matrix>()->type->Is<F32>());
  EXPECT_EQ(m34->As<Matrix>()->rows, 3u);
  EXPECT_EQ(m34->As<Matrix>()->columns, 4u);

  auto* m42 = p->ConvertType(42);
  EXPECT_TRUE(m42->Is<Matrix>());
  EXPECT_TRUE(m42->As<Matrix>()->type->Is<F32>());
  EXPECT_EQ(m42->As<Matrix>()->rows, 4u);
  EXPECT_EQ(m42->As<Matrix>()->columns, 2u);

  auto* m43 = p->ConvertType(43);
  EXPECT_TRUE(m43->Is<Matrix>());
  EXPECT_TRUE(m43->As<Matrix>()->type->Is<F32>());
  EXPECT_EQ(m43->As<Matrix>()->rows, 4u);
  EXPECT_EQ(m43->As<Matrix>()->columns, 3u);

  auto* m44 = p->ConvertType(44);
  EXPECT_TRUE(m44->Is<Matrix>());
  EXPECT_TRUE(m44->As<Matrix>()->type->Is<F32>());
  EXPECT_EQ(m44->As<Matrix>()->rows, 4u);
  EXPECT_EQ(m44->As<Matrix>()->columns, 4u);

  EXPECT_TRUE(p->error().empty());
}

TEST_F(SpvParserTest, ConvertType_RuntimeArray) {
  auto p = parser(test::Assemble(R"(
    %uint = OpTypeInt 32 0
    %10 = OpTypeRuntimeArray %uint
  )"));
  EXPECT_TRUE(p->BuildInternalModule());

  auto* type = p->ConvertType(10);
  ASSERT_NE(type, nullptr);
  EXPECT_TRUE(type->UnwrapAll()->Is<Array>());
  auto* arr_type = type->UnwrapAll()->As<Array>();
  ASSERT_NE(arr_type, nullptr);
  EXPECT_EQ(arr_type->size, 0u);
  EXPECT_EQ(arr_type->stride, 0u);
  auto* elem_type = arr_type->type;
  ASSERT_NE(elem_type, nullptr);
  EXPECT_TRUE(elem_type->Is<U32>());
  EXPECT_TRUE(p->error().empty());
}

TEST_F(SpvParserTest, ConvertType_RuntimeArray_InvalidDecoration) {
  auto p = parser(test::Assemble(R"(
    OpDecorate %10 Block
    %uint = OpTypeInt 32 0
    %10 = OpTypeRuntimeArray %uint
  )"));
  EXPECT_TRUE(p->BuildInternalModule());
  auto* type = p->ConvertType(10);
  EXPECT_EQ(type, nullptr);
  EXPECT_THAT(
      p->error(),
      Eq("invalid array type ID 10: unknown decoration 2 with 1 total words"));
}

TEST_F(SpvParserTest, ConvertType_RuntimeArray_ArrayStride_Valid) {
  auto p = parser(test::Assemble(R"(
    OpDecorate %10 ArrayStride 64
    %uint = OpTypeInt 32 0
    %10 = OpTypeRuntimeArray %uint
  )"));
  EXPECT_TRUE(p->BuildInternalModule());
  auto* type = p->ConvertType(10);
  ASSERT_NE(type, nullptr);
  auto* arr_type = type->UnwrapAll()->As<Array>();
  EXPECT_EQ(arr_type->size, 0u);
  ASSERT_NE(arr_type, nullptr);
  EXPECT_EQ(arr_type->stride, 64u);
}

TEST_F(SpvParserTest, ConvertType_RuntimeArray_ArrayStride_ZeroIsError) {
  auto p = parser(test::Assemble(R"(
    OpDecorate %10 ArrayStride 0
    %uint = OpTypeInt 32 0
    %10 = OpTypeRuntimeArray %uint
  )"));
  EXPECT_TRUE(p->BuildInternalModule());
  auto* type = p->ConvertType(10);
  EXPECT_EQ(type, nullptr);
  EXPECT_THAT(p->error(),
              Eq("invalid array type ID 10: ArrayStride can't be 0"));
}

TEST_F(SpvParserTest,
       ConvertType_RuntimeArray_ArrayStride_SpecifiedTwiceIsError) {
  auto p = parser(test::Assemble(R"(
    OpDecorate %10 ArrayStride 64
    OpDecorate %10 ArrayStride 64
    %uint = OpTypeInt 32 0
    %10 = OpTypeRuntimeArray %uint
  )"));
  EXPECT_TRUE(p->BuildInternalModule());
  auto* type = p->ConvertType(10);
  EXPECT_EQ(type, nullptr);
  EXPECT_THAT(p->error(),
              Eq("invalid array type ID 10: multiple ArrayStride decorations"));
}

TEST_F(SpvParserTest, ConvertType_Array) {
  auto p = parser(test::Assemble(R"(
    %uint = OpTypeInt 32 0
    %uint_42 = OpConstant %uint 42
    %10 = OpTypeArray %uint %uint_42
  )"));
  EXPECT_TRUE(p->BuildInternalModule());

  auto* type = p->ConvertType(10);
  ASSERT_NE(type, nullptr);
  EXPECT_TRUE(type->Is<Array>());
  auto* arr_type = type->As<Array>();
  ASSERT_NE(arr_type, nullptr);
  EXPECT_EQ(arr_type->size, 42u);
  EXPECT_EQ(arr_type->stride, 0u);
  auto* elem_type = arr_type->type;
  ASSERT_NE(elem_type, nullptr);
  EXPECT_TRUE(elem_type->Is<U32>());
  EXPECT_TRUE(p->error().empty());
}

TEST_F(SpvParserTest, ConvertType_ArrayBadLengthIsSpecConstantValue) {
  auto p = parser(test::Assemble(R"(
    OpDecorate %uint_42 SpecId 12
    %uint = OpTypeInt 32 0
    %uint_42 = OpSpecConstant %uint 42
    %10 = OpTypeArray %uint %uint_42
  )"));
  EXPECT_TRUE(p->BuildInternalModule());

  auto* type = p->ConvertType(10);
  ASSERT_EQ(type, nullptr);
  EXPECT_THAT(p->error(),
              Eq("Array type 10 length is a specialization constant"));
}

TEST_F(SpvParserTest, ConvertType_ArrayBadLengthIsSpecConstantExpr) {
  auto p = parser(test::Assemble(R"(
    %uint = OpTypeInt 32 0
    %uint_42 = OpConstant %uint 42
    %sum = OpSpecConstantOp %uint IAdd %uint_42 %uint_42
    %10 = OpTypeArray %uint %sum
  )"));
  EXPECT_TRUE(p->BuildInternalModule());

  auto* type = p->ConvertType(10);
  ASSERT_EQ(type, nullptr);
  EXPECT_THAT(p->error(),
              Eq("Array type 10 length is a specialization constant"));
}

// TODO(dneto): Maybe add a test where the length operand is not a constant.
// E.g. it's the ID of a type.  That won't validate, and the SPIRV-Tools
// optimizer representation doesn't handle it and asserts out instead.

TEST_F(SpvParserTest, ConvertType_ArrayBadTooBig) {
  auto p = parser(test::Assemble(R"(
    %uint64 = OpTypeInt 64 0
    %uint64_big = OpConstant %uint64 5000000000
    %10 = OpTypeArray %uint64 %uint64_big
  )"));
  EXPECT_TRUE(p->BuildInternalModule());

  auto* type = p->ConvertType(10);
  ASSERT_EQ(type, nullptr);
  // TODO(dneto): Right now it's rejected earlier in the flow because
  // we can't even utter the uint64 type.
  EXPECT_THAT(p->error(), Eq("unhandled integer width: 64"));
}

TEST_F(SpvParserTest, ConvertType_Array_InvalidDecoration) {
  auto p = parser(test::Assemble(R"(
    OpDecorate %10 Block
    %uint = OpTypeInt 32 0
    %uint_5 = OpConstant %uint 5
    %10 = OpTypeArray %uint %uint_5
  )"));
  EXPECT_TRUE(p->BuildInternalModule());
  auto* type = p->ConvertType(10);
  EXPECT_EQ(type, nullptr);
  EXPECT_THAT(
      p->error(),
      Eq("invalid array type ID 10: unknown decoration 2 with 1 total words"));
}

TEST_F(SpvParserTest, ConvertType_ArrayStride_Valid) {
  auto p = parser(test::Assemble(R"(
    OpDecorate %10 ArrayStride 8
    %uint = OpTypeInt 32 0
    %uint_5 = OpConstant %uint 5
    %10 = OpTypeArray %uint %uint_5
  )"));
  EXPECT_TRUE(p->BuildInternalModule());

  auto* type = p->ConvertType(10);
  ASSERT_NE(type, nullptr);
  EXPECT_TRUE(type->UnwrapAll()->Is<Array>());
  auto* arr_type = type->UnwrapAll()->As<Array>();
  ASSERT_NE(arr_type, nullptr);
  EXPECT_EQ(arr_type->stride, 8u);
  EXPECT_TRUE(p->error().empty());
}

TEST_F(SpvParserTest, ConvertType_ArrayStride_ZeroIsError) {
  auto p = parser(test::Assemble(R"(
    OpDecorate %10 ArrayStride 0
    %uint = OpTypeInt 32 0
    %uint_5 = OpConstant %uint 5
    %10 = OpTypeArray %uint %uint_5
  )"));
  EXPECT_TRUE(p->BuildInternalModule());

  auto* type = p->ConvertType(10);
  ASSERT_EQ(type, nullptr);
  EXPECT_THAT(p->error(),
              Eq("invalid array type ID 10: ArrayStride can't be 0"));
}

TEST_F(SpvParserTest, ConvertType_ArrayStride_SpecifiedTwiceIsError) {
  auto p = parser(test::Assemble(R"(
    OpDecorate %10 ArrayStride 4
    OpDecorate %10 ArrayStride 4
    %uint = OpTypeInt 32 0
    %uint_5 = OpConstant %uint 5
    %10 = OpTypeArray %uint %uint_5
  )"));
  EXPECT_TRUE(p->BuildInternalModule());

  auto* type = p->ConvertType(10);
  ASSERT_EQ(type, nullptr);
  EXPECT_THAT(p->error(),
              Eq("invalid array type ID 10: multiple ArrayStride decorations"));
}

TEST_F(SpvParserTest, ConvertType_StructTwoMembers) {
  auto p = parser(test::Assemble(R"(
    %uint = OpTypeInt 32 0
    %float = OpTypeFloat 32
    %10 = OpTypeStruct %uint %float
  )"));
  EXPECT_TRUE(p->BuildInternalModule());
  EXPECT_TRUE(p->RegisterUserAndStructMemberNames());

  auto* type = p->ConvertType(10);
  ASSERT_NE(type, nullptr);
  EXPECT_TRUE(type->Is<Struct>());

  auto* str = type->Build(p->builder());
  Program program = p->program();
  EXPECT_THAT(program.str(str), Eq(R"(__type_name_S)"));
}

TEST_F(SpvParserTest, ConvertType_StructWithBlockDecoration) {
  auto p = parser(test::Assemble(R"(
    OpDecorate %10 Block
    %uint = OpTypeInt 32 0
    %10 = OpTypeStruct %uint
  )"));
  EXPECT_TRUE(p->BuildInternalModule());
  EXPECT_TRUE(p->RegisterUserAndStructMemberNames());

  auto* type = p->ConvertType(10);
  ASSERT_NE(type, nullptr);
  EXPECT_TRUE(type->Is<Struct>());

  auto* str = type->Build(p->builder());
  Program program = p->program();
  EXPECT_THAT(program.str(str), Eq(R"(__type_name_S)"));
}

TEST_F(SpvParserTest, ConvertType_StructWithMemberDecorations) {
  auto p = parser(test::Assemble(R"(
    OpMemberDecorate %10 0 Offset 0
    OpMemberDecorate %10 1 Offset 8
    OpMemberDecorate %10 2 Offset 16
    %float = OpTypeFloat 32
    %vec = OpTypeVector %float 2
    %mat = OpTypeMatrix %vec 2
    %10 = OpTypeStruct %float %vec %mat
  )"));
  EXPECT_TRUE(p->BuildInternalModule());
  EXPECT_TRUE(p->RegisterUserAndStructMemberNames());

  auto* type = p->ConvertType(10);
  ASSERT_NE(type, nullptr);
  EXPECT_TRUE(type->Is<Struct>());

  auto* str = type->Build(p->builder());
  Program program = p->program();
  EXPECT_THAT(program.str(str), Eq(R"(__type_name_S)"));
}

// TODO(dneto): Demonstrate other member decorations. Blocked on
// crbug.com/tint/30
// TODO(dneto): Demonstrate multiple member deocrations. Blocked on
// crbug.com/tint/30

TEST_F(SpvParserTest, ConvertType_InvalidPointeetype) {
  // Disallow pointer-to-function
  auto p = parser(test::Assemble(R"(
  %void = OpTypeVoid
  %42 = OpTypeFunction %void
  %3 = OpTypePointer Input %42
  )"));
  EXPECT_TRUE(p->BuildInternalModule()) << p->error();

  auto* type = p->ConvertType(3);
  EXPECT_EQ(type, nullptr);
  EXPECT_THAT(p->error(),
              Eq("SPIR-V pointer type with ID 3 has invalid pointee type 42"));
}

TEST_F(SpvParserTest, DISABLED_ConvertType_InvalidStorageClass) {
  // Disallow invalid storage class
  auto p = parser(test::Assemble(R"(
  %1 = OpTypeFloat 32
  %3 = OpTypePointer !999 %1   ; Special syntax to inject 999 as the storage class
  )"));
  // TODO(dneto): I can't get it past module building.
  EXPECT_FALSE(p->BuildInternalModule()) << p->error();
}

TEST_F(SpvParserTest, ConvertType_PointerInput) {
  auto p = parser(test::Assemble(R"(
  %float = OpTypeFloat 32
  %3 = OpTypePointer Input %float
  )"));
  EXPECT_TRUE(p->BuildInternalModule());

  auto* type = p->ConvertType(3);
  EXPECT_TRUE(type->Is<Pointer>());
  auto* ptr_ty = type->As<Pointer>();
  EXPECT_NE(ptr_ty, nullptr);
  EXPECT_TRUE(ptr_ty->type->Is<F32>());
  EXPECT_EQ(ptr_ty->storage_class, ast::StorageClass::kInput);
  EXPECT_TRUE(p->error().empty());
}

TEST_F(SpvParserTest, ConvertType_PointerOutput) {
  auto p = parser(test::Assemble(R"(
  %float = OpTypeFloat 32
  %3 = OpTypePointer Output %float
  )"));
  EXPECT_TRUE(p->BuildInternalModule());

  auto* type = p->ConvertType(3);
  EXPECT_TRUE(type->Is<Pointer>());
  auto* ptr_ty = type->As<Pointer>();
  EXPECT_NE(ptr_ty, nullptr);
  EXPECT_TRUE(ptr_ty->type->Is<F32>());
  EXPECT_EQ(ptr_ty->storage_class, ast::StorageClass::kOutput);
  EXPECT_TRUE(p->error().empty());
}

TEST_F(SpvParserTest, ConvertType_PointerUniform) {
  auto p = parser(test::Assemble(R"(
  %float = OpTypeFloat 32
  %3 = OpTypePointer Uniform %float
  )"));
  EXPECT_TRUE(p->BuildInternalModule());

  auto* type = p->ConvertType(3);
  EXPECT_TRUE(type->Is<Pointer>());
  auto* ptr_ty = type->As<Pointer>();
  EXPECT_NE(ptr_ty, nullptr);
  EXPECT_TRUE(ptr_ty->type->Is<F32>());
  EXPECT_EQ(ptr_ty->storage_class, ast::StorageClass::kUniform);
  EXPECT_TRUE(p->error().empty());
}

TEST_F(SpvParserTest, ConvertType_PointerWorkgroup) {
  auto p = parser(test::Assemble(R"(
  %float = OpTypeFloat 32
  %3 = OpTypePointer Workgroup %float
  )"));
  EXPECT_TRUE(p->BuildInternalModule());

  auto* type = p->ConvertType(3);
  EXPECT_TRUE(type->Is<Pointer>());
  auto* ptr_ty = type->As<Pointer>();
  EXPECT_NE(ptr_ty, nullptr);
  EXPECT_TRUE(ptr_ty->type->Is<F32>());
  EXPECT_EQ(ptr_ty->storage_class, ast::StorageClass::kWorkgroup);
  EXPECT_TRUE(p->error().empty());
}

TEST_F(SpvParserTest, ConvertType_PointerUniformConstant) {
  auto p = parser(test::Assemble(R"(
  %float = OpTypeFloat 32
  %3 = OpTypePointer UniformConstant %float
  )"));
  EXPECT_TRUE(p->BuildInternalModule());

  auto* type = p->ConvertType(3);
  EXPECT_TRUE(type->Is<Pointer>());
  auto* ptr_ty = type->As<Pointer>();
  EXPECT_NE(ptr_ty, nullptr);
  EXPECT_TRUE(ptr_ty->type->Is<F32>());
  EXPECT_EQ(ptr_ty->storage_class, ast::StorageClass::kNone);
  EXPECT_TRUE(p->error().empty());
}

TEST_F(SpvParserTest, ConvertType_PointerStorageBuffer) {
  auto p = parser(test::Assemble(R"(
  %float = OpTypeFloat 32
  %3 = OpTypePointer StorageBuffer %float
  )"));
  EXPECT_TRUE(p->BuildInternalModule());

  auto* type = p->ConvertType(3);
  EXPECT_TRUE(type->Is<Pointer>());
  auto* ptr_ty = type->As<Pointer>();
  EXPECT_NE(ptr_ty, nullptr);
  EXPECT_TRUE(ptr_ty->type->Is<F32>());
  EXPECT_EQ(ptr_ty->storage_class, ast::StorageClass::kStorage);
  EXPECT_TRUE(p->error().empty());
}

TEST_F(SpvParserTest, ConvertType_PointerImage) {
  auto p = parser(test::Assemble(R"(
  %float = OpTypeFloat 32
  %3 = OpTypePointer Image %float
  )"));
  EXPECT_TRUE(p->BuildInternalModule());

  auto* type = p->ConvertType(3);
  EXPECT_TRUE(type->Is<Pointer>());
  auto* ptr_ty = type->As<Pointer>();
  EXPECT_NE(ptr_ty, nullptr);
  EXPECT_TRUE(ptr_ty->type->Is<F32>());
  EXPECT_EQ(ptr_ty->storage_class, ast::StorageClass::kImage);
  EXPECT_TRUE(p->error().empty());
}

TEST_F(SpvParserTest, ConvertType_PointerPrivate) {
  auto p = parser(test::Assemble(R"(
  %float = OpTypeFloat 32
  %3 = OpTypePointer Private %float
  )"));
  EXPECT_TRUE(p->BuildInternalModule());

  auto* type = p->ConvertType(3);
  EXPECT_TRUE(type->Is<Pointer>());
  auto* ptr_ty = type->As<Pointer>();
  EXPECT_NE(ptr_ty, nullptr);
  EXPECT_TRUE(ptr_ty->type->Is<F32>());
  EXPECT_EQ(ptr_ty->storage_class, ast::StorageClass::kPrivate);
  EXPECT_TRUE(p->error().empty());
}

TEST_F(SpvParserTest, ConvertType_PointerFunction) {
  auto p = parser(test::Assemble(R"(
  %float = OpTypeFloat 32
  %3 = OpTypePointer Function %float
  )"));
  EXPECT_TRUE(p->BuildInternalModule());

  auto* type = p->ConvertType(3);
  EXPECT_TRUE(type->Is<Pointer>());
  auto* ptr_ty = type->As<Pointer>();
  EXPECT_NE(ptr_ty, nullptr);
  EXPECT_TRUE(ptr_ty->type->Is<F32>());
  EXPECT_EQ(ptr_ty->storage_class, ast::StorageClass::kFunction);
  EXPECT_TRUE(p->error().empty());
}

TEST_F(SpvParserTest, ConvertType_PointerToPointer) {
  // FYI:  The reader suports pointer-to-pointer even while WebGPU does not.
  auto p = parser(test::Assemble(R"(
  %float = OpTypeFloat 32
  %42 = OpTypePointer Output %float
  %3 = OpTypePointer Input %42
  )"));
  EXPECT_TRUE(p->BuildInternalModule());

  auto* type = p->ConvertType(3);
  EXPECT_NE(type, nullptr);
  EXPECT_TRUE(type->Is<Pointer>());

  auto* ptr_ty = type->As<Pointer>();
  EXPECT_NE(ptr_ty, nullptr);
  EXPECT_EQ(ptr_ty->storage_class, ast::StorageClass::kInput);
  EXPECT_TRUE(ptr_ty->type->Is<Pointer>());

  auto* ptr_ptr_ty = ptr_ty->type->As<Pointer>();
  EXPECT_NE(ptr_ptr_ty, nullptr);
  EXPECT_EQ(ptr_ptr_ty->storage_class, ast::StorageClass::kOutput);
  EXPECT_TRUE(ptr_ptr_ty->type->Is<F32>());

  EXPECT_TRUE(p->error().empty());
}

TEST_F(SpvParserTest, ConvertType_Sampler_PretendVoid) {
  // We fake the type suport for samplers, images, and sampled images.
  auto p = parser(test::Assemble(R"(
  %1 = OpTypeSampler
  )"));
  EXPECT_TRUE(p->BuildInternalModule());

  auto* type = p->ConvertType(1);
  EXPECT_TRUE(type->Is<Void>());
  EXPECT_TRUE(p->error().empty());
}

TEST_F(SpvParserTest, ConvertType_Image_PretendVoid) {
  // We fake the type suport for samplers, images, and sampled images.
  auto p = parser(test::Assemble(R"(
  %float = OpTypeFloat 32
  %1 = OpTypeImage %float 2D 0 0 0 1 Unknown
  )"));
  EXPECT_TRUE(p->BuildInternalModule());

  auto* type = p->ConvertType(1);
  EXPECT_TRUE(type->Is<Void>());
  EXPECT_TRUE(p->error().empty());
}

TEST_F(SpvParserTest, ConvertType_SampledImage_PretendVoid) {
  auto p = parser(test::Assemble(R"(
  %float = OpTypeFloat 32
  %im = OpTypeImage %float 2D 0 0 0 1 Unknown
  %1 = OpTypeSampledImage %im
  )"));
  EXPECT_TRUE(p->BuildInternalModule());

  auto* type = p->ConvertType(1);
  EXPECT_TRUE(type->Is<Void>());
  EXPECT_TRUE(p->error().empty());
}

}  // namespace
}  // namespace spirv
}  // namespace reader
}  // namespace tint
