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

#include "src/tint/lang/spirv/reader/lower/transpose_row_major.h"

#include "src/tint/lang/core/ir/transform/helper_test.h"

namespace tint::spirv::reader::lower {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

class SpirvReader_TransposeRowMajorTest : public core::ir::transform::TransformTest {
    void SetUp() override { capabilities.Add(core::ir::Capability::kAllowNonCoreTypes); }
};

TEST_F(SpirvReader_TransposeRowMajorTest, DISABLED_ReadUniformMatrix) {
    // struct S {
    //   @offset(16) @row_major m : mat2x3<f32>,
    // };
    // @group(0) @binding(0) var<uniform> s : S;
    //
    // @compute @workgroup_size(1)
    // fn f() {
    //   let x : mat2x3<f32> = s.m;
    // }

    auto* matrix_member = ty.Get<core::type::StructMember>(mod.symbols.New("m"), ty.mat2x3<f32>(),
                                                           0u, 16u, 24u, 24u, core::IOAttributes{});
    matrix_member->SetRowMajor();

    auto* strct = ty.Struct(mod.symbols.New("S"), Vector{matrix_member});

    auto* var = b.Var("s", ty.ptr<uniform>(strct));
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* f = b.ComputeFunction("f");
    b.Append(f->Block(), [&] {
        b.Let("x", b.Load(b.Access<ptr<uniform, mat2x3<f32>>>(var, 0_u)));
        b.Return(f);
    });

    auto* before = R"(
S = struct @align(24) {
  m:mat2x3<f32> @offset(16) @size(24), @row_major
}

$B1: {  # root
  %s:ptr<uniform, S, read> = var undef @binding_point(0, 0)
}

%f = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<uniform, mat2x3<f32>, read> = access %s, 0u
    %4:mat2x3<f32> = load %3
    %x:mat2x3<f32> = let %4
    ret
  }
}
)";

    ASSERT_EQ(before, str());

    auto* after = R"(
S = struct @align(24) {
  m:mat3x2<f32> @offset(16) @size(24), @row_major
}

$B1: {  # root
  %s:ptr<uniform, S, read> = var undef @binding_point(0, 0)
}

%f = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<uniform, mat3x2<f32>, read> = access %s, 0u
    %4:mat3x2<f32> = load %3
    %5:mat2x3<f23> = transpose %4
    %x:mat2x3<f32> = let %5
    ret
  }
}
)";

    Run(TransposeRowMajor);
    EXPECT_EQ(after, str());
}

TEST_F(SpirvReader_TransposeRowMajorTest, DISABLED_ReadUniformColumn) {
    // struct S {
    //   @offset(16)
    //   @row_major
    //   m : mat2x3<f32>,
    // };
    // @group(0) @binding(0) var<uniform> s : S;
    //
    // @compute @workgroup_size(1)
    // fn f() {
    //   let x : vec3<f32> = s.m[1];
    // }
    auto* matrix_member = ty.Get<core::type::StructMember>(mod.symbols.New("m"), ty.mat2x3<f32>(),
                                                           0u, 16u, 24u, 24u, core::IOAttributes{});
    matrix_member->SetRowMajor();

    auto* strct = ty.Struct(mod.symbols.New("S"), Vector{matrix_member});

    auto* var = b.Var("s", ty.ptr<uniform>(strct));
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* f = b.ComputeFunction("f");
    b.Append(f->Block(), [&] {
        b.Let("x", b.Load(b.Access<ptr<uniform, vec3<f32>>>(var, 0_u, 1_u)));
        b.Return(f);
    });

    auto* before = R"(
S = struct @align(24) {
  m:mat2x3<f32> @offset(16) @size(24), @row_major
}

$B1: {  # root
  %s:ptr<uniform, S, read> = var undef @binding_point(0, 0)
}

%f = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<uniform, vec3<f32>, read> = access %s, 0u, 1u
    %4:vec3<f32> = load %3
    %x:vec3<f32> = let %4
    ret
  }
}
)";

    ASSERT_EQ(before, str());

    auto* after = R"(
fn tint_load_row_major_column(tint_from : ptr<uniform, mat3x2<f32>>, tint_idx : u32) -> vec3<f32> {
  return vec3<f32>(tint_from[0][tint_idx], tint_from[1][tint_idx], tint_from[2][tint_idx]);
}

struct S {
  @size(16)
  padding_0 : u32,
  /* @offset(16u) */
  m : mat3x2<f32>,
}

@group(0) @binding(0) var<uniform> s : S;

@compute @workgroup_size(1i)
fn f() {
  let x : vec3<f32> = tint_load_row_major_column(&(s.m), u32(1i));
}
)";

    Run(TransposeRowMajor);
    EXPECT_EQ(after, str());
}

TEST_F(SpirvReader_TransposeRowMajorTest, DISABLED_ReadUniformElement) {
    // struct S {
    //   @offset(16)
    //   @row_major
    //   m : mat2x3<f32>,
    // };
    // @group(0) @binding(0) var<uniform> s : S;
    //
    // @compute @workgroup_size(1)
    // fn f() {
    //   let col_idx : i32 = 1i;
    //   let x : f32 = s.m[col_idx].z;
    // }
    auto* matrix_member = ty.Get<core::type::StructMember>(mod.symbols.New("m"), ty.mat2x3<f32>(),
                                                           0u, 16u, 24u, 24u, core::IOAttributes{});
    matrix_member->SetRowMajor();

    auto* strct = ty.Struct(mod.symbols.New("S"), Vector{matrix_member});

    auto* var = b.Var("s", ty.ptr<uniform>(strct));
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* f = b.ComputeFunction("f");
    b.Append(f->Block(), [&] {
        auto* col = b.Access<ptr<uniform, vec3<f32>>>(var, 0_u, 1_u);
        b.Let("x", b.LoadVectorElement(col, 3_u));
        b.Return(f);
    });

    auto* before = R"(
S = struct @align(24) {
  m:mat2x3<f32> @offset(16) @size(24), @row_major
}

$B1: {  # root
  %s:ptr<uniform, S, read> = var undef @binding_point(0, 0)
}

%f = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<uniform, vec3<f32>, read> = access %s, 0u, 1u
    %4:f32 = load_vector_element %3, 3u
    %x:f32 = let %4
    ret
  }
}
)";

    ASSERT_EQ(before, str());

    auto* after = R"(
struct S {
  @size(16)
  padding_0 : u32,
  /* @offset(16u) */
  m : mat3x2<f32>,
}

@group(0) @binding(0) var<uniform> s : S;

@compute @workgroup_size(1i)
fn f() {
  let col_idx : i32 = 1i;
  let x : f32 = s.m[2u][col_idx];
}
)";

    Run(TransposeRowMajor);
    EXPECT_EQ(after, str());
}

TEST_F(SpirvReader_TransposeRowMajorTest, DISABLED_ReadUniformSwizzle) {
    // struct S {
    //   @offset(16)
    //   @row_major
    //   m : mat2x3<f32>,
    // };
    // @group(0) @binding(0) var<uniform> s : S;
    //
    // @compute @workgroup_size(1)
    // fn f() {
    //   let col_idx : i32 = 1i;
    //   let x : vec2<f32> = s.m[1].zx;
    // }
    auto* matrix_member = ty.Get<core::type::StructMember>(mod.symbols.New("m"), ty.mat2x3<f32>(),
                                                           0u, 16u, 24u, 24u, core::IOAttributes{});
    matrix_member->SetRowMajor();

    auto* strct = ty.Struct(mod.symbols.New("S"), Vector{matrix_member});

    auto* var = b.Var("s", ty.ptr<uniform>(strct));
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* f = b.ComputeFunction("f");
    b.Append(f->Block(), [&] {
        auto* a = b.Access<ptr<uniform, vec3<f32>>>(var, 0_u, 1_u);
        auto* l = b.Load(a);
        b.Let("x", b.Swizzle(ty.vec2<f32>(), l, Vector{2u, 0u}));
        b.Return(f);
    });

    auto* before = R"(
S = struct @align(24) {
  m:mat2x3<f32> @offset(16) @size(24), @row_major
}

$B1: {  # root
  %s:ptr<uniform, S, read> = var undef @binding_point(0, 0)
}

%f = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<uniform, vec3<f32>, read> = access %s, 0u, 1u
    %4:vec3<f32> = load %3
    %5:vec2<f32> = swizzle %4, zx
    %x:vec2<f32> = let %5
    ret
  }
}
)";

    ASSERT_EQ(before, str());

    auto* after = R"(
fn tint_load_row_major_column(tint_from : ptr<uniform, mat3x2<f32>>, tint_idx : u32) -> vec3<f32> {
  return vec3<f32>(tint_from[0][tint_idx], tint_from[1][tint_idx], tint_from[2][tint_idx]);
}

struct S {
  @size(16)
  padding_0 : u32,
  /* @offset(16u) */
  m : mat3x2<f32>,
}

@group(0) @binding(0) var<uniform> s : S;

@compute @workgroup_size(1i)
fn f() {
  let col_idx : i32 = 1i;
  let x : vec2<f32> = tint_load_row_major_column(&(s.m), u32(col_idx)).zx;
}
)";

    Run(TransposeRowMajor);
    EXPECT_EQ(after, str());
}

TEST_F(SpirvReader_TransposeRowMajorTest, DISABLED_WriteStorageMatrix) {
    // struct S {
    //   @offset(8)
    //   @row_major
    //   m : mat2x3<f32>,
    // };
    // @group(0) @binding(0) var<storage, read_write> s : S;
    //
    // @compute @workgroup_size(1)
    // fn f() {
    //   s.m = mat2x3<f32>(1.0, 2.0, 3.0, 4.0, 5.0, 6.0);
    // }
    auto* matrix_member = ty.Get<core::type::StructMember>(mod.symbols.New("m"), ty.mat2x3<f32>(),
                                                           0u, 8u, 24u, 24u, core::IOAttributes{});
    matrix_member->SetRowMajor();

    auto* strct = ty.Struct(mod.symbols.New("S"), Vector{matrix_member});

    auto* var = b.Var("s", ty.ptr(storage, strct, read_write));
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* f = b.ComputeFunction("f");
    b.Append(f->Block(), [&] {
        auto* m = b.Construct(ty.mat2x3<f32>(), 1_f, 2_f, 3_f, 4_f, 5_f, 6_f);
        auto* a = b.Access<ptr<storage, mat2x3<f32>, read_write>>(var, 0_u);
        b.Store(a, m);
        b.Return(f);
    });

    auto* before = R"(
S = struct @align(24) {
  m:mat2x3<f32> @offset(8) @size(24), @row_major
}

$B1: {  # root
  %s:ptr<storage, S, read_write> = var undef @binding_point(0, 0)
}

%f = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:mat2x3<f32> = construct 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f
    %4:ptr<storage, mat2x3<f32>, read_write> = access %s, 0u
    store %4, %3
    ret
  }
}
)";

    ASSERT_EQ(before, str());

    auto* after = R"(
struct S {
  @size(8)
  padding_0 : u32,
  /* @offset(8u) */
  m : mat3x2<f32>,
}

@group(0) @binding(0) var<storage, read_write> s : S;

@compute @workgroup_size(1i)
fn f() {
  s.m = transpose(mat2x3<f32>(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f));
}
)";

    Run(TransposeRowMajor);
    EXPECT_EQ(after, str());
}

TEST_F(SpirvReader_TransposeRowMajorTest, DISABLED_WriteStorageColumn) {
    // struct S {
    //   @offset(8)
    //   @row_major
    //   m : mat2x3<f32>,
    // };
    // @group(0) @binding(0) var<storage, read_write> s : S;
    //
    // @compute @workgroup_size(1)
    // fn f() {
    //   let col_idx : i32 = 1i;
    //   s.m[1] = vec3<f32>(1.0, 2.0, 3.0);
    // }
    auto* matrix_member = ty.Get<core::type::StructMember>(mod.symbols.New("m"), ty.mat2x3<f32>(),
                                                           0u, 8u, 24u, 24u, core::IOAttributes{});
    matrix_member->SetRowMajor();

    auto* strct = ty.Struct(mod.symbols.New("S"), Vector{matrix_member});

    auto* var = b.Var("s", ty.ptr(storage, strct, read_write));
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* f = b.ComputeFunction("f");
    b.Append(f->Block(), [&] {
        auto* a = b.Access<ptr<storage, vec3<f32>, read_write>>(var, 0_u, 1_u);
        auto* c = b.Construct(ty.vec3<f32>(), 1_f, 2_f, 3_f);
        b.Store(a, c);
        b.Return(f);
    });

    auto* before = R"(
S = struct @align(24) {
  m:mat2x3<f32> @offset(8) @size(24), @row_major
}

$B1: {  # root
  %s:ptr<storage, S, read_write> = var undef @binding_point(0, 0)
}

%f = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<storage, vec3<f32>, read_write> = access %s, 0u, 1u
    %4:vec3<f32> = construct 1.0f, 2.0f, 3.0f
    store %3, %4
    ret
  }
}
)";

    ASSERT_EQ(before, str());

    auto* after = R"(
fn tint_store_row_major_column(tint_to : ptr<storage, mat3x2<f32>, read_write>, tint_idx : u32, tint_col : vec3<f32>) {
  tint_to[0][tint_idx] = tint_col[0];
  tint_to[1][tint_idx] = tint_col[1];
  tint_to[2][tint_idx] = tint_col[2];
}

struct S {
  @size(8)
  padding_0 : u32,
  /* @offset(8u) */
  m : mat3x2<f32>,
}

@group(0) @binding(0) var<storage, read_write> s : S;

@compute @workgroup_size(1i)
fn f() {
  let col_idx : i32 = 1i;
  tint_store_row_major_column(&(s.m), u32(col_idx), vec3<f32>(1.0f, 2.0f, 3.0f));
}
)";

    Run(TransposeRowMajor);
    EXPECT_EQ(after, str());
}

TEST_F(SpirvReader_TransposeRowMajorTest, DISABLED_WriteStorageElement_Accessor) {
    // struct S {
    //   @offset(8)
    //   @row_major
    //   m : mat2x3<f32>,
    // };
    // @group(0) @binding(0) var<storage, read_write> s : S;
    //
    // @compute @workgroup_size(1)
    // fn f() {
    //   let col_idx : i32 = 1i;
    //   s.m[1].z = 1.0;
    // }
    auto* matrix_member = ty.Get<core::type::StructMember>(mod.symbols.New("m"), ty.mat2x3<f32>(),
                                                           0u, 8u, 24u, 24u, core::IOAttributes{});
    matrix_member->SetRowMajor();

    auto* strct = ty.Struct(mod.symbols.New("S"), Vector{matrix_member});

    auto* var = b.Var("s", ty.ptr(storage, strct, read_write));
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* f = b.ComputeFunction("f");
    b.Append(f->Block(), [&] {
        auto* a = b.Access<ptr<storage, vec3<f32>>>(var, 0_u, 1_u);
        b.StoreVectorElement(a, 2_u, 1_f);
        b.Return(f);
    });

    auto* before = R"(
S = struct @align(24) {
  m:mat2x3<f32> @offset(8) @size(24), @row_major
}

$B1: {  # root
  %s:ptr<storage, S, read_write> = var undef @binding_point(0, 0)
}

%f = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<storage, vec3<f32>, read_write> = access %s, 0u, 1u
    store_vector_element %3, 2u, 1.0f
    ret
  }
}
)";

    ASSERT_EQ(before, str());

    auto* after = R"(
struct S {
  @size(8)
  padding_0 : u32,
  /* @offset(8u) */
  m : mat3x2<f32>,
}

@group(0) @binding(0) var<storage, read_write> s : S;

@compute @workgroup_size(1i)
fn f() {
  let col_idx : i32 = 1i;
  s.m[2u][col_idx] = 1.0f;
}
)";

    Run(TransposeRowMajor);
    EXPECT_EQ(after, str());
}

TEST_F(SpirvReader_TransposeRowMajorTest, DISABLED_ExtractFromLoadedStruct) {
    // struct S {
    //   @offset(16)
    //   @row_major
    //   m : mat2x3<f32>,
    // };
    // @group(0) @binding(0) var<uniform> s : S;
    //
    // @compute @workgroup_size(1)
    // fn f() {
    //   let col_idx : i32 = 1i;
    //   let row_idx : i32 = 2i;
    //   let load = s;
    //   let m : mat2x3<f32> = load.m;
    //   let c : vec3<f32> = load.m[col_idx];
    //   let e : vec3<f32> = load.m[col_idx][row_idx];
    // }
    auto* matrix_member = ty.Get<core::type::StructMember>(mod.symbols.New("m"), ty.mat2x3<f32>(),
                                                           0u, 16u, 24u, 24u, core::IOAttributes{});
    matrix_member->SetRowMajor();

    auto* strct = ty.Struct(mod.symbols.New("S"), Vector{matrix_member});

    auto* var = b.Var("s", ty.ptr<uniform>(strct));
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* f = b.ComputeFunction("f");
    b.Append(f->Block(), [&] {
        auto* ls = b.Load(var);
        auto* load = b.Let("load", ls);
        b.Let("m", b.Access(ty.mat2x3<f32>(), load, 0_u));
        b.Let("c", b.Access(ty.vec3<f32>(), load, 0_u, 1_u));
        b.Let("e", b.Access(ty.f32(), load, 0_u, 1_u, 2_u));
        b.Return(f);
    });

    auto* before = R"(
S = struct @align(24) {
  m:mat2x3<f32> @offset(16) @size(24), @row_major
}

$B1: {  # root
  %s:ptr<uniform, S, read> = var undef @binding_point(0, 0)
}

%f = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:S = load %s
    %load:S = let %3
    %5:mat2x3<f32> = access %load, 0u
    %m:mat2x3<f32> = let %5
    %7:vec3<f32> = access %load, 0u, 1u
    %c:vec3<f32> = let %7
    %9:f32 = access %load, 0u, 1u, 2u
    %e:f32 = let %9
    ret
  }
}
)";

    ASSERT_EQ(before, str());

    auto* after = R"(
struct S {
  @size(16)
  padding_0 : u32,
  /* @offset(16u) */
  m : mat3x2<f32>,
}

@group(0) @binding(0) var<uniform> s : S;

@compute @workgroup_size(1i)
fn f() {
  let col_idx : i32 = 1i;
  let row_idx : i32 = 2i;
  let load : S = s;
  let m : mat2x3<f32> = transpose(load.m);
  let c : vec3<f32> = transpose(load.m)[col_idx];
  let e : f32 = transpose(load.m)[col_idx][row_idx];
}
)";

    Run(TransposeRowMajor);
    EXPECT_EQ(after, str());
}

TEST_F(SpirvReader_TransposeRowMajorTest, DISABLED_InsertInStructConstructor) {
    // struct S {
    //   @offset(0) @row_major m1 : mat2x3<f32>,
    //   @offset(32) m2 : mat4x2<f32>,
    //   @offset(64) @row_major m3 : mat4x2<f32>,
    // };
    // @group(0) @binding(0) var<uniform> s : S;
    //
    // @compute @workgroup_size(1)
    // fn f() {
    //   let m1 = mat2x3<f32>();
    //   let m2 = mat4x2<f32>();
    //   s = S(m, m2, m2);
    // }
    auto* matrix_member_0 = ty.Get<core::type::StructMember>(
        mod.symbols.New("m"), ty.mat2x3<f32>(), 0u, 0u, 24u, 24u, core::IOAttributes{});
    matrix_member_0->SetRowMajor();

    auto* matrix_member_1 = ty.Get<core::type::StructMember>(
        mod.symbols.New("m"), ty.mat4x2<f32>(), 1u, 32u, 32u, 32u, core::IOAttributes{});

    auto* matrix_member_2 = ty.Get<core::type::StructMember>(
        mod.symbols.New("m"), ty.mat4x2<f32>(), 2u, 64u, 32u, 32u, core::IOAttributes{});
    matrix_member_2->SetRowMajor();

    auto* strct =
        ty.Struct(mod.symbols.New("S"), Vector{matrix_member_0, matrix_member_1, matrix_member_2});

    auto* var = b.Var("s", ty.ptr(storage, strct, read_write));
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* f = b.ComputeFunction("f");
    b.Append(f->Block(), [&] {
        auto* m1 = b.Construct(ty.mat2x3<f32>());
        auto* m2 = b.Construct(ty.mat4x2<f32>());

        b.Store(var, b.Construct(strct, m1, m2, m2));
        b.Return(f);
    });

    auto* before = R"(
S = struct @align(32) {
  m:mat2x3<f32> @offset(0) @size(24), @row_major
  m_1:mat4x2<f32> @offset(32)
  m_2:mat4x2<f32> @offset(64), @row_major
}

$B1: {  # root
  %s:ptr<storage, S, read_write> = var undef @binding_point(0, 0)
}

%f = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:mat2x3<f32> = construct
    %4:mat4x2<f32> = construct
    %5:S = construct %3, %4, %4
    store %s, %5
    ret
  }
}
)";

    ASSERT_EQ(before, str());

    auto* after = R"(
struct S {
  /* @offset(0u) */
  m : mat3x2<f32>,
  @size(8)
  padding_0 : u32,
  /* @offset(32u) */
  m1 : mat4x2<f32>,
  /* @offset(64u) */
  m2 : mat2x4<f32>,
}

@group(0) @binding(0) var<storage, read_write> s : S;

@compute @workgroup_size(1i)
fn f() {
  let m1 = mat2x3f();
  let m2 = mat4x2f();
  s = S(transpose(m1), m2, transpose(m2));
}
)";

    Run(TransposeRowMajor);
    EXPECT_EQ(after, str());
}

TEST_F(SpirvReader_TransposeRowMajorTest, DISABLED_DeeplyNested) {
    // struct Inner {
    //   @offset(0)
    //   @row_major
    //   m : mat4x3<f32>,
    // };
    // struct Outer {
    //   @offset(0)
    //   arr : array<Inner, 4>,
    // };
    // @group(0) @binding(0) var<storage, read_write> buffer : Outer;
    //
    // @compute @workgroup_size(1)
    // fn f() {
    //   let m = buffer.arr[1].m;
    //   buffer.arr[0].m[3] = m[2];
    // }
    auto* matrix_member = ty.Get<core::type::StructMember>(mod.symbols.New("m"), ty.mat4x3<f32>(),
                                                           0u, 16u, 24u, 24u, core::IOAttributes{});
    matrix_member->SetRowMajor();

    auto* inner_strct = ty.Struct(mod.symbols.New("Inner"), Vector{matrix_member});

    auto* outer_strct =
        ty.Struct(mod.symbols.New("Outer"), {{mod.symbols.New("arr"), ty.array(inner_strct, 4_u)}});

    auto* var = b.Var("s", ty.ptr(storage, outer_strct, read_write));
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* f = b.ComputeFunction("f");
    b.Append(f->Block(), [&] {
        auto* m = b.Let("m", b.Load(b.Access<ptr<storage, mat4x3<f32>>>(var, 0_u, 1_u, 0_u)));
        auto* ptr = b.Access(ty.ptr(storage, ty.vec3<f32>(), read_write), var, 0_u, 0_u, 0_u, 3_u);
        b.Store(ptr, b.Access(ty.vec3<f32>(), m, 2_u));
        b.Return(f);
    });

    auto* before = R"(
Inner = struct @align(24) {
  m:mat4x3<f32> @offset(16) @size(24), @row_major
}

Outer = struct @align(24) {
  arr:array<Inner, 4> @offset(0)
}

$B1: {  # root
  %s:ptr<storage, Outer, read_write> = var undef @binding_point(0, 0)
}

%f = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<storage, mat4x3<f32>, read_write> = access %s, 0u, 1u, 0u
    %4:mat4x3<f32> = load %3
    %m:mat4x3<f32> = let %4
    %6:ptr<storage, vec3<f32>, read_write> = access %s, 0u, 0u, 0u, 3u
    %7:vec3<f32> = access %m, 2u
    store %6, %7
    ret
  }
}
)";

    ASSERT_EQ(before, str());

    auto* after = R"(
fn tint_store_row_major_column(tint_to : ptr<storage, mat3x4<f32>, read_write>, tint_idx : u32, tint_col : vec3<f32>) {
  tint_to[0][tint_idx] = tint_col[0];
  tint_to[1][tint_idx] = tint_col[1];
  tint_to[2][tint_idx] = tint_col[2];
}

struct Inner {
  /* @offset(0u) */
  m : mat3x4<f32>,
}

struct Outer {
  /* @offset(0u) */
  arr : array<Inner, 4>,
}

@group(0) @binding(0) var<storage, read_write> buffer : Outer;

@compute @workgroup_size(1i)
fn f() {
  let m : mat4x3<f32> = transpose(buffer.arr[1].m);
  tint_store_row_major_column(&(buffer.arr[0].m), u32(3), m[2]);
}
)";

    Run(TransposeRowMajor);
    EXPECT_EQ(after, str());
}

TEST_F(SpirvReader_TransposeRowMajorTest, DISABLED_MultipleColumnHelpers) {
    // struct S {
    //   @offset(0) @row_major m1 : mat2x3<f32>,
    //   @offset(32) @row_major m2 : mat4x2<f32>,
    // };
    // @group(0) @binding(0) var<storage, read_write> s : S;
    // var<private> ps : S;
    //
    // @compute @workgroup_size(1)
    // fn f() {
    //   ps.m1[0] = s.m1[1];
    //   ps.m1[1] = s.m1[0];
    //   ps.m2[2] = s.m2[3];
    //   ps.m2[3] = s.m2[2];
    //
    //   s.m1[0] = ps.m1[0];
    //   s.m1[1] = ps.m1[1];
    //   s.m2[2] = ps.m2[2];
    //   s.m2[3] = ps.m2[3];
    // }
    auto* matrix_member_0 = ty.Get<core::type::StructMember>(
        mod.symbols.New("m"), ty.mat2x3<f32>(), 0u, 0u, 24u, 24u, core::IOAttributes{});
    matrix_member_0->SetRowMajor();

    auto* matrix_member_1 = ty.Get<core::type::StructMember>(
        mod.symbols.New("m"), ty.mat4x2<f32>(), 1u, 32u, 24u, 24u, core::IOAttributes{});
    matrix_member_1->SetRowMajor();

    auto* strct = ty.Struct(mod.symbols.New("S"), Vector{matrix_member_0, matrix_member_1});

    auto* s = b.Var("s", ty.ptr(storage, strct, read_write));
    s->SetBindingPoint(0, 0);
    mod.root_block->Append(s);

    auto* ps = b.Var("ps", ty.ptr(private_, strct));
    mod.root_block->Append(ps);

    auto* f = b.ComputeFunction("f");
    b.Append(f->Block(), [&] {
        auto* sm10 = b.Access(ty.ptr<storage, vec3<f32>, read_write>(), s, 0_u, 0_u);
        auto* sm11 = b.Access(ty.ptr<storage, vec3<f32>, read_write>(), s, 0_u, 1_u);
        auto* sm22 = b.Access(ty.ptr<storage, vec2<f32>, read_write>(), s, 1_u, 2_u);
        auto* sm23 = b.Access(ty.ptr<storage, vec2<f32>, read_write>(), s, 1_u, 3_u);

        auto* psm10 = b.Access(ty.ptr<private_, vec3<f32>, read_write>(), ps, 0_u, 0_u);
        auto* psm11 = b.Access(ty.ptr<private_, vec3<f32>, read_write>(), ps, 0_u, 1_u);
        auto* psm22 = b.Access(ty.ptr<private_, vec2<f32>, read_write>(), ps, 1_u, 2_u);
        auto* psm23 = b.Access(ty.ptr<private_, vec2<f32>, read_write>(), ps, 1_u, 3_u);

        b.Store(psm10, b.Load(sm11));
        b.Store(psm11, b.Load(sm10));
        b.Store(psm22, b.Load(sm23));
        b.Store(psm23, b.Load(sm22));

        b.Store(sm10, b.Load(psm10));
        b.Store(sm11, b.Load(psm11));
        b.Store(sm22, b.Load(psm22));
        b.Store(sm23, b.Load(psm23));

        b.Return(f);
    });

    auto* before = R"(
S = struct @align(24) {
  m:mat2x3<f32> @offset(0) @size(24), @row_major
  m_1:mat4x2<f32> @offset(32) @size(24), @row_major
}

$B1: {  # root
  %s:ptr<storage, S, read_write> = var undef @binding_point(0, 0)
  %ps:ptr<private, S, read_write> = var undef
}

%f = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %4:ptr<storage, vec3<f32>, read_write> = access %s, 0u, 0u
    %5:ptr<storage, vec3<f32>, read_write> = access %s, 0u, 1u
    %6:ptr<storage, vec2<f32>, read_write> = access %s, 1u, 2u
    %7:ptr<storage, vec2<f32>, read_write> = access %s, 1u, 3u
    %8:ptr<private, vec3<f32>, read_write> = access %ps, 0u, 0u
    %9:ptr<private, vec3<f32>, read_write> = access %ps, 0u, 1u
    %10:ptr<private, vec2<f32>, read_write> = access %ps, 1u, 2u
    %11:ptr<private, vec2<f32>, read_write> = access %ps, 1u, 3u
    %12:vec3<f32> = load %5
    store %8, %12
    %13:vec3<f32> = load %4
    store %9, %13
    %14:vec2<f32> = load %7
    store %10, %14
    %15:vec2<f32> = load %6
    store %11, %15
    %16:vec3<f32> = load %8
    store %4, %16
    %17:vec3<f32> = load %9
    store %5, %17
    %18:vec2<f32> = load %10
    store %6, %18
    %19:vec2<f32> = load %11
    store %7, %19
    ret
  }
}
)";

    ASSERT_EQ(before, str());

    auto* after = R"(
fn tint_load_row_major_column(tint_from : ptr<storage, mat3x2<f32>, read_write>, tint_idx : u32) -> vec3<f32> {
  return vec3<f32>(tint_from[0][tint_idx], tint_from[1][tint_idx], tint_from[2][tint_idx]);
}

fn tint_store_row_major_column(tint_to : ptr<private, mat3x2<f32>>, tint_idx : u32, tint_col : vec3<f32>) {
  tint_to[0][tint_idx] = tint_col[0];
  tint_to[1][tint_idx] = tint_col[1];
  tint_to[2][tint_idx] = tint_col[2];
}

fn tint_load_row_major_column_1(tint_from : ptr<storage, mat2x4<f32>, read_write>, tint_idx : u32) -> vec2<f32> {
  return vec2<f32>(tint_from[0][tint_idx], tint_from[1][tint_idx]);
}

fn tint_store_row_major_column_1(tint_to : ptr<private, mat2x4<f32>>, tint_idx : u32, tint_col : vec2<f32>) {
  tint_to[0][tint_idx] = tint_col[0];
  tint_to[1][tint_idx] = tint_col[1];
}

fn tint_load_row_major_column_2(tint_from : ptr<private, mat3x2<f32>>, tint_idx : u32) -> vec3<f32> {
  return vec3<f32>(tint_from[0][tint_idx], tint_from[1][tint_idx], tint_from[2][tint_idx]);
}

fn tint_store_row_major_column_2(tint_to : ptr<storage, mat3x2<f32>, read_write>, tint_idx : u32, tint_col : vec3<f32>) {
  tint_to[0][tint_idx] = tint_col[0];
  tint_to[1][tint_idx] = tint_col[1];
  tint_to[2][tint_idx] = tint_col[2];
}

fn tint_load_row_major_column_3(tint_from : ptr<private, mat2x4<f32>>, tint_idx : u32) -> vec2<f32> {
  return vec2<f32>(tint_from[0][tint_idx], tint_from[1][tint_idx]);
}

fn tint_store_row_major_column_3(tint_to : ptr<storage, mat2x4<f32>, read_write>, tint_idx : u32, tint_col : vec2<f32>) {
  tint_to[0][tint_idx] = tint_col[0];
  tint_to[1][tint_idx] = tint_col[1];
}

struct S {
  /* @offset(0u) */
  m1 : mat3x2<f32>,
  /* @offset(32u) */
  m2 : mat2x4<f32>,
}

@group(0) @binding(0) var<storage, read_write> s : S;

var<private> ps : S;

@compute @workgroup_size(1i)
fn f() {
  tint_store_row_major_column(&(ps.m1), u32(0u), tint_load_row_major_column(&(s.m1), u32(1u)));
  tint_store_row_major_column(&(ps.m1), u32(1u), tint_load_row_major_column(&(s.m1), u32(0u)));
  tint_store_row_major_column_1(&(ps.m2), u32(2u), tint_load_row_major_column_1(&(s.m2), u32(3u)));
  tint_store_row_major_column_1(&(ps.m2), u32(3u), tint_load_row_major_column_1(&(s.m2), u32(2u)));
  tint_store_row_major_column_2(&(s.m1), u32(0u), tint_load_row_major_column_2(&(ps.m1), u32(0u)));
  tint_store_row_major_column_2(&(s.m1), u32(1u), tint_load_row_major_column_2(&(ps.m1), u32(1u)));
  tint_store_row_major_column_3(&(s.m2), u32(2u), tint_load_row_major_column_3(&(ps.m2), u32(2u)));
  tint_store_row_major_column_3(&(s.m2), u32(3u), tint_load_row_major_column_3(&(ps.m2), u32(3u)));
}
)";

    Run(TransposeRowMajor);
    EXPECT_EQ(after, str());
}

TEST_F(SpirvReader_TransposeRowMajorTest, DISABLED_PreserveMatrixStride) {
    // struct S {
    //   @offset(0)
    //   @stride(32)
    //   @row_major
    //   m : mat2x3<f32>,
    // };
    // @group(0) @binding(0) var<uniform> s : S;
    //
    // @compute @workgroup_size(1)
    // fn f() {
    //   let x : mat2x3<f32> = s.m;
    // }
    auto* matrix_member = ty.Get<core::type::StructMember>(mod.symbols.New("m"), ty.mat2x3<f32>(),
                                                           0u, 0u, 24u, 24u, core::IOAttributes{});
    matrix_member->SetRowMajor();
    matrix_member->SetMatrixStride(32);

    auto* strct = ty.Struct(mod.symbols.New("S"), Vector{matrix_member});

    auto* var = b.Var("s", ty.ptr<uniform>(strct));
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* f = b.ComputeFunction("f");
    b.Append(f->Block(), [&] {
        b.Let("x", b.Load(b.Access<ptr<uniform, mat2x3<f32>>>(var, 0_u)));
        b.Return(f);
    });

    auto* before = R"(
S = struct @align(24) {
  m:mat2x3<f32> @offset(0) @size(24), @row_major, @matrix_stride(32)
}

$B1: {  # root
  %s:ptr<uniform, S, read> = var undef @binding_point(0, 0)
}

%f = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<uniform, mat2x3<f32>, read> = access %s, 0u
    %4:mat2x3<f32> = load %3
    %x:mat2x3<f32> = let %4
    ret
  }
}
)";

    ASSERT_EQ(before, str());

    auto* after = R"(
struct S {
  /* @offset(0u) */
  @stride(32) @internal(disable_validation__ignore_stride)
  m : mat3x2<f32>,
}

@group(0) @binding(0) var<uniform> s : S;

@compute @workgroup_size(1i)
fn f() {
  let x : mat2x3<f32> = transpose(s.m);
}
)";

    Run(TransposeRowMajor);
    EXPECT_EQ(after, str());
}

TEST_F(SpirvReader_TransposeRowMajorTest, DISABLED_ArrayOfMatrix_ReadWholeArray) {
    // struct S {
    //   @offset(0)
    //   @row_major
    //   @stride(8)
    //   arr : @stride(32) array<mat2x3<f32>, 4>,
    // };
    // @group(0) @binding(0) var<storage, read_write> s : S;
    //
    // @compute @workgroup_size(1)
    // fn f() {
    //   let x : array<mat2x3<f32>, 4> = s.arr;
    // }
    auto* matrix_member =
        ty.Get<core::type::StructMember>(mod.symbols.New("arr"), ty.array(ty.mat2x3<f32>(), 4), 0u,
                                         0u, 32u, 32u, core::IOAttributes{});
    matrix_member->SetRowMajor();

    auto* strct = ty.Struct(mod.symbols.New("S"), Vector{matrix_member});

    auto* var = b.Var("s", ty.ptr(storage, strct, read_write));
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* f = b.ComputeFunction("f");
    b.Append(f->Block(), [&] {
        b.Let("x", b.Load(b.Access<ptr<storage, array<mat2x3<f32>, 4>>>(var, 0_u)));
        b.Return(f);
    });

    auto* before = R"(
S = struct @align(32) {
  arr:array<mat2x3<f32>, 4> @offset(0) @size(32), @row_major
}

$B1: {  # root
  %s:ptr<storage, S, read_write> = var undef @binding_point(0, 0)
}

%f = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<storage, array<mat2x3<f32>, 4>, read_write> = access %s, 0u
    %4:array<mat2x3<f32>, 4> = load %3
    %x:array<mat2x3<f32>, 4> = let %4
    ret
  }
}
)";

    ASSERT_EQ(before, str());

    auto* after = R"(
fn tint_transpose_array(tint_from : @stride(32) array<mat3x2<f32>, 4u>) -> array<mat2x3<f32>, 4u> {
  var tint_result : array<mat2x3<f32>, 4u>;
  for(var i = 0u; (i < 4u); i++) {
    tint_result[i] = transpose(tint_from[i]);
  }
  return tint_result;
}

struct S {
  /* @offset(0u) */
  @stride(8) @internal(disable_validation__ignore_stride)
  arr : @stride(32) array<mat3x2<f32>, 4u>,
}

@group(0) @binding(0) var<storage, read_write> s : S;

@compute @workgroup_size(1i)
fn f() {
  let x : array<mat2x3<f32>, 4u> = tint_transpose_array(s.arr);
}
)";

    Run(TransposeRowMajor);
    EXPECT_EQ(after, str());
}

TEST_F(SpirvReader_TransposeRowMajorTest, DISABLED_ArrayOfMatrix_WriteWholeArray) {
    // struct S {
    //   @offset(0)
    //   @row_major
    //   @stride(8)
    //   arr : @stride(32) array<mat2x3<f32>, 4>,
    // };
    // @group(0) @binding(0) var<storage, read_write> s : S;
    //
    // @compute @workgroup_size(1)
    // fn f() {
    //   s.arr = array<mat2x3<f32>, 4>();
    // }
    auto* matrix_member =
        ty.Get<core::type::StructMember>(mod.symbols.New("arr"), ty.array(ty.mat2x3<f32>(), 4), 0u,
                                         0u, 32u, 32u, core::IOAttributes{});
    matrix_member->SetRowMajor();

    auto* strct = ty.Struct(mod.symbols.New("S"), Vector{matrix_member});

    auto* var = b.Var("s", ty.ptr(storage, strct, read_write));
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* f = b.ComputeFunction("f");
    b.Append(f->Block(), [&] {
        auto* a = b.Access(ty.ptr(storage, ty.array(ty.mat2x3<f32>(), 4)), var, 0_u);
        b.Store(a, b.Construct(ty.array<mat2x3<f32>, 4>()));
        b.Return(f);
    });

    auto* before = R"(
S = struct @align(32) {
  arr:array<mat2x3<f32>, 4> @offset(0) @size(32), @row_major
}

$B1: {  # root
  %s:ptr<storage, S, read_write> = var undef @binding_point(0, 0)
}

%f = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<storage, array<mat2x3<f32>, 4>, read_write> = access %s, 0u
    %4:array<mat2x3<f32>, 4> = construct
    store %3, %4
    ret
  }
}
)";

    ASSERT_EQ(before, str());

    auto* after = R"(
fn tint_transpose_array(tint_from : array<mat2x3<f32>, 4u>) -> @stride(32) array<mat3x2<f32>, 4u> {
  var tint_result : @stride(32) array<mat3x2<f32>, 4u>;
  for(var i = 0u; (i < 4u); i++) {
    tint_result[i] = transpose(tint_from[i]);
  }
  return tint_result;
}

struct S {
  /* @offset(0u) */
  @stride(8) @internal(disable_validation__ignore_stride)
  arr : @stride(32) array<mat3x2<f32>, 4u>,
}

@group(0) @binding(0) var<storage, read_write> s : S;

@compute @workgroup_size(1i)
fn f() {
  s.arr = tint_transpose_array(array<mat2x3<f32>, 4u>());
}
)";

    Run(TransposeRowMajor);
    EXPECT_EQ(after, str());
}

TEST_F(SpirvReader_TransposeRowMajorTest, DISABLED_ArrayOfMatrix_NestedArray) {
    // struct S {
    //   @offset(0)
    //   @row_major
    //   @stride(8)
    //   arr : @stride(128) array<@stride(32) array<mat2x3<f32>, 4>, 5>,
    // };
    // @group(0) @binding(0) var<storage, read_write> s : S;
    //
    // @compute @workgroup_size(1)
    // fn f() {
    //   let x = s.arr;
    //   s.arr = array<array<mat2x3<f32>, 4, 5>>();
    //   s.arr[0] = x[1];
    //   s.arr[1][2] = x[2][3];
    //   s.arr[2][3][1] = x[4][3][1];
    //   s.arr[4][2][0][1] = x[1][3][0][2];
    // }
    auto* matrix_member = ty.Get<core::type::StructMember>(
        mod.symbols.New("m"), ty.array(ty.array(ty.mat2x3<f32>(), 4), 5), 0u, 0u, 32u, 128u,
        core::IOAttributes{});
    matrix_member->SetRowMajor();

    auto* strct = ty.Struct(mod.symbols.New("S"), Vector{matrix_member});

    auto* var = b.Var("s", ty.ptr(storage, strct, read_write));
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* f = b.ComputeFunction("f");
    b.Append(f->Block(), [&] {
        auto* sarr = b.Access<ptr<storage, array<array<mat2x3<f32>, 4>, 5>, read_write>>(var, 0_u);
        auto* x = b.Let("x", b.Load(sarr));

        auto* sarr0 = b.Access<ptr<storage, array<mat2x3<f32>, 4>, read_write>>(sarr, 0_u);
        b.Store(sarr0, b.Access(ty.array<mat2x3<f32>, 4>(), x, 1_u));

        auto* sarr12 = b.Access<ptr<storage, mat2x3<f32>, read_write>>(sarr, 1_u, 2_u);
        b.Store(sarr12, b.Access(ty.mat2x3<f32>(), x, 2_u, 3_u));

        auto* sarr231 = b.Access<ptr<storage, vec3<f32>, read_write>>(sarr, 2_u, 3_u, 1_u);
        b.Store(sarr231, b.Access(ty.vec3<f32>(), x, 4_u, 3_u, 1_u));

        auto* sarr420 = b.Access<ptr<storage, vec3<f32>, read_write>>(sarr, 4_u, 2_u, 0_u);
        b.StoreVectorElement(sarr420, 1_u, b.Access(ty.f32(), x, 1_u, 3_u, 0_u, 2_u));

        b.Return(f);
    });

    auto* before = R"(
S = struct @align(32) {
  m:array<array<mat2x3<f32>, 4>, 5> @offset(0) @size(128), @row_major
}

$B1: {  # root
  %s:ptr<storage, S, read_write> = var undef @binding_point(0, 0)
}

%f = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<storage, array<array<mat2x3<f32>, 4>, 5>, read_write> = access %s, 0u
    %4:array<array<mat2x3<f32>, 4>, 5> = load %3
    %x:array<array<mat2x3<f32>, 4>, 5> = let %4
    %6:ptr<storage, array<mat2x3<f32>, 4>, read_write> = access %3, 0u
    %7:array<mat2x3<f32>, 4> = access %x, 1u
    store %6, %7
    %8:ptr<storage, mat2x3<f32>, read_write> = access %3, 1u, 2u
    %9:mat2x3<f32> = access %x, 2u, 3u
    store %8, %9
    %10:ptr<storage, vec3<f32>, read_write> = access %3, 2u, 3u, 1u
    %11:vec3<f32> = access %x, 4u, 3u, 1u
    store %10, %11
    %12:ptr<storage, vec3<f32>, read_write> = access %3, 4u, 2u, 0u
    %13:f32 = access %x, 1u, 3u, 0u, 2u
    store_vector_element %12, 1u, %13
    ret
  }
}
)";

    ASSERT_EQ(before, str());

    auto* after = R"(
fn tint_transpose_array_1(tint_from : @stride(32) array<mat3x2<f32>, 4u>) -> array<mat2x3<f32>, 4u> {
  var tint_result_1 : array<mat2x3<f32>, 4u>;
  for(var i_1 = 0u; (i_1 < 4u); i_1++) {
    tint_result_1[i_1] = transpose(tint_from[i_1]);
  }
  return tint_result_1;
}

fn tint_transpose_array(tint_from : @stride(128) array<@stride(32) array<mat3x2<f32>, 4u>, 5u>) -> array<array<mat2x3<f32>, 4u>, 5u> {
  var tint_result : array<array<mat2x3<f32>, 4u>, 5u>;
  for(var i = 0u; (i < 5u); i++) {
    tint_result[i] = tint_transpose_array_1(tint_from[i]);
  }
  return tint_result;
}

fn tint_transpose_array_3(tint_from : array<mat2x3<f32>, 4u>) -> @stride(32) array<mat3x2<f32>, 4u> {
  var tint_result_3 : @stride(32) array<mat3x2<f32>, 4u>;
  for(var i_3 = 0u; (i_3 < 4u); i_3++) {
    tint_result_3[i_3] = transpose(tint_from[i_3]);
  }
  return tint_result_3;
}

fn tint_transpose_array_2(tint_from : array<array<mat2x3<f32>, 4u>, 5u>) -> @stride(128) array<@stride(32) array<mat3x2<f32>, 4u>, 5u> {
  var tint_result_2 : @stride(128) array<@stride(32) array<mat3x2<f32>, 4u>, 5u>;
  for(var i_2 = 0u; (i_2 < 5u); i_2++) {
    tint_result_2[i_2] = tint_transpose_array_3(tint_from[i_2]);
  }
  return tint_result_2;
}

fn tint_store_row_major_column(tint_to : ptr<storage, mat3x2<f32>, read_write>, tint_idx : u32, tint_col : vec3<f32>) {
  tint_to[0][tint_idx] = tint_col[0];
  tint_to[1][tint_idx] = tint_col[1];
  tint_to[2][tint_idx] = tint_col[2];
}

struct S {
  /* @offset(0u) */
  @stride(8) @internal(disable_validation__ignore_stride)
  arr : @stride(128) array<@stride(32) array<mat3x2<f32>, 4u>, 5u>,
}

@group(0) @binding(0) var<storage, read_write> s : S;

@compute @workgroup_size(1i)
fn f() {
  let x : array<array<mat2x3<f32>, 4u>, 5u> = tint_transpose_array(s.arr);
  s.arr = tint_transpose_array_2(array<array<mat2x3<f32>, 4u>, 5u>());
  s.arr[0] = tint_transpose_array_3(x[1]);
  s.arr[1][2] = transpose(x[2][3]);
  tint_store_row_major_column(&(s.arr[2][3]), u32(1), x[4][3][1]);
  s.arr[4][2][1][0] = x[1][3][0][2];
}
)";

    Run(TransposeRowMajor);
    EXPECT_EQ(after, str());
}

TEST_F(SpirvReader_TransposeRowMajorTest, DISABLED_ArrayOfMatrix_RuntimeSizedArray) {
    // struct S {
    //   @offset(0)
    //   @row_major
    //   @stride(8)
    //   arr : @stride(128) array<mat4x3<f32>>,
    // };
    // @group(0) @binding(0) var<storage, read_write> s : S;
    //
    // @compute @workgroup_size(1)
    // fn f() {
    //   s.arr[1] = s.arr[0];
    //   s.arr[2][3] = s.arr[1][2];
    //   s.arr[3][2][1] = s.arr[4][3][2];
    // }
    auto* matrix_member =
        ty.Get<core::type::StructMember>(mod.symbols.New("arr"), ty.runtime_array(ty.mat4x3<f32>()),
                                         0u, 0u, 32u, 128u, core::IOAttributes{});
    matrix_member->SetRowMajor();

    auto* strct = ty.Struct(mod.symbols.New("S"), Vector{matrix_member});

    auto* var = b.Var("s", ty.ptr(storage, strct, read_write));
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* f = b.ComputeFunction("f");
    b.Append(f->Block(), [&] {
        auto* sarr1 = b.Access(ty.ptr(storage, ty.mat4x3<f32>(), read_write), var, 0_u, 1_u);
        b.Store(sarr1,
                b.Load(b.Access(ty.ptr(storage, ty.mat4x3<f32>(), read_write), var, 0_u, 0_u)));

        auto* sarr23 = b.Access(ty.ptr(storage, ty.vec3<f32>(), read_write), var, 0_u, 2_u, 3_u);
        b.Store(sarr23,
                b.Load(b.Access(ty.ptr(storage, ty.vec3<f32>(), read_write), var, 0_u, 1_u, 2_u)));

        auto* sarr32 = b.Access(ty.ptr(storage, ty.vec3<f32>(), read_write), var, 0_u, 2_u, 3_u);
        auto* sarr43 = b.Access(ty.ptr(storage, ty.vec3<f32>(), read_write), var, 0_u, 2_u, 3_u);
        b.StoreVectorElement(sarr32, 1_u, b.LoadVectorElement(sarr43, 2_u));

        b.Return(f);
    });

    auto* before = R"(
S = struct @align(32) {
  arr:array<mat4x3<f32>> @offset(0) @size(128), @row_major
}

$B1: {  # root
  %s:ptr<storage, S, read_write> = var undef @binding_point(0, 0)
}

%f = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<storage, mat4x3<f32>, read_write> = access %s, 0u, 1u
    %4:ptr<storage, mat4x3<f32>, read_write> = access %s, 0u, 0u
    %5:mat4x3<f32> = load %4
    store %3, %5
    %6:ptr<storage, vec3<f32>, read_write> = access %s, 0u, 2u, 3u
    %7:ptr<storage, vec3<f32>, read_write> = access %s, 0u, 1u, 2u
    %8:vec3<f32> = load %7
    store %6, %8
    %9:ptr<storage, vec3<f32>, read_write> = access %s, 0u, 2u, 3u
    %10:ptr<storage, vec3<f32>, read_write> = access %s, 0u, 2u, 3u
    %11:f32 = load_vector_element %10, 2u
    store_vector_element %9, 1u, %11
    ret
  }
}
)";

    ASSERT_EQ(before, str());

    auto* after = R"(
fn tint_load_row_major_column(tint_from : ptr<storage, mat3x4<f32>, read_write>, tint_idx : u32) -> vec3<f32> {
  return vec3<f32>(tint_from[0][tint_idx], tint_from[1][tint_idx], tint_from[2][tint_idx]);
}

fn tint_store_row_major_column(tint_to : ptr<storage, mat3x4<f32>, read_write>, tint_idx : u32, tint_col : vec3<f32>) {
  tint_to[0][tint_idx] = tint_col[0];
  tint_to[1][tint_idx] = tint_col[1];
  tint_to[2][tint_idx] = tint_col[2];
}

struct S {
  /* @offset(0u) */
  @stride(8) @internal(disable_validation__ignore_stride)
  arr : @stride(128) array<mat3x4<f32>>,
}

@group(0) @binding(0) var<storage, read_write> s : S;

@compute @workgroup_size(1i)
fn f() {
  s.arr[1] = transpose(transpose(s.arr[0]));
  tint_store_row_major_column(&(s.arr[2]), u32(3), tint_load_row_major_column(&(s.arr[1]), u32(2)));
  s.arr[3][1][2] = s.arr[4][2][3];
}
)";

    Run(TransposeRowMajor);
    EXPECT_EQ(after, str());
}

}  // namespace
}  // namespace tint::spirv::reader::lower
