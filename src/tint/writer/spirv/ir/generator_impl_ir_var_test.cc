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
    auto* func = b.CreateFunction("foo", ty.void_());

    func->StartTarget()->SetInstructions(
        utils::Vector{b.Declare(ty.pointer(ty.i32(), builtin::AddressSpace::kFunction,
                                           builtin::Access::kReadWrite)),
                      b.Return(func)});

    ASSERT_TRUE(IRIsValid()) << Error();

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
    auto* func = b.CreateFunction("foo", ty.void_());

    auto* v = b.Declare(
        ty.pointer(ty.i32(), builtin::AddressSpace::kFunction, builtin::Access::kReadWrite));
    v->SetInitializer(b.Constant(42_i));

    func->StartTarget()->SetInstructions(utils::Vector{v, b.Return(func)});

    ASSERT_TRUE(IRIsValid()) << Error();

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
    auto* func = b.CreateFunction("foo", ty.void_());

    auto* v = b.Declare(
        ty.pointer(ty.i32(), builtin::AddressSpace::kFunction, builtin::Access::kReadWrite));
    func->StartTarget()->SetInstructions(utils::Vector{v, b.Return(func)});
    mod.SetName(v, "myvar");

    ASSERT_TRUE(IRIsValid()) << Error();

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
    auto* func = b.CreateFunction("foo", ty.void_());

    auto* v = b.Declare(
        ty.pointer(ty.i32(), builtin::AddressSpace::kFunction, builtin::Access::kReadWrite));
    v->SetInitializer(b.Constant(42_i));

    auto* i = b.CreateIf(b.Constant(true));
    i->True()->SetInstructions(utils::Vector{v, b.ExitIf(i)});
    i->False()->SetInstructions(utils::Vector{b.Return(func)});
    i->Merge()->SetInstructions(utils::Vector{b.Return(func)});

    func->StartTarget()->SetInstructions(utils::Vector{i});

    ASSERT_TRUE(IRIsValid()) << Error();

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
    auto* func = b.CreateFunction("foo", ty.void_());

    auto* store_ty = ty.i32();
    auto* v = b.Declare(
        ty.pointer(store_ty, builtin::AddressSpace::kFunction, builtin::Access::kReadWrite));
    func->StartTarget()->SetInstructions(utils::Vector{v, b.Load(v), b.Return(func)});

    ASSERT_TRUE(IRIsValid()) << Error();

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
    auto* func = b.CreateFunction("foo", ty.void_());

    auto* v = b.Declare(
        ty.pointer(ty.i32(), builtin::AddressSpace::kFunction, builtin::Access::kReadWrite));
    func->StartTarget()->SetInstructions(
        utils::Vector{v, b.Store(v, b.Constant(42_i)), b.Return(func)});

    ASSERT_TRUE(IRIsValid()) << Error();

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

TEST_F(SpvGeneratorImplTest, PrivateVar_NoInit) {
    b.CreateRootBlockIfNeeded()->SetInstructions(utils::Vector{b.Declare(
        ty.pointer(ty.i32(), builtin::AddressSpace::kPrivate, builtin::Access::kReadWrite))});

    ASSERT_TRUE(generator_.Generate()) << generator_.Diagnostics().str();
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %4 "unused_entry_point"
OpExecutionMode %4 LocalSize 1 1 1
OpName %4 "unused_entry_point"
%3 = OpTypeInt 32 1
%2 = OpTypePointer Private %3
%1 = OpVariable %2 Private
%5 = OpTypeVoid
%6 = OpTypeFunction %5
%4 = OpFunction %5 None %6
%7 = OpLabel
OpReturn
OpFunctionEnd
)");
}

TEST_F(SpvGeneratorImplTest, PrivateVar_WithInit) {
    auto* v = b.Declare(
        ty.pointer(ty.i32(), builtin::AddressSpace::kPrivate, builtin::Access::kReadWrite));
    b.CreateRootBlockIfNeeded()->SetInstructions(utils::Vector{v});
    v->SetInitializer(b.Constant(42_i));

    ASSERT_TRUE(generator_.Generate()) << generator_.Diagnostics().str();
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %5 "unused_entry_point"
OpExecutionMode %5 LocalSize 1 1 1
OpName %5 "unused_entry_point"
%3 = OpTypeInt 32 1
%2 = OpTypePointer Private %3
%4 = OpConstant %3 42
%1 = OpVariable %2 Private %4
%6 = OpTypeVoid
%7 = OpTypeFunction %6
%5 = OpFunction %6 None %7
%8 = OpLabel
OpReturn
OpFunctionEnd
)");
}

TEST_F(SpvGeneratorImplTest, PrivateVar_Name) {
    auto* v = b.Declare(
        ty.pointer(ty.i32(), builtin::AddressSpace::kPrivate, builtin::Access::kReadWrite));
    b.CreateRootBlockIfNeeded()->SetInstructions(utils::Vector{v});
    v->SetInitializer(b.Constant(42_i));
    mod.SetName(v, "myvar");

    ASSERT_TRUE(generator_.Generate()) << generator_.Diagnostics().str();
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %5 "unused_entry_point"
OpExecutionMode %5 LocalSize 1 1 1
OpName %1 "myvar"
OpName %5 "unused_entry_point"
%3 = OpTypeInt 32 1
%2 = OpTypePointer Private %3
%4 = OpConstant %3 42
%1 = OpVariable %2 Private %4
%6 = OpTypeVoid
%7 = OpTypeFunction %6
%5 = OpFunction %6 None %7
%8 = OpLabel
OpReturn
OpFunctionEnd
)");
}

TEST_F(SpvGeneratorImplTest, PrivateVar_LoadAndStore) {
    auto* func = b.CreateFunction("foo", ty.void_(), ir::Function::PipelineStage::kFragment);
    mod.functions.Push(func);

    auto* store_ty = ty.i32();
    auto* v = b.Declare(
        ty.pointer(store_ty, builtin::AddressSpace::kPrivate, builtin::Access::kReadWrite));
    b.CreateRootBlockIfNeeded()->SetInstructions(utils::Vector{v});
    v->SetInitializer(b.Constant(42_i));

    auto* load = b.Load(v);
    auto* add = b.Add(store_ty, v, b.Constant(1_i));
    auto* store = b.Store(v, add);
    func->StartTarget()->SetInstructions(utils::Vector{load, add, store, b.Return(func)});

    ASSERT_TRUE(generator_.Generate()) << generator_.Diagnostics().str();
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %5 "foo"
OpExecutionMode %5 OriginUpperLeft
OpName %5 "foo"
%3 = OpTypeInt 32 1
%2 = OpTypePointer Private %3
%4 = OpConstant %3 42
%1 = OpVariable %2 Private %4
%6 = OpTypeVoid
%7 = OpTypeFunction %6
%11 = OpConstant %3 1
%5 = OpFunction %6 None %7
%8 = OpLabel
%9 = OpLoad %3 %1
%10 = OpIAdd %3 %1 %11
OpStore %1 %10
OpReturn
OpFunctionEnd
)");
}

TEST_F(SpvGeneratorImplTest, WorkgroupVar) {
    b.CreateRootBlockIfNeeded()->SetInstructions(utils::Vector{b.Declare(
        ty.pointer(ty.i32(), builtin::AddressSpace::kWorkgroup, builtin::Access::kReadWrite))});

    ASSERT_TRUE(generator_.Generate()) << generator_.Diagnostics().str();
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %4 "unused_entry_point"
OpExecutionMode %4 LocalSize 1 1 1
OpName %4 "unused_entry_point"
%3 = OpTypeInt 32 1
%2 = OpTypePointer Workgroup %3
%1 = OpVariable %2 Workgroup
%5 = OpTypeVoid
%6 = OpTypeFunction %5
%4 = OpFunction %5 None %6
%7 = OpLabel
OpReturn
OpFunctionEnd
)");
}

TEST_F(SpvGeneratorImplTest, WorkgroupVar_Name) {
    auto* v = b.Declare(
        ty.pointer(ty.i32(), builtin::AddressSpace::kWorkgroup, builtin::Access::kReadWrite));
    b.CreateRootBlockIfNeeded()->SetInstructions(utils::Vector{v});
    mod.SetName(v, "myvar");

    ASSERT_TRUE(generator_.Generate()) << generator_.Diagnostics().str();
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %4 "unused_entry_point"
OpExecutionMode %4 LocalSize 1 1 1
OpName %1 "myvar"
OpName %4 "unused_entry_point"
%3 = OpTypeInt 32 1
%2 = OpTypePointer Workgroup %3
%1 = OpVariable %2 Workgroup
%5 = OpTypeVoid
%6 = OpTypeFunction %5
%4 = OpFunction %5 None %6
%7 = OpLabel
OpReturn
OpFunctionEnd
)");
}

TEST_F(SpvGeneratorImplTest, WorkgroupVar_LoadAndStore) {
    auto* func = b.CreateFunction("foo", ty.void_(), ir::Function::PipelineStage::kCompute,
                                  std::array{1u, 1u, 1u});
    mod.functions.Push(func);

    auto* store_ty = ty.i32();
    auto* v = b.Declare(
        ty.pointer(store_ty, builtin::AddressSpace::kWorkgroup, builtin::Access::kReadWrite));
    b.CreateRootBlockIfNeeded()->SetInstructions(utils::Vector{v});

    auto* load = b.Load(v);
    auto* add = b.Add(store_ty, v, b.Constant(1_i));
    auto* store = b.Store(v, add);
    func->StartTarget()->SetInstructions(utils::Vector{load, add, store, b.Return(func)});

    ASSERT_TRUE(generator_.Generate()) << generator_.Diagnostics().str();
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %4 "foo"
OpExecutionMode %4 LocalSize 1 1 1
OpName %4 "foo"
%3 = OpTypeInt 32 1
%2 = OpTypePointer Workgroup %3
%1 = OpVariable %2 Workgroup
%5 = OpTypeVoid
%6 = OpTypeFunction %5
%10 = OpConstant %3 1
%4 = OpFunction %5 None %6
%7 = OpLabel
%8 = OpLoad %3 %1
%9 = OpIAdd %3 %1 %10
OpStore %1 %9
OpReturn
OpFunctionEnd
)");
}

TEST_F(SpvGeneratorImplTest, WorkgroupVar_ZeroInitializeWithExtension) {
    b.CreateRootBlockIfNeeded()->SetInstructions(utils::Vector{b.Declare(
        ty.pointer(ty.i32(), builtin::AddressSpace::kWorkgroup, builtin::Access::kReadWrite))});

    // Create a generator with the zero_init_workgroup_memory flag set to `true`.
    spirv::GeneratorImplIr gen(&mod, true);
    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics().str();
    EXPECT_EQ(DumpModule(gen.Module()), R"(OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %5 "unused_entry_point"
OpExecutionMode %5 LocalSize 1 1 1
OpName %5 "unused_entry_point"
%3 = OpTypeInt 32 1
%2 = OpTypePointer Workgroup %3
%4 = OpConstantNull %3
%1 = OpVariable %2 Workgroup %4
%6 = OpTypeVoid
%7 = OpTypeFunction %6
%5 = OpFunction %6 None %7
%8 = OpLabel
OpReturn
OpFunctionEnd
)");
}

TEST_F(SpvGeneratorImplTest, StorageVar) {
    auto* v = b.Declare(
        ty.pointer(ty.i32(), builtin::AddressSpace::kStorage, builtin::Access::kReadWrite));
    v->SetBindingPoint(0, 0);
    b.CreateRootBlockIfNeeded()->SetInstructions(utils::Vector{v});

    ASSERT_TRUE(generator_.Generate()) << generator_.Diagnostics().str();
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %5 "unused_entry_point"
OpExecutionMode %5 LocalSize 1 1 1
OpMemberName %3 0 "tint_symbol"
OpName %3 "tint_symbol_1"
OpName %5 "unused_entry_point"
OpMemberDecorate %3 0 Offset 0
OpDecorate %3 Block
OpDecorate %1 DescriptorSet 0
OpDecorate %1 Binding 0
%4 = OpTypeInt 32 1
%3 = OpTypeStruct %4
%2 = OpTypePointer StorageBuffer %3
%1 = OpVariable %2 StorageBuffer
%6 = OpTypeVoid
%7 = OpTypeFunction %6
%5 = OpFunction %6 None %7
%8 = OpLabel
OpReturn
OpFunctionEnd
)");
}

TEST_F(SpvGeneratorImplTest, StorageVar_Name) {
    auto* v = b.Declare(
        ty.pointer(ty.i32(), builtin::AddressSpace::kStorage, builtin::Access::kReadWrite));
    v->SetBindingPoint(0, 0);
    b.CreateRootBlockIfNeeded()->SetInstructions(utils::Vector{v});
    mod.SetName(v, "myvar");

    ASSERT_TRUE(generator_.Generate()) << generator_.Diagnostics().str();
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %5 "unused_entry_point"
OpExecutionMode %5 LocalSize 1 1 1
OpMemberName %3 0 "tint_symbol"
OpName %3 "tint_symbol_1"
OpName %5 "unused_entry_point"
OpMemberDecorate %3 0 Offset 0
OpDecorate %3 Block
OpDecorate %1 DescriptorSet 0
OpDecorate %1 Binding 0
%4 = OpTypeInt 32 1
%3 = OpTypeStruct %4
%2 = OpTypePointer StorageBuffer %3
%1 = OpVariable %2 StorageBuffer
%6 = OpTypeVoid
%7 = OpTypeFunction %6
%5 = OpFunction %6 None %7
%8 = OpLabel
OpReturn
OpFunctionEnd
)");
}

TEST_F(SpvGeneratorImplTest, StorageVar_LoadAndStore) {
    auto* v = b.Declare(
        ty.pointer(ty.i32(), builtin::AddressSpace::kStorage, builtin::Access::kReadWrite));
    v->SetBindingPoint(0, 0);
    b.CreateRootBlockIfNeeded()->SetInstructions(utils::Vector{v});

    auto* func = b.CreateFunction("foo", ty.void_(), ir::Function::PipelineStage::kCompute,
                                  std::array{1u, 1u, 1u});
    mod.functions.Push(func);

    auto* load = b.Load(v);
    auto* add = b.Add(ty.i32(), v, b.Constant(1_i));
    auto* store = b.Store(v, add);
    func->StartTarget()->SetInstructions(utils::Vector{load, add, store, b.Return(func)});

    ASSERT_TRUE(generator_.Generate()) << generator_.Diagnostics().str();
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %5 "foo"
OpExecutionMode %5 LocalSize 1 1 1
OpMemberName %3 0 "tint_symbol"
OpName %3 "tint_symbol_1"
OpName %5 "foo"
OpMemberDecorate %3 0 Offset 0
OpDecorate %3 Block
OpDecorate %1 DescriptorSet 0
OpDecorate %1 Binding 0
%4 = OpTypeInt 32 1
%3 = OpTypeStruct %4
%2 = OpTypePointer StorageBuffer %3
%1 = OpVariable %2 StorageBuffer
%6 = OpTypeVoid
%7 = OpTypeFunction %6
%10 = OpTypePointer StorageBuffer %4
%12 = OpTypeInt 32 0
%11 = OpConstant %12 0
%16 = OpConstant %4 1
%5 = OpFunction %6 None %7
%8 = OpLabel
%9 = OpAccessChain %10 %1 %11
%13 = OpLoad %4 %9
%14 = OpAccessChain %10 %1 %11
%15 = OpIAdd %4 %14 %16
%17 = OpAccessChain %10 %1 %11
OpStore %17 %15
OpReturn
OpFunctionEnd
)");
}

TEST_F(SpvGeneratorImplTest, UniformVar) {
    auto* v = b.Declare(
        ty.pointer(ty.i32(), builtin::AddressSpace::kUniform, builtin::Access::kReadWrite));
    v->SetBindingPoint(0, 0);
    b.CreateRootBlockIfNeeded()->SetInstructions(utils::Vector{v});

    ASSERT_TRUE(generator_.Generate()) << generator_.Diagnostics().str();
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %5 "unused_entry_point"
OpExecutionMode %5 LocalSize 1 1 1
OpMemberName %3 0 "tint_symbol"
OpName %3 "tint_symbol_1"
OpName %5 "unused_entry_point"
OpMemberDecorate %3 0 Offset 0
OpDecorate %3 Block
OpDecorate %1 DescriptorSet 0
OpDecorate %1 Binding 0
%4 = OpTypeInt 32 1
%3 = OpTypeStruct %4
%2 = OpTypePointer Uniform %3
%1 = OpVariable %2 Uniform
%6 = OpTypeVoid
%7 = OpTypeFunction %6
%5 = OpFunction %6 None %7
%8 = OpLabel
OpReturn
OpFunctionEnd
)");
}

TEST_F(SpvGeneratorImplTest, UniformVar_Name) {
    auto* v = b.Declare(
        ty.pointer(ty.i32(), builtin::AddressSpace::kUniform, builtin::Access::kReadWrite));
    v->SetBindingPoint(0, 0);
    b.CreateRootBlockIfNeeded()->SetInstructions(utils::Vector{v});
    mod.SetName(v, "myvar");

    ASSERT_TRUE(generator_.Generate()) << generator_.Diagnostics().str();
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %5 "unused_entry_point"
OpExecutionMode %5 LocalSize 1 1 1
OpMemberName %3 0 "tint_symbol"
OpName %3 "tint_symbol_1"
OpName %5 "unused_entry_point"
OpMemberDecorate %3 0 Offset 0
OpDecorate %3 Block
OpDecorate %1 DescriptorSet 0
OpDecorate %1 Binding 0
%4 = OpTypeInt 32 1
%3 = OpTypeStruct %4
%2 = OpTypePointer Uniform %3
%1 = OpVariable %2 Uniform
%6 = OpTypeVoid
%7 = OpTypeFunction %6
%5 = OpFunction %6 None %7
%8 = OpLabel
OpReturn
OpFunctionEnd
)");
}

TEST_F(SpvGeneratorImplTest, UniformVar_Load) {
    auto* v = b.Declare(
        ty.pointer(ty.i32(), builtin::AddressSpace::kUniform, builtin::Access::kReadWrite));
    v->SetBindingPoint(0, 0);
    b.CreateRootBlockIfNeeded()->SetInstructions(utils::Vector{v});

    auto* func = b.CreateFunction("foo", ty.void_(), ir::Function::PipelineStage::kCompute,
                                  std::array{1u, 1u, 1u});
    mod.functions.Push(func);

    auto* load = b.Load(v);
    func->StartTarget()->SetInstructions(utils::Vector{load, b.Return(func)});

    ASSERT_TRUE(generator_.Generate()) << generator_.Diagnostics().str();
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %5 "foo"
OpExecutionMode %5 LocalSize 1 1 1
OpMemberName %3 0 "tint_symbol"
OpName %3 "tint_symbol_1"
OpName %5 "foo"
OpMemberDecorate %3 0 Offset 0
OpDecorate %3 Block
OpDecorate %1 DescriptorSet 0
OpDecorate %1 Binding 0
%4 = OpTypeInt 32 1
%3 = OpTypeStruct %4
%2 = OpTypePointer Uniform %3
%1 = OpVariable %2 Uniform
%6 = OpTypeVoid
%7 = OpTypeFunction %6
%10 = OpTypePointer Uniform %4
%12 = OpTypeInt 32 0
%11 = OpConstant %12 0
%5 = OpFunction %6 None %7
%8 = OpLabel
%9 = OpAccessChain %10 %1 %11
%13 = OpLoad %4 %9
OpReturn
OpFunctionEnd
)");
}

}  // namespace
}  // namespace tint::writer::spirv
