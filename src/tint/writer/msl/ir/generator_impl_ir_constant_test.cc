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

#include "src/tint/utils/string.h"
#include "src/tint/writer/msl/ir/test_helper_ir.h"

namespace tint::writer::msl {
namespace {

using namespace tint::number_suffixes;  // NOLINT

TEST_F(MslGeneratorImplIrTest, Constant_Bool_True) {
    auto* c = b.Constant(true);
    generator_.EmitConstant(generator_.Line(), c);
    ASSERT_TRUE(generator_.Diagnostics().empty()) << generator_.Diagnostics().str();
    EXPECT_EQ(utils::TrimSpace(generator_.Result()), std::string("true"));
}

TEST_F(MslGeneratorImplIrTest, Constant_Bool_False) {
    auto* c = b.Constant(false);
    generator_.EmitConstant(generator_.Line(), c);
    ASSERT_TRUE(generator_.Diagnostics().empty()) << generator_.Diagnostics().str();
    EXPECT_EQ(utils::TrimSpace(generator_.Result()), std::string("false"));
}

TEST_F(MslGeneratorImplIrTest, Constant_i32) {
    auto* c = b.Constant(-12345_i);
    generator_.EmitConstant(generator_.Line(), c);
    ASSERT_TRUE(generator_.Diagnostics().empty()) << generator_.Diagnostics().str();
    EXPECT_EQ(utils::TrimSpace(generator_.Result()), std::string("-12345"));
}

TEST_F(MslGeneratorImplIrTest, Constant_u32) {
    auto* c = b.Constant(12345_u);
    generator_.EmitConstant(generator_.Line(), c);
    ASSERT_TRUE(generator_.Diagnostics().empty()) << generator_.Diagnostics().str();
    EXPECT_EQ(utils::TrimSpace(generator_.Result()), std::string("12345u"));
}

TEST_F(MslGeneratorImplIrTest, Constant_F32) {
    auto* c = b.Constant(f32((1 << 30) - 4));
    generator_.EmitConstant(generator_.Line(), c);
    ASSERT_TRUE(generator_.Diagnostics().empty()) << generator_.Diagnostics().str();
    EXPECT_EQ(utils::TrimSpace(generator_.Result()), std::string("1073741824.0f"));
}

TEST_F(MslGeneratorImplIrTest, Constant_F16) {
    auto* c = b.Constant(f16((1 << 15) - 8));
    generator_.EmitConstant(generator_.Line(), c);
    ASSERT_TRUE(generator_.Diagnostics().empty()) << generator_.Diagnostics().str();
    EXPECT_EQ(utils::TrimSpace(generator_.Result()), std::string("32752.0h"));
}

}  // namespace
}  // namespace tint::writer::msl
