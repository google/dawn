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

namespace tint::writer::msl {
namespace {

using MslGeneratorImplTest = TestHelper;

TEST_F(MslGeneratorImplTest, Generate) {
  Func("my_func", ast::VariableList{}, ty.void_(), ast::StatementList{},
       ast::AttributeList{
           Stage(ast::PipelineStage::kCompute),
           WorkgroupSize(1),
       });

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

using namespace metal;
kernel void my_func() {
  return;
}

)");
}

struct MslBuiltinData {
  ast::Builtin builtin;
  const char* attribute_name;
};
inline std::ostream& operator<<(std::ostream& out, MslBuiltinData data) {
  out << data.builtin;
  return out;
}
using MslBuiltinConversionTest = TestParamHelper<MslBuiltinData>;
TEST_P(MslBuiltinConversionTest, Emit) {
  auto params = GetParam();

  GeneratorImpl& gen = Build();

  EXPECT_EQ(gen.builtin_to_attribute(params.builtin),
            std::string(params.attribute_name));
}
INSTANTIATE_TEST_SUITE_P(
    MslGeneratorImplTest,
    MslBuiltinConversionTest,
    testing::Values(MslBuiltinData{ast::Builtin::kPosition, "position"},
                    MslBuiltinData{ast::Builtin::kVertexIndex, "vertex_id"},
                    MslBuiltinData{ast::Builtin::kInstanceIndex, "instance_id"},
                    MslBuiltinData{ast::Builtin::kFrontFacing, "front_facing"},
                    MslBuiltinData{ast::Builtin::kFragDepth, "depth(any)"},
                    MslBuiltinData{ast::Builtin::kLocalInvocationId,
                                   "thread_position_in_threadgroup"},
                    MslBuiltinData{ast::Builtin::kLocalInvocationIndex,
                                   "thread_index_in_threadgroup"},
                    MslBuiltinData{ast::Builtin::kGlobalInvocationId,
                                   "thread_position_in_grid"},
                    MslBuiltinData{ast::Builtin::kWorkgroupId,
                                   "threadgroup_position_in_grid"},
                    MslBuiltinData{ast::Builtin::kNumWorkgroups,
                                   "threadgroups_per_grid"},
                    MslBuiltinData{ast::Builtin::kSampleIndex, "sample_id"},
                    MslBuiltinData{ast::Builtin::kSampleMask, "sample_mask"},
                    MslBuiltinData{ast::Builtin::kPointSize, "point_size"}));

TEST_F(MslGeneratorImplTest, HasInvariantAttribute_True) {
  auto* out = Structure(
      "Out", {Member("pos", ty.vec4<f32>(),
                     {Builtin(ast::Builtin::kPosition), Invariant()})});
  Func("vert_main", ast::VariableList{}, ty.Of(out),
       {Return(Construct(ty.Of(out)))}, {Stage(ast::PipelineStage::kVertex)});

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_TRUE(gen.HasInvariant());
  EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

using namespace metal;

#if __METAL_VERSION__ >= 210
#define TINT_INVARIANT @invariant
#else
#define TINT_INVARIANT
#endif

struct Out {
  float4 pos [[position]] TINT_INVARIANT;
};

vertex Out vert_main() {
  return {};
}

)");
}

TEST_F(MslGeneratorImplTest, HasInvariantAttribute_False) {
  auto* out = Structure("Out", {Member("pos", ty.vec4<f32>(),
                                       {Builtin(ast::Builtin::kPosition)})});
  Func("vert_main", ast::VariableList{}, ty.Of(out),
       {Return(Construct(ty.Of(out)))}, {Stage(ast::PipelineStage::kVertex)});

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_FALSE(gen.HasInvariant());
  EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

using namespace metal;
struct Out {
  float4 pos [[position]];
};

vertex Out vert_main() {
  return {};
}

)");
}

TEST_F(MslGeneratorImplTest, WorkgroupMatrix) {
  Global("m", ty.mat2x2<f32>(), ast::StorageClass::kWorkgroup);
  Func("comp_main", ast::VariableList{}, ty.void_(),
       {Decl(Let("x", nullptr, Expr("m")))},
       {Stage(ast::PipelineStage::kCompute), WorkgroupSize(1)});

  GeneratorImpl& gen = SanitizeAndBuild();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

using namespace metal;
struct tint_symbol_3 {
  float2x2 m;
};

void comp_main_inner(uint local_invocation_index, threadgroup float2x2* const tint_symbol) {
  {
    *(tint_symbol) = float2x2();
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
  Global("m", ty.array(ty.mat2x2<f32>(), 4), ast::StorageClass::kWorkgroup);
  Func("comp_main", ast::VariableList{}, ty.void_(),
       {Decl(Let("x", nullptr, Expr("m")))},
       {Stage(ast::PipelineStage::kCompute), WorkgroupSize(1)});

  GeneratorImpl& gen = SanitizeAndBuild();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

using namespace metal;
struct tint_array_wrapper {
  float2x2 arr[4];
};

struct tint_symbol_3 {
  tint_array_wrapper m;
};

void comp_main_inner(uint local_invocation_index, threadgroup tint_array_wrapper* const tint_symbol) {
  for(uint idx = local_invocation_index; (idx < 4u); idx = (idx + 1u)) {
    uint const i = idx;
    (*(tint_symbol)).arr[i] = float2x2();
  }
  threadgroup_barrier(mem_flags::mem_threadgroup);
  tint_array_wrapper const x = *(tint_symbol);
}

kernel void comp_main(threadgroup tint_symbol_3* tint_symbol_2 [[threadgroup(0)]], uint local_invocation_index [[thread_index_in_threadgroup]]) {
  threadgroup tint_array_wrapper* const tint_symbol_1 = &((*(tint_symbol_2)).m);
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
  Structure("S1", {
                      Member("m1", ty.mat2x2<f32>()),
                      Member("m2", ty.mat4x4<f32>()),
                  });
  Structure("S2", {
                      Member("s", ty.type_name("S1")),
                  });
  Global("s", ty.type_name("S2"), ast::StorageClass::kWorkgroup);
  Func("comp_main", ast::VariableList{}, ty.void_(),
       {Decl(Let("x", nullptr, Expr("s")))},
       {Stage(ast::PipelineStage::kCompute), WorkgroupSize(1)});

  GeneratorImpl& gen = SanitizeAndBuild();

  ASSERT_TRUE(gen.Generate()) << gen.error();
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
    S2 const tint_symbol = {};
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
  EXPECT_EQ(allocations["comp_main"][0],
            (2 * 2 * sizeof(float)) + (4u * 4u * sizeof(float)));
}

TEST_F(MslGeneratorImplTest, WorkgroupMatrix_Multiples) {
  Global("m1", ty.mat2x2<f32>(), ast::StorageClass::kWorkgroup);
  Global("m2", ty.mat2x3<f32>(), ast::StorageClass::kWorkgroup);
  Global("m3", ty.mat2x4<f32>(), ast::StorageClass::kWorkgroup);
  Global("m4", ty.mat3x2<f32>(), ast::StorageClass::kWorkgroup);
  Global("m5", ty.mat3x3<f32>(), ast::StorageClass::kWorkgroup);
  Global("m6", ty.mat3x4<f32>(), ast::StorageClass::kWorkgroup);
  Global("m7", ty.mat4x2<f32>(), ast::StorageClass::kWorkgroup);
  Global("m8", ty.mat4x3<f32>(), ast::StorageClass::kWorkgroup);
  Global("m9", ty.mat4x4<f32>(), ast::StorageClass::kWorkgroup);
  Func("main1", ast::VariableList{}, ty.void_(),
       {
           Decl(Let("a1", nullptr, Expr("m1"))),
           Decl(Let("a2", nullptr, Expr("m2"))),
           Decl(Let("a3", nullptr, Expr("m3"))),
       },
       {Stage(ast::PipelineStage::kCompute), WorkgroupSize(1)});
  Func("main2", ast::VariableList{}, ty.void_(),
       {
           Decl(Let("a1", nullptr, Expr("m4"))),
           Decl(Let("a2", nullptr, Expr("m5"))),
           Decl(Let("a3", nullptr, Expr("m6"))),
       },
       {Stage(ast::PipelineStage::kCompute), WorkgroupSize(1)});
  Func("main3", ast::VariableList{}, ty.void_(),
       {
           Decl(Let("a1", nullptr, Expr("m7"))),
           Decl(Let("a2", nullptr, Expr("m8"))),
           Decl(Let("a3", nullptr, Expr("m9"))),
       },
       {Stage(ast::PipelineStage::kCompute), WorkgroupSize(1)});
  Func("main4_no_usages", ast::VariableList{}, ty.void_(), {},
       {Stage(ast::PipelineStage::kCompute), WorkgroupSize(1)});

  GeneratorImpl& gen = SanitizeAndBuild();

  ASSERT_TRUE(gen.Generate()) << gen.error();
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
    *(tint_symbol) = float2x2();
    *(tint_symbol_1) = float2x3();
    *(tint_symbol_2) = float2x4();
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
    *(tint_symbol_8) = float3x2();
    *(tint_symbol_9) = float3x3();
    *(tint_symbol_10) = float3x4();
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
    *(tint_symbol_16) = float4x2();
    *(tint_symbol_17) = float4x3();
    *(tint_symbol_18) = float4x4();
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
