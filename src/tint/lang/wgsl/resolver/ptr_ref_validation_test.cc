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

#include "gmock/gmock.h"
#include "src/tint/lang/core/type/reference.h"
#include "src/tint/lang/core/type/texture_dimension.h"
#include "src/tint/lang/wgsl/ast/bitcast_expression.h"
#include "src/tint/lang/wgsl/resolver/resolver.h"
#include "src/tint/lang/wgsl/resolver/resolver_helper_test.h"

namespace tint::resolver {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

struct ResolverPtrRefValidationTest : public resolver::TestHelper, public testing::Test {};

TEST_F(ResolverPtrRefValidationTest, AddressOfLiteral) {
    // &1

    auto* expr = AddressOf(Expr(Source{{12, 34}}, 1_i));

    WrapInFunction(expr);

    EXPECT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(), "12:34 error: cannot take the address of expression");
}

TEST_F(ResolverPtrRefValidationTest, AddressOfLet) {
    // let l : i32 = 1;
    // &l
    auto* l = Let("l", ty.i32(), Expr(1_i));
    auto* expr = AddressOf(Expr(Source{{12, 34}}, "l"));

    WrapInFunction(l, expr);

    EXPECT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(), "12:34 error: cannot take the address of expression");
}

TEST_F(ResolverPtrRefValidationTest, AddressOfHandle) {
    // @group(0) @binding(0) var t: texture_3d<f32>;
    // &t
    GlobalVar("t", ty.sampled_texture(core::type::TextureDimension::k3d, ty.f32()), Group(0_a),
              Binding(0_a));
    auto* expr = AddressOf(Expr(Source{{12, 34}}, "t"));
    WrapInFunction(expr);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: cannot take the address of expression in handle "
              "address space");
}

TEST_F(ResolverPtrRefValidationTest, AddressOfVectorComponent_MemberAccessor) {
    // var v : vec4<i32>;
    // &v.y
    auto* v = Var("v", ty.vec4<i32>());
    auto* expr = AddressOf(MemberAccessor(Source{{12, 34}}, "v", "y"));

    WrapInFunction(v, expr);

    EXPECT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(), "12:34 error: cannot take the address of a vector component");
}

TEST_F(ResolverPtrRefValidationTest, AddressOfVectorComponent_IndexAccessor) {
    // var v : vec4<i32>;
    // &v[2i]
    auto* v = Var("v", ty.vec4<i32>());
    auto* expr = AddressOf(IndexAccessor(Source{{12, 34}}, "v", 2_i));

    WrapInFunction(v, expr);

    EXPECT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(), "12:34 error: cannot take the address of a vector component");
}

TEST_F(ResolverPtrRefValidationTest, IndirectOfAddressOfHandle) {
    // @group(0) @binding(0) var t: texture_3d<f32>;
    // *&t
    GlobalVar("t", ty.sampled_texture(core::type::TextureDimension::k3d, ty.f32()), Group(0_a),
              Binding(0_a));
    auto* expr = Deref(AddressOf(Expr(Source{{12, 34}}, "t")));
    WrapInFunction(expr);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: cannot take the address of expression in handle "
              "address space");
}

TEST_F(ResolverPtrRefValidationTest, DerefOfLiteral) {
    // *1

    auto* expr = Deref(Expr(Source{{12, 34}}, 1_i));

    WrapInFunction(expr);

    EXPECT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(), "12:34 error: cannot dereference expression of type 'i32'");
}

TEST_F(ResolverPtrRefValidationTest, DerefOfVar) {
    // var v : i32;
    // *v
    auto* v = Var("v", ty.i32());
    auto* expr = Deref(Expr(Source{{12, 34}}, "v"));

    WrapInFunction(v, expr);

    EXPECT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(), "12:34 error: cannot dereference expression of type 'i32'");
}

TEST_F(ResolverPtrRefValidationTest, InferredPtrAccessMismatch) {
    // struct Inner {
    //    arr: array<i32, 4u>;
    // }
    // struct S {
    //    inner: Inner;
    // }
    // @group(0) @binding(0) var<storage, read_write> s : S;
    // fn f() {
    //   let p : pointer<storage, i32> = &s.inner.arr[2i];
    // }
    auto* inner = Structure("Inner", Vector{Member("arr", ty.array<i32, 4>())});
    auto* buf = Structure("S", Vector{Member("inner", ty.Of(inner))});
    auto* var = GlobalVar("s", ty.Of(buf), core::AddressSpace::kStorage, core::Access::kReadWrite,
                          Binding(0_a), Group(0_a));

    auto* expr = IndexAccessor(MemberAccessor(MemberAccessor(var, "inner"), "arr"), 2_i);
    auto* ptr = Let(Source{{12, 34}}, "p", ty.ptr<storage, i32>(), AddressOf(expr));

    WrapInFunction(ptr);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: cannot initialize let of type "
              "'ptr<storage, i32, read>' with value of type "
              "'ptr<storage, i32, read_write>'");
}

}  // namespace
}  // namespace tint::resolver
