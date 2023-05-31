// Copyright 2023 The Tint Authors.
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

#include "src/tint/type/bool.h"
#include "src/tint/type/f16.h"
#include "src/tint/type/f32.h"
#include "src/tint/type/i32.h"
#include "src/tint/type/type.h"
#include "src/tint/type/u32.h"
#include "src/tint/type/void.h"
#include "src/tint/writer/spirv/ir/test_helper_ir.h"

namespace tint::writer::spirv {
namespace {

TEST_F(SpvGeneratorImplTest, Type_Void) {
    auto id = generator_.Type(mod.Types().void_());
    EXPECT_EQ(id, 1u);
    EXPECT_EQ(DumpTypes(), "%1 = OpTypeVoid\n");
}

TEST_F(SpvGeneratorImplTest, Type_Bool) {
    auto id = generator_.Type(mod.Types().bool_());
    EXPECT_EQ(id, 1u);
    EXPECT_EQ(DumpTypes(), "%1 = OpTypeBool\n");
}

TEST_F(SpvGeneratorImplTest, Type_I32) {
    auto id = generator_.Type(mod.Types().i32());
    EXPECT_EQ(id, 1u);
    EXPECT_EQ(DumpTypes(), "%1 = OpTypeInt 32 1\n");
}

TEST_F(SpvGeneratorImplTest, Type_U32) {
    auto id = generator_.Type(mod.Types().u32());
    EXPECT_EQ(id, 1u);
    EXPECT_EQ(DumpTypes(), "%1 = OpTypeInt 32 0\n");
}

TEST_F(SpvGeneratorImplTest, Type_F32) {
    auto id = generator_.Type(mod.Types().f32());
    EXPECT_EQ(id, 1u);
    EXPECT_EQ(DumpTypes(), "%1 = OpTypeFloat 32\n");
}

TEST_F(SpvGeneratorImplTest, Type_F16) {
    auto id = generator_.Type(mod.Types().f16());
    EXPECT_EQ(id, 1u);
    EXPECT_EQ(DumpTypes(), "%1 = OpTypeFloat 16\n");
}

TEST_F(SpvGeneratorImplTest, Type_Vec2i) {
    auto* vec = mod.Types().Get<type::Vector>(mod.Types().i32(), 2u);
    auto id = generator_.Type(vec);
    EXPECT_EQ(id, 1u);
    EXPECT_EQ(DumpTypes(),
              "%2 = OpTypeInt 32 1\n"
              "%1 = OpTypeVector %2 2\n");
}

TEST_F(SpvGeneratorImplTest, Type_Vec3u) {
    auto* vec = mod.Types().Get<type::Vector>(mod.Types().u32(), 3u);
    auto id = generator_.Type(vec);
    EXPECT_EQ(id, 1u);
    EXPECT_EQ(DumpTypes(),
              "%2 = OpTypeInt 32 0\n"
              "%1 = OpTypeVector %2 3\n");
}

TEST_F(SpvGeneratorImplTest, Type_Vec4f) {
    auto* vec = mod.Types().Get<type::Vector>(mod.Types().f32(), 4u);
    auto id = generator_.Type(vec);
    EXPECT_EQ(id, 1u);
    EXPECT_EQ(DumpTypes(),
              "%2 = OpTypeFloat 32\n"
              "%1 = OpTypeVector %2 4\n");
}

TEST_F(SpvGeneratorImplTest, Type_Vec4h) {
    auto* vec = mod.Types().Get<type::Vector>(mod.Types().f16(), 2u);
    auto id = generator_.Type(vec);
    EXPECT_EQ(id, 1u);
    EXPECT_EQ(DumpTypes(),
              "%2 = OpTypeFloat 16\n"
              "%1 = OpTypeVector %2 2\n");
}

TEST_F(SpvGeneratorImplTest, Type_Vec4Bool) {
    auto* vec = mod.Types().Get<type::Vector>(mod.Types().bool_(), 4u);
    auto id = generator_.Type(vec);
    EXPECT_EQ(id, 1u);
    EXPECT_EQ(DumpTypes(),
              "%2 = OpTypeBool\n"
              "%1 = OpTypeVector %2 4\n");
}

TEST_F(SpvGeneratorImplTest, Type_Mat2x3f) {
    auto* vec = mod.Types().mat2x3(mod.Types().f32());
    auto id = generator_.Type(vec);
    EXPECT_EQ(id, 1u);
    EXPECT_EQ(DumpTypes(),
              "%3 = OpTypeFloat 32\n"
              "%2 = OpTypeVector %3 3\n"
              "%1 = OpTypeMatrix %2 2\n");
}

TEST_F(SpvGeneratorImplTest, Type_Mat4x2h) {
    auto* vec = mod.Types().mat4x2(mod.Types().f16());
    auto id = generator_.Type(vec);
    EXPECT_EQ(id, 1u);
    EXPECT_EQ(DumpTypes(),
              "%3 = OpTypeFloat 16\n"
              "%2 = OpTypeVector %3 2\n"
              "%1 = OpTypeMatrix %2 4\n");
}

TEST_F(SpvGeneratorImplTest, Type_Array_DefaultStride) {
    auto* arr = mod.Types().array(mod.Types().f32(), 4u);
    auto id = generator_.Type(arr);
    EXPECT_EQ(id, 1u);
    EXPECT_EQ(DumpTypes(),
              "%2 = OpTypeFloat 32\n"
              "%4 = OpTypeInt 32 0\n"
              "%3 = OpConstant %4 4\n"
              "%1 = OpTypeArray %2 %3\n");
    EXPECT_EQ(DumpInstructions(generator_.Module().Annots()), "OpDecorate %1 ArrayStride 4\n");
}

TEST_F(SpvGeneratorImplTest, Type_Array_ExplicitStride) {
    auto* arr = mod.Types().array(mod.Types().f32(), 4u, 16);
    auto id = generator_.Type(arr);
    EXPECT_EQ(id, 1u);
    EXPECT_EQ(DumpTypes(),
              "%2 = OpTypeFloat 32\n"
              "%4 = OpTypeInt 32 0\n"
              "%3 = OpConstant %4 4\n"
              "%1 = OpTypeArray %2 %3\n");
    EXPECT_EQ(DumpInstructions(generator_.Module().Annots()), "OpDecorate %1 ArrayStride 16\n");
}

TEST_F(SpvGeneratorImplTest, Type_RuntimeArray_DefaultStride) {
    auto* arr = mod.Types().runtime_array(mod.Types().f32());
    auto id = generator_.Type(arr);
    EXPECT_EQ(id, 1u);
    EXPECT_EQ(DumpTypes(),
              "%2 = OpTypeFloat 32\n"
              "%1 = OpTypeRuntimeArray %2\n");
    EXPECT_EQ(DumpInstructions(generator_.Module().Annots()), "OpDecorate %1 ArrayStride 4\n");
}

TEST_F(SpvGeneratorImplTest, Type_RuntimeArray_ExplicitStride) {
    auto* arr = mod.Types().runtime_array(mod.Types().f32(), 16);
    auto id = generator_.Type(arr);
    EXPECT_EQ(id, 1u);
    EXPECT_EQ(DumpTypes(),
              "%2 = OpTypeFloat 32\n"
              "%1 = OpTypeRuntimeArray %2\n");
    EXPECT_EQ(DumpInstructions(generator_.Module().Annots()), "OpDecorate %1 ArrayStride 16\n");
}

// Test that we can emit multiple types.
// Includes types with the same opcode but different parameters.
TEST_F(SpvGeneratorImplTest, Type_Multiple) {
    EXPECT_EQ(generator_.Type(mod.Types().i32()), 1u);
    EXPECT_EQ(generator_.Type(mod.Types().u32()), 2u);
    EXPECT_EQ(generator_.Type(mod.Types().f32()), 3u);
    EXPECT_EQ(generator_.Type(mod.Types().f16()), 4u);
    EXPECT_EQ(DumpTypes(), R"(%1 = OpTypeInt 32 1
%2 = OpTypeInt 32 0
%3 = OpTypeFloat 32
%4 = OpTypeFloat 16
)");
}

// Test that we do not emit the same type more than once.
TEST_F(SpvGeneratorImplTest, Type_Deduplicate) {
    auto* i32 = mod.Types().i32();
    EXPECT_EQ(generator_.Type(i32), 1u);
    EXPECT_EQ(generator_.Type(i32), 1u);
    EXPECT_EQ(generator_.Type(i32), 1u);
    EXPECT_EQ(DumpTypes(), "%1 = OpTypeInt 32 1\n");
}

}  // namespace
}  // namespace tint::writer::spirv
