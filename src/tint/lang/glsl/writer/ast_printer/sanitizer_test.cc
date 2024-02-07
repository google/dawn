// Copyright 2021 The Dawn & Tint Authors
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

#include "src/tint/lang/glsl/writer/ast_printer/helper_test.h"
#include "src/tint/lang/wgsl/ast/call_statement.h"
#include "src/tint/lang/wgsl/ast/stage_attribute.h"
#include "src/tint/lang/wgsl/ast/variable_decl_statement.h"

#include "gmock/gmock.h"

namespace tint::glsl::writer {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

using GlslSanitizerTest = TestHelper;

TEST_F(GlslSanitizerTest, Call_ArrayLength) {
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
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());

    auto got = gen.Result();
    auto* expect = R"(#version 310 es
precision highp float;
precision highp int;

layout(binding = 1, std430) buffer my_struct_ssbo {
  float a[];
} b;

void a_func() {
  uint len = uint(b.a.length());
}

void main() {
  a_func();
  return;
}
)";
    EXPECT_EQ(expect, got);
}

TEST_F(GlslSanitizerTest, Call_ArrayLength_OtherMembersInStruct) {
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
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());

    auto got = gen.Result();
    auto* expect = R"(#version 310 es
precision highp float;
precision highp int;

layout(binding = 1, std430) buffer my_struct_ssbo {
  float z;
  float a[];
} b;

void a_func() {
  uint len = uint(b.a.length());
}

void main() {
  a_func();
  return;
}
)";

    EXPECT_EQ(expect, got);
}

TEST_F(GlslSanitizerTest, Call_ArrayLength_ViaLets) {
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
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());

    auto got = gen.Result();
    auto* expect = R"(#version 310 es
precision highp float;
precision highp int;

layout(binding = 1, std430) buffer my_struct_ssbo {
  float a[];
} b;

void a_func() {
  uint len = uint(b.a.length());
}

void main() {
  a_func();
  return;
}
)";

    EXPECT_EQ(expect, got);
}

TEST_F(GlslSanitizerTest, PromoteArrayInitializerToConstVar) {
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
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());

    auto got = gen.Result();
    auto* expect = R"(#version 310 es
precision highp float;
precision highp int;

void tint_symbol() {
  int idx = 3;
  int tint_symbol_1[4] = int[4](1, 2, 3, 4);
  int pos = tint_symbol_1[idx];
}

void main() {
  tint_symbol();
  return;
}
)";
    EXPECT_EQ(expect, got);
}

TEST_F(GlslSanitizerTest, PromoteStructInitializerToConstVar) {
    auto* str = Structure("S", Vector{
                                   Member("a", ty.i32()),
                                   Member("b", ty.vec3<f32>()),
                                   Member("c", ty.i32()),
                               });
    auto* runtime_value = Var("runtime_value", Expr(3_f));
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
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());

    auto got = gen.Result();
    auto* expect = R"(#version 310 es
precision highp float;
precision highp int;

struct S {
  int a;
  vec3 b;
  int c;
};

void tint_symbol() {
  float runtime_value = 3.0f;
  S tint_symbol_1 = S(1, vec3(2.0f, runtime_value, 4.0f), 4);
  vec3 pos = tint_symbol_1.b;
}

void main() {
  tint_symbol();
  return;
}
)";
    EXPECT_EQ(expect, got);
}

TEST_F(GlslSanitizerTest, SimplifyPointersBasic) {
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
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());

    auto got = gen.Result();
    auto* expect = R"(#version 310 es
precision highp float;
precision highp int;

void tint_symbol() {
  int v = 0;
  int x = v;
}

void main() {
  tint_symbol();
  return;
}
)";
    EXPECT_EQ(expect, got);
}

TEST_F(GlslSanitizerTest, SimplifyPointersComplexChain) {
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
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());

    auto got = gen.Result();
    auto* expect = R"(#version 310 es
precision highp float;
precision highp int;

void tint_symbol() {
  mat4 a[4] = mat4[4](mat4(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f), mat4(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f), mat4(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f), mat4(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f));
  vec4 v = a[3][2];
}

void main() {
  tint_symbol();
  return;
}
)";
    EXPECT_EQ(expect, got);
}

}  // namespace
}  // namespace tint::glsl::writer
