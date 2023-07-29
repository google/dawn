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

#include "src/tint/lang/core/ir/transform/std140.h"

#include <utility>

#include "src/tint/lang/core/ir/transform/test_helper.h"
#include "src/tint/lang/core/type/array.h"
#include "src/tint/lang/core/type/matrix.h"
#include "src/tint/lang/core/type/pointer.h"
#include "src/tint/lang/core/type/struct.h"

namespace tint::ir::transform {
namespace {

using namespace tint::builtin::fluent_types;  // NOLINT
using namespace tint::number_suffixes;        // NOLINT

using IR_Std140Test = TransformTest;

TEST_F(IR_Std140Test, NoRootBlock) {
    auto* func = b.Function("foo", ty.void_());
    func->Block()->Append(b.Return(func));

    auto* expect = R"(
%foo = func():void -> %b1 {
  %b1 = block {
    ret
  }
}
)";

    Run<Std140>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_Std140Test, NoModify_Mat2x3) {
    auto* mat = ty.mat2x3<f32>();
    auto* structure = ty.Struct(mod.symbols.New("MyStruct"), {
                                                                 {mod.symbols.New("a"), mat},
                                                             });
    structure->SetStructFlag(type::kBlock);

    auto* buffer = b.Var("buffer", ty.ptr(uniform, structure));
    buffer->SetBindingPoint(0, 0);
    b.RootBlock()->Append(buffer);

    auto* func = b.Function("foo", mat);
    b.Append(func->Block(), [&] {
        auto* access = b.Access(ty.ptr(uniform, mat), buffer, 0_u);
        auto* load = b.Load(access);
        b.Return(func, load);
    });

    auto* src = R"(
MyStruct = struct @align(16), @block {
  a:mat2x3<f32> @offset(0)
}

%b1 = block {  # root
  %buffer:ptr<uniform, MyStruct, read_write> = var @binding_point(0, 0)
}

%foo = func():mat2x3<f32> -> %b2 {
  %b2 = block {
    %3:ptr<uniform, mat2x3<f32>, read_write> = access %buffer, 0u
    %4:mat2x3<f32> = load %3
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    Run<Std140>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_Std140Test, NoModify_Mat2x4) {
    auto* mat = ty.mat2x4<f32>();
    auto* structure = ty.Struct(mod.symbols.New("MyStruct"), {
                                                                 {mod.symbols.New("a"), mat},
                                                             });
    structure->SetStructFlag(type::kBlock);

    auto* buffer = b.Var("buffer", ty.ptr(uniform, structure));
    buffer->SetBindingPoint(0, 0);
    b.RootBlock()->Append(buffer);

    auto* func = b.Function("foo", mat);
    b.Append(func->Block(), [&] {
        auto* access = b.Access(ty.ptr(uniform, mat), buffer, 0_u);
        auto* load = b.Load(access);
        b.Return(func, load);
    });

    auto* src = R"(
MyStruct = struct @align(16), @block {
  a:mat2x4<f32> @offset(0)
}

%b1 = block {  # root
  %buffer:ptr<uniform, MyStruct, read_write> = var @binding_point(0, 0)
}

%foo = func():mat2x4<f32> -> %b2 {
  %b2 = block {
    %3:ptr<uniform, mat2x4<f32>, read_write> = access %buffer, 0u
    %4:mat2x4<f32> = load %3
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    Run<Std140>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_Std140Test, NoModify_Mat3x2_StorageBuffer) {
    auto* mat = ty.mat2x4<f32>();
    auto* structure = ty.Struct(mod.symbols.New("MyStruct"), {
                                                                 {mod.symbols.New("a"), mat},
                                                             });
    structure->SetStructFlag(type::kBlock);

    auto* buffer = b.Var("buffer", ty.ptr(storage, structure));
    buffer->SetBindingPoint(0, 0);
    b.RootBlock()->Append(buffer);

    auto* func = b.Function("foo", mat);
    b.Append(func->Block(), [&] {
        auto* access = b.Access(ty.ptr(storage, mat), buffer, 0_u);
        auto* load = b.Load(access);
        b.Return(func, load);
    });

    auto* src = R"(
MyStruct = struct @align(16), @block {
  a:mat2x4<f32> @offset(0)
}

%b1 = block {  # root
  %buffer:ptr<storage, MyStruct, read_write> = var @binding_point(0, 0)
}

%foo = func():mat2x4<f32> -> %b2 {
  %b2 = block {
    %3:ptr<storage, mat2x4<f32>, read_write> = access %buffer, 0u
    %4:mat2x4<f32> = load %3
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    Run<Std140>();

    EXPECT_EQ(expect, str());
}

// Test that we do not decompose a mat2x2 that is used an array element type.
TEST_F(IR_Std140Test, NoModify_Mat2x2_InsideArray) {
    auto* mat = ty.mat2x2<f32>();
    auto* structure =
        ty.Struct(mod.symbols.New("MyStruct"), {
                                                   {mod.symbols.New("arr"), ty.array(mat, 4u)},
                                               });
    structure->SetStructFlag(type::kBlock);

    auto* buffer = b.Var("buffer", ty.ptr(uniform, structure));
    buffer->SetBindingPoint(0, 0);
    b.RootBlock()->Append(buffer);

    auto* func = b.Function("foo", mat);
    b.Append(func->Block(), [&] {
        auto* load = b.Load(b.Access(ty.ptr(uniform, mat), buffer, 0_u, 2_u));
        b.Return(func, load);
    });

    auto* src = R"(
MyStruct = struct @align(8), @block {
  arr:array<mat2x2<f32>, 4> @offset(0)
}

%b1 = block {  # root
  %buffer:ptr<uniform, MyStruct, read_write> = var @binding_point(0, 0)
}

%foo = func():mat2x2<f32> -> %b2 {
  %b2 = block {
    %3:ptr<uniform, mat2x2<f32>, read_write> = access %buffer, 0u, 2u
    %4:mat2x2<f32> = load %3
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    Run<Std140>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_Std140Test, Mat3x2_LoadMatrix) {
    auto* mat = ty.mat3x2<f32>();
    auto* structure = ty.Struct(mod.symbols.New("MyStruct"), {
                                                                 {mod.symbols.New("a"), mat},
                                                             });
    structure->SetStructFlag(type::kBlock);

    auto* buffer = b.Var("buffer", ty.ptr(uniform, structure));
    buffer->SetBindingPoint(0, 0);
    b.RootBlock()->Append(buffer);

    auto* func = b.Function("foo", mat);
    b.Append(func->Block(), [&] {
        auto* access = b.Access(ty.ptr(uniform, mat), buffer, 0_u);
        auto* load = b.Load(access);
        b.Return(func, load);
    });

    auto* src = R"(
MyStruct = struct @align(8), @block {
  a:mat3x2<f32> @offset(0)
}

%b1 = block {  # root
  %buffer:ptr<uniform, MyStruct, read_write> = var @binding_point(0, 0)
}

%foo = func():mat3x2<f32> -> %b2 {
  %b2 = block {
    %3:ptr<uniform, mat3x2<f32>, read_write> = access %buffer, 0u
    %4:mat3x2<f32> = load %3
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
MyStruct = struct @align(8), @block {
  a:mat3x2<f32> @offset(0)
}

MyStruct_std140 = struct @align(8), @block {
  a_col0:vec2<f32> @offset(0)
  a_col1:vec2<f32> @offset(8)
  a_col2:vec2<f32> @offset(16)
}

%b1 = block {  # root
  %buffer:ptr<uniform, MyStruct_std140, read_write> = var @binding_point(0, 0)
}

%foo = func():mat3x2<f32> -> %b2 {
  %b2 = block {
    %3:ptr<uniform, vec2<f32>, read_write> = access %buffer, 0u
    %4:vec2<f32> = load %3
    %5:ptr<uniform, vec2<f32>, read_write> = access %buffer, 1u
    %6:vec2<f32> = load %5
    %7:ptr<uniform, vec2<f32>, read_write> = access %buffer, 2u
    %8:vec2<f32> = load %7
    %9:mat3x2<f32> = construct %4, %6, %8
    ret %9
  }
}
)";

    Run<Std140>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_Std140Test, Mat3x2_LoadColumn) {
    auto* mat = ty.mat3x2<f32>();
    auto* structure = ty.Struct(mod.symbols.New("MyStruct"), {
                                                                 {mod.symbols.New("a"), mat},
                                                             });
    structure->SetStructFlag(type::kBlock);

    auto* buffer = b.Var("buffer", ty.ptr(uniform, structure));
    buffer->SetBindingPoint(0, 0);
    b.RootBlock()->Append(buffer);

    auto* func = b.Function("foo", mat->ColumnType());
    b.Append(func->Block(), [&] {
        auto* access = b.Access(ty.ptr(uniform, mat->ColumnType()), buffer, 0_u, 1_u);
        auto* load = b.Load(access);
        b.Return(func, load);
    });

    auto* src = R"(
MyStruct = struct @align(8), @block {
  a:mat3x2<f32> @offset(0)
}

%b1 = block {  # root
  %buffer:ptr<uniform, MyStruct, read_write> = var @binding_point(0, 0)
}

%foo = func():vec2<f32> -> %b2 {
  %b2 = block {
    %3:ptr<uniform, vec2<f32>, read_write> = access %buffer, 0u, 1u
    %4:vec2<f32> = load %3
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
MyStruct = struct @align(8), @block {
  a:mat3x2<f32> @offset(0)
}

MyStruct_std140 = struct @align(8), @block {
  a_col0:vec2<f32> @offset(0)
  a_col1:vec2<f32> @offset(8)
  a_col2:vec2<f32> @offset(16)
}

%b1 = block {  # root
  %buffer:ptr<uniform, MyStruct_std140, read_write> = var @binding_point(0, 0)
}

%foo = func():vec2<f32> -> %b2 {
  %b2 = block {
    %3:ptr<uniform, vec2<f32>, read_write> = access %buffer, 0u
    %4:vec2<f32> = load %3
    %5:ptr<uniform, vec2<f32>, read_write> = access %buffer, 1u
    %6:vec2<f32> = load %5
    %7:ptr<uniform, vec2<f32>, read_write> = access %buffer, 2u
    %8:vec2<f32> = load %7
    %9:mat3x2<f32> = construct %4, %6, %8
    %10:vec2<f32> = access %9, 1u
    ret %10
  }
}
)";

    Run<Std140>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_Std140Test, Mat3x2_LoadElement) {
    auto* mat = ty.mat3x2<f32>();
    auto* structure = ty.Struct(mod.symbols.New("MyStruct"), {
                                                                 {mod.symbols.New("a"), mat},
                                                             });
    structure->SetStructFlag(type::kBlock);

    auto* buffer = b.Var("buffer", ty.ptr(uniform, structure));
    buffer->SetBindingPoint(0, 0);
    b.RootBlock()->Append(buffer);

    auto* func = b.Function("foo", ty.f32());
    b.Append(func->Block(), [&] {
        auto* access = b.Access(ty.ptr(uniform, mat->ColumnType()), buffer, 0_u, 1_u);
        auto* load = b.LoadVectorElement(access, 1_u);
        b.Return(func, load);
    });

    auto* src = R"(
MyStruct = struct @align(8), @block {
  a:mat3x2<f32> @offset(0)
}

%b1 = block {  # root
  %buffer:ptr<uniform, MyStruct, read_write> = var @binding_point(0, 0)
}

%foo = func():f32 -> %b2 {
  %b2 = block {
    %3:ptr<uniform, vec2<f32>, read_write> = access %buffer, 0u, 1u
    %4:f32 = load_vector_element %3, 1u
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
MyStruct = struct @align(8), @block {
  a:mat3x2<f32> @offset(0)
}

MyStruct_std140 = struct @align(8), @block {
  a_col0:vec2<f32> @offset(0)
  a_col1:vec2<f32> @offset(8)
  a_col2:vec2<f32> @offset(16)
}

%b1 = block {  # root
  %buffer:ptr<uniform, MyStruct_std140, read_write> = var @binding_point(0, 0)
}

%foo = func():f32 -> %b2 {
  %b2 = block {
    %3:ptr<uniform, vec2<f32>, read_write> = access %buffer, 0u
    %4:vec2<f32> = load %3
    %5:ptr<uniform, vec2<f32>, read_write> = access %buffer, 1u
    %6:vec2<f32> = load %5
    %7:ptr<uniform, vec2<f32>, read_write> = access %buffer, 2u
    %8:vec2<f32> = load %7
    %9:mat3x2<f32> = construct %4, %6, %8
    %10:vec2<f32> = access %9, 1u
    %11:f32 = access %10, 1u
    ret %11
  }
}
)";

    Run<Std140>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_Std140Test, Mat3x2_LoadStruct) {
    auto* mat = ty.mat3x2<f32>();
    auto* structure = ty.Struct(mod.symbols.New("MyStruct"), {
                                                                 {mod.symbols.New("a"), mat},
                                                             });
    structure->SetStructFlag(type::kBlock);

    auto* buffer = b.Var("buffer", ty.ptr(uniform, structure));
    buffer->SetBindingPoint(0, 0);
    b.RootBlock()->Append(buffer);

    auto* func = b.Function("foo", structure);
    b.Append(func->Block(), [&] {
        auto* load = b.Load(buffer);
        b.Return(func, load);
    });

    auto* src = R"(
MyStruct = struct @align(8), @block {
  a:mat3x2<f32> @offset(0)
}

%b1 = block {  # root
  %buffer:ptr<uniform, MyStruct, read_write> = var @binding_point(0, 0)
}

%foo = func():MyStruct -> %b2 {
  %b2 = block {
    %3:MyStruct = load %buffer
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
MyStruct = struct @align(8), @block {
  a:mat3x2<f32> @offset(0)
}

MyStruct_std140 = struct @align(8), @block {
  a_col0:vec2<f32> @offset(0)
  a_col1:vec2<f32> @offset(8)
  a_col2:vec2<f32> @offset(16)
}

%b1 = block {  # root
  %buffer:ptr<uniform, MyStruct_std140, read_write> = var @binding_point(0, 0)
}

%foo = func():MyStruct -> %b2 {
  %b2 = block {
    %3:MyStruct_std140 = load %buffer
    %4:MyStruct = call %convert_MyStruct, %3
    ret %4
  }
}
%convert_MyStruct = func(%input:MyStruct_std140):MyStruct -> %b3 {
  %b3 = block {
    %7:vec2<f32> = access %input, 0u
    %8:vec2<f32> = access %input, 1u
    %9:vec2<f32> = access %input, 2u
    %10:mat3x2<f32> = construct %7, %8, %9
    %11:MyStruct = construct %10
    ret %11
  }
}
)";

    Run<Std140>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_Std140Test, Mat3x2_LoadArrayOfStruct) {
    auto* mat = ty.mat3x2<f32>();
    auto* inner = ty.Struct(mod.symbols.New("Inner"), {
                                                          {mod.symbols.New("a"), mat},
                                                      });
    auto* outer =
        ty.Struct(mod.symbols.New("Outer"), {
                                                {mod.symbols.New("arr"), ty.array(inner, 4u)},
                                            });
    outer->SetStructFlag(type::kBlock);

    auto* buffer = b.Var("buffer", ty.ptr(uniform, outer));
    buffer->SetBindingPoint(0, 0);
    b.RootBlock()->Append(buffer);

    auto* func = b.Function("foo", outer);
    b.Append(func->Block(), [&] {
        auto* load = b.Load(buffer);
        b.Return(func, load);
    });

    auto* src = R"(
Inner = struct @align(8) {
  a:mat3x2<f32> @offset(0)
}

Outer = struct @align(8), @block {
  arr:array<Inner, 4> @offset(0)
}

%b1 = block {  # root
  %buffer:ptr<uniform, Outer, read_write> = var @binding_point(0, 0)
}

%foo = func():Outer -> %b2 {
  %b2 = block {
    %3:Outer = load %buffer
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Inner = struct @align(8) {
  a:mat3x2<f32> @offset(0)
}

Outer = struct @align(8), @block {
  arr:array<Inner, 4> @offset(0)
}

Inner_std140 = struct @align(8) {
  a_col0:vec2<f32> @offset(0)
  a_col1:vec2<f32> @offset(8)
  a_col2:vec2<f32> @offset(16)
}

Outer_std140 = struct @align(8), @block {
  arr:array<Inner_std140, 4> @offset(0)
}

%b1 = block {  # root
  %buffer:ptr<uniform, Outer_std140, read_write> = var @binding_point(0, 0)
}

%foo = func():Outer -> %b2 {
  %b2 = block {
    %3:Outer_std140 = load %buffer
    %4:Outer = call %convert_Outer, %3
    ret %4
  }
}
%convert_Outer = func(%input:Outer_std140):Outer -> %b3 {
  %b3 = block {
    %7:array<Inner_std140, 4> = access %input, 0u
    %8:ptr<function, array<Inner, 4>, read_write> = var
    loop [i: %b4, b: %b5, c: %b6] {  # loop_1
      %b4 = block {  # initializer
        next_iteration %b5 0u
      }
      %b5 = block (%idx:u32) {  # body
        %10:bool = eq %idx:u32, 4u
        if %10 [t: %b7] {  # if_1
          %b7 = block {  # true
            exit_loop  # loop_1
          }
        }
        %11:ptr<function, Inner, read_write> = access %8, %idx:u32
        %12:Inner_std140 = access %7, %idx:u32
        %13:Inner = call %convert_Inner, %12
        store %11, %13
        continue %b6
      }
      %b6 = block {  # continuing
        %15:u32 = add %idx:u32, 1u
        next_iteration %b5 %15
      }
    }
    %16:array<Inner, 4> = load %8
    %17:Outer = construct %16
    ret %17
  }
}
%convert_Inner = func(%input_1:Inner_std140):Inner -> %b8 {  # %input_1: 'input'
  %b8 = block {
    %19:vec2<f32> = access %input_1, 0u
    %20:vec2<f32> = access %input_1, 1u
    %21:vec2<f32> = access %input_1, 2u
    %22:mat3x2<f32> = construct %19, %20, %21
    %23:Inner = construct %22
    ret %23
  }
}
)";

    Run<Std140>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_Std140Test, Mat3x2_LoadNestedStruct) {
    auto* mat = ty.mat3x2<f32>();
    auto* inner = ty.Struct(mod.symbols.New("Inner"), {
                                                          {mod.symbols.New("a"), mat},
                                                      });
    auto* outer = ty.Struct(mod.symbols.New("Outer"), {
                                                          {mod.symbols.New("inner"), inner},
                                                      });
    outer->SetStructFlag(type::kBlock);

    auto* buffer = b.Var("buffer", ty.ptr(uniform, outer));
    buffer->SetBindingPoint(0, 0);
    b.RootBlock()->Append(buffer);

    auto* func = b.Function("foo", inner);
    b.Append(func->Block(), [&] {
        auto* load = b.Load(b.Access(ty.ptr(uniform, inner), buffer, 0_u));
        b.Return(func, load);
    });

    auto* src = R"(
Inner = struct @align(8) {
  a:mat3x2<f32> @offset(0)
}

Outer = struct @align(8), @block {
  inner:Inner @offset(0)
}

%b1 = block {  # root
  %buffer:ptr<uniform, Outer, read_write> = var @binding_point(0, 0)
}

%foo = func():Inner -> %b2 {
  %b2 = block {
    %3:ptr<uniform, Inner, read_write> = access %buffer, 0u
    %4:Inner = load %3
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Inner = struct @align(8) {
  a:mat3x2<f32> @offset(0)
}

Outer = struct @align(8), @block {
  inner:Inner @offset(0)
}

Inner_std140 = struct @align(8) {
  a_col0:vec2<f32> @offset(0)
  a_col1:vec2<f32> @offset(8)
  a_col2:vec2<f32> @offset(16)
}

Outer_std140 = struct @align(8), @block {
  inner:Inner_std140 @offset(0)
}

%b1 = block {  # root
  %buffer:ptr<uniform, Outer_std140, read_write> = var @binding_point(0, 0)
}

%foo = func():Inner -> %b2 {
  %b2 = block {
    %3:ptr<uniform, Inner_std140, read_write> = access %buffer, 0u
    %4:Inner_std140 = load %3
    %5:Inner = call %convert_Inner, %4
    ret %5
  }
}
%convert_Inner = func(%input:Inner_std140):Inner -> %b3 {
  %b3 = block {
    %8:vec2<f32> = access %input, 0u
    %9:vec2<f32> = access %input, 1u
    %10:vec2<f32> = access %input, 2u
    %11:mat3x2<f32> = construct %8, %9, %10
    %12:Inner = construct %11
    ret %12
  }
}
)";

    Run<Std140>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_Std140Test, Mat3x2_LoadStruct_WithUnmodifedNestedStruct) {
    auto* inner = ty.Struct(mod.symbols.New("Inner"), {
                                                          {mod.symbols.New("a"), ty.mat4x4<f32>()},
                                                      });
    auto* outer = ty.Struct(mod.symbols.New("Outer"), {
                                                          {mod.symbols.New("m"), ty.mat3x2<f32>()},
                                                          {mod.symbols.New("inner"), inner},
                                                      });
    outer->SetStructFlag(type::kBlock);

    auto* buffer = b.Var("buffer", ty.ptr(uniform, outer));
    buffer->SetBindingPoint(0, 0);
    b.RootBlock()->Append(buffer);

    auto* func = b.Function("foo", outer);
    b.Append(func->Block(), [&] {
        auto* load = b.Load(buffer);
        b.Return(func, load);
    });

    auto* src = R"(
Inner = struct @align(16) {
  a:mat4x4<f32> @offset(0)
}

Outer = struct @align(16), @block {
  m:mat3x2<f32> @offset(0)
  inner:Inner @offset(32)
}

%b1 = block {  # root
  %buffer:ptr<uniform, Outer, read_write> = var @binding_point(0, 0)
}

%foo = func():Outer -> %b2 {
  %b2 = block {
    %3:Outer = load %buffer
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Inner = struct @align(16) {
  a:mat4x4<f32> @offset(0)
}

Outer = struct @align(16), @block {
  m:mat3x2<f32> @offset(0)
  inner:Inner @offset(32)
}

Outer_std140 = struct @align(16), @block {
  m_col0:vec2<f32> @offset(0)
  m_col1:vec2<f32> @offset(8)
  m_col2:vec2<f32> @offset(16)
  inner:Inner @offset(32)
}

%b1 = block {  # root
  %buffer:ptr<uniform, Outer_std140, read_write> = var @binding_point(0, 0)
}

%foo = func():Outer -> %b2 {
  %b2 = block {
    %3:Outer_std140 = load %buffer
    %4:Outer = call %convert_Outer, %3
    ret %4
  }
}
%convert_Outer = func(%input:Outer_std140):Outer -> %b3 {
  %b3 = block {
    %7:vec2<f32> = access %input, 0u
    %8:vec2<f32> = access %input, 1u
    %9:vec2<f32> = access %input, 2u
    %10:mat3x2<f32> = construct %7, %8, %9
    %11:Inner = access %input, 3u
    %12:Outer = construct %10, %11
    ret %12
  }
}
)";

    Run<Std140>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_Std140Test, Mat3x2_Nested_ChainOfAccessInstructions) {
    auto* mat = ty.mat3x2<f32>();
    auto* inner = ty.Struct(mod.symbols.New("Inner"), {
                                                          {mod.symbols.New("a"), ty.i32()},
                                                          {mod.symbols.New("m"), mat},
                                                          {mod.symbols.New("b"), ty.i32()},
                                                      });
    auto* arr = ty.array(inner, 4u);
    auto* outer = ty.Struct(mod.symbols.New("Outer"), {
                                                          {mod.symbols.New("c"), ty.i32()},
                                                          {mod.symbols.New("arr"), arr},
                                                          {mod.symbols.New("d"), ty.i32()},
                                                      });
    outer->SetStructFlag(type::kBlock);

    auto* buffer = b.Var("buffer", ty.ptr(uniform, outer));
    buffer->SetBindingPoint(0, 0);
    b.RootBlock()->Append(buffer);

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* arr_ptr = b.Access(ty.ptr(uniform, arr), buffer, 1_u);
        auto* inner_ptr = b.Access(ty.ptr(uniform, inner), arr_ptr, 2_u);
        auto* mat_ptr = b.Access(ty.ptr(uniform, mat), inner_ptr, 1_u);
        auto* col_ptr = b.Access(ty.ptr(uniform, mat->ColumnType()), mat_ptr, 2_u);
        b.Let("arr", b.Load(arr_ptr));
        b.Let("inner", b.Load(inner_ptr));
        b.Let("mat", b.Load(mat_ptr));
        b.Let("col", b.Load(col_ptr));
        b.Let("el", b.LoadVectorElement(col_ptr, 1_u));
        b.Return(func);
    });

    auto* src = R"(
Inner = struct @align(8) {
  a:i32 @offset(0)
  m:mat3x2<f32> @offset(8)
  b:i32 @offset(32)
}

Outer = struct @align(8), @block {
  c:i32 @offset(0)
  arr:array<Inner, 4> @offset(8)
  d:i32 @offset(168)
}

%b1 = block {  # root
  %buffer:ptr<uniform, Outer, read_write> = var @binding_point(0, 0)
}

%foo = func():void -> %b2 {
  %b2 = block {
    %3:ptr<uniform, array<Inner, 4>, read_write> = access %buffer, 1u
    %4:ptr<uniform, Inner, read_write> = access %3, 2u
    %5:ptr<uniform, mat3x2<f32>, read_write> = access %4, 1u
    %6:ptr<uniform, vec2<f32>, read_write> = access %5, 2u
    %7:array<Inner, 4> = load %3
    %arr:array<Inner, 4> = let %7
    %9:Inner = load %4
    %inner:Inner = let %9
    %11:mat3x2<f32> = load %5
    %mat:mat3x2<f32> = let %11
    %13:vec2<f32> = load %6
    %col:vec2<f32> = let %13
    %15:f32 = load_vector_element %6, 1u
    %el:f32 = let %15
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Inner = struct @align(8) {
  a:i32 @offset(0)
  m:mat3x2<f32> @offset(8)
  b:i32 @offset(32)
}

Outer = struct @align(8), @block {
  c:i32 @offset(0)
  arr:array<Inner, 4> @offset(8)
  d:i32 @offset(168)
}

Inner_std140 = struct @align(8) {
  a:i32 @offset(0)
  m_col0:vec2<f32> @offset(8)
  m_col1:vec2<f32> @offset(16)
  m_col2:vec2<f32> @offset(24)
  b:i32 @offset(32)
}

Outer_std140 = struct @align(8), @block {
  c:i32 @offset(0)
  arr:array<Inner_std140, 4> @offset(8)
  d:i32 @offset(168)
}

%b1 = block {  # root
  %buffer:ptr<uniform, Outer_std140, read_write> = var @binding_point(0, 0)
}

%foo = func():void -> %b2 {
  %b2 = block {
    %3:ptr<uniform, array<Inner_std140, 4>, read_write> = access %buffer, 1u
    %4:ptr<uniform, Inner_std140, read_write> = access %3, 2u
    %5:ptr<uniform, vec2<f32>, read_write> = access %4, 1u
    %6:vec2<f32> = load %5
    %7:ptr<uniform, vec2<f32>, read_write> = access %4, 2u
    %8:vec2<f32> = load %7
    %9:ptr<uniform, vec2<f32>, read_write> = access %4, 3u
    %10:vec2<f32> = load %9
    %11:mat3x2<f32> = construct %6, %8, %10
    %12:vec2<f32> = access %11, 2u
    %13:array<Inner_std140, 4> = load %3
    %14:ptr<function, array<Inner, 4>, read_write> = var
    loop [i: %b3, b: %b4, c: %b5] {  # loop_1
      %b3 = block {  # initializer
        next_iteration %b4 0u
      }
      %b4 = block (%idx:u32) {  # body
        %16:bool = eq %idx:u32, 4u
        if %16 [t: %b6] {  # if_1
          %b6 = block {  # true
            exit_loop  # loop_1
          }
        }
        %17:ptr<function, Inner, read_write> = access %14, %idx:u32
        %18:Inner_std140 = access %13, %idx:u32
        %19:Inner = call %convert_Inner, %18
        store %17, %19
        continue %b5
      }
      %b5 = block {  # continuing
        %21:u32 = add %idx:u32, 1u
        next_iteration %b4 %21
      }
    }
    %22:array<Inner, 4> = load %14
    %arr:array<Inner, 4> = let %22
    %24:Inner_std140 = load %4
    %25:Inner = call %convert_Inner, %24
    %inner:Inner = let %25
    %mat:mat3x2<f32> = let %11
    %col:vec2<f32> = let %12
    %29:f32 = access %12, 1u
    %el:f32 = let %29
    ret
  }
}
%convert_Inner = func(%input:Inner_std140):Inner -> %b7 {
  %b7 = block {
    %32:i32 = access %input, 0u
    %33:vec2<f32> = access %input, 1u
    %34:vec2<f32> = access %input, 2u
    %35:vec2<f32> = access %input, 3u
    %36:mat3x2<f32> = construct %33, %34, %35
    %37:i32 = access %input, 4u
    %38:Inner = construct %32, %36, %37
    ret %38
  }
}
)";

    Run<Std140>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_Std140Test, Mat3x2_Nested_ChainOfAccessInstructions_ViaLets) {
    auto* mat = ty.mat3x2<f32>();
    auto* inner = ty.Struct(mod.symbols.New("Inner"), {
                                                          {mod.symbols.New("a"), ty.i32()},
                                                          {mod.symbols.New("m"), mat},
                                                          {mod.symbols.New("b"), ty.i32()},
                                                      });
    auto* arr = ty.array(inner, 4u);
    auto* outer = ty.Struct(mod.symbols.New("Outer"), {
                                                          {mod.symbols.New("c"), ty.i32()},
                                                          {mod.symbols.New("arr"), arr},
                                                          {mod.symbols.New("d"), ty.i32()},
                                                      });
    outer->SetStructFlag(type::kBlock);

    auto* buffer = b.Var("buffer", ty.ptr(uniform, outer));
    buffer->SetBindingPoint(0, 0);
    b.RootBlock()->Append(buffer);

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* arr_ptr = b.Let("arr_ptr", b.Access(ty.ptr(uniform, arr), buffer, 1_u));
        auto* inner_ptr = b.Let("inner_ptr", b.Access(ty.ptr(uniform, inner), arr_ptr, 2_u));
        auto* mat_ptr = b.Let("mat_ptr", b.Access(ty.ptr(uniform, mat), inner_ptr, 1_u));
        auto* col_ptr =
            b.Let("col_ptr", b.Access(ty.ptr(uniform, mat->ColumnType()), mat_ptr, 2_u));
        b.Let("arr", b.Load(arr_ptr));
        b.Let("inner", b.Load(inner_ptr));
        b.Let("mat", b.Load(mat_ptr));
        b.Let("col", b.Load(col_ptr));
        b.Let("el", b.LoadVectorElement(col_ptr, 1_u));
        b.Return(func);
    });

    auto* src = R"(
Inner = struct @align(8) {
  a:i32 @offset(0)
  m:mat3x2<f32> @offset(8)
  b:i32 @offset(32)
}

Outer = struct @align(8), @block {
  c:i32 @offset(0)
  arr:array<Inner, 4> @offset(8)
  d:i32 @offset(168)
}

%b1 = block {  # root
  %buffer:ptr<uniform, Outer, read_write> = var @binding_point(0, 0)
}

%foo = func():void -> %b2 {
  %b2 = block {
    %3:ptr<uniform, array<Inner, 4>, read_write> = access %buffer, 1u
    %arr_ptr:ptr<uniform, array<Inner, 4>, read_write> = let %3
    %5:ptr<uniform, Inner, read_write> = access %arr_ptr, 2u
    %inner_ptr:ptr<uniform, Inner, read_write> = let %5
    %7:ptr<uniform, mat3x2<f32>, read_write> = access %inner_ptr, 1u
    %mat_ptr:ptr<uniform, mat3x2<f32>, read_write> = let %7
    %9:ptr<uniform, vec2<f32>, read_write> = access %mat_ptr, 2u
    %col_ptr:ptr<uniform, vec2<f32>, read_write> = let %9
    %11:array<Inner, 4> = load %arr_ptr
    %arr:array<Inner, 4> = let %11
    %13:Inner = load %inner_ptr
    %inner:Inner = let %13
    %15:mat3x2<f32> = load %mat_ptr
    %mat:mat3x2<f32> = let %15
    %17:vec2<f32> = load %col_ptr
    %col:vec2<f32> = let %17
    %19:f32 = load_vector_element %col_ptr, 1u
    %el:f32 = let %19
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Inner = struct @align(8) {
  a:i32 @offset(0)
  m:mat3x2<f32> @offset(8)
  b:i32 @offset(32)
}

Outer = struct @align(8), @block {
  c:i32 @offset(0)
  arr:array<Inner, 4> @offset(8)
  d:i32 @offset(168)
}

Inner_std140 = struct @align(8) {
  a:i32 @offset(0)
  m_col0:vec2<f32> @offset(8)
  m_col1:vec2<f32> @offset(16)
  m_col2:vec2<f32> @offset(24)
  b:i32 @offset(32)
}

Outer_std140 = struct @align(8), @block {
  c:i32 @offset(0)
  arr:array<Inner_std140, 4> @offset(8)
  d:i32 @offset(168)
}

%b1 = block {  # root
  %buffer:ptr<uniform, Outer_std140, read_write> = var @binding_point(0, 0)
}

%foo = func():void -> %b2 {
  %b2 = block {
    %3:ptr<uniform, array<Inner_std140, 4>, read_write> = access %buffer, 1u
    %4:ptr<uniform, Inner_std140, read_write> = access %3, 2u
    %5:ptr<uniform, vec2<f32>, read_write> = access %4, 1u
    %6:vec2<f32> = load %5
    %7:ptr<uniform, vec2<f32>, read_write> = access %4, 2u
    %8:vec2<f32> = load %7
    %9:ptr<uniform, vec2<f32>, read_write> = access %4, 3u
    %10:vec2<f32> = load %9
    %11:mat3x2<f32> = construct %6, %8, %10
    %12:vec2<f32> = access %11, 2u
    %13:array<Inner_std140, 4> = load %3
    %14:ptr<function, array<Inner, 4>, read_write> = var
    loop [i: %b3, b: %b4, c: %b5] {  # loop_1
      %b3 = block {  # initializer
        next_iteration %b4 0u
      }
      %b4 = block (%idx:u32) {  # body
        %16:bool = eq %idx:u32, 4u
        if %16 [t: %b6] {  # if_1
          %b6 = block {  # true
            exit_loop  # loop_1
          }
        }
        %17:ptr<function, Inner, read_write> = access %14, %idx:u32
        %18:Inner_std140 = access %13, %idx:u32
        %19:Inner = call %convert_Inner, %18
        store %17, %19
        continue %b5
      }
      %b5 = block {  # continuing
        %21:u32 = add %idx:u32, 1u
        next_iteration %b4 %21
      }
    }
    %22:array<Inner, 4> = load %14
    %arr:array<Inner, 4> = let %22
    %24:Inner_std140 = load %4
    %25:Inner = call %convert_Inner, %24
    %inner:Inner = let %25
    %mat:mat3x2<f32> = let %11
    %col:vec2<f32> = let %12
    %29:f32 = access %12, 1u
    %el:f32 = let %29
    ret
  }
}
%convert_Inner = func(%input:Inner_std140):Inner -> %b7 {
  %b7 = block {
    %32:i32 = access %input, 0u
    %33:vec2<f32> = access %input, 1u
    %34:vec2<f32> = access %input, 2u
    %35:vec2<f32> = access %input, 3u
    %36:mat3x2<f32> = construct %33, %34, %35
    %37:i32 = access %input, 4u
    %38:Inner = construct %32, %36, %37
    ret %38
  }
}
)";

    Run<Std140>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_Std140Test, Mat3x2_Nested_ChainOfAccessInstructions_DynamicIndices) {
    auto* mat = ty.mat3x2<f32>();
    auto* inner = ty.Struct(mod.symbols.New("Inner"), {
                                                          {mod.symbols.New("a"), ty.i32()},
                                                          {mod.symbols.New("m"), mat},
                                                          {mod.symbols.New("b"), ty.i32()},
                                                      });
    auto* arr = ty.array(inner, 4u);
    auto* outer = ty.Struct(mod.symbols.New("Outer"), {
                                                          {mod.symbols.New("c"), ty.i32()},
                                                          {mod.symbols.New("arr"), arr},
                                                          {mod.symbols.New("d"), ty.i32()},
                                                      });
    outer->SetStructFlag(type::kBlock);

    auto* buffer = b.Var("buffer", ty.ptr(uniform, outer));
    buffer->SetBindingPoint(0, 0);
    b.RootBlock()->Append(buffer);

    auto* func = b.Function("foo", ty.void_());
    auto* arr_idx = b.FunctionParam("arr_idx", ty.i32());
    auto* col_idx = b.FunctionParam("col_idx", ty.i32());
    auto* el_idx = b.FunctionParam("el_idx", ty.i32());
    func->SetParams({arr_idx, col_idx, el_idx});
    b.Append(func->Block(), [&] {
        auto* arr_ptr = b.Access(ty.ptr(uniform, arr), buffer, 1_u);
        auto* inner_ptr = b.Access(ty.ptr(uniform, inner), arr_ptr, arr_idx);
        auto* mat_ptr = b.Access(ty.ptr(uniform, mat), inner_ptr, 1_u);
        auto* col_ptr = b.Access(ty.ptr(uniform, mat->ColumnType()), mat_ptr, col_idx);
        b.Let("arr", b.Load(arr_ptr));
        b.Let("inner", b.Load(inner_ptr));
        b.Let("mat", b.Load(mat_ptr));
        b.Let("col", b.Load(col_ptr));
        b.Let("el", b.LoadVectorElement(col_ptr, el_idx));
        b.Return(func);
    });

    auto* src = R"(
Inner = struct @align(8) {
  a:i32 @offset(0)
  m:mat3x2<f32> @offset(8)
  b:i32 @offset(32)
}

Outer = struct @align(8), @block {
  c:i32 @offset(0)
  arr:array<Inner, 4> @offset(8)
  d:i32 @offset(168)
}

%b1 = block {  # root
  %buffer:ptr<uniform, Outer, read_write> = var @binding_point(0, 0)
}

%foo = func(%arr_idx:i32, %col_idx:i32, %el_idx:i32):void -> %b2 {
  %b2 = block {
    %6:ptr<uniform, array<Inner, 4>, read_write> = access %buffer, 1u
    %7:ptr<uniform, Inner, read_write> = access %6, %arr_idx
    %8:ptr<uniform, mat3x2<f32>, read_write> = access %7, 1u
    %9:ptr<uniform, vec2<f32>, read_write> = access %8, %col_idx
    %10:array<Inner, 4> = load %6
    %arr:array<Inner, 4> = let %10
    %12:Inner = load %7
    %inner:Inner = let %12
    %14:mat3x2<f32> = load %8
    %mat:mat3x2<f32> = let %14
    %16:vec2<f32> = load %9
    %col:vec2<f32> = let %16
    %18:f32 = load_vector_element %9, %el_idx
    %el:f32 = let %18
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Inner = struct @align(8) {
  a:i32 @offset(0)
  m:mat3x2<f32> @offset(8)
  b:i32 @offset(32)
}

Outer = struct @align(8), @block {
  c:i32 @offset(0)
  arr:array<Inner, 4> @offset(8)
  d:i32 @offset(168)
}

Inner_std140 = struct @align(8) {
  a:i32 @offset(0)
  m_col0:vec2<f32> @offset(8)
  m_col1:vec2<f32> @offset(16)
  m_col2:vec2<f32> @offset(24)
  b:i32 @offset(32)
}

Outer_std140 = struct @align(8), @block {
  c:i32 @offset(0)
  arr:array<Inner_std140, 4> @offset(8)
  d:i32 @offset(168)
}

%b1 = block {  # root
  %buffer:ptr<uniform, Outer_std140, read_write> = var @binding_point(0, 0)
}

%foo = func(%arr_idx:i32, %col_idx:i32, %el_idx:i32):void -> %b2 {
  %b2 = block {
    %6:ptr<uniform, array<Inner_std140, 4>, read_write> = access %buffer, 1u
    %7:ptr<uniform, Inner_std140, read_write> = access %6, %arr_idx
    %8:ptr<uniform, vec2<f32>, read_write> = access %7, 1u
    %9:vec2<f32> = load %8
    %10:ptr<uniform, vec2<f32>, read_write> = access %7, 2u
    %11:vec2<f32> = load %10
    %12:ptr<uniform, vec2<f32>, read_write> = access %7, 3u
    %13:vec2<f32> = load %12
    %14:mat3x2<f32> = construct %9, %11, %13
    %15:vec2<f32> = access %14, %col_idx
    %16:array<Inner_std140, 4> = load %6
    %17:ptr<function, array<Inner, 4>, read_write> = var
    loop [i: %b3, b: %b4, c: %b5] {  # loop_1
      %b3 = block {  # initializer
        next_iteration %b4 0u
      }
      %b4 = block (%idx:u32) {  # body
        %19:bool = eq %idx:u32, 4u
        if %19 [t: %b6] {  # if_1
          %b6 = block {  # true
            exit_loop  # loop_1
          }
        }
        %20:ptr<function, Inner, read_write> = access %17, %idx:u32
        %21:Inner_std140 = access %16, %idx:u32
        %22:Inner = call %convert_Inner, %21
        store %20, %22
        continue %b5
      }
      %b5 = block {  # continuing
        %24:u32 = add %idx:u32, 1u
        next_iteration %b4 %24
      }
    }
    %25:array<Inner, 4> = load %17
    %arr:array<Inner, 4> = let %25
    %27:Inner_std140 = load %7
    %28:Inner = call %convert_Inner, %27
    %inner:Inner = let %28
    %mat:mat3x2<f32> = let %14
    %col:vec2<f32> = let %15
    %32:f32 = access %15, %el_idx
    %el:f32 = let %32
    ret
  }
}
%convert_Inner = func(%input:Inner_std140):Inner -> %b7 {
  %b7 = block {
    %35:i32 = access %input, 0u
    %36:vec2<f32> = access %input, 1u
    %37:vec2<f32> = access %input, 2u
    %38:vec2<f32> = access %input, 3u
    %39:mat3x2<f32> = construct %36, %37, %38
    %40:i32 = access %input, 4u
    %41:Inner = construct %35, %39, %40
    ret %41
  }
}
)";

    Run<Std140>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_Std140Test, NonDefaultAlignAndSize) {
    auto* mat = ty.mat4x2<f32>();
    auto* structure = ty.Get<type::Struct>(
        mod.symbols.New("MyStruct"),
        Vector{
            ty.Get<type::StructMember>(mod.symbols.New("a"), ty.i32(), 0u, 0u, 0u, 16u,
                                       type::StructMemberAttributes{}),
            ty.Get<type::StructMember>(mod.symbols.New("m"), mat, 1u, 64u, 32u, 64u,
                                       type::StructMemberAttributes{}),
            ty.Get<type::StructMember>(mod.symbols.New("b"), ty.i32(), 2u, 128u, 8u, 32u,
                                       type::StructMemberAttributes{}),
        },
        128u, 256u, 160u);
    structure->SetStructFlag(type::kBlock);

    auto* buffer = b.Var("buffer", ty.ptr(uniform, structure));
    buffer->SetBindingPoint(0, 0);
    b.RootBlock()->Append(buffer);

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* a_access = b.Access(ty.ptr(uniform, ty.i32()), buffer, 0_u);
        b.Let("a", b.Load(a_access));
        auto* m_access = b.Access(ty.ptr(uniform, mat), buffer, 1_u);
        b.Let("m", b.Load(m_access));
        auto* b_access = b.Access(ty.ptr(uniform, ty.i32()), buffer, 2_u);
        b.Let("b", b.Load(b_access));
        b.Return(func);
    });

    auto* src = R"(
MyStruct = struct @align(128), @block {
  a:i32 @offset(0)
  m:mat4x2<f32> @offset(64)
  b:i32 @offset(128)
}

%b1 = block {  # root
  %buffer:ptr<uniform, MyStruct, read_write> = var @binding_point(0, 0)
}

%foo = func():void -> %b2 {
  %b2 = block {
    %3:ptr<uniform, i32, read_write> = access %buffer, 0u
    %4:i32 = load %3
    %a:i32 = let %4
    %6:ptr<uniform, mat4x2<f32>, read_write> = access %buffer, 1u
    %7:mat4x2<f32> = load %6
    %m:mat4x2<f32> = let %7
    %9:ptr<uniform, i32, read_write> = access %buffer, 2u
    %10:i32 = load %9
    %b:i32 = let %10
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
MyStruct = struct @align(128), @block {
  a:i32 @offset(0)
  m:mat4x2<f32> @offset(64)
  b:i32 @offset(128)
}

MyStruct_std140 = struct @align(128), @block {
  a:i32 @offset(0)
  m_col0:vec2<f32> @offset(64)
  m_col1:vec2<f32> @offset(72)
  m_col2:vec2<f32> @offset(80)
  m_col3:vec2<f32> @offset(88)
  b:i32 @offset(128)
}

%b1 = block {  # root
  %buffer:ptr<uniform, MyStruct_std140, read_write> = var @binding_point(0, 0)
}

%foo = func():void -> %b2 {
  %b2 = block {
    %3:ptr<uniform, i32, read_write> = access %buffer, 0u
    %4:i32 = load %3
    %a:i32 = let %4
    %6:ptr<uniform, vec2<f32>, read_write> = access %buffer, 1u
    %7:vec2<f32> = load %6
    %8:ptr<uniform, vec2<f32>, read_write> = access %buffer, 2u
    %9:vec2<f32> = load %8
    %10:ptr<uniform, vec2<f32>, read_write> = access %buffer, 3u
    %11:vec2<f32> = load %10
    %12:ptr<uniform, vec2<f32>, read_write> = access %buffer, 4u
    %13:vec2<f32> = load %12
    %14:mat4x2<f32> = construct %7, %9, %11, %13
    %m:mat4x2<f32> = let %14
    %16:ptr<uniform, i32, read_write> = access %buffer, 5u
    %17:i32 = load %16
    %b:i32 = let %17
    ret
  }
}
)";

    Run<Std140>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_Std140Test, F16) {
    auto* structure =
        ty.Struct(mod.symbols.New("MyStruct"), {
                                                   {mod.symbols.New("a"), ty.mat2x2<f16>()},
                                                   {mod.symbols.New("b"), ty.mat2x4<f16>()},
                                                   {mod.symbols.New("c"), ty.mat4x3<f16>()},
                                                   {mod.symbols.New("d"), ty.mat4x4<f16>()},
                                               });
    structure->SetStructFlag(type::kBlock);

    auto* buffer = b.Var("buffer", ty.ptr(uniform, structure));
    buffer->SetBindingPoint(0, 0);
    b.RootBlock()->Append(buffer);

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Let("struct", b.Load(buffer));
        b.Let("mat", b.Load(b.Access(ty.ptr(uniform, ty.mat4x4<f16>()), buffer, 3_u)));
        b.Let("col", b.Load(b.Access(ty.ptr(uniform, ty.vec3<f16>()), buffer, 2_u, 1_u)));
        b.Let("el", b.LoadVectorElement(b.Access(ty.ptr(uniform, ty.vec4<f16>()), buffer, 1_u, 0_u),
                                        3_u));
        b.Return(func);
    });

    auto* src = R"(
MyStruct = struct @align(8), @block {
  a:mat2x2<f16> @offset(0)
  b:mat2x4<f16> @offset(8)
  c:mat4x3<f16> @offset(24)
  d:mat4x4<f16> @offset(56)
}

%b1 = block {  # root
  %buffer:ptr<uniform, MyStruct, read_write> = var @binding_point(0, 0)
}

%foo = func():void -> %b2 {
  %b2 = block {
    %3:MyStruct = load %buffer
    %struct:MyStruct = let %3
    %5:ptr<uniform, mat4x4<f16>, read_write> = access %buffer, 3u
    %6:mat4x4<f16> = load %5
    %mat:mat4x4<f16> = let %6
    %8:ptr<uniform, vec3<f16>, read_write> = access %buffer, 2u, 1u
    %9:vec3<f16> = load %8
    %col:vec3<f16> = let %9
    %11:ptr<uniform, vec4<f16>, read_write> = access %buffer, 1u, 0u
    %12:f16 = load_vector_element %11, 3u
    %el:f16 = let %12
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
MyStruct = struct @align(8), @block {
  a:mat2x2<f16> @offset(0)
  b:mat2x4<f16> @offset(8)
  c:mat4x3<f16> @offset(24)
  d:mat4x4<f16> @offset(56)
}

MyStruct_std140 = struct @align(8), @block {
  a_col0:vec2<f16> @offset(0)
  a_col1:vec2<f16> @offset(4)
  b_col0:vec4<f16> @offset(8)
  b_col1:vec4<f16> @offset(16)
  c_col0:vec3<f16> @offset(24)
  c_col1:vec3<f16> @offset(32)
  c_col2:vec3<f16> @offset(40)
  c_col3:vec3<f16> @offset(48)
  d_col0:vec4<f16> @offset(56)
  d_col1:vec4<f16> @offset(64)
  d_col2:vec4<f16> @offset(72)
  d_col3:vec4<f16> @offset(80)
}

%b1 = block {  # root
  %buffer:ptr<uniform, MyStruct_std140, read_write> = var @binding_point(0, 0)
}

%foo = func():void -> %b2 {
  %b2 = block {
    %3:MyStruct_std140 = load %buffer
    %4:MyStruct = call %convert_MyStruct, %3
    %struct:MyStruct = let %4
    %7:ptr<uniform, vec4<f16>, read_write> = access %buffer, 8u
    %8:vec4<f16> = load %7
    %9:ptr<uniform, vec4<f16>, read_write> = access %buffer, 9u
    %10:vec4<f16> = load %9
    %11:ptr<uniform, vec4<f16>, read_write> = access %buffer, 10u
    %12:vec4<f16> = load %11
    %13:ptr<uniform, vec4<f16>, read_write> = access %buffer, 11u
    %14:vec4<f16> = load %13
    %15:mat4x4<f16> = construct %8, %10, %12, %14
    %mat:mat4x4<f16> = let %15
    %17:ptr<uniform, vec3<f16>, read_write> = access %buffer, 4u
    %18:vec3<f16> = load %17
    %19:ptr<uniform, vec3<f16>, read_write> = access %buffer, 5u
    %20:vec3<f16> = load %19
    %21:ptr<uniform, vec3<f16>, read_write> = access %buffer, 6u
    %22:vec3<f16> = load %21
    %23:ptr<uniform, vec3<f16>, read_write> = access %buffer, 7u
    %24:vec3<f16> = load %23
    %25:mat4x3<f16> = construct %18, %20, %22, %24
    %26:vec3<f16> = access %25, 1u
    %col:vec3<f16> = let %26
    %28:ptr<uniform, vec4<f16>, read_write> = access %buffer, 2u
    %29:vec4<f16> = load %28
    %30:ptr<uniform, vec4<f16>, read_write> = access %buffer, 3u
    %31:vec4<f16> = load %30
    %32:mat2x4<f16> = construct %29, %31
    %33:vec4<f16> = access %32, 0u
    %34:f16 = access %33, 3u
    %el:f16 = let %34
    ret
  }
}
%convert_MyStruct = func(%input:MyStruct_std140):MyStruct -> %b3 {
  %b3 = block {
    %37:vec2<f16> = access %input, 0u
    %38:vec2<f16> = access %input, 1u
    %39:mat2x2<f16> = construct %37, %38
    %40:vec4<f16> = access %input, 2u
    %41:vec4<f16> = access %input, 3u
    %42:mat2x4<f16> = construct %40, %41
    %43:vec3<f16> = access %input, 4u
    %44:vec3<f16> = access %input, 5u
    %45:vec3<f16> = access %input, 6u
    %46:vec3<f16> = access %input, 7u
    %47:mat4x3<f16> = construct %43, %44, %45, %46
    %48:vec4<f16> = access %input, 8u
    %49:vec4<f16> = access %input, 9u
    %50:vec4<f16> = access %input, 10u
    %51:vec4<f16> = access %input, 11u
    %52:mat4x4<f16> = construct %48, %49, %50, %51
    %53:MyStruct = construct %39, %42, %47, %52
    ret %53
  }
}
)";

    Run<Std140>();

    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::ir::transform
