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
#include "src/tint/writer/spirv/test_helper_ir.h"

namespace tint::writer::spirv {
namespace {

TEST_F(SpvGeneratorImplTest, Type_Void) {
    auto id = generator_.Type(ir.types.Get<type::Void>());
    EXPECT_EQ(id, 1u);
    EXPECT_EQ(DumpTypes(), "%1 = OpTypeVoid\n");
}

TEST_F(SpvGeneratorImplTest, Type_Bool) {
    auto id = generator_.Type(ir.types.Get<type::Bool>());
    EXPECT_EQ(id, 1u);
    EXPECT_EQ(DumpTypes(), "%1 = OpTypeBool\n");
}

TEST_F(SpvGeneratorImplTest, Type_I32) {
    auto id = generator_.Type(ir.types.Get<type::I32>());
    EXPECT_EQ(id, 1u);
    EXPECT_EQ(DumpTypes(), "%1 = OpTypeInt 32 1\n");
}

TEST_F(SpvGeneratorImplTest, Type_U32) {
    auto id = generator_.Type(ir.types.Get<type::U32>());
    EXPECT_EQ(id, 1u);
    EXPECT_EQ(DumpTypes(), "%1 = OpTypeInt 32 0\n");
}

TEST_F(SpvGeneratorImplTest, Type_F32) {
    auto id = generator_.Type(ir.types.Get<type::F32>());
    EXPECT_EQ(id, 1u);
    EXPECT_EQ(DumpTypes(), "%1 = OpTypeFloat 32\n");
}

TEST_F(SpvGeneratorImplTest, Type_F16) {
    auto id = generator_.Type(ir.types.Get<type::F16>());
    EXPECT_EQ(id, 1u);
    EXPECT_EQ(DumpTypes(), "%1 = OpTypeFloat 16\n");
}

// Test that we do can emit multiple types.
// Includes types with the same opcode but different parameters.
TEST_F(SpvGeneratorImplTest, Type_Multiple) {
    EXPECT_EQ(generator_.Type(ir.types.Get<type::I32>()), 1u);
    EXPECT_EQ(generator_.Type(ir.types.Get<type::U32>()), 2u);
    EXPECT_EQ(generator_.Type(ir.types.Get<type::F32>()), 3u);
    EXPECT_EQ(generator_.Type(ir.types.Get<type::F16>()), 4u);
    EXPECT_EQ(DumpTypes(), R"(%1 = OpTypeInt 32 1
%2 = OpTypeInt 32 0
%3 = OpTypeFloat 32
%4 = OpTypeFloat 16
)");
}

// Test that we do not emit the same type more than once.
TEST_F(SpvGeneratorImplTest, Type_Deduplicate) {
    auto* i32 = ir.types.Get<type::I32>();
    EXPECT_EQ(generator_.Type(i32), 1u);
    EXPECT_EQ(generator_.Type(i32), 1u);
    EXPECT_EQ(generator_.Type(i32), 1u);
    EXPECT_EQ(DumpTypes(), "%1 = OpTypeInt 32 1\n");
}

}  // namespace
}  // namespace tint::writer::spirv
