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
#include "src/tint/resolver/resolver.h"
#include "src/tint/resolver/resolver_test_helper.h"
#include "src/tint/type/texture_dimension.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::resolver {
namespace {

struct ResolverVariableValidationTest : public resolver::TestHelper, public testing::Test {};

TEST_F(ResolverVariableValidationTest, VarNoInitializerNoType) {
    // var a;
    WrapInFunction(Var(Source{{12, 34}}, "a"));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: var declaration requires a type or initializer");
}

TEST_F(ResolverVariableValidationTest, GlobalVarNoInitializerNoType) {
    // var a;
    GlobalVar(Source{{12, 34}}, "a");

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: var declaration requires a type or initializer");
}

TEST_F(ResolverVariableValidationTest, VarInitializerNoReturnValueBuiltin) {
    // fn f() { var a = storageBarrier(); }
    auto* NoReturnValueBuiltin = Call(Source{{12, 34}}, "storageBarrier");
    WrapInFunction(Var("a", NoReturnValueBuiltin));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: builtin 'storageBarrier' does not return a value");
}

TEST_F(ResolverVariableValidationTest, GlobalVarInitializerNoReturnValueBuiltin) {
    // var a = storageBarrier();
    auto* NoReturnValueBuiltin = Call(Source{{12, 34}}, "storageBarrier");
    GlobalVar("a", NoReturnValueBuiltin);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: builtin 'storageBarrier' does not return a value");
}

TEST_F(ResolverVariableValidationTest, GlobalVarNoAddressSpace) {
    // var a : i32;
    GlobalVar(Source{{12, 34}}, "a", ty.i32());

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: module-scope 'var' declarations that are not of texture or sampler types must provide an address space)");
}

TEST_F(ResolverVariableValidationTest, GlobalVarWithInitializerNoAddressSpace) {
    // var a = 1;
    GlobalVar(Source{{12, 34}}, "a", Expr(1_a));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: module-scope 'var' declarations that are not of texture or sampler types must provide an address space)");
}

TEST_F(ResolverVariableValidationTest, GlobalVarUsedAtModuleScope) {
    // var<private> a : i32;
    // var<private> b : i32 = a;
    GlobalVar(Source{{12, 34}}, "a", ty.i32(), builtin::AddressSpace::kPrivate);
    GlobalVar("b", ty.i32(), builtin::AddressSpace::kPrivate, Expr(Source{{56, 78}}, "a"));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(56:78 error: var 'a' cannot be referenced at module-scope
12:34 note: var 'a' declared here)");
}

TEST_F(ResolverVariableValidationTest, OverrideNoInitializerNoType) {
    // override a;
    Override(Source{{12, 34}}, "a");

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: override declaration requires a type or initializer");
}

TEST_F(ResolverVariableValidationTest, OverrideExceedsIDLimit_LastUnreserved) {
    // override o0 : i32;
    // override o1 : i32;
    // ...
    // override bang : i32;
    constexpr size_t kLimit = std::numeric_limits<decltype(OverrideId::value)>::max();
    for (size_t i = 0; i <= kLimit; i++) {
        Override("o" + std::to_string(i), ty.i32());
    }
    Override(Source{{12, 34}}, "bang", ty.i32());

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: number of 'override' variables exceeded limit of 65535");
}

TEST_F(ResolverVariableValidationTest, OverrideExceedsIDLimit_LastReserved) {
    // override o0 : i32;
    // override o1 : i32;
    // ...
    // @id(N) override oN : i32;
    constexpr size_t kLimit = std::numeric_limits<decltype(OverrideId::value)>::max();
    Override("reserved", ty.i32(), Id(AInt(kLimit)));
    for (size_t i = 0; i < kLimit; i++) {
        Override("o" + std::to_string(i), ty.i32());
    }
    Override(Source{{12, 34}}, "bang", ty.i32());

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: number of 'override' variables exceeded limit of 65535");
}

TEST_F(ResolverVariableValidationTest, VarTypeNotConstructible) {
    // var i : i32;
    // var p : pointer<function, i32> = &v;
    auto* i = Var("i", ty.i32());
    auto* p = Var("a", ty.pointer<i32>(Source{{56, 78}}, builtin::AddressSpace::kFunction),
                  builtin::AddressSpace::kUndefined, AddressOf(Source{{12, 34}}, "i"));
    WrapInFunction(i, p);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "56:78 error: function-scope 'var' must have a constructible type");
}

TEST_F(ResolverVariableValidationTest, LetTypeNotConstructible) {
    // @group(0) @binding(0) var t1 : texture_2d<f32>;
    // let t2 : t1;
    auto* t1 = GlobalVar("t1", ty.sampled_texture(type::TextureDimension::k2d, ty.f32()),
                         Group(0_a), Binding(0_a));
    auto* t2 = Let(Source{{56, 78}}, "t2", Expr(t1));
    WrapInFunction(t2);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "56:78 error: texture_2d<f32> cannot be used as the type of a 'let'");
}

TEST_F(ResolverVariableValidationTest, OverrideExplicitTypeNotScalar) {
    // override o : vec3<f32>;
    Override(Source{{56, 78}}, "o", ty.vec3<f32>());

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "56:78 error: vec3<f32> cannot be used as the type of a 'override'");
}

TEST_F(ResolverVariableValidationTest, OverrideInferedTypeNotScalar) {
    // override o = vec3(1.0f);
    Override(Source{{56, 78}}, "o", vec3<f32>(1.0_f));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "56:78 error: vec3<f32> cannot be used as the type of a 'override'");
}

TEST_F(ResolverVariableValidationTest, ConstInitializerWrongType) {
    // const c : i32 = 2u
    WrapInFunction(Const(Source{{3, 3}}, "c", ty.i32(), Expr(2_u)));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(3:3 error: cannot initialize const of type 'i32' with value of type 'u32')");
}

TEST_F(ResolverVariableValidationTest, LetInitializerWrongType) {
    // var v : i32 = 2u
    WrapInFunction(Let(Source{{3, 3}}, "v", ty.i32(), Expr(2_u)));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(3:3 error: cannot initialize let of type 'i32' with value of type 'u32')");
}

TEST_F(ResolverVariableValidationTest, VarInitializerWrongType) {
    // var v : i32 = 2u
    WrapInFunction(Var(Source{{3, 3}}, "v", ty.i32(), Expr(2_u)));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(3:3 error: cannot initialize var of type 'i32' with value of type 'u32')");
}

TEST_F(ResolverVariableValidationTest, ConstInitializerWrongTypeViaAlias) {
    auto* a = Alias("I32", ty.i32());
    WrapInFunction(Const(Source{{3, 3}}, "v", ty.Of(a), Expr(2_u)));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(3:3 error: cannot initialize const of type 'i32' with value of type 'u32')");
}

TEST_F(ResolverVariableValidationTest, LetInitializerWrongTypeViaAlias) {
    auto* a = Alias("I32", ty.i32());
    WrapInFunction(Let(Source{{3, 3}}, "v", ty.Of(a), Expr(2_u)));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(3:3 error: cannot initialize let of type 'i32' with value of type 'u32')");
}

TEST_F(ResolverVariableValidationTest, VarInitializerWrongTypeViaAlias) {
    auto* a = Alias("I32", ty.i32());
    WrapInFunction(Var(Source{{3, 3}}, "v", ty.Of(a), Expr(2_u)));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(3:3 error: cannot initialize var of type 'i32' with value of type 'u32')");
}

TEST_F(ResolverVariableValidationTest, LetOfPtrConstructedWithRef) {
    // var a : f32;
    // let b : ptr<function,f32> = a;
    const auto priv = builtin::AddressSpace::kFunction;
    auto* var_a = Var("a", ty.f32(), priv);
    auto* var_b = Let(Source{{12, 34}}, "b", ty.pointer<f32>(priv), Expr("a"));
    WrapInFunction(var_a, var_b);

    ASSERT_FALSE(r()->Resolve());

    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: cannot initialize let of type 'ptr<function, f32, read_write>' with value of type 'f32')");
}

TEST_F(ResolverVariableValidationTest, LocalLetRedeclared) {
    // let l : f32 = 1.;
    // let l : i32 = 0;
    auto* l1 = Let("l", ty.f32(), Expr(1_f));
    auto* l2 = Let(Source{{12, 34}}, "l", ty.i32(), Expr(0_i));
    WrapInFunction(l1, l2);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: redeclaration of 'l'\nnote: 'l' previously declared here");
}

TEST_F(ResolverVariableValidationTest, GlobalVarRedeclaredAsLocal) {
    // var v : f32 = 2.1;
    // fn my_func() {
    //   var v : f32 = 2.0;
    //   return 0;
    // }

    GlobalVar("v", ty.f32(), builtin::AddressSpace::kPrivate, Expr(2.1_f));

    WrapInFunction(Var(Source{{12, 34}}, "v", ty.f32(), Expr(2_f)));

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverVariableValidationTest, VarRedeclaredInInnerBlock) {
    // {
    //  var v : f32;
    //  { var v : f32; }
    // }
    auto* var_outer = Var("v", ty.f32());
    auto* var_inner = Var(Source{{12, 34}}, "v", ty.f32());
    auto* inner = Block(Decl(var_inner));
    auto* outer_body = Block(Decl(var_outer), inner);

    WrapInFunction(outer_body);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverVariableValidationTest, VarRedeclaredInIfBlock) {
    // {
    //   var v : f32 = 3.14;
    //   if (true) { var v : f32 = 2.0; }
    // }
    auto* var_a_float = Var("v", ty.f32(), Expr(3.1_f));

    auto* var = Var(Source{{12, 34}}, "v", ty.f32(), Expr(2_f));

    auto* cond = Expr(true);
    auto* body = Block(Decl(var));

    auto* outer_body = Block(Decl(var_a_float), If(cond, body));

    WrapInFunction(outer_body);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverVariableValidationTest, InferredPtrStorageAccessMismatch) {
    // struct Inner {
    //    arr: array<i32, 4>;
    // }
    // struct S {
    //    inner: Inner;
    // }
    // @group(0) @binding(0) var<storage> s : S;
    // fn f() {
    //   let p : pointer<storage, i32, read_write> = &s.inner.arr[2i];
    // }
    auto* inner = Structure("Inner", utils::Vector{
                                         Member("arr", ty.array<i32, 4>()),
                                     });
    auto* buf = Structure("S", utils::Vector{
                                   Member("inner", ty.Of(inner)),
                               });
    auto* storage =
        GlobalVar("s", ty.Of(buf), builtin::AddressSpace::kStorage, Binding(0_a), Group(0_a));

    auto* expr = IndexAccessor(MemberAccessor(MemberAccessor(storage, "inner"), "arr"), 2_i);
    auto* ptr = Let(Source{{12, 34}}, "p",
                    ty.pointer<i32>(builtin::AddressSpace::kStorage, builtin::Access::kReadWrite),
                    AddressOf(expr));

    WrapInFunction(ptr);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: cannot initialize let of type "
              "'ptr<storage, i32, read_write>' with value of type "
              "'ptr<storage, i32, read>'");
}

TEST_F(ResolverVariableValidationTest, NonConstructibleType_Atomic) {
    auto* v = Var("v", ty.atomic(Source{{12, 34}}, ty.i32()));
    WrapInFunction(v);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: function-scope 'var' must have a constructible type");
}

TEST_F(ResolverVariableValidationTest, NonConstructibleType_RuntimeArray) {
    auto* s = Structure("S", utils::Vector{
                                 Member(Source{{12, 34}}, "m", ty.array<i32>()),
                             });
    auto* v = Var(Source{{56, 78}}, "v", ty.Of(s));
    WrapInFunction(v);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(error: runtime-sized arrays can only be used in the <storage> address space
12:34 note: while analyzing structure member S.m
56:78 note: while instantiating 'var' v)");
}

TEST_F(ResolverVariableValidationTest, NonConstructibleType_Struct_WithAtomic) {
    auto* s = Structure("S", utils::Vector{
                                 Member("m", ty.atomic(ty.i32())),
                             });
    auto* v = Var("v", ty.Of(s));
    WrapInFunction(v);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "error: function-scope 'var' must have a constructible type");
}

TEST_F(ResolverVariableValidationTest, NonConstructibleType_InferredType) {
    // @group(0) @binding(0) var s : sampler;
    // fn foo() {
    //   var v = s;
    // }
    GlobalVar("s", ty.sampler(type::SamplerKind::kSampler), Group(0_a), Binding(0_a));
    auto* v = Var(Source{{12, 34}}, "v", Expr("s"));
    WrapInFunction(v);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: function-scope 'var' must have a constructible type");
}

TEST_F(ResolverVariableValidationTest, InvalidAddressSpaceForInitializer) {
    // var<workgroup> v : f32 = 1.23;
    GlobalVar(Source{{12, 34}}, "v", ty.f32(), builtin::AddressSpace::kWorkgroup, Expr(1.23_f));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: var of address space 'workgroup' cannot have "
              "an initializer. var initializers are only supported for the "
              "address spaces 'private' and 'function'");
}

TEST_F(ResolverVariableValidationTest, VectorConstNoType) {
    // const a vec3 = vec3<f32>();
    WrapInFunction(Const("a", ty.vec3<Infer>(Source{{12, 34}}), vec3<f32>()));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: expected '<' for 'vec3'");
}

TEST_F(ResolverVariableValidationTest, VectorLetNoType) {
    // let a : vec3 = vec3<f32>();
    WrapInFunction(Let("a", ty.vec3<Infer>(Source{{12, 34}}), vec3<f32>()));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: expected '<' for 'vec3'");
}

TEST_F(ResolverVariableValidationTest, VectorVarNoType) {
    // var a : vec3;
    WrapInFunction(Var("a", ty.vec3<Infer>(Source{{12, 34}})));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: expected '<' for 'vec3'");
}

TEST_F(ResolverVariableValidationTest, MatrixConstNoType) {
    // const a : mat3x3 = mat3x3<f32>();
    WrapInFunction(Const("a", ty.mat3x3<Infer>(Source{{12, 34}}), mat3x3<f32>()));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: expected '<' for 'mat3x3'");
}

TEST_F(ResolverVariableValidationTest, MatrixLetNoType) {
    // let a : mat3x3 = mat3x3<f32>();
    WrapInFunction(Let("a", ty.mat3x3<Infer>(Source{{12, 34}}), mat3x3<f32>()));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: expected '<' for 'mat3x3'");
}

TEST_F(ResolverVariableValidationTest, MatrixVarNoType) {
    // var a : mat3x3;
    WrapInFunction(Var("a", ty.mat3x3<Infer>(Source{{12, 34}})));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: expected '<' for 'mat3x3'");
}

TEST_F(ResolverVariableValidationTest, GlobalConstWithRuntimeExpression) {
    GlobalConst("c", Call(Source{{12, 34}}, "dpdx", 1._a));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: const initializer requires a const-expression, but expression is a runtime-expression)");
}

TEST_F(ResolverVariableValidationTest, ConstInitWithVar) {
    auto* v = Var("v", Expr(1_i));
    auto* c = Const("c", Expr(Source{{12, 34}}, v));
    WrapInFunction(v, Decl(Source{{56, 78}}, c));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: const initializer requires a const-expression, but expression is a runtime-expression
56:78 note: consider changing 'const' to 'let')");
}

TEST_F(ResolverVariableValidationTest, ConstInitWithOverride) {
    auto* o = Override("v", Expr(1_i));
    auto* c = Const("c", Expr(Source{{12, 34}}, o));
    WrapInFunction(Decl(Source{{56, 78}}, c));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: const initializer requires a const-expression, but expression is an override-expression
56:78 note: consider changing 'const' to 'let')");
}

TEST_F(ResolverVariableValidationTest, ConstInitWithLet) {
    auto* l = Let("v", Expr(1_i));
    auto* c = Const("c", Expr(Source{{12, 34}}, l));
    WrapInFunction(l, Decl(Source{{56, 78}}, c));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: const initializer requires a const-expression, but expression is a runtime-expression
56:78 note: consider changing 'const' to 'let')");
}

TEST_F(ResolverVariableValidationTest, ConstInitWithRuntimeExpr) {
    // const c = clamp(2, dpdx(0.5), 3);
    auto* c = Const("c", Call("clamp", 2_a, Call(Source{{12, 34}}, "dpdx", 0.5_a), 3_a));
    WrapInFunction(Decl(Source{{56, 78}}, c));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: const initializer requires a const-expression, but expression is a runtime-expression
56:78 note: consider changing 'const' to 'let')");
}

TEST_F(ResolverVariableValidationTest, ConstInitWithOverrideExpr) {
    auto* o = Override("v", Expr(1_i));
    auto* c = Const("c", Add(10_a, Expr(Source{{12, 34}}, o)));
    WrapInFunction(Decl(Source{{56, 78}}, c));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: const initializer requires a const-expression, but expression is an override-expression
56:78 note: consider changing 'const' to 'let')");
}

TEST_F(ResolverVariableValidationTest, GlobalVariable_PushConstantWithInitializer) {
    // enable chromium_experimental_push_constant;
    // var<push_constant> a : u32 = 0u;
    Enable(builtin::Extension::kChromiumExperimentalPushConstant);
    GlobalVar(Source{{1u, 2u}}, "a", ty.u32(), builtin::AddressSpace::kPushConstant,
              Expr(Source{{3u, 4u}}, u32(0)));

    ASSERT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(1:2 error: var of address space 'push_constant' cannot have an initializer. var initializers are only supported for the address spaces 'private' and 'function')");
}

}  // namespace
}  // namespace tint::resolver
