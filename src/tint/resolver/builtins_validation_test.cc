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

#include "src/tint/ast/call_statement.h"
#include "src/tint/builtin/builtin_value.h"
#include "src/tint/resolver/resolver_test_helper.h"
#include "src/tint/utils/string_stream.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::resolver {
namespace {

template <typename T>
using DataType = builder::DataType<T>;
template <typename T>
using vec2 = builder::vec2<T>;
template <typename T>
using vec3 = builder::vec3<T>;
template <typename T>
using vec4 = builder::vec4<T>;

class ResolverBuiltinsValidationTest : public resolver::TestHelper, public testing::Test {};
namespace StageTest {
struct Params {
    builder::ast_type_func_ptr type;
    builtin::BuiltinValue builtin;
    ast::PipelineStage stage;
    bool is_valid;
};

template <typename T>
constexpr Params ParamsFor(builtin::BuiltinValue builtin, ast::PipelineStage stage, bool is_valid) {
    return Params{DataType<T>::AST, builtin, stage, is_valid};
}
static constexpr Params cases[] = {
    ParamsFor<vec4<f32>>(builtin::BuiltinValue::kPosition, ast::PipelineStage::kVertex, false),
    ParamsFor<vec4<f32>>(builtin::BuiltinValue::kPosition, ast::PipelineStage::kFragment, true),
    ParamsFor<vec4<f32>>(builtin::BuiltinValue::kPosition, ast::PipelineStage::kCompute, false),

    ParamsFor<u32>(builtin::BuiltinValue::kVertexIndex, ast::PipelineStage::kVertex, true),
    ParamsFor<u32>(builtin::BuiltinValue::kVertexIndex, ast::PipelineStage::kFragment, false),
    ParamsFor<u32>(builtin::BuiltinValue::kVertexIndex, ast::PipelineStage::kCompute, false),

    ParamsFor<u32>(builtin::BuiltinValue::kInstanceIndex, ast::PipelineStage::kVertex, true),
    ParamsFor<u32>(builtin::BuiltinValue::kInstanceIndex, ast::PipelineStage::kFragment, false),
    ParamsFor<u32>(builtin::BuiltinValue::kInstanceIndex, ast::PipelineStage::kCompute, false),

    ParamsFor<bool>(builtin::BuiltinValue::kFrontFacing, ast::PipelineStage::kVertex, false),
    ParamsFor<bool>(builtin::BuiltinValue::kFrontFacing, ast::PipelineStage::kFragment, true),
    ParamsFor<bool>(builtin::BuiltinValue::kFrontFacing, ast::PipelineStage::kCompute, false),

    ParamsFor<vec3<u32>>(builtin::BuiltinValue::kLocalInvocationId,
                         ast::PipelineStage::kVertex,
                         false),
    ParamsFor<vec3<u32>>(builtin::BuiltinValue::kLocalInvocationId,
                         ast::PipelineStage::kFragment,
                         false),
    ParamsFor<vec3<u32>>(builtin::BuiltinValue::kLocalInvocationId,
                         ast::PipelineStage::kCompute,
                         true),

    ParamsFor<u32>(builtin::BuiltinValue::kLocalInvocationIndex,
                   ast::PipelineStage::kVertex,
                   false),
    ParamsFor<u32>(builtin::BuiltinValue::kLocalInvocationIndex,
                   ast::PipelineStage::kFragment,
                   false),
    ParamsFor<u32>(builtin::BuiltinValue::kLocalInvocationIndex,
                   ast::PipelineStage::kCompute,
                   true),

    ParamsFor<vec3<u32>>(builtin::BuiltinValue::kGlobalInvocationId,
                         ast::PipelineStage::kVertex,
                         false),
    ParamsFor<vec3<u32>>(builtin::BuiltinValue::kGlobalInvocationId,
                         ast::PipelineStage::kFragment,
                         false),
    ParamsFor<vec3<u32>>(builtin::BuiltinValue::kGlobalInvocationId,
                         ast::PipelineStage::kCompute,
                         true),

    ParamsFor<vec3<u32>>(builtin::BuiltinValue::kWorkgroupId, ast::PipelineStage::kVertex, false),
    ParamsFor<vec3<u32>>(builtin::BuiltinValue::kWorkgroupId, ast::PipelineStage::kFragment, false),
    ParamsFor<vec3<u32>>(builtin::BuiltinValue::kWorkgroupId, ast::PipelineStage::kCompute, true),

    ParamsFor<vec3<u32>>(builtin::BuiltinValue::kNumWorkgroups, ast::PipelineStage::kVertex, false),
    ParamsFor<vec3<u32>>(builtin::BuiltinValue::kNumWorkgroups,
                         ast::PipelineStage::kFragment,
                         false),
    ParamsFor<vec3<u32>>(builtin::BuiltinValue::kNumWorkgroups, ast::PipelineStage::kCompute, true),

    ParamsFor<u32>(builtin::BuiltinValue::kSampleIndex, ast::PipelineStage::kVertex, false),
    ParamsFor<u32>(builtin::BuiltinValue::kSampleIndex, ast::PipelineStage::kFragment, true),
    ParamsFor<u32>(builtin::BuiltinValue::kSampleIndex, ast::PipelineStage::kCompute, false),

    ParamsFor<u32>(builtin::BuiltinValue::kSampleMask, ast::PipelineStage::kVertex, false),
    ParamsFor<u32>(builtin::BuiltinValue::kSampleMask, ast::PipelineStage::kFragment, true),
    ParamsFor<u32>(builtin::BuiltinValue::kSampleMask, ast::PipelineStage::kCompute, false),
};

using ResolverBuiltinsStageTest = ResolverTestWithParam<Params>;
TEST_P(ResolverBuiltinsStageTest, All_input) {
    const Params& params = GetParam();

    auto* p = GlobalVar("p", ty.vec4<f32>(), builtin::AddressSpace::kPrivate);
    auto* input = Param("input", params.type(*this),
                        utils::Vector{Builtin(Source{{12, 34}}, params.builtin)});
    switch (params.stage) {
        case ast::PipelineStage::kVertex:
            Func("main", utils::Vector{input}, ty.vec4<f32>(), utils::Vector{Return(p)},
                 utils::Vector{Stage(ast::PipelineStage::kVertex)},
                 utils::Vector{
                     Builtin(Source{{12, 34}}, builtin::BuiltinValue::kPosition),
                 });
            break;
        case ast::PipelineStage::kFragment:
            Func("main", utils::Vector{input}, ty.void_(), utils::Empty,
                 utils::Vector{
                     Stage(ast::PipelineStage::kFragment),
                 },
                 {});
            break;
        case ast::PipelineStage::kCompute:
            Func("main", utils::Vector{input}, ty.void_(), utils::Empty,
                 utils::Vector{
                     Stage(ast::PipelineStage::kCompute),
                     WorkgroupSize(1_i),
                 });
            break;
        default:
            break;
    }

    if (params.is_valid) {
        EXPECT_TRUE(r()->Resolve()) << r()->error();
    } else {
        utils::StringStream err;
        err << "12:34 error: @builtin(" << params.builtin << ")";
        err << " cannot be used in input of " << params.stage << " pipeline stage";
        EXPECT_FALSE(r()->Resolve());
        EXPECT_EQ(r()->error(), err.str());
    }
}
INSTANTIATE_TEST_SUITE_P(ResolverBuiltinsValidationTest,
                         ResolverBuiltinsStageTest,
                         testing::ValuesIn(cases));

TEST_F(ResolverBuiltinsValidationTest, FragDepthIsInput_Fail) {
    // @fragment
    // fn fs_main(
    //   @builtin(frag_depth) fd: f32,
    // ) -> @location(0) f32 { return 1.0; }
    Func("fs_main",
         utils::Vector{
             Param("fd", ty.f32(),
                   utils::Vector{
                       Builtin(Source{{12, 34}}, builtin::BuiltinValue::kFragDepth),
                   }),
         },
         ty.f32(),
         utils::Vector{
             Return(1_f),
         },
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         },
         utils::Vector{
             Location(0_a),
         });
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        "12:34 error: @builtin(frag_depth) cannot be used in input of fragment pipeline stage");
}

TEST_F(ResolverBuiltinsValidationTest, FragDepthIsInputStruct_Fail) {
    // struct MyInputs {
    //   @builtin(frag_depth) ff: f32;
    // };
    // @fragment
    // fn fragShader(arg: MyInputs) -> @location(0) f32 { return 1.0; }

    auto* s = Structure("MyInputs",
                        utils::Vector{
                            Member("frag_depth", ty.f32(),
                                   utils::Vector{
                                       Builtin(Source{{12, 34}}, builtin::BuiltinValue::kFragDepth),
                                   }),
                        });

    Func("fragShader",
         utils::Vector{
             Param("arg", ty.Of(s)),
         },
         ty.f32(),
         utils::Vector{
             Return(1_f),
         },
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         },
         utils::Vector{
             Location(0_a),
         });
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: @builtin(frag_depth) cannot be used in input of "
              "fragment pipeline stage\n"
              "note: while analyzing entry point 'fragShader'");
}

TEST_F(ResolverBuiltinsValidationTest, StructBuiltinInsideEntryPoint_Ignored) {
    // struct S {
    //   @builtin(vertex_index) idx: u32;
    // };
    // @fragment
    // fn fragShader() { var s : S; }

    Structure("S", utils::Vector{
                       Member("idx", ty.u32(),
                              utils::Vector{
                                  Builtin(builtin::BuiltinValue::kVertexIndex),
                              }),
                   });

    Func("fragShader", utils::Empty, ty.void_(), utils::Vector{Decl(Var("s", ty("S")))},
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         });
    EXPECT_TRUE(r()->Resolve());
}

}  // namespace StageTest

TEST_F(ResolverBuiltinsValidationTest, PositionNotF32_Struct_Fail) {
    // struct MyInputs {
    //   @builtin(kPosition) p: vec4<u32>;
    // };
    // @fragment
    // fn fragShader(is_front: MyInputs) -> @location(0) f32 { return 1.0; }

    auto* s = Structure("MyInputs",
                        utils::Vector{
                            Member("position", ty.vec4<u32>(),
                                   utils::Vector{
                                       Builtin(Source{{12, 34}}, builtin::BuiltinValue::kPosition),
                                   }),
                        });
    Func("fragShader",
         utils::Vector{
             Param("arg", ty.Of(s)),
         },
         ty.f32(),
         utils::Vector{
             Return(1_f),
         },
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         },
         utils::Vector{
             Location(0_a),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: store type of @builtin(position) must be 'vec4<f32>'");
}

TEST_F(ResolverBuiltinsValidationTest, PositionNotF32_ReturnType_Fail) {
    // @vertex
    // fn main() -> @builtin(position) f32 { return 1.0; }
    Func("main", utils::Empty, ty.f32(),
         utils::Vector{
             Return(1_f),
         },
         utils::Vector{Stage(ast::PipelineStage::kVertex)},
         utils::Vector{
             Builtin(Source{{12, 34}}, builtin::BuiltinValue::kPosition),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: store type of @builtin(position) must be 'vec4<f32>'");
}

TEST_F(ResolverBuiltinsValidationTest, FragDepthNotF32_Struct_Fail) {
    // struct MyInputs {
    //   @builtin(kFragDepth) p: i32;
    // };
    // @fragment
    // fn fragShader(is_front: MyInputs) -> @location(0) f32 { return 1.0; }

    auto* s = Structure("MyInputs",
                        utils::Vector{
                            Member("frag_depth", ty.i32(),
                                   utils::Vector{
                                       Builtin(Source{{12, 34}}, builtin::BuiltinValue::kFragDepth),
                                   }),
                        });
    Func("fragShader", utils::Vector{Param("arg", ty.Of(s))}, ty.f32(),
         utils::Vector{
             Return(1_f),
         },
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         },
         utils::Vector{
             Location(0_a),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: store type of @builtin(frag_depth) must be 'f32'");
}

TEST_F(ResolverBuiltinsValidationTest, SampleMaskNotU32_Struct_Fail) {
    // struct MyInputs {
    //   @builtin(sample_mask) m: f32;
    // };
    // @fragment
    // fn fragShader(is_front: MyInputs) -> @location(0) f32 { return 1.0; }

    auto* s = Structure(
        "MyInputs", utils::Vector{
                        Member("m", ty.f32(),
                               utils::Vector{
                                   Builtin(Source{{12, 34}}, builtin::BuiltinValue::kSampleMask),
                               }),
                    });
    Func("fragShader", utils::Vector{Param("arg", ty.Of(s))}, ty.f32(),
         utils::Vector{
             Return(1_f),
         },
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         },
         utils::Vector{
             Location(0_a),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: store type of @builtin(sample_mask) must be 'u32'");
}

TEST_F(ResolverBuiltinsValidationTest, SampleMaskNotU32_ReturnType_Fail) {
    // @fragment
    // fn main() -> @builtin(sample_mask) i32 { return 1; }
    Func("main", utils::Empty, ty.i32(), utils::Vector{Return(1_i)},
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         },
         utils::Vector{
             Builtin(Source{{12, 34}}, builtin::BuiltinValue::kSampleMask),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: store type of @builtin(sample_mask) must be 'u32'");
}

TEST_F(ResolverBuiltinsValidationTest, SampleMaskIsNotU32_Fail) {
    // @fragment
    // fn fs_main(
    //   @builtin(sample_mask) arg: bool
    // ) -> @location(0) f32 { return 1.0; }
    Func("fs_main",
         utils::Vector{
             Param("arg", ty.bool_(),
                   utils::Vector{
                       Builtin(Source{{12, 34}}, builtin::BuiltinValue::kSampleMask),
                   }),
         },
         ty.f32(),
         utils::Vector{
             Return(1_f),
         },
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         },
         utils::Vector{
             Location(0_a),
         });
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: store type of @builtin(sample_mask) must be 'u32'");
}

TEST_F(ResolverBuiltinsValidationTest, SampleIndexIsNotU32_Struct_Fail) {
    // struct MyInputs {
    //   @builtin(sample_index) m: f32;
    // };
    // @fragment
    // fn fragShader(is_front: MyInputs) -> @location(0) f32 { return 1.0; }

    auto* s = Structure(
        "MyInputs", utils::Vector{
                        Member("m", ty.f32(),
                               utils::Vector{
                                   Builtin(Source{{12, 34}}, builtin::BuiltinValue::kSampleIndex),
                               }),
                    });
    Func("fragShader", utils::Vector{Param("arg", ty.Of(s))}, ty.f32(),
         utils::Vector{
             Return(1_f),
         },
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         },
         utils::Vector{
             Location(0_a),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: store type of @builtin(sample_index) must be 'u32'");
}

TEST_F(ResolverBuiltinsValidationTest, SampleIndexIsNotU32_Fail) {
    // @fragment
    // fn fs_main(
    //   @builtin(sample_index) arg: bool
    // ) -> @location(0) f32 { return 1.0; }
    Func("fs_main",
         utils::Vector{
             Param("arg", ty.bool_(),
                   utils::Vector{
                       Builtin(Source{{12, 34}}, builtin::BuiltinValue::kSampleIndex),
                   }),
         },
         ty.f32(),
         utils::Vector{
             Return(1_f),
         },
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         },
         utils::Vector{
             Location(0_a),
         });
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: store type of @builtin(sample_index) must be 'u32'");
}

TEST_F(ResolverBuiltinsValidationTest, PositionIsNotF32_Fail) {
    // @fragment
    // fn fs_main(
    //   @builtin(kPosition) p: vec3<f32>,
    // ) -> @location(0) f32 { return 1.0; }
    Func("fs_main",
         utils::Vector{
             Param("p", ty.vec3<f32>(),
                   utils::Vector{
                       Builtin(Source{{12, 34}}, builtin::BuiltinValue::kPosition),
                   }),
         },
         ty.f32(),
         utils::Vector{
             Return(1_f),
         },
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         },
         utils::Vector{
             Location(0_a),
         });
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: store type of @builtin(position) must be 'vec4<f32>'");
}

TEST_F(ResolverBuiltinsValidationTest, FragDepthIsNotF32_Fail) {
    // @fragment
    // fn fs_main() -> @builtin(kFragDepth) f32 { var fd: i32; return fd; }
    auto* fd = Var("fd", ty.i32());
    Func("fs_main", utils::Empty, ty.i32(),
         utils::Vector{
             Decl(fd),
             Return(fd),
         },
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         },
         utils::Vector{
             Builtin(Source{{12, 34}}, builtin::BuiltinValue::kFragDepth),
         });
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: store type of @builtin(frag_depth) must be 'f32'");
}

TEST_F(ResolverBuiltinsValidationTest, VertexIndexIsNotU32_Fail) {
    // @vertex
    // fn main(
    //   @builtin(kVertexIndex) vi : f32,
    //   @builtin(kPosition) p :vec4<f32>
    // ) -> @builtin(kPosition) vec4<f32> { return vec4<f32>(); }
    auto* p = Param("p", ty.vec4<f32>(),
                    utils::Vector{
                        Builtin(builtin::BuiltinValue::kPosition),
                    });
    auto* vi = Param("vi", ty.f32(),
                     utils::Vector{
                         Builtin(Source{{12, 34}}, builtin::BuiltinValue::kVertexIndex),
                     });
    Func("main", utils::Vector{vi, p}, ty.vec4<f32>(), utils::Vector{Return(Expr("p"))},
         utils::Vector{
             Stage(ast::PipelineStage::kVertex),
         },
         utils::Vector{
             Builtin(builtin::BuiltinValue::kPosition),
         });
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: store type of @builtin(vertex_index) must be 'u32'");
}

TEST_F(ResolverBuiltinsValidationTest, InstanceIndexIsNotU32) {
    // @vertex
    // fn main(
    //   @builtin(kInstanceIndex) ii : f32,
    //   @builtin(kPosition) p :vec4<f32>
    // ) -> @builtin(kPosition) vec4<f32> { return vec4<f32>(); }
    auto* p = Param("p", ty.vec4<f32>(),
                    utils::Vector{
                        Builtin(builtin::BuiltinValue::kPosition),
                    });
    auto* ii = Param("ii", ty.f32(),
                     utils::Vector{
                         Builtin(Source{{12, 34}}, builtin::BuiltinValue::kInstanceIndex),
                     });
    Func("main", utils::Vector{ii, p}, ty.vec4<f32>(), utils::Vector{Return(Expr("p"))},
         utils::Vector{
             Stage(ast::PipelineStage::kVertex),
         },
         utils::Vector{
             Builtin(builtin::BuiltinValue::kPosition),
         });
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: store type of @builtin(instance_index) must be 'u32'");
}

TEST_F(ResolverBuiltinsValidationTest, FragmentBuiltin_Pass) {
    // @fragment
    // fn fs_main(
    //   @builtin(kPosition) p: vec4<f32>,
    //   @builtin(front_facing) ff: bool,
    //   @builtin(sample_index) si: u32,
    //   @builtin(sample_mask) sm : u32
    // ) -> @builtin(frag_depth) f32 { var fd: f32; return fd; }
    auto* p = Param("p", ty.vec4<f32>(),
                    utils::Vector{
                        Builtin(builtin::BuiltinValue::kPosition),
                    });
    auto* ff = Param("ff", ty.bool_(),
                     utils::Vector{
                         Builtin(builtin::BuiltinValue::kFrontFacing),
                     });
    auto* si = Param("si", ty.u32(),
                     utils::Vector{
                         Builtin(builtin::BuiltinValue::kSampleIndex),
                     });
    auto* sm = Param("sm", ty.u32(),
                     utils::Vector{
                         Builtin(builtin::BuiltinValue::kSampleMask),
                     });
    auto* var_fd = Var("fd", ty.f32());
    Func("fs_main", utils::Vector{p, ff, si, sm}, ty.f32(),
         utils::Vector{
             Decl(var_fd),
             Return(var_fd),
         },
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         },
         utils::Vector{
             Builtin(builtin::BuiltinValue::kFragDepth),
         });
    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBuiltinsValidationTest, VertexBuiltin_Pass) {
    // @vertex
    // fn main(
    //   @builtin(vertex_index) vi : u32,
    //   @builtin(instance_index) ii : u32,
    // ) -> @builtin(position) vec4<f32> { var p :vec4<f32>; return p; }
    auto* vi = Param("vi", ty.u32(),
                     utils::Vector{
                         Builtin(Source{{12, 34}}, builtin::BuiltinValue::kVertexIndex),
                     });

    auto* ii = Param("ii", ty.u32(),
                     utils::Vector{
                         Builtin(Source{{12, 34}}, builtin::BuiltinValue::kInstanceIndex),
                     });
    auto* p = Var("p", ty.vec4<f32>());
    Func("main", utils::Vector{vi, ii}, ty.vec4<f32>(),
         utils::Vector{
             Decl(p),
             Return(p),
         },
         utils::Vector{Stage(ast::PipelineStage::kVertex)},
         utils::Vector{
             Builtin(builtin::BuiltinValue::kPosition),
         });

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBuiltinsValidationTest, ComputeBuiltin_Pass) {
    // @compute @workgroup_size(1)
    // fn main(
    //   @builtin(local_invocationId) li_id: vec3<u32>,
    //   @builtin(local_invocationIndex) li_index: u32,
    //   @builtin(global_invocationId) gi: vec3<u32>,
    //   @builtin(workgroup_id) wi: vec3<u32>,
    //   @builtin(num_workgroups) nwgs: vec3<u32>,
    // ) {}

    auto* li_id = Param("li_id", ty.vec3<u32>(),
                        utils::Vector{
                            Builtin(builtin::BuiltinValue::kLocalInvocationId),
                        });
    auto* li_index = Param("li_index", ty.u32(),
                           utils::Vector{
                               Builtin(builtin::BuiltinValue::kLocalInvocationIndex),
                           });
    auto* gi = Param("gi", ty.vec3<u32>(),
                     utils::Vector{
                         Builtin(builtin::BuiltinValue::kGlobalInvocationId),
                     });
    auto* wi = Param("wi", ty.vec3<u32>(),
                     utils::Vector{
                         Builtin(builtin::BuiltinValue::kWorkgroupId),
                     });
    auto* nwgs = Param("nwgs", ty.vec3<u32>(),
                       utils::Vector{
                           Builtin(builtin::BuiltinValue::kNumWorkgroups),
                       });

    Func("main", utils::Vector{li_id, li_index, gi, wi, nwgs}, ty.void_(), utils::Empty,
         utils::Vector{Stage(ast::PipelineStage::kCompute),
                       WorkgroupSize(Expr(Source{Source::Location{12, 34}}, 2_i))});

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBuiltinsValidationTest, ComputeBuiltin_WorkGroupIdNotVec3U32) {
    auto* wi = Param("wi", ty.f32(),
                     utils::Vector{
                         Builtin(Source{{12, 34}}, builtin::BuiltinValue::kWorkgroupId),
                     });
    Func("main", utils::Vector{wi}, ty.void_(), utils::Empty,
         utils::Vector{Stage(ast::PipelineStage::kCompute),
                       WorkgroupSize(Expr(Source{Source::Location{12, 34}}, 2_i))});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: store type of @builtin(workgroup_id) must be "
              "'vec3<u32>'");
}

TEST_F(ResolverBuiltinsValidationTest, ComputeBuiltin_NumWorkgroupsNotVec3U32) {
    auto* nwgs = Param("nwgs", ty.f32(),
                       utils::Vector{
                           Builtin(Source{{12, 34}}, builtin::BuiltinValue::kNumWorkgroups),
                       });
    Func("main", utils::Vector{nwgs}, ty.void_(), utils::Empty,
         utils::Vector{Stage(ast::PipelineStage::kCompute),
                       WorkgroupSize(Expr(Source{Source::Location{12, 34}}, 2_i))});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: store type of @builtin(num_workgroups) must be "
              "'vec3<u32>'");
}

TEST_F(ResolverBuiltinsValidationTest, ComputeBuiltin_GlobalInvocationNotVec3U32) {
    auto* gi = Param("gi", ty.vec3<i32>(),
                     utils::Vector{
                         Builtin(Source{{12, 34}}, builtin::BuiltinValue::kGlobalInvocationId),
                     });
    Func("main", utils::Vector{gi}, ty.void_(), utils::Empty,
         utils::Vector{Stage(ast::PipelineStage::kCompute),
                       WorkgroupSize(Expr(Source{Source::Location{12, 34}}, 2_i))});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: store type of @builtin(global_invocation_id) must be "
              "'vec3<u32>'");
}

TEST_F(ResolverBuiltinsValidationTest, ComputeBuiltin_LocalInvocationIndexNotU32) {
    auto* li_index =
        Param("li_index", ty.vec3<u32>(),
              utils::Vector{
                  Builtin(Source{{12, 34}}, builtin::BuiltinValue::kLocalInvocationIndex),
              });
    Func("main", utils::Vector{li_index}, ty.void_(), utils::Empty,
         utils::Vector{Stage(ast::PipelineStage::kCompute),
                       WorkgroupSize(Expr(Source{Source::Location{12, 34}}, 2_i))});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: store type of @builtin(local_invocation_index) must be "
              "'u32'");
}

TEST_F(ResolverBuiltinsValidationTest, ComputeBuiltin_LocalInvocationNotVec3U32) {
    auto* li_id = Param("li_id", ty.vec2<u32>(),
                        utils::Vector{
                            Builtin(Source{{12, 34}}, builtin::BuiltinValue::kLocalInvocationId),
                        });
    Func("main", utils::Vector{li_id}, ty.void_(), utils::Empty,
         utils::Vector{Stage(ast::PipelineStage::kCompute),
                       WorkgroupSize(Expr(Source{Source::Location{12, 34}}, 2_i))});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: store type of @builtin(local_invocation_id) must be "
              "'vec3<u32>'");
}

TEST_F(ResolverBuiltinsValidationTest, FragmentBuiltinStruct_Pass) {
    // Struct MyInputs {
    //   @builtin(kPosition) p: vec4<f32>;
    //   @builtin(frag_depth) fd: f32;
    //   @builtin(sample_index) si: u32;
    //   @builtin(sample_mask) sm : u32;;
    // };
    // @fragment
    // fn fragShader(arg: MyInputs) -> @location(0) f32 { return 1.0; }

    auto* s = Structure("MyInputs", utils::Vector{
                                        Member("position", ty.vec4<f32>(),
                                               utils::Vector{
                                                   Builtin(builtin::BuiltinValue::kPosition),
                                               }),
                                        Member("front_facing", ty.bool_(),
                                               utils::Vector{
                                                   Builtin(builtin::BuiltinValue::kFrontFacing),
                                               }),
                                        Member("sample_index", ty.u32(),
                                               utils::Vector{
                                                   Builtin(builtin::BuiltinValue::kSampleIndex),
                                               }),
                                        Member("sample_mask", ty.u32(),
                                               utils::Vector{
                                                   Builtin(builtin::BuiltinValue::kSampleMask),
                                               }),
                                    });
    Func("fragShader", utils::Vector{Param("arg", ty.Of(s))}, ty.f32(),
         utils::Vector{
             Return(1_f),
         },
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         },
         utils::Vector{
             Location(0_a),
         });
    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBuiltinsValidationTest, FrontFacingParamIsNotBool_Fail) {
    // @fragment
    // fn fs_main(
    //   @builtin(front_facing) is_front: i32;
    // ) -> @location(0) f32 { return 1.0; }

    auto* is_front = Param("is_front", ty.i32(),
                           utils::Vector{
                               Builtin(Source{{12, 34}}, builtin::BuiltinValue::kFrontFacing),
                           });
    Func("fs_main", utils::Vector{is_front}, ty.f32(),
         utils::Vector{
             Return(1_f),
         },
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         },
         utils::Vector{
             Location(0_a),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: store type of @builtin(front_facing) must be 'bool'");
}

TEST_F(ResolverBuiltinsValidationTest, FrontFacingMemberIsNotBool_Fail) {
    // struct MyInputs {
    //   @builtin(front_facing) pos: f32;
    // };
    // @fragment
    // fn fragShader(is_front: MyInputs) -> @location(0) f32 { return 1.0; }

    auto* s = Structure(
        "MyInputs", utils::Vector{
                        Member("pos", ty.f32(),
                               utils::Vector{
                                   Builtin(Source{{12, 34}}, builtin::BuiltinValue::kFrontFacing),
                               }),
                    });
    Func("fragShader", utils::Vector{Param("is_front", ty.Of(s))}, ty.f32(),
         utils::Vector{
             Return(1_f),
         },
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         },
         utils::Vector{
             Location(0_a),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: store type of @builtin(front_facing) must be 'bool'");
}

// TODO(crbug.com/tint/1846): This isn't a validation test, but this sits next to other @builtin
// tests. Clean this up.
TEST_F(ResolverBuiltinsValidationTest, StructMemberAttributeMapsToSemBuiltinEnum) {
    // struct S {
    //   @builtin(front_facing) b : bool;
    // };
    // @fragment
    // fn f(s : S) {}

    auto* builtin = Builtin(builtin::BuiltinValue::kFrontFacing);
    auto* s = Structure("S", utils::Vector{
                                 Member("f", ty.bool_(), utils::Vector{builtin}),
                             });
    Func("f", utils::Vector{Param("b", ty.Of(s))}, ty.void_(), utils::Empty,
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* builtin_expr = Sem().Get(builtin);
    ASSERT_NE(builtin_expr, nullptr);
    EXPECT_EQ(builtin_expr->Value(), builtin::BuiltinValue::kFrontFacing);
}

// TODO(crbug.com/tint/1846): This isn't a validation test, but this sits next to other @builtin
// tests. Clean this up.
TEST_F(ResolverBuiltinsValidationTest, ParamAttributeMapsToSemBuiltinEnum) {
    // @fragment
    // fn f(@builtin(front_facing) b : bool) {}

    auto* builtin = Builtin(builtin::BuiltinValue::kFrontFacing);
    Func("f", utils::Vector{Param("b", ty.bool_(), utils::Vector{builtin})}, ty.void_(),
         utils::Empty,
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* builtin_expr = Sem().Get(builtin);
    ASSERT_NE(builtin_expr, nullptr);
    EXPECT_EQ(builtin_expr->Value(), builtin::BuiltinValue::kFrontFacing);
}

TEST_F(ResolverBuiltinsValidationTest, Length_Float_Scalar) {
    auto* builtin = Call("length", 1_f);
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBuiltinsValidationTest, Length_Float_Vec2) {
    auto* builtin = Call("length", vec2<f32>(1_f, 1_f));
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBuiltinsValidationTest, Length_Float_Vec3) {
    auto* builtin = Call("length", vec3<f32>(1_f, 1_f, 1_f));
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBuiltinsValidationTest, Length_Float_Vec4) {
    auto* builtin = Call("length", vec4<f32>(1_f, 1_f, 1_f, 1_f));
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBuiltinsValidationTest, Distance_Float_Scalar) {
    auto* builtin = Call("distance", 1_f, 1_f);
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBuiltinsValidationTest, Distance_Float_Vec2) {
    auto* builtin = Call("distance", vec2<f32>(1_f, 1_f), vec2<f32>(1_f, 1_f));
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBuiltinsValidationTest, Distance_Float_Vec3) {
    auto* builtin = Call("distance", vec3<f32>(1_f, 1_f, 1_f), vec3<f32>(1_f, 1_f, 1_f));
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBuiltinsValidationTest, Distance_Float_Vec4) {
    auto* builtin = Call("distance", vec4<f32>(1_f, 1_f, 1_f, 1_f), vec4<f32>(1_f, 1_f, 1_f, 1_f));
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBuiltinsValidationTest, Determinant_Mat2x2) {
    auto* builtin = Call("determinant", mat2x2<f32>(vec2<f32>(1_f, 1_f), vec2<f32>(1_f, 1_f)));
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBuiltinsValidationTest, Determinant_Mat3x3) {
    auto* builtin = Call(
        "determinant",
        mat3x3<f32>(vec3<f32>(1_f, 1_f, 1_f), vec3<f32>(1_f, 1_f, 1_f), vec3<f32>(1_f, 1_f, 1_f)));
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBuiltinsValidationTest, Determinant_Mat4x4) {
    auto* builtin = Call("determinant",
                         mat4x4<f32>(vec4<f32>(1_f, 1_f, 1_f, 1_f), vec4<f32>(1_f, 1_f, 1_f, 1_f),
                                     vec4<f32>(1_f, 1_f, 1_f, 1_f), vec4<f32>(1_f, 1_f, 1_f, 1_f)));
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBuiltinsValidationTest, Frexp_Scalar) {
    auto* builtin = Call("frexp", 1_f);
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* res_ty = TypeOf(builtin)->As<type::Struct>();
    ASSERT_TRUE(res_ty != nullptr);
    auto members = res_ty->Members();
    ASSERT_EQ(members.Length(), 2u);
    EXPECT_TRUE(members[0]->Type()->Is<type::F32>());
    EXPECT_TRUE(members[1]->Type()->Is<type::I32>());
}

TEST_F(ResolverBuiltinsValidationTest, Frexp_Vec2) {
    auto* builtin = Call("frexp", vec2<f32>(1_f, 1_f));
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* res_ty = TypeOf(builtin)->As<type::Struct>();
    ASSERT_TRUE(res_ty != nullptr);
    auto members = res_ty->Members();
    ASSERT_EQ(members.Length(), 2u);
    ASSERT_TRUE(members[0]->Type()->Is<type::Vector>());
    ASSERT_TRUE(members[1]->Type()->Is<type::Vector>());
    EXPECT_EQ(members[0]->Type()->As<type::Vector>()->Width(), 2u);
    EXPECT_TRUE(members[0]->Type()->As<type::Vector>()->type()->Is<type::F32>());
    EXPECT_EQ(members[1]->Type()->As<type::Vector>()->Width(), 2u);
    EXPECT_TRUE(members[1]->Type()->As<type::Vector>()->type()->Is<type::I32>());
}

TEST_F(ResolverBuiltinsValidationTest, Frexp_Vec3) {
    auto* builtin = Call("frexp", vec3<f32>(1_f, 1_f, 1_f));
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* res_ty = TypeOf(builtin)->As<type::Struct>();
    ASSERT_TRUE(res_ty != nullptr);
    auto members = res_ty->Members();
    ASSERT_EQ(members.Length(), 2u);
    ASSERT_TRUE(members[0]->Type()->Is<type::Vector>());
    ASSERT_TRUE(members[1]->Type()->Is<type::Vector>());
    EXPECT_EQ(members[0]->Type()->As<type::Vector>()->Width(), 3u);
    EXPECT_TRUE(members[0]->Type()->As<type::Vector>()->type()->Is<type::F32>());
    EXPECT_EQ(members[1]->Type()->As<type::Vector>()->Width(), 3u);
    EXPECT_TRUE(members[1]->Type()->As<type::Vector>()->type()->Is<type::I32>());
}

TEST_F(ResolverBuiltinsValidationTest, Frexp_Vec4) {
    auto* builtin = Call("frexp", vec4<f32>(1_f, 1_f, 1_f, 1_f));
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* res_ty = TypeOf(builtin)->As<type::Struct>();
    ASSERT_TRUE(res_ty != nullptr);
    auto members = res_ty->Members();
    ASSERT_EQ(members.Length(), 2u);
    ASSERT_TRUE(members[0]->Type()->Is<type::Vector>());
    ASSERT_TRUE(members[1]->Type()->Is<type::Vector>());
    EXPECT_EQ(members[0]->Type()->As<type::Vector>()->Width(), 4u);
    EXPECT_TRUE(members[0]->Type()->As<type::Vector>()->type()->Is<type::F32>());
    EXPECT_EQ(members[1]->Type()->As<type::Vector>()->Width(), 4u);
    EXPECT_TRUE(members[1]->Type()->As<type::Vector>()->type()->Is<type::I32>());
}

TEST_F(ResolverBuiltinsValidationTest, Modf_Scalar) {
    auto* builtin = Call("modf", 1_f);
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* res_ty = TypeOf(builtin)->As<type::Struct>();
    ASSERT_TRUE(res_ty != nullptr);
    auto members = res_ty->Members();
    ASSERT_EQ(members.Length(), 2u);
    EXPECT_TRUE(members[0]->Type()->Is<type::F32>());
    EXPECT_TRUE(members[1]->Type()->Is<type::F32>());
}

TEST_F(ResolverBuiltinsValidationTest, Modf_Vec2) {
    auto* builtin = Call("modf", vec2<f32>(1_f, 1_f));
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* res_ty = TypeOf(builtin)->As<type::Struct>();
    ASSERT_TRUE(res_ty != nullptr);
    auto members = res_ty->Members();
    ASSERT_EQ(members.Length(), 2u);
    ASSERT_TRUE(members[0]->Type()->Is<type::Vector>());
    ASSERT_TRUE(members[1]->Type()->Is<type::Vector>());
    EXPECT_EQ(members[0]->Type()->As<type::Vector>()->Width(), 2u);
    EXPECT_TRUE(members[0]->Type()->As<type::Vector>()->type()->Is<type::F32>());
    EXPECT_EQ(members[1]->Type()->As<type::Vector>()->Width(), 2u);
    EXPECT_TRUE(members[1]->Type()->As<type::Vector>()->type()->Is<type::F32>());
}

TEST_F(ResolverBuiltinsValidationTest, Modf_Vec3) {
    auto* builtin = Call("modf", vec3<f32>(1_f, 1_f, 1_f));
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* res_ty = TypeOf(builtin)->As<type::Struct>();
    ASSERT_TRUE(res_ty != nullptr);
    auto members = res_ty->Members();
    ASSERT_EQ(members.Length(), 2u);
    ASSERT_TRUE(members[0]->Type()->Is<type::Vector>());
    ASSERT_TRUE(members[1]->Type()->Is<type::Vector>());
    EXPECT_EQ(members[0]->Type()->As<type::Vector>()->Width(), 3u);
    EXPECT_TRUE(members[0]->Type()->As<type::Vector>()->type()->Is<type::F32>());
    EXPECT_EQ(members[1]->Type()->As<type::Vector>()->Width(), 3u);
    EXPECT_TRUE(members[1]->Type()->As<type::Vector>()->type()->Is<type::F32>());
}

TEST_F(ResolverBuiltinsValidationTest, Modf_Vec4) {
    auto* builtin = Call("modf", vec4<f32>(1_f, 1_f, 1_f, 1_f));
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* res_ty = TypeOf(builtin)->As<type::Struct>();
    ASSERT_TRUE(res_ty != nullptr);
    auto members = res_ty->Members();
    ASSERT_EQ(members.Length(), 2u);
    ASSERT_TRUE(members[0]->Type()->Is<type::Vector>());
    ASSERT_TRUE(members[1]->Type()->Is<type::Vector>());
    EXPECT_EQ(members[0]->Type()->As<type::Vector>()->Width(), 4u);
    EXPECT_TRUE(members[0]->Type()->As<type::Vector>()->type()->Is<type::F32>());
    EXPECT_EQ(members[1]->Type()->As<type::Vector>()->Width(), 4u);
    EXPECT_TRUE(members[1]->Type()->As<type::Vector>()->type()->Is<type::F32>());
}

TEST_F(ResolverBuiltinsValidationTest, Cross_Float_Vec3) {
    auto* builtin = Call("cross", vec3<f32>(1_f, 1_f, 1_f), vec3<f32>(1_f, 1_f, 1_f));
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBuiltinsValidationTest, Dot_Float_Vec2) {
    auto* builtin = Call("dot", vec2<f32>(1_f, 1_f), vec2<f32>(1_f, 1_f));
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBuiltinsValidationTest, Dot_Float_Vec3) {
    auto* builtin = Call("dot", vec3<f32>(1_f, 1_f, 1_f), vec3<f32>(1_f, 1_f, 1_f));
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBuiltinsValidationTest, Dot_Float_Vec4) {
    auto* builtin = Call("dot", vec4<f32>(1_f, 1_f, 1_f, 1_f), vec4<f32>(1_f, 1_f, 1_f, 1_f));
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBuiltinsValidationTest, Select_Float_Scalar) {
    auto* builtin = Call("select", Expr(1_f), Expr(1_f), Expr(true));
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBuiltinsValidationTest, Select_Integer_Scalar) {
    auto* builtin = Call("select", Expr(1_i), Expr(1_i), Expr(true));
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBuiltinsValidationTest, Select_Boolean_Scalar) {
    auto* builtin = Call("select", Expr(true), Expr(true), Expr(true));
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBuiltinsValidationTest, Select_Float_Vec2) {
    auto* builtin =
        Call("select", vec2<f32>(1_f, 1_f), vec2<f32>(1_f, 1_f), vec2<bool>(true, true));
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBuiltinsValidationTest, Select_Integer_Vec2) {
    auto* builtin =
        Call("select", vec2<i32>(1_i, 1_i), vec2<i32>(1_i, 1_i), vec2<bool>(true, true));
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBuiltinsValidationTest, Select_Boolean_Vec2) {
    auto* builtin =
        Call("select", vec2<bool>(true, true), vec2<bool>(true, true), vec2<bool>(true, true));
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

template <typename T>
class ResolverBuiltinsValidationTestWithParams : public resolver::TestHelper,
                                                 public testing::TestWithParam<T> {};

using FloatAllMatching =
    ResolverBuiltinsValidationTestWithParams<std::tuple<std::string, uint32_t>>;

TEST_P(FloatAllMatching, Scalar) {
    std::string name = std::get<0>(GetParam());
    uint32_t num_params = std::get<1>(GetParam());

    utils::Vector<const ast::Expression*, 8> params;
    for (uint32_t i = 0; i < num_params; ++i) {
        params.Push(Expr(f32(i + 1)));
    }
    auto* builtin = Call(name, params);
    Func("func", utils::Empty, ty.void_(),
         utils::Vector{
             Assign(Phony(), builtin),
         },
         utils::Vector{
             create<ast::StageAttribute>(ast::PipelineStage::kFragment),
         });

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_TRUE(TypeOf(builtin)->Is<type::F32>());
}

TEST_P(FloatAllMatching, Vec2) {
    std::string name = std::get<0>(GetParam());
    uint32_t num_params = std::get<1>(GetParam());

    utils::Vector<const ast::Expression*, 8> params;
    for (uint32_t i = 0; i < num_params; ++i) {
        params.Push(vec2<f32>(f32(i + 1), f32(i + 1)));
    }
    auto* builtin = Call(name, params);
    Func("func", utils::Empty, ty.void_(),
         utils::Vector{
             Assign(Phony(), builtin),
         },
         utils::Vector{
             create<ast::StageAttribute>(ast::PipelineStage::kFragment),
         });

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_TRUE(TypeOf(builtin)->is_float_vector());
}

TEST_P(FloatAllMatching, Vec3) {
    std::string name = std::get<0>(GetParam());
    uint32_t num_params = std::get<1>(GetParam());

    utils::Vector<const ast::Expression*, 8> params;
    for (uint32_t i = 0; i < num_params; ++i) {
        params.Push(vec3<f32>(f32(i + 1), f32(i + 1), f32(i + 1)));
    }
    auto* builtin = Call(name, params);
    Func("func", utils::Empty, ty.void_(),
         utils::Vector{
             Assign(Phony(), builtin),
         },
         utils::Vector{
             create<ast::StageAttribute>(ast::PipelineStage::kFragment),
         });

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_TRUE(TypeOf(builtin)->is_float_vector());
}

TEST_P(FloatAllMatching, Vec4) {
    std::string name = std::get<0>(GetParam());
    uint32_t num_params = std::get<1>(GetParam());

    utils::Vector<const ast::Expression*, 8> params;
    for (uint32_t i = 0; i < num_params; ++i) {
        params.Push(vec4<f32>(f32(i + 1), f32(i + 1), f32(i + 1), f32(i + 1)));
    }
    auto* builtin = Call(name, params);
    Func("func", utils::Empty, ty.void_(),
         utils::Vector{
             Assign(Phony(), builtin),
         },
         utils::Vector{
             create<ast::StageAttribute>(ast::PipelineStage::kFragment),
         });

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_TRUE(TypeOf(builtin)->is_float_vector());
}

INSTANTIATE_TEST_SUITE_P(ResolverBuiltinsValidationTest,
                         FloatAllMatching,
                         ::testing::Values(std::make_tuple("abs", 1),
                                           std::make_tuple("acos", 1),
                                           std::make_tuple("asin", 1),
                                           std::make_tuple("atan", 1),
                                           std::make_tuple("atan2", 2),
                                           std::make_tuple("ceil", 1),
                                           std::make_tuple("clamp", 3),
                                           std::make_tuple("cos", 1),
                                           std::make_tuple("cosh", 1),
                                           std::make_tuple("dpdx", 1),
                                           std::make_tuple("dpdxCoarse", 1),
                                           std::make_tuple("dpdxFine", 1),
                                           std::make_tuple("dpdy", 1),
                                           std::make_tuple("dpdyCoarse", 1),
                                           std::make_tuple("dpdyFine", 1),
                                           std::make_tuple("exp", 1),
                                           std::make_tuple("exp2", 1),
                                           std::make_tuple("floor", 1),
                                           std::make_tuple("fma", 3),
                                           std::make_tuple("fract", 1),
                                           std::make_tuple("fwidth", 1),
                                           std::make_tuple("fwidthCoarse", 1),
                                           std::make_tuple("fwidthFine", 1),
                                           std::make_tuple("inverseSqrt", 1),
                                           std::make_tuple("log", 1),
                                           std::make_tuple("log2", 1),
                                           std::make_tuple("max", 2),
                                           std::make_tuple("min", 2),
                                           std::make_tuple("mix", 3),
                                           std::make_tuple("pow", 2),
                                           std::make_tuple("round", 1),
                                           std::make_tuple("sign", 1),
                                           std::make_tuple("sin", 1),
                                           std::make_tuple("sinh", 1),
                                           std::make_tuple("smoothstep", 3),
                                           std::make_tuple("sqrt", 1),
                                           std::make_tuple("step", 2),
                                           std::make_tuple("tan", 1),
                                           std::make_tuple("tanh", 1),
                                           std::make_tuple("trunc", 1)));

using IntegerAllMatching =
    ResolverBuiltinsValidationTestWithParams<std::tuple<std::string, uint32_t>>;

TEST_P(IntegerAllMatching, ScalarUnsigned) {
    std::string name = std::get<0>(GetParam());
    uint32_t num_params = std::get<1>(GetParam());

    utils::Vector<const ast::Expression*, 8> params;
    for (uint32_t i = 0; i < num_params; ++i) {
        params.Push(Call<u32>(1_i));
    }
    auto* builtin = Call(name, params);
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_TRUE(TypeOf(builtin)->Is<type::U32>());
}

TEST_P(IntegerAllMatching, Vec2Unsigned) {
    std::string name = std::get<0>(GetParam());
    uint32_t num_params = std::get<1>(GetParam());

    utils::Vector<const ast::Expression*, 8> params;
    for (uint32_t i = 0; i < num_params; ++i) {
        params.Push(vec2<u32>(1_u, 1_u));
    }
    auto* builtin = Call(name, params);
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_TRUE(TypeOf(builtin)->is_unsigned_integer_vector());
}

TEST_P(IntegerAllMatching, Vec3Unsigned) {
    std::string name = std::get<0>(GetParam());
    uint32_t num_params = std::get<1>(GetParam());

    utils::Vector<const ast::Expression*, 8> params;
    for (uint32_t i = 0; i < num_params; ++i) {
        params.Push(vec3<u32>(1_u, 1_u, 1_u));
    }
    auto* builtin = Call(name, params);
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_TRUE(TypeOf(builtin)->is_unsigned_integer_vector());
}

TEST_P(IntegerAllMatching, Vec4Unsigned) {
    std::string name = std::get<0>(GetParam());
    uint32_t num_params = std::get<1>(GetParam());

    utils::Vector<const ast::Expression*, 8> params;
    for (uint32_t i = 0; i < num_params; ++i) {
        params.Push(vec4<u32>(1_u, 1_u, 1_u, 1_u));
    }
    auto* builtin = Call(name, params);
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_TRUE(TypeOf(builtin)->is_unsigned_integer_vector());
}

TEST_P(IntegerAllMatching, ScalarSigned) {
    std::string name = std::get<0>(GetParam());
    uint32_t num_params = std::get<1>(GetParam());

    utils::Vector<const ast::Expression*, 8> params;
    for (uint32_t i = 0; i < num_params; ++i) {
        params.Push(Call<i32>(1_i));
    }
    auto* builtin = Call(name, params);
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_TRUE(TypeOf(builtin)->Is<type::I32>());
}

TEST_P(IntegerAllMatching, Vec2Signed) {
    std::string name = std::get<0>(GetParam());
    uint32_t num_params = std::get<1>(GetParam());

    utils::Vector<const ast::Expression*, 8> params;
    for (uint32_t i = 0; i < num_params; ++i) {
        params.Push(vec2<i32>(1_i, 1_i));
    }
    auto* builtin = Call(name, params);
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_TRUE(TypeOf(builtin)->is_signed_integer_vector());
}

TEST_P(IntegerAllMatching, Vec3Signed) {
    std::string name = std::get<0>(GetParam());
    uint32_t num_params = std::get<1>(GetParam());

    utils::Vector<const ast::Expression*, 8> params;
    for (uint32_t i = 0; i < num_params; ++i) {
        params.Push(vec3<i32>(1_i, 1_i, 1_i));
    }
    auto* builtin = Call(name, params);
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_TRUE(TypeOf(builtin)->is_signed_integer_vector());
}

TEST_P(IntegerAllMatching, Vec4Signed) {
    std::string name = std::get<0>(GetParam());
    uint32_t num_params = std::get<1>(GetParam());

    utils::Vector<const ast::Expression*, 8> params;
    for (uint32_t i = 0; i < num_params; ++i) {
        params.Push(vec4<i32>(1_i, 1_i, 1_i, 1_i));
    }
    auto* builtin = Call(name, params);
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_TRUE(TypeOf(builtin)->is_signed_integer_vector());
}

INSTANTIATE_TEST_SUITE_P(ResolverBuiltinsValidationTest,
                         IntegerAllMatching,
                         ::testing::Values(std::make_tuple("abs", 1),
                                           std::make_tuple("clamp", 3),
                                           std::make_tuple("countOneBits", 1),
                                           std::make_tuple("max", 2),
                                           std::make_tuple("min", 2),
                                           std::make_tuple("reverseBits", 1)));

using BooleanVectorInput =
    ResolverBuiltinsValidationTestWithParams<std::tuple<std::string, uint32_t>>;

TEST_P(BooleanVectorInput, Vec2) {
    std::string name = std::get<0>(GetParam());
    uint32_t num_params = std::get<1>(GetParam());

    utils::Vector<const ast::Expression*, 8> params;
    for (uint32_t i = 0; i < num_params; ++i) {
        params.Push(vec2<bool>(true, true));
    }
    auto* builtin = Call(name, params);
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_P(BooleanVectorInput, Vec3) {
    std::string name = std::get<0>(GetParam());
    uint32_t num_params = std::get<1>(GetParam());

    utils::Vector<const ast::Expression*, 8> params;
    for (uint32_t i = 0; i < num_params; ++i) {
        params.Push(vec3<bool>(true, true, true));
    }
    auto* builtin = Call(name, params);
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_P(BooleanVectorInput, Vec4) {
    std::string name = std::get<0>(GetParam());
    uint32_t num_params = std::get<1>(GetParam());

    utils::Vector<const ast::Expression*, 8> params;
    for (uint32_t i = 0; i < num_params; ++i) {
        params.Push(vec4<bool>(true, true, true, true));
    }
    auto* builtin = Call(name, params);
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

INSTANTIATE_TEST_SUITE_P(ResolverBuiltinsValidationTest,
                         BooleanVectorInput,
                         ::testing::Values(std::make_tuple("all", 1), std::make_tuple("any", 1)));

using DataPacking4x8 = ResolverBuiltinsValidationTestWithParams<std::string>;

TEST_P(DataPacking4x8, Float_Vec4) {
    auto name = GetParam();
    auto* builtin = Call(name, vec4<f32>(1_f, 1_f, 1_f, 1_f));
    WrapInFunction(builtin);
    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

INSTANTIATE_TEST_SUITE_P(ResolverBuiltinsValidationTest,
                         DataPacking4x8,
                         ::testing::Values("pack4x8snorm", "pack4x8unorm"));

using DataPacking2x16 = ResolverBuiltinsValidationTestWithParams<std::string>;

TEST_P(DataPacking2x16, Float_Vec2) {
    auto name = GetParam();
    auto* builtin = Call(name, vec2<f32>(1_f, 1_f));
    WrapInFunction(builtin);
    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

INSTANTIATE_TEST_SUITE_P(ResolverBuiltinsValidationTest,
                         DataPacking2x16,
                         ::testing::Values("pack2x16snorm", "pack2x16unorm", "pack2x16float"));

}  // namespace
}  // namespace tint::resolver
