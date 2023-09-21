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

#include <string>

#include "gtest/gtest-spi.h"
#include "src/tint/lang/core/ir/function_param.h"
#include "src/tint/lang/core/ir/ir_helper_test.h"

namespace tint::core::ir {
namespace {

using namespace tint::core::number_suffixes;  // NOLINT
using IR_FunctionParamTest = IRTestHelper;

TEST_F(IR_FunctionParamTest, Fail_NullType) {
    EXPECT_FATAL_FAILURE(
        {
            Module mod;
            Builder b{mod};
            b.FunctionParam(nullptr);
        },
        "");
}

TEST_F(IR_FunctionParamTest, Fail_SetDuplicateBuiltin) {
    EXPECT_FATAL_FAILURE(
        {
            Module mod;
            Builder b{mod};
            auto* fp = b.FunctionParam(mod.Types().f32());
            fp->SetBuiltin(FunctionParam::Builtin::kVertexIndex);
            fp->SetBuiltin(FunctionParam::Builtin::kSampleMask);
        },
        "");
}

TEST_F(IR_FunctionParamTest, CloneEmpty) {
    auto* fp = b.FunctionParam(mod.Types().f32());

    auto* new_fp = clone_ctx.Clone(fp);
    EXPECT_EQ(new_fp->Type(), mod.Types().f32());
    EXPECT_FALSE(new_fp->Builtin().has_value());
    EXPECT_FALSE(new_fp->Location().has_value());
    EXPECT_FALSE(new_fp->BindingPoint().has_value());
    EXPECT_FALSE(new_fp->Invariant());
}

TEST_F(IR_FunctionParamTest, Clone) {
    auto* fp = b.FunctionParam(mod.Types().f32());
    fp->SetBuiltin(FunctionParam::Builtin::kVertexIndex);
    fp->SetLocation(
        1, Interpolation{core::InterpolationType::kFlat, core::InterpolationSampling::kCentroid});
    fp->SetInvariant(true);
    fp->SetBindingPoint(1, 2);

    auto* new_fp = clone_ctx.Clone(fp);

    EXPECT_NE(fp, new_fp);
    EXPECT_EQ(new_fp->Type(), mod.Types().f32());

    EXPECT_TRUE(new_fp->Builtin().has_value());
    EXPECT_EQ(FunctionParam::Builtin::kVertexIndex, new_fp->Builtin().value());

    EXPECT_TRUE(new_fp->Location().has_value());
    auto loc = new_fp->Location();
    EXPECT_EQ(1u, loc->value);
    EXPECT_EQ(core::InterpolationType::kFlat, loc->interpolation->type);
    EXPECT_EQ(core::InterpolationSampling::kCentroid, loc->interpolation->sampling);

    EXPECT_TRUE(new_fp->BindingPoint().has_value());
    auto bp = new_fp->BindingPoint();
    EXPECT_EQ(1u, bp->group);
    EXPECT_EQ(2u, bp->binding);

    EXPECT_TRUE(new_fp->Invariant());
}

TEST_F(IR_FunctionParamTest, CloneWithName) {
    auto* fp = b.FunctionParam("fp", mod.Types().f32());
    auto* new_fp = clone_ctx.Clone(fp);

    EXPECT_EQ(std::string("fp"), mod.NameOf(new_fp).Name());
}

}  // namespace
}  // namespace tint::core::ir
