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

#include "src/tint/lang/core/ir/var.h"

#include "gmock/gmock.h"
#include "gtest/gtest-spi.h"
#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/instruction.h"
#include "src/tint/lang/core/ir/ir_helper_test.h"

namespace tint::core::ir {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

using IR_VarTest = IRTestHelper;

TEST_F(IR_VarTest, Fail_NullType) {
    EXPECT_FATAL_FAILURE(
        {
            Module mod;
            Builder b{mod};
            b.Var(nullptr);
        },
        "");
}

TEST_F(IR_VarTest, Results) {
    auto* var = b.Var(ty.ptr<function, f32>());
    EXPECT_TRUE(var->HasResults());
    EXPECT_FALSE(var->HasMultiResults());
    EXPECT_TRUE(var->Result()->Is<InstructionResult>());
    EXPECT_EQ(var->Result()->Source(), var);
}

TEST_F(IR_VarTest, Initializer_Usage) {
    Module mod;
    Builder b{mod};
    auto* var = b.Var(ty.ptr<function, f32>());
    auto* init = b.Constant(1_f);
    var->SetInitializer(init);

    EXPECT_THAT(init->Usages(), testing::UnorderedElementsAre(Usage{var, 0u}));
    var->SetInitializer(nullptr);
    EXPECT_TRUE(init->Usages().IsEmpty());
}

TEST_F(IR_VarTest, Clone) {
    auto* v = b.Var(mod.Types().ptr(core::AddressSpace::kFunction, mod.Types().f32()));
    v->SetInitializer(b.Constant(4_f));
    v->SetBindingPoint(1, 2);
    v->SetAttributes(IOAttributes{
        3, 4, core::BuiltinValue::kFragDepth,
        Interpolation{core::InterpolationType::kFlat, core::InterpolationSampling::kCentroid},
        true});

    auto* new_v = clone_ctx.Clone(v);

    EXPECT_NE(v, new_v);
    ASSERT_NE(nullptr, new_v->Result());
    EXPECT_NE(v->Result(), new_v->Result());
    EXPECT_EQ(new_v->Result()->Type(),
              mod.Types().ptr(core::AddressSpace::kFunction, mod.Types().f32()));

    auto new_val = v->Initializer()->As<Constant>()->Value();
    ASSERT_TRUE(new_val->Is<core::constant::Scalar<f32>>());
    EXPECT_FLOAT_EQ(4_f, new_val->As<core::constant::Scalar<f32>>()->ValueAs<f32>());

    EXPECT_TRUE(new_v->BindingPoint().has_value());
    EXPECT_EQ(1u, new_v->BindingPoint()->group);
    EXPECT_EQ(2u, new_v->BindingPoint()->binding);

    auto& attrs = new_v->Attributes();
    EXPECT_TRUE(attrs.location.has_value());
    EXPECT_EQ(3u, attrs.location.value());

    EXPECT_TRUE(attrs.index.has_value());
    EXPECT_EQ(4u, attrs.index.value());

    EXPECT_TRUE(attrs.builtin.has_value());
    EXPECT_EQ(core::BuiltinValue::kFragDepth, attrs.builtin.value());

    EXPECT_TRUE(attrs.interpolation.has_value());
    EXPECT_EQ(core::InterpolationType::kFlat, attrs.interpolation->type);
    EXPECT_EQ(core::InterpolationSampling::kCentroid, attrs.interpolation->sampling);

    EXPECT_TRUE(attrs.invariant);
}

TEST_F(IR_VarTest, CloneWithName) {
    auto* v = b.Var("v", mod.Types().ptr(core::AddressSpace::kFunction, mod.Types().f32()));
    auto* new_v = clone_ctx.Clone(v);

    EXPECT_EQ(std::string("v"), mod.NameOf(new_v).Name());
}

}  // namespace
}  // namespace tint::core::ir
