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

#include "src/tint/lang/spirv/reader/ast_lower/transpose_row_major.h"

#include <utility>

#include "src/tint/lang/wgsl/ast/transform/helper_test.h"
#include "src/tint/lang/wgsl/ast/transform/simplify_pointers.h"
#include "src/tint/lang/wgsl/program/clone_context.h"
#include "src/tint/lang/wgsl/program/program_builder.h"
#include "src/tint/lang/wgsl/resolver/resolve.h"

namespace tint::spirv::reader {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

using TransposeRowMajorTest = ast::transform::TransformTest;
using SimplifyPointers = ast::transform::SimplifyPointers;

TEST_F(TransposeRowMajorTest, ShouldRunEmptyModule) {
    auto* src = R"()";

    EXPECT_FALSE(ShouldRun<TransposeRowMajor>(src));
}

TEST_F(TransposeRowMajorTest, ShouldRunColumnMajorMatrix) {
    auto* src = R"(
struct S {
  m : mat3x2<f32>
}

@group(0) @binding(0)
var<uniform> s : S;
)";

    EXPECT_FALSE(ShouldRun<TransposeRowMajor>(src));
}

TEST_F(TransposeRowMajorTest, ReadUniformMatrix) {
    // struct S {
    //   @offset(16)
    //   @row_major
    //   m : mat2x3<f32>,
    // };
    // @group(0) @binding(0) var<uniform> s : S;
    //
    // @compute @workgroup_size(1)
    // fn f() {
    //   let x : mat2x3<f32> = s.m;
    // }
    ProgramBuilder b;
    auto* S = b.Structure("S", Vector{
                                   b.Member("m", b.ty.mat2x3<f32>(),
                                            Vector{
                                                b.MemberOffset(16_u),
                                                b.RowMajor(),
                                            }),
                               });
    b.GlobalVar("s", b.ty.Of(S), core::AddressSpace::kUniform, b.Group(0_a), b.Binding(0_a));
    b.Func("f", tint::Empty, b.ty.void_(),
           Vector{
               b.Decl(b.Let("x", b.ty.mat2x3<f32>(), b.MemberAccessor("s", "m"))),
           },
           Vector{
               b.Stage(ast::PipelineStage::kCompute),
               b.WorkgroupSize(1_i),
           });

    auto* expect = R"(
struct S {
  @size(16)
  padding_0 : u32,
  /* @offset(16u) */
  m : mat3x2<f32>,
}

@group(0) @binding(0) var<uniform> s : S;

@compute @workgroup_size(1i)
fn f() {
  let x : mat2x3<f32> = transpose(s.m);
}
)";

    auto got = Run<SimplifyPointers, TransposeRowMajor>(resolver::Resolve(b));

    EXPECT_EQ(expect, str(got));
}

TEST_F(TransposeRowMajorTest, ReadUniformColumn) {
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
    ProgramBuilder b;
    auto* S = b.Structure("S", Vector{
                                   b.Member("m", b.ty.mat2x3<f32>(),
                                            Vector{
                                                b.MemberOffset(16_u),
                                                b.RowMajor(),
                                            }),
                               });
    b.GlobalVar("s", b.ty.Of(S), core::AddressSpace::kUniform, b.Group(0_a), b.Binding(0_a));
    b.Func(
        "f", tint::Empty, b.ty.void_(),
        Vector{
            b.Decl(b.Let("x", b.ty.vec3<f32>(), b.IndexAccessor(b.MemberAccessor("s", "m"), 1_i))),
        },
        Vector{
            b.Stage(ast::PipelineStage::kCompute),
            b.WorkgroupSize(1_i),
        });

    auto* expect = R"(
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

    auto got = Run<SimplifyPointers, TransposeRowMajor>(resolver::Resolve(b));

    EXPECT_EQ(expect, str(got));
}

TEST_F(TransposeRowMajorTest, ReadUniformElement_MemberAccessor) {
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
    ProgramBuilder b;
    auto* S = b.Structure("S", Vector{
                                   b.Member("m", b.ty.mat2x3<f32>(),
                                            Vector{
                                                b.MemberOffset(16_u),
                                                b.RowMajor(),
                                            }),
                               });
    b.GlobalVar("s", b.ty.Of(S), core::AddressSpace::kUniform, b.Group(0_a), b.Binding(0_a));
    b.Func("f", tint::Empty, b.ty.void_(),
           Vector{
               b.Decl(b.Let("col_idx", b.ty.i32(), b.Expr(1_i))),
               b.Decl(b.Let(
                   "x", b.ty.f32(),
                   b.MemberAccessor(b.IndexAccessor(b.MemberAccessor("s", "m"), "col_idx"), "z"))),
           },
           Vector{
               b.Stage(ast::PipelineStage::kCompute),
               b.WorkgroupSize(1_i),
           });

    auto* expect = R"(
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

    auto got = Run<SimplifyPointers, TransposeRowMajor>(resolver::Resolve(b));

    EXPECT_EQ(expect, str(got));
}

TEST_F(TransposeRowMajorTest, ReadUniformElement_IndexAccessor) {
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
    //   let x : f32 = s.m[col_idx][row_idx];
    // }
    ProgramBuilder b;
    auto* S = b.Structure("S", Vector{
                                   b.Member("m", b.ty.mat2x3<f32>(),
                                            Vector{
                                                b.MemberOffset(16_u),
                                                b.RowMajor(),
                                            }),
                               });
    b.GlobalVar("s", b.ty.Of(S), core::AddressSpace::kUniform, b.Group(0_a), b.Binding(0_a));
    b.Func("f", tint::Empty, b.ty.void_(),
           Vector{
               b.Decl(b.Let("col_idx", b.ty.i32(), b.Expr(1_i))),
               b.Decl(b.Let("row_idx", b.ty.i32(), b.Expr(2_i))),
               b.Decl(b.Let("x", b.ty.f32(),
                            b.IndexAccessor(b.IndexAccessor(b.MemberAccessor("s", "m"), "col_idx"),
                                            "row_idx"))),
           },
           Vector{
               b.Stage(ast::PipelineStage::kCompute),
               b.WorkgroupSize(1_i),
           });

    auto* expect = R"(
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
  let x : f32 = s.m[row_idx][col_idx];
}
)";

    auto got = Run<SimplifyPointers, TransposeRowMajor>(resolver::Resolve(b));

    EXPECT_EQ(expect, str(got));
}

TEST_F(TransposeRowMajorTest, ReadUniformSwizzle) {
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
    ProgramBuilder b;
    auto* S = b.Structure("S", Vector{
                                   b.Member("m", b.ty.mat2x3<f32>(),
                                            Vector{
                                                b.MemberOffset(16_u),
                                                b.RowMajor(),
                                            }),
                               });
    b.GlobalVar("s", b.ty.Of(S), core::AddressSpace::kUniform, b.Group(0_a), b.Binding(0_a));
    b.Func("f", tint::Empty, b.ty.void_(),
           Vector{
               b.Decl(b.Let("col_idx", b.ty.i32(), b.Expr(1_i))),
               b.Decl(b.Let(
                   "x", b.ty.vec2<f32>(),
                   b.MemberAccessor(b.IndexAccessor(b.MemberAccessor("s", "m"), "col_idx"), "zx"))),
           },
           Vector{
               b.Stage(ast::PipelineStage::kCompute),
               b.WorkgroupSize(1_i),
           });

    auto* expect = R"(
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

    auto got = Run<SimplifyPointers, TransposeRowMajor>(resolver::Resolve(b));

    EXPECT_EQ(expect, str(got));
}

TEST_F(TransposeRowMajorTest, WriteStorageMatrix) {
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
    ProgramBuilder b;
    auto* S = b.Structure("S", Vector{
                                   b.Member("m", b.ty.mat2x3<f32>(),
                                            Vector{
                                                b.MemberOffset(8_u),
                                                b.RowMajor(),
                                            }),
                               });
    b.GlobalVar("s", b.ty.Of(S), core::AddressSpace::kStorage, core::Access::kReadWrite,
                b.Group(0_a), b.Binding(0_a));
    b.Func(
        "f", tint::Empty, b.ty.void_(),
        Vector{
            b.Assign(b.MemberAccessor("s", "m"), b.Call<mat2x3<f32>>(1_f, 2_f, 3_f, 4_f, 5_f, 6_f)),
        },
        Vector{
            b.Stage(ast::PipelineStage::kCompute),
            b.WorkgroupSize(1_i),
        });

    auto* expect = R"(
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

    auto got = Run<SimplifyPointers, TransposeRowMajor>(resolver::Resolve(b));

    EXPECT_EQ(expect, str(got));
}

TEST_F(TransposeRowMajorTest, WriteStorageColumn) {
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
    ProgramBuilder b;
    auto* S = b.Structure("S", Vector{
                                   b.Member("m", b.ty.mat2x3<f32>(),
                                            Vector{
                                                b.MemberOffset(8_u),
                                                b.RowMajor(),
                                            }),
                               });
    b.GlobalVar("s", b.ty.Of(S), core::AddressSpace::kStorage, core::Access::kReadWrite,
                b.Group(0_a), b.Binding(0_a));
    b.Func("f", tint::Empty, b.ty.void_(),
           Vector{
               b.Decl(b.Let("col_idx", b.ty.i32(), b.Expr(1_i))),
               b.Assign(b.IndexAccessor(b.MemberAccessor("s", "m"), "col_idx"),
                        b.Call<vec3<f32>>(1_f, 2_f, 3_f)),
           },
           Vector{
               b.Stage(ast::PipelineStage::kCompute),
               b.WorkgroupSize(1_i),
           });

    auto* expect = R"(
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

    auto got = Run<SimplifyPointers, TransposeRowMajor>(resolver::Resolve(b));

    EXPECT_EQ(expect, str(got));
}

TEST_F(TransposeRowMajorTest, WriteStorageElement_MemberAccessor) {
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
    ProgramBuilder b;
    auto* S = b.Structure("S", Vector{
                                   b.Member("m", b.ty.mat2x3<f32>(),
                                            Vector{
                                                b.MemberOffset(8_u),
                                                b.RowMajor(),
                                            }),
                               });
    b.GlobalVar("s", b.ty.Of(S), core::AddressSpace::kStorage, core::Access::kReadWrite,
                b.Group(0_a), b.Binding(0_a));
    b.Func(
        "f", tint::Empty, b.ty.void_(),
        Vector{
            b.Decl(b.Let("col_idx", b.ty.i32(), b.Expr(1_i))),
            b.Assign(b.MemberAccessor(b.IndexAccessor(b.MemberAccessor("s", "m"), "col_idx"), "z"),
                     1_f),
        },
        Vector{
            b.Stage(ast::PipelineStage::kCompute),
            b.WorkgroupSize(1_i),
        });

    auto* expect = R"(
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

    auto got = Run<SimplifyPointers, TransposeRowMajor>(resolver::Resolve(b));

    EXPECT_EQ(expect, str(got));
}

TEST_F(TransposeRowMajorTest, WriteStorageElement_IndexAccessor) {
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
    //   let row_idx : i32 = 2i;
    //   s.m[1][idx] = 1.0;
    // }
    ProgramBuilder b;
    auto* S = b.Structure("S", Vector{
                                   b.Member("m", b.ty.mat2x3<f32>(),
                                            Vector{
                                                b.MemberOffset(8_u),
                                                b.RowMajor(),
                                            }),
                               });
    b.GlobalVar("s", b.ty.Of(S), core::AddressSpace::kStorage, core::Access::kReadWrite,
                b.Group(0_a), b.Binding(0_a));
    b.Func("f", tint::Empty, b.ty.void_(),
           Vector{
               b.Decl(b.Let("col_idx", b.ty.i32(), b.Expr(1_i))),
               b.Decl(b.Let("row_idx", b.ty.i32(), b.Expr(2_i))),
               b.Assign(b.IndexAccessor(b.IndexAccessor(b.MemberAccessor("s", "m"), "col_idx"),
                                        "row_idx"),
                        1_f),
           },
           Vector{
               b.Stage(ast::PipelineStage::kCompute),
               b.WorkgroupSize(1_i),
           });

    auto* expect = R"(
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
  let row_idx : i32 = 2i;
  s.m[row_idx][col_idx] = 1.0f;
}
)";

    auto got = Run<SimplifyPointers, TransposeRowMajor>(resolver::Resolve(b));

    EXPECT_EQ(expect, str(got));
}

TEST_F(TransposeRowMajorTest, ExtractFromLoadedStruct) {
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
    ProgramBuilder b;
    auto* S = b.Structure("S", Vector{
                                   b.Member("m", b.ty.mat2x3<f32>(),
                                            Vector{
                                                b.MemberOffset(16_u),
                                                b.RowMajor(),
                                            }),
                               });
    b.GlobalVar("s", b.ty.Of(S), core::AddressSpace::kUniform, b.Group(0_a), b.Binding(0_a));
    b.Func(
        "f", tint::Empty, b.ty.void_(),
        Vector{
            b.Decl(b.Let("col_idx", b.ty.i32(), b.Expr(1_i))),
            b.Decl(b.Let("row_idx", b.ty.i32(), b.Expr(2_i))),
            b.Decl(b.Let("load", b.ty.Of(S), b.Expr("s"))),
            b.Decl(b.Let("m", b.ty.mat2x3<f32>(), b.MemberAccessor("load", "m"))),
            b.Decl(b.Let("c", b.ty.vec3<f32>(),
                         b.IndexAccessor(b.MemberAccessor("load", "m"), "col_idx"))),
            b.Decl(b.Let("e", b.ty.f32(),
                         b.IndexAccessor(b.IndexAccessor(b.MemberAccessor("load", "m"), "col_idx"),
                                         "row_idx"))),
        },
        Vector{
            b.Stage(ast::PipelineStage::kCompute),
            b.WorkgroupSize(1_i),
        });

    auto* expect = R"(
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

    auto got = Run<SimplifyPointers, TransposeRowMajor>(resolver::Resolve(b));

    EXPECT_EQ(expect, str(got));
}

TEST_F(TransposeRowMajorTest, InsertInStructConstructor) {
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
    ProgramBuilder b;
    auto* S = b.Structure("S", Vector{
                                   b.Member("m", b.ty.mat2x3<f32>(),
                                            Vector{
                                                b.MemberOffset(0_u),
                                                b.RowMajor(),
                                            }),
                                   b.Member("m1", b.ty.mat4x2<f32>(),
                                            Vector{
                                                b.MemberOffset(32_u),
                                            }),
                                   b.Member("m2", b.ty.mat4x2<f32>(),
                                            Vector{
                                                b.MemberOffset(64_u),
                                                b.RowMajor(),
                                            }),
                               });
    b.GlobalVar("s", b.ty.Of(S), core::AddressSpace::kStorage, core::Access::kReadWrite,
                b.Group(0_a), b.Binding(0_a));
    b.Func("f", tint::Empty, b.ty.void_(),
           Vector{
               b.Decl(b.Let("m1", b.Call("mat2x3f"))),
               b.Decl(b.Let("m2", b.Call("mat4x2f"))),
               b.Assign("s", b.Call("S", "m1", "m2", "m2")),
           },
           Vector{
               b.Stage(ast::PipelineStage::kCompute),
               b.WorkgroupSize(1_i),
           });

    auto* expect = R"(
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

    auto got = Run<SimplifyPointers, TransposeRowMajor>(resolver::Resolve(b));

    EXPECT_EQ(expect, str(got));
}

TEST_F(TransposeRowMajorTest, DeeplyNested) {
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
    ProgramBuilder b;
    auto* inner = b.Structure("Inner", Vector{
                                           b.Member("m", b.ty.mat4x3<f32>(),
                                                    Vector{
                                                        b.MemberOffset(0_u),
                                                        b.RowMajor(),
                                                    }),
                                       });
    auto* outer = b.Structure("Outer", Vector{
                                           b.Member("arr", b.ty.array(b.ty.Of(inner), 4_a),
                                                    Vector{
                                                        b.MemberOffset(0_u),
                                                    }),
                                       });
    b.GlobalVar("buffer", b.ty.Of(outer), core::AddressSpace::kStorage, core::Access::kReadWrite,
                b.Group(0_a), b.Binding(0_a));
    b.Func("f", tint::Empty, b.ty.void_(),
           Vector{
               b.Decl(b.Let(
                   "m", b.ty.mat4x3<f32>(),
                   b.MemberAccessor(b.IndexAccessor(b.MemberAccessor("buffer", "arr"), 1_a), "m"))),
               b.Assign(b.IndexAccessor(
                            b.MemberAccessor(
                                b.IndexAccessor(b.MemberAccessor("buffer", "arr"), 0_a), "m"),
                            3_a),
                        b.IndexAccessor("m", 2_a)),
           },
           Vector{
               b.Stage(ast::PipelineStage::kCompute),
               b.WorkgroupSize(1_i),
           });

    auto* expect = R"(
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

    auto got = Run<SimplifyPointers, TransposeRowMajor>(resolver::Resolve(b));

    EXPECT_EQ(expect, str(got));
}

TEST_F(TransposeRowMajorTest, MultipleColumnHelpers) {
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
    //   ps.m2[2] = s.m1[3];
    //   ps.m2[3] = s.m1[2];
    //
    //   s.m1[0] = ps.m1[0];
    //   s.m1[1] = ps.m1[1];
    //   s.m2[2] = ps.m2[2];
    //   s.m2[3] = ps.m2[3];
    // }
    ProgramBuilder b;
    auto* S = b.Structure("S", Vector{
                                   b.Member("m1", b.ty.mat2x3<f32>(),
                                            Vector{
                                                b.MemberOffset(0_u),
                                                b.RowMajor(),
                                            }),
                                   b.Member("m2", b.ty.mat4x2<f32>(),
                                            Vector{
                                                b.MemberOffset(32_u),
                                                b.RowMajor(),
                                            }),
                               });
    b.GlobalVar("s", b.ty.Of(S), core::AddressSpace::kStorage, core::Access::kReadWrite,
                b.Group(0_a), b.Binding(0_a));
    b.GlobalVar("ps", b.ty.Of(S), core::AddressSpace::kPrivate);
    b.Func("f", tint::Empty, b.ty.void_(),
           Vector{
               b.Assign(b.IndexAccessor(b.MemberAccessor("ps", "m1"), 0_u),
                        b.IndexAccessor(b.MemberAccessor("s", "m1"), 1_u)),
               b.Assign(b.IndexAccessor(b.MemberAccessor("ps", "m1"), 1_u),
                        b.IndexAccessor(b.MemberAccessor("s", "m1"), 0_u)),
               b.Assign(b.IndexAccessor(b.MemberAccessor("ps", "m2"), 2_u),
                        b.IndexAccessor(b.MemberAccessor("s", "m2"), 3_u)),
               b.Assign(b.IndexAccessor(b.MemberAccessor("ps", "m2"), 3_u),
                        b.IndexAccessor(b.MemberAccessor("s", "m2"), 2_u)),

               b.Assign(b.IndexAccessor(b.MemberAccessor("s", "m1"), 0_u),
                        b.IndexAccessor(b.MemberAccessor("ps", "m1"), 0_u)),
               b.Assign(b.IndexAccessor(b.MemberAccessor("s", "m1"), 1_u),
                        b.IndexAccessor(b.MemberAccessor("ps", "m1"), 1_u)),
               b.Assign(b.IndexAccessor(b.MemberAccessor("s", "m2"), 2_u),
                        b.IndexAccessor(b.MemberAccessor("ps", "m2"), 2_u)),
               b.Assign(b.IndexAccessor(b.MemberAccessor("s", "m2"), 3_u),
                        b.IndexAccessor(b.MemberAccessor("ps", "m2"), 3_u)),
           },
           Vector{
               b.Stage(ast::PipelineStage::kCompute),
               b.WorkgroupSize(1_i),
           });

    auto* expect = R"(
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

    auto got = Run<SimplifyPointers, TransposeRowMajor>(resolver::Resolve(b));

    EXPECT_EQ(expect, str(got));
}

TEST_F(TransposeRowMajorTest, PreserveMatrixStride) {
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
    ProgramBuilder b;
    auto* S = b.Structure(
        "S", Vector{
                 b.Member("m", b.ty.mat2x3<f32>(),
                          Vector{
                              b.MemberOffset(0_u),
                              b.Stride(32_u),
                              b.Disable(ast::DisabledValidation::kIgnoreStrideAttribute),
                              b.RowMajor(),
                          }),
             });
    b.GlobalVar("s", b.ty.Of(S), core::AddressSpace::kUniform, b.Group(0_a), b.Binding(0_a));
    b.Func("f", tint::Empty, b.ty.void_(),
           Vector{
               b.Decl(b.Let("x", b.ty.mat2x3<f32>(), b.MemberAccessor("s", "m"))),
           },
           Vector{
               b.Stage(ast::PipelineStage::kCompute),
               b.WorkgroupSize(1_i),
           });

    auto* expect = R"(
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

    auto got = Run<SimplifyPointers, TransposeRowMajor>(resolver::Resolve(b));

    EXPECT_EQ(expect, str(got));
}

}  // namespace
}  // namespace tint::spirv::reader
