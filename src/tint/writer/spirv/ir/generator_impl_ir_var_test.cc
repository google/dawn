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

#include "src/tint/type/pointer.h"
#include "src/tint/writer/spirv/ir/test_helper_ir.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::writer::spirv {
namespace {

TEST_F(SpvGeneratorImplTest, FunctionVar_NoInit) {
    auto* func = b.CreateFunction("foo", mod.Types().void_());

    auto* ty = mod.Types().Get<type::Pointer>(mod.Types().i32(), builtin::AddressSpace::kFunction,
                                              builtin::Access::kReadWrite);
    func->StartTarget()->SetInstructions(utils::Vector{b.Declare(ty), b.Return(func)});

    generator_.EmitFunction(func);
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpName %1 "foo"
%2 = OpTypeVoid
%3 = OpTypeFunction %2
%7 = OpTypeInt 32 1
%6 = OpTypePointer Function %7
%1 = OpFunction %2 None %3
%4 = OpLabel
%5 = OpVariable %6 Function
OpReturn
OpFunctionEnd
)");
}

TEST_F(SpvGeneratorImplTest, FunctionVar_WithInit) {
    auto* func = b.CreateFunction("foo", mod.Types().void_());

    auto* ty = mod.Types().Get<type::Pointer>(mod.Types().i32(), builtin::AddressSpace::kFunction,
                                              builtin::Access::kReadWrite);
    auto* v = b.Declare(ty);
    v->SetInitializer(b.Constant(42_i));

    func->StartTarget()->SetInstructions(utils::Vector{v, b.Return(func)});

    generator_.EmitFunction(func);
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpName %1 "foo"
%2 = OpTypeVoid
%3 = OpTypeFunction %2
%7 = OpTypeInt 32 1
%6 = OpTypePointer Function %7
%8 = OpConstant %7 42
%1 = OpFunction %2 None %3
%4 = OpLabel
%5 = OpVariable %6 Function
OpStore %5 %8
OpReturn
OpFunctionEnd
)");
}

TEST_F(SpvGeneratorImplTest, FunctionVar_Name) {
    auto* func = b.CreateFunction("foo", mod.Types().void_());

    auto* ty = mod.Types().Get<type::Pointer>(mod.Types().i32(), builtin::AddressSpace::kFunction,
                                              builtin::Access::kReadWrite);
    auto* v = b.Declare(ty);
    func->StartTarget()->SetInstructions(utils::Vector{v, b.Return(func)});
    mod.SetName(v, "myvar");

    generator_.EmitFunction(func);
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpName %1 "foo"
OpName %5 "myvar"
%2 = OpTypeVoid
%3 = OpTypeFunction %2
%7 = OpTypeInt 32 1
%6 = OpTypePointer Function %7
%1 = OpFunction %2 None %3
%4 = OpLabel
%5 = OpVariable %6 Function
OpReturn
OpFunctionEnd
)");
}

TEST_F(SpvGeneratorImplTest, FunctionVar_DeclInsideBlock) {
    auto* func = b.CreateFunction("foo", mod.Types().void_());

    auto* ty = mod.Types().Get<type::Pointer>(mod.Types().i32(), builtin::AddressSpace::kFunction,
                                              builtin::Access::kReadWrite);
    auto* v = b.Declare(ty);
    v->SetInitializer(b.Constant(42_i));

    auto* i = b.CreateIf(b.Constant(true));
    i->True()->SetInstructions(utils::Vector{v, b.Branch(i->Merge())});
    i->False()->SetInstructions(utils::Vector{b.Return(func)});
    i->Merge()->SetInstructions(utils::Vector{b.Return(func)});

    func->StartTarget()->SetInstructions(utils::Vector{i});

    generator_.EmitFunction(func);
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpName %1 "foo"
%2 = OpTypeVoid
%3 = OpTypeFunction %2
%9 = OpTypeBool
%8 = OpConstantTrue %9
%12 = OpTypeInt 32 1
%11 = OpTypePointer Function %12
%13 = OpConstant %12 42
%1 = OpFunction %2 None %3
%4 = OpLabel
%10 = OpVariable %11 Function
OpSelectionMerge %5 None
OpBranchConditional %8 %6 %7
%6 = OpLabel
OpStore %10 %13
OpBranch %5
%7 = OpLabel
OpReturn
%5 = OpLabel
OpReturn
OpFunctionEnd
)");
}

TEST_F(SpvGeneratorImplTest, FunctionVar_Load) {
    auto* func = b.CreateFunction("foo", mod.Types().void_());

    auto* store_ty = mod.Types().i32();
    auto* ty = mod.Types().Get<type::Pointer>(store_ty, builtin::AddressSpace::kFunction,
                                              builtin::Access::kReadWrite);
    auto* v = b.Declare(ty);
    func->StartTarget()->SetInstructions(utils::Vector{v, b.Load(v), b.Return(func)});

    generator_.EmitFunction(func);
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpName %1 "foo"
%2 = OpTypeVoid
%3 = OpTypeFunction %2
%7 = OpTypeInt 32 1
%6 = OpTypePointer Function %7
%1 = OpFunction %2 None %3
%4 = OpLabel
%5 = OpVariable %6 Function
%8 = OpLoad %7 %5
OpReturn
OpFunctionEnd
)");
}

TEST_F(SpvGeneratorImplTest, FunctionVar_Store) {
    auto* func = b.CreateFunction("foo", mod.Types().void_());

    auto* ty = mod.Types().Get<type::Pointer>(mod.Types().i32(), builtin::AddressSpace::kFunction,
                                              builtin::Access::kReadWrite);
    auto* v = b.Declare(ty);
    func->StartTarget()->SetInstructions(
        utils::Vector{v, b.Store(v, b.Constant(42_i)), b.Return(func)});

    generator_.EmitFunction(func);
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpName %1 "foo"
%2 = OpTypeVoid
%3 = OpTypeFunction %2
%7 = OpTypeInt 32 1
%6 = OpTypePointer Function %7
%8 = OpConstant %7 42
%1 = OpFunction %2 None %3
%4 = OpLabel
%5 = OpVariable %6 Function
OpStore %5 %8
OpReturn
OpFunctionEnd
)");
}

}  // namespace
}  // namespace tint::writer::spirv
