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

#include "src/tint/ast/builtin_texture_helper_test.h"
#include "src/tint/resolver/resolver_test_helper.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::resolver {
namespace {

using ResolverBuiltinValidationTest = ResolverTest;

TEST_F(ResolverBuiltinValidationTest, FunctionTypeMustMatchReturnStatementType_void_fail) {
    // fn func { return workgroupBarrier(); }
    Func("func", {}, ty.void_(),
         {
             Return(Call(Source{Source::Location{12, 34}}, "workgroupBarrier")),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: builtin 'workgroupBarrier' does not return a value");
}

TEST_F(ResolverBuiltinValidationTest, InvalidPipelineStageDirect) {
    // @stage(compute) @workgroup_size(1) fn func { return dpdx(1.0); }

    auto* dpdx =
        create<ast::CallExpression>(Source{{3, 4}}, Expr("dpdx"), ast::ExpressionList{Expr(1.0f)});
    Func(Source{{1, 2}}, "func", ast::VariableList{}, ty.void_(), {CallStmt(dpdx)},
         {Stage(ast::PipelineStage::kCompute), WorkgroupSize(1_i)});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "3:4 error: built-in cannot be used by compute pipeline stage");
}

TEST_F(ResolverBuiltinValidationTest, InvalidPipelineStageIndirect) {
    // fn f0 { return dpdx(1.0); }
    // fn f1 { f0(); }
    // fn f2 { f1(); }
    // @stage(compute) @workgroup_size(1) fn main { return f2(); }

    auto* dpdx =
        create<ast::CallExpression>(Source{{3, 4}}, Expr("dpdx"), ast::ExpressionList{Expr(1.0f)});
    Func(Source{{1, 2}}, "f0", {}, ty.void_(), {CallStmt(dpdx)});

    Func(Source{{3, 4}}, "f1", {}, ty.void_(), {CallStmt(Call("f0"))});

    Func(Source{{5, 6}}, "f2", {}, ty.void_(), {CallStmt(Call("f1"))});

    Func(Source{{7, 8}}, "main", {}, ty.void_(), {CallStmt(Call("f2"))},
         {Stage(ast::PipelineStage::kCompute), WorkgroupSize(1_i)});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(3:4 error: built-in cannot be used by compute pipeline stage
1:2 note: called by function 'f0'
3:4 note: called by function 'f1'
5:6 note: called by function 'f2'
7:8 note: called by entry point 'main')");
}

TEST_F(ResolverBuiltinValidationTest, BuiltinRedeclaredAsFunction) {
    Func(Source{{12, 34}}, "mix", {}, ty.i32(), {});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: 'mix' is a builtin and cannot be redeclared as a function)");
}

TEST_F(ResolverBuiltinValidationTest, BuiltinRedeclaredAsGlobalLet) {
    GlobalConst(Source{{12, 34}}, "mix", ty.i32(), Expr(1_i));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: 'mix' is a builtin and cannot be redeclared as a module-scope let)");
}

TEST_F(ResolverBuiltinValidationTest, BuiltinRedeclaredAsGlobalVar) {
    Global(Source{{12, 34}}, "mix", ty.i32(), Expr(1_i), ast::StorageClass::kPrivate);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: 'mix' is a builtin and cannot be redeclared as a module-scope var)");
}

TEST_F(ResolverBuiltinValidationTest, BuiltinRedeclaredAsAlias) {
    Alias(Source{{12, 34}}, "mix", ty.i32());

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: 'mix' is a builtin and cannot be redeclared as an alias)");
}

TEST_F(ResolverBuiltinValidationTest, BuiltinRedeclaredAsStruct) {
    Structure(Source{{12, 34}}, "mix", {Member("m", ty.i32())});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: 'mix' is a builtin and cannot be redeclared as a struct)");
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
                return b.Construct(src, b.ty.vec2<i32>(), i32(values[0]), i32(values[1]));
            case Kind::kVec3:
                return b.Construct(src, b.ty.vec3<i32>(), i32(values[0]), i32(values[1]),
                                   i32(values[2]));
            case Kind::kVec3_Scalar_Vec2:
                return b.Construct(src, b.ty.vec3<i32>(), i32(values[0]),
                                   b.vec2<i32>(i32(values[1]), i32(values[2])));
            case Kind::kVec3_Vec2_Scalar:
                return b.Construct(src, b.ty.vec3<i32>(),
                                   b.vec2<i32>(i32(values[0]), i32(values[1])), i32(values[2]));
            case Kind::kEmptyVec2:
                return b.Construct(src, b.ty.vec2<i32>());
            case Kind::kEmptyVec3:
                return b.Construct(src, b.ty.vec3<i32>());
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
    auto*& arg_to_replace = (param.position == Position::kFirst) ? args.front() : args.back();

    // BuildTextureVariable() uses a Literal for scalars, and a CallExpression for
    // a vector constructor.
    bool is_vector = arg_to_replace->Is<ast::CallExpression>();

    // Make the expression to be replaced, reachable. This keeps the resolver
    // happy.
    WrapInFunction(arg_to_replace);

    arg_to_replace = expr(Source{{12, 34}}, *this);

    // Call the builtin with the constexpr argument replaced
    Func("func", {}, ty.void_(), {CallStmt(Call(overload.function, args))},
         {Stage(ast::PipelineStage::kFragment)});

    if (expr.invalid_index == Constexpr::kValid) {
        EXPECT_TRUE(r()->Resolve()) << r()->error();
    } else {
        EXPECT_FALSE(r()->Resolve());
        std::stringstream err;
        if (is_vector) {
            err << "12:34 error: each component of the " << param.name
                << " argument must be at least " << param.min << " and at most " << param.max
                << ". " << param.name << " component " << expr.invalid_index << " is "
                << std::to_string(expr.values[expr.invalid_index]);
        } else {
            err << "12:34 error: the " << param.name << " argument must be at least " << param.min
                << " and at most " << param.max << ". " << param.name << " is "
                << std::to_string(expr.values[expr.invalid_index]);
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

    // Build the module-scope let 'G' with the offset value
    GlobalConst("G", nullptr, expr({}, *this));

    auto args = overload.args(this);
    auto*& arg_to_replace = (param.position == Position::kFirst) ? args.front() : args.back();

    // Make the expression to be replaced, reachable. This keeps the resolver
    // happy.
    WrapInFunction(arg_to_replace);

    arg_to_replace = Expr(Source{{12, 34}}, "G");

    // Call the builtin with the constexpr argument replaced
    Func("func", {}, ty.void_(), {CallStmt(Call(overload.function, args))},
         {Stage(ast::PipelineStage::kFragment)});

    EXPECT_FALSE(r()->Resolve());
    std::stringstream err;
    err << "12:34 error: the " << param.name << " argument must be a const_expression";
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
    testing::Combine(testing::ValuesIn(TextureCases({ValidTextureOverload::kGather2dF32,
                                                     ValidTextureOverload::kGather2dOffsetF32,
                                                     ValidTextureOverload::kGather2dArrayF32,
                                                     ValidTextureOverload::kGather2dArrayOffsetF32,
                                                     ValidTextureOverload::kGatherCubeF32,
                                                     ValidTextureOverload::kGatherCubeArrayF32})),
                     testing::Values(Parameter{"component", Position::kFirst, 0, 3}),
                     testing::Values(Constexpr{Constexpr::kValid, Constexpr::Kind::kScalar, 0},
                                     Constexpr{Constexpr::kValid, Constexpr::Kind::kScalar, 1},
                                     Constexpr{Constexpr::kValid, Constexpr::Kind::kScalar, 2},
                                     Constexpr{Constexpr::kValid, Constexpr::Kind::kScalar, 3},
                                     Constexpr{0, Constexpr::Kind::kScalar, 4},
                                     Constexpr{0, Constexpr::Kind::kScalar, 123},
                                     Constexpr{0, Constexpr::Kind::kScalar, -1})));

}  // namespace texture_constexpr_args

}  // namespace
}  // namespace tint::resolver
