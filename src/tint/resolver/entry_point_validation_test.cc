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

#include "src/tint/ast/builtin_attribute.h"
#include "src/tint/ast/location_attribute.h"
#include "src/tint/ast/return_statement.h"
#include "src/tint/ast/stage_attribute.h"
#include "src/tint/builtin/builtin_value.h"
#include "src/tint/resolver/resolver.h"
#include "src/tint/resolver/resolver_test_helper.h"

#include "gmock/gmock.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::resolver {
namespace {

// Helpers and typedefs
template <typename T>
using DataType = builder::DataType<T>;
template <typename T>
using vec2 = builder::vec2<T>;
template <typename T>
using vec3 = builder::vec3<T>;
template <typename T>
using vec4 = builder::vec4<T>;
template <typename T>
using mat2x2 = builder::mat2x2<T>;
template <typename T>
using mat3x3 = builder::mat3x3<T>;
template <typename T>
using mat4x4 = builder::mat4x4<T>;
template <typename T>
using alias = builder::alias<T>;

class ResolverEntryPointValidationTest : public TestHelper, public testing::Test {};

TEST_F(ResolverEntryPointValidationTest, ReturnTypeAttribute_Location) {
    // @fragment
    // fn main() -> @location(0) f32 { return 1.0; }
    Func(Source{{12, 34}}, "main", utils::Empty, ty.f32(),
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

TEST_F(ResolverEntryPointValidationTest, ReturnTypeAttribute_Builtin) {
    // @vertex
    // fn main() -> @builtin(position) vec4<f32> { return vec4<f32>(); }
    Func(Source{{12, 34}}, "main", utils::Empty, ty.vec4<f32>(),
         utils::Vector{
             Return(Call(ty.vec4<f32>())),
         },
         utils::Vector{
             Stage(ast::PipelineStage::kVertex),
         },
         utils::Vector{
             Builtin(builtin::BuiltinValue::kPosition),
         });

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverEntryPointValidationTest, ReturnTypeAttribute_Missing) {
    // @vertex
    // fn main() -> f32 {
    //   return 1.0;
    // }
    Func(Source{{12, 34}}, "main", utils::Empty, ty.vec4<f32>(),
         utils::Vector{
             Return(Call(ty.vec4<f32>())),
         },
         utils::Vector{
             Stage(ast::PipelineStage::kVertex),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: missing entry point IO attribute on return type)");
}

TEST_F(ResolverEntryPointValidationTest, ReturnTypeAttribute_Multiple) {
    // @vertex
    // fn main() -> @location(0) @builtin(position) vec4<f32> {
    //   return vec4<f32>();
    // }
    Func(Source{{12, 34}}, "main", utils::Empty, ty.vec4<f32>(),
         utils::Vector{
             Return(Call(ty.vec4<f32>())),
         },
         utils::Vector{
             Stage(ast::PipelineStage::kVertex),
         },
         utils::Vector{
             Location(Source{{13, 43}}, 0_a),
             Builtin(Source{{14, 52}}, builtin::BuiltinValue::kPosition),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(14:52 error: multiple entry point IO attributes
13:43 note: previously consumed @location)");
}

TEST_F(ResolverEntryPointValidationTest, ReturnType_Struct_Valid) {
    // struct Output {
    //   @location(0) a : f32;
    //   @builtin(frag_depth) b : f32;
    // };
    // @fragment
    // fn main() -> Output {
    //   return Output();
    // }
    auto* output = Structure(
        "Output",
        utils::Vector{
            Member("a", ty.f32(), utils::Vector{Location(0_a)}),
            Member("b", ty.f32(), utils::Vector{Builtin(builtin::BuiltinValue::kFragDepth)}),
        });
    Func(Source{{12, 34}}, "main", utils::Empty, ty.Of(output),
         utils::Vector{
             Return(Call(ty.Of(output))),
         },
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverEntryPointValidationTest, ReturnType_Struct_MemberMultipleAttributes) {
    // struct Output {
    //   @location(0) @builtin(frag_depth) a : f32;
    // };
    // @fragment
    // fn main() -> Output {
    //   return Output();
    // }
    auto* output = Structure(
        "Output",
        utils::Vector{
            Member("a", ty.f32(),
                   utils::Vector{Location(Source{{13, 43}}, 0_a),
                                 Builtin(Source{{14, 52}}, builtin::BuiltinValue::kFragDepth)}),
        });
    Func(Source{{12, 34}}, "main", utils::Empty, ty.Of(output),
         utils::Vector{
             Return(Call(ty.Of(output))),
         },
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(14:52 error: multiple entry point IO attributes
13:43 note: previously consumed @location
12:34 note: while analyzing entry point 'main')");
}

TEST_F(ResolverEntryPointValidationTest, ReturnType_Struct_MemberMissingAttribute) {
    // struct Output {
    //   @location(0) a : f32;
    //   b : f32;
    // };
    // @fragment
    // fn main() -> Output {
    //   return Output();
    // }
    auto* output = Structure(
        "Output", utils::Vector{
                      Member(Source{{13, 43}}, "a", ty.f32(), utils::Vector{Location(0_a)}),
                      Member(Source{{14, 52}}, "b", ty.f32(), {}),
                  });
    Func(Source{{12, 34}}, "main", utils::Empty, ty.Of(output),
         utils::Vector{
             Return(Call(ty.Of(output))),
         },
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(14:52 error: missing entry point IO attribute
12:34 note: while analyzing entry point 'main')");
}

TEST_F(ResolverEntryPointValidationTest, ReturnType_Struct_DuplicateBuiltins) {
    // struct Output {
    //   @builtin(frag_depth) a : f32;
    //   @builtin(frag_depth) b : f32;
    // };
    // @fragment
    // fn main() -> Output {
    //   return Output();
    // }
    auto* output = Structure(
        "Output",
        utils::Vector{
            Member("a", ty.f32(), utils::Vector{Builtin(builtin::BuiltinValue::kFragDepth)}),
            Member("b", ty.f32(), utils::Vector{Builtin(builtin::BuiltinValue::kFragDepth)}),
        });
    Func(Source{{12, 34}}, "main", utils::Empty, ty.Of(output),
         utils::Vector{
             Return(Call(ty.Of(output))),
         },
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: @builtin(frag_depth) appears multiple times as pipeline output
12:34 note: while analyzing entry point 'main')");
}

TEST_F(ResolverEntryPointValidationTest, ParameterAttribute_Location) {
    // @fragment
    // fn main(@location(0) param : f32) {}
    auto* param = Param("param", ty.f32(),
                        utils::Vector{
                            Location(0_a),
                        });
    Func(Source{{12, 34}}, "main",
         utils::Vector{
             param,
         },
         ty.void_(), utils::Empty,
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverEntryPointValidationTest, ParameterAttribute_Missing) {
    // @fragment
    // fn main(param : f32) {}
    auto* param = Param(Source{{13, 43}}, "param", ty.vec4<f32>());
    Func(Source{{12, 34}}, "main",
         utils::Vector{
             param,
         },
         ty.void_(), utils::Empty,
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(13:43 error: missing entry point IO attribute on parameter)");
}

TEST_F(ResolverEntryPointValidationTest, ParameterAttribute_Multiple) {
    // @fragment
    // fn main(@location(0) @builtin(sample_index) param : u32) {}
    auto* param = Param("param", ty.u32(),
                        utils::Vector{
                            Location(Source{{13, 43}}, 0_a),
                            Builtin(Source{{14, 52}}, builtin::BuiltinValue::kSampleIndex),
                        });
    Func(Source{{12, 34}}, "main",
         utils::Vector{
             param,
         },
         ty.void_(), utils::Empty,
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(14:52 error: multiple entry point IO attributes
13:43 note: previously consumed @location)");
}

TEST_F(ResolverEntryPointValidationTest, Parameter_Struct_Valid) {
    // struct Input {
    //   @location(0) a : f32;
    //   @builtin(sample_index) b : u32;
    // };
    // @fragment
    // fn main(param : Input) {}
    auto* input = Structure(
        "Input",
        utils::Vector{
            Member("a", ty.f32(), utils::Vector{Location(0_a)}),
            Member("b", ty.u32(), utils::Vector{Builtin(builtin::BuiltinValue::kSampleIndex)}),
        });
    auto* param = Param("param", ty.Of(input));
    Func(Source{{12, 34}}, "main",
         utils::Vector{
             param,
         },
         ty.void_(), utils::Empty,
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverEntryPointValidationTest, Parameter_Struct_MemberMultipleAttributes) {
    // struct Input {
    //   @location(0) @builtin(sample_index) a : u32;
    // };
    // @fragment
    // fn main(param : Input) {}
    auto* input = Structure(
        "Input",
        utils::Vector{
            Member("a", ty.u32(),
                   utils::Vector{Location(Source{{13, 43}}, 0_a),
                                 Builtin(Source{{14, 52}}, builtin::BuiltinValue::kSampleIndex)}),
        });
    auto* param = Param("param", ty.Of(input));
    Func(Source{{12, 34}}, "main",
         utils::Vector{
             param,
         },
         ty.void_(), utils::Empty,
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(14:52 error: multiple entry point IO attributes
13:43 note: previously consumed @location
12:34 note: while analyzing entry point 'main')");
}

TEST_F(ResolverEntryPointValidationTest, Parameter_Struct_MemberMissingAttribute) {
    // struct Input {
    //   @location(0) a : f32;
    //   b : f32;
    // };
    // @fragment
    // fn main(param : Input) {}
    auto* input = Structure(
        "Input", utils::Vector{
                     Member(Source{{13, 43}}, "a", ty.f32(), utils::Vector{Location(0_a)}),
                     Member(Source{{14, 52}}, "b", ty.f32(), {}),
                 });
    auto* param = Param("param", ty.Of(input));
    Func(Source{{12, 34}}, "main",
         utils::Vector{
             param,
         },
         ty.void_(), utils::Empty,
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(14:52 error: missing entry point IO attribute
12:34 note: while analyzing entry point 'main')");
}

TEST_F(ResolverEntryPointValidationTest, Parameter_DuplicateBuiltins) {
    // @fragment
    // fn main(@builtin(sample_index) param_a : u32,
    //         @builtin(sample_index) param_b : u32) {}
    auto* param_a = Param("param_a", ty.u32(),
                          utils::Vector{
                              Builtin(builtin::BuiltinValue::kSampleIndex),
                          });
    auto* param_b = Param("param_b", ty.u32(),
                          utils::Vector{
                              Builtin(builtin::BuiltinValue::kSampleIndex),
                          });
    Func(Source{{12, 34}}, "main",
         utils::Vector{
             param_a,
             param_b,
         },
         ty.void_(), utils::Empty,
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: @builtin(sample_index) appears multiple times as pipeline input");
}

TEST_F(ResolverEntryPointValidationTest, Parameter_Struct_DuplicateBuiltins) {
    // struct InputA {
    //   @builtin(sample_index) a : u32;
    // };
    // struct InputB {
    //   @builtin(sample_index) a : u32;
    // };
    // @fragment
    // fn main(param_a : InputA, param_b : InputB) {}
    auto* input_a = Structure(
        "InputA",
        utils::Vector{
            Member("a", ty.u32(), utils::Vector{Builtin(builtin::BuiltinValue::kSampleIndex)}),
        });
    auto* input_b = Structure(
        "InputB",
        utils::Vector{
            Member("a", ty.u32(), utils::Vector{Builtin(builtin::BuiltinValue::kSampleIndex)}),
        });
    auto* param_a = Param("param_a", ty.Of(input_a));
    auto* param_b = Param("param_b", ty.Of(input_b));
    Func(Source{{12, 34}}, "main",
         utils::Vector{
             param_a,
             param_b,
         },
         ty.void_(), utils::Empty,
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: @builtin(sample_index) appears multiple times as pipeline input
12:34 note: while analyzing entry point 'main')");
}

TEST_F(ResolverEntryPointValidationTest, VertexShaderMustReturnPosition) {
    // @vertex
    // fn main() {}
    Func(Source{{12, 34}}, "main", utils::Empty, ty.void_(), utils::Empty,
         utils::Vector{
             Stage(ast::PipelineStage::kVertex),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: a vertex shader must include the 'position' builtin "
              "in its return type");
}

TEST_F(ResolverEntryPointValidationTest, PushConstantAllowedWithEnable) {
    // enable chromium_experimental_push_constant;
    // var<push_constant> a : u32;
    Enable(builtin::Extension::kChromiumExperimentalPushConstant);
    GlobalVar("a", ty.u32(), builtin::AddressSpace::kPushConstant);

    EXPECT_TRUE(r()->Resolve());
}

TEST_F(ResolverEntryPointValidationTest, PushConstantDisallowedWithoutEnable) {
    // var<push_constant> a : u32;
    GlobalVar(Source{{1, 2}}, "a", ty.u32(), builtin::AddressSpace::kPushConstant);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "1:2 error: use of variable address space 'push_constant' requires enabling "
              "extension 'chromium_experimental_push_constant'");
}

TEST_F(ResolverEntryPointValidationTest, PushConstantAllowedWithIgnoreAddressSpaceAttribute) {
    // var<push_constant> a : u32; // With ast::DisabledValidation::kIgnoreAddressSpace
    GlobalVar("a", ty.u32(), builtin::AddressSpace::kPushConstant,
              utils::Vector{Disable(ast::DisabledValidation::kIgnoreAddressSpace)});

    EXPECT_TRUE(r()->Resolve());
}

TEST_F(ResolverEntryPointValidationTest, PushConstantOneVariableUsedInEntryPoint) {
    // enable chromium_experimental_push_constant;
    // var<push_constant> a : u32;
    // @compute @workgroup_size(1) fn main() {
    //   _ = a;
    // }
    Enable(builtin::Extension::kChromiumExperimentalPushConstant);
    GlobalVar("a", ty.u32(), builtin::AddressSpace::kPushConstant);

    Func("main", {}, ty.void_(), utils::Vector{Assign(Phony(), "a")},
         utils::Vector{Stage(ast::PipelineStage::kCompute),
                       create<ast::WorkgroupAttribute>(Expr(1_i))});

    EXPECT_TRUE(r()->Resolve());
}

TEST_F(ResolverEntryPointValidationTest, PushConstantTwoVariablesUsedInEntryPoint) {
    // enable chromium_experimental_push_constant;
    // var<push_constant> a : u32;
    // var<push_constant> b : u32;
    // @compute @workgroup_size(1) fn main() {
    //   _ = a;
    //   _ = b;
    // }
    Enable(builtin::Extension::kChromiumExperimentalPushConstant);
    GlobalVar(Source{{1, 2}}, "a", ty.u32(), builtin::AddressSpace::kPushConstant);
    GlobalVar(Source{{3, 4}}, "b", ty.u32(), builtin::AddressSpace::kPushConstant);

    Func(Source{{5, 6}}, "main", {}, ty.void_(),
         utils::Vector{Assign(Phony(), "a"), Assign(Phony(), "b")},
         utils::Vector{Stage(ast::PipelineStage::kCompute),
                       create<ast::WorkgroupAttribute>(Expr(1_i))});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(5:6 error: entry point 'main' uses two different 'push_constant' variables.
3:4 note: first 'push_constant' variable declaration is here
1:2 note: second 'push_constant' variable declaration is here)");
}

TEST_F(ResolverEntryPointValidationTest,
       PushConstantTwoVariablesUsedInEntryPointWithFunctionGraph) {
    // enable chromium_experimental_push_constant;
    // var<push_constant> a : u32;
    // var<push_constant> b : u32;
    // fn uses_a() {
    //   _ = a;
    // }
    // fn uses_b() {
    //   _ = b;
    // }
    // @compute @workgroup_size(1) fn main() {
    //   uses_a();
    //   uses_b();
    // }
    Enable(builtin::Extension::kChromiumExperimentalPushConstant);
    GlobalVar(Source{{1, 2}}, "a", ty.u32(), builtin::AddressSpace::kPushConstant);
    GlobalVar(Source{{3, 4}}, "b", ty.u32(), builtin::AddressSpace::kPushConstant);

    Func(Source{{5, 6}}, "uses_a", {}, ty.void_(), utils::Vector{Assign(Phony(), "a")});
    Func(Source{{7, 8}}, "uses_b", {}, ty.void_(), utils::Vector{Assign(Phony(), "b")});

    Func(Source{{9, 10}}, "main", {}, ty.void_(),
         utils::Vector{CallStmt(Call("uses_a")), CallStmt(Call("uses_b"))},
         utils::Vector{Stage(ast::PipelineStage::kCompute),
                       create<ast::WorkgroupAttribute>(Expr(1_i))});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(9:10 error: entry point 'main' uses two different 'push_constant' variables.
3:4 note: first 'push_constant' variable declaration is here
7:8 note: called by function 'uses_b'
9:10 note: called by entry point 'main'
1:2 note: second 'push_constant' variable declaration is here
5:6 note: called by function 'uses_a'
9:10 note: called by entry point 'main')");
}

TEST_F(ResolverEntryPointValidationTest, PushConstantTwoVariablesUsedInDifferentEntryPoint) {
    // enable chromium_experimental_push_constant;
    // var<push_constant> a : u32;
    // var<push_constant> b : u32;
    // @compute @workgroup_size(1) fn uses_a() {
    //   _ = a;
    // }
    // @compute @workgroup_size(1) fn uses_b() {
    //   _ = a;
    // }
    Enable(builtin::Extension::kChromiumExperimentalPushConstant);
    GlobalVar("a", ty.u32(), builtin::AddressSpace::kPushConstant);
    GlobalVar("b", ty.u32(), builtin::AddressSpace::kPushConstant);

    Func("uses_a", {}, ty.void_(), utils::Vector{Assign(Phony(), "a")},
         utils::Vector{Stage(ast::PipelineStage::kCompute),
                       create<ast::WorkgroupAttribute>(Expr(1_i))});
    Func("uses_b", {}, ty.void_(), utils::Vector{Assign(Phony(), "b")},
         utils::Vector{Stage(ast::PipelineStage::kCompute),
                       create<ast::WorkgroupAttribute>(Expr(1_i))});

    EXPECT_TRUE(r()->Resolve());
}

namespace TypeValidationTests {
struct Params {
    builder::ast_type_func_ptr create_ast_type;
    bool is_valid;
};

template <typename T>
constexpr Params ParamsFor(bool is_valid) {
    return Params{DataType<T>::AST, is_valid};
}

using TypeValidationTest = resolver::ResolverTestWithParam<Params>;

static constexpr Params cases[] = {
    ParamsFor<f32>(true),           //
    ParamsFor<i32>(true),           //
    ParamsFor<u32>(true),           //
    ParamsFor<bool>(false),         //
    ParamsFor<vec2<f32>>(true),     //
    ParamsFor<vec3<f32>>(true),     //
    ParamsFor<vec4<f32>>(true),     //
    ParamsFor<mat2x2<f32>>(false),  //
    ParamsFor<mat3x3<f32>>(false),  //
    ParamsFor<mat4x4<f32>>(false),  //
    ParamsFor<alias<f32>>(true),    //
    ParamsFor<alias<i32>>(true),    //
    ParamsFor<alias<u32>>(true),    //
    ParamsFor<alias<bool>>(false),  //
    ParamsFor<f16>(true),           //
    ParamsFor<vec2<f16>>(true),     //
    ParamsFor<vec3<f16>>(true),     //
    ParamsFor<vec4<f16>>(true),     //
    ParamsFor<mat2x2<f16>>(false),  //
    ParamsFor<mat3x3<f16>>(false),  //
    ParamsFor<mat4x4<f16>>(false),  //
    ParamsFor<alias<f16>>(true),    //
};

TEST_P(TypeValidationTest, BareInputs) {
    // @fragment
    // fn main(@location(0) @interpolate(flat) a : *) {}
    auto params = GetParam();

    Enable(builtin::Extension::kF16);

    auto* a = Param("a", params.create_ast_type(*this),
                    utils::Vector{
                        Location(0_a),
                        Flat(),
                    });
    Func(Source{{12, 34}}, "main",
         utils::Vector{
             a,
         },
         ty.void_(), utils::Empty,
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    if (params.is_valid) {
        EXPECT_TRUE(r()->Resolve()) << r()->error();
    } else {
        EXPECT_FALSE(r()->Resolve());
    }
}

TEST_P(TypeValidationTest, StructInputs) {
    // struct Input {
    //   @location(0) @interpolate(flat) a : *;
    // };
    // @fragment
    // fn main(a : Input) {}
    auto params = GetParam();

    Enable(builtin::Extension::kF16);

    auto* input = Structure("Input", utils::Vector{
                                         Member("a", params.create_ast_type(*this),
                                                utils::Vector{Location(0_a), Flat()}),
                                     });
    auto* a = Param("a", ty.Of(input), {});
    Func(Source{{12, 34}}, "main",
         utils::Vector{
             a,
         },
         ty.void_(), utils::Empty,
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    if (params.is_valid) {
        EXPECT_TRUE(r()->Resolve()) << r()->error();
    } else {
        EXPECT_FALSE(r()->Resolve());
    }
}

TEST_P(TypeValidationTest, BareOutputs) {
    // @fragment
    // fn main() -> @location(0) * {
    //   return *();
    // }
    auto params = GetParam();

    Enable(builtin::Extension::kF16);

    Func(Source{{12, 34}}, "main", utils::Empty, params.create_ast_type(*this),
         utils::Vector{
             Return(Call(params.create_ast_type(*this))),
         },
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         },
         utils::Vector{
             Location(0_a),
         });

    if (params.is_valid) {
        EXPECT_TRUE(r()->Resolve()) << r()->error();
    } else {
        EXPECT_FALSE(r()->Resolve());
    }
}

TEST_P(TypeValidationTest, StructOutputs) {
    // struct Output {
    //   @location(0) a : *;
    // };
    // @fragment
    // fn main() -> Output {
    //   return Output();
    // }
    auto params = GetParam();

    Enable(builtin::Extension::kF16);

    auto* output = Structure(
        "Output", utils::Vector{
                      Member("a", params.create_ast_type(*this), utils::Vector{Location(0_a)}),
                  });
    Func(Source{{12, 34}}, "main", utils::Empty, ty.Of(output),
         utils::Vector{
             Return(Call(ty.Of(output))),
         },
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    if (params.is_valid) {
        EXPECT_TRUE(r()->Resolve()) << r()->error();
    } else {
        EXPECT_FALSE(r()->Resolve());
    }
}
INSTANTIATE_TEST_SUITE_P(ResolverEntryPointValidationTest,
                         TypeValidationTest,
                         testing::ValuesIn(cases));

}  // namespace TypeValidationTests

namespace LocationAttributeTests {
namespace {
using LocationAttributeTests = ResolverTest;

TEST_F(LocationAttributeTests, Pass) {
    // @fragment
    // fn frag_main(@location(0) @interpolate(flat) a: i32) {}

    auto* p = Param(Source{{12, 34}}, "a", ty.i32(),
                    utils::Vector{
                        Location(0_a),
                        Flat(),
                    });
    Func("frag_main",
         utils::Vector{
             p,
         },
         ty.void_(), utils::Empty,
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(LocationAttributeTests, BadType_Input_bool) {
    // @fragment
    // fn frag_main(@location(0) a: bool) {}

    auto* p = Param(Source{{12, 34}}, "a", ty.bool_(),
                    utils::Vector{
                        Location(Source{{34, 56}}, 0_a),
                    });
    Func("frag_main",
         utils::Vector{
             p,
         },
         ty.void_(), utils::Empty,
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: cannot apply @location to declaration of type 'bool'
34:56 note: @location must only be applied to declarations of numeric scalar or numeric vector type)");
}

TEST_F(LocationAttributeTests, BadType_Output_Array) {
    // @fragment
    // fn frag_main()->@location(0) array<f32, 2> { return array<f32, 2>(); }

    Func(Source{{12, 34}}, "frag_main", utils::Empty, ty.array<f32, 2>(),
         utils::Vector{
             Return(Call(ty.array<f32, 2>())),
         },
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         },
         utils::Vector{
             Location(Source{{34, 56}}, 0_a),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: cannot apply @location to declaration of type 'array<f32, 2>'
34:56 note: @location must only be applied to declarations of numeric scalar or numeric vector type)");
}

TEST_F(LocationAttributeTests, BadType_Input_Struct) {
    // struct Input {
    //   a : f32;
    // };
    // @fragment
    // fn main(@location(0) param : Input) {}
    auto* input = Structure("Input", utils::Vector{
                                         Member("a", ty.f32()),
                                     });
    auto* param = Param(Source{{12, 34}}, "param", ty.Of(input),
                        utils::Vector{
                            Location(Source{{13, 43}}, 0_a),
                        });
    Func(Source{{12, 34}}, "main",
         utils::Vector{
             param,
         },
         ty.void_(), utils::Empty,
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: cannot apply @location to declaration of type 'Input'
13:43 note: @location must only be applied to declarations of numeric scalar or numeric vector type)");
}

TEST_F(LocationAttributeTests, BadType_Input_Struct_NestedStruct) {
    // struct Inner {
    //   @location(0) b : f32;
    // };
    // struct Input {
    //   a : Inner;
    // };
    // @fragment
    // fn main(param : Input) {}
    auto* inner = Structure(
        "Inner", utils::Vector{
                     Member(Source{{13, 43}}, "a", ty.f32(), utils::Vector{Location(0_a)}),
                 });
    auto* input = Structure("Input", utils::Vector{
                                         Member(Source{{14, 52}}, "a", ty.Of(inner)),
                                     });
    auto* param = Param("param", ty.Of(input));
    Func(Source{{12, 34}}, "main",
         utils::Vector{
             param,
         },
         ty.void_(), utils::Empty,
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(14:52 error: nested structures cannot be used for entry point IO
12:34 note: while analyzing entry point 'main')");
}

TEST_F(LocationAttributeTests, BadType_Input_Struct_RuntimeArray) {
    // struct Input {
    //   @location(0) a : array<f32>;
    // };
    // @fragment
    // fn main(param : Input) {}
    auto* input = Structure(
        "Input", utils::Vector{
                     Member(Source{{13, 43}}, "a", ty.array<f32>(), utils::Vector{Location(0_a)}),
                 });
    auto* param = Param("param", ty.Of(input));
    Func(Source{{12, 34}}, "main",
         utils::Vector{
             param,
         },
         ty.void_(), utils::Empty,
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(13:43 error: cannot apply @location to declaration of type 'array<f32>'
note: @location must only be applied to declarations of numeric scalar or numeric vector type)");
}

TEST_F(LocationAttributeTests, BadMemberType_Input) {
    // struct S { @location(0) m: array<i32>; };
    // @fragment
    // fn frag_main( a: S) {}

    auto* m = Member(Source{{34, 56}}, "m", ty.array<i32>(),
                     utils::Vector{
                         Location(Source{{12, 34}}, 0_u),
                     });
    auto* s = Structure("S", utils::Vector{m});
    auto* p = Param("a", ty.Of(s));

    Func("frag_main",
         utils::Vector{
             p,
         },
         ty.void_(), utils::Empty,
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(34:56 error: cannot apply @location to declaration of type 'array<i32>'
12:34 note: @location must only be applied to declarations of numeric scalar or numeric vector type)");
}

TEST_F(LocationAttributeTests, BadMemberType_Output) {
    // struct S { @location(0) m: atomic<i32>; };
    // @fragment
    // fn frag_main() -> S {}
    auto* m = Member(Source{{34, 56}}, "m", ty.atomic<i32>(),
                     utils::Vector{
                         Location(Source{{12, 34}}, 0_u),
                     });
    auto* s = Structure("S", utils::Vector{m});

    Func("frag_main", utils::Empty, ty.Of(s),
         utils::Vector{
             Return(Call(ty.Of(s))),
         },
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         },
         {});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(34:56 error: cannot apply @location to declaration of type 'atomic<i32>'
12:34 note: @location must only be applied to declarations of numeric scalar or numeric vector type)");
}

TEST_F(LocationAttributeTests, BadMemberType_Unused) {
    // struct S { @location(0) m: mat3x2<f32>; };

    auto* m = Member(Source{{34, 56}}, "m", ty.mat3x2<f32>(),
                     utils::Vector{
                         Location(Source{{12, 34}}, 0_u),
                     });
    Structure("S", utils::Vector{m});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(34:56 error: cannot apply @location to declaration of type 'mat3x2<f32>'
12:34 note: @location must only be applied to declarations of numeric scalar or numeric vector type)");
}

TEST_F(LocationAttributeTests, ReturnType_Struct_Valid) {
    // struct Output {
    //   @location(0) a : f32;
    //   @builtin(frag_depth) b : f32;
    // };
    // @fragment
    // fn main() -> Output {
    //   return Output();
    // }
    auto* output = Structure(
        "Output",
        utils::Vector{
            Member("a", ty.f32(), utils::Vector{Location(0_a)}),
            Member("b", ty.f32(), utils::Vector{Builtin(builtin::BuiltinValue::kFragDepth)}),
        });
    Func(Source{{12, 34}}, "main", utils::Empty, ty.Of(output),
         utils::Vector{
             Return(Call(ty.Of(output))),
         },
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(LocationAttributeTests, ReturnType_Struct) {
    // struct Output {
    //   a : f32;
    // };
    // @vertex
    // fn main() -> @location(0) Output {
    //   return Output();
    // }
    auto* output = Structure("Output", utils::Vector{
                                           Member("a", ty.f32()),
                                       });
    Func(Source{{12, 34}}, "main", utils::Empty, ty.Of(output),
         utils::Vector{
             Return(Call(ty.Of(output))),
         },
         utils::Vector{
             Stage(ast::PipelineStage::kVertex),
         },
         utils::Vector{
             Location(Source{{13, 43}}, 0_a),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: cannot apply @location to declaration of type 'Output'
13:43 note: @location must only be applied to declarations of numeric scalar or numeric vector type)");
}

TEST_F(LocationAttributeTests, ReturnType_Struct_NestedStruct) {
    // struct Inner {
    //   @location(0) b : f32;
    // };
    // struct Output {
    //   a : Inner;
    // };
    // @fragment
    // fn main() -> Output { return Output(); }
    auto* inner = Structure(
        "Inner", utils::Vector{
                     Member(Source{{13, 43}}, "a", ty.f32(), utils::Vector{Location(0_a)}),
                 });
    auto* output = Structure("Output", utils::Vector{
                                           Member(Source{{14, 52}}, "a", ty.Of(inner)),
                                       });
    Func(Source{{12, 34}}, "main", utils::Empty, ty.Of(output),
         utils::Vector{
             Return(Call(ty.Of(output))),
         },
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(14:52 error: nested structures cannot be used for entry point IO
12:34 note: while analyzing entry point 'main')");
}

TEST_F(LocationAttributeTests, ReturnType_Struct_RuntimeArray) {
    // struct Output {
    //   @location(0) a : array<f32>;
    // };
    // @fragment
    // fn main() -> Output {
    //   return Output();
    // }
    auto* output = Structure("Output", utils::Vector{
                                           Member(Source{{13, 43}}, "a", ty.array<f32>(),
                                                  utils::Vector{Location(Source{{12, 34}}, 0_a)}),
                                       });
    Func(Source{{12, 34}}, "main", utils::Empty, ty.Of(output),
         utils::Vector{
             Return(Call(ty.Of(output))),
         },
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(13:43 error: cannot apply @location to declaration of type 'array<f32>'
12:34 note: @location must only be applied to declarations of numeric scalar or numeric vector type)");
}

TEST_F(LocationAttributeTests, ComputeShaderLocation_Input) {
    Func("main", utils::Empty, ty.i32(),
         utils::Vector{
             Return(Expr(1_i)),
         },
         utils::Vector{
             Stage(ast::PipelineStage::kCompute),
             create<ast::WorkgroupAttribute>(Source{{12, 34}}, Expr(1_i)),
         },
         utils::Vector{
             Location(Source{{12, 34}}, 1_a),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: @location is not valid for compute shader output)");
}

TEST_F(LocationAttributeTests, ComputeShaderLocation_Output) {
    auto* input = Param("input", ty.i32(),
                        utils::Vector{
                            Location(Source{{12, 34}}, 0_u),
                        });
    Func("main", utils::Vector{input}, ty.void_(), utils::Empty,
         utils::Vector{
             Stage(ast::PipelineStage::kCompute),
             create<ast::WorkgroupAttribute>(Source{{12, 34}}, Expr(1_i)),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: @location is not valid for compute shader inputs)");
}

TEST_F(LocationAttributeTests, ComputeShaderLocationStructMember_Output) {
    auto* m = Member("m", ty.i32(),
                     utils::Vector{
                         Location(Source{{12, 34}}, 0_u),
                     });
    auto* s = Structure("S", utils::Vector{m});
    Func(Source{{56, 78}}, "main", utils::Empty, ty.Of(s),
         utils::Vector{
             Return(Expr(Call(ty.Of(s)))),
         },
         utils::Vector{
             Stage(ast::PipelineStage::kCompute),
             create<ast::WorkgroupAttribute>(Source{{12, 34}}, Expr(1_i)),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: @location is not valid for compute shader output\n"
              "56:78 note: while analyzing entry point 'main'");
}

TEST_F(LocationAttributeTests, ComputeShaderLocationStructMember_Input) {
    auto* m = Member("m", ty.i32(),
                     utils::Vector{
                         Location(Source{{12, 34}}, 0_u),
                     });
    auto* s = Structure("S", utils::Vector{m});
    auto* input = Param("input", ty.Of(s));
    Func(Source{{56, 78}}, "main", utils::Vector{input}, ty.void_(), utils::Empty,
         utils::Vector{
             Stage(ast::PipelineStage::kCompute),
             create<ast::WorkgroupAttribute>(Source{{12, 34}}, Expr(1_i)),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: @location is not valid for compute shader inputs\n"
              "56:78 note: while analyzing entry point 'main'");
}

TEST_F(LocationAttributeTests, Duplicate_input) {
    // @fragment
    // fn main(@location(1) param_a : f32,
    //         @location(1) param_b : f32) {}
    auto* param_a = Param("param_a", ty.f32(),
                          utils::Vector{
                              Location(1_a),
                          });
    auto* param_b = Param("param_b", ty.f32(),
                          utils::Vector{
                              Location(Source{{12, 34}}, 1_a),
                          });
    Func(Source{{12, 34}}, "main",
         utils::Vector{
             param_a,
             param_b,
         },
         ty.void_(), utils::Empty,
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: @location(1) appears multiple times)");
}

TEST_F(LocationAttributeTests, Duplicate_struct) {
    // struct InputA {
    //   @location(1) a : f32;
    // };
    // struct InputB {
    //   @location(1) a : f32;
    // };
    // @fragment
    // fn main(param_a : InputA, param_b : InputB) {}
    auto* input_a = Structure("InputA", utils::Vector{
                                            Member("a", ty.f32(), utils::Vector{Location(1_a)}),
                                        });
    auto* input_b = Structure(
        "InputB", utils::Vector{
                      Member("a", ty.f32(), utils::Vector{Location(Source{{34, 56}}, 1_a)}),
                  });
    auto* param_a = Param("param_a", ty.Of(input_a));
    auto* param_b = Param("param_b", ty.Of(input_b));
    Func(Source{{12, 34}}, "main",
         utils::Vector{
             param_a,
             param_b,
         },
         ty.void_(), utils::Empty,
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(34:56 error: @location(1) appears multiple times
12:34 note: while analyzing entry point 'main')");
}

}  // namespace
}  // namespace LocationAttributeTests

}  // namespace
}  // namespace tint::resolver
