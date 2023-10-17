// Copyright 2023 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <string>

#include "gtest/gtest-spi.h"
#include "src/tint/lang/core/ir/function.h"
#include "src/tint/lang/core/ir/ir_helper_test.h"

namespace tint::core::ir {
namespace {

using namespace tint::core::number_suffixes;  // NOLINT
using IR_FunctionTest = IRTestHelper;

TEST_F(IR_FunctionTest, Fail_NullReturnType) {
    EXPECT_FATAL_FAILURE(
        {
            Module mod;
            Builder b{mod};
            b.Function("my_func", nullptr);
        },
        "");
}

TEST_F(IR_FunctionTest, Fail_DoubleReturnBuiltin) {
    EXPECT_FATAL_FAILURE(
        {
            Module mod;
            Builder b{mod};
            auto* f = b.Function("my_func", mod.Types().void_());
            f->SetReturnBuiltin(Function::ReturnBuiltin::kFragDepth);
            f->SetReturnBuiltin(Function::ReturnBuiltin::kPosition);
        },
        "");
}

TEST_F(IR_FunctionTest, Fail_NullParam) {
    EXPECT_FATAL_FAILURE(
        {
            Module mod;
            Builder b{mod};
            auto* f = b.Function("my_func", mod.Types().void_());
            f->SetParams({nullptr});
        },
        "");
}

TEST_F(IR_FunctionTest, Fail_NullBlock) {
    EXPECT_FATAL_FAILURE(
        {
            Module mod;
            Builder b{mod};
            auto* f = b.Function("my_func", mod.Types().void_());
            f->SetBlock(nullptr);
        },
        "");
}

TEST_F(IR_FunctionTest, Clone) {
    auto* f =
        b.Function("my_func", mod.Types().i32(), Function::PipelineStage::kCompute, {{2, 3, 4}});
    f->SetReturnBuiltin(Function::ReturnBuiltin::kFragDepth);
    f->SetReturnLocation(
        1, Interpolation{core::InterpolationType::kFlat, core::InterpolationSampling::kCentroid});
    f->SetReturnInvariant(true);

    auto* param1 = b.FunctionParam("a", mod.Types().i32());
    auto* param2 = b.FunctionParam("b", mod.Types().f32());
    f->SetParams({param1, param2});

    auto* new_param1 = clone_ctx.Clone(param1);
    auto* new_param2 = clone_ctx.Clone(param2);
    auto* new_f = clone_ctx.Clone(f);

    EXPECT_NE(f, new_f);
    EXPECT_EQ(std::string("my_func"), mod.NameOf(new_f).Name());

    EXPECT_EQ(Function::PipelineStage::kCompute, new_f->Stage());
    EXPECT_TRUE(new_f->WorkgroupSize().has_value());
    auto wg = new_f->WorkgroupSize().value();
    EXPECT_EQ(2u, wg[0]);
    EXPECT_EQ(3u, wg[1]);
    EXPECT_EQ(4u, wg[2]);

    EXPECT_EQ(mod.Types().i32(), new_f->ReturnType());

    EXPECT_TRUE(new_f->ReturnBuiltin().has_value());
    EXPECT_EQ(Function::ReturnBuiltin::kFragDepth, new_f->ReturnBuiltin().value());

    EXPECT_TRUE(new_f->ReturnLocation().has_value());
    auto loc = new_f->ReturnLocation().value();
    EXPECT_EQ(1u, loc.value);
    EXPECT_EQ(core::InterpolationType::kFlat, loc.interpolation->type);
    EXPECT_EQ(core::InterpolationSampling::kCentroid, loc.interpolation->sampling);

    EXPECT_TRUE(new_f->ReturnInvariant());

    EXPECT_EQ(2u, new_f->Params().Length());
    EXPECT_EQ(new_param1, new_f->Params()[0]);
    EXPECT_EQ(new_param2, new_f->Params()[1]);

    // Cloned functions are not automatically added to the module.
    EXPECT_EQ(mod.functions.Length(), 1u);
}

TEST_F(IR_FunctionTest, CloneWithExits) {
    auto* f = b.Function("my_func", mod.Types().void_());
    b.Append(f->Block(), [&] { b.Return(f); });

    auto* new_f = clone_ctx.Clone(f);
    EXPECT_EQ(1u, new_f->Block()->Length());
    EXPECT_TRUE(new_f->Block()->Front()->Is<Return>());
    EXPECT_EQ(new_f, new_f->Block()->Front()->As<Return>()->Func());
}

}  // namespace
}  // namespace tint::core::ir
