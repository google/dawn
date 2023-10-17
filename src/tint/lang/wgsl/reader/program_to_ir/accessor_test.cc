// Copyright 2023 The Dawn & Tint Authors
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
#include "src/tint/lang/core/constant/scalar.h"
#include "src/tint/lang/core/ir/block.h"
#include "src/tint/lang/core/ir/constant.h"
#include "src/tint/lang/core/ir/var.h"
#include "src/tint/lang/wgsl/ast/case_selector.h"
#include "src/tint/lang/wgsl/ast/int_literal_expression.h"
#include "src/tint/lang/wgsl/helpers/ir_program_test.h"

namespace tint::wgsl::reader {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

using ProgramToIRAccessorTest = helpers::IRProgramTest;

TEST_F(ProgramToIRAccessorTest, Accessor_Var_ArraySingleIndex) {
    // var a: array<u32, 3>
    // let b = a[2]

    auto* a = Var("a", ty.array<u32, 3>(), core::AddressSpace::kFunction);
    auto* expr = Decl(Let("b", IndexAccessor(a, 2_u)));
    WrapInFunction(Decl(a), expr);

    auto m = Build();
    ASSERT_TRUE(m) << m;

    EXPECT_EQ(Disassemble(m.Get()),
              R"(%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b1 {
  %b1 = block {
    %a:ptr<function, array<u32, 3>, read_write> = var
    %3:ptr<function, u32, read_write> = access %a, 2u
    %b:u32 = load %3
    ret
  }
}
)");
}

TEST_F(ProgramToIRAccessorTest, Accessor_Multiple) {
    // let a: vec4<u32> = vec4();
    // let b = a[2]
    // let c = a[1]

    auto* a = Let("a", ty.vec3<u32>(), vec(ty.u32(), 3));
    auto* expr = Decl(Let("b", IndexAccessor(a, 2_u)));
    auto* expr2 = Decl(Let("c", IndexAccessor(a, 1_u)));

    WrapInFunction(Decl(a), expr, expr2);

    auto m = Build();
    ASSERT_TRUE(m) << m;

    EXPECT_EQ(Disassemble(m.Get()),
              R"(%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b1 {
  %b1 = block {
    %a:vec3<u32> = let vec3<u32>(0u)
    %b:u32 = access %a, 2u
    %c:u32 = access %a, 1u
    ret
  }
}
)");
}

TEST_F(ProgramToIRAccessorTest, Accessor_Var_VectorSingleIndex) {
    // var a: vec3<u32>
    // let b = a[2]

    auto* a = Var("a", ty.vec3<u32>(), core::AddressSpace::kFunction);
    auto* expr = Decl(Let("b", IndexAccessor(a, 2_u)));
    WrapInFunction(Decl(a), expr);

    auto m = Build();
    ASSERT_TRUE(m) << m;

    EXPECT_EQ(Disassemble(m.Get()),
              R"(%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b1 {
  %b1 = block {
    %a:ptr<function, vec3<u32>, read_write> = var
    %b:u32 = load_vector_element %a, 2u
    ret
  }
}
)");
}

TEST_F(ProgramToIRAccessorTest, Accessor_Var_ArraysMultiIndex) {
    // var a: array<array<f32, 4>, 3>
    // let b = a[2][3]

    auto* a = Var("a", ty.array<array<f32, 4>, 3>(), core::AddressSpace::kFunction);
    auto* expr = Decl(Let("b", IndexAccessor(IndexAccessor(a, 2_u), 3_u)));
    WrapInFunction(Decl(a), expr);

    auto m = Build();
    ASSERT_TRUE(m) << m;

    EXPECT_EQ(Disassemble(m.Get()),
              R"(%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b1 {
  %b1 = block {
    %a:ptr<function, array<array<f32, 4>, 3>, read_write> = var
    %3:ptr<function, f32, read_write> = access %a, 2u, 3u
    %b:f32 = load %3
    ret
  }
}
)");
}

TEST_F(ProgramToIRAccessorTest, Accessor_Var_MatrixMultiIndex) {
    // var a: mat3x4<f32>
    // let b = a[2][3]

    auto* a = Var("a", ty.mat3x4<f32>(), core::AddressSpace::kFunction);
    auto* expr = Decl(Let("b", IndexAccessor(IndexAccessor(a, 2_u), 3_u)));
    WrapInFunction(Decl(a), expr);

    auto m = Build();
    ASSERT_TRUE(m) << m;

    EXPECT_EQ(Disassemble(m.Get()),
              R"(%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b1 {
  %b1 = block {
    %a:ptr<function, mat3x4<f32>, read_write> = var
    %3:ptr<function, vec4<f32>, read_write> = access %a, 2u
    %b:f32 = load_vector_element %3, 3u
    ret
  }
}
)");
}

TEST_F(ProgramToIRAccessorTest, Accessor_Var_SingleMember) {
    // struct MyStruct { foo: i32 }
    // var a: MyStruct;
    // let b = a.foo

    auto* s = Structure("MyStruct", Vector{
                                        Member("foo", ty.i32()),
                                    });
    auto* a = Var("a", ty.Of(s), core::AddressSpace::kFunction);
    auto* expr = Decl(Let("b", MemberAccessor(a, "foo")));
    WrapInFunction(Decl(a), expr);

    auto m = Build();
    ASSERT_TRUE(m) << m;

    EXPECT_EQ(Disassemble(m.Get()),
              R"(MyStruct = struct @align(4) {
  foo:i32 @offset(0)
}

%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b1 {
  %b1 = block {
    %a:ptr<function, MyStruct, read_write> = var
    %3:ptr<function, i32, read_write> = access %a, 0u
    %b:i32 = load %3
    ret
  }
}
)");
}

TEST_F(ProgramToIRAccessorTest, Accessor_Var_MultiMember) {
    // struct Inner { bar: f32 }
    // struct Outer { a: i32, foo: Inner }
    // var a: Outer;
    // let b = a.foo.bar

    auto* inner = Structure("Inner", Vector{
                                         Member("bar", ty.f32()),
                                     });
    auto* outer = Structure("Outer", Vector{
                                         Member("a", ty.i32()),
                                         Member("foo", ty.Of(inner)),
                                     });
    auto* a = Var("a", ty.Of(outer), core::AddressSpace::kFunction);
    auto* expr = Decl(Let("b", MemberAccessor(MemberAccessor(a, "foo"), "bar")));
    WrapInFunction(Decl(a), expr);

    auto m = Build();
    ASSERT_TRUE(m) << m;

    EXPECT_EQ(Disassemble(m.Get()),
              R"(Inner = struct @align(4) {
  bar:f32 @offset(0)
}

Outer = struct @align(4) {
  a:i32 @offset(0)
  foo:Inner @offset(4)
}

%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b1 {
  %b1 = block {
    %a:ptr<function, Outer, read_write> = var
    %3:ptr<function, f32, read_write> = access %a, 1u, 0u
    %b:f32 = load %3
    ret
  }
}
)");
}

TEST_F(ProgramToIRAccessorTest, Accessor_Var_Mixed) {
    // struct Inner { b: i32, c: f32, bar: vec4<f32> }
    // struct Outer { a: i32, foo: array<Inner, 4> }
    // var a: array<Outer, 4>
    // let b = a[0].foo[1].bar

    auto* inner = Structure("Inner", Vector{
                                         Member("b", ty.i32()),
                                         Member("c", ty.f32()),
                                         Member("bar", ty.vec4<f32>()),
                                     });
    auto* outer = Structure("Outer", Vector{
                                         Member("a", ty.i32()),
                                         Member("foo", ty.array(ty.Of(inner), 4_u)),
                                     });
    auto* a = Var("a", ty.array(ty.Of(outer), 4_u), core::AddressSpace::kFunction);
    auto* expr = Decl(Let(
        "b",
        MemberAccessor(IndexAccessor(MemberAccessor(IndexAccessor(a, 0_u), "foo"), 1_u), "bar")));
    WrapInFunction(Decl(a), expr);

    auto m = Build();
    ASSERT_TRUE(m) << m;

    EXPECT_EQ(Disassemble(m.Get()),
              R"(Inner = struct @align(16) {
  b:i32 @offset(0)
  c:f32 @offset(4)
  bar:vec4<f32> @offset(16)
}

Outer = struct @align(16) {
  a:i32 @offset(0)
  foo:array<Inner, 4> @offset(16)
}

%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b1 {
  %b1 = block {
    %a:ptr<function, array<Outer, 4>, read_write> = var
    %3:ptr<function, vec4<f32>, read_write> = access %a, 0u, 1u, 1u, 2u
    %b:vec4<f32> = load %3
    ret
  }
}
)");
}

TEST_F(ProgramToIRAccessorTest, Accessor_Var_AssignmentLHS) {
    // var a: array<u32, 4>();
    // a[2] = 0;

    auto* a = Var("a", ty.array<u32, 4>(), core::AddressSpace::kFunction);
    auto* assign = Assign(IndexAccessor(a, 2_u), 0_u);
    WrapInFunction(Decl(a), assign);

    auto m = Build();
    ASSERT_TRUE(m) << m;

    EXPECT_EQ(Disassemble(m.Get()),
              R"(%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b1 {
  %b1 = block {
    %a:ptr<function, array<u32, 4>, read_write> = var
    %3:ptr<function, u32, read_write> = access %a, 2u
    store %3, 0u
    ret
  }
}
)");
}

TEST_F(ProgramToIRAccessorTest, Accessor_Var_VectorElementSwizzle) {
    // var a: vec2<f32>
    // let b = a.y

    auto* a = Var("a", ty.vec2<f32>(), core::AddressSpace::kFunction);
    auto* expr = Decl(Let("b", MemberAccessor(a, "y")));
    WrapInFunction(Decl(a), expr);

    auto m = Build();
    ASSERT_TRUE(m) << m;

    EXPECT_EQ(Disassemble(m.Get()),
              R"(%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b1 {
  %b1 = block {
    %a:ptr<function, vec2<f32>, read_write> = var
    %b:f32 = load_vector_element %a, 1u
    ret
  }
}
)");
}

TEST_F(ProgramToIRAccessorTest, Accessor_Var_MultiElementSwizzle) {
    // var a: vec3<f32>
    // let b = a.zyxz

    auto* a = Var("a", ty.vec3<f32>(), core::AddressSpace::kFunction);
    auto* expr = Decl(Let("b", MemberAccessor(a, "zyxz")));
    WrapInFunction(Decl(a), expr);

    auto m = Build();
    ASSERT_TRUE(m) << m;

    EXPECT_EQ(Disassemble(m.Get()),
              R"(%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b1 {
  %b1 = block {
    %a:ptr<function, vec3<f32>, read_write> = var
    %3:vec3<f32> = load %a
    %b:vec4<f32> = swizzle %3, zyxz
    ret
  }
}
)");
}

TEST_F(ProgramToIRAccessorTest, Accessor_Var_MultiElementSwizzleOfSwizzle) {
    // var a: vec3<f32>
    // let b = a.zyx.yy

    auto* a = Var("a", ty.vec3<f32>(), core::AddressSpace::kFunction);
    auto* expr = Decl(Let("b", MemberAccessor(MemberAccessor(a, "zyx"), "yy")));
    WrapInFunction(Decl(a), expr);

    auto m = Build();
    ASSERT_TRUE(m) << m;

    EXPECT_EQ(Disassemble(m.Get()),
              R"(%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b1 {
  %b1 = block {
    %a:ptr<function, vec3<f32>, read_write> = var
    %3:vec3<f32> = load %a
    %4:vec3<f32> = swizzle %3, zyx
    %b:vec2<f32> = swizzle %4, yy
    ret
  }
}
)");
}

TEST_F(ProgramToIRAccessorTest, Accessor_Var_MultiElementSwizzle_MiddleOfChain) {
    // struct MyStruct { a: i32; foo: vec4<f32> }
    // var a: MyStruct;
    // let b = a.foo.zyx.yx[0]

    auto* s = Structure("MyStruct", Vector{
                                        Member("a", ty.i32()),
                                        Member("foo", ty.vec4<f32>()),
                                    });
    auto* a = Var("a", ty.Of(s), core::AddressSpace::kFunction);
    auto* expr = Decl(Let(
        "b",
        IndexAccessor(MemberAccessor(MemberAccessor(MemberAccessor(a, "foo"), "zyx"), "yx"), 0_u)));
    WrapInFunction(Decl(a), expr);

    auto m = Build();
    ASSERT_TRUE(m) << m;

    EXPECT_EQ(Disassemble(m.Get()),
              R"(MyStruct = struct @align(16) {
  a:i32 @offset(0)
  foo:vec4<f32> @offset(16)
}

%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b1 {
  %b1 = block {
    %a:ptr<function, MyStruct, read_write> = var
    %3:ptr<function, vec4<f32>, read_write> = access %a, 1u
    %4:vec4<f32> = load %3
    %5:vec3<f32> = swizzle %4, zyx
    %6:vec2<f32> = swizzle %5, yx
    %b:f32 = access %6, 0u
    ret
  }
}
)");
}

TEST_F(ProgramToIRAccessorTest, Accessor_Let_SingleIndex) {
    // let a: vec3<u32> = vec3()
    // let b = a[2]
    auto* a = Let("a", ty.vec3<u32>(), vec(ty.u32(), 3));
    auto* expr = Decl(Let("b", IndexAccessor(a, 2_u)));
    WrapInFunction(Decl(a), expr);

    auto m = Build();
    ASSERT_TRUE(m) << m;

    EXPECT_EQ(Disassemble(m.Get()),
              R"(%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b1 {
  %b1 = block {
    %a:vec3<u32> = let vec3<u32>(0u)
    %b:u32 = access %a, 2u
    ret
  }
}
)");
}

TEST_F(ProgramToIRAccessorTest, Accessor_Let_MultiIndex) {
    // let a: mat3x4<f32> = mat3x4<u32>()
    // let b = a[2][3]

    auto* a = Let("a", ty.mat3x4<f32>(), Call<mat3x4<f32>>());
    auto* expr = Decl(Let("b", IndexAccessor(IndexAccessor(a, 2_u), 3_u)));
    WrapInFunction(Decl(a), expr);

    auto m = Build();
    ASSERT_TRUE(m) << m;

    EXPECT_EQ(Disassemble(m.Get()),
              R"(%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b1 {
  %b1 = block {
    %a:mat3x4<f32> = let mat3x4<f32>(vec4<f32>(0.0f))
    %b:f32 = access %a, 2u, 3u
    ret
  }
}
)");
}

TEST_F(ProgramToIRAccessorTest, Accessor_Let_SingleMember) {
    // struct MyStruct { foo: i32 }
    // let a: MyStruct = MyStruct();
    // let b = a.foo

    auto* s = Structure("MyStruct", Vector{
                                        Member("foo", ty.i32()),
                                    });
    auto* a = Let("a", ty.Of(s), Call("MyStruct"));
    auto* expr = Decl(Let("b", MemberAccessor(a, "foo")));
    WrapInFunction(Decl(a), expr);

    auto m = Build();
    ASSERT_TRUE(m) << m;

    EXPECT_EQ(Disassemble(m.Get()),
              R"(MyStruct = struct @align(4) {
  foo:i32 @offset(0)
}

%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b1 {
  %b1 = block {
    %a:MyStruct = let MyStruct(0i)
    %b:i32 = access %a, 0u
    ret
  }
}
)");
}

TEST_F(ProgramToIRAccessorTest, Accessor_Let_MultiMember) {
    // struct Inner { bar: f32 }
    // struct Outer { a: i32, foo: Inner }
    // let a: Outer = Outer();
    // let b = a.foo.bar

    auto* inner = Structure("Inner", Vector{
                                         Member("bar", ty.f32()),
                                     });
    auto* outer = Structure("Outer", Vector{
                                         Member("a", ty.i32()),
                                         Member("foo", ty.Of(inner)),
                                     });
    auto* a = Let("a", ty.Of(outer), Call("Outer"));
    auto* expr = Decl(Let("b", MemberAccessor(MemberAccessor(a, "foo"), "bar")));
    WrapInFunction(Decl(a), expr);

    auto m = Build();
    ASSERT_TRUE(m) << m;

    EXPECT_EQ(Disassemble(m.Get()),
              R"(Inner = struct @align(4) {
  bar:f32 @offset(0)
}

Outer = struct @align(4) {
  a:i32 @offset(0)
  foo:Inner @offset(4)
}

%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b1 {
  %b1 = block {
    %a:Outer = let Outer(0i, Inner(0.0f))
    %b:f32 = access %a, 1u, 0u
    ret
  }
}
)");
}

TEST_F(ProgramToIRAccessorTest, Accessor_Let_Mixed) {
    // struct Outer { a: i32, foo: array<Inner, 4> }
    // struct Inner { b: i32, c: f32, bar: vec4<f32> }
    // let a: array<Outer, 4> = array();
    // let b = a[0].foo[1].bar

    auto* inner = Structure("Inner", Vector{
                                         Member("b", ty.i32()),
                                         Member("c", ty.f32()),
                                         Member("bar", ty.vec4<f32>()),
                                     });
    auto* outer = Structure("Outer", Vector{
                                         Member("a", ty.i32()),
                                         Member("foo", ty.array(ty.Of(inner), 4_u)),
                                     });
    auto* a = Let("a", ty.array(ty.Of(outer), 4_u), Call(ty.array(ty.Of(outer), 4_u)));
    auto* expr = Decl(Let(
        "b",
        MemberAccessor(IndexAccessor(MemberAccessor(IndexAccessor(a, 0_u), "foo"), 1_u), "bar")));
    WrapInFunction(Decl(a), expr);

    auto m = Build();
    ASSERT_TRUE(m) << m;

    EXPECT_EQ(Disassemble(m.Get()),
              R"(Inner = struct @align(16) {
  b:i32 @offset(0)
  c:f32 @offset(4)
  bar:vec4<f32> @offset(16)
}

Outer = struct @align(16) {
  a:i32 @offset(0)
  foo:array<Inner, 4> @offset(16)
}

%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b1 {
  %b1 = block {
    %a:array<Outer, 4> = let array<Outer, 4>(Outer(0i, array<Inner, 4>(Inner(0i, 0.0f, vec4<f32>(0.0f)))))
    %b:vec4<f32> = access %a, 0u, 1u, 1u, 2u
    ret
  }
}
)");
}

TEST_F(ProgramToIRAccessorTest, Accessor_Let_SingleElement) {
    // let a: vec2<f32> = vec2()
    // let b = a.y

    auto* a = Let("a", ty.vec2<f32>(), vec(ty.f32(), 2));
    auto* expr = Decl(Let("b", MemberAccessor(a, "y")));
    WrapInFunction(Decl(a), expr);

    auto m = Build();
    ASSERT_TRUE(m) << m;

    EXPECT_EQ(Disassemble(m.Get()),
              R"(%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b1 {
  %b1 = block {
    %a:vec2<f32> = let vec2<f32>(0.0f)
    %b:f32 = access %a, 1u
    ret
  }
}
)");
}

TEST_F(ProgramToIRAccessorTest, Accessor_Let_MultiElementSwizzle) {
    // let a: vec3<f32 = vec3()>
    // let b = a.zyxz

    auto* a = Let("a", ty.vec3<f32>(), vec(ty.f32(), 3));
    auto* expr = Decl(Let("b", MemberAccessor(a, "zyxz")));
    WrapInFunction(Decl(a), expr);

    auto m = Build();
    ASSERT_TRUE(m) << m;

    EXPECT_EQ(Disassemble(m.Get()),
              R"(%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b1 {
  %b1 = block {
    %a:vec3<f32> = let vec3<f32>(0.0f)
    %b:vec4<f32> = swizzle %a, zyxz
    ret
  }
}
)");
}

TEST_F(ProgramToIRAccessorTest, Accessor_Let_MultiElementSwizzleOfSwizzle) {
    // let a: vec3<f32> = vec3();
    // let b = a.zyx.yy

    auto* a = Let("a", ty.vec3<f32>(), vec(ty.f32(), 3));
    auto* expr = Decl(Let("b", MemberAccessor(MemberAccessor(a, "zyx"), "yy")));
    WrapInFunction(Decl(a), expr);

    auto m = Build();
    ASSERT_TRUE(m) << m;

    EXPECT_EQ(Disassemble(m.Get()),
              R"(%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b1 {
  %b1 = block {
    %a:vec3<f32> = let vec3<f32>(0.0f)
    %3:vec3<f32> = swizzle %a, zyx
    %b:vec2<f32> = swizzle %3, yy
    ret
  }
}
)");
}

TEST_F(ProgramToIRAccessorTest, Accessor_Let_MultiElementSwizzle_MiddleOfChain) {
    // struct MyStruct { a: i32; foo: vec4<f32> }
    // let a: MyStruct = MyStruct();
    // let b = a.foo.zyx.yx[0]

    auto* s = Structure("MyStruct", Vector{
                                        Member("a", ty.i32()),
                                        Member("foo", ty.vec4<f32>()),
                                    });
    auto* a = Let("a", ty.Of(s), Call("MyStruct"));
    auto* expr = Decl(Let(
        "b",
        IndexAccessor(MemberAccessor(MemberAccessor(MemberAccessor(a, "foo"), "zyx"), "yx"), 0_u)));
    WrapInFunction(Decl(a), expr);

    auto m = Build();
    ASSERT_TRUE(m) << m;

    EXPECT_EQ(Disassemble(m.Get()),
              R"(MyStruct = struct @align(16) {
  a:i32 @offset(0)
  foo:vec4<f32> @offset(16)
}

%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b1 {
  %b1 = block {
    %a:MyStruct = let MyStruct(0i, vec4<f32>(0.0f))
    %3:vec4<f32> = access %a, 1u
    %4:vec3<f32> = swizzle %3, zyx
    %5:vec2<f32> = swizzle %4, yx
    %b:f32 = access %5, 0u
    ret
  }
}
)");
}

TEST_F(ProgramToIRAccessorTest, Accessor_Const_AbstractVectorWithIndex) {
    // const v = vec3(1, 2, 3);
    // let i = 1;
    // var b = v[i];

    auto* v = Const("v", Call<vec3<Infer>>(1_a, 2_a, 3_a));
    auto* i = Let("i", Expr(1_i));
    auto* b = Var("b", IndexAccessor("v", "i"));
    WrapInFunction(v, i, b);

    auto m = Build();
    ASSERT_TRUE(m) << m;

    EXPECT_EQ(Disassemble(m.Get()),
              R"(%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b1 {
  %b1 = block {
    %i:i32 = let 1i
    %3:i32 = access vec3<i32>(1i, 2i, 3i), %i
    %b:ptr<function, i32, read_write> = var, %3
    ret
  }
}
)");
}

TEST_F(ProgramToIRAccessorTest, Accessor_Const_AbstractVectorWithSwizzleAndIndex) {
    // const v = vec3(1, 2, 3);
    // let i = 1;
    // var b = v.rg[i];

    auto* v = Const("v", Call<vec3<Infer>>(1_a, 2_a, 3_a));
    auto* i = Let("i", Expr(1_i));
    auto* b = Var("b", IndexAccessor(MemberAccessor("v", "rg"), "i"));
    WrapInFunction(v, i, b);

    auto m = Build();
    ASSERT_TRUE(m) << m;

    EXPECT_EQ(Disassemble(m.Get()),
              R"(%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b1 {
  %b1 = block {
    %i:i32 = let 1i
    %3:i32 = access vec2<i32>(1i, 2i), %i
    %b:ptr<function, i32, read_write> = var, %3
    ret
  }
}
)");
}

TEST_F(ProgramToIRAccessorTest, Accessor_Const_ExpressionIndex) {
    // const v = vec3(1, 2, 3);
    // let i = 1;
    // var b = v.rg[i + 2 - 3];

    auto* v = Const("v", Call<vec3<Infer>>(1_a, 2_a, 3_a));
    auto* i = Let("i", Expr(1_i));
    auto* b = Var("b", IndexAccessor(MemberAccessor("v", "rg"), Sub(Add("i", 2_i), 3_i)));
    WrapInFunction(v, i, b);

    auto m = Build();
    ASSERT_TRUE(m) << m;

    EXPECT_EQ(Disassemble(m.Get()),
              R"(%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b1 {
  %b1 = block {
    %i:i32 = let 1i
    %3:i32 = add %i, 2i
    %4:i32 = sub %3, 3i
    %5:i32 = access vec2<i32>(1i, 2i), %4
    %b:ptr<function, i32, read_write> = var, %5
    ret
  }
}
)");
}

}  // namespace
}  // namespace tint::wgsl::reader
