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

#include "src/tint/lang/glsl/writer/raise/hoist_constant_array_initializers.h"

#include <utility>

#include "gtest/gtest.h"
#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/ir/transform/helper_test.h"
#include "src/tint/lang/core/number.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::glsl::writer::raise {
namespace {

using GlslWriter_HoistConstantArrayInitializersTest = core::ir::transform::TransformTest;

TEST_F(GlslWriter_HoistConstantArrayInitializersTest, NoModify_Scalar) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Var<function>("a", 1_i);
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %a:ptr<function, i32, read_write> = var 1i
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;
    Run(HoistConstantArrayInitializers);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_HoistConstantArrayInitializersTest, NoModify_PlainArray) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Var<function>("a", b.Composite(ty.array<i32, 2>(), 1_i, 2_i));
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %a:ptr<function, array<i32, 2>, read_write> = var array<i32, 2>(1i, 2i)
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;
    Run(HoistConstantArrayInitializers);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_HoistConstantArrayInitializersTest, NoModify_PlainStruct) {
    auto* str_ty = ty.Struct(mod.symbols.New("S"), {
                                                       {mod.symbols.New("a"), ty.i32()},
                                                       {mod.symbols.New("b"), ty.f32()},
                                                   });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Var<function>("a", b.Composite(str_ty, 1_i, 2_f));
        b.Return(func);
    });

    auto* src = R"(
S = struct @align(4) {
  a:i32 @offset(0)
  b:f32 @offset(4)
}

%foo = @fragment func():void {
  $B1: {
    %a:ptr<function, S, read_write> = var S(1i, 2.0f)
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;
    Run(HoistConstantArrayInitializers);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_HoistConstantArrayInitializersTest, StructWithArray) {
    auto* str_ty = ty.Struct(mod.symbols.New("S"), {
                                                       {mod.symbols.New("a"), ty.array<f32, 2>()},
                                                   });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* arr = b.Composite(ty.array<f32, 2>(), 1_f, 2_f);
        b.Var<function>("s", b.Composite(str_ty, arr));
        b.Return(func);
    });

    auto* src = R"(
S = struct @align(4) {
  a:array<f32, 2> @offset(0)
}

%foo = @fragment func():void {
  $B1: {
    %s:ptr<function, S, read_write> = var S(array<f32, 2>(1.0f, 2.0f))
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
S = struct @align(4) {
  a:array<f32, 2> @offset(0)
}

%foo = @fragment func():void {
  $B1: {
    %2:array<f32, 2> = let array<f32, 2>(1.0f, 2.0f)
    %3:S = call %tint_construct_S, %2
    %s:ptr<function, S, read_write> = var %3
    ret
  }
}
%tint_construct_S = func(%a:array<f32, 2>):S {
  $B2: {
    %7:ptr<function, S, read_write> = var undef
    %8:ptr<function, array<f32, 2>, read_write> = access %7, 0u
    store %8, %a
    %9:S = load %7
    ret %9
  }
}
)";
    Run(HoistConstantArrayInitializers);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_HoistConstantArrayInitializersTest, StructWithArrayAndScalars) {
    auto* str_ty = ty.Struct(mod.symbols.New("S"), {
                                                       {mod.symbols.New("x"), ty.i32()},
                                                       {mod.symbols.New("a"), ty.array<f32, 2>()},
                                                       {mod.symbols.New("y"), ty.f32()},
                                                   });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* arr = b.Composite(ty.array<f32, 2>(), 1_f, 2_f);
        b.Var<function>("s", b.Composite(str_ty, 1_i, arr, 2_f));
        b.Return(func);
    });

    auto* src = R"(
S = struct @align(4) {
  x:i32 @offset(0)
  a:array<f32, 2> @offset(4)
  y:f32 @offset(12)
}

%foo = @fragment func():void {
  $B1: {
    %s:ptr<function, S, read_write> = var S(1i, array<f32, 2>(1.0f, 2.0f), 2.0f)
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
S = struct @align(4) {
  x:i32 @offset(0)
  a:array<f32, 2> @offset(4)
  y:f32 @offset(12)
}

%foo = @fragment func():void {
  $B1: {
    %2:array<f32, 2> = let array<f32, 2>(1.0f, 2.0f)
    %3:S = call %tint_construct_S, 1i, %2, 2.0f
    %s:ptr<function, S, read_write> = var %3
    ret
  }
}
%tint_construct_S = func(%x:i32, %a:array<f32, 2>, %y:f32):S {
  $B2: {
    %9:ptr<function, S, read_write> = var undef
    %10:ptr<function, i32, read_write> = access %9, 0u
    store %10, %x
    %11:ptr<function, array<f32, 2>, read_write> = access %9, 1u
    store %11, %a
    %12:ptr<function, f32, read_write> = access %9, 2u
    store %12, %y
    %13:S = load %9
    ret %13
  }
}
)";
    Run(HoistConstantArrayInitializers);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_HoistConstantArrayInitializersTest, NestedStructWithArray) {
    auto* inner_ty =
        ty.Struct(mod.symbols.New("Inner"), {
                                                {mod.symbols.New("a"), ty.array<f32, 2>()},
                                            });
    auto* outer_ty = ty.Struct(mod.symbols.New("Outer"), {
                                                             {mod.symbols.New("inner"), inner_ty},
                                                             {mod.symbols.New("y"), ty.f32()},
                                                         });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* arr = b.Composite(ty.array<f32, 2>(), 1_f, 2_f);
        auto* inner = b.Composite(inner_ty, arr);
        b.Var<function>("o", b.Composite(outer_ty, inner, 3_f));
        b.Return(func);
    });

    auto* src = R"(
Inner = struct @align(4) {
  a:array<f32, 2> @offset(0)
}

Outer = struct @align(4) {
  inner:Inner @offset(0)
  y:f32 @offset(8)
}

%foo = @fragment func():void {
  $B1: {
    %o:ptr<function, Outer, read_write> = var Outer(Inner(array<f32, 2>(1.0f, 2.0f)), 3.0f)
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Inner = struct @align(4) {
  a:array<f32, 2> @offset(0)
}

Outer = struct @align(4) {
  inner:Inner @offset(0)
  y:f32 @offset(8)
}

%foo = @fragment func():void {
  $B1: {
    %2:array<f32, 2> = let array<f32, 2>(1.0f, 2.0f)
    %3:Inner = call %tint_construct_Inner, %2
    %5:Outer = call %tint_construct_Outer, %3, 3.0f
    %o:ptr<function, Outer, read_write> = var %5
    ret
  }
}
%tint_construct_Inner = func(%a:array<f32, 2>):Inner {
  $B2: {
    %9:ptr<function, Inner, read_write> = var undef
    %10:ptr<function, array<f32, 2>, read_write> = access %9, 0u
    store %10, %a
    %11:Inner = load %9
    ret %11
  }
}
%tint_construct_Outer = func(%inner:Inner, %y:f32):Outer {
  $B3: {
    %14:ptr<function, Outer, read_write> = var undef
    %15:ptr<function, Inner, read_write> = access %14, 0u
    store %15, %inner
    %16:ptr<function, f32, read_write> = access %14, 1u
    store %16, %y
    %17:Outer = load %14
    ret %17
  }
}
)";
    Run(HoistConstantArrayInitializers);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_HoistConstantArrayInitializersTest, MultipleUsesAcrossFunctions) {
    auto* str_ty = ty.Struct(mod.symbols.New("S"), {
                                                       {mod.symbols.New("a"), ty.array<f32, 2>()},
                                                   });

    auto* func1 = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func1->Block(), [&] {
        auto* arr = b.Composite(ty.array<f32, 2>(), 1_f, 2_f);
        b.Var<function>("s1", b.Composite(str_ty, arr));
        b.Return(func1);
    });

    auto* func2 = b.Function("bar", ty.void_());
    b.Append(func2->Block(), [&] {
        auto* arr = b.Composite(ty.array<f32, 2>(), 1_f, 2_f);
        b.Var<function>("s2", b.Composite(str_ty, arr));
        b.Return(func2);
    });

    auto* src = R"(
S = struct @align(4) {
  a:array<f32, 2> @offset(0)
}

%foo = @fragment func():void {
  $B1: {
    %s1:ptr<function, S, read_write> = var S(array<f32, 2>(1.0f, 2.0f))
    ret
  }
}
%bar = func():void {
  $B2: {
    %s2:ptr<function, S, read_write> = var S(array<f32, 2>(1.0f, 2.0f))
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
S = struct @align(4) {
  a:array<f32, 2> @offset(0)
}

%foo = @fragment func():void {
  $B1: {
    %2:array<f32, 2> = let array<f32, 2>(1.0f, 2.0f)
    %3:S = call %tint_construct_S, %2
    %s1:ptr<function, S, read_write> = var %3
    ret
  }
}
%bar = func():void {
  $B2: {
    %7:array<f32, 2> = let array<f32, 2>(1.0f, 2.0f)
    %8:S = call %tint_construct_S, %7
    %s2:ptr<function, S, read_write> = var %8
    ret
  }
}
%tint_construct_S = func(%a:array<f32, 2>):S {
  $B3: {
    %11:ptr<function, S, read_write> = var undef
    %12:ptr<function, array<f32, 2>, read_write> = access %11, 0u
    store %12, %a
    %13:S = load %11
    ret %13
  }
}
)";
    Run(HoistConstantArrayInitializers);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_HoistConstantArrayInitializersTest, MultipleUsesInSameFunction) {
    auto* str_ty = ty.Struct(mod.symbols.New("S"), {
                                                       {mod.symbols.New("a"), ty.array<f32, 2>()},
                                                   });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* arr = b.Composite(ty.array<f32, 2>(), 1_f, 2_f);
        b.Var<function>("s1", b.Composite(str_ty, arr));
        b.Var<function>("s2", b.Composite(str_ty, arr));
        b.Return(func);
    });

    auto* src = R"(
S = struct @align(4) {
  a:array<f32, 2> @offset(0)
}

%foo = @fragment func():void {
  $B1: {
    %s1:ptr<function, S, read_write> = var S(array<f32, 2>(1.0f, 2.0f))
    %s2:ptr<function, S, read_write> = var S(array<f32, 2>(1.0f, 2.0f))
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
S = struct @align(4) {
  a:array<f32, 2> @offset(0)
}

%foo = @fragment func():void {
  $B1: {
    %2:array<f32, 2> = let array<f32, 2>(1.0f, 2.0f)
    %3:S = call %tint_construct_S, %2
    %s1:ptr<function, S, read_write> = var %3
    %6:array<f32, 2> = let array<f32, 2>(1.0f, 2.0f)
    %7:S = call %tint_construct_S, %6
    %s2:ptr<function, S, read_write> = var %7
    ret
  }
}
%tint_construct_S = func(%a:array<f32, 2>):S {
  $B2: {
    %10:ptr<function, S, read_write> = var undef
    %11:ptr<function, array<f32, 2>, read_write> = access %10, 0u
    store %11, %a
    %12:S = load %10
    ret %12
  }
}
)";
    Run(HoistConstantArrayInitializers);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_HoistConstantArrayInitializersTest, ModuleScopeVar) {
    auto* str_ty = ty.Struct(mod.symbols.New("S"), {
                                                       {mod.symbols.New("a"), ty.array<f32, 2>()},
                                                   });

    b.Append(mod.root_block, [&] {
        auto* arr = b.Composite(ty.array<f32, 2>(), 1_f, 2_f);
        b.Var<private_>("g", b.Composite(str_ty, arr));
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] { b.Return(func); });

    auto* src = R"(
S = struct @align(4) {
  a:array<f32, 2> @offset(0)
}

$B1: {  # root
  %g:ptr<private, S, read_write> = var S(array<f32, 2>(1.0f, 2.0f))
}

%foo = @fragment func():void {
  $B2: {
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
S = struct @align(4) {
  a:array<f32, 2> @offset(0)
}

$B1: {  # root
  %g:ptr<private, S, read_write> = var undef
}

%foo = @fragment func():void {
  $B2: {
    %3:array<f32, 2> = let array<f32, 2>(1.0f, 2.0f)
    %4:S = call %tint_construct_S, %3
    store %g, %4
    ret
  }
}
%tint_construct_S = func(%a:array<f32, 2>):S {
  $B3: {
    %7:ptr<function, S, read_write> = var undef
    %8:ptr<function, array<f32, 2>, read_write> = access %7, 0u
    store %8, %a
    %9:S = load %7
    ret %9
  }
}
)";
    Run(HoistConstantArrayInitializers);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_HoistConstantArrayInitializersTest, ModuleScopeUninitializedVar) {
    auto* str_ty = ty.Struct(mod.symbols.New("S"), {
                                                       {mod.symbols.New("a"), ty.array<f32, 2>()},
                                                   });

    b.Append(mod.root_block, [&] { b.Var("g", private_, str_ty); });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] { b.Return(func); });

    auto* src = R"(
S = struct @align(4) {
  a:array<f32, 2> @offset(0)
}

$B1: {  # root
  %g:ptr<private, S, read_write> = var
}

%foo = @fragment func():void {
  $B2: {
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
S = struct @align(4) {
  a:array<f32, 2> @offset(0)
}

$B1: {  # root
  %g:ptr<private, S, read_write> = var undef
}

%foo = @fragment func():void {
  $B2: {
    %3:array<f32, 2> = let array<f32, 2>(0.0f)
    %4:S = call %tint_construct_S, %3
    store %g, %4
    ret
  }
}
%tint_construct_S = func(%a:array<f32, 2>):S {
  $B3: {
    %7:ptr<function, S, read_write> = var undef
    %8:ptr<function, array<f32, 2>, read_write> = access %7, 0u
    store %8, %a
    %9:S = load %7
    ret %9
  }
}
)";
    Run(HoistConstantArrayInitializers);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_HoistConstantArrayInitializersTest, UninitializedVar) {
    auto* str_ty = ty.Struct(mod.symbols.New("S"), {
                                                       {mod.symbols.New("a"), ty.array<f32, 2>()},
                                                   });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Var("s", function, str_ty);
        b.Return(func);
    });

    auto* src = R"(
S = struct @align(4) {
  a:array<f32, 2> @offset(0)
}

%foo = @fragment func():void {
  $B1: {
    %s:ptr<function, S, read_write> = var
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
S = struct @align(4) {
  a:array<f32, 2> @offset(0)
}

%foo = @fragment func():void {
  $B1: {
    %2:array<f32, 2> = let array<f32, 2>(0.0f)
    %3:S = call %tint_construct_S, %2
    %s:ptr<function, S, read_write> = var %3
    ret
  }
}
%tint_construct_S = func(%a:array<f32, 2>):S {
  $B2: {
    %7:ptr<function, S, read_write> = var undef
    %8:ptr<function, array<f32, 2>, read_write> = access %7, 0u
    store %8, %a
    %9:S = load %7
    ret %9
  }
}
)";
    Run(HoistConstantArrayInitializers);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_HoistConstantArrayInitializersTest, ArrayOfStructsWithArray) {
    auto* str_ty =
        ty.Struct(mod.symbols.New("Inner"), {
                                                {mod.symbols.New("a"), ty.array<f32, 2>()},
                                            });
    auto* arr_ty = ty.array(str_ty, 2);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a1 = b.Composite(ty.array<f32, 2>(), 1_f, 2_f);
        auto* s1 = b.Composite(str_ty, a1);
        auto* a2 = b.Composite(ty.array<f32, 2>(), 3_f, 4_f);
        auto* s2 = b.Composite(str_ty, a2);
        b.Var<function>("arr", b.Composite(arr_ty, s1, s2));
        b.Return(func);
    });

    auto* src = R"(
Inner = struct @align(4) {
  a:array<f32, 2> @offset(0)
}

%foo = @fragment func():void {
  $B1: {
    %arr:ptr<function, array<Inner, 2>, read_write> = var array<Inner, 2>(Inner(array<f32, 2>(1.0f, 2.0f)), Inner(array<f32, 2>(3.0f, 4.0f)))
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Inner = struct @align(4) {
  a:array<f32, 2> @offset(0)
}

%foo = @fragment func():void {
  $B1: {
    %2:array<f32, 2> = let array<f32, 2>(1.0f, 2.0f)
    %3:Inner = call %tint_construct_Inner, %2
    %5:array<f32, 2> = let array<f32, 2>(3.0f, 4.0f)
    %6:Inner = call %tint_construct_Inner, %5
    %7:array<Inner, 2> = construct %3, %6
    %arr:ptr<function, array<Inner, 2>, read_write> = var %7
    ret
  }
}
%tint_construct_Inner = func(%a:array<f32, 2>):Inner {
  $B2: {
    %10:ptr<function, Inner, read_write> = var undef
    %11:ptr<function, array<f32, 2>, read_write> = access %10, 0u
    store %11, %a
    %12:Inner = load %10
    ret %12
  }
}
)";
    Run(HoistConstantArrayInitializers);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_HoistConstantArrayInitializersTest, StructWithArrayOfStructsWithArray) {
    auto* inner_ty =
        ty.Struct(mod.symbols.New("Inner"), {
                                                {mod.symbols.New("a"), ty.array<f32, 2>()},
                                            });
    auto* outer_ty =
        ty.Struct(mod.symbols.New("Outer"), {
                                                {mod.symbols.New("inners"), ty.array(inner_ty, 2)},
                                            });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a1 = b.Composite(ty.array<f32, 2>(), 1_f, 2_f);
        auto* s1 = b.Composite(inner_ty, a1);
        auto* a2 = b.Composite(ty.array<f32, 2>(), 3_f, 4_f);
        auto* s2 = b.Composite(inner_ty, a2);
        auto* arr = b.Composite(ty.array(inner_ty, 2), s1, s2);
        b.Var<function>("o", b.Composite(outer_ty, arr));
        b.Return(func);
    });

    auto* src = R"(
Inner = struct @align(4) {
  a:array<f32, 2> @offset(0)
}

Outer = struct @align(4) {
  inners:array<Inner, 2> @offset(0)
}

%foo = @fragment func():void {
  $B1: {
    %o:ptr<function, Outer, read_write> = var Outer(array<Inner, 2>(Inner(array<f32, 2>(1.0f, 2.0f)), Inner(array<f32, 2>(3.0f, 4.0f))))
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Inner = struct @align(4) {
  a:array<f32, 2> @offset(0)
}

Outer = struct @align(4) {
  inners:array<Inner, 2> @offset(0)
}

%foo = @fragment func():void {
  $B1: {
    %2:array<f32, 2> = let array<f32, 2>(1.0f, 2.0f)
    %3:Inner = call %tint_construct_Inner, %2
    %5:array<f32, 2> = let array<f32, 2>(3.0f, 4.0f)
    %6:Inner = call %tint_construct_Inner, %5
    %7:array<Inner, 2> = construct %3, %6
    %8:array<Inner, 2> = let %7
    %9:Outer = call %tint_construct_Outer, %8
    %o:ptr<function, Outer, read_write> = var %9
    ret
  }
}
%tint_construct_Inner = func(%a:array<f32, 2>):Inner {
  $B2: {
    %13:ptr<function, Inner, read_write> = var undef
    %14:ptr<function, array<f32, 2>, read_write> = access %13, 0u
    store %14, %a
    %15:Inner = load %13
    ret %15
  }
}
%tint_construct_Outer = func(%inners:array<Inner, 2>):Outer {
  $B3: {
    %17:ptr<function, Outer, read_write> = var undef
    %18:ptr<function, array<Inner, 2>, read_write> = access %17, 0u
    store %18, %inners
    %19:Outer = load %17
    ret %19
  }
}
)";
    Run(HoistConstantArrayInitializers);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_HoistConstantArrayInitializersTest, FunctionVarWithNonConstantInitializer) {
    auto* str_ty = ty.Struct(mod.symbols.New("S"), {
                                                       {mod.symbols.New("a"), ty.array<f32, 2>()},
                                                   });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* arr = b.Composite(ty.array<f32, 2>(), 1_f, 2_f);
        auto* s1 = b.Var<function>("s1", b.Composite(str_ty, arr));
        auto* load = b.Load(s1);
        b.Var<function>("s2", load);
        b.Return(func);
    });

    auto* src = R"(
S = struct @align(4) {
  a:array<f32, 2> @offset(0)
}

%foo = @fragment func():void {
  $B1: {
    %s1:ptr<function, S, read_write> = var S(array<f32, 2>(1.0f, 2.0f))
    %3:S = load %s1
    %s2:ptr<function, S, read_write> = var %3
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
S = struct @align(4) {
  a:array<f32, 2> @offset(0)
}

%foo = @fragment func():void {
  $B1: {
    %2:array<f32, 2> = let array<f32, 2>(1.0f, 2.0f)
    %3:S = call %tint_construct_S, %2
    %s1:ptr<function, S, read_write> = var %3
    %6:S = load %s1
    %s2:ptr<function, S, read_write> = var %6
    ret
  }
}
%tint_construct_S = func(%a:array<f32, 2>):S {
  $B2: {
    %9:ptr<function, S, read_write> = var undef
    %10:ptr<function, array<f32, 2>, read_write> = access %9, 0u
    store %10, %a
    %11:S = load %9
    ret %11
  }
}
)";
    Run(HoistConstantArrayInitializers);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_HoistConstantArrayInitializersTest, MultipleModuleScopeVars) {
    auto* str_ty = ty.Struct(mod.symbols.New("S"), {
                                                       {mod.symbols.New("a"), ty.array<f32, 2>()},
                                                   });

    b.Append(mod.root_block, [&] {
        auto* a1 = b.Composite(ty.array<f32, 2>(), 1_f, 2_f);
        b.Var<private_>("g1", b.Composite(str_ty, a1));
        auto* a2 = b.Composite(ty.array<f32, 2>(), 3_f, 4_f);
        b.Var<private_>("g2", b.Composite(str_ty, a2));
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] { b.Return(func); });

    auto* src = R"(
S = struct @align(4) {
  a:array<f32, 2> @offset(0)
}

$B1: {  # root
  %g1:ptr<private, S, read_write> = var S(array<f32, 2>(1.0f, 2.0f))
  %g2:ptr<private, S, read_write> = var S(array<f32, 2>(3.0f, 4.0f))
}

%foo = @fragment func():void {
  $B2: {
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
S = struct @align(4) {
  a:array<f32, 2> @offset(0)
}

$B1: {  # root
  %g1:ptr<private, S, read_write> = var undef
  %g2:ptr<private, S, read_write> = var undef
}

%foo = @fragment func():void {
  $B2: {
    %4:array<f32, 2> = let array<f32, 2>(3.0f, 4.0f)
    %5:S = call %tint_construct_S, %4
    store %g2, %5
    %7:array<f32, 2> = let array<f32, 2>(1.0f, 2.0f)
    %8:S = call %tint_construct_S, %7
    store %g1, %8
    ret
  }
}
%tint_construct_S = func(%a:array<f32, 2>):S {
  $B3: {
    %10:ptr<function, S, read_write> = var undef
    %11:ptr<function, array<f32, 2>, read_write> = access %10, 0u
    store %11, %a
    %12:S = load %10
    ret %12
  }
}
)";
    Run(HoistConstantArrayInitializers);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_HoistConstantArrayInitializersTest, StructWithMultiDimensionalArray) {
    auto* arr_inner_ty = ty.array<f32, 2>();
    auto* arr_outer_ty = ty.array(arr_inner_ty, 2);
    auto* str_ty = ty.Struct(mod.symbols.New("S"), {
                                                       {mod.symbols.New("a"), arr_outer_ty},
                                                   });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* a1 = b.Composite(arr_inner_ty, 1_f, 2_f);
        auto* a2 = b.Composite(arr_inner_ty, 3_f, 4_f);
        auto* arr = b.Composite(arr_outer_ty, a1, a2);
        b.Var<function>("s", b.Composite(str_ty, arr));
        b.Return(func);
    });

    auto* src = R"(
S = struct @align(4) {
  a:array<array<f32, 2>, 2> @offset(0)
}

%foo = @fragment func():void {
  $B1: {
    %s:ptr<function, S, read_write> = var S(array<array<f32, 2>, 2>(array<f32, 2>(1.0f, 2.0f), array<f32, 2>(3.0f, 4.0f)))
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
S = struct @align(4) {
  a:array<array<f32, 2>, 2> @offset(0)
}

%foo = @fragment func():void {
  $B1: {
    %2:array<array<f32, 2>, 2> = let array<array<f32, 2>, 2>(array<f32, 2>(1.0f, 2.0f), array<f32, 2>(3.0f, 4.0f))
    %3:S = call %tint_construct_S, %2
    %s:ptr<function, S, read_write> = var %3
    ret
  }
}
%tint_construct_S = func(%a:array<array<f32, 2>, 2>):S {
  $B2: {
    %7:ptr<function, S, read_write> = var undef
    %8:ptr<function, array<array<f32, 2>, 2>, read_write> = access %7, 0u
    store %8, %a
    %9:S = load %7
    ret %9
  }
}
)";
    Run(HoistConstantArrayInitializers);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_HoistConstantArrayInitializersTest, UsageInLetAndReturn) {
    auto* str_ty = ty.Struct(mod.symbols.New("S"), {
                                                       {mod.symbols.New("a"), ty.array<f32, 2>()},
                                                   });

    auto* helper = b.Function("helper", str_ty);
    b.Append(helper->Block(), [&] {
        auto* arr = b.Composite(ty.array<f32, 2>(), 1_f, 2_f);
        auto* s = b.Composite(str_ty, arr);
        b.Let("l", s);
        b.Return(helper, s);
    });

    auto* ep = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(ep->Block(), [&] {
        b.Call(helper);
        b.Return(ep);
    });

    auto* src = R"(
S = struct @align(4) {
  a:array<f32, 2> @offset(0)
}

%helper = func():S {
  $B1: {
    %l:S = let S(array<f32, 2>(1.0f, 2.0f))
    ret S(array<f32, 2>(1.0f, 2.0f))
  }
}
%foo = @fragment func():void {
  $B2: {
    %4:S = call %helper
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
S = struct @align(4) {
  a:array<f32, 2> @offset(0)
}

%helper = func():S {
  $B1: {
    %2:array<f32, 2> = let array<f32, 2>(1.0f, 2.0f)
    %3:S = call %tint_construct_S, %2
    %l:S = let %3
    %6:array<f32, 2> = let array<f32, 2>(1.0f, 2.0f)
    %7:S = call %tint_construct_S, %6
    ret %7
  }
}
%foo = @fragment func():void {
  $B2: {
    %9:S = call %helper
    ret
  }
}
%tint_construct_S = func(%a:array<f32, 2>):S {
  $B3: {
    %11:ptr<function, S, read_write> = var undef
    %12:ptr<function, array<f32, 2>, read_write> = access %11, 0u
    store %12, %a
    %13:S = load %11
    ret %13
  }
}
)";
    Run(HoistConstantArrayInitializers);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_HoistConstantArrayInitializersTest, WorkgroupVarWithAtomics) {
    auto* str_ty =
        ty.Struct(mod.symbols.New("S"), {
                                            {mod.symbols.New("a"), ty.array(ty.atomic<i32>(), 4_u)},
                                        });

    b.Append(mod.root_block, [&] { b.Var("wg", workgroup, str_ty); });

    auto* func = b.ComputeFunction("foo");
    b.Append(func->Block(), [&] { b.Return(func); });

    auto* src = R"(
S = struct @align(4) {
  a:array<atomic<i32>, 4> @offset(0)
}

$B1: {  # root
  %wg:ptr<workgroup, S, read_write> = var
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;
    Run(HoistConstantArrayInitializers);
    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_HoistConstantArrayInitializersTest, ManyStructMembers) {
    auto* str_ty = ty.Struct(mod.symbols.New("S"), {
                                                       {mod.symbols.New("a"), ty.array<f32, 2>()},
                                                       {mod.symbols.New("b"), ty.array<f32, 2>()},
                                                       {mod.symbols.New("c"), ty.array<f32, 2>()},
                                                       {mod.symbols.New("d"), ty.array<f32, 2>()},
                                                       {mod.symbols.New("e"), ty.array<f32, 2>()},
                                                       {mod.symbols.New("f"), ty.array<f32, 2>()},
                                                       {mod.symbols.New("g"), ty.array<f32, 2>()},
                                                       {mod.symbols.New("h"), ty.array<f32, 2>()},
                                                       {mod.symbols.New("i"), ty.array<f32, 2>()},
                                                       {mod.symbols.New("j"), ty.array<f32, 2>()},
                                                   });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* arr = b.Composite(ty.array<f32, 2>(), 1_f, 2_f);
        b.Var<function>("s", b.Composite(str_ty, arr, arr, arr, arr, arr, arr, arr, arr, arr, arr));
        b.Return(func);
    });

    auto* src = R"(
S = struct @align(4) {
  a:array<f32, 2> @offset(0)
  b:array<f32, 2> @offset(8)
  c:array<f32, 2> @offset(16)
  d:array<f32, 2> @offset(24)
  e:array<f32, 2> @offset(32)
  f:array<f32, 2> @offset(40)
  g:array<f32, 2> @offset(48)
  h:array<f32, 2> @offset(56)
  i:array<f32, 2> @offset(64)
  j:array<f32, 2> @offset(72)
}

%foo = @fragment func():void {
  $B1: {
    %s:ptr<function, S, read_write> = var S(array<f32, 2>(1.0f, 2.0f))
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
S = struct @align(4) {
  a:array<f32, 2> @offset(0)
  b:array<f32, 2> @offset(8)
  c:array<f32, 2> @offset(16)
  d:array<f32, 2> @offset(24)
  e:array<f32, 2> @offset(32)
  f:array<f32, 2> @offset(40)
  g:array<f32, 2> @offset(48)
  h:array<f32, 2> @offset(56)
  i:array<f32, 2> @offset(64)
  j:array<f32, 2> @offset(72)
}

%foo = @fragment func():void {
  $B1: {
    %2:array<f32, 2> = let array<f32, 2>(1.0f, 2.0f)
    %3:array<f32, 2> = let array<f32, 2>(1.0f, 2.0f)
    %4:array<f32, 2> = let array<f32, 2>(1.0f, 2.0f)
    %5:array<f32, 2> = let array<f32, 2>(1.0f, 2.0f)
    %6:array<f32, 2> = let array<f32, 2>(1.0f, 2.0f)
    %7:array<f32, 2> = let array<f32, 2>(1.0f, 2.0f)
    %8:array<f32, 2> = let array<f32, 2>(1.0f, 2.0f)
    %9:array<f32, 2> = let array<f32, 2>(1.0f, 2.0f)
    %10:array<f32, 2> = let array<f32, 2>(1.0f, 2.0f)
    %11:array<f32, 2> = let array<f32, 2>(1.0f, 2.0f)
    %12:S = call %tint_construct_S, %2, %3, %4, %5, %6, %7, %8, %9, %10, %11
    %s:ptr<function, S, read_write> = var %12
    ret
  }
}
%tint_construct_S = func(%a:array<f32, 2>, %b:array<f32, 2>, %c:array<f32, 2>, %d:array<f32, 2>, %e:array<f32, 2>, %f:array<f32, 2>, %g:array<f32, 2>, %h:array<f32, 2>, %i:array<f32, 2>, %j:array<f32, 2>):S {
  $B2: {
    %25:ptr<function, S, read_write> = var undef
    %26:ptr<function, array<f32, 2>, read_write> = access %25, 0u
    store %26, %a
    %27:ptr<function, array<f32, 2>, read_write> = access %25, 1u
    store %27, %b
    %28:ptr<function, array<f32, 2>, read_write> = access %25, 2u
    store %28, %c
    %29:ptr<function, array<f32, 2>, read_write> = access %25, 3u
    store %29, %d
    %30:ptr<function, array<f32, 2>, read_write> = access %25, 4u
    store %30, %e
    %31:ptr<function, array<f32, 2>, read_write> = access %25, 5u
    store %31, %f
    %32:ptr<function, array<f32, 2>, read_write> = access %25, 6u
    store %32, %g
    %33:ptr<function, array<f32, 2>, read_write> = access %25, 7u
    store %33, %h
    %34:ptr<function, array<f32, 2>, read_write> = access %25, 8u
    store %34, %i
    %35:ptr<function, array<f32, 2>, read_write> = access %25, 9u
    store %35, %j
    %36:S = load %25
    ret %36
  }
}
)";
    Run(HoistConstantArrayInitializers);
    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::glsl::writer::raise
