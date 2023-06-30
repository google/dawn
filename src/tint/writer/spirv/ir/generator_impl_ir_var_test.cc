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
#include "src/tint/type/sampled_texture.h"
#include "src/tint/writer/spirv/ir/test_helper_ir.h"

namespace tint::writer::spirv {
namespace {

using namespace tint::builtin::fluent_types;  // NOLINT
using namespace tint::number_suffixes;        // NOLINT

TEST_F(SpvGeneratorImplTest, FunctionVar_NoInit) {
    auto* func = b.Function("foo", ty.void_());
    b.With(func->Block(), [&] {
        b.Var("v", ty.ptr<function, i32>());
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%v = OpVariable %_ptr_Function_int Function");
}

TEST_F(SpvGeneratorImplTest, FunctionVar_WithInit) {
    auto* func = b.Function("foo", ty.void_());
    b.With(func->Block(), [&] {
        auto* v = b.Var("v", ty.ptr<function, i32>());
        v->SetInitializer(b.Constant(42_i));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%v = OpVariable %_ptr_Function_int Function");
    EXPECT_INST("OpStore %v %int_42");
}

TEST_F(SpvGeneratorImplTest, FunctionVar_DeclInsideBlock) {
    auto* func = b.Function("foo", ty.void_());
    b.With(func->Block(), [&] {
        auto* i = b.If(true);
        b.With(i->True(), [&] {
            auto* v = b.Var("v", ty.ptr<function, i32>());
            v->SetInitializer(b.Constant(42_i));
            b.ExitIf(i);
        });
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
        %foo = OpFunction %void None %3
          %4 = OpLabel
          %v = OpVariable %_ptr_Function_int Function
               OpSelectionMerge %5 None
               OpBranchConditional %true %6 %5
          %6 = OpLabel
               OpStore %v %int_42
               OpBranch %5
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
)");
}

TEST_F(SpvGeneratorImplTest, FunctionVar_Load) {
    auto* func = b.Function("foo", ty.void_());
    b.With(func->Block(), [&] {
        auto* v = b.Var("v", ty.ptr<function, i32>());
        auto* result = b.Load(v);
        b.Return(func);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%v = OpVariable %_ptr_Function_int Function");
    EXPECT_INST("%result = OpLoad %int %v");
}

TEST_F(SpvGeneratorImplTest, FunctionVar_Store) {
    auto* func = b.Function("foo", ty.void_());
    b.With(func->Block(), [&] {
        auto* v = b.Var("v", ty.ptr<function, i32>());
        b.Store(v, 42_i);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%v = OpVariable %_ptr_Function_int Function");
    EXPECT_INST("OpStore %v %int_42");
}

TEST_F(SpvGeneratorImplTest, PrivateVar_NoInit) {
    b.RootBlock()->Append(b.Var("v", ty.ptr<private_, i32>()));

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%v = OpVariable %_ptr_Private_int Private");
}

TEST_F(SpvGeneratorImplTest, PrivateVar_WithInit) {
    auto* v = b.Var("v", ty.ptr<private_, i32>());
    v->SetInitializer(b.Constant(42_i));
    b.RootBlock()->Append(v);

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%v = OpVariable %_ptr_Private_int Private %int_42");
}

TEST_F(SpvGeneratorImplTest, PrivateVar_LoadAndStore) {
    auto* v = b.Var("v", ty.ptr<private_, i32>());
    v->SetInitializer(b.Constant(42_i));
    b.RootBlock()->Append(v);

    auto* func = b.Function("foo", ty.void_(), ir::Function::PipelineStage::kFragment);
    b.With(func->Block(), [&] {
        auto* load = b.Load(v);
        auto* add = b.Add(ty.i32(), load, 1_i);
        b.Store(v, add);
        b.Return(func);
        mod.SetName(load, "load");
        mod.SetName(add, "add");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%v = OpVariable %_ptr_Private_int Private %int_42");
    EXPECT_INST("%load = OpLoad %int %v");
    EXPECT_INST("OpStore %v %add");
}

TEST_F(SpvGeneratorImplTest, WorkgroupVar) {
    b.RootBlock()->Append(b.Var("v", ty.ptr<workgroup, i32>()));

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%v = OpVariable %_ptr_Workgroup_int Workgroup");
}

TEST_F(SpvGeneratorImplTest, WorkgroupVar_LoadAndStore) {
    auto* v = b.RootBlock()->Append(b.Var("v", ty.ptr<workgroup, i32>()));

    auto* func = b.Function("foo", ty.void_(), ir::Function::PipelineStage::kCompute,
                            std::array{1u, 1u, 1u});
    b.With(func->Block(), [&] {
        auto* load = b.Load(v);
        auto* add = b.Add(ty.i32(), load, 1_i);
        b.Store(v, add);
        b.Return(func);
        mod.SetName(load, "load");
        mod.SetName(add, "add");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%v = OpVariable %_ptr_Workgroup_int Workgroup");
    EXPECT_INST("%load = OpLoad %int %v");
    EXPECT_INST("OpStore %v %add");
}

TEST_F(SpvGeneratorImplTest, WorkgroupVar_ZeroInitializeWithExtension) {
    b.RootBlock()->Append(b.Var("v", ty.ptr<workgroup, i32>()));

    // Create a generator with the zero_init_workgroup_memory flag set to `true`.
    spirv::GeneratorImplIr gen(&mod, true);
    ASSERT_TRUE(Generate(gen)) << Error() << output_;
    EXPECT_INST("%4 = OpConstantNull %int");
    EXPECT_INST("%v = OpVariable %_ptr_Workgroup_int Workgroup %4");
}

TEST_F(SpvGeneratorImplTest, StorageVar) {
    auto* v = b.Var("v", ty.ptr<storage, i32>());
    v->SetBindingPoint(0, 0);
    b.RootBlock()->Append(v);

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
               OpDecorate %tint_symbol_1 Block
               OpDecorate %1 DescriptorSet 0
               OpDecorate %1 Binding 0
)");
    EXPECT_INST(R"(
%tint_symbol_1 = OpTypeStruct %int
%_ptr_StorageBuffer_tint_symbol_1 = OpTypePointer StorageBuffer %tint_symbol_1
          %1 = OpVariable %_ptr_StorageBuffer_tint_symbol_1 StorageBuffer
)");
}

TEST_F(SpvGeneratorImplTest, StorageVar_LoadAndStore) {
    auto* v = b.Var("v", ty.ptr<storage, i32>());
    v->SetBindingPoint(0, 0);
    b.RootBlock()->Append(v);

    auto* func = b.Function("foo", ty.void_(), ir::Function::PipelineStage::kCompute,
                            std::array{1u, 1u, 1u});
    b.With(func->Block(), [&] {
        auto* load = b.Load(v);
        auto* add = b.Add(ty.i32(), load, 1_i);
        b.Store(v, add);
        b.Return(func);
        mod.SetName(load, "load");
        mod.SetName(add, "add");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
          %9 = OpAccessChain %_ptr_StorageBuffer_int %1 %uint_0
       %load = OpLoad %int %9
        %add = OpIAdd %int %load %int_1
         %16 = OpAccessChain %_ptr_StorageBuffer_int %1 %uint_0
               OpStore %16 %add
)");
}

TEST_F(SpvGeneratorImplTest, UniformVar) {
    auto* v = b.Var("v", ty.ptr<uniform, i32>());
    v->SetBindingPoint(0, 0);
    b.RootBlock()->Append(v);

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
               OpDecorate %tint_symbol_1 Block
               OpDecorate %1 DescriptorSet 0
               OpDecorate %1 Binding 0
)");
    EXPECT_INST(R"(
%tint_symbol_1 = OpTypeStruct %int
%_ptr_Uniform_tint_symbol_1 = OpTypePointer Uniform %tint_symbol_1
          %1 = OpVariable %_ptr_Uniform_tint_symbol_1 Uniform
)");
}

TEST_F(SpvGeneratorImplTest, UniformVar_Load) {
    auto* v = b.Var("v", ty.ptr<uniform, i32>());
    v->SetBindingPoint(0, 0);
    b.RootBlock()->Append(v);

    auto* func = b.Function("foo", ty.void_(), ir::Function::PipelineStage::kCompute,
                            std::array{1u, 1u, 1u});
    b.With(func->Block(), [&] {
        auto* load = b.Load(v);
        b.Return(func);
        mod.SetName(load, "load");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
          %9 = OpAccessChain %_ptr_Uniform_int %1 %uint_0
       %load = OpLoad %int %9
)");
}

TEST_F(SpvGeneratorImplTest, SamplerVar) {
    auto* v =
        b.Var("v", ty.ptr(builtin::AddressSpace::kHandle, ty.sampler(), builtin::Access::kRead));
    v->SetBindingPoint(0, 0);
    b.RootBlock()->Append(v);

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
               OpDecorate %v DescriptorSet 0
               OpDecorate %v Binding 0
)");
    EXPECT_INST(R"(
          %3 = OpTypeSampler
%_ptr_UniformConstant_3 = OpTypePointer UniformConstant %3
          %v = OpVariable %_ptr_UniformConstant_3 UniformConstant
)");
}

TEST_F(SpvGeneratorImplTest, SamplerVar_Load) {
    auto* v =
        b.Var("v", ty.ptr(builtin::AddressSpace::kHandle, ty.sampler(), builtin::Access::kRead));
    v->SetBindingPoint(0, 0);
    b.RootBlock()->Append(v);

    auto* func = b.Function("foo", ty.void_());
    b.With(func->Block(), [&] {
        auto* load = b.Load(v);
        b.Return(func);
        mod.SetName(load, "load");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%load = OpLoad %3 %v");
}

TEST_F(SpvGeneratorImplTest, TextureVar) {
    auto* v = b.Var("v", ty.ptr(builtin::AddressSpace::kHandle,
                                ty.Get<type::SampledTexture>(type::TextureDimension::k2d, ty.f32()),
                                builtin::Access::kRead));
    v->SetBindingPoint(0, 0);
    b.RootBlock()->Append(v);

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
               OpDecorate %v DescriptorSet 0
               OpDecorate %v Binding 0
)");
    EXPECT_INST(R"(
          %3 = OpTypeImage %float 2D 0 0 0 1 Unknown
%_ptr_UniformConstant_3 = OpTypePointer UniformConstant %3
          %v = OpVariable %_ptr_UniformConstant_3 UniformConstant
)");
}

TEST_F(SpvGeneratorImplTest, TextureVar_Load) {
    auto* v = b.Var("v", ty.ptr(builtin::AddressSpace::kHandle,
                                ty.Get<type::SampledTexture>(type::TextureDimension::k2d, ty.f32()),
                                builtin::Access::kRead));
    v->SetBindingPoint(0, 0);
    b.RootBlock()->Append(v);

    auto* func = b.Function("foo", ty.void_());
    b.With(func->Block(), [&] {
        auto* load = b.Load(v);
        b.Return(func);
        mod.SetName(load, "load");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%load = OpLoad %3 %v");
}

}  // namespace
}  // namespace tint::writer::spirv
