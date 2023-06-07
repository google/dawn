// Copyright 2023 The Tint Authors.
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
#include "src/tint/ast/case_selector.h"
#include "src/tint/ast/int_literal_expression.h"
#include "src/tint/constant/scalar.h"
#include "src/tint/ir/block.h"
#include "src/tint/ir/constant.h"
#include "src/tint/ir/program_test_helper.h"
#include "src/tint/ir/var.h"

namespace tint::ir {
namespace {

using namespace tint::number_suffixes;  // NOLINT

using IR_FromProgramAccessorTest = ProgramTestHelper;

TEST_F(IR_FromProgramAccessorTest, Accessor_Var_SingleIndex) {
    // var a: vec3<u32>
    // let b = a[2]

    auto* a = Var("a", ty.vec3<u32>(), builtin::AddressSpace::kFunction);
    auto* expr = Decl(Let("b", IndexAccessor(a, 2_u)));
    WrapInFunction(Block(utils::Vector{Decl(a), expr}));

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(Disassemble(m.Get()),
              R"(%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b1 {
  %b1 = block {
    %a:ptr<function, vec3<u32>, read_write> = var
    %3:ptr<function, u32, read_write> = access %a, 2u
    %b:u32 = load %3
    ret
  }
}
)");
}

TEST_F(IR_FromProgramAccessorTest, Accessor_Var_MultiIndex) {
    // var a: mat3x4<f32>
    // let b = a[2][3]

    auto* a = Var("a", ty.mat3x4<f32>(), builtin::AddressSpace::kFunction);
    auto* expr = Decl(Let("b", IndexAccessor(IndexAccessor(a, 2_u), 3_u)));
    WrapInFunction(Block(utils::Vector{Decl(a), expr}));

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(Disassemble(m.Get()),
              R"(%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b1 {
  %b1 = block {
    %a:ptr<function, mat3x4<f32>, read_write> = var
    %3:ptr<function, f32, read_write> = access %a, 2u, 3u
    %b:f32 = load %3
    ret
  }
}
)");
}

TEST_F(IR_FromProgramAccessorTest, Accessor_Var_SingleMember) {
    // struct MyStruct { foo: i32 }
    // var a: MyStruct;
    // let b = a.foo

    auto* s = Structure("MyStruct", utils::Vector{
                                        Member("foo", ty.i32()),
                                    });
    auto* a = Var("a", ty.Of(s), builtin::AddressSpace::kFunction);
    auto* expr = Decl(Let("b", MemberAccessor(a, "foo")));
    WrapInFunction(Block(utils::Vector{Decl(a), expr}));

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

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

TEST_F(IR_FromProgramAccessorTest, Accessor_Var_MultiMember) {
    // struct Inner { bar: f32 }
    // struct Outer { a: i32, foo: Inner }
    // var a: Outer;
    // let b = a.foo.bar

    auto* inner = Structure("Inner", utils::Vector{
                                         Member("bar", ty.f32()),
                                     });
    auto* outer = Structure("Outer", utils::Vector{
                                         Member("a", ty.i32()),
                                         Member("foo", ty.Of(inner)),
                                     });
    auto* a = Var("a", ty.Of(outer), builtin::AddressSpace::kFunction);
    auto* expr = Decl(Let("b", MemberAccessor(MemberAccessor(a, "foo"), "bar")));
    WrapInFunction(Block(utils::Vector{Decl(a), expr}));

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

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

TEST_F(IR_FromProgramAccessorTest, Accessor_Var_Mixed) {
    // struct Inner { b: i32, c: f32, bar: vec4<f32> }
    // struct Outer { a: i32, foo: array<Inner, 4> }
    // var a: array<Outer, 4>
    // let b = a[0].foo[1].bar

    auto* inner = Structure("Inner", utils::Vector{
                                         Member("b", ty.i32()),
                                         Member("c", ty.f32()),
                                         Member("bar", ty.vec4<f32>()),
                                     });
    auto* outer = Structure("Outer", utils::Vector{
                                         Member("a", ty.i32()),
                                         Member("foo", ty.array(ty.Of(inner), 4_u)),
                                     });
    auto* a = Var("a", ty.array(ty.Of(outer), 4_u), builtin::AddressSpace::kFunction);
    auto* expr = Decl(Let(
        "b",
        MemberAccessor(IndexAccessor(MemberAccessor(IndexAccessor(a, 0_u), "foo"), 1_u), "bar")));
    WrapInFunction(Block(utils::Vector{Decl(a), expr}));

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

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

TEST_F(IR_FromProgramAccessorTest, Accessor_Var_AssignmentLHS) {
    // var a: array<u32, 4>();
    // a[2] = 0;

    auto* a = Var("a", ty.array<u32, 4>(), builtin::AddressSpace::kFunction);
    auto* assign = Assign(IndexAccessor(a, 2_u), 0_u);
    WrapInFunction(Block(utils::Vector{Decl(a), assign}));

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

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

TEST_F(IR_FromProgramAccessorTest, Accessor_Var_SingleElementSwizzle) {
    // var a: vec2<f32>
    // let b = a.y

    auto* a = Var("a", ty.vec2<f32>(), builtin::AddressSpace::kFunction);
    auto* expr = Decl(Let("b", MemberAccessor(a, "y")));
    WrapInFunction(Block(utils::Vector{Decl(a), expr}));

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(Disassemble(m.Get()),
              R"(%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b1 {
  %b1 = block {
    %a:ptr<function, vec2<f32>, read_write> = var
    %3:ptr<function, f32, read_write> = access %a, 1u
    %b:f32 = load %3
    ret
  }
}
)");
}

TEST_F(IR_FromProgramAccessorTest, Accessor_Var_MultiElementSwizzle) {
    // var a: vec3<f32>
    // let b = a.zyxz

    auto* a = Var("a", ty.vec3<f32>(), builtin::AddressSpace::kFunction);
    auto* expr = Decl(Let("b", MemberAccessor(a, "zyxz")));
    WrapInFunction(Block(utils::Vector{Decl(a), expr}));

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

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

TEST_F(IR_FromProgramAccessorTest, Accessor_Var_MultiElementSwizzleOfSwizzle) {
    // var a: vec3<f32>
    // let b = a.zyx.yy

    auto* a = Var("a", ty.vec3<f32>(), builtin::AddressSpace::kFunction);
    auto* expr = Decl(Let("b", MemberAccessor(MemberAccessor(a, "zyx"), "yy")));
    WrapInFunction(Block(utils::Vector{Decl(a), expr}));

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

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

TEST_F(IR_FromProgramAccessorTest, Accessor_Var_MultiElementSwizzle_MiddleOfChain) {
    // struct MyStruct { a: i32; foo: vec4<f32> }
    // var a: MyStruct;
    // let b = a.foo.zyx.yx[0]

    auto* s = Structure("MyStruct", utils::Vector{
                                        Member("a", ty.i32()),
                                        Member("foo", ty.vec4<f32>()),
                                    });
    auto* a = Var("a", ty.Of(s), builtin::AddressSpace::kFunction);
    auto* expr = Decl(Let(
        "b",
        IndexAccessor(MemberAccessor(MemberAccessor(MemberAccessor(a, "foo"), "zyx"), "yx"), 0_u)));
    WrapInFunction(Block(utils::Vector{Decl(a), expr}));

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

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

TEST_F(IR_FromProgramAccessorTest, Accessor_Let_SingleIndex) {
    // let a: vec3<u32> = vec3()
    // let b = a[2]
    auto* a = Let("a", ty.vec3<u32>(), vec(ty.u32(), 3));
    auto* expr = Decl(Let("b", IndexAccessor(a, 2_u)));
    WrapInFunction(Block(utils::Vector{Decl(a), expr}));

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(Disassemble(m.Get()),
              R"(%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b1 {
  %b1 = block {
    %b:u32 = access vec3<u32>(0u), 2u
    ret
  }
}
)");
}

TEST_F(IR_FromProgramAccessorTest, Accessor_Let_MultiIndex) {
    // let a: mat3x4<f32> = mat3x4<u32>()
    // let b = a[2][3]

    auto* a = Let("a", ty.mat3x4<f32>(), mat3x4<f32>());
    auto* expr = Decl(Let("b", IndexAccessor(IndexAccessor(a, 2_u), 3_u)));
    WrapInFunction(Block(utils::Vector{Decl(a), expr}));

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(Disassemble(m.Get()),
              R"(%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b1 {
  %b1 = block {
    %b:f32 = access mat3x4<f32>(vec4<f32>(0.0f)), 2u, 3u
    ret
  }
}
)");
}

TEST_F(IR_FromProgramAccessorTest, Accessor_Let_SingleMember) {
    // struct MyStruct { foo: i32 }
    // let a: MyStruct = MyStruct();
    // let b = a.foo

    auto* s = Structure("MyStruct", utils::Vector{
                                        Member("foo", ty.i32()),
                                    });
    auto* a = Let("a", ty.Of(s), Call("MyStruct"));
    auto* expr = Decl(Let("b", MemberAccessor(a, "foo")));
    WrapInFunction(Block(utils::Vector{Decl(a), expr}));

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(Disassemble(m.Get()),
              R"(MyStruct = struct @align(4) {
  foo:i32 @offset(0)
}

%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b1 {
  %b1 = block {
    %b:i32 = access MyStruct(0i), 0u
    ret
  }
}
)");
}

TEST_F(IR_FromProgramAccessorTest, Accessor_Let_MultiMember) {
    // struct Inner { bar: f32 }
    // struct Outer { a: i32, foo: Inner }
    // let a: Outer = Outer();
    // let b = a.foo.bar

    auto* inner = Structure("Inner", utils::Vector{
                                         Member("bar", ty.f32()),
                                     });
    auto* outer = Structure("Outer", utils::Vector{
                                         Member("a", ty.i32()),
                                         Member("foo", ty.Of(inner)),
                                     });
    auto* a = Let("a", ty.Of(outer), Call("Outer"));
    auto* expr = Decl(Let("b", MemberAccessor(MemberAccessor(a, "foo"), "bar")));
    WrapInFunction(Block(utils::Vector{Decl(a), expr}));

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

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
    %b:f32 = access Outer(0i, Inner(0.0f)), 1u, 0u
    ret
  }
}
)");
}

TEST_F(IR_FromProgramAccessorTest, Accessor_Let_Mixed) {
    // struct Outer { a: i32, foo: array<Inner, 4> }
    // struct Inner { b: i32, c: f32, bar: vec4<f32> }
    // let a: array<Outer, 4> = array();
    // let b = a[0].foo[1].bar

    auto* inner = Structure("Inner", utils::Vector{
                                         Member("b", ty.i32()),
                                         Member("c", ty.f32()),
                                         Member("bar", ty.vec4<f32>()),
                                     });
    auto* outer = Structure("Outer", utils::Vector{
                                         Member("a", ty.i32()),
                                         Member("foo", ty.array(ty.Of(inner), 4_u)),
                                     });
    auto* a = Let("a", ty.array(ty.Of(outer), 4_u), array(ty.Of(outer), 4_u));
    auto* expr = Decl(Let(
        "b",
        MemberAccessor(IndexAccessor(MemberAccessor(IndexAccessor(a, 0_u), "foo"), 1_u), "bar")));
    WrapInFunction(Block(utils::Vector{Decl(a), expr}));

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

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
    %b:vec4<f32> = access array<Outer, 4>(Outer(0i, array<Inner, 4>(Inner(0i, 0.0f, vec4<f32>(0.0f))))), 0u, 1u, 1u, 2u
    ret
  }
}
)");
}

TEST_F(IR_FromProgramAccessorTest, Accessor_Let_SingleElement) {
    // let a: vec2<f32> = vec2()
    // let b = a.y

    auto* a = Let("a", ty.vec2<f32>(), vec(ty.f32(), 2));
    auto* expr = Decl(Let("b", MemberAccessor(a, "y")));
    WrapInFunction(Block(utils::Vector{Decl(a), expr}));

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(Disassemble(m.Get()),
              R"(%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b1 {
  %b1 = block {
    %b:f32 = access vec2<f32>(0.0f), 1u
    ret
  }
}
)");
}

TEST_F(IR_FromProgramAccessorTest, Accessor_Let_MultiElementSwizzle) {
    // let a: vec3<f32 = vec3()>
    // let b = a.zyxz

    auto* a = Let("a", ty.vec3<f32>(), vec(ty.f32(), 3));
    auto* expr = Decl(Let("b", MemberAccessor(a, "zyxz")));
    WrapInFunction(Block(utils::Vector{Decl(a), expr}));

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(Disassemble(m.Get()),
              R"(%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b1 {
  %b1 = block {
    %b:vec4<f32> = swizzle vec3<f32>(0.0f), zyxz
    ret
  }
}
)");
}

TEST_F(IR_FromProgramAccessorTest, Accessor_Let_MultiElementSwizzleOfSwizzle) {
    // let a: vec3<f32> = vec3();
    // let b = a.zyx.yy

    auto* a = Let("a", ty.vec3<f32>(), vec(ty.f32(), 3));
    auto* expr = Decl(Let("b", MemberAccessor(MemberAccessor(a, "zyx"), "yy")));
    WrapInFunction(Block(utils::Vector{Decl(a), expr}));

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(Disassemble(m.Get()),
              R"(%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b1 {
  %b1 = block {
    %2:vec3<f32> = swizzle vec3<f32>(0.0f), zyx
    %b:vec2<f32> = swizzle %2, yy
    ret
  }
}
)");
}

TEST_F(IR_FromProgramAccessorTest, Accessor_Let_MultiElementSwizzle_MiddleOfChain) {
    // struct MyStruct { a: i32; foo: vec4<f32> }
    // let a: MyStruct = MyStruct();
    // let b = a.foo.zyx.yx[0]

    auto* s = Structure("MyStruct", utils::Vector{
                                        Member("a", ty.i32()),
                                        Member("foo", ty.vec4<f32>()),
                                    });
    auto* a = Let("a", ty.Of(s), Call("MyStruct"));
    auto* expr = Decl(Let(
        "b",
        IndexAccessor(MemberAccessor(MemberAccessor(MemberAccessor(a, "foo"), "zyx"), "yx"), 0_u)));
    WrapInFunction(Block(utils::Vector{Decl(a), expr}));

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(Disassemble(m.Get()),
              R"(MyStruct = struct @align(16) {
  a:i32 @offset(0)
  foo:vec4<f32> @offset(16)
}

%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b1 {
  %b1 = block {
    %2:vec4<f32> = access MyStruct(0i, vec4<f32>(0.0f)), 1u
    %3:vec3<f32> = swizzle %2, zyx
    %4:vec2<f32> = swizzle %3, yx
    %b:f32 = access %4, 0u
    ret
  }
}
)");
}

}  // namespace
}  // namespace tint::ir
