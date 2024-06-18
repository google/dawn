// Copyright 2024 The Dawn & Tint Authors
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

#include <utility>

#include "gtest/gtest.h"
#include "src/tint/lang/core/access.h"
#include "src/tint/lang/core/builtin_value.h"
#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/number.h"
#include "src/tint/lang/core/type/struct.h"
#include "src/tint/lang/hlsl/writer/helper_test.h"
#include "src/tint/utils/containers/vector.h"

using namespace tint::core::number_suffixes;  // NOLINT
using namespace tint::core::fluent_types;     // NOLINT

namespace tint::hlsl::writer {
namespace {

TEST_F(HlslWriterTest, FunctionEmpty) {
    auto* func = b.Function("foo", ty.void_());
    func->Block()->Append(b.Return(func));

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
void foo() {
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

)");
}

TEST_F(HlslWriterTest, DISABLED_FunctionWithParams) {
    auto* func = b.Function("my_func", ty.void_());
    func->SetParams({b.FunctionParam("a", ty.f32()), b.FunctionParam("b", ty.i32())});
    func->Block()->Append(b.Return(func));

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(void my_func(float a, int b) {
  return;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

)");
}

TEST_F(HlslWriterTest, FunctionEntryPoint) {
    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    func->SetWorkgroupSize(1, 1, 1);
    func->Block()->Append(b.Return(func));

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
[numthreads(1, 1, 1)]
void main() {
}

)");
}

TEST_F(HlslWriterTest, DISABLED_FunctionPtrParameter) {
    // fn f(foo : ptr<function, f32>) -> f32 {
    //   return *foo;
    // }

    auto* foo = b.FunctionParam("foo", ty.ptr<function, f32>());
    auto* func = b.Function("f", ty.f32());
    func->SetParams({foo});
    b.Append(func->Block(), [&] { b.Return(func, b.Load(foo)); });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(float f(inout float foo) {
  return foo;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

)");
}

TEST_F(HlslWriterTest, DISABLED_FunctionEntryPointWithInAndOutLocations) {
    // fn frag_main(@location(0) foo : f32) -> @location(1) f32 {
    //   return foo;
    // }

    auto* foo = b.FunctionParam("foo", ty.f32());
    foo->SetLocation(0, {});

    auto* func = b.Function("frag_main", ty.f32(), core::ir::Function::PipelineStage::kFragment);
    func->SetReturnLocation(1, {});
    func->Block()->Append(b.Return(func, foo));

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(struct tint_symbol_1 {
  float foo : TEXCOORD0;
};
struct tint_symbol_2 {
  float value : SV_Target1;
};

float frag_main_inner(float foo) {
  return foo;
}

tint_symbol_2 frag_main(tint_symbol_1 tint_symbol) {
  float inner_result = frag_main_inner(tint_symbol.foo);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

)");
}

TEST_F(HlslWriterTest, DISABLED_FunctionEntryPointWithInOutBuiltins) {
    // fn frag_main(@position(0) coord : vec4<f32>) -> @frag_depth f32 {
    //   return coord.x;
    // }

    auto* coord = b.FunctionParam("coord", ty.vec4<f32>());
    coord->SetBuiltin(core::BuiltinValue::kPosition);

    auto* func = b.Function("frag_main", ty.f32());
    func->SetReturnBuiltin(core::BuiltinValue::kFragDepth);
    func->SetParams({coord});
    b.Append(func->Block(), [&] {  //
        auto* a = b.Access(ty.ptr(function, ty.f32()), coord, 0_u);
        b.Return(func, b.Load(a));
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(struct tint_symbol_1 {
  float4 coord : SV_Position;
};
struct tint_symbol_2 {
  float value : SV_Depth;
};

float frag_main_inner(float4 coord) {
  return coord.x;
}

tint_symbol_2 frag_main(tint_symbol_1 tint_symbol) {
  float inner_result = frag_main_inner(float4(tint_symbol.coord.xyz, (1.0f / tint_symbol.coord.w)));
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

)");
}

TEST_F(HlslWriterTest, DISABLED_FunctiontEntryPointSharedStructDifferentStages) {
    // struct Interface {
    //   @builtin(position) pos : vec4<f32>;
    //   @location(1) col1 : f32;
    //   @location(2) col2 : f32;
    // };
    //
    // fn vert_main() -> Interface {
    //   return Interface(vec4<f32>(), 0.4, 0.6);
    // }
    //
    // fn frag_main(inputs : Interface) {
    //   const r = inputs.col1;
    //   const g = inputs.col2;
    //   const p = inputs.pos;
    // }

    core::type::StructMemberAttributes pos_attrs{};
    pos_attrs.builtin = core::BuiltinValue::kPosition;
    core::type::StructMemberAttributes col1_attrs{};
    col1_attrs.location = 1;
    core::type::StructMemberAttributes col2_attrs{};
    col2_attrs.location = 2;

    Vector members{
        ty.Get<core::type::StructMember>(b.ir.symbols.New("pos"), ty.vec4<f32>(), 0u, 0u, 16u, 16u,
                                         pos_attrs),
        ty.Get<core::type::StructMember>(b.ir.symbols.New("col1"), ty.f32(), 1u, 16u, 4u, 4u,
                                         col1_attrs),
        ty.Get<core::type::StructMember>(b.ir.symbols.New("col2"), ty.f32(), 2u, 16u, 4u, 4u,
                                         col2_attrs),
    };
    auto* strct = ty.Struct(b.ir.symbols.New("Interface"), std::move(members));

    auto* vert_func = b.Function("vert_main", strct, core::ir::Function::PipelineStage::kVertex);
    b.Append(vert_func->Block(), [&] {  //
        b.Return(vert_func, b.Construct(strct, b.Construct(ty.vec4<f32>()), 0.5_f, 0.25_f));
    });

    auto* frag_param = b.FunctionParam("inputs", strct);
    auto* frag_func =
        b.Function("frag_main", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    frag_func->SetParams({frag_param});
    b.Append(frag_func->Block(), [&] {
        auto* r = b.Access(ty.ptr(function, ty.f32()), frag_param, 1_u);
        auto* g = b.Access(ty.ptr(function, ty.f32()), frag_param, 2_u);
        auto* p = b.Access(ty.ptr(function, ty.vec4<f32>()), frag_param, 0_u);

        b.Let("r", b.Load(r));
        b.Let("g", b.Load(g));
        b.Let("p", b.Load(p));
        b.Return(frag_func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(struct Interface {
  float4 pos;
  float col1;
  float col2;
};
struct tint_symbol {
  float col1 : TEXCOORD1;
  float col2 : TEXCOORD2;
  float4 pos : SV_Position;
};

Interface vert_main_inner() {
  Interface tint_symbol_3 = {(0.0f).xxxx, 0.5f, 0.25f};
  return tint_symbol_3;
}

tint_symbol vert_main() {
  Interface inner_result = vert_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.pos = inner_result.pos;
  wrapper_result.col1 = inner_result.col1;
  wrapper_result.col2 = inner_result.col2;
  return wrapper_result;
}

struct tint_symbol_2 {
  float col1 : TEXCOORD1;
  float col2 : TEXCOORD2;
  float4 pos : SV_Position;
};

void frag_main_inner(Interface inputs) {
  float r = inputs.col1;
  float g = inputs.col2;
  float4 p = inputs.pos;
}

void frag_main(tint_symbol_2 tint_symbol_1) {
  Interface tint_symbol_4 = {float4(tint_symbol_1.pos.xyz, (1.0f / tint_symbol_1.pos.w)), tint_symbol_1.col1, tint_symbol_1.col2};
  frag_main_inner(tint_symbol_4);
  return;
}

)");
}

TEST_F(HlslWriterTest, DISABLED_FunctionEntryPointSharedStructHelperFunction) {
    // struct VertexOutput {
    //   @builtin(position) pos : vec4<f32>;
    // };
    // fn foo(x : f32) -> VertexOutput {
    //   return VertexOutput(vec4<f32>(x, x, x, 1.0));
    // }
    // fn vert_main1() -> VertexOutput {
    //   return foo(0.5);
    // }
    // fn vert_main2() -> VertexOutput {
    //   return foo(0.25);
    // }

    core::type::StructMemberAttributes pos_attrs{};
    pos_attrs.builtin = core::BuiltinValue::kPosition;

    Vector members{ty.Get<core::type::StructMember>(b.ir.symbols.New("pos"), ty.vec4<f32>(), 0u, 0u,
                                                    16u, 16u, pos_attrs)};
    auto* strct = ty.Struct(b.ir.symbols.New("VertexOutput"), std::move(members));

    auto* x = b.FunctionParam("x", ty.f32());
    auto* foo_func = b.Function("foo", strct);
    foo_func->SetParams({x});
    b.Append(foo_func->Block(), [&] {  //
        b.Return(foo_func, b.Construct(strct, b.Construct(ty.vec4<f32>(), x, x, x, 1_f)));
    });

    {
        auto* vert1_func =
            b.Function("vert1_main1", strct, core::ir::Function::PipelineStage::kVertex);
        b.Append(vert1_func->Block(), [&] {  //
            b.Return(vert1_func, b.Call(foo_func, 0.5_f));
        });
    }

    {
        auto* vert2_func =
            b.Function("vert2_main1", strct, core::ir::Function::PipelineStage::kVertex);
        b.Append(vert2_func->Block(), [&] {  //
            b.Return(vert2_func, b.Call(foo_func, 0.25_f));
        });
    }

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(struct VertexOutput {
  float4 pos;
};

VertexOutput foo(float x) {
  VertexOutput tint_symbol_2 = {float4(x, x, x, 1.0f)};
  return tint_symbol_2;
}

struct tint_symbol {
  float4 pos : SV_Position;
};

VertexOutput vert_main1_inner() {
  return foo(0.5f);
}

tint_symbol vert_main1() {
  VertexOutput inner_result = vert_main1_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.pos = inner_result.pos;
  return wrapper_result;
}

struct tint_symbol_1 {
  float4 pos : SV_Position;
};

VertexOutput vert_main2_inner() {
  return foo(0.25f);
}

tint_symbol_1 vert_main2() {
  VertexOutput inner_result_1 = vert_main2_inner();
  tint_symbol_1 wrapper_result_1 = (tint_symbol_1)0;
  wrapper_result_1.pos = inner_result_1.pos;
  return wrapper_result_1;
}

)");
}

TEST_F(HlslWriterTest, DISABLED_FunctionEntryPointWithUniform) {
    // struct Uniforms {
    //   coord: vec4f,
    // }
    // @group(1) @binding(0) var<uniform> ubo : Uniforms;
    //
    // fn sub_func(param: f32) -> f32 {
    //   return ubo.coord.x;
    // }
    // @fragment fn frag_main() {
    //   var v = sub_func(1f);
    // }

    Vector members{ty.Get<core::type::StructMember>(b.ir.symbols.New("coord"), ty.vec4<f32>(), 0u,
                                                    0u, 16u, 16u,
                                                    core::type::StructMemberAttributes{})};
    auto* strct = ty.Struct(b.ir.symbols.New("Uniforms"), std::move(members));

    auto* ubo = b.Var("ubo", uniform, strct);
    ubo->SetBindingPoint(1, 0);
    b.ir.root_block->Append(ubo);

    auto* param = b.FunctionParam("param", ty.f32());
    auto* sub_func = b.Function("sub_func", ty.f32());
    sub_func->SetParams({param});

    b.Append(sub_func->Block(), [&] {  //
        auto* a = b.Access(ty.ptr<uniform, f32>(), ubo, 0_u, 0_u);
        b.Return(sub_func, b.Load(a));
    });

    auto* frag_func =
        b.Function("frag_main", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(frag_func->Block(), [&] {
        b.Var("v", b.Call(sub_func, 1_f));
        b.Return(frag_func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(cbuffer cbuffer_uniforms : register(b0, space1) {
  uint4 ubo[1];
};

float sub_func(float param) {
  return asfloat(ubo[0].x);
}

void frag_main() {
  float v = sub_func(1.0f);
  return;
}

)");
}

TEST_F(HlslWriterTest, DISABLED_FunctionEntryPointWithUniformStruct) {
    // struct Uniforms {
    //   coord: vec4f,
    // }
    //
    // @group(1) @binding(0) var<uniform> ubo: Uniforms;
    //
    // @fragment fn frag_main() {
    //   var v = ubo.coord.x;
    // }

    Vector members{ty.Get<core::type::StructMember>(b.ir.symbols.New("coord"), ty.vec4<f32>(), 0u,
                                                    0u, 16u, 16u,
                                                    core::type::StructMemberAttributes{})};
    auto* strct = ty.Struct(b.ir.symbols.New("Uniforms"), std::move(members));

    auto* ubo = b.Var("ubo", uniform, strct);
    ubo->SetBindingPoint(1, 0);
    b.ir.root_block->Append(ubo);

    auto* func = b.Function("frag_main", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {  //
        auto* a = b.Access(ty.ptr<uniform, f32>(), ubo, 0_u, 0_u);
        b.Var("v", b.Load(a));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(cbuffer cbuffer_uniforms : register(b0, space1) {
  uint4 ubo[1];
};

void frag_main() {
  float v = asfloat(ubo[0].x);
  return;
}

)");
}

TEST_F(HlslWriterTest, DISABLED_FunctionEntryPointWithRWStorageBufferRead) {
    // struct Data {
    //   a: i32,
    //   b: f32,
    // }
    // @group(1) @binding(0) var<storage, read_write> coord: Data;
    //
    // @fragment fn frag_main() {
    //   var v = coord.b;
    // }

    Vector members{ty.Get<core::type::StructMember>(b.ir.symbols.New("a"), ty.i32(), 0u, 0u, 4u, 4u,
                                                    core::type::StructMemberAttributes{}),
                   ty.Get<core::type::StructMember>(b.ir.symbols.New("b"), ty.f32(), 1u, 4u, 4u, 4u,
                                                    core::type::StructMemberAttributes{})};
    auto* strct = ty.Struct(b.ir.symbols.New("Data"), std::move(members));

    auto* coord = b.Var("coord", storage, strct, read_write);
    coord->SetBindingPoint(1, 0);
    b.ir.root_block->Append(coord);

    auto* func = b.Function("frag_main", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {  //
        auto* a = b.Access(ty.ptr(storage, ty.f32()), coord, 0_u);
        b.Var("v", b.Load(a));

        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl,
              R"(RWByteAddressBuffer coord : register(u0, space1);

void frag_main() {
  float v = asfloat(coord.Load(4u));
  return;
}

)");
}

TEST_F(HlslWriterTest, DISABLED_FunctionEntryPointWithROStorageBufferRead) {
    // struct Data {
    //   a: i32,
    //   b: f32,
    // }
    // @group(1) @binding(0) var<storage, read> coord: Data;
    //
    // @fragment fn frag_main() {
    //   var v = coord.b;
    // }

    Vector members{ty.Get<core::type::StructMember>(b.ir.symbols.New("a"), ty.i32(), 0u, 0u, 4u, 4u,
                                                    core::type::StructMemberAttributes{}),
                   ty.Get<core::type::StructMember>(b.ir.symbols.New("b"), ty.f32(), 1u, 4u, 4u, 4u,
                                                    core::type::StructMemberAttributes{})};
    auto* strct = ty.Struct(b.ir.symbols.New("Data"), std::move(members));

    auto* coord = b.Var("coord", storage, strct, core::Access::kRead);
    coord->SetBindingPoint(1, 0);
    b.ir.root_block->Append(coord);

    auto* func = b.Function("frag_main", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {  //
        auto* a = b.Access(ty.ptr<storage, f32>(), coord, 0_u);
        b.Var("v", b.Load(a));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl,
              R"(ByteAddressBuffer coord : register(t0, space1);

void frag_main() {
  float v = asfloat(coord.Load(4u));
  return;
}

)");
}

TEST_F(HlslWriterTest, DISABLED_FunctionEntryPointWithWOStorageBufferStore) {
    // struct Data {
    //   a: i32,
    //   b: f32,
    // }
    // @group(1) @binding(0) var<storage, write> coord: Data;
    //
    // @fragment fn frag_main() {
    //   coord.b = 2f;
    // }

    Vector members{ty.Get<core::type::StructMember>(b.ir.symbols.New("a"), ty.i32(), 0u, 0u, 4u, 4u,
                                                    core::type::StructMemberAttributes{}),
                   ty.Get<core::type::StructMember>(b.ir.symbols.New("b"), ty.f32(), 1u, 4u, 4u, 4u,
                                                    core::type::StructMemberAttributes{})};
    auto* strct = ty.Struct(b.ir.symbols.New("Data"), std::move(members));

    auto* coord = b.Var("coord", storage, strct, core::Access::kWrite);
    coord->SetBindingPoint(1, 0);
    b.ir.root_block->Append(coord);

    auto* func = b.Function("frag_main", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {  //
        b.Store(b.Access(ty.ptr(storage, ty.f32()), coord, 1_u), 2_f);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl,
              R"(RWByteAddressBuffer coord : register(u0, space1);

void frag_main() {
  coord.Store(4u, asuint(2.0f));
  return;
}

)");
}

TEST_F(HlslWriterTest, DISABLED_FunctionEntryPointWithStorageBufferStore) {
    // struct Data {
    //   a: i32,
    //   b: f32,
    // }
    // @group(1) @binding(0) var<storage, write> coord: Data;
    //
    // @fragment fn frag_main() {
    //   coord.b = 2f;
    // }

    Vector members{ty.Get<core::type::StructMember>(b.ir.symbols.New("a"), ty.i32(), 0u, 0u, 4u, 4u,
                                                    core::type::StructMemberAttributes{}),
                   ty.Get<core::type::StructMember>(b.ir.symbols.New("b"), ty.f32(), 1u, 4u, 4u, 4u,
                                                    core::type::StructMemberAttributes{})};
    auto* strct = ty.Struct(b.ir.symbols.New("Data"), std::move(members));

    auto* coord = b.Var("coord", storage, strct, read_write);
    coord->SetBindingPoint(1, 0);
    b.ir.root_block->Append(coord);

    auto* func = b.Function("frag_main", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {  //
        b.Store(b.Access(ty.ptr(storage, ty.f32()), coord, 1_u), 2_f);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl,
              R"(RWByteAddressBuffer coord : register(u0, space1);

void frag_main() {
  coord.Store(4u, asuint(2.0f));
  return;
}

)");
}

TEST_F(HlslWriterTest, DISABLED_FunctionCalledByEntryPointWithUniform) {
    // Struct S {
    //   x: f32,
    // }
    // @group(1) @binding(0) var<uniform> coord: S;
    //
    // fn sub_func() -> f32 {
    //   return coord.x;
    // }
    // @fragment fn frag_main() {
    //   var v = sub_func(1f);
    // }

    Vector members{ty.Get<core::type::StructMember>(b.ir.symbols.New("x"), ty.f32(), 0u, 0u, 4u, 4u,
                                                    core::type::StructMemberAttributes{})};
    auto* strct = ty.Struct(b.ir.symbols.New("S"), std::move(members));

    auto* coord = b.Var("coord", uniform, strct);
    coord->SetBindingPoint(1, 0);
    b.ir.root_block->Append(coord);

    auto* sub_func = b.Function("sub_func", ty.f32());
    b.Append(sub_func->Block(), [&] {
        auto* a = b.Access(ty.ptr<storage, f32>(), coord, 0_u);
        b.Return(sub_func, b.Load(a));
    });

    auto* func = b.Function("frag_main", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {  //
        b.Var("v", b.Call(sub_func, 1_f));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(cbuffer cbuffer_coord : register(b0, space1) {
  uint4 coord[1];
};

float sub_func(float param) {
  return coord.x;
}

void frag_main() {
  float v = sub_func(1.0f);
  return;
}

)");
}

TEST_F(HlslWriterTest, DISABLED_FunctionCalledByEntryPointWithStorageBuffer) {
    // Struct S {
    //   x: f32,
    // }
    // @group(1) @binding(0) var<storage, read_write> coord: S;
    //
    // fn sub_func() -> f32 {
    //   return coord.x;
    // }
    // @fragment fn frag_main() {
    //   var v = sub_func(1f);
    // }

    Vector members{ty.Get<core::type::StructMember>(b.ir.symbols.New("x"), ty.f32(), 0u, 0u, 4u, 4u,
                                                    core::type::StructMemberAttributes{})};
    auto* strct = ty.Struct(b.ir.symbols.New("S"), std::move(members));

    auto* coord = b.Var("coord", storage, strct, core::Access::kReadWrite);
    coord->SetBindingPoint(1, 0);
    b.ir.root_block->Append(coord);

    auto* sub_func = b.Function("sub_func", ty.f32());
    b.Append(sub_func->Block(), [&] {
        auto* a = b.Access(ty.ptr<storage, f32>(), coord, 0_u);
        b.Return(sub_func, b.Load(a));
    });

    auto* func = b.Function("frag_main", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {  //
        b.Var("v", b.Call(sub_func, 1_f));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl,
              R"(RWByteAddressBuffer coord : register(u0, space1);

float sub_func(float param) {
  return asfloat(coord.Load(0u));
}

void frag_main() {
  float v = sub_func(1.0f);
  return;
}

)");
}

TEST_F(HlslWriterTest, FunctionEntryPointCompute) {
    // @compute @workgroup_size(1) fn main() {}

    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    func->SetWorkgroupSize(1, 1, 1);
    func->Block()->Append(b.Return(func));

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
[numthreads(1, 1, 1)]
void main() {
}

)");
}

TEST_F(HlslWriterTest, FunctionEntryPointComputeWithWorkgroupLiteral) {
    // @compute @workgroup_size(2, 4, 6) fn main() {}

    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    func->SetWorkgroupSize(2, 4, 6);
    func->Block()->Append(b.Return(func));

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
[numthreads(2, 4, 6)]
void main() {
}

)");
}

TEST_F(HlslWriterTest, DISABLED_FunctionWithArrayParams) {
    // fn my_func(a: array<f32, 5>) {}

    auto* func = b.Function("my_func", ty.void_());
    auto* p = b.FunctionParam("a", ty.array<f32, 5>());
    func->SetParams({p});
    func->Block()->Append(b.Return(func));

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(void my_func(float a[5]) {
  return;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

)");
}

TEST_F(HlslWriterTest, DISABLED_FunctionWithArrayReturn) {
    // fn my_func() -> array<f32, 5> {
    //   return array<f32, 5>();
    // }

    auto* func = b.Function("my_func", ty.array<f32, 5>());
    func->Block()->Append(b.Return(func, b.Construct(ty.array<f32, 5>())));

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(typedef float my_func_ret[5];
my_func_ret my_func() {
  return (float[5])0;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

)");
}

TEST_F(HlslWriterTest, DISABLED_FunctionWithDiscardAndVoidReturn) {
    // fn my_func(a: i32) {
    //   if (a == 0) {
    //     discard;
    //   }
    // }

    auto* func = b.Function("my_func", ty.void_());
    auto* p = b.FunctionParam("a", ty.i32());
    func->SetParams({p});

    b.Append(func->Block(), [&] {
        auto* i = b.If(b.Equal(ty.bool_(), p, 0_i));
        b.Append(i->True(), [&] { b.Discard(); });
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(void my_func(int a) {
  if ((a == 0)) {
    discard;
  }
  return;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

)");
}

TEST_F(HlslWriterTest, DISABLED_FunctionWithDiscardAndNonVoidReturn) {
    // fn my_func(a: i32) -> i32 {
    //   if (a == 0) {
    //     discard;
    //   }
    //   return 42;
    // }

    auto* func = b.Function("my_func", ty.i32());
    auto* p = b.FunctionParam("a", ty.i32());
    func->SetParams({p});

    b.Append(func->Block(), [&] {
        auto* i = b.If(b.Equal(ty.bool_(), p, 0_i));
        b.Append(i->True(), [&] { b.Discard(); });
        b.Return(func, 42_i);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(int my_func(int a) {
  if (true) {
    if ((a == 0)) {
      discard;
    }
    return 42;
  }
  int unused;
  return unused;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

)");
}

// https://crbug.com/tint/297
TEST_F(HlslWriterTest, DISABLED_FunctionMultipleEntryPointWithSameModuleVar) {
    // struct Data {
    //   d : f32;
    // };
    // @binding(0) @group(0) var<storage, read_write> data : Data;
    //
    // @compute @workgroup_size(1)
    // fn a() {
    //   var v = data.d;
    //   return;
    // }
    //
    // @compute @workgroup_size(1)
    // fn b() {
    //   var v = data.d;
    //   return;
    // }

    Vector members{ty.Get<core::type::StructMember>(b.ir.symbols.New("d"), ty.f32(), 0u, 0u, 4u, 4u,
                                                    core::type::StructMemberAttributes{})};
    auto* strct = ty.Struct(b.ir.symbols.New("Data"), std::move(members));

    auto* data = b.Var("data", storage, strct, read_write);
    data->SetBindingPoint(0, 0);
    b.ir.root_block->Append(data);

    {
        auto* func = b.Function("a", ty.void_(), core::ir::Function::PipelineStage::kCompute);
        func->SetWorkgroupSize(1, 1, 1);
        b.Append(func->Block(), [&] {  //
            auto* a = b.Access(ty.ptr<storage, f32>(), data, 0_u);
            b.Var("v", b.Load(a));
            b.Return(func);
        });
    }

    {
        auto* func = b.Function("b", ty.void_(), core::ir::Function::PipelineStage::kCompute);
        func->SetWorkgroupSize(1, 1, 1);
        b.Append(func->Block(), [&] {  //
            auto* a = b.Access(ty.ptr<storage, f32>(), data, 0_u);
            b.Var("v", b.Load(a));
            b.Return(func);
        });
    }

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(RWByteAddressBuffer data : register(u0);

[numthreads(1, 1, 1)]
void a() {
  float v = asfloat(data.Load(0u));
  return;
}

[numthreads(1, 1, 1)]
void b() {
  float v = asfloat(data.Load(0u));
  return;
}
)");
}

}  // namespace
}  // namespace tint::hlsl::writer
