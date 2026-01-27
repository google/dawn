// Copyright 2026 The Dawn & Tint Authors
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

#include "src/tint/lang/wgsl/resolver/resolver.h"
#include "src/tint/lang/wgsl/resolver/resolver_helper_test.h"
#include "src/tint/lang/wgsl/sem/builtin_fn.h"
#include "src/tint/lang/wgsl/sem/value_constructor.h"

#include "gmock/gmock.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::resolver {
namespace {

using ResolverBufferTest = ResolverTest;

TEST_F(ResolverBufferTest, UnsizedBuffer) {
    auto* alias = Alias("b", ty.buffer());

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* b = TypeOf(alias)->UnwrapRef()->As<core::type::Buffer>();
    ASSERT_NE(b, nullptr);
    EXPECT_EQ(b->Size(), 0u);
}

TEST_F(ResolverBufferTest, SizedBuffer) {
    auto* alias = Alias("b", ty.buffer(16_u));

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* b = TypeOf(alias)->UnwrapRef()->As<core::type::Buffer>();
    ASSERT_NE(b, nullptr);
    EXPECT_EQ(b->Size(), 16u);
}

TEST_F(ResolverBufferTest, SizedBufferNegative) {
    Alias("b", ty.AsType("buffer", -1_i));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(error: buffer size (-1) must be greater than 0)");
}

TEST_F(ResolverBufferTest, SizedBufferZero) {
    Alias("b", ty.AsType("buffer", 0_i));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(error: buffer size (0) must be greater than 0)");
}

TEST_F(ResolverBufferTest, EquivalentTypes) {
    auto* a1 = Alias("b1", ty.buffer());
    auto* a2 = Alias("b2", ty.buffer());
    auto* a3 = Alias("b3", ty.buffer(4_i));
    auto* a4 = Alias("b4", ty.buffer(16_u));
    auto* a5 = Alias("b5", ty.buffer(16_i));

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* b1 = TypeOf(a1)->UnwrapRef()->As<core::type::Buffer>();
    auto* b2 = TypeOf(a2)->UnwrapRef()->As<core::type::Buffer>();
    auto* b3 = TypeOf(a3)->UnwrapRef()->As<core::type::Buffer>();
    auto* b4 = TypeOf(a4)->UnwrapRef()->As<core::type::Buffer>();
    auto* b5 = TypeOf(a5)->UnwrapRef()->As<core::type::Buffer>();

    EXPECT_EQ(b1, b2);
    EXPECT_NE(b1, b3);
    EXPECT_EQ(b4, b5);
}

TEST_F(ResolverBufferTest, Struct) {
    Structure("S", Vector{Member("a", ty.buffer(16_u))});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(error: buffer<16> cannot be used as the type of a structure member)");
}

TEST_F(ResolverBufferTest, Array) {
    Alias("b", ty.array(ty.buffer(), 4_u));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(error: buffer cannot be used as an element type of an array)");
}

TEST_F(ResolverBufferTest, Pointer_Function) {
    Alias("p", ty.ptr<function>(ty.buffer()));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(error: 'buffer' variables must have 'storage', 'uniform', or 'workgroup' address space)");
}

TEST_F(ResolverBufferTest, Pointer_Private) {
    Alias("p", ty.ptr<private_>(ty.buffer()));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(error: 'buffer' variables must have 'storage', 'uniform', or 'workgroup' address space)");
}

TEST_F(ResolverBufferTest, Pointer_Storage) {
    Alias("p", ty.ptr<storage>(ty.buffer()));

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBufferTest, Pointer_Uniform) {
    Alias("p", ty.ptr<uniform>(ty.buffer(16_u)));

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBufferTest, Pointer_Workgroup) {
    Alias("p", ty.ptr<workgroup>(ty.buffer(16_u)));

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBufferTest, Var_Function) {
    Func("foo", Empty, ty.void_(),
         Vector{
             Decl(Var("v", function, ty.buffer(16_u))),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(error: buffer types cannot be declared in the 'function' address space
error: function-scope 'var' must have a constructible type)");
}

TEST_F(ResolverBufferTest, Var_Private) {
    GlobalVar("v", private_, ty.buffer(16_u));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(error: buffer types cannot be declared in the 'private' address space
error: 'buffer' variables must have 'storage', 'uniform', or 'workgroup' address space)");
}

TEST_F(ResolverBufferTest, Var_Storage) {
    GlobalVar("v", storage, ty.buffer(), Group(0_a), Binding(0_a));

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBufferTest, Var_Storage_Override) {
    Override("o", Expr(4_i));
    GlobalVar("v", storage, ty.AsType("buffer", Expr(Ident("o"))), Group(0_a), Binding(0_a));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(error: buffer type must not be sized with an override-expression in 'storage' address space)");
}

TEST_F(ResolverBufferTest, Var_Uniform) {
    GlobalVar("v", uniform, ty.buffer(16_u), Group(0_a), Binding(0_a));

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBufferTest, Var_Uniform_Unsized) {
    GlobalVar("v", uniform, ty.buffer(), Group(0_a), Binding(0_a));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(error: buffer type must be sized in 'uniform' address space)");
}

TEST_F(ResolverBufferTest, Var_Uniform_Override) {
    Override("o", Expr(4_i));
    GlobalVar("v", uniform, ty.AsType("buffer", Expr(Ident("o"))), Group(0_a), Binding(0_a));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(error: buffer type must not be sized with an override-expression in 'uniform' address space)");
}

TEST_F(ResolverBufferTest, Var_Workgroup) {
    GlobalVar("v", workgroup, ty.buffer(16_u));

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBufferTest, Var_Workgroup_Override) {
    Override("o", Expr(4_i));
    GlobalVar("v", workgroup, ty.AsType("buffer", Expr(Ident("o"))));

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBufferTest, Var_Workgroup_Unsized) {
    GlobalVar("v", workgroup, ty.buffer(), Group(0_a), Binding(0_a));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(error: buffer type must be sized in 'workgroup' address space)");
}

TEST_F(ResolverBufferTest, FunctionParameter) {
    Func("foo", Vector{Param("b", ty.buffer())}, ty.void_(), Empty);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(error: buffer types cannot be declared in the 'undefined' address space
error: type of function parameter cannot be buffer)");
}

TEST_F(ResolverBufferTest, FunctionParameter_Pointer) {
    Func("foo", Vector{Param("b", ty.ptr<storage>(ty.buffer()))}, ty.void_(), Empty);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBufferTest, FunctionParameter_SizedMatchesUnsized) {
    Func("foo", Vector{Param("p", ty.ptr<storage>(ty.buffer()))}, ty.void_(), Empty);
    auto* v = GlobalVar("v", storage, ty.buffer(16_u), Group(0_a), Binding(0_a));
    Func("bar", Empty, ty.void_(), Vector{CallStmt(Call(Ident("foo"), AddressOf(v)))});

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBufferTest, FunctionParameter_LargerSizedMatchesSmallerSized) {
    Func("foo", Vector{Param("p", ty.ptr<storage>(ty.buffer(8_i)))}, ty.void_(), Empty);
    auto* v = GlobalVar("v", storage, ty.buffer(16_u), Group(0_a), Binding(0_a));
    Func("bar", Empty, ty.void_(), Vector{CallStmt(Call(Ident("foo"), AddressOf(v)))});

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBufferTest, FunctionParameter_UnsizedDoesNotMatchSized) {
    Func("foo", Vector{Param("p", ty.ptr<storage>(ty.buffer(16_u)))}, ty.void_(), Empty);
    auto* v = GlobalVar("v", storage, ty.buffer(), Group(0_a), Binding(0_a));
    Func("bar", Empty, ty.void_(), Vector{CallStmt(Call(Ident("foo"), AddressOf(v)))});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(error: type mismatch for argument 1 in call to 'foo', expected 'ptr<storage, buffer<16>, read>', got 'ptr<storage, buffer, read>')");
}

using ResolverBufferViewTest = ResolverTest;

TEST_F(ResolverBufferViewTest, Storage_Unsized) {
    auto* gv = GlobalVar("v", storage, ty.buffer(), Group(0_a), Binding(0_a));
    Func(
        "foo", Empty, ty.void_(),
        Vector{Assign(Phony(), Call(Ident("bufferView", ty.array(ty.u32())), AddressOf(gv), 0_u))});

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBufferViewTest, Storage_Sized) {
    auto* gv = GlobalVar("v", storage, ty.buffer(16_a), Group(0_a), Binding(0_a));
    Func(
        "foo", Empty, ty.void_(),
        Vector{Assign(Phony(), Call(Ident("bufferView", ty.array(ty.u32())), AddressOf(gv), 0_u))});

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBufferViewTest, Uniform_Sized) {
    auto* gv = GlobalVar("v", uniform, ty.buffer(64_a), Group(0_a), Binding(0_a));
    Func("foo", Empty, ty.void_(),
         Vector{Assign(Phony(),
                       Call(Ident("bufferView", ty.array(ty.u32(), 4_u)), AddressOf(gv), 0_u))});

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBufferViewTest, Workgroup_Sized) {
    auto* gv = GlobalVar("v", workgroup, ty.buffer(64_a));
    Func("foo", Empty, ty.void_(),
         Vector{Assign(Phony(),
                       Call(Ident("bufferView", ty.array(ty.u32(), 4_u)), AddressOf(gv), 0_u))});

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBufferViewTest, ReturnTypeContainsAtomic) {
    Structure("S", Vector{Member("a", ty.array(ty.atomic(ty.u32()), 4_u))});
    auto* gv = GlobalVar("v", storage, ty.buffer(), Group(0_a), Binding(0_a));
    Func("foo", Empty, ty.void_(),
         Vector{Assign(Phony(), Call(Ident("bufferView", Expr(Ident("S"))), AddressOf(gv), 0_u))});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(error: return type of bufferView cannot contain an atomic type)");
}

TEST_F(ResolverBufferViewTest, Offset_Unsigned_TooSmall) {
    auto* gv = GlobalVar("v", storage, ty.buffer(16_a), Group(0_a), Binding(0_a));
    Func("foo", Empty, ty.void_(),
         Vector{Assign(Phony(), Call(Ident("bufferView", ty.u32()), AddressOf(gv), 20_u))});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(error: the offset argument of bufferView plus the size of the return type must be smaller than the buffer size)");
}

TEST_F(ResolverBufferViewTest, Offset_Unsigned_Unaligned) {
    auto* gv = GlobalVar("v", storage, ty.buffer(16_a), Group(0_a), Binding(0_a));
    Func("foo", Empty, ty.void_(),
         Vector{Assign(Phony(), Call(Ident("bufferView", ty.u32()), AddressOf(gv), 1_u))});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(error: the offset argument of bufferView must evenly divide the alignment of the return type (4))");
}

TEST_F(ResolverBufferViewTest, Offset_Signed_TooSmall) {
    auto* gv = GlobalVar("v", storage, ty.buffer(16_a), Group(0_a), Binding(0_a));
    Func("foo", Empty, ty.void_(),
         Vector{Assign(Phony(), Call(Ident("bufferView", ty.u32()), AddressOf(gv), 20_i))});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(error: the offset argument of bufferView plus the size of the return type must be smaller than the buffer size)");
}

TEST_F(ResolverBufferViewTest, Offset_Signed_Unaligned) {
    auto* gv = GlobalVar("v", storage, ty.buffer(16_a), Group(0_a), Binding(0_a));
    Func("foo", Empty, ty.void_(),
         Vector{Assign(Phony(), Call(Ident("bufferView", ty.u32()), AddressOf(gv), 1_a))});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(error: the offset argument of bufferView must evenly divide the alignment of the return type (4))");
}

TEST_F(ResolverBufferViewTest, Offset_Signed_Negative) {
    auto* gv = GlobalVar("v", storage, ty.buffer(16_a), Group(0_a), Binding(0_a));
    Func("foo", Empty, ty.void_(),
         Vector{Assign(Phony(), Call(Ident("bufferView", ty.u32()), AddressOf(gv), -1_i))});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(error: the offset argument of bufferView must be non-negative)");
}

TEST_F(ResolverBufferViewTest, Return_Buffer_Unsized) {
    auto* gv = GlobalVar("v", storage, ty.buffer(), Group(0_a), Binding(0_a));
    Func("foo", Empty, ty.void_(),
         Vector{Assign(Phony(), Call(Ident("bufferView", ty.buffer()), AddressOf(gv), 0_i))});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(error: return type of bufferView cannot be a buffer)");
}

TEST_F(ResolverBufferViewTest, Return_Buffer_Sized) {
    auto* gv = GlobalVar("v", storage, ty.buffer(), Group(0_a), Binding(0_a));
    Func("foo", Empty, ty.void_(),
         Vector{Assign(Phony(), Call(Ident("bufferView", ty.buffer(16_u)), AddressOf(gv), 0_i))});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(error: return type of bufferView cannot be a buffer)");
}

TEST_F(ResolverBufferViewTest, Return_NonHostShareable) {
    Override("o", ty.u32());
    auto* gv = GlobalVar("v", storage, ty.buffer(), Group(0_a), Binding(0_a));
    auto array = ty.array(ty.u32(), Expr(Ident("o")));
    Func("foo", Empty, ty.void_(),
         Vector{Assign(Phony(), Call(Ident("bufferView", array), AddressOf(gv), 0_i))});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(error: override-sized arrays can only be used in the <workgroup> address space
note:  while instantiating bufferView)");
}

}  // namespace
}  // namespace tint::resolver
