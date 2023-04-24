// Copyright 2020 The Tint Authors.
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

#include "src/tint/ast/stage_attribute.h"
#include "src/tint/writer/msl/test_helper.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::writer::msl {
namespace {

using MslGeneratorImplTest = TestHelper;

TEST_F(MslGeneratorImplTest, InvalidProgram) {
    Diagnostics().add_error(diag::System::Writer, "make the program invalid");
    ASSERT_FALSE(IsValid());
    auto program = std::make_unique<Program>(std::move(*this));
    ASSERT_FALSE(program->IsValid());
    auto result = Generate(program.get(), Options{});
    EXPECT_EQ(result.error, "input program is not valid");
}

TEST_F(MslGeneratorImplTest, UnsupportedExtension) {
    Enable(Source{{12, 34}}, builtin::Extension::kUndefined);

    GeneratorImpl& gen = Build();

    ASSERT_FALSE(gen.Generate());
    EXPECT_EQ(gen.Diagnostics().str(),
              R"(12:34 error: MSL backend does not support extension 'undefined')");
}

TEST_F(MslGeneratorImplTest, Generate) {
    Func("my_func", utils::Empty, ty.void_(), utils::Empty,
         utils::Vector{
             Stage(ast::PipelineStage::kCompute),
             WorkgroupSize(1_i),
         });

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

using namespace metal;
kernel void my_func() {
  return;
}

)");
}

struct MslBuiltinData {
    builtin::BuiltinValue builtin;
    const char* attribute_name;
};
inline std::ostream& operator<<(std::ostream& out, MslBuiltinData data) {
    utils::StringStream str;
    str << data.builtin;
    out << str.str();
    return out;
}
using MslBuiltinConversionTest = TestParamHelper<MslBuiltinData>;
TEST_P(MslBuiltinConversionTest, Emit) {
    auto params = GetParam();

    GeneratorImpl& gen = Build();

    EXPECT_EQ(gen.builtin_to_attribute(params.builtin), std::string(params.attribute_name));
}
INSTANTIATE_TEST_SUITE_P(
    MslGeneratorImplTest,
    MslBuiltinConversionTest,
    testing::Values(
        MslBuiltinData{builtin::BuiltinValue::kPosition, "position"},
        MslBuiltinData{builtin::BuiltinValue::kVertexIndex, "vertex_id"},
        MslBuiltinData{builtin::BuiltinValue::kInstanceIndex, "instance_id"},
        MslBuiltinData{builtin::BuiltinValue::kFrontFacing, "front_facing"},
        MslBuiltinData{builtin::BuiltinValue::kFragDepth, "depth(any)"},
        MslBuiltinData{builtin::BuiltinValue::kLocalInvocationId, "thread_position_in_threadgroup"},
        MslBuiltinData{builtin::BuiltinValue::kLocalInvocationIndex, "thread_index_in_threadgroup"},
        MslBuiltinData{builtin::BuiltinValue::kGlobalInvocationId, "thread_position_in_grid"},
        MslBuiltinData{builtin::BuiltinValue::kWorkgroupId, "threadgroup_position_in_grid"},
        MslBuiltinData{builtin::BuiltinValue::kNumWorkgroups, "threadgroups_per_grid"},
        MslBuiltinData{builtin::BuiltinValue::kSampleIndex, "sample_id"},
        MslBuiltinData{builtin::BuiltinValue::kSampleMask, "sample_mask"},
        MslBuiltinData{builtin::BuiltinValue::kPointSize, "point_size"}));

TEST_F(MslGeneratorImplTest, HasInvariantAttribute_True) {
    auto* out = Structure("Out", utils::Vector{
                                     Member("pos", ty.vec4<f32>(),
                                            utils::Vector{
                                                Builtin(builtin::BuiltinValue::kPosition),
                                                Invariant(),
                                            }),
                                 });
    Func("vert_main", utils::Empty, ty.Of(out), utils::Vector{Return(Call(ty.Of(out)))},
         utils::Vector{
             Stage(ast::PipelineStage::kVertex),
         });

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_TRUE(gen.HasInvariant());
    EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

using namespace metal;

#if __METAL_VERSION__ >= 210
#define TINT_INVARIANT [[invariant]]
#else
#define TINT_INVARIANT
#endif

struct Out {
  float4 pos [[position]] TINT_INVARIANT;
};

vertex Out vert_main() {
  return Out{};
}

)");
}

TEST_F(MslGeneratorImplTest, HasInvariantAttribute_False) {
    auto* out = Structure("Out", utils::Vector{
                                     Member("pos", ty.vec4<f32>(),
                                            utils::Vector{
                                                Builtin(builtin::BuiltinValue::kPosition),
                                            }),
                                 });
    Func("vert_main", utils::Empty, ty.Of(out), utils::Vector{Return(Call(ty.Of(out)))},
         utils::Vector{
             Stage(ast::PipelineStage::kVertex),
         });

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_FALSE(gen.HasInvariant());
    EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

using namespace metal;
struct Out {
  float4 pos [[position]];
};

vertex Out vert_main() {
  return Out{};
}

)");
}

TEST_F(MslGeneratorImplTest, WorkgroupMatrix) {
    GlobalVar("m", ty.mat2x2<f32>(), builtin::AddressSpace::kWorkgroup);
    Func("comp_main", utils::Empty, ty.void_(), utils::Vector{Decl(Let("x", Expr("m")))},
         utils::Vector{
             Stage(ast::PipelineStage::kCompute),
             WorkgroupSize(1_i),
         });

    GeneratorImpl& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

using namespace metal;
struct tint_symbol_3 {
  float2x2 m;
};

void comp_main_inner(uint local_invocation_index, threadgroup float2x2* const tint_symbol) {
  {
    *(tint_symbol) = float2x2(float2(0.0f), float2(0.0f));
  }
  threadgroup_barrier(mem_flags::mem_threadgroup);
  float2x2 const x = *(tint_symbol);
}

kernel void comp_main(threadgroup tint_symbol_3* tint_symbol_2 [[threadgroup(0)]], uint local_invocation_index [[thread_index_in_threadgroup]]) {
  threadgroup float2x2* const tint_symbol_1 = &((*(tint_symbol_2)).m);
  comp_main_inner(local_invocation_index, tint_symbol_1);
  return;
}

)");

    auto allocations = gen.DynamicWorkgroupAllocations();
    ASSERT_TRUE(allocations.count("comp_main"));
    ASSERT_EQ(allocations["comp_main"].size(), 1u);
    EXPECT_EQ(allocations["comp_main"][0], 2u * 2u * sizeof(float));
}

TEST_F(MslGeneratorImplTest, WorkgroupMatrixInArray) {
    GlobalVar("m", ty.array(ty.mat2x2<f32>(), 4_i), builtin::AddressSpace::kWorkgroup);
    Func("comp_main", utils::Empty, ty.void_(), utils::Vector{Decl(Let("x", Expr("m")))},
         utils::Vector{
             Stage(ast::PipelineStage::kCompute),
             WorkgroupSize(1_i),
         });

    GeneratorImpl& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

using namespace metal;

template<typename T, size_t N>
struct tint_array {
    const constant T& operator[](size_t i) const constant { return elements[i]; }
    device T& operator[](size_t i) device { return elements[i]; }
    const device T& operator[](size_t i) const device { return elements[i]; }
    thread T& operator[](size_t i) thread { return elements[i]; }
    const thread T& operator[](size_t i) const thread { return elements[i]; }
    threadgroup T& operator[](size_t i) threadgroup { return elements[i]; }
    const threadgroup T& operator[](size_t i) const threadgroup { return elements[i]; }
    T elements[N];
};

struct tint_symbol_3 {
  tint_array<float2x2, 4> m;
};

void comp_main_inner(uint local_invocation_index, threadgroup tint_array<float2x2, 4>* const tint_symbol) {
  for(uint idx = local_invocation_index; (idx < 4u); idx = (idx + 1u)) {
    uint const i = idx;
    (*(tint_symbol))[i] = float2x2(float2(0.0f), float2(0.0f));
  }
  threadgroup_barrier(mem_flags::mem_threadgroup);
  tint_array<float2x2, 4> const x = *(tint_symbol);
}

kernel void comp_main(threadgroup tint_symbol_3* tint_symbol_2 [[threadgroup(0)]], uint local_invocation_index [[thread_index_in_threadgroup]]) {
  threadgroup tint_array<float2x2, 4>* const tint_symbol_1 = &((*(tint_symbol_2)).m);
  comp_main_inner(local_invocation_index, tint_symbol_1);
  return;
}

)");

    auto allocations = gen.DynamicWorkgroupAllocations();
    ASSERT_TRUE(allocations.count("comp_main"));
    ASSERT_EQ(allocations["comp_main"].size(), 1u);
    EXPECT_EQ(allocations["comp_main"][0], 4u * 2u * 2u * sizeof(float));
}

TEST_F(MslGeneratorImplTest, WorkgroupMatrixInStruct) {
    Structure("S1", utils::Vector{
                        Member("m1", ty.mat2x2<f32>()),
                        Member("m2", ty.mat4x4<f32>()),
                    });
    Structure("S2", utils::Vector{
                        Member("s", ty("S1")),
                    });
    GlobalVar("s", ty("S2"), builtin::AddressSpace::kWorkgroup);
    Func("comp_main", utils::Empty, ty.void_(), utils::Vector{Decl(Let("x", Expr("s")))},
         utils::Vector{
             Stage(ast::PipelineStage::kCompute),
             WorkgroupSize(1_i),
         });

    GeneratorImpl& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

using namespace metal;
struct S1 {
  float2x2 m1;
  float4x4 m2;
};

struct S2 {
  S1 s;
};

struct tint_symbol_4 {
  S2 s;
};

void comp_main_inner(uint local_invocation_index, threadgroup S2* const tint_symbol_1) {
  {
    S2 const tint_symbol = S2{};
    *(tint_symbol_1) = tint_symbol;
  }
  threadgroup_barrier(mem_flags::mem_threadgroup);
  S2 const x = *(tint_symbol_1);
}

kernel void comp_main(threadgroup tint_symbol_4* tint_symbol_3 [[threadgroup(0)]], uint local_invocation_index [[thread_index_in_threadgroup]]) {
  threadgroup S2* const tint_symbol_2 = &((*(tint_symbol_3)).s);
  comp_main_inner(local_invocation_index, tint_symbol_2);
  return;
}

)");

    auto allocations = gen.DynamicWorkgroupAllocations();
    ASSERT_TRUE(allocations.count("comp_main"));
    ASSERT_EQ(allocations["comp_main"].size(), 1u);
    EXPECT_EQ(allocations["comp_main"][0], (2 * 2 * sizeof(float)) + (4u * 4u * sizeof(float)));
}

TEST_F(MslGeneratorImplTest, WorkgroupMatrix_Multiples) {
    GlobalVar("m1", ty.mat2x2<f32>(), builtin::AddressSpace::kWorkgroup);
    GlobalVar("m2", ty.mat2x3<f32>(), builtin::AddressSpace::kWorkgroup);
    GlobalVar("m3", ty.mat2x4<f32>(), builtin::AddressSpace::kWorkgroup);
    GlobalVar("m4", ty.mat3x2<f32>(), builtin::AddressSpace::kWorkgroup);
    GlobalVar("m5", ty.mat3x3<f32>(), builtin::AddressSpace::kWorkgroup);
    GlobalVar("m6", ty.mat3x4<f32>(), builtin::AddressSpace::kWorkgroup);
    GlobalVar("m7", ty.mat4x2<f32>(), builtin::AddressSpace::kWorkgroup);
    GlobalVar("m8", ty.mat4x3<f32>(), builtin::AddressSpace::kWorkgroup);
    GlobalVar("m9", ty.mat4x4<f32>(), builtin::AddressSpace::kWorkgroup);
    Func("main1", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(Let("a1", Expr("m1"))),
             Decl(Let("a2", Expr("m2"))),
             Decl(Let("a3", Expr("m3"))),
         },
         utils::Vector{
             Stage(ast::PipelineStage::kCompute),
             WorkgroupSize(1_i),
         });
    Func("main2", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(Let("a1", Expr("m4"))),
             Decl(Let("a2", Expr("m5"))),
             Decl(Let("a3", Expr("m6"))),
         },
         utils::Vector{
             Stage(ast::PipelineStage::kCompute),
             WorkgroupSize(1_i),
         });
    Func("main3", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(Let("a1", Expr("m7"))),
             Decl(Let("a2", Expr("m8"))),
             Decl(Let("a3", Expr("m9"))),
         },
         utils::Vector{
             Stage(ast::PipelineStage::kCompute),
             WorkgroupSize(1_i),
         });
    Func("main4_no_usages", utils::Empty, ty.void_(), utils::Empty,
         utils::Vector{
             Stage(ast::PipelineStage::kCompute),
             WorkgroupSize(1_i),
         });

    GeneratorImpl& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

using namespace metal;
struct tint_symbol_7 {
  float2x2 m1;
  float2x3 m2;
  float2x4 m3;
};

struct tint_symbol_15 {
  float3x2 m4;
  float3x3 m5;
  float3x4 m6;
};

struct tint_symbol_23 {
  float4x2 m7;
  float4x3 m8;
  float4x4 m9;
};

void main1_inner(uint local_invocation_index, threadgroup float2x2* const tint_symbol, threadgroup float2x3* const tint_symbol_1, threadgroup float2x4* const tint_symbol_2) {
  {
    *(tint_symbol) = float2x2(float2(0.0f), float2(0.0f));
    *(tint_symbol_1) = float2x3(float3(0.0f), float3(0.0f));
    *(tint_symbol_2) = float2x4(float4(0.0f), float4(0.0f));
  }
  threadgroup_barrier(mem_flags::mem_threadgroup);
  float2x2 const a1 = *(tint_symbol);
  float2x3 const a2 = *(tint_symbol_1);
  float2x4 const a3 = *(tint_symbol_2);
}

kernel void main1(threadgroup tint_symbol_7* tint_symbol_4 [[threadgroup(0)]], uint local_invocation_index [[thread_index_in_threadgroup]]) {
  threadgroup float2x2* const tint_symbol_3 = &((*(tint_symbol_4)).m1);
  threadgroup float2x3* const tint_symbol_5 = &((*(tint_symbol_4)).m2);
  threadgroup float2x4* const tint_symbol_6 = &((*(tint_symbol_4)).m3);
  main1_inner(local_invocation_index, tint_symbol_3, tint_symbol_5, tint_symbol_6);
  return;
}

void main2_inner(uint local_invocation_index_1, threadgroup float3x2* const tint_symbol_8, threadgroup float3x3* const tint_symbol_9, threadgroup float3x4* const tint_symbol_10) {
  {
    *(tint_symbol_8) = float3x2(float2(0.0f), float2(0.0f), float2(0.0f));
    *(tint_symbol_9) = float3x3(float3(0.0f), float3(0.0f), float3(0.0f));
    *(tint_symbol_10) = float3x4(float4(0.0f), float4(0.0f), float4(0.0f));
  }
  threadgroup_barrier(mem_flags::mem_threadgroup);
  float3x2 const a1 = *(tint_symbol_8);
  float3x3 const a2 = *(tint_symbol_9);
  float3x4 const a3 = *(tint_symbol_10);
}

kernel void main2(threadgroup tint_symbol_15* tint_symbol_12 [[threadgroup(0)]], uint local_invocation_index_1 [[thread_index_in_threadgroup]]) {
  threadgroup float3x2* const tint_symbol_11 = &((*(tint_symbol_12)).m4);
  threadgroup float3x3* const tint_symbol_13 = &((*(tint_symbol_12)).m5);
  threadgroup float3x4* const tint_symbol_14 = &((*(tint_symbol_12)).m6);
  main2_inner(local_invocation_index_1, tint_symbol_11, tint_symbol_13, tint_symbol_14);
  return;
}

void main3_inner(uint local_invocation_index_2, threadgroup float4x2* const tint_symbol_16, threadgroup float4x3* const tint_symbol_17, threadgroup float4x4* const tint_symbol_18) {
  {
    *(tint_symbol_16) = float4x2(float2(0.0f), float2(0.0f), float2(0.0f), float2(0.0f));
    *(tint_symbol_17) = float4x3(float3(0.0f), float3(0.0f), float3(0.0f), float3(0.0f));
    *(tint_symbol_18) = float4x4(float4(0.0f), float4(0.0f), float4(0.0f), float4(0.0f));
  }
  threadgroup_barrier(mem_flags::mem_threadgroup);
  float4x2 const a1 = *(tint_symbol_16);
  float4x3 const a2 = *(tint_symbol_17);
  float4x4 const a3 = *(tint_symbol_18);
}

kernel void main3(threadgroup tint_symbol_23* tint_symbol_20 [[threadgroup(0)]], uint local_invocation_index_2 [[thread_index_in_threadgroup]]) {
  threadgroup float4x2* const tint_symbol_19 = &((*(tint_symbol_20)).m7);
  threadgroup float4x3* const tint_symbol_21 = &((*(tint_symbol_20)).m8);
  threadgroup float4x4* const tint_symbol_22 = &((*(tint_symbol_20)).m9);
  main3_inner(local_invocation_index_2, tint_symbol_19, tint_symbol_21, tint_symbol_22);
  return;
}

kernel void main4_no_usages() {
  return;
}

)");

    auto allocations = gen.DynamicWorkgroupAllocations();
    ASSERT_TRUE(allocations.count("main1"));
    ASSERT_TRUE(allocations.count("main2"));
    ASSERT_TRUE(allocations.count("main3"));
    EXPECT_EQ(allocations.count("main4_no_usages"), 0u);
    ASSERT_EQ(allocations["main1"].size(), 1u);
    EXPECT_EQ(allocations["main1"][0], 20u * sizeof(float));
    ASSERT_EQ(allocations["main2"].size(), 1u);
    EXPECT_EQ(allocations["main2"][0], 32u * sizeof(float));
    ASSERT_EQ(allocations["main3"].size(), 1u);
    EXPECT_EQ(allocations["main3"][0], 40u * sizeof(float));
}

}  // namespace
}  // namespace tint::writer::msl
