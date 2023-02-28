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

#include <unordered_set>

#include "src/tint/ast/builtin_texture_helper_test.h"
#include "src/tint/resolver/resolver_test_helper.h"
#include "src/tint/sem/value_constructor.h"
#include "src/tint/utils/string_stream.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::resolver {
namespace {

using ResolverBuiltinValidationTest = ResolverTest;

TEST_F(ResolverBuiltinValidationTest, FunctionTypeMustMatchReturnStatementType_void_fail) {
    // fn func { return workgroupBarrier(); }
    Func("func", utils::Empty, ty.void_(),
         utils::Vector{
             Return(Call(Source{Source::Location{12, 34}}, "workgroupBarrier")),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: builtin 'workgroupBarrier' does not return a value");
}

TEST_F(ResolverBuiltinValidationTest, InvalidPipelineStageDirect) {
    // @compute @workgroup_size(1) fn func { return dpdx(1.0); }

    auto* dpdx = Call(Source{{3, 4}}, "dpdx", 1_f);
    Func(Source{{1, 2}}, "func", utils::Empty, ty.void_(),
         utils::Vector{
             Assign(Phony(), dpdx),
         },
         utils::Vector{
             Stage(ast::PipelineStage::kCompute),
             WorkgroupSize(1_i),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "3:4 error: built-in cannot be used by compute pipeline stage");
}

TEST_F(ResolverBuiltinValidationTest, InvalidPipelineStageIndirect) {
    // fn f0 { return dpdx(1.0); }
    // fn f1 { f0(); }
    // fn f2 { f1(); }
    // @compute @workgroup_size(1) fn main { return f2(); }

    auto* dpdx = Call(Source{{3, 4}}, "dpdx", 1_f);
    Func(Source{{1, 2}}, "f0", utils::Empty, ty.void_(),
         utils::Vector{
             Assign(Phony(), dpdx),
         });

    Func(Source{{3, 4}}, "f1", utils::Empty, ty.void_(),
         utils::Vector{
             CallStmt(Call("f0")),
         });

    Func(Source{{5, 6}}, "f2", utils::Empty, ty.void_(),
         utils::Vector{
             CallStmt(Call("f1")),
         });

    Func(Source{{7, 8}}, "main", utils::Empty, ty.void_(),
         utils::Vector{
             CallStmt(Call("f2")),
         },
         utils::Vector{
             Stage(ast::PipelineStage::kCompute),
             WorkgroupSize(1_i),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(3:4 error: built-in cannot be used by compute pipeline stage
1:2 note: called by function 'f0'
3:4 note: called by function 'f1'
5:6 note: called by function 'f2'
7:8 note: called by entry point 'main')");
}

TEST_F(ResolverBuiltinValidationTest, BuiltinRedeclaredAsFunctionUsedAsFunction) {
    auto* mix = Func(Source{{12, 34}}, "mix", utils::Empty, ty.i32(),
                     utils::Vector{
                         Return(1_i),
                     });
    auto* use = Call("mix");
    WrapInFunction(use);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    auto* sem = Sem().Get<sem::Call>(use);
    ASSERT_NE(sem, nullptr);
    EXPECT_EQ(sem->Target(), Sem().Get(mix));
}

TEST_F(ResolverBuiltinValidationTest, BuiltinRedeclaredAsFunctionUsedAsVariable) {
    Func(Source{{12, 34}}, "mix", utils::Empty, ty.i32(),
         utils::Vector{
             Return(1_i),
         });
    WrapInFunction(Decl(Var("v", Expr(Source{{56, 78}}, "mix"))));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(56:78 error: cannot use function 'mix' as value
12:34 note: function 'mix' declared here)");
}

TEST_F(ResolverBuiltinValidationTest, BuiltinRedeclaredAsGlobalConstUsedAsVariable) {
    auto* mix = GlobalConst(Source{{12, 34}}, "mix", ty.i32(), Expr(1_i));
    auto* use = Expr("mix");
    WrapInFunction(Decl(Var("v", use)));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    auto* sem = Sem().Get<sem::VariableUser>(use);
    ASSERT_NE(sem, nullptr);
    EXPECT_EQ(sem->Variable(), Sem().Get(mix));
}

TEST_F(ResolverBuiltinValidationTest, BuiltinRedeclaredAsGlobalVarUsedAsVariable) {
    auto* mix =
        GlobalVar(Source{{12, 34}}, "mix", ty.i32(), Expr(1_i), builtin::AddressSpace::kPrivate);
    auto* use = Expr("mix");
    WrapInFunction(Decl(Var("v", use)));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    auto* sem = Sem().GetVal(use)->UnwrapLoad()->As<sem::VariableUser>();
    ASSERT_NE(sem, nullptr);
    EXPECT_EQ(sem->Variable(), Sem().Get(mix));
}

TEST_F(ResolverBuiltinValidationTest, BuiltinRedeclaredAsAliasUsedAsFunction) {
    Alias(Source{{12, 34}}, "mix", ty.i32());
    WrapInFunction(Call(Source{{56, 78}}, "mix", 1_f, 2_f, 3_f));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(56:78 error: no matching constructor for i32(f32, f32, f32)

2 candidate constructors:
  i32(i32) -> i32
  i32() -> i32

1 candidate conversion:
  i32<T>(T) -> i32  where: T is abstract-int, abstract-float, f32, f16, u32 or bool
)");
}

TEST_F(ResolverBuiltinValidationTest, BuiltinRedeclaredAsAliasUsedAsType) {
    auto* mix = Alias(Source{{12, 34}}, "mix", ty.i32());
    auto* use = Call("mix");
    WrapInFunction(use);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    auto* sem = Sem().Get<sem::Call>(use);
    ASSERT_NE(sem, nullptr);
    EXPECT_EQ(sem->Type(), Sem().Get(mix));
}

TEST_F(ResolverBuiltinValidationTest, BuiltinRedeclaredAsStructUsedAsFunction) {
    Structure("mix", utils::Vector{
                         Member("m", ty.i32()),
                     });
    WrapInFunction(Call(Source{{12, 34}}, "mix", 1_f, 2_f, 3_f));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: structure constructor has too many inputs: expected 1, found 3)");
}

TEST_F(ResolverBuiltinValidationTest, BuiltinRedeclaredAsStructUsedAsType) {
    auto* mix = Structure("mix", utils::Vector{
                                     Member("m", ty.i32()),
                                 });
    auto* use = Call("mix");
    WrapInFunction(use);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    auto* sem = Sem().Get<sem::Call>(use);
    ASSERT_NE(sem, nullptr);
    EXPECT_EQ(sem->Type(), Sem().Get(mix));
}

namespace texture_constexpr_args {

using TextureOverloadCase = ast::builtin::test::TextureOverloadCase;
using ValidTextureOverload = ast::builtin::test::ValidTextureOverload;
using TextureKind = ast::builtin::test::TextureKind;
using TextureDataType = ast::builtin::test::TextureDataType;

static std::vector<TextureOverloadCase> TextureCases(
    std::unordered_set<ValidTextureOverload> overloads) {
    std::vector<TextureOverloadCase> cases;
    for (auto c : TextureOverloadCase::ValidCases()) {
        if (overloads.count(c.overload)) {
            cases.push_back(c);
        }
    }
    return cases;
}

enum class Position {
    kFirst,
    kLast,
};

struct Parameter {
    const char* const name;
    const Position position;
    int min;
    int max;
};

class Constexpr {
  public:
    enum class Kind {
        kScalar,
        kVec2,
        kVec3,
        kVec3_Scalar_Vec2,
        kVec3_Vec2_Scalar,
        kEmptyVec2,
        kEmptyVec3,
    };

    Constexpr(int32_t invalid_idx, Kind k, int32_t x = 0, int32_t y = 0, int32_t z = 0)
        : invalid_index(invalid_idx), kind(k), values{x, y, z} {}

    const ast::Expression* operator()(Source src, ProgramBuilder& b) {
        switch (kind) {
            case Kind::kScalar:
                return b.Expr(src, i32(values[0]));
            case Kind::kVec2:
                return b.Call(src, b.ty.vec2<i32>(), i32(values[0]), i32(values[1]));
            case Kind::kVec3:
                return b.Call(src, b.ty.vec3<i32>(), i32(values[0]), i32(values[1]),
                              i32(values[2]));
            case Kind::kVec3_Scalar_Vec2:
                return b.Call(src, b.ty.vec3<i32>(), i32(values[0]),
                              b.vec2<i32>(i32(values[1]), i32(values[2])));
            case Kind::kVec3_Vec2_Scalar:
                return b.Call(src, b.ty.vec3<i32>(), b.vec2<i32>(i32(values[0]), i32(values[1])),
                              i32(values[2]));
            case Kind::kEmptyVec2:
                return b.Call(src, b.ty.vec2<i32>());
            case Kind::kEmptyVec3:
                return b.Call(src, b.ty.vec3<i32>());
        }
        return nullptr;
    }

    static const constexpr int32_t kValid = -1;
    const int32_t invalid_index;  // Expected error value, or kValid
    const Kind kind;
    const std::array<int32_t, 3> values;
};

static std::ostream& operator<<(std::ostream& out, Parameter param) {
    return out << param.name;
}

static std::ostream& operator<<(std::ostream& out, Constexpr expr) {
    switch (expr.kind) {
        case Constexpr::Kind::kScalar:
            return out << expr.values[0];
        case Constexpr::Kind::kVec2:
            return out << "vec2(" << expr.values[0] << ", " << expr.values[1] << ")";
        case Constexpr::Kind::kVec3:
            return out << "vec3(" << expr.values[0] << ", " << expr.values[1] << ", "
                       << expr.values[2] << ")";
        case Constexpr::Kind::kVec3_Scalar_Vec2:
            return out << "vec3(" << expr.values[0] << ", vec2(" << expr.values[1] << ", "
                       << expr.values[2] << "))";
        case Constexpr::Kind::kVec3_Vec2_Scalar:
            return out << "vec3(vec2(" << expr.values[0] << ", " << expr.values[1] << "), "
                       << expr.values[2] << ")";
        case Constexpr::Kind::kEmptyVec2:
            return out << "vec2()";
        case Constexpr::Kind::kEmptyVec3:
            return out << "vec3()";
    }
    return out;
}

using BuiltinTextureConstExprArgValidationTest =
    ResolverTestWithParam<std::tuple<TextureOverloadCase, Parameter, Constexpr>>;

TEST_P(BuiltinTextureConstExprArgValidationTest, Immediate) {
    auto& p = GetParam();
    auto overload = std::get<0>(p);
    auto param = std::get<1>(p);
    auto expr = std::get<2>(p);

    overload.BuildTextureVariable(this);
    overload.BuildSamplerVariable(this);

    auto args = overload.args(this);
    auto*& arg_to_replace = (param.position == Position::kFirst) ? args.Front() : args.Back();

    // BuildTextureVariable() uses a Literal for scalars, and a CallExpression for a vector
    // constructor.
    bool is_vector = arg_to_replace->Is<ast::CallExpression>();

    // Make the expression to be replaced, reachable. This keeps the resolver happy.
    WrapInFunction(arg_to_replace);

    arg_to_replace = expr(Source{{12, 34}}, *this);

    auto* call = Call(overload.function, args);
    auto* stmt = overload.returns_value ? static_cast<const ast::Statement*>(Assign(Phony(), call))
                                        : static_cast<const ast::Statement*>(CallStmt(call));

    // Call the builtin with the constexpr argument replaced
    Func("func", utils::Empty, ty.void_(),
         utils::Vector{
             stmt,
         },
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    if (expr.invalid_index == Constexpr::kValid) {
        EXPECT_TRUE(r()->Resolve()) << r()->error();
    } else {
        EXPECT_FALSE(r()->Resolve());
        utils::StringStream err;
        if (is_vector) {
            err << "12:34 error: each component of the " << param.name
                << " argument must be at least " << param.min << " and at most " << param.max
                << ". " << param.name << " component " << expr.invalid_index << " is "
                << std::to_string(expr.values[static_cast<size_t>(expr.invalid_index)]);
        } else {
            err << "12:34 error: the " << param.name << " argument must be at least " << param.min
                << " and at most " << param.max << ". " << param.name << " is "
                << std::to_string(expr.values[static_cast<size_t>(expr.invalid_index)]);
        }
        EXPECT_EQ(r()->error(), err.str());
    }
}

TEST_P(BuiltinTextureConstExprArgValidationTest, GlobalConst) {
    auto& p = GetParam();
    auto overload = std::get<0>(p);
    auto param = std::get<1>(p);
    auto expr = std::get<2>(p);

    // Build the global texture and sampler variables
    overload.BuildTextureVariable(this);
    overload.BuildSamplerVariable(this);

    // Build the module-scope const 'G' with the offset value
    GlobalConst("G", expr({}, *this));

    auto args = overload.args(this);
    auto*& arg_to_replace = (param.position == Position::kFirst) ? args.Front() : args.Back();

    // BuildTextureVariable() uses a Literal for scalars, and a CallExpression for a vector
    // constructor.
    bool is_vector = arg_to_replace->Is<ast::CallExpression>();

    // Make the expression to be replaced, reachable. This keeps the resolver happy.
    WrapInFunction(arg_to_replace);

    arg_to_replace = Expr(Source{{12, 34}}, "G");

    auto* call = Call(overload.function, args);
    auto* stmt = overload.returns_value ? static_cast<const ast::Statement*>(Assign(Phony(), call))
                                        : static_cast<const ast::Statement*>(CallStmt(call));

    // Call the builtin with the constant-expression argument replaced
    Func("func", utils::Empty, ty.void_(),
         utils::Vector{
             stmt,
         },
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    if (expr.invalid_index == Constexpr::kValid) {
        EXPECT_TRUE(r()->Resolve()) << r()->error();
    } else {
        EXPECT_FALSE(r()->Resolve());
        utils::StringStream err;
        if (is_vector) {
            err << "12:34 error: each component of the " << param.name
                << " argument must be at least " << param.min << " and at most " << param.max
                << ". " << param.name << " component " << expr.invalid_index << " is "
                << std::to_string(expr.values[static_cast<size_t>(expr.invalid_index)]);
        } else {
            err << "12:34 error: the " << param.name << " argument must be at least " << param.min
                << " and at most " << param.max << ". " << param.name << " is "
                << std::to_string(expr.values[static_cast<size_t>(expr.invalid_index)]);
        }
        EXPECT_EQ(r()->error(), err.str());
    }
}

TEST_P(BuiltinTextureConstExprArgValidationTest, GlobalVar) {
    auto& p = GetParam();
    auto overload = std::get<0>(p);
    auto param = std::get<1>(p);
    auto expr = std::get<2>(p);

    // Build the global texture and sampler variables
    overload.BuildTextureVariable(this);
    overload.BuildSamplerVariable(this);

    // Build the module-scope var 'G' with the offset value
    GlobalVar("G", expr({}, *this), builtin::AddressSpace::kPrivate);

    auto args = overload.args(this);
    auto*& arg_to_replace = (param.position == Position::kFirst) ? args.Front() : args.Back();

    // Make the expression to be replaced, reachable. This keeps the resolver happy.
    WrapInFunction(arg_to_replace);

    arg_to_replace = Expr(Source{{12, 34}}, "G");

    auto* call = Call(overload.function, args);
    auto* stmt = overload.returns_value ? static_cast<const ast::Statement*>(Assign(Phony(), call))
                                        : static_cast<const ast::Statement*>(CallStmt(call));

    // Call the builtin with the constant-expression argument replaced
    Func("func", utils::Empty, ty.void_(),
         utils::Vector{
             stmt,
         },
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    EXPECT_FALSE(r()->Resolve());
    utils::StringStream err;
    err << "12:34 error: the " << param.name << " argument must be a const-expression";
    EXPECT_EQ(r()->error(), err.str());
}
INSTANTIATE_TEST_SUITE_P(
    Offset2D,
    BuiltinTextureConstExprArgValidationTest,
    testing::Combine(testing::ValuesIn(TextureCases({
                         ValidTextureOverload::kSample2dOffsetF32,
                         ValidTextureOverload::kSample2dArrayOffsetF32,
                         ValidTextureOverload::kSampleDepth2dOffsetF32,
                         ValidTextureOverload::kSampleDepth2dArrayOffsetF32,
                         ValidTextureOverload::kSampleBias2dOffsetF32,
                         ValidTextureOverload::kSampleBias2dArrayOffsetF32,
                         ValidTextureOverload::kSampleLevel2dOffsetF32,
                         ValidTextureOverload::kSampleLevel2dArrayOffsetF32,
                         ValidTextureOverload::kSampleLevelDepth2dOffsetF32,
                         ValidTextureOverload::kSampleLevelDepth2dArrayOffsetF32,
                         ValidTextureOverload::kSampleGrad2dOffsetF32,
                         ValidTextureOverload::kSampleGrad2dArrayOffsetF32,
                         ValidTextureOverload::kSampleCompareDepth2dOffsetF32,
                         ValidTextureOverload::kSampleCompareDepth2dArrayOffsetF32,
                         ValidTextureOverload::kSampleCompareLevelDepth2dOffsetF32,
                         ValidTextureOverload::kSampleCompareLevelDepth2dArrayOffsetF32,
                     })),
                     testing::Values(Parameter{"offset", Position::kLast, -8, 7}),
                     testing::Values(Constexpr{Constexpr::kValid, Constexpr::Kind::kEmptyVec2},
                                     Constexpr{Constexpr::kValid, Constexpr::Kind::kVec2, -1, 1},
                                     Constexpr{Constexpr::kValid, Constexpr::Kind::kVec2, 7, -8},
                                     Constexpr{0, Constexpr::Kind::kVec2, 8, 0},
                                     Constexpr{1, Constexpr::Kind::kVec2, 0, 8},
                                     Constexpr{0, Constexpr::Kind::kVec2, -9, 0},
                                     Constexpr{1, Constexpr::Kind::kVec2, 0, -9},
                                     Constexpr{0, Constexpr::Kind::kVec2, 8, 8},
                                     Constexpr{0, Constexpr::Kind::kVec2, -9, -9})));

INSTANTIATE_TEST_SUITE_P(
    Offset3D,
    BuiltinTextureConstExprArgValidationTest,
    testing::Combine(testing::ValuesIn(TextureCases({
                         ValidTextureOverload::kSample3dOffsetF32,
                         ValidTextureOverload::kSampleBias3dOffsetF32,
                         ValidTextureOverload::kSampleLevel3dOffsetF32,
                         ValidTextureOverload::kSampleGrad3dOffsetF32,
                     })),
                     testing::Values(Parameter{"offset", Position::kLast, -8, 7}),
                     testing::Values(Constexpr{Constexpr::kValid, Constexpr::Kind::kEmptyVec3},
                                     Constexpr{Constexpr::kValid, Constexpr::Kind::kVec3, 0, 0, 0},
                                     Constexpr{Constexpr::kValid, Constexpr::Kind::kVec3, 7, -8, 7},
                                     Constexpr{0, Constexpr::Kind::kVec3, 10, 0, 0},
                                     Constexpr{1, Constexpr::Kind::kVec3, 0, 10, 0},
                                     Constexpr{2, Constexpr::Kind::kVec3, 0, 0, 10},
                                     Constexpr{0, Constexpr::Kind::kVec3, 10, 11, 12},
                                     Constexpr{0, Constexpr::Kind::kVec3_Scalar_Vec2, 10, 0, 0},
                                     Constexpr{1, Constexpr::Kind::kVec3_Scalar_Vec2, 0, 10, 0},
                                     Constexpr{2, Constexpr::Kind::kVec3_Scalar_Vec2, 0, 0, 10},
                                     Constexpr{0, Constexpr::Kind::kVec3_Scalar_Vec2, 10, 11, 12},
                                     Constexpr{0, Constexpr::Kind::kVec3_Vec2_Scalar, 10, 0, 0},
                                     Constexpr{1, Constexpr::Kind::kVec3_Vec2_Scalar, 0, 10, 0},
                                     Constexpr{2, Constexpr::Kind::kVec3_Vec2_Scalar, 0, 0, 10},
                                     Constexpr{0, Constexpr::Kind::kVec3_Vec2_Scalar, 10, 11,
                                               12})));

INSTANTIATE_TEST_SUITE_P(
    Component,
    BuiltinTextureConstExprArgValidationTest,
    testing::Combine(
        testing::ValuesIn(TextureCases({
            ValidTextureOverload::kGather2dF32, ValidTextureOverload::kGather2dOffsetF32,
            ValidTextureOverload::kGather2dArrayF32, ValidTextureOverload::kGatherCubeF32,
            // The below require mixed integer signedness.
            // See https://github.com/gpuweb/gpuweb/issues/3536
            // ValidTextureOverload::kGather2dArrayOffsetF32,
            // ValidTextureOverload::kGatherCubeArrayF32,
        })),
        testing::Values(Parameter{"component", Position::kFirst, 0, 3}),
        testing::Values(Constexpr{Constexpr::kValid, Constexpr::Kind::kScalar, 0},
                        Constexpr{Constexpr::kValid, Constexpr::Kind::kScalar, 1},
                        Constexpr{Constexpr::kValid, Constexpr::Kind::kScalar, 2},
                        Constexpr{Constexpr::kValid, Constexpr::Kind::kScalar, 3},
                        Constexpr{0, Constexpr::Kind::kScalar, 4},
                        Constexpr{0, Constexpr::Kind::kScalar, 123},
                        Constexpr{0, Constexpr::Kind::kScalar, -1})));

}  // namespace texture_constexpr_args

// TODO(crbug.com/tint/1497): Update or remove ResolverDP4aExtensionValidationTest when the
// experimental extension chromium_experimental_dp4a is not needed.
using ResolverDP4aExtensionValidationTest = ResolverTest;

TEST_F(ResolverDP4aExtensionValidationTest, Dot4I8PackedWithExtension) {
    // enable chromium_experimental_dp4a;
    // fn func { return dot4I8Packed(1u, 2u); }
    Enable(builtin::Extension::kChromiumExperimentalDp4A);

    Func("func", utils::Empty, ty.i32(),
         utils::Vector{
             Return(Call(Source{Source::Location{12, 34}}, "dot4I8Packed",
                         utils::Vector{Expr(1_u), Expr(2_u)})),
         });

    EXPECT_TRUE(r()->Resolve());
}

TEST_F(ResolverDP4aExtensionValidationTest, Dot4I8PackedWithoutExtension) {
    // fn func { return dot4I8Packed(1u, 2u); }
    Func("func", utils::Empty, ty.i32(),
         utils::Vector{
             Return(Call(Source{Source::Location{12, 34}}, "dot4I8Packed",
                         utils::Vector{Expr(1_u), Expr(2_u)})),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: cannot call built-in function 'dot4I8Packed' without extension chromium_experimental_dp4a)");
}

TEST_F(ResolverDP4aExtensionValidationTest, Dot4U8PackedWithExtension) {
    // enable chromium_experimental_dp4a;
    // fn func { return dot4U8Packed(1u, 2u); }
    Enable(builtin::Extension::kChromiumExperimentalDp4A);

    Func("func", utils::Empty, ty.u32(),
         utils::Vector{
             Return(Call(Source{Source::Location{12, 34}}, "dot4U8Packed",
                         utils::Vector{Expr(1_u), Expr(2_u)})),
         });

    EXPECT_TRUE(r()->Resolve());
}

TEST_F(ResolverDP4aExtensionValidationTest, Dot4U8PackedWithoutExtension) {
    // fn func { return dot4U8Packed(1u, 2u); }
    Func("func", utils::Empty, ty.u32(),
         utils::Vector{
             Return(Call(Source{Source::Location{12, 34}}, "dot4U8Packed",
                         utils::Vector{Expr(1_u), Expr(2_u)})),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: cannot call built-in function 'dot4U8Packed' without extension chromium_experimental_dp4a)");
}

TEST_F(ResolverBuiltinValidationTest, WorkgroupUniformLoad_WrongAddressSpace) {
    // @group(0) @binding(0) var<storage, read_write> v : i32;
    // fn foo() {
    //   workgroupUniformLoad(&v);
    // }
    GlobalVar("v", ty.i32(), builtin::AddressSpace::kStorage, builtin::Access::kReadWrite,
              utils::Vector{Group(0_a), Binding(0_a)});
    WrapInFunction(CallStmt(Call("workgroupUniformLoad", AddressOf(Source{{12, 34}}, "v"))));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(error: no matching call to workgroupUniformLoad(ptr<storage, i32, read_write>)

1 candidate function:
  workgroupUniformLoad(ptr<workgroup, T, read_write>) -> T
)");
}

TEST_F(ResolverBuiltinValidationTest, WorkgroupUniformLoad_Atomic) {
    // var<workgroup> v : atomic<i32>;
    // fn foo() {
    //   workgroupUniformLoad(&v);
    // }
    GlobalVar("v", ty.atomic<i32>(), builtin::AddressSpace::kWorkgroup);
    WrapInFunction(CallStmt(Call("workgroupUniformLoad", AddressOf(Source{{12, 34}}, "v"))));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: workgroupUniformLoad must not be called with an argument that contains an atomic type)");
}

TEST_F(ResolverBuiltinValidationTest, WorkgroupUniformLoad_AtomicInArray) {
    // var<workgroup> v : array<atomic<i32>, 4>;
    // fn foo() {
    //   workgroupUniformLoad(&v);
    // }
    GlobalVar("v", ty.array(ty.atomic<i32>(), 4_a), builtin::AddressSpace::kWorkgroup);
    WrapInFunction(CallStmt(Call("workgroupUniformLoad", AddressOf(Source{{12, 34}}, "v"))));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: workgroupUniformLoad must not be called with an argument that contains an atomic type)");
}

TEST_F(ResolverBuiltinValidationTest, WorkgroupUniformLoad_AtomicInStruct) {
    // struct Inner { a : array<atomic<i32, 4> }
    // struct S { i : Inner }
    // var<workgroup> v : array<S, 4>;
    // fn foo() {
    //   workgroupUniformLoad(&v);
    // }
    Structure("Inner", utils::Vector{Member("a", ty.array(ty.atomic<i32>(), 4_a))});
    Structure("S", utils::Vector{Member("i", ty("Inner"))});
    GlobalVar(Source{{12, 34}}, "v", ty.array(ty("S"), 4_a), builtin::AddressSpace::kWorkgroup);
    WrapInFunction(CallStmt(Call("workgroupUniformLoad", AddressOf("v"))));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(error: workgroupUniformLoad must not be called with an argument that contains an atomic type)");
}

}  // namespace
}  // namespace tint::resolver
