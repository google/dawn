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

#include <cstdint>
#include <string>
#include <vector>

#include "gmock/gmock.h"
#include "src/ast/type/matrix_type.h"
#include "src/ast/type/vector_type.h"
#include "src/reader/spirv/parser_impl.h"
#include "src/reader/spirv/parser_impl_test_helper.h"
#include "src/reader/spirv/spirv_tools_helpers_test.h"
#include "src/type_manager.h"

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
  EXPECT_TRUE(p->BuildAndParseInternalModule()) << p->error();

  auto* type = p->ConvertType(10);
  EXPECT_EQ(type, nullptr);
  EXPECT_EQ(nullptr, type);
  EXPECT_THAT(p->error(), Eq("ID is not a SPIR-V type: 10"));
}

TEST_F(SpvParserTest, ConvertType_IdExistsButIsNotAType) {
  auto p = parser(test::Assemble("%1 = OpExtInstImport \"GLSL.std.450\""));
  EXPECT_TRUE(p->BuildAndParseInternalModule());

  auto* type = p->ConvertType(1);
  EXPECT_EQ(nullptr, type);
  EXPECT_THAT(p->error(), Eq("ID is not a SPIR-V type: 1"));
}

TEST_F(SpvParserTest, ConvertType_UnhandledType) {
  // Pipes are an OpenCL type. Tint doesn't support them.
  auto p = parser(test::Assemble("%70 = OpTypePipe WriteOnly"));
  EXPECT_TRUE(p->BuildAndParseInternalModule());

  auto* type = p->ConvertType(70);
  EXPECT_EQ(nullptr, type);
  EXPECT_THAT(p->error(), Eq("unknown SPIR-V type: 70"));
}

TEST_F(SpvParserTest, ConvertType_Void) {
  auto p = parser(test::Assemble("%1 = OpTypeVoid"));
  EXPECT_TRUE(p->BuildAndParseInternalModule());

  auto* type = p->ConvertType(1);
  EXPECT_TRUE(type->IsVoid());
  EXPECT_TRUE(p->error().empty());
}

TEST_F(SpvParserTest, ConvertType_Bool) {
  auto p = parser(test::Assemble("%100 = OpTypeBool"));
  EXPECT_TRUE(p->BuildAndParseInternalModule());

  auto* type = p->ConvertType(100);
  EXPECT_TRUE(type->IsBool());
  EXPECT_TRUE(p->error().empty());
}

TEST_F(SpvParserTest, ConvertType_I32) {
  auto p = parser(test::Assemble("%2 = OpTypeInt 32 1"));
  EXPECT_TRUE(p->BuildAndParseInternalModule());

  auto* type = p->ConvertType(2);
  EXPECT_TRUE(type->IsI32());
  EXPECT_TRUE(p->error().empty());
}

TEST_F(SpvParserTest, ConvertType_U32) {
  auto p = parser(test::Assemble("%3 = OpTypeInt 32 0"));
  EXPECT_TRUE(p->BuildAndParseInternalModule());

  auto* type = p->ConvertType(3);
  EXPECT_TRUE(type->IsU32());
  EXPECT_TRUE(p->error().empty());
}

TEST_F(SpvParserTest, ConvertType_F32) {
  auto p = parser(test::Assemble("%4 = OpTypeFloat 32"));
  EXPECT_TRUE(p->BuildAndParseInternalModule());

  auto* type = p->ConvertType(4);
  EXPECT_TRUE(type->IsF32());
  EXPECT_TRUE(p->error().empty());
}

TEST_F(SpvParserTest, ConvertType_BadIntWidth) {
  auto p = parser(test::Assemble("%5 = OpTypeInt 17 1"));
  EXPECT_TRUE(p->BuildAndParseInternalModule());

  auto* type = p->ConvertType(5);
  EXPECT_EQ(type, nullptr);
  EXPECT_THAT(p->error(), Eq("unhandled integer width: 17"));
}

TEST_F(SpvParserTest, ConvertType_BadFloatWidth) {
  auto p = parser(test::Assemble("%6 = OpTypeFloat 19"));
  EXPECT_TRUE(p->BuildAndParseInternalModule());

  auto* type = p->ConvertType(6);
  EXPECT_EQ(type, nullptr);
  EXPECT_THAT(p->error(), Eq("unhandled float width: 19"));
}

TEST_F(SpvParserTest, DISABLED_ConvertType_InvalidVectorElement) {
  auto p = parser(test::Assemble(R"(
    %5 = OpTypePipe ReadOnly
    %20 = OpTypeVector %5 2
  )"));
  EXPECT_TRUE(p->BuildAndParseInternalModule());

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
  EXPECT_TRUE(p->BuildAndParseInternalModule());

  auto* v2xf32 = p->ConvertType(20);
  EXPECT_TRUE(v2xf32->IsVector());
  EXPECT_TRUE(v2xf32->AsVector()->type()->IsF32());
  EXPECT_EQ(v2xf32->AsVector()->size(), 2u);

  auto* v3xf32 = p->ConvertType(30);
  EXPECT_TRUE(v3xf32->IsVector());
  EXPECT_TRUE(v3xf32->AsVector()->type()->IsF32());
  EXPECT_EQ(v3xf32->AsVector()->size(), 3u);

  auto* v4xf32 = p->ConvertType(40);
  EXPECT_TRUE(v4xf32->IsVector());
  EXPECT_TRUE(v4xf32->AsVector()->type()->IsF32());
  EXPECT_EQ(v4xf32->AsVector()->size(), 4u);

  EXPECT_TRUE(p->error().empty());
}

TEST_F(SpvParserTest, ConvertType_VecOverI32) {
  auto p = parser(test::Assemble(R"(
    %int = OpTypeInt 32 1
    %20 = OpTypeVector %int 2
    %30 = OpTypeVector %int 3
    %40 = OpTypeVector %int 4
  )"));
  EXPECT_TRUE(p->BuildAndParseInternalModule());

  auto* v2xi32 = p->ConvertType(20);
  EXPECT_TRUE(v2xi32->IsVector());
  EXPECT_TRUE(v2xi32->AsVector()->type()->IsI32());
  EXPECT_EQ(v2xi32->AsVector()->size(), 2u);

  auto* v3xi32 = p->ConvertType(30);
  EXPECT_TRUE(v3xi32->IsVector());
  EXPECT_TRUE(v3xi32->AsVector()->type()->IsI32());
  EXPECT_EQ(v3xi32->AsVector()->size(), 3u);

  auto* v4xi32 = p->ConvertType(40);
  EXPECT_TRUE(v4xi32->IsVector());
  EXPECT_TRUE(v4xi32->AsVector()->type()->IsI32());
  EXPECT_EQ(v4xi32->AsVector()->size(), 4u);

  EXPECT_TRUE(p->error().empty());
}

TEST_F(SpvParserTest, ConvertType_VecOverU32) {
  auto p = parser(test::Assemble(R"(
    %uint = OpTypeInt 32 0
    %20 = OpTypeVector %uint 2
    %30 = OpTypeVector %uint 3
    %40 = OpTypeVector %uint 4
  )"));
  EXPECT_TRUE(p->BuildAndParseInternalModule());

  auto* v2xu32 = p->ConvertType(20);
  EXPECT_TRUE(v2xu32->IsVector());
  EXPECT_TRUE(v2xu32->AsVector()->type()->IsU32());
  EXPECT_EQ(v2xu32->AsVector()->size(), 2u);

  auto* v3xu32 = p->ConvertType(30);
  EXPECT_TRUE(v3xu32->IsVector());
  EXPECT_TRUE(v3xu32->AsVector()->type()->IsU32());
  EXPECT_EQ(v3xu32->AsVector()->size(), 3u);

  auto* v4xu32 = p->ConvertType(40);
  EXPECT_TRUE(v4xu32->IsVector());
  EXPECT_TRUE(v4xu32->AsVector()->type()->IsU32());
  EXPECT_EQ(v4xu32->AsVector()->size(), 4u);

  EXPECT_TRUE(p->error().empty());
}

TEST_F(SpvParserTest, DISABLED_ConvertType_InvalidMatrixElement) {
  auto p = parser(test::Assemble(R"(
    %5 = OpTypePipe ReadOnly
    %10 = OpTypeVector %5 2
    %20 = OpTypeMatrix %10 2
  )"));
  EXPECT_TRUE(p->BuildAndParseInternalModule());

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
  EXPECT_TRUE(p->BuildAndParseInternalModule());

  auto* m22 = p->ConvertType(22);
  EXPECT_TRUE(m22->IsMatrix());
  EXPECT_TRUE(m22->AsMatrix()->type()->IsF32());
  EXPECT_EQ(m22->AsMatrix()->rows(), 2);
  EXPECT_EQ(m22->AsMatrix()->columns(), 2);

  auto* m23 = p->ConvertType(23);
  EXPECT_TRUE(m23->IsMatrix());
  EXPECT_TRUE(m23->AsMatrix()->type()->IsF32());
  EXPECT_EQ(m23->AsMatrix()->rows(), 2);
  EXPECT_EQ(m23->AsMatrix()->columns(), 3);

  auto* m24 = p->ConvertType(24);
  EXPECT_TRUE(m24->IsMatrix());
  EXPECT_TRUE(m24->AsMatrix()->type()->IsF32());
  EXPECT_EQ(m24->AsMatrix()->rows(), 2);
  EXPECT_EQ(m24->AsMatrix()->columns(), 4);

  auto* m32 = p->ConvertType(32);
  EXPECT_TRUE(m32->IsMatrix());
  EXPECT_TRUE(m32->AsMatrix()->type()->IsF32());
  EXPECT_EQ(m32->AsMatrix()->rows(), 3);
  EXPECT_EQ(m32->AsMatrix()->columns(), 2);

  auto* m33 = p->ConvertType(33);
  EXPECT_TRUE(m33->IsMatrix());
  EXPECT_TRUE(m33->AsMatrix()->type()->IsF32());
  EXPECT_EQ(m33->AsMatrix()->rows(), 3);
  EXPECT_EQ(m33->AsMatrix()->columns(), 3);

  auto* m34 = p->ConvertType(34);
  EXPECT_TRUE(m34->IsMatrix());
  EXPECT_TRUE(m34->AsMatrix()->type()->IsF32());
  EXPECT_EQ(m34->AsMatrix()->rows(), 3);
  EXPECT_EQ(m34->AsMatrix()->columns(), 4);

  auto* m42 = p->ConvertType(42);
  EXPECT_TRUE(m42->IsMatrix());
  EXPECT_TRUE(m42->AsMatrix()->type()->IsF32());
  EXPECT_EQ(m42->AsMatrix()->rows(), 4);
  EXPECT_EQ(m42->AsMatrix()->columns(), 2);

  auto* m43 = p->ConvertType(43);
  EXPECT_TRUE(m43->IsMatrix());
  EXPECT_TRUE(m43->AsMatrix()->type()->IsF32());
  EXPECT_EQ(m43->AsMatrix()->rows(), 4);
  EXPECT_EQ(m43->AsMatrix()->columns(), 3);

  auto* m44 = p->ConvertType(44);
  EXPECT_TRUE(m44->IsMatrix());
  EXPECT_TRUE(m44->AsMatrix()->type()->IsF32());
  EXPECT_EQ(m44->AsMatrix()->rows(), 4);
  EXPECT_EQ(m44->AsMatrix()->columns(), 4);

  EXPECT_TRUE(p->error().empty());
}

}  // namespace
}  // namespace spirv
}  // namespace reader
}  // namespace tint
