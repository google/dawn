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
#include "src/tint/resolver/resolver_test_helper.h"

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
using f32 = builder::f32;
using i32 = builder::i32;
using u32 = builder::u32;

class ResolverBuiltinsValidationTest : public resolver::TestHelper, public testing::Test {};
namespace StageTest {
struct Params {
    builder::ast_type_func_ptr type;
    ast::Builtin builtin;
    ast::PipelineStage stage;
    bool is_valid;
};

template <typename T>
constexpr Params ParamsFor(ast::Builtin builtin, ast::PipelineStage stage, bool is_valid) {
    return Params{DataType<T>::AST, builtin, stage, is_valid};
}
static constexpr Params cases[] = {
    ParamsFor<vec4<f32>>(ast::Builtin::kPosition, ast::PipelineStage::kVertex, false),
    ParamsFor<vec4<f32>>(ast::Builtin::kPosition, ast::PipelineStage::kFragment, true),
    ParamsFor<vec4<f32>>(ast::Builtin::kPosition, ast::PipelineStage::kCompute, false),

    ParamsFor<u32>(ast::Builtin::kVertexIndex, ast::PipelineStage::kVertex, true),
    ParamsFor<u32>(ast::Builtin::kVertexIndex, ast::PipelineStage::kFragment, false),
    ParamsFor<u32>(ast::Builtin::kVertexIndex, ast::PipelineStage::kCompute, false),

    ParamsFor<u32>(ast::Builtin::kInstanceIndex, ast::PipelineStage::kVertex, true),
    ParamsFor<u32>(ast::Builtin::kInstanceIndex, ast::PipelineStage::kFragment, false),
    ParamsFor<u32>(ast::Builtin::kInstanceIndex, ast::PipelineStage::kCompute, false),

    ParamsFor<bool>(ast::Builtin::kFrontFacing, ast::PipelineStage::kVertex, false),
    ParamsFor<bool>(ast::Builtin::kFrontFacing, ast::PipelineStage::kFragment, true),
    ParamsFor<bool>(ast::Builtin::kFrontFacing, ast::PipelineStage::kCompute, false),

    ParamsFor<vec3<u32>>(ast::Builtin::kLocalInvocationId, ast::PipelineStage::kVertex, false),
    ParamsFor<vec3<u32>>(ast::Builtin::kLocalInvocationId, ast::PipelineStage::kFragment, false),
    ParamsFor<vec3<u32>>(ast::Builtin::kLocalInvocationId, ast::PipelineStage::kCompute, true),

    ParamsFor<u32>(ast::Builtin::kLocalInvocationIndex, ast::PipelineStage::kVertex, false),
    ParamsFor<u32>(ast::Builtin::kLocalInvocationIndex, ast::PipelineStage::kFragment, false),
    ParamsFor<u32>(ast::Builtin::kLocalInvocationIndex, ast::PipelineStage::kCompute, true),

    ParamsFor<vec3<u32>>(ast::Builtin::kGlobalInvocationId, ast::PipelineStage::kVertex, false),
    ParamsFor<vec3<u32>>(ast::Builtin::kGlobalInvocationId, ast::PipelineStage::kFragment, false),
    ParamsFor<vec3<u32>>(ast::Builtin::kGlobalInvocationId, ast::PipelineStage::kCompute, true),

    ParamsFor<vec3<u32>>(ast::Builtin::kWorkgroupId, ast::PipelineStage::kVertex, false),
    ParamsFor<vec3<u32>>(ast::Builtin::kWorkgroupId, ast::PipelineStage::kFragment, false),
    ParamsFor<vec3<u32>>(ast::Builtin::kWorkgroupId, ast::PipelineStage::kCompute, true),

    ParamsFor<vec3<u32>>(ast::Builtin::kNumWorkgroups, ast::PipelineStage::kVertex, false),
    ParamsFor<vec3<u32>>(ast::Builtin::kNumWorkgroups, ast::PipelineStage::kFragment, false),
    ParamsFor<vec3<u32>>(ast::Builtin::kNumWorkgroups, ast::PipelineStage::kCompute, true),

    ParamsFor<u32>(ast::Builtin::kSampleIndex, ast::PipelineStage::kVertex, false),
    ParamsFor<u32>(ast::Builtin::kSampleIndex, ast::PipelineStage::kFragment, true),
    ParamsFor<u32>(ast::Builtin::kSampleIndex, ast::PipelineStage::kCompute, false),

    ParamsFor<u32>(ast::Builtin::kSampleMask, ast::PipelineStage::kVertex, false),
    ParamsFor<u32>(ast::Builtin::kSampleMask, ast::PipelineStage::kFragment, true),
    ParamsFor<u32>(ast::Builtin::kSampleMask, ast::PipelineStage::kCompute, false),
};

using ResolverBuiltinsStageTest = ResolverTestWithParam<Params>;
TEST_P(ResolverBuiltinsStageTest, All_input) {
    const Params& params = GetParam();

    auto* p = Global("p", ty.vec4<f32>(), ast::StorageClass::kPrivate);
    auto* input = Param("input", params.type(*this),
                        ast::AttributeList{Builtin(Source{{12, 34}}, params.builtin)});
    switch (params.stage) {
        case ast::PipelineStage::kVertex:
            Func("main", {input}, ty.vec4<f32>(), {Return(p)}, {Stage(ast::PipelineStage::kVertex)},
                 {Builtin(Source{{12, 34}}, ast::Builtin::kPosition)});
            break;
        case ast::PipelineStage::kFragment:
            Func("main", {input}, ty.void_(), {}, {Stage(ast::PipelineStage::kFragment)}, {});
            break;
        case ast::PipelineStage::kCompute:
            Func("main", {input}, ty.void_(), {},
                 ast::AttributeList{Stage(ast::PipelineStage::kCompute), WorkgroupSize(1)});
            break;
        default:
            break;
    }

    if (params.is_valid) {
        EXPECT_TRUE(r()->Resolve()) << r()->error();
    } else {
        std::stringstream err;
        err << "12:34 error: builtin(" << params.builtin << ")";
        err << " cannot be used in input of " << params.stage << " pipeline stage";
        EXPECT_FALSE(r()->Resolve());
        EXPECT_EQ(r()->error(), err.str());
    }
}
INSTANTIATE_TEST_SUITE_P(ResolverBuiltinsValidationTest,
                         ResolverBuiltinsStageTest,
                         testing::ValuesIn(cases));

TEST_F(ResolverBuiltinsValidationTest, FragDepthIsInput_Fail) {
    // @stage(fragment)
    // fn fs_main(
    //   @builtin(frag_depth) fd: f32,
    // ) -> @location(0) f32 { return 1.0; }
    auto* fd = Param("fd", ty.f32(),
                     ast::AttributeList{Builtin(Source{{12, 34}}, ast::Builtin::kFragDepth)});
    Func("fs_main", ast::VariableList{fd}, ty.f32(), {Return(1.0f)},
         ast::AttributeList{Stage(ast::PipelineStage::kFragment)}, {Location(0)});
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: builtin(frag_depth) cannot be used in input of "
              "fragment pipeline stage");
}

TEST_F(ResolverBuiltinsValidationTest, FragDepthIsInputStruct_Fail) {
    // struct MyInputs {
    //   @builtin(frag_depth) ff: f32;
    // };
    // @stage(fragment)
    // fn fragShader(arg: MyInputs) -> @location(0) f32 { return 1.0; }

    auto* s = Structure(
        "MyInputs",
        {Member("frag_depth", ty.f32(),
                ast::AttributeList{Builtin(Source{{12, 34}}, ast::Builtin::kFragDepth)})});

    Func("fragShader", {Param("arg", ty.Of(s))}, ty.f32(), {Return(1.0f)},
         {Stage(ast::PipelineStage::kFragment)}, {Location(0)});
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: builtin(frag_depth) cannot be used in input of "
              "fragment pipeline stage\n"
              "note: while analysing entry point 'fragShader'");
}

TEST_F(ResolverBuiltinsValidationTest, StructBuiltinInsideEntryPoint_Ignored) {
    // struct S {
    //   @builtin(vertex_index) idx: u32;
    // };
    // @stage(fragment)
    // fn fragShader() { var s : S; }

    Structure("S", {Member("idx", ty.u32(), {Builtin(ast::Builtin::kVertexIndex)})});

    Func("fragShader", {}, ty.void_(), {Decl(Var("s", ty.type_name("S")))},
         {Stage(ast::PipelineStage::kFragment)});
    EXPECT_TRUE(r()->Resolve());
}

}  // namespace StageTest

TEST_F(ResolverBuiltinsValidationTest, PositionNotF32_Struct_Fail) {
    // struct MyInputs {
    //   @builtin(kPosition) p: vec4<u32>;
    // };
    // @stage(fragment)
    // fn fragShader(is_front: MyInputs) -> @location(0) f32 { return 1.0; }

    auto* m = Member("position", ty.vec4<u32>(),
                     ast::AttributeList{Builtin(Source{{12, 34}}, ast::Builtin::kPosition)});
    auto* s = Structure("MyInputs", {m});
    Func("fragShader", {Param("arg", ty.Of(s))}, ty.f32(), {Return(1.0f)},
         {Stage(ast::PipelineStage::kFragment)}, {Location(0)});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: store type of builtin(position) must be 'vec4<f32>'");
}

TEST_F(ResolverBuiltinsValidationTest, PositionNotF32_ReturnType_Fail) {
    // @stage(vertex)
    // fn main() -> @builtin(position) f32 { return 1.0; }
    Func("main", {}, ty.f32(), {Return(1.0f)}, {Stage(ast::PipelineStage::kVertex)},
         {Builtin(Source{{12, 34}}, ast::Builtin::kPosition)});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: store type of builtin(position) must be 'vec4<f32>'");
}

TEST_F(ResolverBuiltinsValidationTest, FragDepthNotF32_Struct_Fail) {
    // struct MyInputs {
    //   @builtin(kFragDepth) p: i32;
    // };
    // @stage(fragment)
    // fn fragShader(is_front: MyInputs) -> @location(0) f32 { return 1.0; }

    auto* m = Member("frag_depth", ty.i32(),
                     ast::AttributeList{Builtin(Source{{12, 34}}, ast::Builtin::kFragDepth)});
    auto* s = Structure("MyInputs", {m});
    Func("fragShader", {Param("arg", ty.Of(s))}, ty.f32(), {Return(1.0f)},
         {Stage(ast::PipelineStage::kFragment)}, {Location(0)});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: store type of builtin(frag_depth) must be 'f32'");
}

TEST_F(ResolverBuiltinsValidationTest, SampleMaskNotU32_Struct_Fail) {
    // struct MyInputs {
    //   @builtin(sample_mask) m: f32;
    // };
    // @stage(fragment)
    // fn fragShader(is_front: MyInputs) -> @location(0) f32 { return 1.0; }

    auto* s = Structure(
        "MyInputs",
        {Member("m", ty.f32(),
                ast::AttributeList{Builtin(Source{{12, 34}}, ast::Builtin::kSampleMask)})});
    Func("fragShader", {Param("arg", ty.Of(s))}, ty.f32(), {Return(1.0f)},
         {Stage(ast::PipelineStage::kFragment)}, {Location(0)});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: store type of builtin(sample_mask) must be 'u32'");
}

TEST_F(ResolverBuiltinsValidationTest, SampleMaskNotU32_ReturnType_Fail) {
    // @stage(fragment)
    // fn main() -> @builtin(sample_mask) i32 { return 1; }
    Func("main", {}, ty.i32(), {Return(1)}, {Stage(ast::PipelineStage::kFragment)},
         {Builtin(Source{{12, 34}}, ast::Builtin::kSampleMask)});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: store type of builtin(sample_mask) must be 'u32'");
}

TEST_F(ResolverBuiltinsValidationTest, SampleMaskIsNotU32_Fail) {
    // @stage(fragment)
    // fn fs_main(
    //   @builtin(sample_mask) arg: bool
    // ) -> @location(0) f32 { return 1.0; }
    auto* arg = Param("arg", ty.bool_(),
                      ast::AttributeList{Builtin(Source{{12, 34}}, ast::Builtin::kSampleMask)});
    Func("fs_main", ast::VariableList{arg}, ty.f32(), {Return(1.0f)},
         ast::AttributeList{Stage(ast::PipelineStage::kFragment)}, {Location(0)});
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: store type of builtin(sample_mask) must be 'u32'");
}

TEST_F(ResolverBuiltinsValidationTest, SampleIndexIsNotU32_Struct_Fail) {
    // struct MyInputs {
    //   @builtin(sample_index) m: f32;
    // };
    // @stage(fragment)
    // fn fragShader(is_front: MyInputs) -> @location(0) f32 { return 1.0; }

    auto* s = Structure(
        "MyInputs",
        {Member("m", ty.f32(),
                ast::AttributeList{Builtin(Source{{12, 34}}, ast::Builtin::kSampleIndex)})});
    Func("fragShader", {Param("arg", ty.Of(s))}, ty.f32(), {Return(1.0f)},
         {Stage(ast::PipelineStage::kFragment)}, {Location(0)});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: store type of builtin(sample_index) must be 'u32'");
}

TEST_F(ResolverBuiltinsValidationTest, SampleIndexIsNotU32_Fail) {
    // @stage(fragment)
    // fn fs_main(
    //   @builtin(sample_index) arg: bool
    // ) -> @location(0) f32 { return 1.0; }
    auto* arg = Param("arg", ty.bool_(),
                      ast::AttributeList{Builtin(Source{{12, 34}}, ast::Builtin::kSampleIndex)});
    Func("fs_main", ast::VariableList{arg}, ty.f32(), {Return(1.0f)},
         ast::AttributeList{Stage(ast::PipelineStage::kFragment)}, {Location(0)});
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: store type of builtin(sample_index) must be 'u32'");
}

TEST_F(ResolverBuiltinsValidationTest, PositionIsNotF32_Fail) {
    // @stage(fragment)
    // fn fs_main(
    //   @builtin(kPosition) p: vec3<f32>,
    // ) -> @location(0) f32 { return 1.0; }
    auto* p = Param("p", ty.vec3<f32>(),
                    ast::AttributeList{Builtin(Source{{12, 34}}, ast::Builtin::kPosition)});
    Func("fs_main", ast::VariableList{p}, ty.f32(), {Return(1.0f)},
         ast::AttributeList{Stage(ast::PipelineStage::kFragment)}, {Location(0)});
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: store type of builtin(position) must be 'vec4<f32>'");
}

TEST_F(ResolverBuiltinsValidationTest, FragDepthIsNotF32_Fail) {
    // @stage(fragment)
    // fn fs_main() -> @builtin(kFragDepth) f32 { var fd: i32; return fd; }
    auto* fd = Var("fd", ty.i32());
    Func("fs_main", {}, ty.i32(), {Decl(fd), Return(fd)},
         ast::AttributeList{Stage(ast::PipelineStage::kFragment)},
         ast::AttributeList{Builtin(Source{{12, 34}}, ast::Builtin::kFragDepth)});
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: store type of builtin(frag_depth) must be 'f32'");
}

TEST_F(ResolverBuiltinsValidationTest, VertexIndexIsNotU32_Fail) {
    // @stage(vertex)
    // fn main(
    //   @builtin(kVertexIndex) vi : f32,
    //   @builtin(kPosition) p :vec4<f32>
    // ) -> @builtin(kPosition) vec4<f32> { return vec4<f32>(); }
    auto* p = Param("p", ty.vec4<f32>(), ast::AttributeList{Builtin(ast::Builtin::kPosition)});
    auto* vi = Param("vi", ty.f32(),
                     ast::AttributeList{Builtin(Source{{12, 34}}, ast::Builtin::kVertexIndex)});
    Func("main", ast::VariableList{vi, p}, ty.vec4<f32>(), {Return(Expr("p"))},
         ast::AttributeList{Stage(ast::PipelineStage::kVertex)},
         ast::AttributeList{Builtin(ast::Builtin::kPosition)});
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: store type of builtin(vertex_index) must be 'u32'");
}

TEST_F(ResolverBuiltinsValidationTest, InstanceIndexIsNotU32) {
    // @stage(vertex)
    // fn main(
    //   @builtin(kInstanceIndex) ii : f32,
    //   @builtin(kPosition) p :vec4<f32>
    // ) -> @builtin(kPosition) vec4<f32> { return vec4<f32>(); }
    auto* p = Param("p", ty.vec4<f32>(), ast::AttributeList{Builtin(ast::Builtin::kPosition)});
    auto* ii = Param("ii", ty.f32(),
                     ast::AttributeList{Builtin(Source{{12, 34}}, ast::Builtin::kInstanceIndex)});
    Func("main", ast::VariableList{ii, p}, ty.vec4<f32>(), {Return(Expr("p"))},
         ast::AttributeList{Stage(ast::PipelineStage::kVertex)},
         ast::AttributeList{Builtin(ast::Builtin::kPosition)});
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: store type of builtin(instance_index) must be 'u32'");
}

TEST_F(ResolverBuiltinsValidationTest, FragmentBuiltin_Pass) {
    // @stage(fragment)
    // fn fs_main(
    //   @builtin(kPosition) p: vec4<f32>,
    //   @builtin(front_facing) ff: bool,
    //   @builtin(sample_index) si: u32,
    //   @builtin(sample_mask) sm : u32
    // ) -> @builtin(frag_depth) f32 { var fd: f32; return fd; }
    auto* p = Param("p", ty.vec4<f32>(), ast::AttributeList{Builtin(ast::Builtin::kPosition)});
    auto* ff = Param("ff", ty.bool_(), ast::AttributeList{Builtin(ast::Builtin::kFrontFacing)});
    auto* si = Param("si", ty.u32(), ast::AttributeList{Builtin(ast::Builtin::kSampleIndex)});
    auto* sm = Param("sm", ty.u32(), ast::AttributeList{Builtin(ast::Builtin::kSampleMask)});
    auto* var_fd = Var("fd", ty.f32());
    Func("fs_main", ast::VariableList{p, ff, si, sm}, ty.f32(), {Decl(var_fd), Return(var_fd)},
         ast::AttributeList{Stage(ast::PipelineStage::kFragment)},
         ast::AttributeList{Builtin(ast::Builtin::kFragDepth)});
    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBuiltinsValidationTest, VertexBuiltin_Pass) {
    // @stage(vertex)
    // fn main(
    //   @builtin(vertex_index) vi : u32,
    //   @builtin(instance_index) ii : u32,
    // ) -> @builtin(position) vec4<f32> { var p :vec4<f32>; return p; }
    auto* vi = Param("vi", ty.u32(),
                     ast::AttributeList{Builtin(Source{{12, 34}}, ast::Builtin::kVertexIndex)});

    auto* ii = Param("ii", ty.u32(),
                     ast::AttributeList{Builtin(Source{{12, 34}}, ast::Builtin::kInstanceIndex)});
    auto* p = Var("p", ty.vec4<f32>());
    Func("main", ast::VariableList{vi, ii}, ty.vec4<f32>(),
         {
             Decl(p),
             Return(p),
         },
         ast::AttributeList{Stage(ast::PipelineStage::kVertex)},
         ast::AttributeList{Builtin(ast::Builtin::kPosition)});

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBuiltinsValidationTest, ComputeBuiltin_Pass) {
    // @stage(compute) @workgroup_size(1)
    // fn main(
    //   @builtin(local_invocationId) li_id: vec3<u32>,
    //   @builtin(local_invocationIndex) li_index: u32,
    //   @builtin(global_invocationId) gi: vec3<u32>,
    //   @builtin(workgroup_id) wi: vec3<u32>,
    //   @builtin(num_workgroups) nwgs: vec3<u32>,
    // ) {}

    auto* li_id = Param("li_id", ty.vec3<u32>(),
                        ast::AttributeList{Builtin(ast::Builtin::kLocalInvocationId)});
    auto* li_index = Param("li_index", ty.u32(),
                           ast::AttributeList{Builtin(ast::Builtin::kLocalInvocationIndex)});
    auto* gi =
        Param("gi", ty.vec3<u32>(), ast::AttributeList{Builtin(ast::Builtin::kGlobalInvocationId)});
    auto* wi = Param("wi", ty.vec3<u32>(), ast::AttributeList{Builtin(ast::Builtin::kWorkgroupId)});
    auto* nwgs =
        Param("nwgs", ty.vec3<u32>(), ast::AttributeList{Builtin(ast::Builtin::kNumWorkgroups)});

    Func("main", ast::VariableList{li_id, li_index, gi, wi, nwgs}, ty.void_(), {},
         ast::AttributeList{Stage(ast::PipelineStage::kCompute),
                            WorkgroupSize(Expr(Source{Source::Location{12, 34}}, 2))});

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBuiltinsValidationTest, ComputeBuiltin_WorkGroupIdNotVec3U32) {
    auto* wi = Param("wi", ty.f32(),
                     ast::AttributeList{Builtin(Source{{12, 34}}, ast::Builtin::kWorkgroupId)});
    Func("main", ast::VariableList{wi}, ty.void_(), {},
         ast::AttributeList{Stage(ast::PipelineStage::kCompute),
                            WorkgroupSize(Expr(Source{Source::Location{12, 34}}, 2))});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: store type of builtin(workgroup_id) must be "
              "'vec3<u32>'");
}

TEST_F(ResolverBuiltinsValidationTest, ComputeBuiltin_NumWorkgroupsNotVec3U32) {
    auto* nwgs = Param("nwgs", ty.f32(),
                       ast::AttributeList{Builtin(Source{{12, 34}}, ast::Builtin::kNumWorkgroups)});
    Func("main", ast::VariableList{nwgs}, ty.void_(), {},
         ast::AttributeList{Stage(ast::PipelineStage::kCompute),
                            WorkgroupSize(Expr(Source{Source::Location{12, 34}}, 2))});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: store type of builtin(num_workgroups) must be "
              "'vec3<u32>'");
}

TEST_F(ResolverBuiltinsValidationTest, ComputeBuiltin_GlobalInvocationNotVec3U32) {
    auto* gi =
        Param("gi", ty.vec3<i32>(),
              ast::AttributeList{Builtin(Source{{12, 34}}, ast::Builtin::kGlobalInvocationId)});
    Func("main", ast::VariableList{gi}, ty.void_(), {},
         ast::AttributeList{Stage(ast::PipelineStage::kCompute),
                            WorkgroupSize(Expr(Source{Source::Location{12, 34}}, 2))});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: store type of builtin(global_invocation_id) must be "
              "'vec3<u32>'");
}

TEST_F(ResolverBuiltinsValidationTest, ComputeBuiltin_LocalInvocationIndexNotU32) {
    auto* li_index =
        Param("li_index", ty.vec3<u32>(),
              ast::AttributeList{Builtin(Source{{12, 34}}, ast::Builtin::kLocalInvocationIndex)});
    Func("main", ast::VariableList{li_index}, ty.void_(), {},
         ast::AttributeList{Stage(ast::PipelineStage::kCompute),
                            WorkgroupSize(Expr(Source{Source::Location{12, 34}}, 2))});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: store type of builtin(local_invocation_index) must be "
              "'u32'");
}

TEST_F(ResolverBuiltinsValidationTest, ComputeBuiltin_LocalInvocationNotVec3U32) {
    auto* li_id =
        Param("li_id", ty.vec2<u32>(),
              ast::AttributeList{Builtin(Source{{12, 34}}, ast::Builtin::kLocalInvocationId)});
    Func("main", ast::VariableList{li_id}, ty.void_(), {},
         ast::AttributeList{Stage(ast::PipelineStage::kCompute),
                            WorkgroupSize(Expr(Source{Source::Location{12, 34}}, 2))});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: store type of builtin(local_invocation_id) must be "
              "'vec3<u32>'");
}

TEST_F(ResolverBuiltinsValidationTest, FragmentBuiltinStruct_Pass) {
    // Struct MyInputs {
    //   @builtin(kPosition) p: vec4<f32>;
    //   @builtin(frag_depth) fd: f32;
    //   @builtin(sample_index) si: u32;
    //   @builtin(sample_mask) sm : u32;;
    // };
    // @stage(fragment)
    // fn fragShader(arg: MyInputs) -> @location(0) f32 { return 1.0; }

    auto* s = Structure(
        "MyInputs",
        {Member("position", ty.vec4<f32>(), ast::AttributeList{Builtin(ast::Builtin::kPosition)}),
         Member("front_facing", ty.bool_(),
                ast::AttributeList{Builtin(ast::Builtin::kFrontFacing)}),
         Member("sample_index", ty.u32(), ast::AttributeList{Builtin(ast::Builtin::kSampleIndex)}),
         Member("sample_mask", ty.u32(), ast::AttributeList{Builtin(ast::Builtin::kSampleMask)})});
    Func("fragShader", {Param("arg", ty.Of(s))}, ty.f32(), {Return(1.0f)},
         {Stage(ast::PipelineStage::kFragment)}, {Location(0)});
    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBuiltinsValidationTest, FrontFacingParamIsNotBool_Fail) {
    // @stage(fragment)
    // fn fs_main(
    //   @builtin(front_facing) is_front: i32;
    // ) -> @location(0) f32 { return 1.0; }

    auto* is_front =
        Param("is_front", ty.i32(),
              ast::AttributeList{Builtin(Source{{12, 34}}, ast::Builtin::kFrontFacing)});
    Func("fs_main", ast::VariableList{is_front}, ty.f32(), {Return(1.0f)},
         ast::AttributeList{Stage(ast::PipelineStage::kFragment)}, {Location(0)});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: store type of builtin(front_facing) must be 'bool'");
}

TEST_F(ResolverBuiltinsValidationTest, FrontFacingMemberIsNotBool_Fail) {
    // struct MyInputs {
    //   @builtin(front_facing) pos: f32;
    // };
    // @stage(fragment)
    // fn fragShader(is_front: MyInputs) -> @location(0) f32 { return 1.0; }

    auto* s = Structure(
        "MyInputs",
        {Member("pos", ty.f32(),
                ast::AttributeList{Builtin(Source{{12, 34}}, ast::Builtin::kFrontFacing)})});
    Func("fragShader", {Param("is_front", ty.Of(s))}, ty.f32(), {Return(1.0f)},
         {Stage(ast::PipelineStage::kFragment)}, {Location(0)});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: store type of builtin(front_facing) must be 'bool'");
}

TEST_F(ResolverBuiltinsValidationTest, Length_Float_Scalar) {
    auto* builtin = Call("length", 1.0f);
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBuiltinsValidationTest, Length_Float_Vec2) {
    auto* builtin = Call("length", vec2<f32>(1.0f, 1.0f));
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBuiltinsValidationTest, Length_Float_Vec3) {
    auto* builtin = Call("length", vec3<f32>(1.0f, 1.0f, 1.0f));
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBuiltinsValidationTest, Length_Float_Vec4) {
    auto* builtin = Call("length", vec4<f32>(1.0f, 1.0f, 1.0f, 1.0f));
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBuiltinsValidationTest, Distance_Float_Scalar) {
    auto* builtin = Call("distance", 1.0f, 1.0f);
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBuiltinsValidationTest, Distance_Float_Vec2) {
    auto* builtin = Call("distance", vec2<f32>(1.0f, 1.0f), vec2<f32>(1.0f, 1.0f));
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBuiltinsValidationTest, Distance_Float_Vec3) {
    auto* builtin = Call("distance", vec3<f32>(1.0f, 1.0f, 1.0f), vec3<f32>(1.0f, 1.0f, 1.0f));
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBuiltinsValidationTest, Distance_Float_Vec4) {
    auto* builtin =
        Call("distance", vec4<f32>(1.0f, 1.0f, 1.0f, 1.0f), vec4<f32>(1.0f, 1.0f, 1.0f, 1.0f));
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBuiltinsValidationTest, Determinant_Mat2x2) {
    auto* builtin = Call("determinant", mat2x2<f32>(vec2<f32>(1.0f, 1.0f), vec2<f32>(1.0f, 1.0f)));
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBuiltinsValidationTest, Determinant_Mat3x3) {
    auto* builtin =
        Call("determinant", mat3x3<f32>(vec3<f32>(1.0f, 1.0f, 1.0f), vec3<f32>(1.0f, 1.0f, 1.0f),
                                        vec3<f32>(1.0f, 1.0f, 1.0f)));
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBuiltinsValidationTest, Determinant_Mat4x4) {
    auto* builtin =
        Call("determinant",
             mat4x4<f32>(vec4<f32>(1.0f, 1.0f, 1.0f, 1.0f), vec4<f32>(1.0f, 1.0f, 1.0f, 1.0f),
                         vec4<f32>(1.0f, 1.0f, 1.0f, 1.0f), vec4<f32>(1.0f, 1.0f, 1.0f, 1.0f)));
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBuiltinsValidationTest, Frexp_Scalar) {
    auto* builtin = Call("frexp", 1.0f);
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* res_ty = TypeOf(builtin)->As<sem::Struct>();
    ASSERT_TRUE(res_ty != nullptr);
    auto& members = res_ty->Members();
    ASSERT_EQ(members.size(), 2u);
    EXPECT_TRUE(members[0]->Type()->Is<sem::F32>());
    EXPECT_TRUE(members[1]->Type()->Is<sem::I32>());
}

TEST_F(ResolverBuiltinsValidationTest, Frexp_Vec2) {
    auto* builtin = Call("frexp", vec2<f32>(1.0f, 1.0f));
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* res_ty = TypeOf(builtin)->As<sem::Struct>();
    ASSERT_TRUE(res_ty != nullptr);
    auto& members = res_ty->Members();
    ASSERT_EQ(members.size(), 2u);
    ASSERT_TRUE(members[0]->Type()->Is<sem::Vector>());
    ASSERT_TRUE(members[1]->Type()->Is<sem::Vector>());
    EXPECT_EQ(members[0]->Type()->As<sem::Vector>()->Width(), 2u);
    EXPECT_TRUE(members[0]->Type()->As<sem::Vector>()->type()->Is<sem::F32>());
    EXPECT_EQ(members[1]->Type()->As<sem::Vector>()->Width(), 2u);
    EXPECT_TRUE(members[1]->Type()->As<sem::Vector>()->type()->Is<sem::I32>());
}

TEST_F(ResolverBuiltinsValidationTest, Frexp_Vec3) {
    auto* builtin = Call("frexp", vec3<f32>(1.0f, 1.0f, 1.0f));
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* res_ty = TypeOf(builtin)->As<sem::Struct>();
    ASSERT_TRUE(res_ty != nullptr);
    auto& members = res_ty->Members();
    ASSERT_EQ(members.size(), 2u);
    ASSERT_TRUE(members[0]->Type()->Is<sem::Vector>());
    ASSERT_TRUE(members[1]->Type()->Is<sem::Vector>());
    EXPECT_EQ(members[0]->Type()->As<sem::Vector>()->Width(), 3u);
    EXPECT_TRUE(members[0]->Type()->As<sem::Vector>()->type()->Is<sem::F32>());
    EXPECT_EQ(members[1]->Type()->As<sem::Vector>()->Width(), 3u);
    EXPECT_TRUE(members[1]->Type()->As<sem::Vector>()->type()->Is<sem::I32>());
}

TEST_F(ResolverBuiltinsValidationTest, Frexp_Vec4) {
    auto* builtin = Call("frexp", vec4<f32>(1.0f, 1.0f, 1.0f, 1.0f));
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* res_ty = TypeOf(builtin)->As<sem::Struct>();
    ASSERT_TRUE(res_ty != nullptr);
    auto& members = res_ty->Members();
    ASSERT_EQ(members.size(), 2u);
    ASSERT_TRUE(members[0]->Type()->Is<sem::Vector>());
    ASSERT_TRUE(members[1]->Type()->Is<sem::Vector>());
    EXPECT_EQ(members[0]->Type()->As<sem::Vector>()->Width(), 4u);
    EXPECT_TRUE(members[0]->Type()->As<sem::Vector>()->type()->Is<sem::F32>());
    EXPECT_EQ(members[1]->Type()->As<sem::Vector>()->Width(), 4u);
    EXPECT_TRUE(members[1]->Type()->As<sem::Vector>()->type()->Is<sem::I32>());
}

TEST_F(ResolverBuiltinsValidationTest, Modf_Scalar) {
    auto* builtin = Call("modf", 1.0f);
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* res_ty = TypeOf(builtin)->As<sem::Struct>();
    ASSERT_TRUE(res_ty != nullptr);
    auto& members = res_ty->Members();
    ASSERT_EQ(members.size(), 2u);
    EXPECT_TRUE(members[0]->Type()->Is<sem::F32>());
    EXPECT_TRUE(members[1]->Type()->Is<sem::F32>());
}

TEST_F(ResolverBuiltinsValidationTest, Modf_Vec2) {
    auto* builtin = Call("modf", vec2<f32>(1.0f, 1.0f));
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* res_ty = TypeOf(builtin)->As<sem::Struct>();
    ASSERT_TRUE(res_ty != nullptr);
    auto& members = res_ty->Members();
    ASSERT_EQ(members.size(), 2u);
    ASSERT_TRUE(members[0]->Type()->Is<sem::Vector>());
    ASSERT_TRUE(members[1]->Type()->Is<sem::Vector>());
    EXPECT_EQ(members[0]->Type()->As<sem::Vector>()->Width(), 2u);
    EXPECT_TRUE(members[0]->Type()->As<sem::Vector>()->type()->Is<sem::F32>());
    EXPECT_EQ(members[1]->Type()->As<sem::Vector>()->Width(), 2u);
    EXPECT_TRUE(members[1]->Type()->As<sem::Vector>()->type()->Is<sem::F32>());
}

TEST_F(ResolverBuiltinsValidationTest, Modf_Vec3) {
    auto* builtin = Call("modf", vec3<f32>(1.0f, 1.0f, 1.0f));
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* res_ty = TypeOf(builtin)->As<sem::Struct>();
    ASSERT_TRUE(res_ty != nullptr);
    auto& members = res_ty->Members();
    ASSERT_EQ(members.size(), 2u);
    ASSERT_TRUE(members[0]->Type()->Is<sem::Vector>());
    ASSERT_TRUE(members[1]->Type()->Is<sem::Vector>());
    EXPECT_EQ(members[0]->Type()->As<sem::Vector>()->Width(), 3u);
    EXPECT_TRUE(members[0]->Type()->As<sem::Vector>()->type()->Is<sem::F32>());
    EXPECT_EQ(members[1]->Type()->As<sem::Vector>()->Width(), 3u);
    EXPECT_TRUE(members[1]->Type()->As<sem::Vector>()->type()->Is<sem::F32>());
}

TEST_F(ResolverBuiltinsValidationTest, Modf_Vec4) {
    auto* builtin = Call("modf", vec4<f32>(1.0f, 1.0f, 1.0f, 1.0f));
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* res_ty = TypeOf(builtin)->As<sem::Struct>();
    ASSERT_TRUE(res_ty != nullptr);
    auto& members = res_ty->Members();
    ASSERT_EQ(members.size(), 2u);
    ASSERT_TRUE(members[0]->Type()->Is<sem::Vector>());
    ASSERT_TRUE(members[1]->Type()->Is<sem::Vector>());
    EXPECT_EQ(members[0]->Type()->As<sem::Vector>()->Width(), 4u);
    EXPECT_TRUE(members[0]->Type()->As<sem::Vector>()->type()->Is<sem::F32>());
    EXPECT_EQ(members[1]->Type()->As<sem::Vector>()->Width(), 4u);
    EXPECT_TRUE(members[1]->Type()->As<sem::Vector>()->type()->Is<sem::F32>());
}

TEST_F(ResolverBuiltinsValidationTest, Cross_Float_Vec3) {
    auto* builtin = Call("cross", vec3<f32>(1.0f, 1.0f, 1.0f), vec3<f32>(1.0f, 1.0f, 1.0f));
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBuiltinsValidationTest, Dot_Float_Vec2) {
    auto* builtin = Call("dot", vec2<f32>(1.0f, 1.0f), vec2<f32>(1.0f, 1.0f));
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBuiltinsValidationTest, Dot_Float_Vec3) {
    auto* builtin = Call("dot", vec3<f32>(1.0f, 1.0f, 1.0f), vec3<f32>(1.0f, 1.0f, 1.0f));
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBuiltinsValidationTest, Dot_Float_Vec4) {
    auto* builtin =
        Call("dot", vec4<f32>(1.0f, 1.0f, 1.0f, 1.0f), vec4<f32>(1.0f, 1.0f, 1.0f, 1.0f));
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBuiltinsValidationTest, Select_Float_Scalar) {
    auto* builtin = Call("select", Expr(1.0f), Expr(1.0f), Expr(true));
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBuiltinsValidationTest, Select_Integer_Scalar) {
    auto* builtin = Call("select", Expr(1), Expr(1), Expr(true));
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
        Call("select", vec2<f32>(1.0f, 1.0f), vec2<f32>(1.0f, 1.0f), vec2<bool>(true, true));
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBuiltinsValidationTest, Select_Integer_Vec2) {
    auto* builtin = Call("select", vec2<int>(1, 1), vec2<int>(1, 1), vec2<bool>(true, true));
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

    ast::ExpressionList params;
    for (uint32_t i = 0; i < num_params; ++i) {
        params.push_back(Expr(1.0f));
    }
    auto* builtin = Call(name, params);
    Func("func", {}, ty.void_(), {CallStmt(builtin)},
         {create<ast::StageAttribute>(ast::PipelineStage::kFragment)});

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_TRUE(TypeOf(builtin)->Is<sem::F32>());
}

TEST_P(FloatAllMatching, Vec2) {
    std::string name = std::get<0>(GetParam());
    uint32_t num_params = std::get<1>(GetParam());

    ast::ExpressionList params;
    for (uint32_t i = 0; i < num_params; ++i) {
        params.push_back(vec2<f32>(1.0f, 1.0f));
    }
    auto* builtin = Call(name, params);
    Func("func", {}, ty.void_(), {CallStmt(builtin)},
         {create<ast::StageAttribute>(ast::PipelineStage::kFragment)});

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_TRUE(TypeOf(builtin)->is_float_vector());
}

TEST_P(FloatAllMatching, Vec3) {
    std::string name = std::get<0>(GetParam());
    uint32_t num_params = std::get<1>(GetParam());

    ast::ExpressionList params;
    for (uint32_t i = 0; i < num_params; ++i) {
        params.push_back(vec3<f32>(1.0f, 1.0f, 1.0f));
    }
    auto* builtin = Call(name, params);
    Func("func", {}, ty.void_(), {CallStmt(builtin)},
         {create<ast::StageAttribute>(ast::PipelineStage::kFragment)});

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_TRUE(TypeOf(builtin)->is_float_vector());
}

TEST_P(FloatAllMatching, Vec4) {
    std::string name = std::get<0>(GetParam());
    uint32_t num_params = std::get<1>(GetParam());

    ast::ExpressionList params;
    for (uint32_t i = 0; i < num_params; ++i) {
        params.push_back(vec4<f32>(1.0f, 1.0f, 1.0f, 1.0f));
    }
    auto* builtin = Call(name, params);
    Func("func", {}, ty.void_(), {CallStmt(builtin)},
         {create<ast::StageAttribute>(ast::PipelineStage::kFragment)});

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

    ast::ExpressionList params;
    for (uint32_t i = 0; i < num_params; ++i) {
        params.push_back(Construct<uint32_t>(1));
    }
    auto* builtin = Call(name, params);
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_TRUE(TypeOf(builtin)->Is<sem::U32>());
}

TEST_P(IntegerAllMatching, Vec2Unsigned) {
    std::string name = std::get<0>(GetParam());
    uint32_t num_params = std::get<1>(GetParam());

    ast::ExpressionList params;
    for (uint32_t i = 0; i < num_params; ++i) {
        params.push_back(vec2<uint32_t>(1u, 1u));
    }
    auto* builtin = Call(name, params);
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_TRUE(TypeOf(builtin)->is_unsigned_integer_vector());
}

TEST_P(IntegerAllMatching, Vec3Unsigned) {
    std::string name = std::get<0>(GetParam());
    uint32_t num_params = std::get<1>(GetParam());

    ast::ExpressionList params;
    for (uint32_t i = 0; i < num_params; ++i) {
        params.push_back(vec3<uint32_t>(1u, 1u, 1u));
    }
    auto* builtin = Call(name, params);
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_TRUE(TypeOf(builtin)->is_unsigned_integer_vector());
}

TEST_P(IntegerAllMatching, Vec4Unsigned) {
    std::string name = std::get<0>(GetParam());
    uint32_t num_params = std::get<1>(GetParam());

    ast::ExpressionList params;
    for (uint32_t i = 0; i < num_params; ++i) {
        params.push_back(vec4<uint32_t>(1u, 1u, 1u, 1u));
    }
    auto* builtin = Call(name, params);
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_TRUE(TypeOf(builtin)->is_unsigned_integer_vector());
}

TEST_P(IntegerAllMatching, ScalarSigned) {
    std::string name = std::get<0>(GetParam());
    uint32_t num_params = std::get<1>(GetParam());

    ast::ExpressionList params;
    for (uint32_t i = 0; i < num_params; ++i) {
        params.push_back(Construct<int32_t>(1));
    }
    auto* builtin = Call(name, params);
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_TRUE(TypeOf(builtin)->Is<sem::I32>());
}

TEST_P(IntegerAllMatching, Vec2Signed) {
    std::string name = std::get<0>(GetParam());
    uint32_t num_params = std::get<1>(GetParam());

    ast::ExpressionList params;
    for (uint32_t i = 0; i < num_params; ++i) {
        params.push_back(vec2<int32_t>(1, 1));
    }
    auto* builtin = Call(name, params);
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_TRUE(TypeOf(builtin)->is_signed_integer_vector());
}

TEST_P(IntegerAllMatching, Vec3Signed) {
    std::string name = std::get<0>(GetParam());
    uint32_t num_params = std::get<1>(GetParam());

    ast::ExpressionList params;
    for (uint32_t i = 0; i < num_params; ++i) {
        params.push_back(vec3<int32_t>(1, 1, 1));
    }
    auto* builtin = Call(name, params);
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_TRUE(TypeOf(builtin)->is_signed_integer_vector());
}

TEST_P(IntegerAllMatching, Vec4Signed) {
    std::string name = std::get<0>(GetParam());
    uint32_t num_params = std::get<1>(GetParam());

    ast::ExpressionList params;
    for (uint32_t i = 0; i < num_params; ++i) {
        params.push_back(vec4<int32_t>(1, 1, 1, 1));
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

    ast::ExpressionList params;
    for (uint32_t i = 0; i < num_params; ++i) {
        params.push_back(vec2<bool>(true, true));
    }
    auto* builtin = Call(name, params);
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_P(BooleanVectorInput, Vec3) {
    std::string name = std::get<0>(GetParam());
    uint32_t num_params = std::get<1>(GetParam());

    ast::ExpressionList params;
    for (uint32_t i = 0; i < num_params; ++i) {
        params.push_back(vec3<bool>(true, true, true));
    }
    auto* builtin = Call(name, params);
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_P(BooleanVectorInput, Vec4) {
    std::string name = std::get<0>(GetParam());
    uint32_t num_params = std::get<1>(GetParam());

    ast::ExpressionList params;
    for (uint32_t i = 0; i < num_params; ++i) {
        params.push_back(vec4<bool>(true, true, true, true));
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
    auto* builtin = Call(name, vec4<f32>(1.0f, 1.0f, 1.0f, 1.0f));
    WrapInFunction(builtin);
    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

INSTANTIATE_TEST_SUITE_P(ResolverBuiltinsValidationTest,
                         DataPacking4x8,
                         ::testing::Values("pack4x8snorm", "pack4x8unorm"));

using DataPacking2x16 = ResolverBuiltinsValidationTestWithParams<std::string>;

TEST_P(DataPacking2x16, Float_Vec2) {
    auto name = GetParam();
    auto* builtin = Call(name, vec2<f32>(1.0f, 1.0f));
    WrapInFunction(builtin);
    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

INSTANTIATE_TEST_SUITE_P(ResolverBuiltinsValidationTest,
                         DataPacking2x16,
                         ::testing::Values("pack2x16snorm", "pack2x16unorm", "pack2x16float"));

}  // namespace
}  // namespace tint::resolver
