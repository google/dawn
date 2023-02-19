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

#include "gmock/gmock.h"
#include "src/tint/ast/bitcast_expression.h"
#include "src/tint/resolver/resolver.h"
#include "src/tint/resolver/resolver_test_helper.h"
#include "src/tint/type/reference.h"
#include "src/tint/type/texture_dimension.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::resolver {
namespace {

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
    GlobalVar("t", ty.sampled_texture(type::TextureDimension::k3d, ty.f32()), Group(0_a),
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
    GlobalVar("t", ty.sampled_texture(type::TextureDimension::k3d, ty.f32()), Group(0_a),
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
    auto* inner = Structure("Inner", utils::Vector{Member("arr", ty.array<i32, 4>())});
    auto* buf = Structure("S", utils::Vector{Member("inner", ty.Of(inner))});
    auto* storage = GlobalVar("s", ty.Of(buf), builtin::AddressSpace::kStorage,
                              builtin::Access::kReadWrite, Binding(0_a), Group(0_a));

    auto* expr = IndexAccessor(MemberAccessor(MemberAccessor(storage, "inner"), "arr"), 2_i);
    auto* ptr = Let(Source{{12, 34}}, "p", ty.pointer<i32>(builtin::AddressSpace::kStorage),
                    AddressOf(expr));

    WrapInFunction(ptr);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: cannot initialize let of type "
              "'ptr<storage, i32, read>' with value of type "
              "'ptr<storage, i32, read_write>'");
}

}  // namespace
}  // namespace tint::resolver
