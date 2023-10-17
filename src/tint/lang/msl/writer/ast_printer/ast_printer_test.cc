// Copyright 2020 The Dawn & Tint Authors
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

#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/msl/writer/ast_printer/helper_test.h"
#include "src/tint/lang/msl/writer/writer.h"
#include "src/tint/lang/wgsl/ast/stage_attribute.h"

using namespace tint::core::number_suffixes;  // NOLINT
using namespace tint::core::fluent_types;     // NOLINT

namespace tint::msl::writer {
namespace {

using MslASTPrinterTest = TestHelper;

TEST_F(MslASTPrinterTest, InvalidProgram) {
    Diagnostics().add_error(diag::System::Writer, "make the program invalid");
    ASSERT_FALSE(IsValid());
    auto program = resolver::Resolve(*this);
    ASSERT_FALSE(program.IsValid());
    auto result = Generate(program, Options{});
    EXPECT_FALSE(result);
    EXPECT_EQ(result.Failure().reason.str(), "error: make the program invalid");
}

TEST_F(MslASTPrinterTest, UnsupportedExtension) {
    Enable(Source{{12, 34}}, wgsl::Extension::kUndefined);

    ASTPrinter& gen = Build();

    ASSERT_FALSE(gen.Generate());
    EXPECT_EQ(gen.Diagnostics().str(),
              R"(12:34 error: MSL backend does not support extension 'undefined')");
}

TEST_F(MslASTPrinterTest, Generate) {
    Func("my_func", tint::Empty, ty.void_(), tint::Empty,
         Vector{
             Stage(ast::PipelineStage::kCompute),
             WorkgroupSize(1_i),
         });

    ASTPrinter& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_EQ(gen.Result(), R"(#include <metal_stdlib>

using namespace metal;
kernel void my_func() {
  return;
}

)");
}

TEST_F(MslASTPrinterTest, HasInvariantAttribute_True) {
    auto* out = Structure("Out", Vector{
                                     Member("pos", ty.vec4<f32>(),
                                            Vector{
                                                Builtin(core::BuiltinValue::kPosition),
                                                Invariant(),
                                            }),
                                 });
    Func("vert_main", tint::Empty, ty.Of(out), Vector{Return(Call(ty.Of(out)))},
         Vector{
             Stage(ast::PipelineStage::kVertex),
         });

    ASTPrinter& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_TRUE(gen.HasInvariant());
    EXPECT_EQ(gen.Result(), R"(#include <metal_stdlib>

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

TEST_F(MslASTPrinterTest, HasInvariantAttribute_False) {
    auto* out = Structure("Out", Vector{
                                     Member("pos", ty.vec4<f32>(),
                                            Vector{
                                                Builtin(core::BuiltinValue::kPosition),
                                            }),
                                 });
    Func("vert_main", tint::Empty, ty.Of(out), Vector{Return(Call(ty.Of(out)))},
         Vector{
             Stage(ast::PipelineStage::kVertex),
         });

    ASTPrinter& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_FALSE(gen.HasInvariant());
    EXPECT_EQ(gen.Result(), R"(#include <metal_stdlib>

using namespace metal;
struct Out {
  float4 pos [[position]];
};

vertex Out vert_main() {
  return Out{};
}

)");
}

TEST_F(MslASTPrinterTest, WorkgroupMatrix) {
    GlobalVar("m", ty.mat2x2<f32>(), core::AddressSpace::kWorkgroup);
    Func("comp_main", tint::Empty, ty.void_(), Vector{Decl(Let("x", Expr("m")))},
         Vector{
             Stage(ast::PipelineStage::kCompute),
             WorkgroupSize(1_i),
         });

    ASTPrinter& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_EQ(gen.Result(), R"(#include <metal_stdlib>

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
    ASSERT_EQ(allocations.at("comp_main").size(), 1u);
    EXPECT_EQ(allocations.at("comp_main")[0], 2u * 2u * sizeof(float));
}

TEST_F(MslASTPrinterTest, WorkgroupMatrixInArray) {
    GlobalVar("m", ty.array(ty.mat2x2<f32>(), 4_i), core::AddressSpace::kWorkgroup);
    Func("comp_main", tint::Empty, ty.void_(), Vector{Decl(Let("x", Expr("m")))},
         Vector{
             Stage(ast::PipelineStage::kCompute),
             WorkgroupSize(1_i),
         });

    ASTPrinter& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_EQ(gen.Result(), R"(#include <metal_stdlib>

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
    ASSERT_EQ(allocations.at("comp_main").size(), 1u);
    EXPECT_EQ(allocations.at("comp_main")[0], 4u * 2u * 2u * sizeof(float));
}

TEST_F(MslASTPrinterTest, WorkgroupMatrixInStruct) {
    Structure("S1", Vector{
                        Member("m1", ty.mat2x2<f32>()),
                        Member("m2", ty.mat4x4<f32>()),
                    });
    Structure("S2", Vector{
                        Member("s", ty("S1")),
                    });
    GlobalVar("s", ty("S2"), core::AddressSpace::kWorkgroup);
    Func("comp_main", tint::Empty, ty.void_(), Vector{Decl(Let("x", Expr("s")))},
         Vector{
             Stage(ast::PipelineStage::kCompute),
             WorkgroupSize(1_i),
         });

    ASTPrinter& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_EQ(gen.Result(), R"(#include <metal_stdlib>

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
    ASSERT_EQ(allocations.at("comp_main").size(), 1u);
    EXPECT_EQ(allocations.at("comp_main")[0], (2 * 2 * sizeof(float)) + (4u * 4u * sizeof(float)));
}

TEST_F(MslASTPrinterTest, WorkgroupMatrix_Multiples) {
    GlobalVar("m1", ty.mat2x2<f32>(), core::AddressSpace::kWorkgroup);
    GlobalVar("m2", ty.mat2x3<f32>(), core::AddressSpace::kWorkgroup);
    GlobalVar("m3", ty.mat2x4<f32>(), core::AddressSpace::kWorkgroup);
    GlobalVar("m4", ty.mat3x2<f32>(), core::AddressSpace::kWorkgroup);
    GlobalVar("m5", ty.mat3x3<f32>(), core::AddressSpace::kWorkgroup);
    GlobalVar("m6", ty.mat3x4<f32>(), core::AddressSpace::kWorkgroup);
    GlobalVar("m7", ty.mat4x2<f32>(), core::AddressSpace::kWorkgroup);
    GlobalVar("m8", ty.mat4x3<f32>(), core::AddressSpace::kWorkgroup);
    GlobalVar("m9", ty.mat4x4<f32>(), core::AddressSpace::kWorkgroup);
    Func("main1", tint::Empty, ty.void_(),
         Vector{
             Decl(Let("a1", Expr("m1"))),
             Decl(Let("a2", Expr("m2"))),
             Decl(Let("a3", Expr("m3"))),
         },
         Vector{
             Stage(ast::PipelineStage::kCompute),
             WorkgroupSize(1_i),
         });
    Func("main2", tint::Empty, ty.void_(),
         Vector{
             Decl(Let("a1", Expr("m4"))),
             Decl(Let("a2", Expr("m5"))),
             Decl(Let("a3", Expr("m6"))),
         },
         Vector{
             Stage(ast::PipelineStage::kCompute),
             WorkgroupSize(1_i),
         });
    Func("main3", tint::Empty, ty.void_(),
         Vector{
             Decl(Let("a1", Expr("m7"))),
             Decl(Let("a2", Expr("m8"))),
             Decl(Let("a3", Expr("m9"))),
         },
         Vector{
             Stage(ast::PipelineStage::kCompute),
             WorkgroupSize(1_i),
         });
    Func("main4_no_usages", tint::Empty, ty.void_(), tint::Empty,
         Vector{
             Stage(ast::PipelineStage::kCompute),
             WorkgroupSize(1_i),
         });

    ASTPrinter& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_EQ(gen.Result(), R"(#include <metal_stdlib>

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
    ASSERT_EQ(allocations.at("main1").size(), 1u);
    EXPECT_EQ(allocations.at("main1")[0], 20u * sizeof(float));
    ASSERT_EQ(allocations.at("main2").size(), 1u);
    EXPECT_EQ(allocations.at("main2")[0], 32u * sizeof(float));
    ASSERT_EQ(allocations.at("main3").size(), 1u);
    EXPECT_EQ(allocations.at("main3")[0], 40u * sizeof(float));
    EXPECT_EQ(allocations.at("main4_no_usages").size(), 0u);
}

}  // namespace
}  // namespace tint::msl::writer
