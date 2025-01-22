// Copyright 2025 The Dawn & Tint Authors
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

#include "src/tint/lang/core/ir/validator_test.h"

#include <string>
#include <tuple>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "src/tint/lang/core/address_space.h"
#include "src/tint/lang/core/ir/ir_helper_test.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/core/number.h"
#include "src/tint/lang/core/texel_format.h"
#include "src/tint/lang/core/type/abstract_float.h"
#include "src/tint/lang/core/type/abstract_int.h"
#include "src/tint/lang/core/type/manager.h"
#include "src/tint/lang/core/type/matrix.h"
#include "src/tint/lang/core/type/memory_view.h"
#include "src/tint/lang/core/type/reference.h"
#include "src/tint/lang/core/type/storage_texture.h"

namespace tint::core::ir {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

TEST_F(IR_ValidatorTest, Abstract_Scalar) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {
        b.Var("af", function, ty.AFloat());
        b.Var("af", function, ty.AInt());
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:3:5 error: var: abstracts are not permitted
    %af:ptr<function, abstract-float, read_write> = var
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

:2:3 note: in block
  $B1: {
  ^^^

:4:5 error: var: abstracts are not permitted
    %af_1:ptr<function, abstract-int, read_write> = var  # %af_1: 'af'
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %af:ptr<function, abstract-float, read_write> = var
    %af_1:ptr<function, abstract-int, read_write> = var  # %af_1: 'af'
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Abstract_Vector) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {
        b.Var("af", function, ty.vec2<AFloat>());
        b.Var("ai", function, ty.vec3<AInt>());
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:3:5 error: var: abstracts are not permitted
    %af:ptr<function, vec2<abstract-float>, read_write> = var
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

:2:3 note: in block
  $B1: {
  ^^^

:4:5 error: var: abstracts are not permitted
    %ai:ptr<function, vec3<abstract-int>, read_write> = var
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %af:ptr<function, vec2<abstract-float>, read_write> = var
    %ai:ptr<function, vec3<abstract-int>, read_write> = var
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Abstract_Matrix) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {
        b.Var("af", function, ty.mat2x2<AFloat>());
        b.Var("ai", function, ty.mat3x4<AInt>());
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:3:5 error: var: abstracts are not permitted
    %af:ptr<function, mat2x2<abstract-float>, read_write> = var
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

:2:3 note: in block
  $B1: {
  ^^^

:4:5 error: var: abstracts are not permitted
    %ai:ptr<function, mat3x4<abstract-int>, read_write> = var
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %af:ptr<function, mat2x2<abstract-float>, read_write> = var
    %ai:ptr<function, mat3x4<abstract-int>, read_write> = var
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Abstract_Struct) {
    auto* str_ty =
        ty.Struct(mod.symbols.New("MyStruct"), {
                                                   {mod.symbols.New("af"), ty.AFloat(), {}},
                                                   {mod.symbols.New("ai"), ty.AInt(), {}},
                                               });
    auto* v = b.Var(ty.ptr(private_, str_ty));
    mod.root_block->Append(v);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:7:3 error: var: abstracts are not permitted
  %1:ptr<private, MyStruct, read_write> = var
  ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

:6:1 note: in block
$B1: {  # root
^^^

note: # Disassembly
MyStruct = struct @align(1) {
  af:abstract-float @offset(0)
  ai:abstract-int @offset(0)
}

$B1: {  # root
  %1:ptr<private, MyStruct, read_write> = var
}

)");
}

TEST_F(IR_ValidatorTest, Abstract_FunctionParam) {
    auto* f = b.Function("my_func", ty.void_());

    f->SetParams({b.FunctionParam(ty.AFloat()), b.FunctionParam(ty.AInt())});
    f->Block()->Append(b.Return(f));

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:1:17 error: abstracts are not permitted
%my_func = func(%2:abstract-float, %3:abstract-int):void {
                ^^^^^^^^^^^^^^^^^

:1:36 error: abstracts are not permitted
%my_func = func(%2:abstract-float, %3:abstract-int):void {
                                   ^^^^^^^^^^^^^^^

note: # Disassembly
%my_func = func(%2:abstract-float, %3:abstract-int):void {
  $B1: {
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Type_VectorElements) {
    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        b.Var("u32_valid", AddressSpace::kFunction, ty.vec4(ty.u32()));
        b.Var("i32_valid", AddressSpace::kFunction, ty.vec4(ty.i32()));
        b.Var("bool_valid", AddressSpace::kFunction, ty.vec2(ty.bool_()));
        b.Var("f16_valid", AddressSpace::kFunction, ty.vec3(ty.f16()));
        b.Var("f32_valid", AddressSpace::kFunction, ty.vec3(ty.f32()));
        b.Var("void_invalid", AddressSpace::kFunction, ty.vec2(ty.void_()));
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:8:5 error: var: vector elements, 'vec2<void>', must be scalars
    %void_invalid:ptr<function, vec2<void>, read_write> = var
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %u32_valid:ptr<function, vec4<u32>, read_write> = var
    %i32_valid:ptr<function, vec4<i32>, read_write> = var
    %bool_valid:ptr<function, vec2<bool>, read_write> = var
    %f16_valid:ptr<function, vec3<f16>, read_write> = var
    %f32_valid:ptr<function, vec3<f32>, read_write> = var
    %void_invalid:ptr<function, vec2<void>, read_write> = var
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Type_MatrixElements) {
    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        b.Var("u32_invalid", AddressSpace::kFunction, ty.mat2x2(ty.u32()));
        b.Var("i32_invalid", AddressSpace::kFunction, ty.mat3x2(ty.i32()));
        b.Var("bool_invalid", AddressSpace::kFunction, ty.mat4x2(ty.bool_()));
        b.Var("f16_valid", AddressSpace::kFunction, ty.mat2x3(ty.f16()));
        b.Var("f32_valid", AddressSpace::kFunction, ty.mat4x4(ty.f32()));
        b.Var("void_invalid", AddressSpace::kFunction, ty.mat3x3(ty.void_()));
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:3:5 error: var: matrix elements, 'mat2x2<u32>', must be float scalars
    %u32_invalid:ptr<function, mat2x2<u32>, read_write> = var
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

:2:3 note: in block
  $B1: {
  ^^^

:4:5 error: var: matrix elements, 'mat3x2<i32>', must be float scalars
    %i32_invalid:ptr<function, mat3x2<i32>, read_write> = var
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

:2:3 note: in block
  $B1: {
  ^^^

:5:5 error: var: matrix elements, 'mat4x2<bool>', must be float scalars
    %bool_invalid:ptr<function, mat4x2<bool>, read_write> = var
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

:2:3 note: in block
  $B1: {
  ^^^

:8:5 error: var: matrix elements, 'mat3x3<void>', must be float scalars
    %void_invalid:ptr<function, mat3x3<void>, read_write> = var
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %u32_invalid:ptr<function, mat2x2<u32>, read_write> = var
    %i32_invalid:ptr<function, mat3x2<i32>, read_write> = var
    %bool_invalid:ptr<function, mat4x2<bool>, read_write> = var
    %f16_valid:ptr<function, mat2x3<f16>, read_write> = var
    %f32_valid:ptr<function, mat4x4<f32>, read_write> = var
    %void_invalid:ptr<function, mat3x3<void>, read_write> = var
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Type_StorageTextureDimension) {
    auto* valid =
        b.Var("valid", AddressSpace::kStorage,
              ty.storage_texture(core::type::TextureDimension::k2d, core::TexelFormat::kRgba32Float,
                                 core::Access::kReadWrite),
              read_write);
    valid->SetBindingPoint(0, 0);
    mod.root_block->Append(valid);

    auto* cube =
        b.Var("cube_invalid", AddressSpace::kStorage,
              ty.storage_texture(core::type::TextureDimension::kCube,
                                 core::TexelFormat::kRgba32Float, core::Access::kReadWrite),
              read_write);
    cube->SetBindingPoint(1, 1);
    mod.root_block->Append(cube);

    auto* cube_array =
        b.Var("cube_array_invalid", AddressSpace::kStorage,
              ty.storage_texture(core::type::TextureDimension::kCubeArray,
                                 core::TexelFormat::kRgba32Float, core::Access::kReadWrite),
              read_write);
    cube_array->SetBindingPoint(2, 2);
    mod.root_block->Append(cube_array);

    auto* none =
        b.Var("none_invalid", AddressSpace::kStorage,
              ty.storage_texture(core::type::TextureDimension::kNone,
                                 core::TexelFormat::kRgba32Float, core::Access::kReadWrite),
              read_write);
    none->SetBindingPoint(3, 3);
    mod.root_block->Append(none);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:3:3 error: var: dimension 'cube' for storage textures does not in WGSL yet
  %cube_invalid:ptr<storage, texture_storage_cube<rgba32float, read_write>, read_write> = var @binding_point(1, 1)
  ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

:1:1 note: in block
$B1: {  # root
^^^

:4:3 error: var: dimension 'cube_array' for storage textures does not in WGSL yet
  %cube_array_invalid:ptr<storage, texture_storage_cube_array<rgba32float, read_write>, read_write> = var @binding_point(2, 2)
  ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

:1:1 note: in block
$B1: {  # root
^^^

:5:3 error: var: invalid texture dimension 'none'
  %none_invalid:ptr<storage, texture_storage_none<rgba32float, read_write>, read_write> = var @binding_point(3, 3)
  ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

:1:1 note: in block
$B1: {  # root
^^^

note: # Disassembly
$B1: {  # root
  %valid:ptr<storage, texture_storage_2d<rgba32float, read_write>, read_write> = var @binding_point(0, 0)
  %cube_invalid:ptr<storage, texture_storage_cube<rgba32float, read_write>, read_write> = var @binding_point(1, 1)
  %cube_array_invalid:ptr<storage, texture_storage_cube_array<rgba32float, read_write>, read_write> = var @binding_point(2, 2)
  %none_invalid:ptr<storage, texture_storage_none<rgba32float, read_write>, read_write> = var @binding_point(3, 3)
}

)");
}

namespace {
template <typename T>
static const core::type::Type* TypeBuilder(core::type::Manager& m) {
    return m.Get<T>();
}

template <typename T>
static const core::type::Type* RefTypeBuilder(core::type::Manager& m) {
    return m.ref<AddressSpace::kFunction, T>();
}

using TypeBuilderFn = decltype(&TypeBuilder<i32>);
}  // namespace

using IR_ValidatorRefTypeTest = IRTestParamHelper<std::tuple</* holds_ref */ bool,
                                                             /* refs_allowed */ bool,
                                                             /* type_builder */ TypeBuilderFn>>;

TEST_P(IR_ValidatorRefTypeTest, Var) {
    bool holds_ref = std::get<0>(GetParam());
    bool refs_allowed = std::get<1>(GetParam());
    auto* type = std::get<2>(GetParam())(ty);

    auto* fn = b.Function("my_func", ty.void_());
    b.Append(fn->Block(), [&] {
        if (auto* view = type->As<core::type::MemoryView>()) {
            b.Var(view);
        } else {
            b.Var(ty.ptr<function>(type));
        }

        b.Return(fn);
    });

    Capabilities caps;
    if (refs_allowed) {
        caps.Add(Capability::kAllowRefTypes);
    }
    auto res = ir::Validate(mod, caps);
    if (!holds_ref || refs_allowed) {
        ASSERT_EQ(res, Success) << res.Failure();
    } else {
        ASSERT_NE(res, Success);
        EXPECT_THAT(res.Failure().reason.Str(),
                    testing::HasSubstr("3:5 error: var: reference types are not permitted"));
    }
}

TEST_P(IR_ValidatorRefTypeTest, FnParam) {
    bool holds_ref = std::get<0>(GetParam());
    bool refs_allowed = std::get<1>(GetParam());
    auto* type = std::get<2>(GetParam())(ty);

    auto* fn = b.Function("my_func", ty.void_());
    fn->SetParams(Vector{b.FunctionParam(type)});
    b.Append(fn->Block(), [&] { b.Return(fn); });

    Capabilities caps;
    if (refs_allowed) {
        caps.Add(Capability::kAllowRefTypes);
    }
    auto res = ir::Validate(mod, caps);
    if (!holds_ref) {
        ASSERT_EQ(res, Success) << res.Failure();
    } else {
        ASSERT_NE(res, Success);
        EXPECT_THAT(res.Failure().reason.Str(),
                    testing::HasSubstr("reference types are not permitted"));
    }
}

TEST_P(IR_ValidatorRefTypeTest, FnRet) {
    bool holds_ref = std::get<0>(GetParam());
    bool refs_allowed = std::get<1>(GetParam());
    auto* type = std::get<2>(GetParam())(ty);

    auto* fn = b.Function("my_func", type);
    b.Append(fn->Block(), [&] { b.Unreachable(); });

    Capabilities caps;
    if (refs_allowed) {
        caps.Add(Capability::kAllowRefTypes);
    }
    auto res = ir::Validate(mod, caps);
    if (!holds_ref) {
        ASSERT_EQ(res, Success) << res.Failure();
    } else {
        ASSERT_NE(res, Success);
        EXPECT_THAT(res.Failure().reason.Str(),
                    testing::HasSubstr("reference types are not permitted"));
    }
}

TEST_P(IR_ValidatorRefTypeTest, BlockParam) {
    bool holds_ref = std::get<0>(GetParam());
    bool refs_allowed = std::get<1>(GetParam());
    auto* type = std::get<2>(GetParam())(ty);

    auto* fn = b.Function("my_func", ty.void_());
    b.Append(fn->Block(), [&] {
        auto* loop = b.Loop();
        loop->Continuing()->SetParams({b.BlockParam(type)});
        b.Append(loop->Body(), [&] {  //
            b.Continue(loop, nullptr);
        });
        b.Append(loop->Continuing(), [&] {  //
            b.NextIteration(loop);
        });
        b.Unreachable();
    });

    Capabilities caps;
    if (refs_allowed) {
        caps.Add(Capability::kAllowRefTypes);
    }
    auto res = ir::Validate(mod, caps);
    if (!holds_ref) {
        ASSERT_EQ(res, Success) << res.Failure();
    } else {
        ASSERT_NE(res, Success);
        EXPECT_THAT(res.Failure().reason.Str(),
                    testing::HasSubstr("reference types are not permitted"));
    }
}

INSTANTIATE_TEST_SUITE_P(NonRefTypes,
                         IR_ValidatorRefTypeTest,
                         testing::Combine(/* holds_ref */ testing::Values(false),
                                          /* refs_allowed */ testing::Values(false, true),
                                          /* type_builder */
                                          testing::Values(TypeBuilder<i32>,
                                                          TypeBuilder<bool>,
                                                          TypeBuilder<vec4<f32>>,
                                                          TypeBuilder<array<f32, 3>>)));

INSTANTIATE_TEST_SUITE_P(RefTypes,
                         IR_ValidatorRefTypeTest,
                         testing::Combine(/* holds_ref */ testing::Values(true),
                                          /* refs_allowed */ testing::Values(false, true),
                                          /* type_builder */
                                          testing::Values(RefTypeBuilder<i32>,
                                                          RefTypeBuilder<bool>,
                                                          RefTypeBuilder<vec4<f32>>)));

TEST_F(IR_ValidatorTest, PointerToPointer) {
    auto* type = ty.ptr<function, ptr<function, i32>>();
    auto* fn = b.Function("my_func", ty.void_());
    fn->SetParams(Vector{b.FunctionParam(type)});
    b.Append(fn->Block(), [&] {  //
        b.Return(fn);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason.Str(),
                testing::HasSubstr("nested pointer types are not permitted"));
}

TEST_F(IR_ValidatorTest, PointerToVoid) {
    auto* type = ty.ptr(AddressSpace::kFunction, ty.void_());
    auto* fn = b.Function("my_func", ty.void_());
    fn->SetParams(Vector{b.FunctionParam(type)});
    b.Append(fn->Block(), [&] {  //
        b.Return(fn);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason.Str(),
                testing::HasSubstr("pointers to void are not permitted"));
}

TEST_F(IR_ValidatorTest, ReferenceToReference) {
    auto* type = ty.ref<function>(ty.ref<function, i32>());
    auto* fn = b.Function("my_func", ty.void_());
    b.Append(fn->Block(), [&] {  //
        b.Var(type);
        b.Return(fn);
    });

    Capabilities caps;
    caps.Add(Capability::kAllowRefTypes);

    auto res = ir::Validate(mod, caps);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason.Str(),
                testing::HasSubstr("nested reference types are not permitted"));
}

TEST_F(IR_ValidatorTest, ReferenceToVoid) {
    auto* type = ty.ref(AddressSpace::kFunction, ty.void_());
    auto* fn = b.Function("my_func", ty.void_());
    b.Append(fn->Block(), [&] {  //
        b.Var(type);
        b.Return(fn);
    });

    Capabilities caps;
    caps.Add(Capability::kAllowRefTypes);

    auto res = ir::Validate(mod, caps);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason.Str(),
                testing::HasSubstr("references to void are not permitted"));
}

TEST_F(IR_ValidatorTest, PointerInStructure_WithoutCapability) {
    auto* str_ty =
        ty.Struct(mod.symbols.New("S"), {
                                            {mod.symbols.New("a"), ty.ptr<private_, i32>()},
                                        });
    mod.root_block->Append(b.Var("my_struct", private_, str_ty));

    auto* fn = b.Function("F", ty.void_());
    b.Append(fn->Block(), [&] { b.Return(fn); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason.Str(),
                testing::HasSubstr("nested pointer types are not permitted"));
}

TEST_F(IR_ValidatorTest, PointerInStructure_WithCapability) {
    auto* str_ty =
        ty.Struct(mod.symbols.New("S"), {
                                            {mod.symbols.New("a"), ty.ptr<private_, i32>()},
                                        });

    auto* fn = b.Function("F", ty.void_());
    auto* param = b.FunctionParam("param", str_ty);
    fn->SetParams({param});
    b.Append(fn->Block(), [&] { b.Return(fn); });

    auto res = ir::Validate(mod, Capabilities{Capability::kAllowPointersAndHandlesInStructures});
    EXPECT_EQ(res, Success) << res.Failure();
}

using IR_Validator8BitIntTypeTest = IRTestParamHelper<std::tuple<
    /* int8_allowed */ bool,
    /* type_builder */ TypeBuilderFn>>;

TEST_P(IR_Validator8BitIntTypeTest, Var) {
    bool int8_allowed = std::get<0>(GetParam());
    auto* type = std::get<1>(GetParam())(ty);

    auto* fn = b.Function("my_func", ty.void_());
    b.Append(fn->Block(), [&] {
        b.Var(ty.ptr<function>(type));
        b.Return(fn);
    });

    Capabilities caps;
    if (int8_allowed) {
        caps.Add(Capability::kAllow8BitIntegers);
    }
    auto res = ir::Validate(mod, caps);
    if (int8_allowed) {
        ASSERT_EQ(res, Success) << res.Failure();
    } else {
        ASSERT_NE(res, Success);
        EXPECT_THAT(res.Failure().reason.Str(),
                    testing::HasSubstr("3:5 error: var: 8-bit integer types are not permitted"));
    }
}

TEST_P(IR_Validator8BitIntTypeTest, FnParam) {
    bool int8_allowed = std::get<0>(GetParam());
    auto* type = std::get<1>(GetParam())(ty);

    auto* fn = b.Function("my_func", ty.void_());
    fn->SetParams(Vector{b.FunctionParam(type)});
    b.Append(fn->Block(), [&] { b.Return(fn); });

    Capabilities caps;
    if (int8_allowed) {
        caps.Add(Capability::kAllow8BitIntegers);
    }
    auto res = ir::Validate(mod, caps);
    if (int8_allowed) {
        ASSERT_EQ(res, Success) << res.Failure();
    } else {
        ASSERT_NE(res, Success);
        EXPECT_THAT(res.Failure().reason.Str(),
                    testing::HasSubstr("8-bit integer types are not permitted"));
    }
}

TEST_P(IR_Validator8BitIntTypeTest, FnRet) {
    bool int8_allowed = std::get<0>(GetParam());
    auto* type = std::get<1>(GetParam())(ty);

    auto* fn = b.Function("my_func", type);
    b.Append(fn->Block(), [&] { b.Unreachable(); });

    Capabilities caps;
    if (int8_allowed) {
        caps.Add(Capability::kAllow8BitIntegers);
    }
    auto res = ir::Validate(mod, caps);
    if (int8_allowed) {
        ASSERT_EQ(res, Success) << res.Failure();
    } else {
        ASSERT_NE(res, Success);
        EXPECT_THAT(res.Failure().reason.Str(),
                    testing::HasSubstr("8-bit integer types are not permitted"));
    }
}

TEST_P(IR_Validator8BitIntTypeTest, BlockParam) {
    bool int8_allowed = std::get<0>(GetParam());
    auto* type = std::get<1>(GetParam())(ty);

    auto* fn = b.Function("my_func", ty.void_());
    b.Append(fn->Block(), [&] {
        auto* loop = b.Loop();
        loop->Continuing()->SetParams({b.BlockParam(type)});
        b.Append(loop->Body(), [&] {  //
            b.Continue(loop, nullptr);
        });
        b.Append(loop->Continuing(), [&] {  //
            b.NextIteration(loop);
        });
        b.Unreachable();
    });

    Capabilities caps;
    if (int8_allowed) {
        caps.Add(Capability::kAllow8BitIntegers);
    }
    auto res = ir::Validate(mod, caps);
    if (int8_allowed) {
        ASSERT_EQ(res, Success) << res.Failure();
    } else {
        ASSERT_NE(res, Success);
        EXPECT_THAT(res.Failure().reason.Str(),
                    testing::HasSubstr("8-bit integer types are not permitted"));
    }
}

INSTANTIATE_TEST_SUITE_P(Int8Types,
                         IR_Validator8BitIntTypeTest,
                         testing::Combine(
                             /* int8_allowed */ testing::Values(false, true),
                             /* type_builder */
                             testing::Values(TypeBuilder<i8>,
                                             TypeBuilder<u8>,
                                             TypeBuilder<vec4<i8>>,
                                             TypeBuilder<array<u8, 4>>)));

TEST_F(IR_ValidatorTest, Int8Type_InstructionOperand_NotAllowed) {
    auto* fn = b.Function("my_func", ty.void_());
    b.Append(fn->Block(), [&] {
        b.Let("l", u8(1));
        b.Return(fn);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:3:5 error: let: 8-bit integer types are not permitted
    %l:u8 = let 1u8
    ^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %l:u8 = let 1u8
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Int8Type_InstructionOperand_Allowed) {
    auto* fn = b.Function("my_func", ty.void_());
    b.Append(fn->Block(), [&] {
        b.Let("l", u8(1));
        b.Return(fn);
    });

    auto res = ir::Validate(mod, Capabilities{Capability::kAllow8BitIntegers});
    ASSERT_EQ(res, Success) << res.Failure();
}

}  // namespace tint::core::ir
