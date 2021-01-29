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

#include <string>
#include <vector>

#include "gmock/gmock.h"
#include "src/demangler.h"
#include "src/reader/spirv/function.h"
#include "src/reader/spirv/parser_impl.h"
#include "src/reader/spirv/parser_impl_test_helper.h"
#include "src/reader/spirv/spirv_tools_helpers_test.h"

namespace tint {
namespace reader {
namespace spirv {
namespace {

using ::testing::Eq;
using ::testing::HasSubstr;
using ::testing::Not;
using ::testing::StartsWith;

std::string Preamble() {
  return R"(
    OpCapability Shader
    OpCapability Sampled1D
    OpCapability Image1D
    OpCapability StorageImageExtendedFormats
    OpMemoryModel Logical Simple
  )";
}

std::string CommonBasicTypes() {
  return R"(
    %void = OpTypeVoid
    %voidfn = OpTypeFunction %void

    %float = OpTypeFloat 32
    %uint = OpTypeInt 32 0
    %int = OpTypeInt 32 1

    %int_0 = OpConstant %int 0
    %int_1 = OpConstant %int 1
    %int_2 = OpConstant %int 2
    %int_3 = OpConstant %int 3
    %int_4 = OpConstant %int 4
    %uint_0 = OpConstant %uint 0
    %uint_1 = OpConstant %uint 1
    %uint_2 = OpConstant %uint 2
    %uint_3 = OpConstant %uint 3
    %uint_4 = OpConstant %uint 4
    %uint_100 = OpConstant %uint 100

    %v2int = OpTypeVector %int 2
    %v3int = OpTypeVector %int 3
    %v4int = OpTypeVector %int 4
    %v2uint = OpTypeVector %uint 2
    %v3uint = OpTypeVector %uint 3
    %v4uint = OpTypeVector %uint 4
    %v2float = OpTypeVector %float 2
    %v3float = OpTypeVector %float 3
    %v4float = OpTypeVector %float 4

    %float_null = OpConstantNull %float
    %float_1 = OpConstant %float 1
    %float_2 = OpConstant %float 2
    %float_3 = OpConstant %float 3
    %float_4 = OpConstant %float 4
    %float_7 = OpConstant %float 7
    %v2float_null = OpConstantNull %v2float
    %v3float_null = OpConstantNull %v3float
    %v4float_null = OpConstantNull %v4float

    %the_vi12 = OpConstantComposite %v2int %int_1 %int_2
    %the_vi123 = OpConstantComposite %v3int %int_1 %int_2 %int_3
    %the_vi1234 = OpConstantComposite %v4int %int_1 %int_2 %int_3 %int_4

    %the_vu12 = OpConstantComposite %v2uint %uint_1 %uint_2
    %the_vu123 = OpConstantComposite %v3uint %uint_1 %uint_2 %uint_3
    %the_vu1234 = OpConstantComposite %v4uint %uint_1 %uint_2 %uint_3 %uint_4

    %the_vf12 = OpConstantComposite %v2float %float_1 %float_2
    %the_vf123 = OpConstantComposite %v3float %float_1 %float_2 %float_3
    %the_vf1234 = OpConstantComposite %v4float %float_1 %float_2 %float_3 %float_4


    %depth = OpConstant %float 0.2
  )";
}

std::string CommonImageTypes() {
  return R"(

; Define types for all sampler and texture types that can map to WGSL,
; modulo texel formats for storage textures. For now, we limit
; ourselves to 2-channel 32-bit texel formats.

; Because the SPIR-V reader also already generalizes so it can work with
; combined image-samplers, we also test that too.

    %sampler = OpTypeSampler

    ; sampled images
    %f_texture_1d          = OpTypeImage %float 1D   0 0 0 1 Unknown
    %f_texture_1d_array    = OpTypeImage %float 1D   0 1 0 1 Unknown
    %f_texture_2d          = OpTypeImage %float 2D   0 0 0 1 Unknown
    %f_texture_2d_ms       = OpTypeImage %float 2D   0 0 1 1 Unknown
    %f_texture_2d_array    = OpTypeImage %float 2D   0 1 0 1 Unknown
    %f_texture_2d_ms_array = OpTypeImage %float 2D   0 1 1 1 Unknown ; not in WebGPU
    %f_texture_3d          = OpTypeImage %float 3D   0 0 0 1 Unknown
    %f_texture_cube        = OpTypeImage %float Cube 0 0 0 1 Unknown
    %f_texture_cube_array  = OpTypeImage %float Cube 0 1 0 1 Unknown

    ; storage images
    %f_storage_1d         = OpTypeImage %float 1D   0 0 0 1 Rg32f
    %f_storage_1d_array   = OpTypeImage %float 1D   0 1 0 1 Rg32f
    %f_storage_2d         = OpTypeImage %float 2D   0 0 0 1 Rg32f
    %f_storage_2d_array   = OpTypeImage %float 2D   0 1 0 1 Rg32f
    %f_storage_3d         = OpTypeImage %float 3D   0 0 0 1 Rg32f

    ; Now all the same, but for unsigned integer sampled type.

    %u_texture_1d          = OpTypeImage %uint  1D   0 0 0 1 Unknown
    %u_texture_1d_array    = OpTypeImage %uint  1D   0 1 0 1 Unknown
    %u_texture_2d          = OpTypeImage %uint  2D   0 0 0 1 Unknown
    %u_texture_2d_ms       = OpTypeImage %uint  2D   0 0 1 1 Unknown
    %u_texture_2d_array    = OpTypeImage %uint  2D   0 1 0 1 Unknown
    %u_texture_2d_ms_array = OpTypeImage %uint  2D   0 1 1 1 Unknown ; not in WebGPU
    %u_texture_3d          = OpTypeImage %uint  3D   0 0 0 1 Unknown
    %u_texture_cube        = OpTypeImage %uint  Cube 0 0 0 1 Unknown
    %u_texture_cube_array  = OpTypeImage %uint  Cube 0 1 0 1 Unknown

    %u_storage_1d         = OpTypeImage %uint  1D   0 0 0 1 Rg32ui
    %u_storage_1d_array   = OpTypeImage %uint  1D   0 1 0 1 Rg32ui
    %u_storage_2d         = OpTypeImage %uint  2D   0 0 0 1 Rg32ui
    %u_storage_2d_array   = OpTypeImage %uint  2D   0 1 0 1 Rg32ui
    %u_storage_3d         = OpTypeImage %uint  3D   0 0 0 1 Rg32ui

    ; Now all the same, but for signed integer sampled type.

    %i_texture_1d          = OpTypeImage %int  1D   0 0 0 1 Unknown
    %i_texture_1d_array    = OpTypeImage %int  1D   0 1 0 1 Unknown
    %i_texture_2d          = OpTypeImage %int  2D   0 0 0 1 Unknown
    %i_texture_2d_ms       = OpTypeImage %int  2D   0 0 1 1 Unknown
    %i_texture_2d_array    = OpTypeImage %int  2D   0 1 0 1 Unknown
    %i_texture_2d_ms_array = OpTypeImage %int  2D   0 1 1 1 Unknown ; not in WebGPU
    %i_texture_3d          = OpTypeImage %int  3D   0 0 0 1 Unknown
    %i_texture_cube        = OpTypeImage %int  Cube 0 0 0 1 Unknown
    %i_texture_cube_array  = OpTypeImage %int  Cube 0 1 0 1 Unknown

    %i_storage_1d         = OpTypeImage %int  1D   0 0 0 1 Rg32i
    %i_storage_1d_array   = OpTypeImage %int  1D   0 1 0 1 Rg32i
    %i_storage_2d         = OpTypeImage %int  2D   0 0 0 1 Rg32i
    %i_storage_2d_array   = OpTypeImage %int  2D   0 1 0 1 Rg32i
    %i_storage_3d         = OpTypeImage %int  3D   0 0 0 1 Rg32i

    ;; Now pointers to each of the above, so we can declare variables for them.

    %ptr_sampler = OpTypePointer UniformConstant %sampler

    %ptr_f_texture_1d          = OpTypePointer UniformConstant %f_texture_1d
    %ptr_f_texture_1d_array    = OpTypePointer UniformConstant %f_texture_1d_array
    %ptr_f_texture_2d          = OpTypePointer UniformConstant %f_texture_2d
    %ptr_f_texture_2d_ms       = OpTypePointer UniformConstant %f_texture_2d_ms
    %ptr_f_texture_2d_array    = OpTypePointer UniformConstant %f_texture_2d_array
    %ptr_f_texture_2d_ms_array = OpTypePointer UniformConstant %f_texture_2d_ms_array
    %ptr_f_texture_3d          = OpTypePointer UniformConstant %f_texture_3d
    %ptr_f_texture_cube        = OpTypePointer UniformConstant %f_texture_cube
    %ptr_f_texture_cube_array  = OpTypePointer UniformConstant %f_texture_cube_array

    ; storage images
    %ptr_f_storage_1d         = OpTypePointer UniformConstant %f_storage_1d
    %ptr_f_storage_1d_array   = OpTypePointer UniformConstant %f_storage_1d_array
    %ptr_f_storage_2d         = OpTypePointer UniformConstant %f_storage_2d
    %ptr_f_storage_2d_array   = OpTypePointer UniformConstant %f_storage_2d_array
    %ptr_f_storage_3d         = OpTypePointer UniformConstant %f_storage_3d

    ; Now all the same, but for unsigned integer sampled type.

    %ptr_u_texture_1d          = OpTypePointer UniformConstant %u_texture_1d
    %ptr_u_texture_1d_array    = OpTypePointer UniformConstant %u_texture_1d_array
    %ptr_u_texture_2d          = OpTypePointer UniformConstant %u_texture_2d
    %ptr_u_texture_2d_ms       = OpTypePointer UniformConstant %u_texture_2d_ms
    %ptr_u_texture_2d_array    = OpTypePointer UniformConstant %u_texture_2d_array
    %ptr_u_texture_2d_ms_array = OpTypePointer UniformConstant %u_texture_2d_ms_array
    %ptr_u_texture_3d          = OpTypePointer UniformConstant %u_texture_3d
    %ptr_u_texture_cube        = OpTypePointer UniformConstant %u_texture_cube
    %ptr_u_texture_cube_array  = OpTypePointer UniformConstant %u_texture_cube_array

    %ptr_u_storage_1d         = OpTypePointer UniformConstant %u_storage_1d
    %ptr_u_storage_1d_array   = OpTypePointer UniformConstant %u_storage_1d_array
    %ptr_u_storage_2d         = OpTypePointer UniformConstant %u_storage_2d
    %ptr_u_storage_2d_array   = OpTypePointer UniformConstant %u_storage_2d_array
    %ptr_u_storage_3d         = OpTypePointer UniformConstant %u_storage_3d

    ; Now all the same, but for signed integer sampled type.

    %ptr_i_texture_1d          = OpTypePointer UniformConstant %i_texture_1d
    %ptr_i_texture_1d_array    = OpTypePointer UniformConstant %i_texture_1d_array
    %ptr_i_texture_2d          = OpTypePointer UniformConstant %i_texture_2d
    %ptr_i_texture_2d_ms       = OpTypePointer UniformConstant %i_texture_2d_ms
    %ptr_i_texture_2d_array    = OpTypePointer UniformConstant %i_texture_2d_array
    %ptr_i_texture_2d_ms_array = OpTypePointer UniformConstant %i_texture_2d_ms_array
    %ptr_i_texture_3d          = OpTypePointer UniformConstant %i_texture_3d
    %ptr_i_texture_cube        = OpTypePointer UniformConstant %i_texture_cube
    %ptr_i_texture_cube_array  = OpTypePointer UniformConstant %i_texture_cube_array

    %ptr_i_storage_1d         = OpTypePointer UniformConstant %i_storage_1d
    %ptr_i_storage_1d_array   = OpTypePointer UniformConstant %i_storage_1d_array
    %ptr_i_storage_2d         = OpTypePointer UniformConstant %i_storage_2d
    %ptr_i_storage_2d_array   = OpTypePointer UniformConstant %i_storage_2d_array
    %ptr_i_storage_3d         = OpTypePointer UniformConstant %i_storage_3d

  )";
}

std::string CommonTypes() {
  return CommonBasicTypes() + CommonImageTypes();
}

TEST_F(SpvParserTest,
       GetMemoryObjectDeclarationForHandle_WellFormedButNotAHandle) {
  const auto assembly = Preamble() + CommonTypes() + R"(
     %10 = OpConstantNull %ptr_sampler
     %20 = OpConstantNull %ptr_f_texture_1d
  )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildInternalModule()) << assembly;
  const auto* sampler = p->GetMemoryObjectDeclarationForHandle(10, false);
  const auto* image = p->GetMemoryObjectDeclarationForHandle(20, true);

  EXPECT_EQ(sampler, nullptr);
  EXPECT_EQ(image, nullptr);
  EXPECT_TRUE(p->error().empty());
}

TEST_F(SpvParserTest, GetMemoryObjectDeclarationForHandle_Variable_Direct) {
  const auto assembly = Preamble() + CommonTypes() + R"(
     %10 = OpVariable %ptr_sampler UniformConstant
     %20 = OpVariable %ptr_f_texture_1d UniformConstant
  )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildInternalModule());
  EXPECT_TRUE(p->error().empty());
  const auto* sampler = p->GetMemoryObjectDeclarationForHandle(10, false);
  const auto* image = p->GetMemoryObjectDeclarationForHandle(20, true);

  ASSERT_TRUE(sampler != nullptr);
  EXPECT_EQ(sampler->result_id(), 10u);

  ASSERT_TRUE(image != nullptr);
  EXPECT_EQ(image->result_id(), 20u);
}

TEST_F(SpvParserTest,
       GetMemoryObjectDeclarationForHandle_Variable_AccessChain) {
  // Show that we would generalize to arrays of handles, even though that
  // is not supported in WGSL MVP.
  const auto assembly = Preamble() + CommonTypes() + R"(

     %sampler_array = OpTypeArray %sampler %uint_100
     %image_array = OpTypeArray %f_texture_1d %uint_100

     %ptr_sampler_array = OpTypePointer UniformConstant %sampler_array
     %ptr_image_array = OpTypePointer UniformConstant %image_array

     %10 = OpVariable %ptr_sampler_array UniformConstant
     %20 = OpVariable %ptr_image_array UniformConstant

     %main = OpFunction %void None %voidfn
     %entry = OpLabel

     %110 = OpAccessChain %ptr_sampler %10 %uint_1
     %120 = OpAccessChain %ptr_f_texture_1d %20 %uint_2

     OpReturn
     OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildInternalModule());
  EXPECT_TRUE(p->error().empty());
  const auto* sampler = p->GetMemoryObjectDeclarationForHandle(110, false);
  const auto* image = p->GetMemoryObjectDeclarationForHandle(120, true);

  ASSERT_TRUE(sampler != nullptr);
  EXPECT_EQ(sampler->result_id(), 10u);

  ASSERT_TRUE(image != nullptr);
  EXPECT_EQ(image->result_id(), 20u);
}

TEST_F(SpvParserTest,
       GetMemoryObjectDeclarationForHandle_Variable_InBoundsAccessChain) {
  const auto assembly = Preamble() + CommonTypes() + R"(

     %sampler_array = OpTypeArray %sampler %uint_100
     %image_array = OpTypeArray %f_texture_1d %uint_100

     %ptr_sampler_array = OpTypePointer UniformConstant %sampler_array
     %ptr_image_array = OpTypePointer UniformConstant %image_array

     %10 = OpVariable %ptr_sampler_array UniformConstant
     %20 = OpVariable %ptr_image_array UniformConstant

     %main = OpFunction %void None %voidfn
     %entry = OpLabel

     %110 = OpInBoundsAccessChain %ptr_sampler %10 %uint_1
     %120 = OpInBoundsAccessChain %ptr_f_texture_1d %20 %uint_2

     OpReturn
     OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildInternalModule());
  EXPECT_TRUE(p->error().empty());
  const auto* sampler = p->GetMemoryObjectDeclarationForHandle(110, false);
  const auto* image = p->GetMemoryObjectDeclarationForHandle(120, true);

  ASSERT_TRUE(sampler != nullptr);
  EXPECT_EQ(sampler->result_id(), 10u);

  ASSERT_TRUE(image != nullptr);
  EXPECT_EQ(image->result_id(), 20u);
}

TEST_F(SpvParserTest,
       GetMemoryObjectDeclarationForHandle_Variable_PtrAccessChain) {
  // Show that we would generalize to arrays of handles, even though that
  // is not supported in WGSL MVP.
  const auto assembly = Preamble() + CommonTypes() + R"(

     %sampler_array = OpTypeArray %sampler %uint_100
     %image_array = OpTypeArray %f_texture_1d %uint_100

     %ptr_sampler_array = OpTypePointer UniformConstant %sampler_array
     %ptr_image_array = OpTypePointer UniformConstant %image_array

     %10 = OpVariable %ptr_sampler_array UniformConstant
     %20 = OpVariable %ptr_image_array UniformConstant

     %main = OpFunction %void None %voidfn
     %entry = OpLabel

     %110 = OpPtrAccessChain %ptr_sampler %10 %uint_1 %uint_1
     %120 = OpPtrAccessChain %ptr_f_texture_1d %20 %uint_1 %uint_2

     OpReturn
     OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildInternalModule());
  EXPECT_TRUE(p->error().empty());
  const auto* sampler = p->GetMemoryObjectDeclarationForHandle(110, false);
  const auto* image = p->GetMemoryObjectDeclarationForHandle(120, true);

  ASSERT_TRUE(sampler != nullptr);
  EXPECT_EQ(sampler->result_id(), 10u);

  ASSERT_TRUE(image != nullptr);
  EXPECT_EQ(image->result_id(), 20u);
}

TEST_F(SpvParserTest,
       GetMemoryObjectDeclarationForHandle_Variable_InBoundsPtrAccessChain) {
  const auto assembly = Preamble() + CommonTypes() + R"(

     %sampler_array = OpTypeArray %sampler %uint_100
     %image_array = OpTypeArray %f_texture_1d %uint_100

     %ptr_sampler_array = OpTypePointer UniformConstant %sampler_array
     %ptr_image_array = OpTypePointer UniformConstant %image_array

     %10 = OpVariable %ptr_sampler_array UniformConstant
     %20 = OpVariable %ptr_image_array UniformConstant

     %main = OpFunction %void None %voidfn
     %entry = OpLabel

     %110 = OpInBoundsPtrAccessChain %ptr_sampler %10 %uint_1 %uint_1
     %120 = OpInBoundsPtrAccessChain %ptr_f_texture_1d %20 %uint_1 %uint_2

     OpReturn
     OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildInternalModule());
  EXPECT_TRUE(p->error().empty());
  const auto* sampler = p->GetMemoryObjectDeclarationForHandle(110, false);
  const auto* image = p->GetMemoryObjectDeclarationForHandle(120, true);

  ASSERT_TRUE(sampler != nullptr);
  EXPECT_EQ(sampler->result_id(), 10u);

  ASSERT_TRUE(image != nullptr);
  EXPECT_EQ(image->result_id(), 20u);
}

TEST_F(SpvParserTest, GetMemoryObjectDeclarationForHandle_Variable_CopyObject) {
  const auto assembly = Preamble() + CommonTypes() + R"(

     %10 = OpVariable %ptr_sampler UniformConstant
     %20 = OpVariable %ptr_f_texture_1d UniformConstant

     %main = OpFunction %void None %voidfn
     %entry = OpLabel

     %110 = OpCopyObject %ptr_sampler %10
     %120 = OpCopyObject %ptr_f_texture_1d %20

     OpReturn
     OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildInternalModule());
  EXPECT_TRUE(p->error().empty());
  const auto* sampler = p->GetMemoryObjectDeclarationForHandle(110, false);
  const auto* image = p->GetMemoryObjectDeclarationForHandle(120, true);

  ASSERT_TRUE(sampler != nullptr);
  EXPECT_EQ(sampler->result_id(), 10u);

  ASSERT_TRUE(image != nullptr);
  EXPECT_EQ(image->result_id(), 20u);
}

TEST_F(SpvParserTest, GetMemoryObjectDeclarationForHandle_Variable_Load) {
  const auto assembly = Preamble() + CommonTypes() + R"(

     %10 = OpVariable %ptr_sampler UniformConstant
     %20 = OpVariable %ptr_f_texture_1d UniformConstant

     %main = OpFunction %void None %voidfn
     %entry = OpLabel

     %110 = OpLoad %sampler %10
     %120 = OpLoad %f_texture_1d %20

     OpReturn
     OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildInternalModule());
  EXPECT_TRUE(p->error().empty());
  const auto* sampler = p->GetMemoryObjectDeclarationForHandle(110, false);
  const auto* image = p->GetMemoryObjectDeclarationForHandle(120, true);

  ASSERT_TRUE(sampler != nullptr);
  EXPECT_EQ(sampler->result_id(), 10u);

  ASSERT_TRUE(image != nullptr);
  EXPECT_EQ(image->result_id(), 20u);
}

TEST_F(SpvParserTest,
       GetMemoryObjectDeclarationForHandle_Variable_SampledImage) {
  // Trace through the sampled image instruction, but in two different
  // directions.
  const auto assembly = Preamble() + CommonTypes() + R"(
     %sampled_image_type = OpTypeSampledImage %f_texture_1d

     %10 = OpVariable %ptr_sampler UniformConstant
     %20 = OpVariable %ptr_f_texture_1d UniformConstant

     %main = OpFunction %void None %voidfn
     %entry = OpLabel

     %s = OpLoad %sampler %10
     %im = OpLoad %f_texture_1d %20
     %100 = OpSampledImage %sampled_image_type %im %s

     OpReturn
     OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildInternalModule());
  EXPECT_TRUE(p->error().empty());
  const auto* sampler = p->GetMemoryObjectDeclarationForHandle(100, false);
  const auto* image = p->GetMemoryObjectDeclarationForHandle(100, true);

  ASSERT_TRUE(sampler != nullptr);
  EXPECT_EQ(sampler->result_id(), 10u);

  ASSERT_TRUE(image != nullptr);
  EXPECT_EQ(image->result_id(), 20u);
}

TEST_F(SpvParserTest, GetMemoryObjectDeclarationForHandle_Variable_Image) {
  const auto assembly = Preamble() + CommonTypes() + R"(
     %sampled_image_type = OpTypeSampledImage %f_texture_1d

     %10 = OpVariable %ptr_sampler UniformConstant
     %20 = OpVariable %ptr_f_texture_1d UniformConstant

     %main = OpFunction %void None %voidfn
     %entry = OpLabel

     %s = OpLoad %sampler %10
     %im = OpLoad %f_texture_1d %20
     %100 = OpSampledImage %sampled_image_type %im %s
     %200 = OpImage %im %100

     OpReturn
     OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildInternalModule());
  EXPECT_TRUE(p->error().empty());

  const auto* image = p->GetMemoryObjectDeclarationForHandle(200, true);
  ASSERT_TRUE(image != nullptr);
  EXPECT_EQ(image->result_id(), 20u);
}

TEST_F(SpvParserTest, GetMemoryObjectDeclarationForHandle_FuncParam_Direct) {
  const auto assembly = Preamble() + CommonTypes() + R"(
     %fty = OpTypeFunction %void %ptr_sampler %ptr_f_texture_1d

     %func = OpFunction %void None %fty
     %10 = OpFunctionParameter %ptr_sampler
     %20 = OpFunctionParameter %ptr_f_texture_1d
     %entry = OpLabel
     OpReturn
  )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildInternalModule());
  EXPECT_TRUE(p->error().empty());
  const auto* sampler = p->GetMemoryObjectDeclarationForHandle(10, false);
  const auto* image = p->GetMemoryObjectDeclarationForHandle(20, true);

  ASSERT_TRUE(sampler != nullptr);
  EXPECT_EQ(sampler->result_id(), 10u);

  ASSERT_TRUE(image != nullptr);
  EXPECT_EQ(image->result_id(), 20u);
}

TEST_F(SpvParserTest,
       GetMemoryObjectDeclarationForHandle_FuncParam_AccessChain) {
  // Show that we would generalize to arrays of handles, even though that
  // is not supported in WGSL MVP.
  const auto assembly = Preamble() + CommonTypes() + R"(
     %sampler_array = OpTypeArray %sampler %uint_100
     %image_array = OpTypeArray %f_texture_1d %uint_100

     %ptr_sampler_array = OpTypePointer UniformConstant %sampler_array
     %ptr_image_array = OpTypePointer UniformConstant %image_array

     %fty = OpTypeFunction %void %ptr_sampler_array %ptr_image_array

     %func = OpFunction %void None %fty
     %10 = OpFunctionParameter %ptr_sampler_array
     %20 = OpFunctionParameter %ptr_image_array
     %entry = OpLabel

     %110 = OpAccessChain %ptr_sampler %10 %uint_1
     %120 = OpAccessChain %ptr_f_texture_1d %20 %uint_2

     OpReturn
     OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildInternalModule());
  EXPECT_TRUE(p->error().empty());
  const auto* sampler = p->GetMemoryObjectDeclarationForHandle(110, false);
  const auto* image = p->GetMemoryObjectDeclarationForHandle(120, true);

  ASSERT_TRUE(sampler != nullptr);
  EXPECT_EQ(sampler->result_id(), 10u);

  ASSERT_TRUE(image != nullptr);
  EXPECT_EQ(image->result_id(), 20u);
}

TEST_F(SpvParserTest,
       GetMemoryObjectDeclarationForHandle_FuncParam_InBoundsAccessChain) {
  const auto assembly = Preamble() + CommonTypes() + R"(
     %sampler_array = OpTypeArray %sampler %uint_100
     %image_array = OpTypeArray %f_texture_1d %uint_100

     %ptr_sampler_array = OpTypePointer UniformConstant %sampler_array
     %ptr_image_array = OpTypePointer UniformConstant %image_array

     %fty = OpTypeFunction %void %ptr_sampler_array %ptr_image_array

     %func = OpFunction %void None %fty
     %10 = OpFunctionParameter %ptr_sampler_array
     %20 = OpFunctionParameter %ptr_image_array
     %entry = OpLabel

     %110 = OpInBoundsAccessChain %ptr_sampler %10 %uint_1
     %120 = OpInBoundsAccessChain %ptr_f_texture_1d %20 %uint_2

     OpReturn
     OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildInternalModule());
  EXPECT_TRUE(p->error().empty());
  const auto* sampler = p->GetMemoryObjectDeclarationForHandle(110, false);
  const auto* image = p->GetMemoryObjectDeclarationForHandle(120, true);

  ASSERT_TRUE(sampler != nullptr);
  EXPECT_EQ(sampler->result_id(), 10u);

  ASSERT_TRUE(image != nullptr);
  EXPECT_EQ(image->result_id(), 20u);
}

TEST_F(SpvParserTest,
       GetMemoryObjectDeclarationForHandle_FuncParam_PtrAccessChain) {
  // Show that we would generalize to arrays of handles, even though that
  // is not supported in WGSL MVP.
  const auto assembly = Preamble() + CommonTypes() + R"(
     %sampler_array = OpTypeArray %sampler %uint_100
     %image_array = OpTypeArray %f_texture_1d %uint_100

     %ptr_sampler_array = OpTypePointer UniformConstant %sampler_array
     %ptr_image_array = OpTypePointer UniformConstant %image_array

     %fty = OpTypeFunction %void %ptr_sampler_array %ptr_image_array

     %func = OpFunction %void None %fty
     %10 = OpFunctionParameter %ptr_sampler_array
     %20 = OpFunctionParameter %ptr_image_array
     %entry = OpLabel

     %110 = OpPtrAccessChain %ptr_sampler %10 %uint_1 %uint_1
     %120 = OpPtrAccessChain %ptr_f_texture_1d %20 %uint_1 %uint_2

     OpReturn
     OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildInternalModule());
  EXPECT_TRUE(p->error().empty());
  const auto* sampler = p->GetMemoryObjectDeclarationForHandle(110, false);
  const auto* image = p->GetMemoryObjectDeclarationForHandle(120, true);

  ASSERT_TRUE(sampler != nullptr);
  EXPECT_EQ(sampler->result_id(), 10u);

  ASSERT_TRUE(image != nullptr);
  EXPECT_EQ(image->result_id(), 20u);
}

TEST_F(SpvParserTest,
       GetMemoryObjectDeclarationForHandle_FuncParam_InBoundsPtrAccessChain) {
  const auto assembly = Preamble() + CommonTypes() + R"(
     %sampler_array = OpTypeArray %sampler %uint_100
     %image_array = OpTypeArray %f_texture_1d %uint_100

     %ptr_sampler_array = OpTypePointer UniformConstant %sampler_array
     %ptr_image_array = OpTypePointer UniformConstant %image_array

     %fty = OpTypeFunction %void %ptr_sampler_array %ptr_image_array

     %func = OpFunction %void None %fty
     %10 = OpFunctionParameter %ptr_sampler_array
     %20 = OpFunctionParameter %ptr_image_array
     %entry = OpLabel

     %110 = OpInBoundsPtrAccessChain %ptr_sampler %10 %uint_1 %uint_1
     %120 = OpInBoundsPtrAccessChain %ptr_f_texture_1d %20 %uint_1 %uint_2

     OpReturn
     OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildInternalModule());
  EXPECT_TRUE(p->error().empty());
  const auto* sampler = p->GetMemoryObjectDeclarationForHandle(110, false);
  const auto* image = p->GetMemoryObjectDeclarationForHandle(120, true);

  ASSERT_TRUE(sampler != nullptr);
  EXPECT_EQ(sampler->result_id(), 10u);

  ASSERT_TRUE(image != nullptr);
  EXPECT_EQ(image->result_id(), 20u);
}

TEST_F(SpvParserTest,
       GetMemoryObjectDeclarationForHandle_FuncParam_CopyObject) {
  const auto assembly = Preamble() + CommonTypes() + R"(
     %fty = OpTypeFunction %void %ptr_sampler %ptr_f_texture_1d

     %func = OpFunction %void None %fty
     %10 = OpFunctionParameter %ptr_sampler
     %20 = OpFunctionParameter %ptr_f_texture_1d
     %entry = OpLabel

     %110 = OpCopyObject %ptr_sampler %10
     %120 = OpCopyObject %ptr_f_texture_1d %20

     OpReturn
     OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildInternalModule());
  EXPECT_TRUE(p->error().empty());
  const auto* sampler = p->GetMemoryObjectDeclarationForHandle(110, false);
  const auto* image = p->GetMemoryObjectDeclarationForHandle(120, true);

  ASSERT_TRUE(sampler != nullptr);
  EXPECT_EQ(sampler->result_id(), 10u);

  ASSERT_TRUE(image != nullptr);
  EXPECT_EQ(image->result_id(), 20u);
}

TEST_F(SpvParserTest, GetMemoryObjectDeclarationForHandle_FuncParam_Load) {
  const auto assembly = Preamble() + CommonTypes() + R"(
     %fty = OpTypeFunction %void %ptr_sampler %ptr_f_texture_1d

     %func = OpFunction %void None %fty
     %10 = OpFunctionParameter %ptr_sampler
     %20 = OpFunctionParameter %ptr_f_texture_1d
     %entry = OpLabel

     %110 = OpLoad %sampler %10
     %120 = OpLoad %f_texture_1d %20

     OpReturn
     OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildInternalModule());
  EXPECT_TRUE(p->error().empty());
  const auto* sampler = p->GetMemoryObjectDeclarationForHandle(110, false);
  const auto* image = p->GetMemoryObjectDeclarationForHandle(120, true);

  ASSERT_TRUE(sampler != nullptr);
  EXPECT_EQ(sampler->result_id(), 10u);

  ASSERT_TRUE(image != nullptr);
  EXPECT_EQ(image->result_id(), 20u);
}

TEST_F(SpvParserTest,
       GetMemoryObjectDeclarationForHandle_FuncParam_SampledImage) {
  // Trace through the sampled image instruction, but in two different
  // directions.
  const auto assembly = Preamble() + CommonTypes() + R"(
     %sampled_image_type = OpTypeSampledImage %f_texture_1d

     %fty = OpTypeFunction %void %ptr_sampler %ptr_f_texture_1d

     %func = OpFunction %void None %fty
     %10 = OpFunctionParameter %ptr_sampler
     %20 = OpFunctionParameter %ptr_f_texture_1d
     %entry = OpLabel

     %s = OpLoad %sampler %10
     %im = OpLoad %f_texture_1d %20
     %100 = OpSampledImage %sampled_image_type %im %s

     OpReturn
     OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildInternalModule());
  EXPECT_TRUE(p->error().empty());
  const auto* sampler = p->GetMemoryObjectDeclarationForHandle(100, false);
  const auto* image = p->GetMemoryObjectDeclarationForHandle(100, true);

  ASSERT_TRUE(sampler != nullptr);
  EXPECT_EQ(sampler->result_id(), 10u);

  ASSERT_TRUE(image != nullptr);
  EXPECT_EQ(image->result_id(), 20u);
}

TEST_F(SpvParserTest, GetMemoryObjectDeclarationForHandle_FuncParam_Image) {
  const auto assembly = Preamble() + CommonTypes() + R"(
     %sampled_image_type = OpTypeSampledImage %f_texture_1d

     %fty = OpTypeFunction %void %ptr_sampler %ptr_f_texture_1d

     %func = OpFunction %void None %fty
     %10 = OpFunctionParameter %ptr_sampler
     %20 = OpFunctionParameter %ptr_f_texture_1d
     %entry = OpLabel

     %s = OpLoad %sampler %10
     %im = OpLoad %f_texture_1d %20
     %100 = OpSampledImage %sampled_image_type %im %s
     %200 = OpImage %im %100

     OpReturn
     OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildInternalModule());
  EXPECT_TRUE(p->error().empty());

  const auto* image = p->GetMemoryObjectDeclarationForHandle(200, true);
  ASSERT_TRUE(image != nullptr);
  EXPECT_EQ(image->result_id(), 20u);
}

// Test RegisterHandleUsage, sampled image cases

struct UsageImageAccessCase {
  std::string inst;
  std::string expected_sampler_usage;
  std::string expected_image_usage;
};
inline std::ostream& operator<<(std::ostream& out,
                                const UsageImageAccessCase& c) {
  out << "UsageImageAccessCase(" << c.inst << ", " << c.expected_sampler_usage
      << ", " << c.expected_image_usage << ")";
  return out;
}

using SpvParserTest_RegisterHandleUsage_SampledImage =
    SpvParserTestBase<::testing::TestWithParam<UsageImageAccessCase>>;

TEST_P(SpvParserTest_RegisterHandleUsage_SampledImage, Variable) {
  const auto assembly = Preamble() + CommonTypes() + R"(
     %si_ty = OpTypeSampledImage %f_texture_2d
     %coords = OpConstantNull %v2float

     %10 = OpVariable %ptr_sampler UniformConstant
     %20 = OpVariable %ptr_f_texture_2d UniformConstant

     %main = OpFunction %void None %voidfn
     %entry = OpLabel

     %sam = OpLoad %sampler %10
     %im = OpLoad %f_texture_2d %20
     %sampled_image = OpSampledImage %si_ty %im %sam
)" + GetParam().inst + R"(

     OpReturn
     OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildInternalModule());
  EXPECT_TRUE(p->RegisterHandleUsage());
  EXPECT_TRUE(p->error().empty());
  Usage su = p->GetHandleUsage(10);
  Usage iu = p->GetHandleUsage(20);

  EXPECT_THAT(su.to_str(), Eq(GetParam().expected_sampler_usage));
  EXPECT_THAT(iu.to_str(), Eq(GetParam().expected_image_usage));
}

TEST_P(SpvParserTest_RegisterHandleUsage_SampledImage, FunctionParam) {
  const auto assembly = Preamble() + CommonTypes() + R"(
     %f_ty = OpTypeFunction %void %ptr_sampler %ptr_f_texture_2d
     %si_ty = OpTypeSampledImage %f_texture_2d
     %coords = OpConstantNull %v2float
     %component = OpConstant %uint 1

     %10 = OpVariable %ptr_sampler UniformConstant
     %20 = OpVariable %ptr_f_texture_2d UniformConstant

     %func = OpFunction %void None %f_ty
     %110 = OpFunctionParameter %ptr_sampler
     %120 = OpFunctionParameter %ptr_f_texture_2d
     %func_entry = OpLabel
     %sam = OpLoad %sampler %110
     %im = OpLoad %f_texture_2d %120
     %sampled_image = OpSampledImage %si_ty %im %sam

)" + GetParam().inst + R"(

     OpReturn
     OpFunctionEnd

     %main = OpFunction %void None %voidfn
     %entry = OpLabel
     %foo = OpFunctionCall %void %func %10 %20
     OpReturn
     OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildInternalModule()) << p->error() << assembly << std::endl;
  EXPECT_TRUE(p->RegisterHandleUsage()) << p->error() << assembly << std::endl;
  EXPECT_TRUE(p->error().empty()) << p->error() << assembly << std::endl;
  Usage su = p->GetHandleUsage(10);
  Usage iu = p->GetHandleUsage(20);

  EXPECT_THAT(su.to_str(), Eq(GetParam().expected_sampler_usage));
  EXPECT_THAT(iu.to_str(), Eq(GetParam().expected_image_usage));
}

INSTANTIATE_TEST_SUITE_P(
    Samples,
    SpvParserTest_RegisterHandleUsage_SampledImage,
    ::testing::Values(

        // OpImageGather
        UsageImageAccessCase{"%result = OpImageGather "
                             "%v4float %sampled_image %coords %uint_1",
                             "Usage(Sampler( ))",
                             "Usage(Texture( is_sampled ))"},
        // OpImageDrefGather
        UsageImageAccessCase{"%result = OpImageDrefGather "
                             "%v4float %sampled_image %coords %depth",
                             "Usage(Sampler( comparison ))",
                             "Usage(Texture( is_sampled depth ))"},

        // Sample the texture.

        // OpImageSampleImplicitLod
        UsageImageAccessCase{"%result = OpImageSampleImplicitLod "
                             "%v4float %sampled_image %coords",
                             "Usage(Sampler( ))",
                             "Usage(Texture( is_sampled ))"},
        // OpImageSampleExplicitLod
        UsageImageAccessCase{"%result = OpImageSampleExplicitLod "
                             "%v4float %sampled_image %coords Lod %float_null",
                             "Usage(Sampler( ))",
                             "Usage(Texture( is_sampled ))"},
        // OpImageSampleDrefImplicitLod
        UsageImageAccessCase{"%result = OpImageSampleDrefImplicitLod "
                             "%float %sampled_image %coords %depth",
                             "Usage(Sampler( comparison ))",
                             "Usage(Texture( is_sampled depth ))"},
        // OpImageSampleDrefExplicitLod
        UsageImageAccessCase{
            "%result = OpImageSampleDrefExplicitLod "
            "%float %sampled_image %coords %depth Lod %float_null",
            "Usage(Sampler( comparison ))",
            "Usage(Texture( is_sampled depth ))"},

        // Sample the texture, with *Proj* variants, even though WGSL doesn't
        // support them.

        // OpImageSampleProjImplicitLod
        UsageImageAccessCase{"%result = OpImageSampleProjImplicitLod "
                             "%v4float %sampled_image %coords",
                             "Usage(Sampler( ))",
                             "Usage(Texture( is_sampled ))"},
        // OpImageSampleProjExplicitLod
        UsageImageAccessCase{"%result = OpImageSampleProjExplicitLod "
                             "%v4float %sampled_image %coords Lod %float_null",
                             "Usage(Sampler( ))",
                             "Usage(Texture( is_sampled ))"},
        // OpImageSampleProjDrefImplicitLod
        UsageImageAccessCase{"%result = OpImageSampleProjDrefImplicitLod "
                             "%float %sampled_image %coords %depth",
                             "Usage(Sampler( comparison ))",
                             "Usage(Texture( is_sampled depth ))"},
        // OpImageSampleProjDrefExplicitLod
        UsageImageAccessCase{
            "%result = OpImageSampleProjDrefExplicitLod "
            "%float %sampled_image %coords %depth Lod %float_null",
            "Usage(Sampler( comparison ))",
            "Usage(Texture( is_sampled depth ))"},

        // OpImageQueryLod
        UsageImageAccessCase{
            "%result = OpImageQueryLod %v2float %sampled_image %coords",
            "Usage(Sampler( ))", "Usage(Texture( is_sampled ))"}));

// Test RegisterHandleUsage, raw image cases.
// For these we test the use of an image value directly, and not combined
// with the sampler. The image still could be of sampled image type.

struct UsageRawImageCase {
  std::string type;  // Example: f_storage_1d or f_texture_1d
  std::string inst;
  std::string expected_image_usage;
};
inline std::ostream& operator<<(std::ostream& out, const UsageRawImageCase& c) {
  out << "UsageRawImageCase(" << c.type << ", " << c.inst << ", "
      << c.expected_image_usage << ")";
  return out;
}

using SpvParserTest_RegisterHandleUsage_RawImage =
    SpvParserTestBase<::testing::TestWithParam<UsageRawImageCase>>;

TEST_P(SpvParserTest_RegisterHandleUsage_RawImage, Variable) {
  const auto assembly = Preamble() + CommonTypes() + R"(
     %20 = OpVariable %ptr_)" +
                        GetParam().type + R"( UniformConstant

     %main = OpFunction %void None %voidfn
     %entry = OpLabel

     %im = OpLoad %)" + GetParam().type +
                        R"( %20
)" + GetParam().inst + R"(

     OpReturn
     OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildInternalModule());
  EXPECT_TRUE(p->RegisterHandleUsage());
  EXPECT_TRUE(p->error().empty());

  Usage iu = p->GetHandleUsage(20);
  EXPECT_THAT(iu.to_str(), Eq(GetParam().expected_image_usage));

  Usage su = p->GetHandleUsage(20);
}

TEST_P(SpvParserTest_RegisterHandleUsage_RawImage, FunctionParam) {
  const auto assembly = Preamble() + CommonTypes() + R"(
     %f_ty = OpTypeFunction %void %ptr_)" +
                        GetParam().type + R"(

     %20 = OpVariable %ptr_)" +
                        GetParam().type + R"( UniformConstant

     %func = OpFunction %void None %f_ty
     %i_param = OpFunctionParameter %ptr_)" +
                        GetParam().type + R"(
     %func_entry = OpLabel
     %im = OpLoad %)" + GetParam().type +
                        R"( %i_param

)" + GetParam().inst + R"(

     OpReturn
     OpFunctionEnd

     %main = OpFunction %void None %voidfn
     %entry = OpLabel
     %foo = OpFunctionCall %void %func %20
     OpReturn
     OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildInternalModule());
  EXPECT_TRUE(p->RegisterHandleUsage());
  EXPECT_TRUE(p->error().empty());
  Usage iu = p->GetHandleUsage(20);

  EXPECT_THAT(iu.to_str(), Eq(GetParam().expected_image_usage));
}

INSTANTIATE_TEST_SUITE_P(
    Samples,
    SpvParserTest_RegisterHandleUsage_RawImage,
    ::testing::Values(

        // OpImageRead
        UsageRawImageCase{"f_storage_1d",
                          "%result = OpImageRead %v4float %im %uint_1",
                          "Usage(Texture( read ))"},

        // OpImageWrite
        UsageRawImageCase{"f_storage_1d",
                          "OpImageWrite %im %uint_1 %v4float_null",
                          "Usage(Texture( write ))"},

        // OpImageFetch
        UsageRawImageCase{"f_texture_1d",
                          "%result = OpImageFetch "
                          "%v4float %im %float_null",
                          "Usage(Texture( is_sampled ))"},

        // Image queries

        // OpImageQuerySizeLod
        UsageRawImageCase{"f_storage_2d",
                          "%result = OpImageQuerySizeLod "
                          "%v2uint %im %uint_1",
                          "Usage(Texture( is_sampled ))"},

        // OpImageQuerySize
        // Could be MS=1 or storage image. So it's non-committal.
        UsageRawImageCase{"f_storage_2d",
                          "%result = OpImageQuerySize "
                          "%v2uint %im",
                          "Usage()"},

        // OpImageQueryLevels
        UsageRawImageCase{"f_texture_2d",
                          "%result = OpImageQueryLevels "
                          "%uint %im",
                          "Usage(Texture( ))"},

        // OpImageQuerySamples
        UsageRawImageCase{"f_texture_2d_ms",
                          "%result = OpImageQuerySamples "
                          "%uint %im",
                          "Usage(Texture( is_sampled ms ))"}));

// Test emission of handle variables.

// Test emission of variables where we don't have enough clues from their
// use in image access instructions in executable code.  For these we have
// to infer usage from the SPIR-V sampler or image type.
struct DeclUnderspecifiedHandleCase {
  std::string decorations;  // SPIR-V decorations
  std::string inst;         // SPIR-V variable declarations
  std::string var_decl;     // WGSL variable declaration
};
inline std::ostream& operator<<(std::ostream& out,
                                const DeclUnderspecifiedHandleCase& c) {
  out << "DeclUnderspecifiedHandleCase(" << c.inst << "\n" << c.var_decl << ")";
  return out;
}

using SpvParserTest_DeclUnderspecifiedHandle =
    SpvParserTestBase<::testing::TestWithParam<DeclUnderspecifiedHandleCase>>;

TEST_P(SpvParserTest_DeclUnderspecifiedHandle, Variable) {
  const auto assembly = Preamble() + R"(
     OpDecorate %10 DescriptorSet 0
     OpDecorate %10 Binding 0
)" + GetParam().decorations +
                        CommonTypes() + GetParam().inst +
                        R"(

     %main = OpFunction %void None %voidfn
     %entry = OpLabel
     OpReturn
     OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModule()) << p->error() << assembly;
  EXPECT_TRUE(p->error().empty()) << p->error();
  const auto program = Demangler().Demangle(p->program());
  EXPECT_THAT(program, HasSubstr(GetParam().var_decl)) << program;
}

INSTANTIATE_TEST_SUITE_P(Samplers,
                         SpvParserTest_DeclUnderspecifiedHandle,
                         ::testing::Values(

                             DeclUnderspecifiedHandleCase{"", R"(
         %ptr = OpTypePointer UniformConstant %sampler
         %10 = OpVariable %ptr UniformConstant
)",
                                                          R"(
  Variable{
    Decorations{
      GroupDecoration{0}
      BindingDecoration{0}
    }
    x_10
    uniform_constant
    __sampler_sampler
  })"}));

INSTANTIATE_TEST_SUITE_P(Images,
                         SpvParserTest_DeclUnderspecifiedHandle,
                         ::testing::Values(

                             DeclUnderspecifiedHandleCase{"", R"(
         %10 = OpVariable %ptr_f_texture_1d UniformConstant
)",
                                                          R"(
  Variable{
    Decorations{
      GroupDecoration{0}
      BindingDecoration{0}
    }
    x_10
    uniform_constant
    __sampled_texture_1d__f32
  })"},
                             DeclUnderspecifiedHandleCase{R"(
         OpDecorate %10 NonWritable
)",
                                                          R"(
         %10 = OpVariable %ptr_f_storage_1d UniformConstant
)",
                                                          R"(
  Variable{
    Decorations{
      GroupDecoration{0}
      BindingDecoration{0}
    }
    x_10
    uniform_constant
    __access_control_read_only__storage_texture_1d_rg32float
  })"},
                             DeclUnderspecifiedHandleCase{R"(
         OpDecorate %10 NonReadable
)",
                                                          R"(
         %10 = OpVariable %ptr_f_storage_1d UniformConstant
)",
                                                          R"(
  Variable{
    Decorations{
      GroupDecoration{0}
      BindingDecoration{0}
    }
    x_10
    uniform_constant
    __access_control_write_only__storage_texture_1d_rg32float
  })"}));

// Test emission of variables when we have image accesses in executable code.

struct ImageAccessCase {
  // SPIR-V image type, excluding result ID and opcode
  std::string spirv_image_type_details;
  std::string spirv_image_access;  // The provoking image access instruction.
  std::string var_decl;            // WGSL variable declaration
  std::string texture_builtin;     // WGSL texture usage.
};
inline std::ostream& operator<<(std::ostream& out, const ImageAccessCase& c) {
  out << "ImageCase(" << c.spirv_image_type_details << "\n"
      << c.spirv_image_access << "\n"
      << c.var_decl << "\n"
      << c.texture_builtin << ")";
  return out;
}

using SpvParserTest_SampledImageAccessTest =
    SpvParserTestBase<::testing::TestWithParam<ImageAccessCase>>;

TEST_P(SpvParserTest_SampledImageAccessTest, Variable) {
  const auto assembly = Preamble() + R"(
     OpEntryPoint Fragment %main "main"
     OpExecutionMode %main OriginUpperLeft
     OpName %f1 "f1"
     OpName %vf12 "vf12"
     OpName %vf123 "vf123"
     OpName %vf1234 "vf1234"
     OpName %u1 "u1"
     OpName %vu12 "vu12"
     OpName %vu123 "vu123"
     OpName %vu1234 "vu1234"
     OpName %i1 "i1"
     OpName %vi12 "vi12"
     OpName %vi123 "vi123"
     OpName %vi1234 "vi1234"
     OpName %coords1 "coords1"
     OpName %coords12 "coords12"
     OpName %coords123 "coords123"
     OpName %coords1234 "coords1234"
     OpName %offsets2d "offsets2d"
     OpName %u_offsets2d "u_offsets2d"
     OpDecorate %10 DescriptorSet 0
     OpDecorate %10 Binding 0
     OpDecorate %20 DescriptorSet 2
     OpDecorate %20 Binding 1
     OpDecorate %30 DescriptorSet 0
     OpDecorate %30 Binding 1
)" + CommonBasicTypes() +
                        R"(
     %sampler = OpTypeSampler
     %ptr_sampler = OpTypePointer UniformConstant %sampler
     %im_ty = OpTypeImage )" +
                        GetParam().spirv_image_type_details + R"(
     %ptr_im_ty = OpTypePointer UniformConstant %im_ty
     %si_ty = OpTypeSampledImage %im_ty

     %10 = OpVariable %ptr_sampler UniformConstant
     %20 = OpVariable %ptr_im_ty UniformConstant
     %30 = OpVariable %ptr_sampler UniformConstant ; comparison sampler, when needed

     %main = OpFunction %void None %voidfn
     %entry = OpLabel

     %f1 = OpCopyObject %float %float_1
     %vf12 = OpCopyObject %v2float %the_vf12
     %vf123 = OpCopyObject %v3float %the_vf123
     %vf1234 = OpCopyObject %v4float %the_vf1234

     %i1 = OpCopyObject %int %int_1
     %vi12 = OpCopyObject %v2int %the_vi12
     %vi123 = OpCopyObject %v3int %the_vi123
     %vi1234 = OpCopyObject %v4int %the_vi1234

     %u1 = OpCopyObject %uint %uint_1
     %vu12 = OpCopyObject %v2uint %the_vu12
     %vu123 = OpCopyObject %v3uint %the_vu123
     %vu1234 = OpCopyObject %v4uint %the_vu1234

     %coords1 = OpCopyObject %float %float_1
     %coords12 = OpCopyObject %v2float %vf12
     %coords123 = OpCopyObject %v3float %vf123
     %coords1234 = OpCopyObject %v4float %vf1234

     %value_offset = OpCompositeConstruct %v2int %int_3 %int_4
     %offsets2d = OpCopyObject %v2int %value_offset

     %u_value_offset = OpCompositeConstruct %v2uint %uint_3 %uint_4
     %u_offsets2d = OpCopyObject %v2uint %u_value_offset

     %sam = OpLoad %sampler %10
     %im = OpLoad %im_ty %20
     %sampled_image = OpSampledImage %si_ty %im %sam

)" + GetParam().spirv_image_access +
                        R"(

     OpReturn
     OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModule()) << p->error() << assembly;
  EXPECT_TRUE(p->error().empty()) << p->error();
  const auto program = Demangler().Demangle(p->program());
  EXPECT_THAT(program, HasSubstr(GetParam().var_decl))
      << "DECLARATIONS ARE BAD " << program;
  EXPECT_THAT(program, HasSubstr(GetParam().texture_builtin))
      << "TEXTURE BUILTIN IS BAD " << program << assembly;
}

// TODO(dneto): Test variable declaration and texture builtins provoked by
// use of an image access instruction inside helper function.
TEST_P(SpvParserTest_RegisterHandleUsage_SampledImage, DISABLED_FunctionParam) {
}

INSTANTIATE_TEST_SUITE_P(
    DISABLED_ImageGather,
    SpvParserTest_SampledImageAccessTest,
    ::testing::ValuesIn(std::vector<ImageAccessCase>{
        // TODO(dneto): OpImageGather
        // TODO(dneto): OpImageGather with ConstOffset (signed and unsigned)
        // TODO(dneto): OpImageGather with Offset (signed and unsigned)
        // TODO(dneto): OpImageGather with Offsets (signed and unsigned)
    }));

INSTANTIATE_TEST_SUITE_P(
    DISABLED_ImageDrefGather,
    SpvParserTest_SampledImageAccessTest,
    ::testing::ValuesIn(std::vector<ImageAccessCase>{
        // TODO(dneto): OpImageDrefGather
        // TODO(dneto): OpImageDrefGather with ConstOffset (signed and
        // unsigned)
        // TODO(dneto): OpImageDrefGather with Offset (signed and unsigned)
        // TODO(dneto): OpImageDrefGather with Offsets (signed and unsigned)
    }));

INSTANTIATE_TEST_SUITE_P(
    ImageSampleImplicitLod,
    SpvParserTest_SampledImageAccessTest,
    ::testing::Values(

        // OpImageSampleImplicitLod
        ImageAccessCase{"%float 2D 0 0 0 1 Unknown",
                        "%result = OpImageSampleImplicitLod "
                        "%v4float %sampled_image %coords12",
                        R"(
  Variable{
    Decorations{
      GroupDecoration{0}
      BindingDecoration{0}
    }
    x_10
    uniform_constant
    __sampler_sampler
  }
  Variable{
    Decorations{
      GroupDecoration{2}
      BindingDecoration{1}
    }
    x_20
    uniform_constant
    __sampled_texture_2d__f32
  })",
                        R"(
          Call[not set]{
            Identifier[not set]{textureSample}
            (
              Identifier[not set]{x_20}
              Identifier[not set]{x_10}
              Identifier[not set]{coords12}
            )
          })"},

        // OpImageSampleImplicitLod arrayed
        ImageAccessCase{"%float 2D 0 1 0 1 Unknown",
                        "%result = OpImageSampleImplicitLod "
                        "%v4float %sampled_image %coords123",
                        R"(
  Variable{
    Decorations{
      GroupDecoration{0}
      BindingDecoration{0}
    }
    x_10
    uniform_constant
    __sampler_sampler
  }
  Variable{
    Decorations{
      GroupDecoration{2}
      BindingDecoration{1}
    }
    x_20
    uniform_constant
    __sampled_texture_2d_array__f32
  })",
                        R"(
          Call[not set]{
            Identifier[not set]{textureSample}
            (
              Identifier[not set]{x_20}
              Identifier[not set]{x_10}
              MemberAccessor[not set]{
                Identifier[not set]{coords123}
                Identifier[not set]{xy}
              }
              TypeConstructor[not set]{
                __i32
                MemberAccessor[not set]{
                  Identifier[not set]{coords123}
                  Identifier[not set]{z}
                }
              }
            )
          })"},

        // OpImageSampleImplicitLod with ConstOffset
        ImageAccessCase{
            "%float 2D 0 0 0 1 Unknown",
            "%result = OpImageSampleImplicitLod "
            "%v4float %sampled_image %coords12 ConstOffset %offsets2d",
            R"(
  Variable{
    Decorations{
      GroupDecoration{0}
      BindingDecoration{0}
    }
    x_10
    uniform_constant
    __sampler_sampler
  }
  Variable{
    Decorations{
      GroupDecoration{2}
      BindingDecoration{1}
    }
    x_20
    uniform_constant
    __sampled_texture_2d__f32
  })",
            R"(
          Call[not set]{
            Identifier[not set]{textureSample}
            (
              Identifier[not set]{x_20}
              Identifier[not set]{x_10}
              Identifier[not set]{coords12}
              Identifier[not set]{offsets2d}
            )
          })"},

        // OpImageSampleImplicitLod arrayed with ConstOffset
        ImageAccessCase{
            "%float 2D 0 1 0 1 Unknown",
            "%result = OpImageSampleImplicitLod "
            "%v4float %sampled_image %coords123 ConstOffset %offsets2d",
            R"(
  Variable{
    Decorations{
      GroupDecoration{0}
      BindingDecoration{0}
    }
    x_10
    uniform_constant
    __sampler_sampler
  }
  Variable{
    Decorations{
      GroupDecoration{2}
      BindingDecoration{1}
    }
    x_20
    uniform_constant
    __sampled_texture_2d_array__f32
  })",
            R"(
          Call[not set]{
            Identifier[not set]{textureSample}
            (
              Identifier[not set]{x_20}
              Identifier[not set]{x_10}
              MemberAccessor[not set]{
                Identifier[not set]{coords123}
                Identifier[not set]{xy}
              }
              TypeConstructor[not set]{
                __i32
                MemberAccessor[not set]{
                  Identifier[not set]{coords123}
                  Identifier[not set]{z}
                }
              }
              Identifier[not set]{offsets2d}
            )
          })"},

        // OpImageSampleImplicitLod with Bias
        ImageAccessCase{"%float 2D 0 0 0 1 Unknown",
                        "%result = OpImageSampleImplicitLod "
                        "%v4float %sampled_image %coords12 Bias %float_7",
                        R"(
  Variable{
    Decorations{
      GroupDecoration{0}
      BindingDecoration{0}
    }
    x_10
    uniform_constant
    __sampler_sampler
  }
  Variable{
    Decorations{
      GroupDecoration{2}
      BindingDecoration{1}
    }
    x_20
    uniform_constant
    __sampled_texture_2d__f32
  })",
                        R"(
          Call[not set]{
            Identifier[not set]{textureSampleBias}
            (
              Identifier[not set]{x_20}
              Identifier[not set]{x_10}
              Identifier[not set]{coords12}
              ScalarConstructor[not set]{7.000000}
            )
          })"},

        // OpImageSampleImplicitLod arrayed with Bias
        ImageAccessCase{"%float 2D 0 1 0 1 Unknown",
                        "%result = OpImageSampleImplicitLod "
                        "%v4float %sampled_image %coords123 Bias %float_7",
                        R"(
  Variable{
    Decorations{
      GroupDecoration{0}
      BindingDecoration{0}
    }
    x_10
    uniform_constant
    __sampler_sampler
  }
  Variable{
    Decorations{
      GroupDecoration{2}
      BindingDecoration{1}
    }
    x_20
    uniform_constant
    __sampled_texture_2d_array__f32
  })",
                        R"(
          Call[not set]{
            Identifier[not set]{textureSampleBias}
            (
              Identifier[not set]{x_20}
              Identifier[not set]{x_10}
              MemberAccessor[not set]{
                Identifier[not set]{coords123}
                Identifier[not set]{xy}
              }
              TypeConstructor[not set]{
                __i32
                MemberAccessor[not set]{
                  Identifier[not set]{coords123}
                  Identifier[not set]{z}
                }
              }
              ScalarConstructor[not set]{7.000000}
            )
          })"},

        // OpImageSampleImplicitLod with Bias and signed ConstOffset
        ImageAccessCase{"%float 2D 0 0 0 1 Unknown",
                        "%result = OpImageSampleImplicitLod "
                        "%v4float %sampled_image %coords12 Bias|ConstOffset "
                        "%float_7 %offsets2d",
                        R"(
  Variable{
    Decorations{
      GroupDecoration{0}
      BindingDecoration{0}
    }
    x_10
    uniform_constant
    __sampler_sampler
  }
  Variable{
    Decorations{
      GroupDecoration{2}
      BindingDecoration{1}
    }
    x_20
    uniform_constant
    __sampled_texture_2d__f32
  })",
                        R"(
          Call[not set]{
            Identifier[not set]{textureSampleBias}
            (
              Identifier[not set]{x_20}
              Identifier[not set]{x_10}
              Identifier[not set]{coords12}
              ScalarConstructor[not set]{7.000000}
              Identifier[not set]{offsets2d}
            )
          })"},

        // OpImageSampleImplicitLod with Bias and unsigned ConstOffset
        // Convert ConstOffset to signed
        ImageAccessCase{"%float 2D 0 0 0 1 Unknown",
                        "%result = OpImageSampleImplicitLod "
                        "%v4float %sampled_image %coords12 Bias|ConstOffset "
                        "%float_7 %u_offsets2d",
                        R"(
  Variable{
    Decorations{
      GroupDecoration{0}
      BindingDecoration{0}
    }
    x_10
    uniform_constant
    __sampler_sampler
  }
  Variable{
    Decorations{
      GroupDecoration{2}
      BindingDecoration{1}
    }
    x_20
    uniform_constant
    __sampled_texture_2d__f32
  })",
                        R"(
          Call[not set]{
            Identifier[not set]{textureSampleBias}
            (
              Identifier[not set]{x_20}
              Identifier[not set]{x_10}
              Identifier[not set]{coords12}
              ScalarConstructor[not set]{7.000000}
              TypeConstructor[not set]{
                __vec_2__i32
                Identifier[not set]{u_offsets2d}
              }
            )
          })"},
        // OpImageSampleImplicitLod arrayed with Bias
        ImageAccessCase{"%float 2D 0 1 0 1 Unknown",
                        "%result = OpImageSampleImplicitLod "
                        "%v4float %sampled_image %coords123 Bias|ConstOffset "
                        "%float_7 %offsets2d",
                        R"(
  Variable{
    Decorations{
      GroupDecoration{0}
      BindingDecoration{0}
    }
    x_10
    uniform_constant
    __sampler_sampler
  }
  Variable{
    Decorations{
      GroupDecoration{2}
      BindingDecoration{1}
    }
    x_20
    uniform_constant
    __sampled_texture_2d_array__f32
  })",
                        R"(
          Call[not set]{
            Identifier[not set]{textureSampleBias}
            (
              Identifier[not set]{x_20}
              Identifier[not set]{x_10}
              MemberAccessor[not set]{
                Identifier[not set]{coords123}
                Identifier[not set]{xy}
              }
              TypeConstructor[not set]{
                __i32
                MemberAccessor[not set]{
                  Identifier[not set]{coords123}
                  Identifier[not set]{z}
                }
              }
              ScalarConstructor[not set]{7.000000}
              Identifier[not set]{offsets2d}
            )
          })"}));

INSTANTIATE_TEST_SUITE_P(
    // This test shows the use of a sampled image used with both regular
    // sampling and depth-reference sampling.  The texture is a depth-texture,
    // and we use builtins textureSample and textureSampleCompare
    ImageSampleImplicitLod_BothDrefAndNonDref,
    SpvParserTest_SampledImageAccessTest,
    ::testing::Values(

        // OpImageSampleImplicitLod
        ImageAccessCase{"%float 2D 0 0 0 1 Unknown", R"(
     %sam_dref = OpLoad %sampler %30
     %sampled_dref_image = OpSampledImage %si_ty %im %sam_dref

     %200 = OpImageSampleImplicitLod %v4float %sampled_image %coords12
     %210 = OpImageSampleDrefImplicitLod %float %sampled_dref_image %coords12 %depth
)",
                        R"(
  Variable{
    Decorations{
      GroupDecoration{0}
      BindingDecoration{0}
    }
    x_10
    uniform_constant
    __sampler_sampler
  }
  Variable{
    Decorations{
      GroupDecoration{2}
      BindingDecoration{1}
    }
    x_20
    uniform_constant
    __depth_texture_2d
  }
  Variable{
    Decorations{
      GroupDecoration{0}
      BindingDecoration{1}
    }
    x_30
    uniform_constant
    __sampler_comparison
  })",
                        R"(
    VariableDeclStatement{
      VariableConst{
        x_200
        none
        __vec_4__f32
        {
          TypeConstructor[not set]{
            __vec_4__f32
            Call[not set]{
              Identifier[not set]{textureSample}
              (
                Identifier[not set]{x_20}
                Identifier[not set]{x_10}
                Identifier[not set]{coords12}
              )
            }
            ScalarConstructor[not set]{0.000000}
            ScalarConstructor[not set]{0.000000}
            ScalarConstructor[not set]{0.000000}
          }
        }
      }
    }
    VariableDeclStatement{
      VariableConst{
        x_210
        none
        __f32
        {
          Call[not set]{
            Identifier[not set]{textureSampleCompare}
            (
              Identifier[not set]{x_20}
              Identifier[not set]{x_30}
              Identifier[not set]{coords12}
              ScalarConstructor[not set]{0.200000}
            )
          }
        }
      }
    })"}));

INSTANTIATE_TEST_SUITE_P(
    ImageSampleDrefImplicitLod,
    SpvParserTest_SampledImageAccessTest,
    ::testing::Values(
        // ImageSampleDrefImplicitLod
        ImageAccessCase{"%float 2D 0 0 0 1 Unknown",
                        "%result = OpImageSampleDrefImplicitLod "
                        "%float %sampled_image %coords12 %depth",
                        R"(
  Variable{
    Decorations{
      GroupDecoration{0}
      BindingDecoration{0}
    }
    x_10
    uniform_constant
    __sampler_comparison
  }
  Variable{
    Decorations{
      GroupDecoration{2}
      BindingDecoration{1}
    }
    x_20
    uniform_constant
    __depth_texture_2d
  })",
                        R"(
          Call[not set]{
            Identifier[not set]{textureSampleCompare}
            (
              Identifier[not set]{x_20}
              Identifier[not set]{x_10}
              Identifier[not set]{coords12}
              ScalarConstructor[not set]{0.200000}
            )
          })"},
        // ImageSampleDrefImplicitLod - arrayed
        ImageAccessCase{"%float 2D 0 1 0 1 Unknown",
                        "%result = OpImageSampleDrefImplicitLod "
                        "%float %sampled_image %coords123 %depth",
                        R"(
  Variable{
    Decorations{
      GroupDecoration{0}
      BindingDecoration{0}
    }
    x_10
    uniform_constant
    __sampler_comparison
  }
  Variable{
    Decorations{
      GroupDecoration{2}
      BindingDecoration{1}
    }
    x_20
    uniform_constant
    __depth_texture_2d_array
  })",
                        R"(
          Call[not set]{
            Identifier[not set]{textureSampleCompare}
            (
              Identifier[not set]{x_20}
              Identifier[not set]{x_10}
              MemberAccessor[not set]{
                Identifier[not set]{coords123}
                Identifier[not set]{xy}
              }
              TypeConstructor[not set]{
                __i32
                MemberAccessor[not set]{
                  Identifier[not set]{coords123}
                  Identifier[not set]{z}
                }
              }
              ScalarConstructor[not set]{0.200000}
            )
          })"},
        // ImageSampleDrefImplicitLod with ConstOffset
        ImageAccessCase{
            "%float 2D 0 0 0 1 Unknown",
            "%result = OpImageSampleDrefImplicitLod %float "
            "%sampled_image %coords12 %depth ConstOffset %offsets2d",
            R"(
  Variable{
    Decorations{
      GroupDecoration{0}
      BindingDecoration{0}
    }
    x_10
    uniform_constant
    __sampler_comparison
  }
  Variable{
    Decorations{
      GroupDecoration{2}
      BindingDecoration{1}
    }
    x_20
    uniform_constant
    __depth_texture_2d
  })",
            R"(
          Call[not set]{
            Identifier[not set]{textureSampleCompare}
            (
              Identifier[not set]{x_20}
              Identifier[not set]{x_10}
              Identifier[not set]{coords12}
              ScalarConstructor[not set]{0.200000}
              Identifier[not set]{offsets2d}
            )
          })"},
        // ImageSampleDrefImplicitLod arrayed with ConstOffset
        ImageAccessCase{
            "%float 2D 0 1 0 1 Unknown",
            "%result = OpImageSampleDrefImplicitLod %float "
            "%sampled_image %coords123 %depth ConstOffset %offsets2d",
            R"(
  Variable{
    Decorations{
      GroupDecoration{0}
      BindingDecoration{0}
    }
    x_10
    uniform_constant
    __sampler_comparison
  }
  Variable{
    Decorations{
      GroupDecoration{2}
      BindingDecoration{1}
    }
    x_20
    uniform_constant
    __depth_texture_2d_array
  })",
            R"(
          Call[not set]{
            Identifier[not set]{textureSampleCompare}
            (
              Identifier[not set]{x_20}
              Identifier[not set]{x_10}
              MemberAccessor[not set]{
                Identifier[not set]{coords123}
                Identifier[not set]{xy}
              }
              TypeConstructor[not set]{
                __i32
                MemberAccessor[not set]{
                  Identifier[not set]{coords123}
                  Identifier[not set]{z}
                }
              }
              ScalarConstructor[not set]{0.200000}
              Identifier[not set]{offsets2d}
            )
          })"}));

INSTANTIATE_TEST_SUITE_P(
    ImageSampleExplicitLod,
    SpvParserTest_SampledImageAccessTest,
    ::testing::Values(

        // OpImageSampleExplicitLod - using Lod
        ImageAccessCase{"%float 2D 0 0 0 1 Unknown",
                        "%result = OpImageSampleExplicitLod "
                        "%v4float %sampled_image %coords12 Lod %float_null",
                        R"(
  Variable{
    Decorations{
      GroupDecoration{0}
      BindingDecoration{0}
    }
    x_10
    uniform_constant
    __sampler_sampler
  }
  Variable{
    Decorations{
      GroupDecoration{2}
      BindingDecoration{1}
    }
    x_20
    uniform_constant
    __sampled_texture_2d__f32
  })",
                        R"(
          Call[not set]{
            Identifier[not set]{textureSampleLevel}
            (
              Identifier[not set]{x_20}
              Identifier[not set]{x_10}
              Identifier[not set]{coords12}
              ScalarConstructor[not set]{0.000000}
            )
          })"},

        // OpImageSampleExplicitLod arrayed - using Lod
        ImageAccessCase{"%float 2D 0 1 0 1 Unknown",
                        "%result = OpImageSampleExplicitLod "
                        "%v4float %sampled_image %coords123 Lod %float_null",
                        R"(
  Variable{
    Decorations{
      GroupDecoration{0}
      BindingDecoration{0}
    }
    x_10
    uniform_constant
    __sampler_sampler
  }
  Variable{
    Decorations{
      GroupDecoration{2}
      BindingDecoration{1}
    }
    x_20
    uniform_constant
    __sampled_texture_2d_array__f32
  })",
                        R"(
          Call[not set]{
            Identifier[not set]{textureSampleLevel}
            (
              Identifier[not set]{x_20}
              Identifier[not set]{x_10}
              MemberAccessor[not set]{
                Identifier[not set]{coords123}
                Identifier[not set]{xy}
              }
              TypeConstructor[not set]{
                __i32
                MemberAccessor[not set]{
                  Identifier[not set]{coords123}
                  Identifier[not set]{z}
                }
              }
              ScalarConstructor[not set]{0.000000}
            )
          })"},

        // OpImageSampleExplicitLod - using Lod and ConstOffset
        ImageAccessCase{"%float 2D 0 0 0 1 Unknown",
                        "%result = OpImageSampleExplicitLod "
                        "%v4float %sampled_image %coords12 Lod|ConstOffset "
                        "%float_null %offsets2d",
                        R"(
  Variable{
    Decorations{
      GroupDecoration{0}
      BindingDecoration{0}
    }
    x_10
    uniform_constant
    __sampler_sampler
  }
  Variable{
    Decorations{
      GroupDecoration{2}
      BindingDecoration{1}
    }
    x_20
    uniform_constant
    __sampled_texture_2d__f32
  })",
                        R"(
          Call[not set]{
            Identifier[not set]{textureSampleLevel}
            (
              Identifier[not set]{x_20}
              Identifier[not set]{x_10}
              Identifier[not set]{coords12}
              ScalarConstructor[not set]{0.000000}
              Identifier[not set]{offsets2d}
            )
          })"},

        // OpImageSampleExplicitLod - using Lod and unsigned ConstOffset
        // Convert the ConstOffset operand to signed
        ImageAccessCase{"%float 2D 0 0 0 1 Unknown",
                        "%result = OpImageSampleExplicitLod "
                        "%v4float %sampled_image %coords12 Lod|ConstOffset "
                        "%float_null %u_offsets2d",
                        R"(
  Variable{
    Decorations{
      GroupDecoration{0}
      BindingDecoration{0}
    }
    x_10
    uniform_constant
    __sampler_sampler
  }
  Variable{
    Decorations{
      GroupDecoration{2}
      BindingDecoration{1}
    }
    x_20
    uniform_constant
    __sampled_texture_2d__f32
  })",
                        R"(
          Call[not set]{
            Identifier[not set]{textureSampleLevel}
            (
              Identifier[not set]{x_20}
              Identifier[not set]{x_10}
              Identifier[not set]{coords12}
              ScalarConstructor[not set]{0.000000}
              TypeConstructor[not set]{
                __vec_2__i32
                Identifier[not set]{u_offsets2d}
              }
            )
          })"},

        // OpImageSampleExplicitLod arrayed - using Lod and ConstOffset
        ImageAccessCase{"%float 2D 0 1 0 1 Unknown",
                        "%result = OpImageSampleExplicitLod "
                        "%v4float %sampled_image %coords123 Lod|ConstOffset "
                        "%float_null %offsets2d",
                        R"(
  Variable{
    Decorations{
      GroupDecoration{0}
      BindingDecoration{0}
    }
    x_10
    uniform_constant
    __sampler_sampler
  }
  Variable{
    Decorations{
      GroupDecoration{2}
      BindingDecoration{1}
    }
    x_20
    uniform_constant
    __sampled_texture_2d_array__f32
  })",
                        R"(
          Call[not set]{
            Identifier[not set]{textureSampleLevel}
            (
              Identifier[not set]{x_20}
              Identifier[not set]{x_10}
              MemberAccessor[not set]{
                Identifier[not set]{coords123}
                Identifier[not set]{xy}
              }
              TypeConstructor[not set]{
                __i32
                MemberAccessor[not set]{
                  Identifier[not set]{coords123}
                  Identifier[not set]{z}
                }
              }
              ScalarConstructor[not set]{0.000000}
              Identifier[not set]{offsets2d}
            )
          })"},

        // OpImageSampleExplicitLod - using Grad
        ImageAccessCase{
            "%float 2D 0 0 0 1 Unknown",
            "%result = OpImageSampleExplicitLod "
            "%v4float %sampled_image %coords12 Grad %float_7 %float_null",
            R"(
  Variable{
    Decorations{
      GroupDecoration{0}
      BindingDecoration{0}
    }
    x_10
    uniform_constant
    __sampler_sampler
  }
  Variable{
    Decorations{
      GroupDecoration{2}
      BindingDecoration{1}
    }
    x_20
    uniform_constant
    __sampled_texture_2d__f32
  })",
            R"(
          Call[not set]{
            Identifier[not set]{textureSampleGrad}
            (
              Identifier[not set]{x_20}
              Identifier[not set]{x_10}
              Identifier[not set]{coords12}
              ScalarConstructor[not set]{7.000000}
              ScalarConstructor[not set]{0.000000}
            )
          })"},

        // OpImageSampleExplicitLod arrayed - using Grad
        ImageAccessCase{
            "%float 2D 0 1 0 1 Unknown",
            "%result = OpImageSampleExplicitLod "
            "%v4float %sampled_image %coords123 Grad %float_7 %float_null",
            R"(
  Variable{
    Decorations{
      GroupDecoration{0}
      BindingDecoration{0}
    }
    x_10
    uniform_constant
    __sampler_sampler
  }
  Variable{
    Decorations{
      GroupDecoration{2}
      BindingDecoration{1}
    }
    x_20
    uniform_constant
    __sampled_texture_2d_array__f32
  })",
            R"(
          Call[not set]{
            Identifier[not set]{textureSampleGrad}
            (
              Identifier[not set]{x_20}
              Identifier[not set]{x_10}
              MemberAccessor[not set]{
                Identifier[not set]{coords123}
                Identifier[not set]{xy}
              }
              TypeConstructor[not set]{
                __i32
                MemberAccessor[not set]{
                  Identifier[not set]{coords123}
                  Identifier[not set]{z}
                }
              }
              ScalarConstructor[not set]{7.000000}
              ScalarConstructor[not set]{0.000000}
            )
          })"},

        // OpImageSampleExplicitLod - using Grad and ConstOffset
        ImageAccessCase{"%float 2D 0 0 0 1 Unknown",
                        "%result = OpImageSampleExplicitLod "
                        "%v4float %sampled_image %coords12 Grad|ConstOffset "
                        "%float_7 %float_null %offsets2d",
                        R"(
  Variable{
    Decorations{
      GroupDecoration{0}
      BindingDecoration{0}
    }
    x_10
    uniform_constant
    __sampler_sampler
  }
  Variable{
    Decorations{
      GroupDecoration{2}
      BindingDecoration{1}
    }
    x_20
    uniform_constant
    __sampled_texture_2d__f32
  })",
                        R"(
          Call[not set]{
            Identifier[not set]{textureSampleGrad}
            (
              Identifier[not set]{x_20}
              Identifier[not set]{x_10}
              Identifier[not set]{coords12}
              ScalarConstructor[not set]{7.000000}
              ScalarConstructor[not set]{0.000000}
              Identifier[not set]{offsets2d}
            )
          })"},

        // OpImageSampleExplicitLod - using Grad and unsigned ConstOffset
        ImageAccessCase{"%float 2D 0 0 0 1 Unknown",
                        "%result = OpImageSampleExplicitLod "
                        "%v4float %sampled_image %coords12 Grad|ConstOffset "
                        "%float_7 %float_null %u_offsets2d",
                        R"(
  Variable{
    Decorations{
      GroupDecoration{0}
      BindingDecoration{0}
    }
    x_10
    uniform_constant
    __sampler_sampler
  }
  Variable{
    Decorations{
      GroupDecoration{2}
      BindingDecoration{1}
    }
    x_20
    uniform_constant
    __sampled_texture_2d__f32
  })",
                        R"(
          Call[not set]{
            Identifier[not set]{textureSampleGrad}
            (
              Identifier[not set]{x_20}
              Identifier[not set]{x_10}
              Identifier[not set]{coords12}
              ScalarConstructor[not set]{7.000000}
              ScalarConstructor[not set]{0.000000}
              TypeConstructor[not set]{
                __vec_2__i32
                Identifier[not set]{u_offsets2d}
              }
            )
          })"},

        // OpImageSampleExplicitLod arrayed - using Grad and ConstOffset
        ImageAccessCase{"%float 2D 0 1 0 1 Unknown",
                        "%result = OpImageSampleExplicitLod "
                        "%v4float %sampled_image %coords123 Grad|ConstOffset "
                        "%float_7 %float_null %offsets2d",
                        R"(
  Variable{
    Decorations{
      GroupDecoration{0}
      BindingDecoration{0}
    }
    x_10
    uniform_constant
    __sampler_sampler
  }
  Variable{
    Decorations{
      GroupDecoration{2}
      BindingDecoration{1}
    }
    x_20
    uniform_constant
    __sampled_texture_2d_array__f32
  })",
                        R"(
          Call[not set]{
            Identifier[not set]{textureSampleGrad}
            (
              Identifier[not set]{x_20}
              Identifier[not set]{x_10}
              MemberAccessor[not set]{
                Identifier[not set]{coords123}
                Identifier[not set]{xy}
              }
              TypeConstructor[not set]{
                __i32
                MemberAccessor[not set]{
                  Identifier[not set]{coords123}
                  Identifier[not set]{z}
                }
              }
              ScalarConstructor[not set]{7.000000}
              ScalarConstructor[not set]{0.000000}
              Identifier[not set]{offsets2d}
            )
          })"},

        // OpImageSampleExplicitLod arrayed - using Grad and unsigned
        // ConstOffset
        ImageAccessCase{"%float 2D 0 1 0 1 Unknown",
                        "%result = OpImageSampleExplicitLod "
                        "%v4float %sampled_image %coords123 Grad|ConstOffset "
                        "%float_7 %float_null %u_offsets2d",
                        R"(
  Variable{
    Decorations{
      GroupDecoration{0}
      BindingDecoration{0}
    }
    x_10
    uniform_constant
    __sampler_sampler
  }
  Variable{
    Decorations{
      GroupDecoration{2}
      BindingDecoration{1}
    }
    x_20
    uniform_constant
    __sampled_texture_2d_array__f32
  })",
                        R"(
          Call[not set]{
            Identifier[not set]{textureSampleGrad}
            (
              Identifier[not set]{x_20}
              Identifier[not set]{x_10}
              MemberAccessor[not set]{
                Identifier[not set]{coords123}
                Identifier[not set]{xy}
              }
              TypeConstructor[not set]{
                __i32
                MemberAccessor[not set]{
                  Identifier[not set]{coords123}
                  Identifier[not set]{z}
                }
              }
              ScalarConstructor[not set]{7.000000}
              ScalarConstructor[not set]{0.000000}
              TypeConstructor[not set]{
                __vec_2__i32
                Identifier[not set]{u_offsets2d}
              }
            )
          })"}));

// Test crbug.com/378:
// In WGSL, sampling from depth texture with explicit level of detail
// requires the Lod parameter as an unsigned integer.
// This corresponds to SPIR-V OpSampleExplicitLod and WGSL textureSampleLevel.
INSTANTIATE_TEST_SUITE_P(
    ImageSampleExplicitLod_DepthTexture,
    SpvParserTest_SampledImageAccessTest,
    ::testing::ValuesIn(std::vector<ImageAccessCase>{
        // Test a non-depth case.
        // (This is already tested above in the ImageSampleExplicitLod suite,
        // but I'm repeating here for the contrast with the depth case.)
        {"%float 2D 0 0 0 1 Unknown",
         "%result = OpImageSampleExplicitLod %v4float "
         "%sampled_image %vf12 Lod %f1",
         R"(
  Variable{
    Decorations{
      GroupDecoration{0}
      BindingDecoration{0}
    }
    x_10
    uniform_constant
    __sampler_sampler
  }
  Variable{
    Decorations{
      GroupDecoration{2}
      BindingDecoration{1}
    }
    x_20
    uniform_constant
    __sampled_texture_2d__f32
  })",
         R"(
          Call[not set]{
            Identifier[not set]{textureSampleLevel}
            (
              Identifier[not set]{x_20}
              Identifier[not set]{x_10}
              Identifier[not set]{vf12}
              Identifier[not set]{f1}
            )
          })"},
        // Test a depth case
        {"%float 2D 1 0 0 1 Unknown",
         "%result = OpImageSampleExplicitLod %v4float "
         "%sampled_image %vf12 Lod %f1",
         R"(
  Variable{
    Decorations{
      GroupDecoration{0}
      BindingDecoration{0}
    }
    x_10
    uniform_constant
    __sampler_sampler
  }
  Variable{
    Decorations{
      GroupDecoration{2}
      BindingDecoration{1}
    }
    x_20
    uniform_constant
    __depth_texture_2d
  })",
         R"(
          TypeConstructor[not set]{
            __vec_4__f32
            Call[not set]{
              Identifier[not set]{textureSampleLevel}
              (
                Identifier[not set]{x_20}
                Identifier[not set]{x_10}
                Identifier[not set]{vf12}
                TypeConstructor[not set]{
                  __i32
                  Identifier[not set]{f1}
                }
              )
            }
            ScalarConstructor[not set]{0.000000}
            ScalarConstructor[not set]{0.000000}
            ScalarConstructor[not set]{0.000000}
          })"}}));

using SpvParserTest_ImageAccessTest =
    SpvParserTestBase<::testing::TestWithParam<ImageAccessCase>>;

TEST_P(SpvParserTest_ImageAccessTest, Variable) {
  // In this test harness, we only create an image.
  const auto assembly = Preamble() + R"(
     OpEntryPoint Fragment %main "main"
     OpExecutionMode %main OriginUpperLeft
     OpName %f1 "f1"
     OpName %vf12 "vf12"
     OpName %vf123 "vf123"
     OpName %vf1234 "vf1234"
     OpName %u1 "u1"
     OpName %vu12 "vu12"
     OpName %vu123 "vu123"
     OpName %vu1234 "vu1234"
     OpName %i1 "i1"
     OpName %vi12 "vi12"
     OpName %vi123 "vi123"
     OpName %vi1234 "vi1234"
     OpName %offsets2d "offsets2d"
     OpDecorate %20 DescriptorSet 2
     OpDecorate %20 Binding 1
)" + CommonBasicTypes() +
                        R"(
     %im_ty = OpTypeImage )" +
                        GetParam().spirv_image_type_details + R"(
     %ptr_im_ty = OpTypePointer UniformConstant %im_ty
     %20 = OpVariable %ptr_im_ty UniformConstant

     %main = OpFunction %void None %voidfn
     %entry = OpLabel

     %f1 = OpCopyObject %float %float_1
     %vf12 = OpCopyObject %v2float %the_vf12
     %vf123 = OpCopyObject %v3float %the_vf123
     %vf1234 = OpCopyObject %v4float %the_vf1234

     %i1 = OpCopyObject %int %int_1
     %vi12 = OpCopyObject %v2int %the_vi12
     %vi123 = OpCopyObject %v3int %the_vi123
     %vi1234 = OpCopyObject %v4int %the_vi1234

     %u1 = OpCopyObject %uint %uint_1
     %vu12 = OpCopyObject %v2uint %the_vu12
     %vu123 = OpCopyObject %v3uint %the_vu123
     %vu1234 = OpCopyObject %v4uint %the_vu1234

     %value_offset = OpCompositeConstruct %v2int %int_3 %int_4
     %offsets2d = OpCopyObject %v2int %value_offset
     %im = OpLoad %im_ty %20

)" + GetParam().spirv_image_access +
                        R"(
     OpReturn
     OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));
  ASSERT_TRUE(p->BuildAndParseInternalModule()) << p->error() << assembly;
  EXPECT_TRUE(p->error().empty()) << p->error();
  const auto program = Demangler().Demangle(p->program());
  EXPECT_THAT(program, HasSubstr(GetParam().var_decl))
      << "DECLARATIONS ARE BAD " << program;
  EXPECT_THAT(program, HasSubstr(GetParam().texture_builtin))
      << "TEXTURE BUILTIN IS BAD " << program << assembly;
}

INSTANTIATE_TEST_SUITE_P(ImageWrite_OptionalParams,
                         SpvParserTest_ImageAccessTest,
                         ::testing::ValuesIn(std::vector<ImageAccessCase>{
                             // OpImageWrite with no extra params
                             {"%float 2D 0 0 0 2 Rgba32f",
                              "OpImageWrite %im %vi12 %vf1234",
                              R"(
  Variable{
    Decorations{
      GroupDecoration{2}
      BindingDecoration{1}
    }
    x_20
    uniform_constant
    __access_control_write_only__storage_texture_2d_rgba32float
  })",
                              R"(Call[not set]{
      Identifier[not set]{textureStore}
      (
        Identifier[not set]{x_20}
        Identifier[not set]{vi12}
        Identifier[not set]{vf1234}
      )
    })"}}));

INSTANTIATE_TEST_SUITE_P(
    // SPIR-V's texel parameter is a 4-element vector with the component
    // type matching the sampled type. WGSL's texel parameter might be
    // scalar or vector, depending on the number of channels in the texture.
    ImageWrite_ConvertTexelOperand_Arity,
    SpvParserTest_ImageAccessTest,
    ::testing::ValuesIn(std::vector<ImageAccessCase>{
        // Source 1 component, dest 1 component
        {"%float 2D 0 0 0 2 R32f", "OpImageWrite %im %vi12 %f1",
         R"(
  Variable{
    Decorations{
      GroupDecoration{2}
      BindingDecoration{1}
    }
    x_20
    uniform_constant
    __access_control_write_only__storage_texture_2d_r32float
  })",
         R"(Call[not set]{
      Identifier[not set]{textureStore}
      (
        Identifier[not set]{x_20}
        Identifier[not set]{vi12}
        Identifier[not set]{f1}
      )
    })"},
        // Source 2 component, dest 1 component
        {"%float 2D 0 0 0 2 R32f", "OpImageWrite %im %vi12 %vf12",
         R"(
  Variable{
    Decorations{
      GroupDecoration{2}
      BindingDecoration{1}
    }
    x_20
    uniform_constant
    __access_control_write_only__storage_texture_2d_r32float
  })",
         R"(Call[not set]{
      Identifier[not set]{textureStore}
      (
        Identifier[not set]{x_20}
        Identifier[not set]{vi12}
        MemberAccessor[not set]{
          Identifier[not set]{vf12}
          Identifier[not set]{x}
        }
      )
    })"},
        // Source 3 component, dest 1 component
        {"%float 2D 0 0 0 2 R32f", "OpImageWrite %im %vi12 %vf123",
         R"(
  Variable{
    Decorations{
      GroupDecoration{2}
      BindingDecoration{1}
    }
    x_20
    uniform_constant
    __access_control_write_only__storage_texture_2d_r32float
  })",
         R"(Call[not set]{
      Identifier[not set]{textureStore}
      (
        Identifier[not set]{x_20}
        Identifier[not set]{vi12}
        MemberAccessor[not set]{
          Identifier[not set]{vf123}
          Identifier[not set]{x}
        }
      )
    })"},
        // Source 4 component, dest 1 component
        {"%float 2D 0 0 0 2 R32f", "OpImageWrite %im %vi12 %vf1234",
         R"(
  Variable{
    Decorations{
      GroupDecoration{2}
      BindingDecoration{1}
    }
    x_20
    uniform_constant
    __access_control_write_only__storage_texture_2d_r32float
  })",
         R"(Call[not set]{
      Identifier[not set]{textureStore}
      (
        Identifier[not set]{x_20}
        Identifier[not set]{vi12}
        MemberAccessor[not set]{
          Identifier[not set]{vf1234}
          Identifier[not set]{x}
        }
      )
    })"},
        // Source 2 component, dest 2 component
        {"%float 2D 0 0 0 2 Rg32f", "OpImageWrite %im %vi12 %vf12",
         R"(
  Variable{
    Decorations{
      GroupDecoration{2}
      BindingDecoration{1}
    }
    x_20
    uniform_constant
    __access_control_write_only__storage_texture_2d_rg32float
  })",
         R"(Call[not set]{
      Identifier[not set]{textureStore}
      (
        Identifier[not set]{x_20}
        Identifier[not set]{vi12}
        Identifier[not set]{vf12}
      )
    })"},
        // Source 3 component, dest 2 component
        {"%float 2D 0 0 0 2 Rg32f", "OpImageWrite %im %vi12 %vf123",
         R"(
  Variable{
    Decorations{
      GroupDecoration{2}
      BindingDecoration{1}
    }
    x_20
    uniform_constant
    __access_control_write_only__storage_texture_2d_rg32float
  })",
         R"(Call[not set]{
      Identifier[not set]{textureStore}
      (
        Identifier[not set]{x_20}
        Identifier[not set]{vi12}
        MemberAccessor[not set]{
          Identifier[not set]{vf123}
          Identifier[not set]{xy}
        }
      )
    })"},
        // Source 4 component, dest 2 component
        {"%float 2D 0 0 0 2 Rg32f", "OpImageWrite %im %vi12 %vf1234",
         R"(
  Variable{
    Decorations{
      GroupDecoration{2}
      BindingDecoration{1}
    }
    x_20
    uniform_constant
    __access_control_write_only__storage_texture_2d_rg32float
  })",
         R"(Call[not set]{
      Identifier[not set]{textureStore}
      (
        Identifier[not set]{x_20}
        Identifier[not set]{vi12}
        MemberAccessor[not set]{
          Identifier[not set]{vf1234}
          Identifier[not set]{xy}
        }
      )
    })"},
        // WGSL does not support 3-component storage textures.
        // Source 4 component, dest 4 component
        {"%float 2D 0 0 0 2 Rgba32f", "OpImageWrite %im %vi12 %vf1234",
         R"(
  Variable{
    Decorations{
      GroupDecoration{2}
      BindingDecoration{1}
    }
    x_20
    uniform_constant
    __access_control_write_only__storage_texture_2d_rgba32float
  })",
         R"(Call[not set]{
      Identifier[not set]{textureStore}
      (
        Identifier[not set]{x_20}
        Identifier[not set]{vi12}
        Identifier[not set]{vf1234}
      )
    })"}}));

TEST_F(SpvParserTest, ImageWrite_TooFewSrcTexelComponents_1_vs_4) {
  const auto assembly = Preamble() + R"(
     OpEntryPoint Fragment %main "main"
     OpExecutionMode %main OriginUpperLeft
     OpName %f1 "f1"
     OpName %coords12 "coords12"
     OpDecorate %20 DescriptorSet 2
     OpDecorate %20 Binding 1
)" + CommonBasicTypes() +
                        R"(
     %im_ty = OpTypeImage %void 2D 0 0 0 2 Rgba32f
     %ptr_im_ty = OpTypePointer UniformConstant %im_ty

     %20 = OpVariable %ptr_im_ty UniformConstant

     %main = OpFunction %void None %voidfn
     %entry = OpLabel

     %f1 = OpCopyObject %float %float_1

     %coords12 = OpCopyObject %v2float %the_vf12

     %im = OpLoad %im_ty %20
     OpImageWrite %im %coords12 %f1
     OpReturn
     OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));
  EXPECT_FALSE(p->BuildAndParseInternalModule());
  EXPECT_THAT(p->error(),
              Eq("texel has too few components for storage texture: 1 provided "
                 "but 4 required, in: OpImageWrite %52 %3 %2"))
      << p->error();
}

TEST_F(SpvParserTest, ImageWrite_ThreeComponentStorageTexture_IsError) {
  // SPIR-V doesn't allow a 3-element storage texture format.
  const auto assembly = Preamble() + R"(
     OpEntryPoint Fragment %main "main"
     OpExecutionMode %main OriginUpperLeft
     OpName %vf123 "vf123"
     OpName %coords12 "coords12"
     OpDecorate %20 DescriptorSet 2
     OpDecorate %20 Binding 1
)" + CommonBasicTypes() +
                        R"(
     %im_ty = OpTypeImage %void 2D 0 0 0 2 Rgb32f
     %ptr_im_ty = OpTypePointer UniformConstant %im_ty

     %20 = OpVariable %ptr_im_ty UniformConstant

     %main = OpFunction %void None %voidfn
     %entry = OpLabel

     %vf123 = OpCopyObject %v3float %the_vf123

     %coords12 = OpCopyObject %v2float %the_vf12

     %im = OpLoad %im_ty %20
     OpImageWrite %im %coords12 %vf123
     OpReturn
     OpFunctionEnd
  )";
  auto error = test::AssembleFailure(assembly);
  EXPECT_THAT(error, HasSubstr("Invalid image format 'Rgb32f'"));
}

TEST_F(SpvParserTest, ImageWrite_FloatDest_IntegralSrc_IsError) {
  const auto assembly = Preamble() + R"(
     OpEntryPoint Fragment %main "main"
     OpExecutionMode %main OriginUpperLeft
     OpName %coords12 "coords12"
     OpDecorate %20 DescriptorSet 2
     OpDecorate %20 Binding 1
)" + CommonBasicTypes() +
                        R"(
     %im_ty = OpTypeImage %void 2D 0 0 0 2 R32f
     %ptr_im_ty = OpTypePointer UniformConstant %im_ty

     %20 = OpVariable %ptr_im_ty UniformConstant

     %main = OpFunction %void None %voidfn
     %entry = OpLabel

     %f1 = OpCopyObject %float %float_1

     %coords12 = OpCopyObject %v2float %the_vf12

     %im = OpLoad %im_ty %20
     OpImageWrite %im %coords12 %uint_0
     OpReturn
     OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));
  EXPECT_FALSE(p->BuildAndParseInternalModule());
  EXPECT_THAT(p->error(),
              Eq("can only write float or float vector to a storage image with "
                 "floating texel format: OpImageWrite %52 %2 %13"))
      << p->error();
}

TEST_F(SpvParserTest, ImageWrite_IntegralDest_FloatSrc_IsError) {
  const auto assembly = Preamble() + R"(
     OpEntryPoint Fragment %main "main"
     OpExecutionMode %main OriginUpperLeft
     OpName %coords12 "coords12"
     OpDecorate %20 DescriptorSet 2
     OpDecorate %20 Binding 1
)" + CommonBasicTypes() +
                        R"(
     %im_ty = OpTypeImage %void 2D 0 0 0 2 R32ui
     %ptr_im_ty = OpTypePointer UniformConstant %im_ty

     %20 = OpVariable %ptr_im_ty UniformConstant

     %main = OpFunction %void None %voidfn
     %entry = OpLabel

     %f1 = OpCopyObject %float %float_1

     %coords12 = OpCopyObject %v2float %f1

     %im = OpLoad %im_ty %20
     OpImageWrite %im %coords12 %f1
     OpReturn
     OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));
  EXPECT_FALSE(p->BuildAndParseInternalModule());
  EXPECT_THAT(p->error(),
              Eq("float or float vector can only be written to a storage image "
                 "with floating texel format: OpImageWrite %52 %2 %51"))
      << p->error();
}

INSTANTIATE_TEST_SUITE_P(
    // Convert texel values when the sampled type of the texture is of the
    // wrong signedness:
    //  unsigned int channel type -> signed int sampled texture
    //  signed int channel type -> unsigned int sampled texture
    // (It is already a SPIR-V validation rule that floating point texels
    // must already be used with textures of floating point sampled types)
    ImageWrite_ConvertTexelOperand_Signedness,
    SpvParserTest_ImageAccessTest,
    ::testing::ValuesIn(std::vector<ImageAccessCase>{
        // Sampled type is unsigned int, texel is unsigned int
        {"%uint 2D 0 0 0 2 Rgba32ui", "OpImageWrite %im %vi12 %vu1234",
         R"(
  Variable{
    Decorations{
      GroupDecoration{2}
      BindingDecoration{1}
    }
    x_20
    uniform_constant
    __access_control_write_only__storage_texture_2d_rgba32uint
  })",
         R"(Call[not set]{
      Identifier[not set]{textureStore}
      (
        Identifier[not set]{x_20}
        Identifier[not set]{vi12}
        Identifier[not set]{vu1234}
      )
    })"},
        // Sampled type is unsigned int, texel is signed int
        {"%uint 2D 0 0 0 2 Rgba32ui", "OpImageWrite %im %vi12 %vi1234",
         R"(
  Variable{
    Decorations{
      GroupDecoration{2}
      BindingDecoration{1}
    }
    x_20
    uniform_constant
    __access_control_write_only__storage_texture_2d_rgba32uint
  })",
         R"(Call[not set]{
      Identifier[not set]{textureStore}
      (
        Identifier[not set]{x_20}
        Identifier[not set]{vi12}
        Bitcast[not set]<__vec_4__u32>{
          Identifier[not set]{vi1234}
        }
      )
    })"},
        // Sampled type is signed int, texel is unsigned int
        {"%int 2D 0 0 0 2 Rgba32i", "OpImageWrite %im %vi12 %vu1234",
         R"(
  Variable{
    Decorations{
      GroupDecoration{2}
      BindingDecoration{1}
    }
    x_20
    uniform_constant
    __access_control_write_only__storage_texture_2d_rgba32sint
  })",
         R"(Call[not set]{
      Identifier[not set]{textureStore}
      (
        Identifier[not set]{x_20}
        Identifier[not set]{vi12}
        Bitcast[not set]<__vec_4__i32>{
          Identifier[not set]{vu1234}
        }
      )
    })"},
        // Sampled type is signed int, texel is signed int
        {"%int 2D 0 0 0 2 Rgba32i", "OpImageWrite %im %vi12 %vi1234",
         R"(
  Variable{
    Decorations{
      GroupDecoration{2}
      BindingDecoration{1}
    }
    x_20
    uniform_constant
    __access_control_write_only__storage_texture_2d_rgba32sint
  })",
         R"(Call[not set]{
      Identifier[not set]{textureStore}
      (
        Identifier[not set]{x_20}
        Identifier[not set]{vi12}
        Identifier[not set]{vi1234}
      )
    })"}}));

INSTANTIATE_TEST_SUITE_P(ImageRead_OptionalParams,
                         SpvParserTest_ImageAccessTest,
                         ::testing::ValuesIn(std::vector<ImageAccessCase>{
                             // OpImageRead with no extra params
                             {"%float 2D 0 0 0 2 Rgba32f",
                              "%99 = OpImageRead %v4float %im %vi12",
                              R"(Variable{
    Decorations{
      GroupDecoration{2}
      BindingDecoration{1}
    }
    x_20
    uniform_constant
    __access_control_read_only__storage_texture_2d_rgba32float
  })",
                              R"(VariableDeclStatement{
      VariableConst{
        x_99
        none
        __vec_4__f32
        {
          Call[not set]{
            Identifier[not set]{textureLoad}
            (
              Identifier[not set]{x_20}
              Identifier[not set]{vi12}
            )
          }
        }
      }
    })"}}));

INSTANTIATE_TEST_SUITE_P(
    ImageFetch_OptionalParams,
    SpvParserTest_ImageAccessTest,
    ::testing::ValuesIn(std::vector<ImageAccessCase>{
        // OpImageFetch with no extra params, on sampled texture
        // Level of detail is injected for sampled texture
        {"%float 2D 0 0 0 1 Unknown", "%99 = OpImageFetch %v4float %im %vi12",
         R"(Variable{
    Decorations{
      GroupDecoration{2}
      BindingDecoration{1}
    }
    x_20
    uniform_constant
    __sampled_texture_2d__f32
  })",
         R"(VariableDeclStatement{
      VariableConst{
        x_99
        none
        __vec_4__f32
        {
          Call[not set]{
            Identifier[not set]{textureLoad}
            (
              Identifier[not set]{x_20}
              Identifier[not set]{vi12}
              ScalarConstructor[not set]{0}
            )
          }
        }
      }
    })"},
        // OpImageFetch with explicit level, on sampled texture
        {"%float 2D 0 0 0 1 Unknown",
         "%99 = OpImageFetch %v4float %im %vi12 Lod %int_3",
         R"(Variable{
    Decorations{
      GroupDecoration{2}
      BindingDecoration{1}
    }
    x_20
    uniform_constant
    __sampled_texture_2d__f32
  })",
         R"(VariableDeclStatement{
      VariableConst{
        x_99
        none
        __vec_4__f32
        {
          Call[not set]{
            Identifier[not set]{textureLoad}
            (
              Identifier[not set]{x_20}
              Identifier[not set]{vi12}
              ScalarConstructor[not set]{3}
            )
          }
        }
      }
    })"},
        // OpImageFetch with no extra params, on depth texture
        // Level of detail is injected for depth texture
        {"%float 2D 1 0 0 1 Unknown", "%99 = OpImageFetch %v4float %im %vi12",
         R"(Variable{
    Decorations{
      GroupDecoration{2}
      BindingDecoration{1}
    }
    x_20
    uniform_constant
    __depth_texture_2d
  })",
         R"(VariableDeclStatement{
      VariableConst{
        x_99
        none
        __vec_4__f32
        {
          TypeConstructor[not set]{
            __vec_4__f32
            Call[not set]{
              Identifier[not set]{textureLoad}
              (
                Identifier[not set]{x_20}
                Identifier[not set]{vi12}
                ScalarConstructor[not set]{0}
              )
            }
            ScalarConstructor[not set]{0.000000}
            ScalarConstructor[not set]{0.000000}
            ScalarConstructor[not set]{0.000000}
          }
        }
      }
    })"},
        // OpImageFetch with extra params, on depth texture
        {"%float 2D 1 0 0 1 Unknown",
         "%99 = OpImageFetch %v4float %im %vi12 Lod %int_3",
         R"(Variable{
    Decorations{
      GroupDecoration{2}
      BindingDecoration{1}
    }
    x_20
    uniform_constant
    __depth_texture_2d
  })",
         R"(VariableDeclStatement{
      VariableConst{
        x_99
        none
        __vec_4__f32
        {
          TypeConstructor[not set]{
            __vec_4__f32
            Call[not set]{
              Identifier[not set]{textureLoad}
              (
                Identifier[not set]{x_20}
                Identifier[not set]{vi12}
                ScalarConstructor[not set]{3}
              )
            }
            ScalarConstructor[not set]{0.000000}
            ScalarConstructor[not set]{0.000000}
            ScalarConstructor[not set]{0.000000}
          }
        }
      }
    })"}}));

INSTANTIATE_TEST_SUITE_P(ImageFetch_Depth,
                         // In SPIR-V OpImageFetch always yields a vector of 4
                         // elements, even for depth images.  But in WGSL,
                         // textureLoad on a depth image yields f32.
                         // crbug.com/tint/439
                         SpvParserTest_ImageAccessTest,
                         ::testing::ValuesIn(std::vector<ImageAccessCase>{
                             // ImageFetch on depth image.
                             {"%float 2D 1 0 0 1 Unknown",
                              "%99 = OpImageFetch %v4float %im %vi12 ",
                              R"(Variable{
    Decorations{
      GroupDecoration{2}
      BindingDecoration{1}
    }
    x_20
    uniform_constant
    __depth_texture_2d
  })",
                              R"(VariableDeclStatement{
      VariableConst{
        x_99
        none
        __vec_4__f32
        {
          TypeConstructor[not set]{
            __vec_4__f32
            Call[not set]{
              Identifier[not set]{textureLoad}
              (
                Identifier[not set]{x_20}
                Identifier[not set]{vi12}
                ScalarConstructor[not set]{0}
              )
            }
            ScalarConstructor[not set]{0.000000}
            ScalarConstructor[not set]{0.000000}
            ScalarConstructor[not set]{0.000000}
          }
        }
      }
    })"}}));

INSTANTIATE_TEST_SUITE_P(
    ImageFetch_Multisampled,
    SpvParserTest_ImageAccessTest,
    ::testing::ValuesIn(std::vector<ImageAccessCase>{
        // SPIR-V requires a Sample image operand when operating on a
        // multisampled image.

        // ImageFetch non-arrayed
        {"%float 2D 0 0 1 1 Unknown",
         "%99 = OpImageFetch %v4float %im %vi12 Sample %i1",
         R"(Variable{
    Decorations{
      GroupDecoration{2}
      BindingDecoration{1}
    }
    x_20
    uniform_constant
    __multisampled_texture_2d__f32
  })",
         R"(VariableDeclStatement{
      VariableConst{
        x_99
        none
        __vec_4__f32
        {
          Call[not set]{
            Identifier[not set]{textureLoad}
            (
              Identifier[not set]{x_20}
              Identifier[not set]{vi12}
              Identifier[not set]{i1}
            )
          }
        }
      }
    })"},
        // ImageFetch arrayed
        {"%float 2D 0 1 1 1 Unknown",
         "%99 = OpImageFetch %v4float %im %vi123 Sample %i1",
         R"(Variable{
    Decorations{
      GroupDecoration{2}
      BindingDecoration{1}
    }
    x_20
    uniform_constant
    __multisampled_texture_2d_array__f32
  })",
         R"(VariableDeclStatement{
      VariableConst{
        x_99
        none
        __vec_4__f32
        {
          Call[not set]{
            Identifier[not set]{textureLoad}
            (
              Identifier[not set]{x_20}
              MemberAccessor[not set]{
                Identifier[not set]{vi123}
                Identifier[not set]{xy}
              }
              MemberAccessor[not set]{
                Identifier[not set]{vi123}
                Identifier[not set]{z}
              }
              Identifier[not set]{i1}
            )
          }
        }
      }
    })"}}));

INSTANTIATE_TEST_SUITE_P(
    ImageFetch_Multisampled_ConvertSampleOperand,
    SpvParserTest_ImageAccessTest,
    ::testing::ValuesIn(std::vector<ImageAccessCase>{
        {"%float 2D 0 0 1 1 Unknown",
         "%99 = OpImageFetch %v4float %im %vi12 Sample %u1",
         R"(Variable{
    Decorations{
      GroupDecoration{2}
      BindingDecoration{1}
    }
    x_20
    uniform_constant
    __multisampled_texture_2d__f32
  })",
         R"(VariableDeclStatement{
      VariableConst{
        x_99
        none
        __vec_4__f32
        {
          Call[not set]{
            Identifier[not set]{textureLoad}
            (
              Identifier[not set]{x_20}
              Identifier[not set]{vi12}
              TypeConstructor[not set]{
                __i32
                Identifier[not set]{u1}
              }
            )
          }
        }
      }
    })"}}));

INSTANTIATE_TEST_SUITE_P(
    ConvertResultSignedness,
    SpvParserTest_SampledImageAccessTest,
    ::testing::ValuesIn(std::vector<ImageAccessCase>{
        // Valid SPIR-V only has:
        //      float scalar sampled type vs. floating result
        //      integral scalar sampled type vs. integral result
        // Any of the sampling, reading, or fetching use the same codepath.

        // We'll test with:
        //     OpImageFetch
        //     OpImageRead
        //     OpImageSampleImplicitLod - representative of sampling

        //
        // OpImageRead
        //

        // OpImageFetch requires no conversion, float -> v4float
        {"%float 2D 0 0 0 1 Unknown", "%99 = OpImageFetch %v4float %im %vi12",
         R"(Variable{
    Decorations{
      GroupDecoration{2}
      BindingDecoration{1}
    }
    x_20
    uniform_constant
    __sampled_texture_2d__f32
  })",
         R"(VariableDeclStatement{
      VariableConst{
        x_99
        none
        __vec_4__f32
        {
          Call[not set]{
            Identifier[not set]{textureLoad}
            (
              Identifier[not set]{x_20}
              Identifier[not set]{vi12}
              ScalarConstructor[not set]{0}
            )
          }
        }
      }
    })"},
        // OpImageFetch requires no conversion, uint -> v4uint
        {"%uint 2D 0 0 0 1 Unknown", "%99 = OpImageFetch %v4uint %im %vi12",
         R"(Variable{
    Decorations{
      GroupDecoration{2}
      BindingDecoration{1}
    }
    x_20
    uniform_constant
    __sampled_texture_2d__u32
  })",
         R"(VariableDeclStatement{
      VariableConst{
        x_99
        none
        __vec_4__u32
        {
          Call[not set]{
            Identifier[not set]{textureLoad}
            (
              Identifier[not set]{x_20}
              Identifier[not set]{vi12}
              ScalarConstructor[not set]{0}
            )
          }
        }
      }
    })"},
        // OpImageFetch requires conversion, uint -> v4int
        {"%uint 2D 0 0 0 1 Unknown", "%99 = OpImageFetch %v4int %im %vi12",
         R"(Variable{
    Decorations{
      GroupDecoration{2}
      BindingDecoration{1}
    }
    x_20
    uniform_constant
    __sampled_texture_2d__u32
  })",
         R"(VariableDeclStatement{
      VariableConst{
        x_99
        none
        __vec_4__i32
        {
          Bitcast[not set]<__vec_4__i32>{
            Call[not set]{
              Identifier[not set]{textureLoad}
              (
                Identifier[not set]{x_20}
                Identifier[not set]{vi12}
                ScalarConstructor[not set]{0}
              )
            }
          }
        }
      }
    })"},
        // OpImageFetch requires no conversion, int -> v4int
        {"%int 2D 0 0 0 1 Unknown", "%99 = OpImageFetch %v4int %im %vi12",
         R"(Variable{
    Decorations{
      GroupDecoration{2}
      BindingDecoration{1}
    }
    x_20
    uniform_constant
    __sampled_texture_2d__i32
  })",
         R"(VariableDeclStatement{
      VariableConst{
        x_99
        none
        __vec_4__i32
        {
          Call[not set]{
            Identifier[not set]{textureLoad}
            (
              Identifier[not set]{x_20}
              Identifier[not set]{vi12}
              ScalarConstructor[not set]{0}
            )
          }
        }
      }
    })"},
        // OpImageFetch requires conversion, int -> v4uint
        {"%int 2D 0 0 0 1 Unknown", "%99 = OpImageFetch %v4uint %im %vi12",
         R"(Variable{
    Decorations{
      GroupDecoration{2}
      BindingDecoration{1}
    }
    x_20
    uniform_constant
    __sampled_texture_2d__i32
  })",
         R"(VariableDeclStatement{
      VariableConst{
        x_99
        none
        __vec_4__u32
        {
          Bitcast[not set]<__vec_4__u32>{
            Call[not set]{
              Identifier[not set]{textureLoad}
              (
                Identifier[not set]{x_20}
                Identifier[not set]{vi12}
                ScalarConstructor[not set]{0}
              )
            }
          }
        }
      }
    })"},

        //
        // OpImageRead
        //

        // OpImageRead requires no conversion, float -> v4float
        {"%float 2D 0 0 0 1 Rgba32f", "%99 = OpImageRead %v4float %im %vi12",
         R"(Variable{
    Decorations{
      GroupDecoration{2}
      BindingDecoration{1}
    }
    x_20
    uniform_constant
    __access_control_read_only__storage_texture_2d_rgba32float
  })",
         R"(VariableDeclStatement{
      VariableConst{
        x_99
        none
        __vec_4__f32
        {
          Call[not set]{
            Identifier[not set]{textureLoad}
            (
              Identifier[not set]{x_20}
              Identifier[not set]{vi12}
            )
          }
        }
      }
    })"},
        // OpImageRead requires no conversion, uint -> v4uint
        {"%uint 2D 0 0 0 1 Rgba32ui", "%99 = OpImageRead %v4uint %im %vi12",
         R"(Variable{
    Decorations{
      GroupDecoration{2}
      BindingDecoration{1}
    }
    x_20
    uniform_constant
    __access_control_read_only__storage_texture_2d_rgba32uint
  })",
         R"(VariableDeclStatement{
      VariableConst{
        x_99
        none
        __vec_4__u32
        {
          Call[not set]{
            Identifier[not set]{textureLoad}
            (
              Identifier[not set]{x_20}
              Identifier[not set]{vi12}
            )
          }
        }
      }
    })"},
        // OpImageRead requires conversion, uint -> v4int
        {"%uint 2D 0 0 0 1 Rgba32ui", "%99 = OpImageRead %v4int %im %vi12",
         R"(Variable{
    Decorations{
      GroupDecoration{2}
      BindingDecoration{1}
    }
    x_20
    uniform_constant
    __access_control_read_only__storage_texture_2d_rgba32uint
  })",
         R"(VariableDeclStatement{
      VariableConst{
        x_99
        none
        __vec_4__i32
        {
          Bitcast[not set]<__vec_4__i32>{
            Call[not set]{
              Identifier[not set]{textureLoad}
              (
                Identifier[not set]{x_20}
                Identifier[not set]{vi12}
              )
            }
          }
        }
      }
    })"},
        // OpImageRead requires no conversion, int -> v4int
        {"%int 2D 0 0 0 1 Rgba32i", "%99 = OpImageRead %v4int %im %vi12",
         R"(Variable{
    Decorations{
      GroupDecoration{2}
      BindingDecoration{1}
    }
    x_20
    uniform_constant
    __access_control_read_only__storage_texture_2d_rgba32sint
  })",
         R"(VariableDeclStatement{
      VariableConst{
        x_99
        none
        __vec_4__i32
        {
          Call[not set]{
            Identifier[not set]{textureLoad}
            (
              Identifier[not set]{x_20}
              Identifier[not set]{vi12}
            )
          }
        }
      }
    })"},
        // OpImageRead requires conversion, int -> v4uint
        {"%int 2D 0 0 0 1 Rgba32i", "%99 = OpImageRead %v4uint %im %vi12",
         R"(Variable{
    Decorations{
      GroupDecoration{2}
      BindingDecoration{1}
    }
    x_20
    uniform_constant
    __access_control_read_only__storage_texture_2d_rgba32sint
  })",
         R"(VariableDeclStatement{
      VariableConst{
        x_99
        none
        __vec_4__u32
        {
          Bitcast[not set]<__vec_4__u32>{
            Call[not set]{
              Identifier[not set]{textureLoad}
              (
                Identifier[not set]{x_20}
                Identifier[not set]{vi12}
              )
            }
          }
        }
      }
    })"},

        //
        // Sampling operations, using OpImageSampleImplicitLod as an example.
        // WGSL sampling operations only work on textures with a float sampled
        // component.  So we can only test the float -> float (non-conversion)
        // case.

        // OpImageSampleImplicitLod requires no conversion, float -> v4float
        {"%float 2D 0 0 0 1 Unknown",
         "%99 = OpImageSampleImplicitLod %v4float %sampled_image %vf12",
         R"(
  Variable{
    Decorations{
      GroupDecoration{0}
      BindingDecoration{0}
    }
    x_10
    uniform_constant
    __sampler_sampler
  }
  Variable{
    Decorations{
      GroupDecoration{2}
      BindingDecoration{1}
    }
    x_20
    uniform_constant
    __sampled_texture_2d__f32
  })",
         R"(VariableDeclStatement{
      VariableConst{
        x_99
        none
        __vec_4__f32
        {
          Call[not set]{
            Identifier[not set]{textureSample}
            (
              Identifier[not set]{x_20}
              Identifier[not set]{x_10}
              Identifier[not set]{vf12}
            )
          }
        }
      }
    })"}}));

struct ImageCoordsCase {
  // SPIR-V image type, excluding result ID and opcode
  std::string spirv_image_type_details;
  std::string spirv_image_access;
  std::string expected_error;
  std::vector<std::string> expected_expressions;
};

inline std::ostream& operator<<(std::ostream& out, const ImageCoordsCase& c) {
  out << "ImageAccessCase(" << c.spirv_image_type_details << "\n"
      << c.spirv_image_access << "\n";

  for (auto e : c.expected_expressions) {
    out << e << ",";
  }
  out << ")" << std::endl;
  return out;
}

using SpvParserTest_ImageCoordsTest =
    SpvParserTestBase<::testing::TestWithParam<ImageCoordsCase>>;

TEST_P(SpvParserTest_ImageCoordsTest, MakeCoordinateOperandsForImageAccess) {
  const auto assembly = Preamble() + R"(
     OpEntryPoint Fragment %100 "main"
     OpExecutionMode %100 OriginUpperLeft
     OpName %float_var "float_var"
     OpName %ptr_float "ptr_float"
     OpName %i1 "i1"
     OpName %vi12 "vi12"
     OpName %vi123 "vi123"
     OpName %vi1234 "vi1234"
     OpName %u1 "u1"
     OpName %vu12 "vu12"
     OpName %vu123 "vu123"
     OpName %vu1234 "vu1234"
     OpName %f1 "f1"
     OpName %vf12 "vf12"
     OpName %vf123 "vf123"
     OpName %vf1234 "vf1234"
     OpDecorate %10 DescriptorSet 0
     OpDecorate %10 Binding 0
     OpDecorate %20 DescriptorSet 2
     OpDecorate %20 Binding 1
     OpDecorate %30 DescriptorSet 0
     OpDecorate %30 Binding 1
)" + CommonBasicTypes() +
                        R"(
     %sampler = OpTypeSampler
     %ptr_sampler = OpTypePointer UniformConstant %sampler
     %im_ty = OpTypeImage )" +
                        GetParam().spirv_image_type_details + R"(
     %ptr_im_ty = OpTypePointer UniformConstant %im_ty

     %si_ty = OpTypeSampledImage %im_ty

     %ptr_float = OpTypePointer Function %float

     %10 = OpVariable %ptr_sampler UniformConstant
     %20 = OpVariable %ptr_im_ty UniformConstant
     %30 = OpVariable %ptr_sampler UniformConstant ; comparison sampler, when needed

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel

     %float_var = OpVariable %ptr_float Function

     %i1 = OpCopyObject %int %int_1
     %vi12 = OpCopyObject %v2int %the_vi12
     %vi123 = OpCopyObject %v3int %the_vi123
     %vi1234 = OpCopyObject %v4int %the_vi1234

     %u1 = OpCopyObject %uint %uint_1
     %vu12 = OpCopyObject %v2uint %the_vu12
     %vu123 = OpCopyObject %v3uint %the_vu123
     %vu1234 = OpCopyObject %v4uint %the_vu1234

     %f1 = OpCopyObject %float %float_1
     %vf12 = OpCopyObject %v2float %the_vf12
     %vf123 = OpCopyObject %v3float %the_vf123
     %vf1234 = OpCopyObject %v4float %the_vf1234

     %sam = OpLoad %sampler %10
     %im = OpLoad %im_ty %20
     %sampled_image = OpSampledImage %si_ty %im %sam

)" + GetParam().spirv_image_access +
                        R"(
     ; Use an anchor for the cases when the image access doesn't have a result ID.
     %1000 = OpCopyObject %uint %uint_0

     OpReturn
     OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));
  if (!p->BuildAndParseInternalModule()) {
    EXPECT_THAT(p->error(), StartsWith(GetParam().expected_error)) << assembly;
  } else {
    EXPECT_TRUE(p->error().empty()) << p->error();
    FunctionEmitter fe(p.get(), *spirv_function(p.get(), 100));
    // We actually have to generate the module to cache expressions for the
    // result IDs, particularly the OpCopyObject
    fe.Emit();

    const spvtools::opt::Instruction* anchor = p->GetInstructionForTest(1000);
    ASSERT_NE(anchor, nullptr);
    const spvtools::opt::Instruction& image_access = *(anchor->PreviousNode());

    ast::ExpressionList result =
        fe.MakeCoordinateOperandsForImageAccess(image_access);
    if (GetParam().expected_error.empty()) {
      EXPECT_TRUE(fe.success()) << p->error();
      EXPECT_TRUE(p->error().empty());
      std::vector<std::string> result_strings;
      Program program = p->program();
      for (auto* expr : result) {
        ASSERT_NE(expr, nullptr);
        result_strings.push_back(
            Demangler().Demangle(program.Symbols(), expr->str(program.Sem())));
      }
      EXPECT_THAT(result_strings,
                  ::testing::ContainerEq(GetParam().expected_expressions));
    } else {
      EXPECT_FALSE(fe.success());
      EXPECT_THAT(p->error(), Eq(GetParam().expected_error));
      EXPECT_TRUE(result.empty());
    }
  }
}

INSTANTIATE_TEST_SUITE_P(Good_1D,
                         SpvParserTest_ImageCoordsTest,
                         ::testing::ValuesIn(std::vector<ImageCoordsCase>{
                             {"%float 1D 0 0 0 1 Unknown",
                              "%result = OpImageSampleImplicitLod %v4float "
                              "%sampled_image %f1",
                              "",
                              {"Identifier[not set]{f1}\n"}},
                             {"%float 1D 0 0 0 1 Unknown",
                              "%result = OpImageSampleImplicitLod %v4float "
                              "%sampled_image %vf12",  // one excess arg
                              "",
                              {R"(MemberAccessor[not set]{
  Identifier[not set]{vf12}
  Identifier[not set]{x}
}
)"}},
                             {"%float 1D 0 0 0 1 Unknown",
                              "%result = OpImageSampleImplicitLod %v4float "
                              "%sampled_image %vf123",  // two excess args
                              "",
                              {R"(MemberAccessor[not set]{
  Identifier[not set]{vf123}
  Identifier[not set]{x}
}
)"}},
                             {"%float 1D 0 0 0 1 Unknown",
                              "%result = OpImageSampleImplicitLod %v4float "
                              "%sampled_image %vf1234",  // three excess args
                              "",
                              {R"(MemberAccessor[not set]{
  Identifier[not set]{vf1234}
  Identifier[not set]{x}
}
)"}}}));

INSTANTIATE_TEST_SUITE_P(Good_1DArray,
                         SpvParserTest_ImageCoordsTest,
                         ::testing::ValuesIn(std::vector<ImageCoordsCase>{
                             {"%float 1D 0 1 0 1 Unknown",
                              "%result = OpImageSampleImplicitLod %v4float "
                              "%sampled_image %vf12",
                              "",
                              {
                                  R"(MemberAccessor[not set]{
  Identifier[not set]{vf12}
  Identifier[not set]{x}
}
)",
                                  R"(TypeConstructor[not set]{
  __i32
  MemberAccessor[not set]{
    Identifier[not set]{vf12}
    Identifier[not set]{y}
  }
}
)"}},
                             {"%float 1D 0 1 0 1 Unknown",
                              "%result = OpImageSampleImplicitLod %v4float "
                              "%sampled_image %vf123",  // one excess arg
                              "",
                              {
                                  R"(MemberAccessor[not set]{
  Identifier[not set]{vf123}
  Identifier[not set]{x}
}
)",
                                  R"(TypeConstructor[not set]{
  __i32
  MemberAccessor[not set]{
    Identifier[not set]{vf123}
    Identifier[not set]{y}
  }
}
)"}},
                             {"%float 1D 0 1 0 1 Unknown",
                              "%result = OpImageSampleImplicitLod %v4float "
                              "%sampled_image %vf1234",  // two excess args
                              "",
                              {
                                  R"(MemberAccessor[not set]{
  Identifier[not set]{vf1234}
  Identifier[not set]{x}
}
)",
                                  R"(TypeConstructor[not set]{
  __i32
  MemberAccessor[not set]{
    Identifier[not set]{vf1234}
    Identifier[not set]{y}
  }
}
)"}}}));

INSTANTIATE_TEST_SUITE_P(Good_2D,
                         SpvParserTest_ImageCoordsTest,
                         ::testing::ValuesIn(std::vector<ImageCoordsCase>{
                             {"%float 2D 0 0 0 1 Unknown",
                              "%result = OpImageSampleImplicitLod %v4float "
                              "%sampled_image %vf12",
                              "",
                              {"Identifier[not set]{vf12}\n"}},
                             {"%float 2D 0 0 0 1 Unknown",
                              "%result = OpImageSampleImplicitLod %v4float "
                              "%sampled_image %vf123",  // one excess arg
                              "",
                              {R"(MemberAccessor[not set]{
  Identifier[not set]{vf123}
  Identifier[not set]{xy}
}
)"}},
                             {"%float 2D 0 0 0 1 Unknown",
                              "%result = OpImageSampleImplicitLod %v4float "
                              "%sampled_image %vf1234",  // two excess args
                              "",
                              {R"(MemberAccessor[not set]{
  Identifier[not set]{vf1234}
  Identifier[not set]{xy}
}
)"}}}));

INSTANTIATE_TEST_SUITE_P(Good_2DArray,
                         SpvParserTest_ImageCoordsTest,
                         ::testing::ValuesIn(std::vector<ImageCoordsCase>{
                             {"%float 2D 0 1 0 1 Unknown",
                              "%result = OpImageSampleImplicitLod %v4float "
                              "%sampled_image %vf123",
                              "",
                              {
                                  R"(MemberAccessor[not set]{
  Identifier[not set]{vf123}
  Identifier[not set]{xy}
}
)",
                                  R"(TypeConstructor[not set]{
  __i32
  MemberAccessor[not set]{
    Identifier[not set]{vf123}
    Identifier[not set]{z}
  }
}
)"}},
                             {"%float 2D 0 1 0 1 Unknown",
                              "%result = OpImageSampleImplicitLod %v4float "
                              "%sampled_image %vf1234",  // one excess arg
                              "",
                              {
                                  R"(MemberAccessor[not set]{
  Identifier[not set]{vf1234}
  Identifier[not set]{xy}
}
)",
                                  R"(TypeConstructor[not set]{
  __i32
  MemberAccessor[not set]{
    Identifier[not set]{vf1234}
    Identifier[not set]{z}
  }
}
)"}}}));

INSTANTIATE_TEST_SUITE_P(Good_3D,
                         SpvParserTest_ImageCoordsTest,
                         ::testing::ValuesIn(std::vector<ImageCoordsCase>{
                             {"%float 3D 0 0 0 1 Unknown",
                              "%result = OpImageSampleImplicitLod "
                              "%v4float "
                              "%sampled_image %vf123",
                              "",
                              {"Identifier[not set]{vf123}\n"}},
                             {"%float 3D 0 0 0 1 Unknown",
                              "%result = OpImageSampleImplicitLod "
                              "%v4float "
                              "%sampled_image %vf1234",  // one excess
                                                         // arg
                              "",
                              {R"(MemberAccessor[not set]{
  Identifier[not set]{vf1234}
  Identifier[not set]{xyz}
}
)"}}}));

INSTANTIATE_TEST_SUITE_P(Good_Cube,
                         SpvParserTest_ImageCoordsTest,
                         ::testing::ValuesIn(std::vector<ImageCoordsCase>{
                             {"%float Cube 0 0 0 1 Unknown",
                              "%result = OpImageSampleImplicitLod "
                              "%v4float "
                              "%sampled_image %vf123",
                              "",
                              {"Identifier[not set]{vf123}\n"}},
                             {"%float Cube 0 0 0 1 Unknown",
                              "%result = OpImageSampleImplicitLod "
                              "%v4float "
                              "%sampled_image %vf1234",  // one excess
                                                         // arg
                              "",
                              {R"(MemberAccessor[not set]{
  Identifier[not set]{vf1234}
  Identifier[not set]{xyz}
}
)"}}}));

INSTANTIATE_TEST_SUITE_P(Good_CubeArray,
                         SpvParserTest_ImageCoordsTest,
                         ::testing::ValuesIn(std::vector<ImageCoordsCase>{
                             {"%float Cube 0 1 0 1 Unknown",
                              "%result = OpImageSampleImplicitLod "
                              "%v4float "
                              "%sampled_image %vf1234",
                              "",
                              {R"(MemberAccessor[not set]{
  Identifier[not set]{vf1234}
  Identifier[not set]{xyz}
}
)",
                               R"(TypeConstructor[not set]{
  __i32
  MemberAccessor[not set]{
    Identifier[not set]{vf1234}
    Identifier[not set]{w}
  }
}
)"}}}));

INSTANTIATE_TEST_SUITE_P(
    PreserveFloatCoords_NonArrayed,
    // In SPIR-V, sampling and dref sampling operations use floating point
    // coordinates.  Prove that we preserve floating point-ness.
    // Test across all such instructions.
    SpvParserTest_ImageCoordsTest,
    ::testing::ValuesIn(std::vector<ImageCoordsCase>{
        // Scalar cases
        {"%float 1D 0 0 0 1 Unknown",
         "%result = OpImageSampleImplicitLod %v4float %sampled_image %f1",
         "",
         {"Identifier[not set]{f1}\n"}},
        {"%float 1D 0 0 0 1 Unknown",
         "%result = OpImageSampleExplicitLod %v4float %sampled_image %f1 Lod "
         "%f1",
         "",
         {"Identifier[not set]{f1}\n"}},
        // WGSL does not support depth textures with 1D coordinates
        // Vector cases
        {"%float 2D 0 0 0 1 Unknown",
         "%result = OpImageSampleImplicitLod %v4float %sampled_image %vf12",
         "",
         {"Identifier[not set]{vf12}\n"}},
        {"%float 2D 0 0 0 1 Unknown",
         "%result = OpImageSampleExplicitLod %v4float %sampled_image %vf12 Lod "
         "%f1",
         "",
         {"Identifier[not set]{vf12}\n"}},
        {"%float 2D 1 0 0 1 Unknown",
         "%result = OpImageSampleDrefImplicitLod %float %sampled_image %vf12 "
         "%depth",
         "",
         {"Identifier[not set]{vf12}\n"}},
        {"%float 2D 1 0 0 1 Unknown",
         "%result = OpImageSampleDrefExplicitLod %float %sampled_image %vf12 "
         "%depth Lod %f1",
         "",
         {"Identifier[not set]{vf12}\n"}},
    }));

INSTANTIATE_TEST_SUITE_P(
    PreserveFloatCoords_Arrayed,
    // In SPIR-V, sampling and dref sampling operations use floating point
    // coordinates.  Prove that we preserve floating point-ness of the
    // coordinate part, but convert the array index to signed integer. Test
    // across all such instructions.
    SpvParserTest_ImageCoordsTest,
    ::testing::ValuesIn(std::vector<ImageCoordsCase>{
        {"%float 2D 0 1 0 1 Unknown",
         "%result = OpImageSampleImplicitLod %v4float %sampled_image %vf123",
         "",
         {
             R"(MemberAccessor[not set]{
  Identifier[not set]{vf123}
  Identifier[not set]{xy}
}
)",
             R"(TypeConstructor[not set]{
  __i32
  MemberAccessor[not set]{
    Identifier[not set]{vf123}
    Identifier[not set]{z}
  }
}
)"}},

        {"%float 2D 0 1 0 1 Unknown",
         "%result = OpImageSampleExplicitLod %v4float %sampled_image %vf123 "
         "Lod %f1",
         "",
         {
             R"(MemberAccessor[not set]{
  Identifier[not set]{vf123}
  Identifier[not set]{xy}
}
)",
             R"(TypeConstructor[not set]{
  __i32
  MemberAccessor[not set]{
    Identifier[not set]{vf123}
    Identifier[not set]{z}
  }
}
)"}},
        {"%float 2D 1 1 0 1 Unknown",
         "%result = OpImageSampleDrefImplicitLod %float %sampled_image "
         "%vf123 %depth",
         "",
         {
             R"(MemberAccessor[not set]{
  Identifier[not set]{vf123}
  Identifier[not set]{xy}
}
)",
             R"(TypeConstructor[not set]{
  __i32
  MemberAccessor[not set]{
    Identifier[not set]{vf123}
    Identifier[not set]{z}
  }
}
)"}},
        {"%float 2D 1 1 0 1 Unknown",
         "%result = OpImageSampleDrefExplicitLod %float %sampled_image "
         "%vf123 %depth Lod %f1",
         "",
         {
             R"(MemberAccessor[not set]{
  Identifier[not set]{vf123}
  Identifier[not set]{xy}
}
)",
             R"(TypeConstructor[not set]{
  __i32
  MemberAccessor[not set]{
    Identifier[not set]{vf123}
    Identifier[not set]{z}
  }
}
)"}}}));

INSTANTIATE_TEST_SUITE_P(
    PreserveIntCoords_NonArrayed,
    // In SPIR-V, image read, fetch, and write use integer coordinates.
    // Prove that we preserve signed integer coordinates.
    SpvParserTest_ImageCoordsTest,
    ::testing::ValuesIn(std::vector<ImageCoordsCase>{
        // Scalar cases
        {"%float 1D 0 0 0 1 Unknown",
         "%result = OpImageFetch %float %im %i1",
         "",
         {"Identifier[not set]{i1}\n"}},
        {"%float 1D 0 0 0 2 R32f",
         "%result = OpImageRead %float %im %i1",
         "",
         {"Identifier[not set]{i1}\n"}},
        {"%float 1D 0 0 0 2 R32f",
         "OpImageWrite %im %i1 %float_1",
         "",
         {"Identifier[not set]{i1}\n"}},
        // Vector cases
        {"%float 2D 0 0 0 1 Unknown",
         "%result = OpImageFetch %float %im %vi12",
         "",
         {"Identifier[not set]{vi12}\n"}},
        {"%float 2D 0 0 0 2 R32f",
         "%result = OpImageRead %float %im %vi12",
         "",
         {"Identifier[not set]{vi12}\n"}},
        {"%float 2D 0 0 0 2 R32f",
         "OpImageWrite %im %vi12 %float_1",
         "",
         {"Identifier[not set]{vi12}\n"}}}));

INSTANTIATE_TEST_SUITE_P(
    PreserveIntCoords_Arrayed,
    // In SPIR-V, image read, fetch, and write use integer coordinates.
    // Prove that we preserve signed integer coordinates.
    SpvParserTest_ImageCoordsTest,
    ::testing::ValuesIn(std::vector<ImageCoordsCase>{
        {"%float 2D 0 1 0 1 Unknown",
         "%result = OpImageFetch %float %im %vi123",
         "",
         {
             R"(MemberAccessor[not set]{
  Identifier[not set]{vi123}
  Identifier[not set]{xy}
}
)",
             R"(MemberAccessor[not set]{
  Identifier[not set]{vi123}
  Identifier[not set]{z}
}
)"}},
        {"%float 2D 0 1 0 2 R32f",
         "%result = OpImageRead %float %im %vi123",
         "",
         {
             R"(MemberAccessor[not set]{
  Identifier[not set]{vi123}
  Identifier[not set]{xy}
}
)",
             R"(MemberAccessor[not set]{
  Identifier[not set]{vi123}
  Identifier[not set]{z}
}
)"}},
        {"%float 2D 0 1 0 2 R32f",
         "OpImageWrite %im %vi123 %float_1",
         "",
         {
             R"(MemberAccessor[not set]{
  Identifier[not set]{vi123}
  Identifier[not set]{xy}
}
)",
             R"(MemberAccessor[not set]{
  Identifier[not set]{vi123}
  Identifier[not set]{z}
}
)"}}}));

INSTANTIATE_TEST_SUITE_P(
    ConvertUintCoords_NonArrayed,
    // In SPIR-V, image read, fetch, and write use integer coordinates.
    // Prove that we convert unsigned integer coordinates to signed.
    SpvParserTest_ImageCoordsTest,
    ::testing::ValuesIn(std::vector<ImageCoordsCase>{
        // Scalar cases
        {"%float 1D 0 0 0 1 Unknown",
         "%result = OpImageFetch %float %im %u1",
         "",
         {R"(TypeConstructor[not set]{
  __i32
  Identifier[not set]{u1}
}
)"}},
        {"%float 1D 0 0 0 2 R32f",
         "%result = OpImageRead %float %im %u1",
         "",
         {R"(TypeConstructor[not set]{
  __i32
  Identifier[not set]{u1}
}
)"}},
        {"%float 1D 0 0 0 2 R32f",
         "OpImageWrite %im %u1 %float_1",
         "",
         {R"(TypeConstructor[not set]{
  __i32
  Identifier[not set]{u1}
}
)"}},
        // Vector cases
        {"%float 2D 0 0 0 1 Unknown",
         "%result = OpImageFetch %float %im %vu12",
         "",
         {R"(TypeConstructor[not set]{
  __vec_2__i32
  Identifier[not set]{vu12}
}
)"}},
        {"%float 2D 0 0 0 2 R32f",
         "%result = OpImageRead %float %im %vu12",
         "",
         {R"(TypeConstructor[not set]{
  __vec_2__i32
  Identifier[not set]{vu12}
}
)"}},
        {"%float 2D 0 0 0 2 R32f",
         "OpImageWrite %im %vu12 %float_1",
         "",
         {R"(TypeConstructor[not set]{
  __vec_2__i32
  Identifier[not set]{vu12}
}
)"}}}));

INSTANTIATE_TEST_SUITE_P(
    ConvertUintCoords_Arrayed,
    // In SPIR-V, image read, fetch, and write use integer coordinates.
    // Prove that we convert unsigned integer coordinates to signed.
    SpvParserTest_ImageCoordsTest,
    ::testing::ValuesIn(std::vector<ImageCoordsCase>{
        {"%float 2D 0 1 0 1 Unknown",
         "%result = OpImageFetch %float %im %vu123",
         "",
         {
             R"(TypeConstructor[not set]{
  __vec_2__i32
  MemberAccessor[not set]{
    Identifier[not set]{vu123}
    Identifier[not set]{xy}
  }
}
)",
             R"(TypeConstructor[not set]{
  __i32
  MemberAccessor[not set]{
    Identifier[not set]{vu123}
    Identifier[not set]{z}
  }
}
)"}},
        {"%float 2D 0 1 0 2 R32f",
         "%result = OpImageRead %float %im %vu123",
         "",
         {
             R"(TypeConstructor[not set]{
  __vec_2__i32
  MemberAccessor[not set]{
    Identifier[not set]{vu123}
    Identifier[not set]{xy}
  }
}
)",
             R"(TypeConstructor[not set]{
  __i32
  MemberAccessor[not set]{
    Identifier[not set]{vu123}
    Identifier[not set]{z}
  }
}
)"}},
        {"%float 2D 0 1 0 2 R32f",
         "OpImageWrite %im %vu123 %float_1",
         "",
         {
             R"(TypeConstructor[not set]{
  __vec_2__i32
  MemberAccessor[not set]{
    Identifier[not set]{vu123}
    Identifier[not set]{xy}
  }
}
)",
             R"(TypeConstructor[not set]{
  __i32
  MemberAccessor[not set]{
    Identifier[not set]{vu123}
    Identifier[not set]{z}
  }
}
)"}}}));

INSTANTIATE_TEST_SUITE_P(
    BadInstructions,
    SpvParserTest_ImageCoordsTest,
    ::testing::ValuesIn(std::vector<ImageCoordsCase>{
        {"%float 1D 0 0 0 1 Unknown",
         "OpNop",
         "not an image access instruction: OpNop",
         {}},
        {"%float 1D 0 0 0 1 Unknown",
         "%50 = OpCopyObject %float %float_1",
         "internal error: couldn't find image for "
         "%50 = OpCopyObject %18 %44",
         {}},
        {"%float 1D 0 0 0 1 Unknown",
         "OpStore %float_var %float_1",
         "invalid type for image or sampler "
         "variable or function parameter: %1 = OpVariable %2 Function",
         {}},
        // An example with a missing coordinate
        // won't assemble, so we skip it.
    }));

INSTANTIATE_TEST_SUITE_P(
    Bad_Coordinate,
    SpvParserTest_ImageCoordsTest,
    ::testing::ValuesIn(std::vector<ImageCoordsCase>{
        {"%float 1D 0 0 0 1 Unknown",
         "%result = OpImageSampleImplicitLod "
         // bad type for coordinate: not a number
         "%v4float %sampled_image %float_var",
         "bad or unsupported coordinate type for image access: %71 = "
         "OpImageSampleImplicitLod %42 %70 %1",
         {}},
        {"%float 1D 0 1 0 1 Unknown",  // 1DArray
         "%result = OpImageSampleImplicitLod "
         // 1 component, but need 2
         "%v4float %sampled_image %f1",
         "image access required 2 coordinate components, but only 1 provided, "
         "in: %71 = OpImageSampleImplicitLod %42 %70 %12",
         {}},
        {"%float 2D 0 0 0 1 Unknown",  // 2D
         "%result = OpImageSampleImplicitLod "
         // 1 component, but need 2
         "%v4float %sampled_image %f1",
         "image access required 2 coordinate components, but only 1 provided, "
         "in: %71 = OpImageSampleImplicitLod %42 %70 %12",
         {}},
        {"%float 2D 0 1 0 1 Unknown",  // 2DArray
         "%result = OpImageSampleImplicitLod "
         // 2 component, but need 3
         "%v4float %sampled_image %vf12",
         "image access required 3 coordinate components, but only 2 provided, "
         "in: %71 = OpImageSampleImplicitLod %42 %70 %13",
         {}},
        {"%float 3D 0 0 0 1 Unknown",  // 3D
         "%result = OpImageSampleImplicitLod "
         // 2 components, but need 3
         "%v4float %sampled_image %vf12",
         "image access required 3 coordinate components, but only 2 provided, "
         "in: %71 = OpImageSampleImplicitLod %42 %70 %13",
         {}},
    }));

INSTANTIATE_TEST_SUITE_P(
    SampleNonFloatTexture_IsError,
    SpvParserTest_ImageCoordsTest,
    ::testing::ValuesIn(std::vector<ImageCoordsCase>{
        // ImageSampleImplicitLod
        {"%uint 2D 0 0 0 1 Unknown",
         "%result = OpImageSampleImplicitLod %v4uint %sampled_image %vf12",
         "sampled image must have float component type",
         {}},
        {"%int 2D 0 0 0 1 Unknown",
         "%result = OpImageSampleImplicitLod %v4int %sampled_image %vf12",
         "sampled image must have float component type",
         {}},
        // ImageSampleExplicitLod
        {"%uint 2D 0 0 0 1 Unknown",
         "%result = OpImageSampleExplicitLod %v4uint %sampled_image %vf12 "
         "Lod %f1",
         "sampled image must have float component type",
         {}},
        {"%int 2D 0 0 0 1 Unknown",
         "%result = OpImageSampleExplicitLod %v4int %sampled_image %vf12 "
         "Lod %f1",
         "sampled image must have float component type",
         {}},
        // ImageSampleDrefImplicitLod
        {"%uint 2D 0 0 0 1 Unknown",
         "%result = OpImageSampleDrefImplicitLod %uint %sampled_image %vf12 "
         "%f1",
         "sampled image must have float component type",
         {}},
        {"%int 2D 0 0 0 1 Unknown",
         "%result = OpImageSampleDrefImplicitLod %int %sampled_image %vf12 "
         "%f1",
         "sampled image must have float component type",
         {}},
        // ImageSampleDrefExplicitLod
        {"%uint 2D 0 0 0 1 Unknown",
         "%result = OpImageSampleDrefExplicitLod %uint %sampled_image %vf12 "
         "%f1 Lod %f1",
         "sampled image must have float component type",
         {}},
        {"%int 2D 0 0 0 1 Unknown",
         "%result = OpImageSampleDrefExplicitLod %int %sampled_image %vf12 "
         "%f1 Lod %f1",
         "sampled image must have float component type",
         {}}}));

INSTANTIATE_TEST_SUITE_P(
    ConstOffset_BadInstruction_Errors,
    SpvParserTest_ImageCoordsTest,
    ::testing::ValuesIn(std::vector<ImageCoordsCase>{
        // ImageFetch
        {"%uint 2D 0 0 0 1 Unknown",
         "%result = OpImageFetch %v4uint %sampled_image %vf12 ConstOffset "
         "%the_vu12",
         "ConstOffset is only permitted for sampling operations: ",
         {}},
        // ImageRead
        {"%uint 2D 0 0 0 2 Rgba32ui",
         "%result = OpImageRead %v4uint %im %vu12 ConstOffset %the_vu12",
         "ConstOffset is only permitted for sampling operations: ",
         {}},
        // ImageWrite
        {"%uint 2D 0 0 0 2 Rgba32ui",
         "OpImageWrite %im %vu12 %vu1234 ConstOffset %the_vu12",
         "ConstOffset is only permitted for sampling operations: ",
         {}}
        // TODO(dneto): Gather
        // TODO(dneto): DrefGather
    }));

INSTANTIATE_TEST_SUITE_P(
    ConstOffset_BadDim_Errors,
    SpvParserTest_ImageCoordsTest,
    ::testing::ValuesIn(std::vector<ImageCoordsCase>{
        // 1D
        {"%uint 1D 0 0 0 1 Unknown",
         "%result = OpImageSampleImplicitLod %v4float %sampled_image %vf1234 "
         "ConstOffset %the_vu12",
         "ConstOffset is only permitted for 2D, 2D Arrayed, and 3D textures: ",
         {}},
        // 1D Array
        {"%uint 1D 0 1 0 1 Unknown",
         "%result = OpImageSampleImplicitLod %v4float %sampled_image %vf1234 "
         "ConstOffset %the_vu12",
         "ConstOffset is only permitted for 2D, 2D Arrayed, and 3D textures: ",
         {}},
        // Cube
        {"%uint Cube 0 0 0 1 Unknown",
         "%result = OpImageSampleImplicitLod %v4float %sampled_image %vf1234 "
         "ConstOffset %the_vu12",
         "ConstOffset is only permitted for 2D, 2D Arrayed, and 3D textures: ",
         {}},
        // Cube Array
        {"%uint Cube 0 1 0 1 Unknown",
         "%result = OpImageSampleImplicitLod %v4float %sampled_image %vf1234 "
         "ConstOffset %the_vu12",
         "ConstOffset is only permitted for 2D, 2D Arrayed, and 3D textures: ",
         {}}}));

INSTANTIATE_TEST_SUITE_P(
    ImageSampleDref_Bias_IsError,
    SpvParserTest_ImageCoordsTest,
    ::testing::ValuesIn(std::vector<ImageCoordsCase>{
        // Implicit Lod
        {"%float 2D 1 0 0 1 Unknown",
         "%result = OpImageSampleDrefImplicitLod %float %sampled_image %vf1234 "
         "%depth Bias %float_null",
         "WGSL does not support depth-reference sampling with level-of-detail "
         "bias: ",
         {}},
        // Explicit Lod
        {"%float 2D 1 0 0 1 Unknown",
         "%result = OpImageSampleDrefExplicitLod %float %sampled_image %vf1234 "
         "%depth Lod|Bias %float_null %float_null",
         "WGSL does not support depth-reference sampling with level-of-detail "
         "bias: ",
         {}}}));

INSTANTIATE_TEST_SUITE_P(
    ImageSampleDref_Grad_IsError,
    SpvParserTest_ImageCoordsTest,
    ::testing::ValuesIn(std::vector<ImageCoordsCase>{
        // Implicit Lod
        {"%float 2D 1 0 0 1 Unknown",
         "%result = OpImageSampleDrefImplicitLod %float %sampled_image %vf1234 "
         "%depth Grad %float_1 %float_2",
         "WGSL does not support depth-reference sampling with explicit "
         "gradient: ",
         {}},
        // Explicit Lod
        {"%float 2D 1 0 0 1 Unknown",
         "%result = OpImageSampleDrefExplicitLod %float %sampled_image %vf1234 "
         "%depth Lod|Grad %float_null %float_1  %float_2",
         "WGSL does not support depth-reference sampling with explicit "
         "gradient: ",
         {}}}));

TEST_F(SpvParserTest, CombinedImageSampler_IsError) {
  const auto assembly = Preamble() + R"(
     OpEntryPoint Fragment %100 "main"
     OpExecutionMode %100 OriginUpperLeft

     OpDecorate %var DescriptorSet 0
     OpDecorate %var Binding 0
  %float = OpTypeFloat 32
     %im = OpTypeImage %float 2D 0 0 0 1 Unknown
     %si = OpTypeSampledImage %im
 %ptr_si = OpTypePointer UniformConstant %si
    %var = OpVariable %ptr_si UniformConstant
   %void = OpTypeVoid
 %voidfn = OpTypeFunction %void

    %100 = OpFunction %void None %voidfn
  %entry = OpLabel
           OpReturn
           OpFunctionEnd
  )";
  auto p = parser(test::Assemble(assembly));
  EXPECT_FALSE(p->BuildAndParseInternalModule()) << assembly;
  EXPECT_THAT(p->error(),
              HasSubstr("WGSL does not support combined image-samplers: "));
}

}  // namespace
}  // namespace spirv
}  // namespace reader
}  // namespace tint
