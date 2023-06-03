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

#include "src/tint/ir/validate.h"
#include "gmock/gmock.h"
#include "src/tint/ir/builder.h"
#include "src/tint/ir/ir_test_helper.h"
#include "src/tint/type/pointer.h"

namespace tint::ir {
namespace {

using namespace tint::number_suffixes;  // NOLINT

using IR_ValidateTest = IRTestHelper;

TEST_F(IR_ValidateTest, RootBlock_Var) {
    mod.root_block = b.CreateRootBlockIfNeeded();
    mod.root_block->Append(b.Declare(mod.Types().pointer(
        mod.Types().i32(), builtin::AddressSpace::kPrivate, builtin::Access::kReadWrite)));
    auto res = ir::Validate(mod);
    EXPECT_TRUE(res) << res.Failure();
}

TEST_F(IR_ValidateTest, RootBlock_NonVar) {
    mod.root_block = b.CreateRootBlockIfNeeded();
    mod.root_block->Append(b.CreateLoop());

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure(), "error: root block: invalid instruction: tint::ir::Loop");
}

TEST_F(IR_ValidateTest, RootBlock_VarBadType) {
    mod.root_block = b.CreateRootBlockIfNeeded();
    mod.root_block->Append(b.Declare(mod.Types().i32()));
    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure(), "error: root block: 'var' type is not a pointer: tint::type::I32");
}

TEST_F(IR_ValidateTest, Function) {
    auto* f = b.CreateFunction("my_func", mod.Types().void_());
    mod.functions.Push(f);

    f->SetParams(
        utils::Vector{b.FunctionParam(mod.Types().i32()), b.FunctionParam(mod.Types().f32())});
    f->StartTarget()->SetInstructions(utils::Vector{b.Return(f)});
    auto res = ir::Validate(mod);
    EXPECT_TRUE(res) << res.Failure();
}

TEST_F(IR_ValidateTest, Function_NullStartTarget) {
    auto* f = b.CreateFunction("my_func", mod.Types().void_());
    mod.functions.Push(f);

    f->SetStartTarget(nullptr);
    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure(), "error: function 'my_func': null start target");
}

TEST_F(IR_ValidateTest, Function_ParamNull) {
    auto* f = b.CreateFunction("my_func", mod.Types().void_());
    mod.functions.Push(f);

    f->SetParams(utils::Vector<FunctionParam*, 1>{nullptr});
    f->StartTarget()->SetInstructions(utils::Vector{b.Return(f)});
    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure(), "error: function 'my_func': null parameter");
}

TEST_F(IR_ValidateTest, Block_NoBranchAtEnd) {
    auto* f = b.CreateFunction("my_func", mod.Types().void_());
    mod.functions.Push(f);

    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure(), "error: block: does not end in a branch");
}

TEST_F(IR_ValidateTest, Block_BranchInMiddle) {
    auto* f = b.CreateFunction("my_func", mod.Types().void_());
    mod.functions.Push(f);

    f->StartTarget()->SetInstructions(utils::Vector{b.Return(f), b.Return(f)});
    auto res = ir::Validate(mod);
    ASSERT_FALSE(res);
    EXPECT_EQ(res.Failure(), "error: block: branch which isn't the final instruction");
}

}  // namespace
}  // namespace tint::ir
