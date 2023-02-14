// Copyright 2022 The Tint Authors.
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

#include "src/tint/sem/load.h"
#include "gmock/gmock.h"
#include "src/tint/resolver/resolver.h"
#include "src/tint/resolver/resolver_test_helper.h"
#include "src/tint/sem/test_helper.h"
#include "src/tint/type/reference.h"
#include "src/tint/type/texture_dimension.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::resolver {
namespace {

using ResolverLoadTest = ResolverTest;

TEST_F(ResolverLoadTest, VarInitializer) {
    // var ref = 1i;
    // var v = ref;
    auto* ident = Expr("ref");
    WrapInFunction(Var("ref", Expr(1_i)),  //
                   Var("v", ident));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    auto* load = Sem().Get<sem::Load>(ident);
    ASSERT_NE(load, nullptr);
    EXPECT_TRUE(load->Type()->Is<type::I32>());
    EXPECT_TRUE(load->Reference()->Type()->Is<type::Reference>());
    EXPECT_TRUE(load->Reference()->Type()->UnwrapRef()->Is<type::I32>());
}

TEST_F(ResolverLoadTest, LetInitializer) {
    // var ref = 1i;
    // let l = ref;
    auto* ident = Expr("ref");
    WrapInFunction(Var("ref", Expr(1_i)),  //
                   Let("l", ident));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    auto* load = Sem().Get<sem::Load>(ident);
    ASSERT_NE(load, nullptr);
    EXPECT_TRUE(load->Type()->Is<type::I32>());
    EXPECT_TRUE(load->Reference()->Type()->Is<type::Reference>());
    EXPECT_TRUE(load->Reference()->Type()->UnwrapRef()->Is<type::I32>());
}

TEST_F(ResolverLoadTest, Assignment) {
    // var ref = 1i;
    // var v : i32;
    // v = ref;
    auto* ident = Expr("ref");
    WrapInFunction(Var("ref", Expr(1_i)),  //
                   Var("v", ty.i32()),     //
                   Assign("v", ident));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    auto* load = Sem().Get<sem::Load>(ident);
    ASSERT_NE(load, nullptr);
    EXPECT_TRUE(load->Type()->Is<type::I32>());
    EXPECT_TRUE(load->Reference()->Type()->Is<type::Reference>());
    EXPECT_TRUE(load->Reference()->Type()->UnwrapRef()->Is<type::I32>());
}

TEST_F(ResolverLoadTest, CompoundAssignment) {
    // var ref = 1i;
    // var v : i32;
    // v += ref;
    auto* ident = Expr("ref");
    WrapInFunction(Var("ref", Expr(1_i)),  //
                   Var("v", ty.i32()),     //
                   CompoundAssign("v", ident, ast::BinaryOp::kAdd));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    auto* load = Sem().Get<sem::Load>(ident);
    ASSERT_NE(load, nullptr);
    EXPECT_TRUE(load->Type()->Is<type::I32>());
    EXPECT_TRUE(load->Reference()->Type()->Is<type::Reference>());
    EXPECT_TRUE(load->Reference()->Type()->UnwrapRef()->Is<type::I32>());
}

TEST_F(ResolverLoadTest, UnaryOp) {
    // var ref = 1i;
    // var v = -ref;
    auto* ident = Expr("ref");
    WrapInFunction(Var("ref", Expr(1_i)),  //
                   Var("v", Negation(ident)));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    auto* load = Sem().Get<sem::Load>(ident);
    ASSERT_NE(load, nullptr);
    EXPECT_TRUE(load->Type()->Is<type::I32>());
    EXPECT_TRUE(load->Reference()->Type()->Is<type::Reference>());
    EXPECT_TRUE(load->Reference()->Type()->UnwrapRef()->Is<type::I32>());
}

TEST_F(ResolverLoadTest, UnaryOp_NoLoad) {
    // var ref = 1i;
    // let v = &ref;
    auto* ident = Expr("ref");
    WrapInFunction(Var("ref", Expr(1_i)),  //
                   Let("v", AddressOf(ident)));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    auto* var_user = Sem().Get<sem::VariableUser>(ident);
    ASSERT_NE(var_user, nullptr);
    EXPECT_TRUE(var_user->Type()->Is<type::Reference>());
    EXPECT_TRUE(var_user->Type()->UnwrapRef()->Is<type::I32>());
}

TEST_F(ResolverLoadTest, BinaryOp) {
    // var ref = 1i;
    // var v = ref * 1i;
    auto* ident = Expr("ref");
    WrapInFunction(Var("ref", Expr(1_i)),  //
                   Var("v", Mul(ident, 1_i)));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    auto* load = Sem().Get<sem::Load>(ident);
    ASSERT_NE(load, nullptr);
    EXPECT_TRUE(load->Type()->Is<type::I32>());
    EXPECT_TRUE(load->Reference()->Type()->Is<type::Reference>());
    EXPECT_TRUE(load->Reference()->Type()->UnwrapRef()->Is<type::I32>());
}

TEST_F(ResolverLoadTest, Index) {
    // var ref = 1i;
    // var v = array<i32, 3>(1i, 2i, 3i)[ref];
    auto* ident = Expr("ref");
    WrapInFunction(Var("ref", Expr(1_i)),  //
                   IndexAccessor(array<i32, 3>(1_i, 2_i, 3_i), ident));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    auto* load = Sem().Get<sem::Load>(ident);
    ASSERT_NE(load, nullptr);
    EXPECT_TRUE(load->Type()->Is<type::I32>());
    EXPECT_TRUE(load->Reference()->Type()->Is<type::Reference>());
    EXPECT_TRUE(load->Reference()->Type()->UnwrapRef()->Is<type::I32>());
}

TEST_F(ResolverLoadTest, MultiComponentSwizzle) {
    // var ref = vec4(1);
    // var v = ref.xyz;
    auto* ident = Expr("ref");
    WrapInFunction(Var("ref", vec4<i32>(1_i)),  //
                   Var("v", MemberAccessor(ident, "xyz")));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    auto* load = Sem().Get<sem::Load>(ident);
    ASSERT_NE(load, nullptr);
    EXPECT_TRUE(load->Type()->Is<type::Vector>());
    EXPECT_TRUE(load->Reference()->Type()->Is<type::Reference>());
    EXPECT_TRUE(load->Reference()->Type()->UnwrapRef()->Is<type::Vector>());
}

TEST_F(ResolverLoadTest, Bitcast) {
    // var ref = 1f;
    // var v = bitcast<i32>(ref);
    auto* ident = Expr("ref");
    WrapInFunction(Var("ref", Expr(1_f)),  //
                   Bitcast<i32>(ident));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    auto* load = Sem().Get<sem::Load>(ident);
    ASSERT_NE(load, nullptr);
    EXPECT_TRUE(load->Type()->Is<type::F32>());
    EXPECT_TRUE(load->Reference()->Type()->Is<type::Reference>());
    EXPECT_TRUE(load->Reference()->Type()->UnwrapRef()->Is<type::F32>());
}

TEST_F(ResolverLoadTest, BuiltinArg) {
    // var ref = 1f;
    // var v = abs(ref);
    auto* ident = Expr("ref");
    WrapInFunction(Var("ref", Expr(1_f)),  //
                   Call("abs", ident));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    auto* load = Sem().Get<sem::Load>(ident);
    ASSERT_NE(load, nullptr);
    EXPECT_TRUE(load->Type()->Is<type::F32>());
    EXPECT_TRUE(load->Reference()->Type()->Is<type::Reference>());
    EXPECT_TRUE(load->Reference()->Type()->UnwrapRef()->Is<type::F32>());
}

TEST_F(ResolverLoadTest, FunctionArg) {
    // fn f(x : f32) {}
    // var ref = 1f;
    // f(ref);
    Func("f", utils::Vector{Param("x", ty.f32())}, ty.void_(), utils::Empty);
    auto* ident = Expr("ref");
    WrapInFunction(Var("ref", Expr(1_f)),  //
                   CallStmt(Call("f", ident)));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    auto* load = Sem().Get<sem::Load>(ident);
    ASSERT_NE(load, nullptr);
    EXPECT_TRUE(load->Type()->Is<type::F32>());
    EXPECT_TRUE(load->Reference()->Type()->Is<type::Reference>());
    EXPECT_TRUE(load->Reference()->Type()->UnwrapRef()->Is<type::F32>());
}

TEST_F(ResolverLoadTest, FunctionArg_Handles) {
    // @group(0) @binding(0) var t : texture_2d<f32>;
    // @group(0) @binding(1) var s : sampler;
    // fn f(tp : texture_2d<f32>, sp : sampler) -> vec4<f32> {
    //   return textureSampleLevel(tp, sp, vec2(), 0);
    // }
    // f(t, s);
    GlobalVar("t", ty.sampled_texture(type::TextureDimension::k2d, ty.f32()),
              utils::Vector{Group(0_a), Binding(0_a)});
    GlobalVar("s", ty.sampler(type::SamplerKind::kSampler),
              utils::Vector{Group(0_a), Binding(1_a)});
    Func("f",
         utils::Vector{
             Param("tp", ty.sampled_texture(type::TextureDimension::k2d, ty.f32())),
             Param("sp", ty.sampler(type::SamplerKind::kSampler)),
         },
         ty.vec4<f32>(),
         utils::Vector{
             Return(Call("textureSampleLevel", "tp", "sp", vec2<f32>(), 0_a)),
         });
    auto* t_ident = Expr("t");
    auto* s_ident = Expr("s");
    WrapInFunction(CallStmt(Call("f", t_ident, s_ident)));

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    {
        auto* load = Sem().Get<sem::Load>(t_ident);
        ASSERT_NE(load, nullptr);
        EXPECT_TRUE(load->Type()->Is<type::SampledTexture>());
        EXPECT_TRUE(load->Reference()->Type()->Is<type::Reference>());
        EXPECT_TRUE(load->Reference()->Type()->UnwrapRef()->Is<type::SampledTexture>());
    }
    {
        auto* load = Sem().Get<sem::Load>(s_ident);
        ASSERT_NE(load, nullptr);
        EXPECT_TRUE(load->Type()->Is<type::Sampler>());
        EXPECT_TRUE(load->Reference()->Type()->Is<type::Reference>());
        EXPECT_TRUE(load->Reference()->Type()->UnwrapRef()->Is<type::Sampler>());
    }
}

TEST_F(ResolverLoadTest, FunctionReturn) {
    // var ref = 1f;
    // return ref;
    auto* ident = Expr("ref");
    Func("f", utils::Empty, ty.f32(),
         utils::Vector{
             Decl(Var("ref", Expr(1_f))),
             Return(ident),
         });

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    auto* load = Sem().Get<sem::Load>(ident);
    ASSERT_NE(load, nullptr);
    EXPECT_TRUE(load->Type()->Is<type::F32>());
    EXPECT_TRUE(load->Reference()->Type()->Is<type::Reference>());
    EXPECT_TRUE(load->Reference()->Type()->UnwrapRef()->Is<type::F32>());
}

TEST_F(ResolverLoadTest, IfCond) {
    // var ref = false;
    // if (ref) {}
    auto* ident = Expr("ref");
    WrapInFunction(Var("ref", Expr(false)),  //
                   If(ident, Block()));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    auto* load = Sem().Get<sem::Load>(ident);
    ASSERT_NE(load, nullptr);
    EXPECT_TRUE(load->Type()->Is<type::Bool>());
    EXPECT_TRUE(load->Reference()->Type()->Is<type::Reference>());
    EXPECT_TRUE(load->Reference()->Type()->UnwrapRef()->Is<type::Bool>());
}

TEST_F(ResolverLoadTest, Switch) {
    // var ref = 1i;
    // switch (ref) {
    //   default:
    // }
    auto* ident = Expr("ref");
    WrapInFunction(Var("ref", Expr(1_i)),  //
                   Switch(ident, DefaultCase()));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    auto* load = Sem().Get<sem::Load>(ident);
    ASSERT_NE(load, nullptr);
    EXPECT_TRUE(load->Type()->Is<type::I32>());
    EXPECT_TRUE(load->Reference()->Type()->Is<type::Reference>());
    EXPECT_TRUE(load->Reference()->Type()->UnwrapRef()->Is<type::I32>());
}

TEST_F(ResolverLoadTest, BreakIfCond) {
    // var ref = false;
    // loop {
    //   continuing {
    //     break if (ref);
    //   }
    // }
    auto* ident = Expr("ref");
    WrapInFunction(Var("ref", Expr(false)),  //
                   Loop(Block(), Block(BreakIf(ident))));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    auto* load = Sem().Get<sem::Load>(ident);
    ASSERT_NE(load, nullptr);
    EXPECT_TRUE(load->Type()->Is<type::Bool>());
    EXPECT_TRUE(load->Reference()->Type()->Is<type::Reference>());
    EXPECT_TRUE(load->Reference()->Type()->UnwrapRef()->Is<type::Bool>());
}

TEST_F(ResolverLoadTest, ForCond) {
    // var ref = false;
    // for (; ref; ) {}
    auto* ident = Expr("ref");
    WrapInFunction(Var("ref", Expr(false)),  //
                   For(nullptr, ident, nullptr, Block()));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    auto* load = Sem().Get<sem::Load>(ident);
    ASSERT_NE(load, nullptr);
    EXPECT_TRUE(load->Type()->Is<type::Bool>());
    EXPECT_TRUE(load->Reference()->Type()->Is<type::Reference>());
    EXPECT_TRUE(load->Reference()->Type()->UnwrapRef()->Is<type::Bool>());
}

TEST_F(ResolverLoadTest, WhileCond) {
    // var ref = false;
    // while (ref) {}
    auto* ident = Expr("ref");
    WrapInFunction(Var("ref", Expr(false)),  //
                   While(ident, Block()));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    auto* load = Sem().Get<sem::Load>(ident);
    ASSERT_NE(load, nullptr);
    EXPECT_TRUE(load->Type()->Is<type::Bool>());
    EXPECT_TRUE(load->Reference()->Type()->Is<type::Reference>());
    EXPECT_TRUE(load->Reference()->Type()->UnwrapRef()->Is<type::Bool>());
}

TEST_F(ResolverLoadTest, AddressOf) {
    // var ref = 1i;
    // let l = &ref;
    auto* ident = Expr("ref");
    WrapInFunction(Var("ref", Expr(1_i)),  //
                   Let("l", AddressOf(ident)));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    auto* no_load = Sem().GetVal(ident);
    ASSERT_NE(no_load, nullptr);
    EXPECT_TRUE(no_load->Type()->Is<type::Reference>());  // No load
}

}  // namespace
}  // namespace tint::resolver
