// Copyright 2022 The Tint Authors.
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

#include "src/tint/transform/decompose_strided_array.h"

#include <memory>
#include <utility>
#include <vector>

#include "src/tint/program_builder.h"
#include "src/tint/transform/simplify_pointers.h"
#include "src/tint/transform/test_helper.h"
#include "src/tint/transform/unshadow.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::transform {
namespace {

using DecomposeStridedArrayTest = TransformTest;

TEST_F(DecomposeStridedArrayTest, ShouldRunEmptyModule) {
    ProgramBuilder b;
    EXPECT_FALSE(ShouldRun<DecomposeStridedArray>(Program(std::move(b))));
}

TEST_F(DecomposeStridedArrayTest, ShouldRunNonStridedArray) {
    // var<private> arr : array<f32, 4u>

    ProgramBuilder b;
    b.GlobalVar("arr", b.ty.array<f32, 4u>(), builtin::AddressSpace::kPrivate);
    EXPECT_FALSE(ShouldRun<DecomposeStridedArray>(Program(std::move(b))));
}

TEST_F(DecomposeStridedArrayTest, ShouldRunDefaultStridedArray) {
    // var<private> arr : @stride(4) array<f32, 4u>

    ProgramBuilder b;
    b.GlobalVar("arr",
                b.ty.array<f32, 4u>(utils::Vector{
                    b.Stride(4),
                }),
                builtin::AddressSpace::kPrivate);
    EXPECT_TRUE(ShouldRun<DecomposeStridedArray>(Program(std::move(b))));
}

TEST_F(DecomposeStridedArrayTest, ShouldRunExplicitStridedArray) {
    // var<private> arr : @stride(16) array<f32, 4u>

    ProgramBuilder b;
    b.GlobalVar("arr",
                b.ty.array<f32, 4u>(utils::Vector{
                    b.Stride(16),
                }),
                builtin::AddressSpace::kPrivate);
    EXPECT_TRUE(ShouldRun<DecomposeStridedArray>(Program(std::move(b))));
}

TEST_F(DecomposeStridedArrayTest, Empty) {
    auto* src = R"()";
    auto* expect = src;

    auto got = Run<DecomposeStridedArray>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DecomposeStridedArrayTest, PrivateDefaultStridedArray) {
    // var<private> arr : @stride(4) array<f32, 4u>
    //
    // @compute @workgroup_size(1)
    // fn f() {
    //   let a : @stride(4) array<f32, 4u> = a;
    //   let b : f32 = arr[1];
    // }

    ProgramBuilder b;
    b.GlobalVar("arr",
                b.ty.array<f32, 4u>(utils::Vector{
                    b.Stride(4),
                }),
                builtin::AddressSpace::kPrivate);
    b.Func("f", utils::Empty, b.ty.void_(),
           utils::Vector{
               b.Decl(b.Let("a",
                            b.ty.array<f32, 4u>(utils::Vector{
                                b.Stride(4),
                            }),
                            b.Expr("arr"))),
               b.Decl(b.Let("b", b.ty.f32(), b.IndexAccessor("arr", 1_i))),
           },
           utils::Vector{
               b.Stage(ast::PipelineStage::kCompute),
               b.WorkgroupSize(1_i),
           });

    auto* expect = R"(
var<private> arr : array<f32, 4u>;

@compute @workgroup_size(1i)
fn f() {
  let a : array<f32, 4u> = arr;
  let b : f32 = arr[1i];
}
)";

    auto got = Run<Unshadow, SimplifyPointers, DecomposeStridedArray>(Program(std::move(b)));

    EXPECT_EQ(expect, str(got));
}

TEST_F(DecomposeStridedArrayTest, PrivateStridedArray) {
    // var<private> arr : @stride(32) array<f32, 4u>
    //
    // @compute @workgroup_size(1)
    // fn f() {
    //   let a : @stride(32) array<f32, 4u> = a;
    //   let b : f32 = arr[1];
    // }

    ProgramBuilder b;
    b.GlobalVar("arr",
                b.ty.array<f32, 4u>(utils::Vector{
                    b.Stride(32),
                }),
                builtin::AddressSpace::kPrivate);
    b.Func("f", utils::Empty, b.ty.void_(),
           utils::Vector{
               b.Decl(b.Let("a",
                            b.ty.array<f32, 4u>(utils::Vector{
                                b.Stride(32),
                            }),
                            b.Expr("arr"))),
               b.Decl(b.Let("b", b.ty.f32(), b.IndexAccessor("arr", 1_i))),
           },
           utils::Vector{
               b.Stage(ast::PipelineStage::kCompute),
               b.WorkgroupSize(1_i),
           });

    auto* expect = R"(
struct strided_arr {
  @size(32)
  el : f32,
}

var<private> arr : array<strided_arr, 4u>;

@compute @workgroup_size(1i)
fn f() {
  let a : array<strided_arr, 4u> = arr;
  let b : f32 = arr[1i].el;
}
)";

    auto got = Run<Unshadow, SimplifyPointers, DecomposeStridedArray>(Program(std::move(b)));

    EXPECT_EQ(expect, str(got));
}

TEST_F(DecomposeStridedArrayTest, ReadUniformStridedArray) {
    // struct S {
    //   a : @stride(32) array<f32, 4u>,
    // };
    // @group(0) @binding(0) var<uniform> s : S;
    //
    // @compute @workgroup_size(1)
    // fn f() {
    //   let a : @stride(32) array<f32, 4u> = s.a;
    //   let b : f32 = s.a[1];
    // }
    ProgramBuilder b;
    auto* S = b.Structure("S", utils::Vector{b.Member("a", b.ty.array<f32, 4u>(utils::Vector{
                                                               b.Stride(32),
                                                           }))});
    b.GlobalVar("s", b.ty.Of(S), builtin::AddressSpace::kUniform, b.Group(0_a), b.Binding(0_a));
    b.Func("f", utils::Empty, b.ty.void_(),
           utils::Vector{
               b.Decl(b.Let("a",
                            b.ty.array<f32, 4u>(utils::Vector{
                                b.Stride(32),
                            }),
                            b.MemberAccessor("s", "a"))),
               b.Decl(b.Let("b", b.ty.f32(), b.IndexAccessor(b.MemberAccessor("s", "a"), 1_i))),
           },
           utils::Vector{
               b.Stage(ast::PipelineStage::kCompute),
               b.WorkgroupSize(1_i),
           });

    auto* expect = R"(
struct strided_arr {
  @size(32)
  el : f32,
}

struct S {
  a : array<strided_arr, 4u>,
}

@group(0) @binding(0) var<uniform> s : S;

@compute @workgroup_size(1i)
fn f() {
  let a : array<strided_arr, 4u> = s.a;
  let b : f32 = s.a[1i].el;
}
)";

    auto got = Run<Unshadow, SimplifyPointers, DecomposeStridedArray>(Program(std::move(b)));

    EXPECT_EQ(expect, str(got));
}

TEST_F(DecomposeStridedArrayTest, ReadUniformDefaultStridedArray) {
    // struct S {
    //   a : @stride(16) array<vec4<f32>, 4u>,
    // };
    // @group(0) @binding(0) var<uniform> s : S;
    //
    // @compute @workgroup_size(1)
    // fn f() {
    //   let a : @stride(16) array<vec4<f32>, 4u> = s.a;
    //   let b : f32 = s.a[1][2];
    // }
    ProgramBuilder b;
    auto* S = b.Structure("S", utils::Vector{b.Member("a", b.ty.array(b.ty.vec4<f32>(), 4_u,
                                                                      utils::Vector{
                                                                          b.Stride(16),
                                                                      }))});
    b.GlobalVar("s", b.ty.Of(S), builtin::AddressSpace::kUniform, b.Group(0_a), b.Binding(0_a));
    b.Func(
        "f", utils::Empty, b.ty.void_(),
        utils::Vector{
            b.Decl(b.Let("a",
                         b.ty.array(b.ty.vec4<f32>(), 4_u,
                                    utils::Vector{
                                        b.Stride(16),
                                    }),
                         b.MemberAccessor("s", "a"))),
            b.Decl(b.Let("b", b.ty.f32(),
                         b.IndexAccessor(b.IndexAccessor(b.MemberAccessor("s", "a"), 1_i), 2_i))),
        },
        utils::Vector{
            b.Stage(ast::PipelineStage::kCompute),
            b.WorkgroupSize(1_i),
        });

    auto* expect =
        R"(
struct S {
  a : array<vec4<f32>, 4u>,
}

@group(0) @binding(0) var<uniform> s : S;

@compute @workgroup_size(1i)
fn f() {
  let a : array<vec4<f32>, 4u> = s.a;
  let b : f32 = s.a[1i][2i];
}
)";

    auto got = Run<Unshadow, SimplifyPointers, DecomposeStridedArray>(Program(std::move(b)));

    EXPECT_EQ(expect, str(got));
}

TEST_F(DecomposeStridedArrayTest, ReadStorageStridedArray) {
    // struct S {
    //   a : @stride(32) array<f32, 4u>,
    // };
    // @group(0) @binding(0) var<storage> s : S;
    //
    // @compute @workgroup_size(1)
    // fn f() {
    //   let a : @stride(32) array<f32, 4u> = s.a;
    //   let b : f32 = s.a[1];
    // }
    ProgramBuilder b;
    auto* S = b.Structure("S", utils::Vector{b.Member("a", b.ty.array<f32, 4u>(utils::Vector{
                                                               b.Stride(32),
                                                           }))});
    b.GlobalVar("s", b.ty.Of(S), builtin::AddressSpace::kStorage, b.Group(0_a), b.Binding(0_a));
    b.Func("f", utils::Empty, b.ty.void_(),
           utils::Vector{
               b.Decl(b.Let("a",
                            b.ty.array<f32, 4u>(utils::Vector{
                                b.Stride(32),
                            }),
                            b.MemberAccessor("s", "a"))),
               b.Decl(b.Let("b", b.ty.f32(), b.IndexAccessor(b.MemberAccessor("s", "a"), 1_i))),
           },
           utils::Vector{
               b.Stage(ast::PipelineStage::kCompute),
               b.WorkgroupSize(1_i),
           });

    auto* expect = R"(
struct strided_arr {
  @size(32)
  el : f32,
}

struct S {
  a : array<strided_arr, 4u>,
}

@group(0) @binding(0) var<storage> s : S;

@compute @workgroup_size(1i)
fn f() {
  let a : array<strided_arr, 4u> = s.a;
  let b : f32 = s.a[1i].el;
}
)";

    auto got = Run<Unshadow, SimplifyPointers, DecomposeStridedArray>(Program(std::move(b)));

    EXPECT_EQ(expect, str(got));
}

TEST_F(DecomposeStridedArrayTest, ReadStorageDefaultStridedArray) {
    // struct S {
    //   a : @stride(4) array<f32, 4u>,
    // };
    // @group(0) @binding(0) var<storage> s : S;
    //
    // @compute @workgroup_size(1)
    // fn f() {
    //   let a : @stride(4) array<f32, 4u> = s.a;
    //   let b : f32 = s.a[1];
    // }
    ProgramBuilder b;
    auto* S = b.Structure("S", utils::Vector{b.Member("a", b.ty.array<f32, 4u>(utils::Vector{
                                                               b.Stride(4),
                                                           }))});
    b.GlobalVar("s", b.ty.Of(S), builtin::AddressSpace::kStorage, b.Group(0_a), b.Binding(0_a));
    b.Func("f", utils::Empty, b.ty.void_(),
           utils::Vector{
               b.Decl(b.Let("a",
                            b.ty.array<f32, 4u>(utils::Vector{
                                b.Stride(4),
                            }),
                            b.MemberAccessor("s", "a"))),
               b.Decl(b.Let("b", b.ty.f32(), b.IndexAccessor(b.MemberAccessor("s", "a"), 1_i))),
           },
           utils::Vector{
               b.Stage(ast::PipelineStage::kCompute),
               b.WorkgroupSize(1_i),
           });

    auto* expect = R"(
struct S {
  a : array<f32, 4u>,
}

@group(0) @binding(0) var<storage> s : S;

@compute @workgroup_size(1i)
fn f() {
  let a : array<f32, 4u> = s.a;
  let b : f32 = s.a[1i];
}
)";

    auto got = Run<Unshadow, SimplifyPointers, DecomposeStridedArray>(Program(std::move(b)));

    EXPECT_EQ(expect, str(got));
}

TEST_F(DecomposeStridedArrayTest, WriteStorageStridedArray) {
    // struct S {
    //   a : @stride(32) array<f32, 4u>,
    // };
    // @group(0) @binding(0) var<storage, read_write> s : S;
    //
    // @compute @workgroup_size(1)
    // fn f() {
    //   s.a = @stride(32) array<f32, 4u>();
    //   s.a = @stride(32) array<f32, 4u>(1.0, 2.0, 3.0, 4.0);
    //   s.a[1i] = 5.0;
    // }
    ProgramBuilder b;
    auto* S = b.Structure("S", utils::Vector{b.Member("a", b.ty.array<f32, 4u>(utils::Vector{
                                                               b.Stride(32),
                                                           }))});
    b.GlobalVar("s", b.ty.Of(S), builtin::AddressSpace::kStorage, builtin::Access::kReadWrite,
                b.Group(0_a), b.Binding(0_a));
    b.Func("f", utils::Empty, b.ty.void_(),
           utils::Vector{
               b.Assign(b.MemberAccessor("s", "a"), b.Call(b.ty.array<f32, 4u>(utils::Vector{
                                                        b.Stride(32),
                                                    }))),
               b.Assign(b.MemberAccessor("s", "a"), b.Call(b.ty.array<f32, 4u>(utils::Vector{
                                                               b.Stride(32),
                                                           }),
                                                           1_f, 2_f, 3_f, 4_f)),
               b.Assign(b.IndexAccessor(b.MemberAccessor("s", "a"), 1_i), 5_f),
           },
           utils::Vector{
               b.Stage(ast::PipelineStage::kCompute),
               b.WorkgroupSize(1_i),
           });

    auto* expect =
        R"(
struct strided_arr {
  @size(32)
  el : f32,
}

struct S {
  a : array<strided_arr, 4u>,
}

@group(0) @binding(0) var<storage, read_write> s : S;

@compute @workgroup_size(1i)
fn f() {
  s.a = array<strided_arr, 4u>();
  s.a = array<strided_arr, 4u>(strided_arr(1.0f), strided_arr(2.0f), strided_arr(3.0f), strided_arr(4.0f));
  s.a[1i].el = 5.0f;
}
)";

    auto got = Run<Unshadow, SimplifyPointers, DecomposeStridedArray>(Program(std::move(b)));

    EXPECT_EQ(expect, str(got));
}

TEST_F(DecomposeStridedArrayTest, WriteStorageDefaultStridedArray) {
    // struct S {
    //   a : @stride(4) array<f32, 4u>,
    // };
    // @group(0) @binding(0) var<storage, read_write> s : S;
    //
    // @compute @workgroup_size(1)
    // fn f() {
    //   s.a = @stride(4) array<f32, 4u>();
    //   s.a = @stride(4) array<f32, 4u>(1.0, 2.0, 3.0, 4.0);
    //   s.a[1] = 5.0;
    // }
    ProgramBuilder b;
    auto* S = b.Structure("S", utils::Vector{
                                   b.Member("a", b.ty.array<f32, 4u>(utils::Vector{
                                                     b.Stride(4),
                                                 })),
                               });
    b.GlobalVar("s", b.ty.Of(S), builtin::AddressSpace::kStorage, builtin::Access::kReadWrite,
                b.Group(0_a), b.Binding(0_a));
    b.Func("f", utils::Empty, b.ty.void_(),
           utils::Vector{
               b.Assign(b.MemberAccessor("s", "a"), b.Call(b.ty.array<f32, 4u>(utils::Vector{
                                                        b.Stride(4),
                                                    }))),
               b.Assign(b.MemberAccessor("s", "a"), b.Call(b.ty.array<f32, 4u>(utils::Vector{
                                                               b.Stride(4),
                                                           }),
                                                           1_f, 2_f, 3_f, 4_f)),
               b.Assign(b.IndexAccessor(b.MemberAccessor("s", "a"), 1_i), 5_f),
           },
           utils::Vector{
               b.Stage(ast::PipelineStage::kCompute),
               b.WorkgroupSize(1_i),
           });

    auto* expect =
        R"(
struct S {
  a : array<f32, 4u>,
}

@group(0) @binding(0) var<storage, read_write> s : S;

@compute @workgroup_size(1i)
fn f() {
  s.a = array<f32, 4u>();
  s.a = array<f32, 4u>(1.0f, 2.0f, 3.0f, 4.0f);
  s.a[1i] = 5.0f;
}
)";

    auto got = Run<Unshadow, SimplifyPointers, DecomposeStridedArray>(Program(std::move(b)));

    EXPECT_EQ(expect, str(got));
}

TEST_F(DecomposeStridedArrayTest, ReadWriteViaPointerLets) {
    // struct S {
    //   a : @stride(32) array<f32, 4u>,
    // };
    // @group(0) @binding(0) var<storage, read_write> s : S;
    //
    // @compute @workgroup_size(1)
    // fn f() {
    //   let a = &s.a;
    //   let b = &*&*(a);
    //   let c = *b;
    //   let d = (*b)[1];
    //   (*b) = @stride(32) array<f32, 4u>(1.0, 2.0, 3.0, 4.0);
    //   (*b)[1] = 5.0;
    // }
    ProgramBuilder b;
    auto* S = b.Structure("S", utils::Vector{b.Member("a", b.ty.array<f32, 4u>(utils::Vector{
                                                               b.Stride(32),
                                                           }))});
    b.GlobalVar("s", b.ty.Of(S), builtin::AddressSpace::kStorage, builtin::Access::kReadWrite,
                b.Group(0_a), b.Binding(0_a));
    b.Func("f", utils::Empty, b.ty.void_(),
           utils::Vector{
               b.Decl(b.Let("a", b.AddressOf(b.MemberAccessor("s", "a")))),
               b.Decl(b.Let("b", b.AddressOf(b.Deref(b.AddressOf(b.Deref("a")))))),
               b.Decl(b.Let("c", b.Deref("b"))),
               b.Decl(b.Let("d", b.IndexAccessor(b.Deref("b"), 1_i))),
               b.Assign(b.Deref("b"), b.Call(b.ty.array<f32, 4u>(utils::Vector{
                                                 b.Stride(32),
                                             }),
                                             1_f, 2_f, 3_f, 4_f)),
               b.Assign(b.IndexAccessor(b.Deref("b"), 1_i), 5_f),
           },
           utils::Vector{
               b.Stage(ast::PipelineStage::kCompute),
               b.WorkgroupSize(1_i),
           });

    auto* expect =
        R"(
struct strided_arr {
  @size(32)
  el : f32,
}

struct S {
  a : array<strided_arr, 4u>,
}

@group(0) @binding(0) var<storage, read_write> s : S;

@compute @workgroup_size(1i)
fn f() {
  let c = s.a;
  let d = s.a[1i].el;
  s.a = array<strided_arr, 4u>(strided_arr(1.0f), strided_arr(2.0f), strided_arr(3.0f), strided_arr(4.0f));
  s.a[1i].el = 5.0f;
}
)";

    auto got = Run<Unshadow, SimplifyPointers, DecomposeStridedArray>(Program(std::move(b)));

    EXPECT_EQ(expect, str(got));
}

TEST_F(DecomposeStridedArrayTest, PrivateAliasedStridedArray) {
    // type ARR = @stride(32) array<f32, 4u>;
    // struct S {
    //   a : ARR,
    // };
    // @group(0) @binding(0) var<storage, read_write> s : S;
    //
    // @compute @workgroup_size(1)
    // fn f() {
    //   let a : ARR = s.a;
    //   let b : f32 = s.a[1];
    //   s.a = ARR();
    //   s.a = ARR(1.0, 2.0, 3.0, 4.0);
    //   s.a[1] = 5.0;
    // }
    ProgramBuilder b;
    b.Alias("ARR", b.ty.array<f32, 4u>(utils::Vector{
                       b.Stride(32),
                   }));
    auto* S = b.Structure("S", utils::Vector{b.Member("a", b.ty("ARR"))});
    b.GlobalVar("s", b.ty.Of(S), builtin::AddressSpace::kStorage, builtin::Access::kReadWrite,
                b.Group(0_a), b.Binding(0_a));
    b.Func("f", utils::Empty, b.ty.void_(),
           utils::Vector{
               b.Decl(b.Let("a", b.ty("ARR"), b.MemberAccessor("s", "a"))),
               b.Decl(b.Let("b", b.ty.f32(), b.IndexAccessor(b.MemberAccessor("s", "a"), 1_i))),
               b.Assign(b.MemberAccessor("s", "a"), b.Call("ARR")),
               b.Assign(b.MemberAccessor("s", "a"), b.Call("ARR", 1_f, 2_f, 3_f, 4_f)),
               b.Assign(b.IndexAccessor(b.MemberAccessor("s", "a"), 1_i), 5_f),
           },
           utils::Vector{
               b.Stage(ast::PipelineStage::kCompute),
               b.WorkgroupSize(1_i),
           });

    auto* expect = R"(
struct strided_arr {
  @size(32)
  el : f32,
}

alias ARR = array<strided_arr, 4u>;

struct S {
  a : ARR,
}

@group(0) @binding(0) var<storage, read_write> s : S;

@compute @workgroup_size(1i)
fn f() {
  let a : ARR = s.a;
  let b : f32 = s.a[1i].el;
  s.a = ARR();
  s.a = ARR(strided_arr(1.0f), strided_arr(2.0f), strided_arr(3.0f), strided_arr(4.0f));
  s.a[1i].el = 5.0f;
}
)";

    auto got = Run<Unshadow, SimplifyPointers, DecomposeStridedArray>(Program(std::move(b)));

    EXPECT_EQ(expect, str(got));
}

TEST_F(DecomposeStridedArrayTest, PrivateNestedStridedArray) {
    // type ARR_A = @stride(8) array<f32, 2u>;
    // type ARR_B = @stride(128) array<@stride(16) array<ARR_A, 3u>, 4u>;
    // struct S {
    //   a : ARR_B,
    // };
    // @group(0) @binding(0) var<storage, read_write> s : S;
    //
    // @compute @workgroup_size(1)
    // fn f() {
    //   let a : ARR_B = s.a;
    //   let b : array<@stride(8) array<f32, 2u>, 3u> = s.a[3];
    //   let c = s.a[3][2];
    //   let d = s.a[3][2][1];
    //   s.a = ARR_B();
    //   s.a[3][2][1] = 5.0;
    // }

    ProgramBuilder b;
    b.Alias("ARR_A", b.ty.array<f32, 2>(utils::Vector{
                         b.Stride(8),
                     }));
    b.Alias("ARR_B", b.ty.array(  //
                         b.ty.array(b.ty("ARR_A"), 3_u,
                                    utils::Vector{
                                        b.Stride(16),
                                    }),
                         4_u,
                         utils::Vector{
                             b.Stride(128),
                         }));
    auto* S = b.Structure("S", utils::Vector{b.Member("a", b.ty("ARR_B"))});
    b.GlobalVar("s", b.ty.Of(S), builtin::AddressSpace::kStorage, builtin::Access::kReadWrite,
                b.Group(0_a), b.Binding(0_a));
    b.Func("f", utils::Empty, b.ty.void_(),
           utils::Vector{
               b.Decl(b.Let("a", b.ty("ARR_B"), b.MemberAccessor("s", "a"))),
               b.Decl(b.Let("b",
                            b.ty.array(b.ty("ARR_A"), 3_u,
                                       utils::Vector{
                                           b.Stride(16),
                                       }),
                            b.IndexAccessor(                 //
                                b.MemberAccessor("s", "a"),  //
                                3_i))),
               b.Decl(b.Let("c", b.ty("ARR_A"),
                            b.IndexAccessor(                     //
                                b.IndexAccessor(                 //
                                    b.MemberAccessor("s", "a"),  //
                                    3_i),
                                2_i))),
               b.Decl(b.Let("d", b.ty.f32(),
                            b.IndexAccessor(                         //
                                b.IndexAccessor(                     //
                                    b.IndexAccessor(                 //
                                        b.MemberAccessor("s", "a"),  //
                                        3_i),
                                    2_i),
                                1_i))),
               b.Assign(b.MemberAccessor("s", "a"), b.Call("ARR_B")),
               b.Assign(b.IndexAccessor(                         //
                            b.IndexAccessor(                     //
                                b.IndexAccessor(                 //
                                    b.MemberAccessor("s", "a"),  //
                                    3_i),
                                2_i),
                            1_i),
                        5_f),
           },
           utils::Vector{
               b.Stage(ast::PipelineStage::kCompute),
               b.WorkgroupSize(1_i),
           });

    auto* expect =
        R"(
struct strided_arr {
  @size(8)
  el : f32,
}

alias ARR_A = array<strided_arr, 2u>;

struct strided_arr_1 {
  @size(128)
  el : array<ARR_A, 3u>,
}

alias ARR_B = array<strided_arr_1, 4u>;

struct S {
  a : ARR_B,
}

@group(0) @binding(0) var<storage, read_write> s : S;

@compute @workgroup_size(1i)
fn f() {
  let a : ARR_B = s.a;
  let b : array<ARR_A, 3u> = s.a[3i].el;
  let c : ARR_A = s.a[3i].el[2i];
  let d : f32 = s.a[3i].el[2i][1i].el;
  s.a = ARR_B();
  s.a[3i].el[2i][1i].el = 5.0f;
}
)";

    auto got = Run<Unshadow, SimplifyPointers, DecomposeStridedArray>(Program(std::move(b)));

    EXPECT_EQ(expect, str(got));
}
}  // namespace
}  // namespace tint::transform
