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

#include "src/tint/lang/wgsl/ast/transform/decompose_strided_matrix.h"

#include <memory>
#include <utility>
#include <vector>

#include "src/tint/lang/wgsl/ast/disable_validation_attribute.h"
#include "src/tint/lang/wgsl/ast/transform/helper_test.h"
#include "src/tint/lang/wgsl/ast/transform/simplify_pointers.h"
#include "src/tint/lang/wgsl/ast/transform/unshadow.h"
#include "src/tint/lang/wgsl/program/clone_context.h"
#include "src/tint/lang/wgsl/program/program_builder.h"
#include "src/tint/lang/wgsl/resolver/resolve.h"

namespace tint::ast::transform {
namespace {

using namespace tint::core::fluent_types;  // NOLINT
using namespace tint::number_suffixes;     // NOLINT

using DecomposeStridedMatrixTest = TransformTest;

TEST_F(DecomposeStridedMatrixTest, ShouldRunEmptyModule) {
    auto* src = R"()";

    EXPECT_FALSE(ShouldRun<DecomposeStridedMatrix>(src));
}

TEST_F(DecomposeStridedMatrixTest, ShouldRunNonStridedMatrox) {
    auto* src = R"(
var<private> m : mat3x2<f32>;
)";

    EXPECT_FALSE(ShouldRun<DecomposeStridedMatrix>(src));
}

TEST_F(DecomposeStridedMatrixTest, Empty) {
    auto* src = R"()";
    auto* expect = src;

    auto got = Run<DecomposeStridedMatrix>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DecomposeStridedMatrixTest, ReadUniformMatrix) {
    // struct S {
    //   @offset(16) @stride(32)
    //   @internal(ignore_stride_attribute)
    //   m : mat2x2<f32>,
    // };
    // @group(0) @binding(0) var<uniform> s : S;
    //
    // @compute @workgroup_size(1)
    // fn f() {
    //   let x : mat2x2<f32> = s.m;
    // }
    ProgramBuilder b;
    auto* S =
        b.Structure("S", tint::Vector{
                             b.Member("m", b.ty.mat2x2<f32>(),
                                      tint::Vector{
                                          b.MemberOffset(16_u),
                                          b.create<StrideAttribute>(32u),
                                          b.Disable(DisabledValidation::kIgnoreStrideAttribute),
                                      }),
                         });
    b.GlobalVar("s", b.ty.Of(S), core::AddressSpace::kUniform, b.Group(0_a), b.Binding(0_a));
    b.Func("f", tint::Empty, b.ty.void_(),
           tint::Vector{
               b.Decl(b.Let("x", b.ty.mat2x2<f32>(), b.MemberAccessor("s", "m"))),
           },
           tint::Vector{
               b.Stage(PipelineStage::kCompute),
               b.WorkgroupSize(1_i),
           });

    auto* expect = R"(
struct S {
  @size(16)
  padding_0 : u32,
  /* @offset(16) */
  m : @stride(32) array<vec2<f32>, 2u>,
}

@group(0) @binding(0) var<uniform> s : S;

fn arr_to_mat2x2_stride_32(arr : @stride(32) array<vec2<f32>, 2u>) -> mat2x2<f32> {
  return mat2x2<f32>(arr[0u], arr[1u]);
}

@compute @workgroup_size(1i)
fn f() {
  let x : mat2x2<f32> = arr_to_mat2x2_stride_32(s.m);
}
)";

    auto got = Run<Unshadow, SimplifyPointers, DecomposeStridedMatrix>(resolver::Resolve(b));

    EXPECT_EQ(expect, str(got));
}

TEST_F(DecomposeStridedMatrixTest, ReadUniformColumn) {
    // struct S {
    //   @offset(16) @stride(32)
    //   @internal(ignore_stride_attribute)
    //   m : mat2x2<f32>,
    // };
    // @group(0) @binding(0) var<uniform> s : S;
    //
    // @compute @workgroup_size(1)
    // fn f() {
    //   let x : vec2<f32> = s.m[1];
    // }
    ProgramBuilder b;
    auto* S =
        b.Structure("S", tint::Vector{
                             b.Member("m", b.ty.mat2x2<f32>(),
                                      tint::Vector{
                                          b.MemberOffset(16_u),
                                          b.create<StrideAttribute>(32u),
                                          b.Disable(DisabledValidation::kIgnoreStrideAttribute),
                                      }),
                         });
    b.GlobalVar("s", b.ty.Of(S), core::AddressSpace::kUniform, b.Group(0_a), b.Binding(0_a));
    b.Func(
        "f", tint::Empty, b.ty.void_(),
        tint::Vector{
            b.Decl(b.Let("x", b.ty.vec2<f32>(), b.IndexAccessor(b.MemberAccessor("s", "m"), 1_i))),
        },
        tint::Vector{
            b.Stage(PipelineStage::kCompute),
            b.WorkgroupSize(1_i),
        });

    auto* expect = R"(
struct S {
  @size(16)
  padding_0 : u32,
  /* @offset(16) */
  m : @stride(32) array<vec2<f32>, 2u>,
}

@group(0) @binding(0) var<uniform> s : S;

@compute @workgroup_size(1i)
fn f() {
  let x : vec2<f32> = s.m[1i];
}
)";

    auto got = Run<Unshadow, SimplifyPointers, DecomposeStridedMatrix>(resolver::Resolve(b));

    EXPECT_EQ(expect, str(got));
}

TEST_F(DecomposeStridedMatrixTest, ReadUniformMatrix_DefaultStride) {
    // struct S {
    //   @offset(16) @stride(8)
    //   @internal(ignore_stride_attribute)
    //   m : mat2x2<f32>,
    // };
    // @group(0) @binding(0) var<uniform> s : S;
    //
    // @compute @workgroup_size(1)
    // fn f() {
    //   let x : mat2x2<f32> = s.m;
    // }
    ProgramBuilder b;
    auto* S =
        b.Structure("S", tint::Vector{
                             b.Member("m", b.ty.mat2x2<f32>(),
                                      tint::Vector{
                                          b.MemberOffset(16_u),
                                          b.create<StrideAttribute>(8u),
                                          b.Disable(DisabledValidation::kIgnoreStrideAttribute),
                                      }),
                         });
    b.GlobalVar("s", b.ty.Of(S), core::AddressSpace::kUniform, b.Group(0_a), b.Binding(0_a));
    b.Func("f", tint::Empty, b.ty.void_(),
           tint::Vector{
               b.Decl(b.Let("x", b.ty.mat2x2<f32>(), b.MemberAccessor("s", "m"))),
           },
           tint::Vector{
               b.Stage(PipelineStage::kCompute),
               b.WorkgroupSize(1_i),
           });

    auto* expect = R"(
struct S {
  @size(16)
  padding_0 : u32,
  /* @offset(16u) */
  @stride(8) @internal(disable_validation__ignore_stride)
  m : mat2x2<f32>,
}

@group(0) @binding(0) var<uniform> s : S;

@compute @workgroup_size(1i)
fn f() {
  let x : mat2x2<f32> = s.m;
}
)";

    auto got = Run<Unshadow, SimplifyPointers, DecomposeStridedMatrix>(resolver::Resolve(b));

    EXPECT_EQ(expect, str(got));
}

TEST_F(DecomposeStridedMatrixTest, ReadStorageMatrix) {
    // struct S {
    //   @offset(8) @stride(32)
    //   @internal(ignore_stride_attribute)
    //   m : mat2x2<f32>,
    // };
    // @group(0) @binding(0) var<storage, read_write> s : S;
    //
    // @compute @workgroup_size(1)
    // fn f() {
    //   let x : mat2x2<f32> = s.m;
    // }
    ProgramBuilder b;
    auto* S =
        b.Structure("S", tint::Vector{
                             b.Member("m", b.ty.mat2x2<f32>(),
                                      tint::Vector{
                                          b.MemberOffset(8_u),
                                          b.create<StrideAttribute>(32u),
                                          b.Disable(DisabledValidation::kIgnoreStrideAttribute),
                                      }),
                         });
    b.GlobalVar("s", b.ty.Of(S), core::AddressSpace::kStorage, core::Access::kReadWrite,
                b.Group(0_a), b.Binding(0_a));
    b.Func("f", tint::Empty, b.ty.void_(),
           tint::Vector{
               b.Decl(b.Let("x", b.ty.mat2x2<f32>(), b.MemberAccessor("s", "m"))),
           },
           tint::Vector{
               b.Stage(PipelineStage::kCompute),
               b.WorkgroupSize(1_i),
           });

    auto* expect = R"(
struct S {
  @size(8)
  padding_0 : u32,
  /* @offset(8) */
  m : @stride(32) array<vec2<f32>, 2u>,
}

@group(0) @binding(0) var<storage, read_write> s : S;

fn arr_to_mat2x2_stride_32(arr : @stride(32) array<vec2<f32>, 2u>) -> mat2x2<f32> {
  return mat2x2<f32>(arr[0u], arr[1u]);
}

@compute @workgroup_size(1i)
fn f() {
  let x : mat2x2<f32> = arr_to_mat2x2_stride_32(s.m);
}
)";

    auto got = Run<Unshadow, SimplifyPointers, DecomposeStridedMatrix>(resolver::Resolve(b));

    EXPECT_EQ(expect, str(got));
}

TEST_F(DecomposeStridedMatrixTest, ReadStorageColumn) {
    // struct S {
    //   @offset(16) @stride(32)
    //   @internal(ignore_stride_attribute)
    //   m : mat2x2<f32>,
    // };
    // @group(0) @binding(0) var<storage, read_write> s : S;
    //
    // @compute @workgroup_size(1)
    // fn f() {
    //   let x : vec2<f32> = s.m[1];
    // }
    ProgramBuilder b;
    auto* S =
        b.Structure("S", tint::Vector{
                             b.Member("m", b.ty.mat2x2<f32>(),
                                      tint::Vector{
                                          b.MemberOffset(16_u),
                                          b.create<StrideAttribute>(32u),
                                          b.Disable(DisabledValidation::kIgnoreStrideAttribute),
                                      }),
                         });
    b.GlobalVar("s", b.ty.Of(S), core::AddressSpace::kStorage, core::Access::kReadWrite,
                b.Group(0_a), b.Binding(0_a));
    b.Func(
        "f", tint::Empty, b.ty.void_(),
        tint::Vector{
            b.Decl(b.Let("x", b.ty.vec2<f32>(), b.IndexAccessor(b.MemberAccessor("s", "m"), 1_i))),
        },
        tint::Vector{
            b.Stage(PipelineStage::kCompute),
            b.WorkgroupSize(1_i),
        });

    auto* expect = R"(
struct S {
  @size(16)
  padding_0 : u32,
  /* @offset(16) */
  m : @stride(32) array<vec2<f32>, 2u>,
}

@group(0) @binding(0) var<storage, read_write> s : S;

@compute @workgroup_size(1i)
fn f() {
  let x : vec2<f32> = s.m[1i];
}
)";

    auto got = Run<Unshadow, SimplifyPointers, DecomposeStridedMatrix>(resolver::Resolve(b));

    EXPECT_EQ(expect, str(got));
}

TEST_F(DecomposeStridedMatrixTest, WriteStorageMatrix) {
    // struct S {
    //   @offset(8) @stride(32)
    //   @internal(ignore_stride_attribute)
    //   m : mat2x2<f32>,
    // };
    // @group(0) @binding(0) var<storage, read_write> s : S;
    //
    // @compute @workgroup_size(1)
    // fn f() {
    //   s.m = mat2x2<f32>(vec2<f32>(1.0, 2.0), vec2<f32>(3.0, 4.0));
    // }
    ProgramBuilder b;
    auto* S =
        b.Structure("S", tint::Vector{
                             b.Member("m", b.ty.mat2x2<f32>(),
                                      tint::Vector{
                                          b.MemberOffset(8_u),
                                          b.create<StrideAttribute>(32u),
                                          b.Disable(DisabledValidation::kIgnoreStrideAttribute),
                                      }),
                         });
    b.GlobalVar("s", b.ty.Of(S), core::AddressSpace::kStorage, core::Access::kReadWrite,
                b.Group(0_a), b.Binding(0_a));
    b.Func(
        "f", tint::Empty, b.ty.void_(),
        tint::Vector{
            b.Assign(b.MemberAccessor("s", "m"),
                     b.Call<mat2x2<f32>>(b.Call<vec2<f32>>(1_f, 2_f), b.Call<vec2<f32>>(3_f, 4_f))),
        },
        tint::Vector{
            b.Stage(PipelineStage::kCompute),
            b.WorkgroupSize(1_i),
        });

    auto* expect = R"(
struct S {
  @size(8)
  padding_0 : u32,
  /* @offset(8) */
  m : @stride(32) array<vec2<f32>, 2u>,
}

@group(0) @binding(0) var<storage, read_write> s : S;

fn mat2x2_stride_32_to_arr(m : mat2x2<f32>) -> @stride(32) array<vec2<f32>, 2u> {
  return @stride(32) array<vec2<f32>, 2u>(m[0u], m[1u]);
}

@compute @workgroup_size(1i)
fn f() {
  s.m = mat2x2_stride_32_to_arr(mat2x2<f32>(vec2<f32>(1.0f, 2.0f), vec2<f32>(3.0f, 4.0f)));
}
)";

    auto got = Run<Unshadow, SimplifyPointers, DecomposeStridedMatrix>(resolver::Resolve(b));

    EXPECT_EQ(expect, str(got));
}

TEST_F(DecomposeStridedMatrixTest, WriteStorageColumn) {
    // struct S {
    //   @offset(8) @stride(32)
    //   @internal(ignore_stride_attribute)
    //   m : mat2x2<f32>,
    // };
    // @group(0) @binding(0) var<storage, read_write> s : S;
    //
    // @compute @workgroup_size(1)
    // fn f() {
    //   s.m[1] = vec2<f32>(1.0, 2.0);
    // }
    ProgramBuilder b;
    auto* S =
        b.Structure("S", tint::Vector{
                             b.Member("m", b.ty.mat2x2<f32>(),
                                      tint::Vector{
                                          b.MemberOffset(8_u),
                                          b.create<StrideAttribute>(32u),
                                          b.Disable(DisabledValidation::kIgnoreStrideAttribute),
                                      }),
                         });
    b.GlobalVar("s", b.ty.Of(S), core::AddressSpace::kStorage, core::Access::kReadWrite,
                b.Group(0_a), b.Binding(0_a));
    b.Func(
        "f", tint::Empty, b.ty.void_(),
        tint::Vector{
            b.Assign(b.IndexAccessor(b.MemberAccessor("s", "m"), 1_i), b.Call<vec2<f32>>(1_f, 2_f)),
        },
        tint::Vector{
            b.Stage(PipelineStage::kCompute),
            b.WorkgroupSize(1_i),
        });

    auto* expect = R"(
struct S {
  @size(8)
  padding_0 : u32,
  /* @offset(8) */
  m : @stride(32) array<vec2<f32>, 2u>,
}

@group(0) @binding(0) var<storage, read_write> s : S;

@compute @workgroup_size(1i)
fn f() {
  s.m[1i] = vec2<f32>(1.0f, 2.0f);
}
)";

    auto got = Run<Unshadow, SimplifyPointers, DecomposeStridedMatrix>(resolver::Resolve(b));

    EXPECT_EQ(expect, str(got));
}

TEST_F(DecomposeStridedMatrixTest, ReadWriteViaPointerLets) {
    // struct S {
    //   @offset(8) @stride(32)
    //   @internal(ignore_stride_attribute)
    //   m : mat2x2<f32>,
    // };
    // @group(0) @binding(0) var<storage, read_write> s : S;
    //
    // @compute @workgroup_size(1)
    // fn f() {
    //   let a = &s.m;
    //   let b = &*&*(a);
    //   let x = *b;
    //   let y = (*b)[1];
    //   let z = x[1];
    //   (*b) = mat2x2<f32>(vec2<f32>(1.0, 2.0), vec2<f32>(3.0, 4.0));
    //   (*b)[1] = vec2<f32>(5.0, 6.0);
    // }
    ProgramBuilder b;
    auto* S =
        b.Structure("S", tint::Vector{
                             b.Member("m", b.ty.mat2x2<f32>(),
                                      tint::Vector{
                                          b.MemberOffset(8_u),
                                          b.create<StrideAttribute>(32u),
                                          b.Disable(DisabledValidation::kIgnoreStrideAttribute),
                                      }),
                         });
    b.GlobalVar("s", b.ty.Of(S), core::AddressSpace::kStorage, core::Access::kReadWrite,
                b.Group(0_a), b.Binding(0_a));
    b.Func("f", tint::Empty, b.ty.void_(),
           tint::Vector{
               b.Decl(b.Let("a", b.AddressOf(b.MemberAccessor("s", "m")))),
               b.Decl(b.Let("b", b.AddressOf(b.Deref(b.AddressOf(b.Deref("a")))))),
               b.Decl(b.Let("x", b.Deref("b"))),
               b.Decl(b.Let("y", b.IndexAccessor(b.Deref("b"), 1_i))),
               b.Decl(b.Let("z", b.IndexAccessor("x", 1_i))),
               b.Assign(b.Deref("b"), b.Call<mat2x2<f32>>(b.Call<vec2<f32>>(1_f, 2_f),
                                                          b.Call<vec2<f32>>(3_f, 4_f))),
               b.Assign(b.IndexAccessor(b.Deref("b"), 1_i), b.Call<vec2<f32>>(5_f, 6_f)),
           },
           tint::Vector{
               b.Stage(PipelineStage::kCompute),
               b.WorkgroupSize(1_i),
           });

    auto* expect = R"(
struct S {
  @size(8)
  padding_0 : u32,
  /* @offset(8) */
  m : @stride(32) array<vec2<f32>, 2u>,
}

@group(0) @binding(0) var<storage, read_write> s : S;

fn arr_to_mat2x2_stride_32(arr : @stride(32) array<vec2<f32>, 2u>) -> mat2x2<f32> {
  return mat2x2<f32>(arr[0u], arr[1u]);
}

fn mat2x2_stride_32_to_arr(m : mat2x2<f32>) -> @stride(32) array<vec2<f32>, 2u> {
  return @stride(32) array<vec2<f32>, 2u>(m[0u], m[1u]);
}

@compute @workgroup_size(1i)
fn f() {
  let x = arr_to_mat2x2_stride_32(s.m);
  let y = s.m[1i];
  let z = x[1i];
  s.m = mat2x2_stride_32_to_arr(mat2x2<f32>(vec2<f32>(1.0f, 2.0f), vec2<f32>(3.0f, 4.0f)));
  s.m[1i] = vec2<f32>(5.0f, 6.0f);
}
)";

    auto got = Run<Unshadow, SimplifyPointers, DecomposeStridedMatrix>(resolver::Resolve(b));

    EXPECT_EQ(expect, str(got));
}

TEST_F(DecomposeStridedMatrixTest, ReadPrivateMatrix) {
    // struct S {
    //   @offset(8) @stride(32)
    //   @internal(ignore_stride_attribute)
    //   m : mat2x2<f32>,
    // };
    // var<private> s : S;
    //
    // @compute @workgroup_size(1)
    // fn f() {
    //   let x : mat2x2<f32> = s.m;
    // }
    ProgramBuilder b;
    auto* S =
        b.Structure("S", tint::Vector{
                             b.Member("m", b.ty.mat2x2<f32>(),
                                      tint::Vector{
                                          b.MemberOffset(8_u),
                                          b.create<StrideAttribute>(32u),
                                          b.Disable(DisabledValidation::kIgnoreStrideAttribute),
                                      }),
                         });
    b.GlobalVar("s", b.ty.Of(S), core::AddressSpace::kPrivate);
    b.Func("f", tint::Empty, b.ty.void_(),
           tint::Vector{
               b.Decl(b.Let("x", b.ty.mat2x2<f32>(), b.MemberAccessor("s", "m"))),
           },
           tint::Vector{
               b.Stage(PipelineStage::kCompute),
               b.WorkgroupSize(1_i),
           });

    auto* expect = R"(
struct S {
  @size(8)
  padding_0 : u32,
  /* @offset(8u) */
  @stride(32) @internal(disable_validation__ignore_stride)
  m : mat2x2<f32>,
}

var<private> s : S;

@compute @workgroup_size(1i)
fn f() {
  let x : mat2x2<f32> = s.m;
}
)";

    auto got = Run<Unshadow, SimplifyPointers, DecomposeStridedMatrix>(resolver::Resolve(b));

    EXPECT_EQ(expect, str(got));
}

TEST_F(DecomposeStridedMatrixTest, WritePrivateMatrix) {
    // struct S {
    //   @offset(8) @stride(32)
    //   @internal(ignore_stride_attribute)
    //   m : mat2x2<f32>,
    // };
    // var<private> s : S;
    //
    // @compute @workgroup_size(1)
    // fn f() {
    //   s.m = mat2x2<f32>(vec2<f32>(1.0, 2.0), vec2<f32>(3.0, 4.0));
    // }
    ProgramBuilder b;
    auto* S =
        b.Structure("S", tint::Vector{
                             b.Member("m", b.ty.mat2x2<f32>(),
                                      tint::Vector{
                                          b.MemberOffset(8_u),
                                          b.create<StrideAttribute>(32u),
                                          b.Disable(DisabledValidation::kIgnoreStrideAttribute),
                                      }),
                         });
    b.GlobalVar("s", b.ty.Of(S), core::AddressSpace::kPrivate);
    b.Func(
        "f", tint::Empty, b.ty.void_(),
        tint::Vector{
            b.Assign(b.MemberAccessor("s", "m"),
                     b.Call<mat2x2<f32>>(b.Call<vec2<f32>>(1_f, 2_f), b.Call<vec2<f32>>(3_f, 4_f))),
        },
        tint::Vector{
            b.Stage(PipelineStage::kCompute),
            b.WorkgroupSize(1_i),
        });

    auto* expect = R"(
struct S {
  @size(8)
  padding_0 : u32,
  /* @offset(8u) */
  @stride(32) @internal(disable_validation__ignore_stride)
  m : mat2x2<f32>,
}

var<private> s : S;

@compute @workgroup_size(1i)
fn f() {
  s.m = mat2x2<f32>(vec2<f32>(1.0f, 2.0f), vec2<f32>(3.0f, 4.0f));
}
)";

    auto got = Run<Unshadow, SimplifyPointers, DecomposeStridedMatrix>(resolver::Resolve(b));

    EXPECT_EQ(expect, str(got));
}

}  // namespace
}  // namespace tint::ast::transform
