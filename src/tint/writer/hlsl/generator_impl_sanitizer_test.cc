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

#include "src/tint/ast/call_statement.h"
#include "src/tint/ast/stage_attribute.h"
#include "src/tint/ast/variable_decl_statement.h"
#include "src/tint/writer/hlsl/test_helper.h"

namespace tint::writer::hlsl {
namespace {

using HlslSanitizerTest = TestHelper;

TEST_F(HlslSanitizerTest, Call_ArrayLength) {
    auto* s = Structure("my_struct", {Member(0, "a", ty.array<f32>(4))});
    Global("b", ty.Of(s), ast::StorageClass::kStorage, ast::Access::kRead,
           ast::AttributeList{
               create<ast::BindingAttribute>(1),
               create<ast::GroupAttribute>(2),
           });

    Func("a_func", ast::VariableList{}, ty.void_(),
         ast::StatementList{
             Decl(Var("len", ty.u32(), ast::StorageClass::kNone,
                      Call("arrayLength", AddressOf(MemberAccessor("b", "a"))))),
         },
         ast::AttributeList{
             Stage(ast::PipelineStage::kFragment),
         });

    GeneratorImpl& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.error();

    auto got = gen.result();
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
    auto* s = Structure("my_struct", {
                                         Member(0, "z", ty.f32()),
                                         Member(4, "a", ty.array<f32>(4)),
                                     });
    Global("b", ty.Of(s), ast::StorageClass::kStorage, ast::Access::kRead,
           ast::AttributeList{
               create<ast::BindingAttribute>(1),
               create<ast::GroupAttribute>(2),
           });

    Func("a_func", ast::VariableList{}, ty.void_(),
         ast::StatementList{
             Decl(Var("len", ty.u32(), ast::StorageClass::kNone,
                      Call("arrayLength", AddressOf(MemberAccessor("b", "a"))))),
         },
         ast::AttributeList{
             Stage(ast::PipelineStage::kFragment),
         });

    GeneratorImpl& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.error();

    auto got = gen.result();
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
    auto* s = Structure("my_struct", {Member(0, "a", ty.array<f32>(4))});
    Global("b", ty.Of(s), ast::StorageClass::kStorage, ast::Access::kRead,
           ast::AttributeList{
               create<ast::BindingAttribute>(1),
               create<ast::GroupAttribute>(2),
           });

    auto* p = Let("p", nullptr, AddressOf("b"));
    auto* p2 = Let("p2", nullptr, AddressOf(MemberAccessor(Deref(p), "a")));

    Func("a_func", ast::VariableList{}, ty.void_(),
         ast::StatementList{
             Decl(p),
             Decl(p2),
             Decl(Var("len", ty.u32(), ast::StorageClass::kNone, Call("arrayLength", p2))),
         },
         ast::AttributeList{
             Stage(ast::PipelineStage::kFragment),
         });

    GeneratorImpl& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.error();

    auto got = gen.result();
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
    auto* s = Structure("my_struct", {Member(0, "a", ty.array<f32>(4))});
    Global("b", ty.Of(s), ast::StorageClass::kStorage, ast::Access::kRead,
           ast::AttributeList{
               create<ast::BindingAttribute>(1),
               create<ast::GroupAttribute>(2),
           });
    Global("c", ty.Of(s), ast::StorageClass::kStorage, ast::Access::kRead,
           ast::AttributeList{
               create<ast::BindingAttribute>(2),
               create<ast::GroupAttribute>(2),
           });

    Func("a_func", ast::VariableList{}, ty.void_(),
         ast::StatementList{
             Decl(Var("len", ty.u32(), ast::StorageClass::kNone,
                      Add(Call("arrayLength", AddressOf(MemberAccessor("b", "a"))),
                          Call("arrayLength", AddressOf(MemberAccessor("c", "a")))))),
         },
         ast::AttributeList{
             Stage(ast::PipelineStage::kFragment),
         });

    Options options;
    options.array_length_from_uniform.ubo_binding = {3, 4};
    options.array_length_from_uniform.bindpoint_to_size_index.emplace(sem::BindingPoint{2, 2}, 7u);
    GeneratorImpl& gen = SanitizeAndBuild(options);

    ASSERT_TRUE(gen.Generate()) << gen.error();

    auto got = gen.result();
    auto* expect = R"(cbuffer cbuffer_tint_symbol_1 : register(b4, space3) {
  uint4 tint_symbol_1[2];
};
ByteAddressBuffer b : register(t1, space2);
ByteAddressBuffer c : register(t2, space2);

void a_func() {
  uint tint_symbol_4 = 0u;
  b.GetDimensions(tint_symbol_4);
  const uint tint_symbol_5 = ((tint_symbol_4 - 0u) / 4u);
  uint len = (tint_symbol_5 + ((tint_symbol_1[1].w - 0u) / 4u));
  return;
}
)";
    EXPECT_EQ(expect, got);
}

TEST_F(HlslSanitizerTest, PromoteArrayInitializerToConstVar) {
    auto* array_init = array<i32, 4>(1, 2, 3, 4);
    auto* array_index = IndexAccessor(array_init, 3);
    auto* pos = Var("pos", ty.i32(), ast::StorageClass::kNone, array_index);

    Func("main", ast::VariableList{}, ty.void_(),
         {
             Decl(pos),
         },
         {
             Stage(ast::PipelineStage::kFragment),
         });

    GeneratorImpl& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.error();

    auto got = gen.result();
    auto* expect = R"(void main() {
  const int tint_symbol[4] = {1, 2, 3, 4};
  int pos = tint_symbol[3];
  return;
}
)";
    EXPECT_EQ(expect, got);
}

TEST_F(HlslSanitizerTest, PromoteStructInitializerToConstVar) {
    auto* str = Structure("S", {
                                   Member("a", ty.i32()),
                                   Member("b", ty.vec3<f32>()),
                                   Member("c", ty.i32()),
                               });
    auto* struct_init = Construct(ty.Of(str), 1, vec3<f32>(2.f, 3.f, 4.f), 4);
    auto* struct_access = MemberAccessor(struct_init, "b");
    auto* pos = Var("pos", ty.vec3<f32>(), ast::StorageClass::kNone, struct_access);

    Func("main", ast::VariableList{}, ty.void_(),
         {
             Decl(pos),
         },
         {
             Stage(ast::PipelineStage::kFragment),
         });

    GeneratorImpl& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.error();

    auto got = gen.result();
    auto* expect = R"(struct S {
  int a;
  float3 b;
  int c;
};

void main() {
  const S tint_symbol = {1, float3(2.0f, 3.0f, 4.0f), 4};
  float3 pos = tint_symbol.b;
  return;
}
)";
    EXPECT_EQ(expect, got);
}

TEST_F(HlslSanitizerTest, InlinePtrLetsBasic) {
    // var v : i32;
    // let p : ptr<function, i32> = &v;
    // let x : i32 = *p;
    auto* v = Var("v", ty.i32());
    auto* p = Let("p", ty.pointer<i32>(ast::StorageClass::kFunction), AddressOf(v));
    auto* x = Var("x", ty.i32(), ast::StorageClass::kNone, Deref(p));

    Func("main", ast::VariableList{}, ty.void_(),
         {
             Decl(v),
             Decl(p),
             Decl(x),
         },
         {
             Stage(ast::PipelineStage::kFragment),
         });

    GeneratorImpl& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.error();

    auto got = gen.result();
    auto* expect = R"(void main() {
  int v = 0;
  int x = v;
  return;
}
)";
    EXPECT_EQ(expect, got);
}

TEST_F(HlslSanitizerTest, InlinePtrLetsComplexChain) {
    // var a : array<mat4x4<f32>, 4>;
    // let ap : ptr<function, array<mat4x4<f32>, 4>> = &a;
    // let mp : ptr<function, mat4x4<f32>> = &(*ap)[3];
    // let vp : ptr<function, vec4<f32>> = &(*mp)[2];
    // let v : vec4<f32> = *vp;
    auto* a = Var("a", ty.array(ty.mat4x4<f32>(), 4));
    auto* ap = Let("ap", ty.pointer(ty.array(ty.mat4x4<f32>(), 4), ast::StorageClass::kFunction),
                   AddressOf(a));
    auto* mp = Let("mp", ty.pointer(ty.mat4x4<f32>(), ast::StorageClass::kFunction),
                   AddressOf(IndexAccessor(Deref(ap), 3)));
    auto* vp = Let("vp", ty.pointer(ty.vec4<f32>(), ast::StorageClass::kFunction),
                   AddressOf(IndexAccessor(Deref(mp), 2)));
    auto* v = Var("v", ty.vec4<f32>(), ast::StorageClass::kNone, Deref(vp));

    Func("main", ast::VariableList{}, ty.void_(),
         {
             Decl(a),
             Decl(ap),
             Decl(mp),
             Decl(vp),
             Decl(v),
         },
         {
             Stage(ast::PipelineStage::kFragment),
         });

    GeneratorImpl& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.error();

    auto got = gen.result();
    auto* expect = R"(void main() {
  float4x4 a[4] = (float4x4[4])0;
  float4 v = a[3][2];
  return;
}
)";
    EXPECT_EQ(expect, got);
}

}  // namespace
}  // namespace tint::writer::hlsl
