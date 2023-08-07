// Copyright 2021 The Tint Authors.
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

#include "src/tint/lang/hlsl/writer/ast_printer/helper_test.h"
#include "src/tint/lang/wgsl/ast/call_statement.h"
#include "src/tint/lang/wgsl/ast/stage_attribute.h"
#include "src/tint/lang/wgsl/ast/variable_decl_statement.h"

namespace tint::hlsl::writer {
namespace {

using namespace tint::core::fluent_types;  // NOLINT
using namespace tint::number_suffixes;     // NOLINT

using HlslSanitizerTest = TestHelper;

TEST_F(HlslSanitizerTest, Call_ArrayLength) {
    auto* s = Structure("my_struct", Vector{Member(0, "a", ty.array<f32>())});
    GlobalVar("b", ty.Of(s), core::AddressSpace::kStorage, core::Access::kRead, Binding(1_a),
              Group(2_a));

    Func("a_func", tint::Empty, ty.void_(),
         Vector{
             Decl(Var("len", ty.u32(), Call("arrayLength", AddressOf(MemberAccessor("b", "a"))))),
         },
         Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    ASTPrinter& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();

    auto got = gen.Result();
    auto* expect = R"(ByteAddressBuffer b : register(t1, space2);

void a_func() {
  uint tint_symbol_1 = 0u;
  b.GetDimensions(tint_symbol_1);
  const uint tint_symbol_2 = ((tint_symbol_1 - 0u) / 4u);
  uint len = tint_symbol_2;
  return;
}
)";
    EXPECT_EQ(expect, got);
}

TEST_F(HlslSanitizerTest, Call_ArrayLength_OtherMembersInStruct) {
    auto* s = Structure("my_struct", Vector{
                                         Member(0, "z", ty.f32()),
                                         Member(4, "a", ty.array<f32>()),
                                     });
    GlobalVar("b", ty.Of(s), core::AddressSpace::kStorage, core::Access::kRead, Binding(1_a),
              Group(2_a));

    Func("a_func", tint::Empty, ty.void_(),
         Vector{
             Decl(Var("len", ty.u32(), Call("arrayLength", AddressOf(MemberAccessor("b", "a"))))),
         },
         Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    ASTPrinter& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();

    auto got = gen.Result();
    auto* expect = R"(ByteAddressBuffer b : register(t1, space2);

void a_func() {
  uint tint_symbol_1 = 0u;
  b.GetDimensions(tint_symbol_1);
  const uint tint_symbol_2 = ((tint_symbol_1 - 4u) / 4u);
  uint len = tint_symbol_2;
  return;
}
)";

    EXPECT_EQ(expect, got);
}

TEST_F(HlslSanitizerTest, Call_ArrayLength_ViaLets) {
    auto* s = Structure("my_struct", Vector{Member(0, "a", ty.array<f32>())});
    GlobalVar("b", ty.Of(s), core::AddressSpace::kStorage, core::Access::kRead, Binding(1_a),
              Group(2_a));

    auto* p = Let("p", AddressOf("b"));
    auto* p2 = Let("p2", AddressOf(MemberAccessor(Deref(p), "a")));

    Func("a_func", tint::Empty, ty.void_(),
         Vector{
             Decl(p),
             Decl(p2),
             Decl(Var("len", ty.u32(), Call("arrayLength", p2))),
         },
         Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    ASTPrinter& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();

    auto got = gen.Result();
    auto* expect = R"(ByteAddressBuffer b : register(t1, space2);

void a_func() {
  uint tint_symbol_1 = 0u;
  b.GetDimensions(tint_symbol_1);
  const uint tint_symbol_2 = ((tint_symbol_1 - 0u) / 4u);
  uint len = tint_symbol_2;
  return;
}
)";

    EXPECT_EQ(expect, got);
}

TEST_F(HlslSanitizerTest, Call_ArrayLength_ArrayLengthFromUniform) {
    auto* s = Structure("my_struct", Vector{Member(0, "a", ty.array<f32>())});
    GlobalVar("b", ty.Of(s), core::AddressSpace::kStorage, core::Access::kRead, Binding(1_a),
              Group(2_a));
    GlobalVar("c", ty.Of(s), core::AddressSpace::kStorage, core::Access::kRead, Binding(2_a),
              Group(2_a));

    Func("a_func", tint::Empty, ty.void_(),
         Vector{
             Decl(Var("len", ty.u32(),
                      Add(Call("arrayLength", AddressOf(MemberAccessor("b", "a"))),
                          Call("arrayLength", AddressOf(MemberAccessor("c", "a")))))),
         },
         Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    Options options;
    options.array_length_from_uniform.ubo_binding = {3, 4};
    options.array_length_from_uniform.bindpoint_to_size_index.emplace(BindingPoint{2, 2}, 7u);
    ASTPrinter& gen = SanitizeAndBuild(options);

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();

    auto got = gen.Result();
    auto* expect = R"(cbuffer cbuffer_tint_symbol_1 : register(b4, space3) {
  uint4 tint_symbol_1[2];
};
ByteAddressBuffer b : register(t1, space2);
ByteAddressBuffer c : register(t2, space2);

void a_func() {
  uint tint_symbol_3 = 0u;
  b.GetDimensions(tint_symbol_3);
  const uint tint_symbol_4 = ((tint_symbol_3 - 0u) / 4u);
  uint len = (tint_symbol_4 + ((tint_symbol_1[1].w - 0u) / 4u));
  return;
}
)";
    EXPECT_EQ(expect, got);
}

TEST_F(HlslSanitizerTest, PromoteArrayInitializerToConstVar) {
    auto* array_init = Call<array<i32, 4>>(1_i, 2_i, 3_i, 4_i);

    Func("main", tint::Empty, ty.void_(),
         Vector{
             Decl(Var("idx", Expr(3_i))),
             Decl(Var("pos", ty.i32(), IndexAccessor(array_init, "idx"))),
         },
         Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    ASTPrinter& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();

    auto got = gen.Result();
    auto* expect = R"(void main() {
  int idx = 3;
  const int tint_symbol[4] = {1, 2, 3, 4};
  int pos = tint_symbol[idx];
  return;
}
)";
    EXPECT_EQ(expect, got);
}

TEST_F(HlslSanitizerTest, PromoteStructInitializerToConstVar) {
    auto* runtime_value = Var("runtime_value", Expr(3_f));
    auto* str = Structure("S", Vector{
                                   Member("a", ty.i32()),
                                   Member("b", ty.vec3<f32>()),
                                   Member("c", ty.i32()),
                               });
    auto* struct_init = Call(ty.Of(str), 1_i, Call<vec3<f32>>(2_f, runtime_value, 4_f), 4_i);
    auto* struct_access = MemberAccessor(struct_init, "b");
    auto* pos = Var("pos", ty.vec3<f32>(), struct_access);

    Func("main", tint::Empty, ty.void_(),
         Vector{
             Decl(runtime_value),
             Decl(pos),
         },
         Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    ASTPrinter& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();

    auto got = gen.Result();
    auto* expect = R"(struct S {
  int a;
  float3 b;
  int c;
};

void main() {
  float runtime_value = 3.0f;
  const S tint_symbol = {1, float3(2.0f, runtime_value, 4.0f), 4};
  float3 pos = tint_symbol.b;
  return;
}
)";
    EXPECT_EQ(expect, got);
}

TEST_F(HlslSanitizerTest, SimplifyPointersBasic) {
    // var v : i32;
    // let p : ptr<function, i32> = &v;
    // let x : i32 = *p;
    auto* v = Var("v", ty.i32());
    auto* p = Let("p", ty.ptr<function, i32>(), AddressOf(v));
    auto* x = Var("x", ty.i32(), Deref(p));

    Func("main", tint::Empty, ty.void_(),
         Vector{
             Decl(v),
             Decl(p),
             Decl(x),
         },
         Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    ASTPrinter& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();

    auto got = gen.Result();
    auto* expect = R"(void main() {
  int v = 0;
  int x = v;
  return;
}
)";
    EXPECT_EQ(expect, got);
}

TEST_F(HlslSanitizerTest, SimplifyPointersComplexChain) {
    // var a : array<mat4x4<f32>, 4u>;
    // let ap : ptr<function, array<mat4x4<f32>, 4u>> = &a;
    // let mp : ptr<function, mat4x4<f32>> = &(*ap)[3i];
    // let vp : ptr<function, vec4<f32>> = &(*mp)[2i];
    // let v : vec4<f32> = *vp;
    auto* a = Var("a", ty.array(ty.mat4x4<f32>(), 4_u));
    auto* ap = Let("ap", ty.ptr<function, array<mat4x4<f32>, 4>>(), AddressOf(a));
    auto* mp = Let("mp", ty.ptr<function, mat4x4<f32>>(), AddressOf(IndexAccessor(Deref(ap), 3_i)));
    auto* vp = Let("vp", ty.ptr<function, vec4<f32>>(), AddressOf(IndexAccessor(Deref(mp), 2_i)));
    auto* v = Var("v", ty.vec4<f32>(), Deref(vp));

    Func("main", tint::Empty, ty.void_(),
         Vector{
             Decl(a),
             Decl(ap),
             Decl(mp),
             Decl(vp),
             Decl(v),
         },
         Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    ASTPrinter& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();

    auto got = gen.Result();
    auto* expect = R"(void main() {
  float4x4 a[4] = (float4x4[4])0;
  float4 v = a[3][2];
  return;
}
)";
    EXPECT_EQ(expect, got);
}

}  // namespace
}  // namespace tint::hlsl::writer
