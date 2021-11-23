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

#include "src/transform/decompose_strided_matrix.h"

#include <memory>
#include <utility>
#include <vector>

#include "src/ast/disable_validation_decoration.h"
#include "src/program_builder.h"
#include "src/transform/simplify_pointers.h"
#include "src/transform/test_helper.h"
#include "src/transform/unshadow.h"

namespace tint {
namespace transform {
namespace {

using DecomposeStridedMatrixTest = TransformTest;
using f32 = ProgramBuilder::f32;

TEST_F(DecomposeStridedMatrixTest, Empty) {
  auto* src = R"()";
  auto* expect = src;

  auto got = Run<Unshadow, SimplifyPointers, DecomposeStridedMatrix>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(DecomposeStridedMatrixTest, MissingDependencySimplify) {
  auto* src = R"()";
  auto* expect =
      R"(error: tint::transform::DecomposeStridedMatrix depends on tint::transform::SimplifyPointers but the dependency was not run)";

  auto got = Run<DecomposeStridedMatrix>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(DecomposeStridedMatrixTest, ReadUniformMatrix) {
  // [[block]]
  // struct S {
  //   [[offset(16), stride(32)]]
  //   [[internal(ignore_stride_decoration)]]
  //   m : mat2x2<f32>;
  // };
  // [[group(0), binding(0)]] var<uniform> s : S;
  //
  // [[stage(compute), workgroup_size(1)]]
  // fn f() {
  //   let x : mat2x2<f32> = s.m;
  // }
  ProgramBuilder b;
  auto* S = b.Structure(
      "S",
      {
          b.Member(
              "m", b.ty.mat2x2<f32>(),
              {
                  b.create<ast::StructMemberOffsetDecoration>(16),
                  b.create<ast::StrideDecoration>(32),
                  b.Disable(ast::DisabledValidation::kIgnoreStrideDecoration),
              }),
      },
      {
          b.StructBlock(),
      });
  b.Global("s", b.ty.Of(S), ast::StorageClass::kUniform,
           b.GroupAndBinding(0, 0));
  b.Func(
      "f", {}, b.ty.void_(),
      {
          b.Decl(b.Const("x", b.ty.mat2x2<f32>(), b.MemberAccessor("s", "m"))),
      },
      {
          b.Stage(ast::PipelineStage::kCompute),
          b.WorkgroupSize(1),
      });

  auto* expect = R"(
[[block]]
struct S {
  [[size(16)]]
  padding : u32;
  m : [[stride(32)]] array<vec2<f32>, 2u>;
};

[[group(0), binding(0)]] var<uniform> s : S;

fn arr_to_mat2x2_stride_32(arr : [[stride(32)]] array<vec2<f32>, 2u>) -> mat2x2<f32> {
  return mat2x2<f32>(arr[0u], arr[1u]);
}

[[stage(compute), workgroup_size(1)]]
fn f() {
  let x : mat2x2<f32> = arr_to_mat2x2_stride_32(s.m);
}
)";

  auto got = Run<Unshadow, SimplifyPointers, DecomposeStridedMatrix>(
      Program(std::move(b)));

  EXPECT_EQ(expect, str(got));
}

TEST_F(DecomposeStridedMatrixTest, ReadUniformColumn) {
  // [[block]]
  // struct S {
  //   [[offset(16), stride(32)]]
  //   [[internal(ignore_stride_decoration)]]
  //   m : mat2x2<f32>;
  // };
  // [[group(0), binding(0)]] var<uniform> s : S;
  //
  // [[stage(compute), workgroup_size(1)]]
  // fn f() {
  //   let x : vec2<f32> = s.m[1];
  // }
  ProgramBuilder b;
  auto* S = b.Structure(
      "S",
      {
          b.Member(
              "m", b.ty.mat2x2<f32>(),
              {
                  b.create<ast::StructMemberOffsetDecoration>(16),
                  b.create<ast::StrideDecoration>(32),
                  b.Disable(ast::DisabledValidation::kIgnoreStrideDecoration),
              }),
      },
      {
          b.StructBlock(),
      });
  b.Global("s", b.ty.Of(S), ast::StorageClass::kUniform,
           b.GroupAndBinding(0, 0));
  b.Func("f", {}, b.ty.void_(),
         {
             b.Decl(b.Const("x", b.ty.vec2<f32>(),
                            b.IndexAccessor(b.MemberAccessor("s", "m"), 1))),
         },
         {
             b.Stage(ast::PipelineStage::kCompute),
             b.WorkgroupSize(1),
         });

  auto* expect = R"(
[[block]]
struct S {
  [[size(16)]]
  padding : u32;
  m : [[stride(32)]] array<vec2<f32>, 2u>;
};

[[group(0), binding(0)]] var<uniform> s : S;

[[stage(compute), workgroup_size(1)]]
fn f() {
  let x : vec2<f32> = s.m[1];
}
)";

  auto got = Run<Unshadow, SimplifyPointers, DecomposeStridedMatrix>(
      Program(std::move(b)));

  EXPECT_EQ(expect, str(got));
}

TEST_F(DecomposeStridedMatrixTest, ReadUniformMatrix_DefaultStride) {
  // [[block]]
  // struct S {
  //   [[offset(16), stride(8)]]
  //   [[internal(ignore_stride_decoration)]]
  //   m : mat2x2<f32>;
  // };
  // [[group(0), binding(0)]] var<uniform> s : S;
  //
  // [[stage(compute), workgroup_size(1)]]
  // fn f() {
  //   let x : mat2x2<f32> = s.m;
  // }
  ProgramBuilder b;
  auto* S = b.Structure(
      "S",
      {
          b.Member(
              "m", b.ty.mat2x2<f32>(),
              {
                  b.create<ast::StructMemberOffsetDecoration>(16),
                  b.create<ast::StrideDecoration>(8),
                  b.Disable(ast::DisabledValidation::kIgnoreStrideDecoration),
              }),
      },
      {
          b.StructBlock(),
      });
  b.Global("s", b.ty.Of(S), ast::StorageClass::kUniform,
           b.GroupAndBinding(0, 0));
  b.Func(
      "f", {}, b.ty.void_(),
      {
          b.Decl(b.Const("x", b.ty.mat2x2<f32>(), b.MemberAccessor("s", "m"))),
      },
      {
          b.Stage(ast::PipelineStage::kCompute),
          b.WorkgroupSize(1),
      });

  auto* expect = R"(
[[block]]
struct S {
  [[size(16)]]
  padding : u32;
  [[stride(8), internal(disable_validation__ignore_stride)]]
  m : mat2x2<f32>;
};

[[group(0), binding(0)]] var<uniform> s : S;

[[stage(compute), workgroup_size(1)]]
fn f() {
  let x : mat2x2<f32> = s.m;
}
)";

  auto got = Run<Unshadow, SimplifyPointers, DecomposeStridedMatrix>(
      Program(std::move(b)));

  EXPECT_EQ(expect, str(got));
}

TEST_F(DecomposeStridedMatrixTest, ReadStorageMatrix) {
  // [[block]]
  // struct S {
  //   [[offset(8), stride(32)]]
  //   [[internal(ignore_stride_decoration)]]
  //   m : mat2x2<f32>;
  // };
  // [[group(0), binding(0)]] var<storage, read_write> s : S;
  //
  // [[stage(compute), workgroup_size(1)]]
  // fn f() {
  //   let x : mat2x2<f32> = s.m;
  // }
  ProgramBuilder b;
  auto* S = b.Structure(
      "S",
      {
          b.Member(
              "m", b.ty.mat2x2<f32>(),
              {
                  b.create<ast::StructMemberOffsetDecoration>(8),
                  b.create<ast::StrideDecoration>(32),
                  b.Disable(ast::DisabledValidation::kIgnoreStrideDecoration),
              }),
      },
      {
          b.StructBlock(),
      });
  b.Global("s", b.ty.Of(S), ast::StorageClass::kStorage,
           ast::Access::kReadWrite, b.GroupAndBinding(0, 0));
  b.Func(
      "f", {}, b.ty.void_(),
      {
          b.Decl(b.Const("x", b.ty.mat2x2<f32>(), b.MemberAccessor("s", "m"))),
      },
      {
          b.Stage(ast::PipelineStage::kCompute),
          b.WorkgroupSize(1),
      });

  auto* expect = R"(
[[block]]
struct S {
  [[size(8)]]
  padding : u32;
  m : [[stride(32)]] array<vec2<f32>, 2u>;
};

[[group(0), binding(0)]] var<storage, read_write> s : S;

fn arr_to_mat2x2_stride_32(arr : [[stride(32)]] array<vec2<f32>, 2u>) -> mat2x2<f32> {
  return mat2x2<f32>(arr[0u], arr[1u]);
}

[[stage(compute), workgroup_size(1)]]
fn f() {
  let x : mat2x2<f32> = arr_to_mat2x2_stride_32(s.m);
}
)";

  auto got = Run<Unshadow, SimplifyPointers, DecomposeStridedMatrix>(
      Program(std::move(b)));

  EXPECT_EQ(expect, str(got));
}

TEST_F(DecomposeStridedMatrixTest, ReadStorageColumn) {
  // [[block]]
  // struct S {
  //   [[offset(16), stride(32)]]
  //   [[internal(ignore_stride_decoration)]]
  //   m : mat2x2<f32>;
  // };
  // [[group(0), binding(0)]] var<storage, read_write> s : S;
  //
  // [[stage(compute), workgroup_size(1)]]
  // fn f() {
  //   let x : vec2<f32> = s.m[1];
  // }
  ProgramBuilder b;
  auto* S = b.Structure(
      "S",
      {
          b.Member(
              "m", b.ty.mat2x2<f32>(),
              {
                  b.create<ast::StructMemberOffsetDecoration>(16),
                  b.create<ast::StrideDecoration>(32),
                  b.Disable(ast::DisabledValidation::kIgnoreStrideDecoration),
              }),
      },
      {
          b.StructBlock(),
      });
  b.Global("s", b.ty.Of(S), ast::StorageClass::kStorage,
           ast::Access::kReadWrite, b.GroupAndBinding(0, 0));
  b.Func("f", {}, b.ty.void_(),
         {
             b.Decl(b.Const("x", b.ty.vec2<f32>(),
                            b.IndexAccessor(b.MemberAccessor("s", "m"), 1))),
         },
         {
             b.Stage(ast::PipelineStage::kCompute),
             b.WorkgroupSize(1),
         });

  auto* expect = R"(
[[block]]
struct S {
  [[size(16)]]
  padding : u32;
  m : [[stride(32)]] array<vec2<f32>, 2u>;
};

[[group(0), binding(0)]] var<storage, read_write> s : S;

[[stage(compute), workgroup_size(1)]]
fn f() {
  let x : vec2<f32> = s.m[1];
}
)";

  auto got = Run<Unshadow, SimplifyPointers, DecomposeStridedMatrix>(
      Program(std::move(b)));

  EXPECT_EQ(expect, str(got));
}

TEST_F(DecomposeStridedMatrixTest, WriteStorageMatrix) {
  // [[block]]
  // struct S {
  //   [[offset(8), stride(32)]]
  //   [[internal(ignore_stride_decoration)]]
  //   m : mat2x2<f32>;
  // };
  // [[group(0), binding(0)]] var<storage, read_write> s : S;
  //
  // [[stage(compute), workgroup_size(1)]]
  // fn f() {
  //   s.m = mat2x2<f32>(vec2<f32>(1.0, 2.0), vec2<f32>(3.0, 4.0));
  // }
  ProgramBuilder b;
  auto* S = b.Structure(
      "S",
      {
          b.Member(
              "m", b.ty.mat2x2<f32>(),
              {
                  b.create<ast::StructMemberOffsetDecoration>(8),
                  b.create<ast::StrideDecoration>(32),
                  b.Disable(ast::DisabledValidation::kIgnoreStrideDecoration),
              }),
      },
      {
          b.StructBlock(),
      });
  b.Global("s", b.ty.Of(S), ast::StorageClass::kStorage,
           ast::Access::kReadWrite, b.GroupAndBinding(0, 0));
  b.Func("f", {}, b.ty.void_(),
         {
             b.Assign(b.MemberAccessor("s", "m"),
                      b.mat2x2<f32>(b.vec2<f32>(1.0f, 2.0f),
                                    b.vec2<f32>(3.0f, 4.0f))),
         },
         {
             b.Stage(ast::PipelineStage::kCompute),
             b.WorkgroupSize(1),
         });

  auto* expect = R"(
[[block]]
struct S {
  [[size(8)]]
  padding : u32;
  m : [[stride(32)]] array<vec2<f32>, 2u>;
};

[[group(0), binding(0)]] var<storage, read_write> s : S;

fn mat2x2_stride_32_to_arr(mat : mat2x2<f32>) -> [[stride(32)]] array<vec2<f32>, 2u> {
  return [[stride(32)]] array<vec2<f32>, 2u>(mat[0u], mat[1u]);
}

[[stage(compute), workgroup_size(1)]]
fn f() {
  s.m = mat2x2_stride_32_to_arr(mat2x2<f32>(vec2<f32>(1.0, 2.0), vec2<f32>(3.0, 4.0)));
}
)";

  auto got = Run<Unshadow, SimplifyPointers, DecomposeStridedMatrix>(
      Program(std::move(b)));

  EXPECT_EQ(expect, str(got));
}

TEST_F(DecomposeStridedMatrixTest, WriteStorageColumn) {
  // [[block]]
  // struct S {
  //   [[offset(8), stride(32)]]
  //   [[internal(ignore_stride_decoration)]]
  //   m : mat2x2<f32>;
  // };
  // [[group(0), binding(0)]] var<storage, read_write> s : S;
  //
  // [[stage(compute), workgroup_size(1)]]
  // fn f() {
  //   s.m[1] = vec2<f32>(1.0, 2.0);
  // }
  ProgramBuilder b;
  auto* S = b.Structure(
      "S",
      {
          b.Member(
              "m", b.ty.mat2x2<f32>(),
              {
                  b.create<ast::StructMemberOffsetDecoration>(8),
                  b.create<ast::StrideDecoration>(32),
                  b.Disable(ast::DisabledValidation::kIgnoreStrideDecoration),
              }),
      },
      {
          b.StructBlock(),
      });
  b.Global("s", b.ty.Of(S), ast::StorageClass::kStorage,
           ast::Access::kReadWrite, b.GroupAndBinding(0, 0));
  b.Func("f", {}, b.ty.void_(),
         {
             b.Assign(b.IndexAccessor(b.MemberAccessor("s", "m"), 1),
                      b.vec2<f32>(1.0f, 2.0f)),
         },
         {
             b.Stage(ast::PipelineStage::kCompute),
             b.WorkgroupSize(1),
         });

  auto* expect = R"(
[[block]]
struct S {
  [[size(8)]]
  padding : u32;
  m : [[stride(32)]] array<vec2<f32>, 2u>;
};

[[group(0), binding(0)]] var<storage, read_write> s : S;

[[stage(compute), workgroup_size(1)]]
fn f() {
  s.m[1] = vec2<f32>(1.0, 2.0);
}
)";

  auto got = Run<Unshadow, SimplifyPointers, DecomposeStridedMatrix>(
      Program(std::move(b)));

  EXPECT_EQ(expect, str(got));
}

TEST_F(DecomposeStridedMatrixTest, ReadWriteViaPointerLets) {
  // [[block]]
  // struct S {
  //   [[offset(8), stride(32)]]
  //   [[internal(ignore_stride_decoration)]]
  //   m : mat2x2<f32>;
  // };
  // [[group(0), binding(0)]] var<storage, read_write> s : S;
  //
  // [[stage(compute), workgroup_size(1)]]
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
  auto* S = b.Structure(
      "S",
      {
          b.Member(
              "m", b.ty.mat2x2<f32>(),
              {
                  b.create<ast::StructMemberOffsetDecoration>(8),
                  b.create<ast::StrideDecoration>(32),
                  b.Disable(ast::DisabledValidation::kIgnoreStrideDecoration),
              }),
      },
      {
          b.StructBlock(),
      });
  b.Global("s", b.ty.Of(S), ast::StorageClass::kStorage,
           ast::Access::kReadWrite, b.GroupAndBinding(0, 0));
  b.Func(
      "f", {}, b.ty.void_(),
      {
          b.Decl(
              b.Const("a", nullptr, b.AddressOf(b.MemberAccessor("s", "m")))),
          b.Decl(b.Const("b", nullptr,
                         b.AddressOf(b.Deref(b.AddressOf(b.Deref("a")))))),
          b.Decl(b.Const("x", nullptr, b.Deref("b"))),
          b.Decl(b.Const("y", nullptr, b.IndexAccessor(b.Deref("b"), 1))),
          b.Decl(b.Const("z", nullptr, b.IndexAccessor("x", 1))),
          b.Assign(b.Deref("b"), b.mat2x2<f32>(b.vec2<f32>(1.0f, 2.0f),
                                               b.vec2<f32>(3.0f, 4.0f))),
          b.Assign(b.IndexAccessor(b.Deref("b"), 1), b.vec2<f32>(5.0f, 6.0f)),
      },
      {
          b.Stage(ast::PipelineStage::kCompute),
          b.WorkgroupSize(1),
      });

  auto* expect = R"(
[[block]]
struct S {
  [[size(8)]]
  padding : u32;
  m : [[stride(32)]] array<vec2<f32>, 2u>;
};

[[group(0), binding(0)]] var<storage, read_write> s : S;

fn arr_to_mat2x2_stride_32(arr : [[stride(32)]] array<vec2<f32>, 2u>) -> mat2x2<f32> {
  return mat2x2<f32>(arr[0u], arr[1u]);
}

fn mat2x2_stride_32_to_arr(mat : mat2x2<f32>) -> [[stride(32)]] array<vec2<f32>, 2u> {
  return [[stride(32)]] array<vec2<f32>, 2u>(mat[0u], mat[1u]);
}

[[stage(compute), workgroup_size(1)]]
fn f() {
  let x = arr_to_mat2x2_stride_32(s.m);
  let y = s.m[1];
  let z = x[1];
  s.m = mat2x2_stride_32_to_arr(mat2x2<f32>(vec2<f32>(1.0, 2.0), vec2<f32>(3.0, 4.0)));
  s.m[1] = vec2<f32>(5.0, 6.0);
}
)";

  auto got = Run<Unshadow, SimplifyPointers, DecomposeStridedMatrix>(
      Program(std::move(b)));

  EXPECT_EQ(expect, str(got));
}

TEST_F(DecomposeStridedMatrixTest, ReadPrivateMatrix) {
  // struct S {
  //   [[offset(8), stride(32)]]
  //   [[internal(ignore_stride_decoration)]]
  //   m : mat2x2<f32>;
  // };
  // var<private> s : S;
  //
  // [[stage(compute), workgroup_size(1)]]
  // fn f() {
  //   let x : mat2x2<f32> = s.m;
  // }
  ProgramBuilder b;
  auto* S = b.Structure(
      "S",
      {
          b.Member(
              "m", b.ty.mat2x2<f32>(),
              {
                  b.create<ast::StructMemberOffsetDecoration>(8),
                  b.create<ast::StrideDecoration>(32),
                  b.Disable(ast::DisabledValidation::kIgnoreStrideDecoration),
              }),
      });
  b.Global("s", b.ty.Of(S), ast::StorageClass::kPrivate);
  b.Func(
      "f", {}, b.ty.void_(),
      {
          b.Decl(b.Const("x", b.ty.mat2x2<f32>(), b.MemberAccessor("s", "m"))),
      },
      {
          b.Stage(ast::PipelineStage::kCompute),
          b.WorkgroupSize(1),
      });

  auto* expect = R"(
struct S {
  [[size(8)]]
  padding : u32;
  [[stride(32), internal(disable_validation__ignore_stride)]]
  m : mat2x2<f32>;
};

var<private> s : S;

[[stage(compute), workgroup_size(1)]]
fn f() {
  let x : mat2x2<f32> = s.m;
}
)";

  auto got = Run<Unshadow, SimplifyPointers, DecomposeStridedMatrix>(
      Program(std::move(b)));

  EXPECT_EQ(expect, str(got));
}

TEST_F(DecomposeStridedMatrixTest, WritePrivateMatrix) {
  // struct S {
  //   [[offset(8), stride(32)]]
  //   [[internal(ignore_stride_decoration)]]
  //   m : mat2x2<f32>;
  // };
  // var<private> s : S;
  //
  // [[stage(compute), workgroup_size(1)]]
  // fn f() {
  //   s.m = mat2x2<f32>(vec2<f32>(1.0, 2.0), vec2<f32>(3.0, 4.0));
  // }
  ProgramBuilder b;
  auto* S = b.Structure(
      "S",
      {
          b.Member(
              "m", b.ty.mat2x2<f32>(),
              {
                  b.create<ast::StructMemberOffsetDecoration>(8),
                  b.create<ast::StrideDecoration>(32),
                  b.Disable(ast::DisabledValidation::kIgnoreStrideDecoration),
              }),
      });
  b.Global("s", b.ty.Of(S), ast::StorageClass::kPrivate);
  b.Func("f", {}, b.ty.void_(),
         {
             b.Assign(b.MemberAccessor("s", "m"),
                      b.mat2x2<f32>(b.vec2<f32>(1.0f, 2.0f),
                                    b.vec2<f32>(3.0f, 4.0f))),
         },
         {
             b.Stage(ast::PipelineStage::kCompute),
             b.WorkgroupSize(1),
         });

  auto* expect = R"(
struct S {
  [[size(8)]]
  padding : u32;
  [[stride(32), internal(disable_validation__ignore_stride)]]
  m : mat2x2<f32>;
};

var<private> s : S;

[[stage(compute), workgroup_size(1)]]
fn f() {
  s.m = mat2x2<f32>(vec2<f32>(1.0, 2.0), vec2<f32>(3.0, 4.0));
}
)";

  auto got = Run<Unshadow, SimplifyPointers, DecomposeStridedMatrix>(
      Program(std::move(b)));

  EXPECT_EQ(expect, str(got));
}

}  // namespace
}  // namespace transform
}  // namespace tint
