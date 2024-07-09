// Copyright 2024 The Dawn & Tint Authors
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

#include "src/tint/lang/hlsl/writer/raise/decompose_uniform_access.h"

#include <gtest/gtest.h>

#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/ir/function.h"
#include "src/tint/lang/core/ir/transform/helper_test.h"
#include "src/tint/lang/core/number.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::hlsl::writer::raise {
namespace {

using HlslWriterDecomposeUniformAccessTest = core::ir::transform::TransformTest;

TEST_F(HlslWriterDecomposeUniformAccessTest, NoBufferAccess) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] { b.Return(func); });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;
    Run(DecomposeUniformAccess);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterDecomposeUniformAccessTest, DISABLED_UniformAccessChainFromUnnamedAccessChain) {
    auto* Inner = ty.Struct(mod.symbols.New("Inner"), {
                                                          {mod.symbols.New("c"), ty.f32()},
                                                          {mod.symbols.New("d"), ty.u32()},
                                                      });

    tint::Vector<const core::type::StructMember*, 2> members;
    members.Push(ty.Get<core::type::StructMember>(mod.symbols.New("a"), ty.i32(), 0u, 0u, 4u,
                                                  ty.i32()->Size(), core::IOAttributes{}));
    members.Push(ty.Get<core::type::StructMember>(mod.symbols.New("b"), Inner, 1u, 16u, 16u,
                                                  Inner->Size(), core::IOAttributes{}));
    auto* sb = ty.Struct(mod.symbols.New("SB"), members);

    auto* var = b.Var("v", uniform, sb, core::Access::kRead);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* x = b.Access(ty.ptr(uniform, sb, core::Access::kRead), var);
        auto* y = b.Access(ty.ptr(uniform, Inner, core::Access::kRead), x->Result(0), 1_u);
        b.Let("b",
              b.Load(b.Access(ty.ptr(uniform, ty.u32(), core::Access::kRead), y->Result(0), 1_u)));
        b.Return(func);
    });

    auto* src = R"(
Inner = struct @align(4) {
  c:f32 @offset(0)
  d:u32 @offset(4)
}

SB = struct @align(16) {
  a:i32 @offset(0)
  b:Inner @offset(16)
}

$B1: {  # root
  %v:ptr<uniform, SB, read> = var @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<uniform, SB, read> = access %v
    %4:ptr<uniform, Inner, read> = access %3, 1u
    %5:ptr<uniform, u32, read> = access %4, 1u
    %6:u32 = load %5
    %b:u32 = let %6
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
Inner = struct @align(4) {
  c:f32 @offset(0)
  d:u32 @offset(4)
}

SB = struct @align(16) {
  a:i32 @offset(0)
  b:Inner @offset(16)
}

$B1: {  # root
  %v:hlsl.byte_address_buffer<read> = var @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:u32 = %v.Load 8u
    %4:u32 = bitcast %3
    %b:u32 = let %4
    ret
  }
}
)";

    Run(DecomposeUniformAccess);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterDecomposeUniformAccessTest, DISABLED_UniformAccessChainFromLetAccessChain) {
    auto* Inner = ty.Struct(mod.symbols.New("Inner"), {
                                                          {mod.symbols.New("c"), ty.f32()},
                                                      });

    tint::Vector<const core::type::StructMember*, 2> members;
    members.Push(ty.Get<core::type::StructMember>(mod.symbols.New("a"), ty.i32(), 0u, 0u, 4u,
                                                  ty.i32()->Size(), core::IOAttributes{}));
    members.Push(ty.Get<core::type::StructMember>(mod.symbols.New("b"), Inner, 1u, 16u, 16u,
                                                  Inner->Size(), core::IOAttributes{}));
    auto* sb = ty.Struct(mod.symbols.New("SB"), members);

    auto* var = b.Var("v", uniform, sb, core::Access::kRead);
    var->SetBindingPoint(0, 0);
    b.ir.root_block->Append(var);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* x = b.Let("x", var);
        auto* y =
            b.Let("y", b.Access(ty.ptr(uniform, Inner, core::Access::kRead), x->Result(0), 1_u));
        auto* z =
            b.Let("z", b.Access(ty.ptr(uniform, ty.f32(), core::Access::kRead), y->Result(0), 0_u));
        b.Let("a", b.Load(z));
        b.Return(func);
    });

    auto* src = R"(
Inner = struct @align(4) {
  c:f32 @offset(0)
}

SB = struct @align(16) {
  a:i32 @offset(0)
  b:Inner @offset(16)
}

$B1: {  # root
  %v:ptr<uniform, SB, read> = var @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %x:ptr<uniform, SB, read> = let %v
    %4:ptr<uniform, Inner, read> = access %x, 1u
    %y:ptr<uniform, Inner, read> = let %4
    %6:ptr<uniform, f32, read> = access %y, 0u
    %z:ptr<uniform, f32, read> = let %6
    %8:f32 = load %z
    %a:f32 = let %8
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
Inner = struct @align(4) {
  c:f32 @offset(0)
}

SB = struct @align(16) {
  a:i32 @offset(0)
  b:Inner @offset(16)
}

$B1: {  # root
  %v:hlsl.byte_address_buffer<read> = var @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:u32 = %v.Load 4u
    %4:f32 = bitcast %3
    %a:f32 = let %4
    ret
  }
}
)";

    Run(DecomposeUniformAccess);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterDecomposeUniformAccessTest, UniformAccessVectorLoad) {
    auto* var = b.Var<uniform, vec4<f32>, core::Access::kRead>("v");
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("a", b.Load(var));
        b.Let("b", b.LoadVectorElement(var, 0_u));
        b.Let("c", b.LoadVectorElement(var, 1_u));
        b.Let("d", b.LoadVectorElement(var, 2_u));
        b.Let("e", b.LoadVectorElement(var, 3_u));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<uniform, vec4<f32>, read> = var @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:vec4<f32> = load %v
    %a:vec4<f32> = let %3
    %5:f32 = load_vector_element %v, 0u
    %b:f32 = let %5
    %7:f32 = load_vector_element %v, 1u
    %c:f32 = let %7
    %9:f32 = load_vector_element %v, 2u
    %d:f32 = let %9
    %11:f32 = load_vector_element %v, 3u
    %e:f32 = let %11
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:ptr<uniform, array<vec4<u32>, 1>, read> = var @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:ptr<uniform, vec4<u32>, read> = access %v, 0u
    %4:vec4<u32> = load %3
    %5:vec4<f32> = bitcast %4
    %a:vec4<f32> = let %5
    %7:ptr<uniform, vec4<u32>, read> = access %v, 0u
    %8:u32 = load_vector_element %7, 0u
    %9:f32 = bitcast %8
    %b:f32 = let %9
    %11:ptr<uniform, vec4<u32>, read> = access %v, 0u
    %12:u32 = load_vector_element %11, 1u
    %13:f32 = bitcast %12
    %c:f32 = let %13
    %15:ptr<uniform, vec4<u32>, read> = access %v, 0u
    %16:u32 = load_vector_element %15, 2u
    %17:f32 = bitcast %16
    %d:f32 = let %17
    %19:ptr<uniform, vec4<u32>, read> = access %v, 0u
    %20:u32 = load_vector_element %19, 3u
    %21:f32 = bitcast %20
    %e:f32 = let %21
    ret
  }
}
)";
    Run(DecomposeUniformAccess);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterDecomposeUniformAccessTest, DISABLED_UniformAccessVectorF16) {
    auto* var = b.Var<uniform, vec4<f16>, core::Access::kRead>("v");
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("a", b.Load(var));
        b.Let("b", b.LoadVectorElement(var, 0_u));
        b.Let("c", b.LoadVectorElement(var, 1_u));
        b.Let("d", b.LoadVectorElement(var, 2_u));
        b.Let("e", b.LoadVectorElement(var, 3_u));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<uniform, vec4<f16>, read> = var @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:vec4<f16> = load %v
    %a:vec4<f16> = let %3
    %5:f16 = load_vector_element %v, 0u
    %b:f16 = let %5
    %7:f16 = load_vector_element %v, 1u
    %c:f16 = let %7
    %9:f16 = load_vector_element %v, 2u
    %d:f16 = let %9
    %11:f16 = load_vector_element %v, 3u
    %e:f16 = let %11
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:hlsl.byte_address_buffer<read> = var @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:vec4<f16> = %v.Load4F16 0u
    %a:vec4<f16> = let %3
    %5:f16 = %v.LoadF16 0u
    %b:f16 = let %5
    %7:f16 = %v.LoadF16 2u
    %c:f16 = let %7
    %9:f16 = %v.LoadF16 4u
    %d:f16 = let %9
    %11:f16 = %v.LoadF16 6u
    %e:f16 = let %11
    ret
  }
}
)";
    Run(DecomposeUniformAccess);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterDecomposeUniformAccessTest, UniformAccessMatrix) {
    auto* var = b.Var<uniform, mat4x4<f32>, core::Access::kRead>("v");
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("a", b.Load(var));
        b.Let("b", b.Load(b.Access(ty.ptr<uniform, vec4<f32>, core::Access::kRead>(), var, 3_u)));
        b.Let("c", b.LoadVectorElement(
                       b.Access(ty.ptr<uniform, vec4<f32>, core::Access::kRead>(), var, 1_u), 2_u));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<uniform, mat4x4<f32>, read> = var @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:mat4x4<f32> = load %v
    %a:mat4x4<f32> = let %3
    %5:ptr<uniform, vec4<f32>, read> = access %v, 3u
    %6:vec4<f32> = load %5
    %b:vec4<f32> = let %6
    %8:ptr<uniform, vec4<f32>, read> = access %v, 1u
    %9:f32 = load_vector_element %8, 2u
    %c:f32 = let %9
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:ptr<uniform, array<vec4<u32>, 4>, read> = var @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:mat4x4<f32> = call %4, 0u
    %a:mat4x4<f32> = let %3
    %6:ptr<uniform, vec4<u32>, read> = access %v, 3u
    %7:vec4<u32> = load %6
    %8:vec4<f32> = bitcast %7
    %b:vec4<f32> = let %8
    %10:ptr<uniform, vec4<u32>, read> = access %v, 1u
    %11:u32 = load_vector_element %10, 2u
    %12:f32 = bitcast %11
    %c:f32 = let %12
    ret
  }
}
%4 = func(%start_byte_offset:u32):mat4x4<f32> {
  $B3: {
    %15:u32 = div %start_byte_offset, 16u
    %16:u32 = mod %start_byte_offset, 16u
    %17:u32 = div %16, 4u
    %18:ptr<uniform, vec4<u32>, read> = access %v, %15
    %19:vec4<u32> = load %18
    %20:vec4<f32> = bitcast %19
    %21:u32 = add 16u, %start_byte_offset
    %22:u32 = div %21, 16u
    %23:u32 = mod %21, 16u
    %24:u32 = div %23, 4u
    %25:ptr<uniform, vec4<u32>, read> = access %v, %22
    %26:vec4<u32> = load %25
    %27:vec4<f32> = bitcast %26
    %28:u32 = add 32u, %start_byte_offset
    %29:u32 = div %28, 16u
    %30:u32 = mod %28, 16u
    %31:u32 = div %30, 4u
    %32:ptr<uniform, vec4<u32>, read> = access %v, %29
    %33:vec4<u32> = load %32
    %34:vec4<f32> = bitcast %33
    %35:u32 = add 48u, %start_byte_offset
    %36:u32 = div %35, 16u
    %37:u32 = mod %35, 16u
    %38:u32 = div %37, 4u
    %39:ptr<uniform, vec4<u32>, read> = access %v, %36
    %40:vec4<u32> = load %39
    %41:vec4<f32> = bitcast %40
    %42:mat4x4<f32> = construct %20, %27, %34, %41
    ret %42
  }
}
)";
    Run(DecomposeUniformAccess);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterDecomposeUniformAccessTest, UniformAccessArray) {
    auto* var = b.Var<uniform, array<vec3<f32>, 5>, core::Access::kRead>("v");
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("a", b.Load(var));
        b.Let("b", b.Load(b.Access(ty.ptr<uniform, vec3<f32>, core::Access::kRead>(), var, 3_u)));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<uniform, array<vec3<f32>, 5>, read> = var @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:array<vec3<f32>, 5> = load %v
    %a:array<vec3<f32>, 5> = let %3
    %5:ptr<uniform, vec3<f32>, read> = access %v, 3u
    %6:vec3<f32> = load %5
    %b:vec3<f32> = let %6
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:ptr<uniform, array<vec4<u32>, 5>, read> = var @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:array<vec3<f32>, 5> = call %4, 0u
    %a:array<vec3<f32>, 5> = let %3
    %6:ptr<uniform, vec4<u32>, read> = access %v, 3u
    %7:vec4<u32> = load %6
    %8:vec3<u32> = swizzle %7, xyz
    %9:vec3<f32> = bitcast %8
    %b:vec3<f32> = let %9
    ret
  }
}
%4 = func(%start_byte_offset:u32):array<vec3<f32>, 5> {
  $B3: {
    %a_1:ptr<function, array<vec3<f32>, 5>, read_write> = var, array<vec3<f32>, 5>(vec3<f32>(0.0f))  # %a_1: 'a'
    loop [i: $B4, b: $B5, c: $B6] {  # loop_1
      $B4: {  # initializer
        next_iteration 0u  # -> $B5
      }
      $B5 (%idx:u32): {  # body
        %14:bool = gte %idx, 5u
        if %14 [t: $B7] {  # if_1
          $B7: {  # true
            exit_loop  # loop_1
          }
        }
        %15:u32 = mul %idx, 16u
        %16:u32 = add %start_byte_offset, %15
        %17:ptr<function, vec3<f32>, read_write> = access %a_1, %idx
        %18:u32 = div %16, 16u
        %19:u32 = mod %16, 16u
        %20:u32 = div %19, 4u
        %21:ptr<uniform, vec4<u32>, read> = access %v, %18
        %22:vec4<u32> = load %21
        %23:vec3<u32> = swizzle %22, xyz
        %24:vec3<f32> = bitcast %23
        store %17, %24
        continue  # -> $B6
      }
      $B6: {  # continuing
        %25:u32 = add %idx, 1u
        next_iteration %25  # -> $B5
      }
    }
    %26:array<vec3<f32>, 5> = load %a_1
    ret %26
  }
}
)";
    Run(DecomposeUniformAccess);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterDecomposeUniformAccessTest, UniformAccessArrayWhichCanHaveSizesOtherThenFive) {
    auto* var = b.Var<uniform, array<vec3<f32>, 42>, core::Access::kRead>("v");
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("a", b.Load(var));
        b.Let("b", b.Load(b.Access(ty.ptr<uniform, vec3<f32>, core::Access::kRead>(), var, 3_u)));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<uniform, array<vec3<f32>, 42>, read> = var @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:array<vec3<f32>, 42> = load %v
    %a:array<vec3<f32>, 42> = let %3
    %5:ptr<uniform, vec3<f32>, read> = access %v, 3u
    %6:vec3<f32> = load %5
    %b:vec3<f32> = let %6
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %v:ptr<uniform, array<vec4<u32>, 42>, read> = var @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:array<vec3<f32>, 42> = call %4, 0u
    %a:array<vec3<f32>, 42> = let %3
    %6:ptr<uniform, vec4<u32>, read> = access %v, 3u
    %7:vec4<u32> = load %6
    %8:vec3<u32> = swizzle %7, xyz
    %9:vec3<f32> = bitcast %8
    %b:vec3<f32> = let %9
    ret
  }
}
%4 = func(%start_byte_offset:u32):array<vec3<f32>, 42> {
  $B3: {
    %a_1:ptr<function, array<vec3<f32>, 42>, read_write> = var, array<vec3<f32>, 42>(vec3<f32>(0.0f))  # %a_1: 'a'
    loop [i: $B4, b: $B5, c: $B6] {  # loop_1
      $B4: {  # initializer
        next_iteration 0u  # -> $B5
      }
      $B5 (%idx:u32): {  # body
        %14:bool = gte %idx, 42u
        if %14 [t: $B7] {  # if_1
          $B7: {  # true
            exit_loop  # loop_1
          }
        }
        %15:u32 = mul %idx, 16u
        %16:u32 = add %start_byte_offset, %15
        %17:ptr<function, vec3<f32>, read_write> = access %a_1, %idx
        %18:u32 = div %16, 16u
        %19:u32 = mod %16, 16u
        %20:u32 = div %19, 4u
        %21:ptr<uniform, vec4<u32>, read> = access %v, %18
        %22:vec4<u32> = load %21
        %23:vec3<u32> = swizzle %22, xyz
        %24:vec3<f32> = bitcast %23
        store %17, %24
        continue  # -> $B6
      }
      $B6: {  # continuing
        %25:u32 = add %idx, 1u
        next_iteration %25  # -> $B5
      }
    }
    %26:array<vec3<f32>, 42> = load %a_1
    ret %26
  }
}
)";
    Run(DecomposeUniformAccess);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterDecomposeUniformAccessTest, UniformAccessStruct) {
    auto* SB = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("a"), ty.i32()},
                                                    {mod.symbols.New("b"), ty.f32()},
                                                });

    auto* var = b.Var("v", uniform, SB, core::Access::kRead);
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("a", b.Load(var));
        b.Let("b", b.Load(b.Access(ty.ptr<uniform, f32, core::Access::kRead>(), var, 1_u)));
        b.Return(func);
    });

    auto* src = R"(
SB = struct @align(4) {
  a:i32 @offset(0)
  b:f32 @offset(4)
}

$B1: {  # root
  %v:ptr<uniform, SB, read> = var @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:SB = load %v
    %a:SB = let %3
    %5:ptr<uniform, f32, read> = access %v, 1u
    %6:f32 = load %5
    %b:f32 = let %6
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
SB = struct @align(4) {
  a:i32 @offset(0)
  b:f32 @offset(4)
}

$B1: {  # root
  %v:ptr<uniform, array<vec4<u32>, 1>, read> = var @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:SB = call %4, 0u
    %a:SB = let %3
    %6:ptr<uniform, vec4<u32>, read> = access %v, 0u
    %7:u32 = load_vector_element %6, 1u
    %8:f32 = bitcast %7
    %b:f32 = let %8
    ret
  }
}
%4 = func(%start_byte_offset:u32):SB {
  $B3: {
    %11:u32 = div %start_byte_offset, 16u
    %12:u32 = mod %start_byte_offset, 16u
    %13:u32 = div %12, 4u
    %14:ptr<uniform, vec4<u32>, read> = access %v, %11
    %15:u32 = load_vector_element %14, %13
    %16:i32 = bitcast %15
    %17:u32 = add 4u, %start_byte_offset
    %18:u32 = div %17, 16u
    %19:u32 = mod %17, 16u
    %20:u32 = div %19, 4u
    %21:ptr<uniform, vec4<u32>, read> = access %v, %18
    %22:u32 = load_vector_element %21, %20
    %23:f32 = bitcast %22
    %24:SB = construct %16, %23
    ret %24
  }
}
)";
    Run(DecomposeUniformAccess);
    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterDecomposeUniformAccessTest, UniformAccessStructNested) {
    auto* Inner =
        ty.Struct(mod.symbols.New("Inner"), {
                                                {mod.symbols.New("s"), ty.mat3x3<f32>()},
                                                {mod.symbols.New("t"), ty.array<vec3<f32>, 5>()},
                                            });
    auto* Outer = ty.Struct(mod.symbols.New("Outer"), {
                                                          {mod.symbols.New("x"), ty.f32()},
                                                          {mod.symbols.New("y"), Inner},
                                                      });

    auto* SB = ty.Struct(mod.symbols.New("SB"), {
                                                    {mod.symbols.New("a"), ty.i32()},
                                                    {mod.symbols.New("b"), Outer},
                                                });

    auto* var = b.Var("v", uniform, SB, core::Access::kRead);
    var->SetBindingPoint(0, 0);

    b.ir.root_block->Append(var);
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Let("a", b.Load(var));
        b.Let("b", b.LoadVectorElement(b.Access(ty.ptr<uniform, vec3<f32>, core::Access::kRead>(),
                                                var, 1_u, 1_u, 1_u, 3_u),
                                       2_u));
        b.Return(func);
    });

    auto* src = R"(
Inner = struct @align(16) {
  s:mat3x3<f32> @offset(0)
  t:array<vec3<f32>, 5> @offset(48)
}

Outer = struct @align(16) {
  x:f32 @offset(0)
  y:Inner @offset(16)
}

SB = struct @align(16) {
  a:i32 @offset(0)
  b:Outer @offset(16)
}

$B1: {  # root
  %v:ptr<uniform, SB, read> = var @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:SB = load %v
    %a:SB = let %3
    %5:ptr<uniform, vec3<f32>, read> = access %v, 1u, 1u, 1u, 3u
    %6:f32 = load_vector_element %5, 2u
    %b:f32 = let %6
    ret
  }
}
)";
    ASSERT_EQ(src, str());

    auto* expect = R"(
Inner = struct @align(16) {
  s:mat3x3<f32> @offset(0)
  t:array<vec3<f32>, 5> @offset(48)
}

Outer = struct @align(16) {
  x:f32 @offset(0)
  y:Inner @offset(16)
}

SB = struct @align(16) {
  a:i32 @offset(0)
  b:Outer @offset(16)
}

$B1: {  # root
  %v:ptr<uniform, array<vec4<u32>, 10>, read> = var @binding_point(0, 0)
}

%foo = @fragment func():void {
  $B2: {
    %3:SB = call %4, 0u
    %a:SB = let %3
    %6:ptr<uniform, vec4<u32>, read> = access %v, 8u
    %7:u32 = load_vector_element %6, 2u
    %8:f32 = bitcast %7
    %b:f32 = let %8
    ret
  }
}
%4 = func(%start_byte_offset:u32):SB {
  $B3: {
    %11:u32 = div %start_byte_offset, 16u
    %12:u32 = mod %start_byte_offset, 16u
    %13:u32 = div %12, 4u
    %14:ptr<uniform, vec4<u32>, read> = access %v, %11
    %15:u32 = load_vector_element %14, %13
    %16:i32 = bitcast %15
    %17:u32 = add 16u, %start_byte_offset
    %18:u32 = div %17, 16u
    %19:u32 = mod %17, 16u
    %20:u32 = div %19, 4u
    %21:Outer = call %22, %17
    %23:SB = construct %16, %21
    ret %23
  }
}
%22 = func(%start_byte_offset_1:u32):Outer {  # %start_byte_offset_1: 'start_byte_offset'
  $B4: {
    %25:u32 = div %start_byte_offset_1, 16u
    %26:u32 = mod %start_byte_offset_1, 16u
    %27:u32 = div %26, 4u
    %28:ptr<uniform, vec4<u32>, read> = access %v, %25
    %29:u32 = load_vector_element %28, %27
    %30:f32 = bitcast %29
    %31:u32 = add 16u, %start_byte_offset_1
    %32:u32 = div %31, 16u
    %33:u32 = mod %31, 16u
    %34:u32 = div %33, 4u
    %35:Inner = call %36, %31
    %37:Outer = construct %30, %35
    ret %37
  }
}
%36 = func(%start_byte_offset_2:u32):Inner {  # %start_byte_offset_2: 'start_byte_offset'
  $B5: {
    %39:u32 = div %start_byte_offset_2, 16u
    %40:u32 = mod %start_byte_offset_2, 16u
    %41:u32 = div %40, 4u
    %42:mat3x3<f32> = call %43, %start_byte_offset_2
    %44:u32 = add 48u, %start_byte_offset_2
    %45:u32 = div %44, 16u
    %46:u32 = mod %44, 16u
    %47:u32 = div %46, 4u
    %48:array<vec3<f32>, 5> = call %49, %44
    %50:Inner = construct %42, %48
    ret %50
  }
}
%43 = func(%start_byte_offset_3:u32):mat3x3<f32> {  # %start_byte_offset_3: 'start_byte_offset'
  $B6: {
    %52:u32 = div %start_byte_offset_3, 16u
    %53:u32 = mod %start_byte_offset_3, 16u
    %54:u32 = div %53, 4u
    %55:ptr<uniform, vec4<u32>, read> = access %v, %52
    %56:vec4<u32> = load %55
    %57:vec3<u32> = swizzle %56, xyz
    %58:vec3<f32> = bitcast %57
    %59:u32 = add 16u, %start_byte_offset_3
    %60:u32 = div %59, 16u
    %61:u32 = mod %59, 16u
    %62:u32 = div %61, 4u
    %63:ptr<uniform, vec4<u32>, read> = access %v, %60
    %64:vec4<u32> = load %63
    %65:vec3<u32> = swizzle %64, xyz
    %66:vec3<f32> = bitcast %65
    %67:u32 = add 32u, %start_byte_offset_3
    %68:u32 = div %67, 16u
    %69:u32 = mod %67, 16u
    %70:u32 = div %69, 4u
    %71:ptr<uniform, vec4<u32>, read> = access %v, %68
    %72:vec4<u32> = load %71
    %73:vec3<u32> = swizzle %72, xyz
    %74:vec3<f32> = bitcast %73
    %75:mat3x3<f32> = construct %58, %66, %74
    ret %75
  }
}
%49 = func(%start_byte_offset_4:u32):array<vec3<f32>, 5> {  # %start_byte_offset_4: 'start_byte_offset'
  $B7: {
    %a_1:ptr<function, array<vec3<f32>, 5>, read_write> = var, array<vec3<f32>, 5>(vec3<f32>(0.0f))  # %a_1: 'a'
    loop [i: $B8, b: $B9, c: $B10] {  # loop_1
      $B8: {  # initializer
        next_iteration 0u  # -> $B9
      }
      $B9 (%idx:u32): {  # body
        %79:bool = gte %idx, 5u
        if %79 [t: $B11] {  # if_1
          $B11: {  # true
            exit_loop  # loop_1
          }
        }
        %80:u32 = mul %idx, 16u
        %81:u32 = add %start_byte_offset_4, %80
        %82:ptr<function, vec3<f32>, read_write> = access %a_1, %idx
        %83:u32 = div %81, 16u
        %84:u32 = mod %81, 16u
        %85:u32 = div %84, 4u
        %86:ptr<uniform, vec4<u32>, read> = access %v, %83
        %87:vec4<u32> = load %86
        %88:vec3<u32> = swizzle %87, xyz
        %89:vec3<f32> = bitcast %88
        store %82, %89
        continue  # -> $B10
      }
      $B10: {  # continuing
        %90:u32 = add %idx, 1u
        next_iteration %90  # -> $B9
      }
    }
    %91:array<vec3<f32>, 5> = load %a_1
    ret %91
  }
}
)";
    Run(DecomposeUniformAccess);
    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::hlsl::writer::raise
