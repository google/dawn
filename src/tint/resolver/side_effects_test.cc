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

#include "src/tint/resolver/resolver.h"

#include "gtest/gtest.h"
#include "src/tint/resolver/resolver_test_helper.h"
#include "src/tint/sem/expression.h"
#include "src/tint/sem/index_accessor_expression.h"
#include "src/tint/sem/member_accessor_expression.h"
#include "src/tint/utils/vector.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::resolver {
namespace {

struct SideEffectsTest : ResolverTest {
    template <typename T>
    void MakeSideEffectFunc(const char* name) {
        auto global = Sym();
        GlobalVar(global, ty.Of<T>(), ast::AddressSpace::kPrivate);
        auto local = Sym();
        Func(name, utils::Empty, ty.Of<T>(),
             utils::Vector{
                 Decl(Var(local, ty.Of<T>())),
                 Assign(global, local),
                 Return(global),
             });
    }

    template <typename MAKE_TYPE_FUNC>
    void MakeSideEffectFunc(const char* name, MAKE_TYPE_FUNC make_type) {
        auto global = Sym();
        GlobalVar(global, make_type(), ast::AddressSpace::kPrivate);
        auto local = Sym();
        Func(name, utils::Empty, make_type(),
             utils::Vector{
                 Decl(Var(local, make_type())),
                 Assign(global, local),
                 Return(global),
             });
    }
};

TEST_F(SideEffectsTest, Phony) {
    auto* expr = Phony();
    auto* body = Assign(expr, 1_i);
    WrapInFunction(body);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    EXPECT_FALSE(sem->HasSideEffects());
}

TEST_F(SideEffectsTest, Literal) {
    auto* expr = Expr(1_i);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    EXPECT_FALSE(sem->HasSideEffects());
}

TEST_F(SideEffectsTest, VariableUser) {
    auto* var = Decl(Var("a", ty.i32()));
    auto* expr = Expr("a");
    WrapInFunction(var, expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    EXPECT_TRUE(sem->Is<sem::VariableUser>());
    EXPECT_FALSE(sem->HasSideEffects());
}

TEST_F(SideEffectsTest, Call_Builtin_NoSE) {
    GlobalVar("a", ty.f32(), ast::AddressSpace::kPrivate);
    auto* expr = Call("dpdx", "a");
    Func("f", utils::Empty, ty.void_(), utils::Vector{Ignore(expr)},
         utils::Vector{create<ast::StageAttribute>(ast::PipelineStage::kFragment)});

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    EXPECT_TRUE(sem->Is<sem::Call>());
    EXPECT_FALSE(sem->HasSideEffects());
}

TEST_F(SideEffectsTest, Call_Builtin_NoSE_WithSEArg) {
    MakeSideEffectFunc<f32>("se");
    auto* expr = Call("dpdx", Call("se"));
    Func("f", utils::Empty, ty.void_(), utils::Vector{Ignore(expr)},
         utils::Vector{create<ast::StageAttribute>(ast::PipelineStage::kFragment)});

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    EXPECT_TRUE(sem->Is<sem::Call>());
    EXPECT_TRUE(sem->HasSideEffects());
}

TEST_F(SideEffectsTest, Call_Builtin_SE) {
    GlobalVar("a", ty.atomic(ty.i32()), ast::AddressSpace::kWorkgroup);
    auto* expr = Call("atomicAdd", AddressOf("a"), 1_i);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    EXPECT_TRUE(sem->Is<sem::Call>());
    EXPECT_TRUE(sem->HasSideEffects());
}

namespace builtin {
struct Case {
    const char* name;
    utils::Vector<const char*, 3> args;
    bool has_side_effects;
    ast::PipelineStage pipeline_stage;
};
static Case C(const char* name,
              utils::VectorRef<const char*> args,
              bool has_side_effects,
              ast::PipelineStage stage = ast::PipelineStage::kFragment) {
    Case c;
    c.name = name;
    c.args = std::move(args);
    c.has_side_effects = has_side_effects;
    c.pipeline_stage = stage;
    return c;
}
static std::ostream& operator<<(std::ostream& o, const Case& c) {
    o << c.name << "(";
    for (size_t i = 0; i < c.args.Length(); ++i) {
        o << c.args[i];
        if (i + 1 != c.args.Length()) {
            o << ", ";
        }
    }
    o << "), ";
    o << "has_side_effects = " << c.has_side_effects;
    return o;
}

using SideEffectsBuiltinTest = resolver::ResolverTestWithParam<Case>;

TEST_P(SideEffectsBuiltinTest, Test) {
    Enable(ast::Extension::kChromiumExperimentalDp4A);
    auto& c = GetParam();

    uint32_t next_binding = 0;
    GlobalVar("f", ty.f32(), ast::AddressSpace::kPrivate);
    GlobalVar("i", ty.i32(), ast::AddressSpace::kPrivate);
    GlobalVar("u", ty.u32(), ast::AddressSpace::kPrivate);
    GlobalVar("b", ty.bool_(), ast::AddressSpace::kPrivate);
    GlobalVar("vf", ty.vec3<f32>(), ast::AddressSpace::kPrivate);
    GlobalVar("vf2", ty.vec2<f32>(), ast::AddressSpace::kPrivate);
    GlobalVar("vi2", ty.vec2<i32>(), ast::AddressSpace::kPrivate);
    GlobalVar("vf4", ty.vec4<f32>(), ast::AddressSpace::kPrivate);
    GlobalVar("vb", ty.vec3<bool>(), ast::AddressSpace::kPrivate);
    GlobalVar("m", ty.mat3x3<f32>(), ast::AddressSpace::kPrivate);
    GlobalVar("arr", ty.array<f32, 10>(), ast::AddressSpace::kPrivate);
    GlobalVar("storage_arr", ty.array<f32>(), ast::AddressSpace::kStorage, Group(0_a),
              Binding(AInt(next_binding++)));
    GlobalVar("a", ty.atomic(ty.i32()), ast::AddressSpace::kStorage, ast::Access::kReadWrite,
              Group(0_a), Binding(AInt(next_binding++)));
    if (c.pipeline_stage != ast::PipelineStage::kCompute) {
        GlobalVar("t2d", ty.sampled_texture(ast::TextureDimension::k2d, ty.f32()), Group(0_a),
                  Binding(AInt(next_binding++)));
        GlobalVar("tdepth2d", ty.depth_texture(ast::TextureDimension::k2d), Group(0_a),
                  Binding(AInt(next_binding++)));
        GlobalVar("t2d_arr", ty.sampled_texture(ast::TextureDimension::k2dArray, ty.f32()),
                  Group(0_a), Binding(AInt(next_binding++)));
        GlobalVar("t2d_multi", ty.multisampled_texture(ast::TextureDimension::k2d, ty.f32()),
                  Group(0_a), Binding(AInt(next_binding++)));
        GlobalVar("tstorage2d",
                  ty.storage_texture(ast::TextureDimension::k2d, ast::TexelFormat::kR32Float,
                                     ast::Access::kWrite),
                  Group(0_a), Binding(AInt(next_binding++)));
        GlobalVar("s2d", ty.sampler(ast::SamplerKind::kSampler), Group(0_a),
                  Binding(AInt(next_binding++)));
        GlobalVar("scomp", ty.sampler(ast::SamplerKind::kComparisonSampler), Group(0_a),
                  Binding(AInt(next_binding++)));
    }

    utils::Vector<const ast::Statement*, 4> stmts;
    stmts.Push(Decl(Let("pstorage_arr", AddressOf("storage_arr"))));
    stmts.Push(Decl(Let("pa", AddressOf("a"))));

    utils::Vector<const ast::Expression*, 5> args;
    for (auto& a : c.args) {
        args.Push(Expr(a));
    }
    auto* expr = Call(c.name, args);

    utils::Vector<const ast::Attribute*, 2> attrs;
    attrs.Push(create<ast::StageAttribute>(c.pipeline_stage));
    if (c.pipeline_stage == ast::PipelineStage::kCompute) {
        attrs.Push(WorkgroupSize(Expr(1_u)));
    }

    stmts.Push(create<ast::CallStatement>(expr));

    Func("func", utils::Empty, ty.void_(), stmts, attrs);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    EXPECT_TRUE(sem->Is<sem::Call>());
    EXPECT_EQ(c.has_side_effects, sem->HasSideEffects());
}
INSTANTIATE_TEST_SUITE_P(
    SideEffectsTest_Builtins,
    SideEffectsBuiltinTest,
    testing::ValuesIn(std::vector<Case>{
        // No side-effect builts
        C("abs", utils::Vector{"f"}, false),                                                    //
        C("acos", utils::Vector{"f"}, false),                                                   //
        C("acosh", utils::Vector{"f"}, false),                                                  //
        C("all", utils::Vector{"vb"}, false),                                                   //
        C("any", utils::Vector{"vb"}, false),                                                   //
        C("arrayLength", utils::Vector{"pstorage_arr"}, false),                                 //
        C("asin", utils::Vector{"f"}, false),                                                   //
        C("asinh", utils::Vector{"f"}, false),                                                  //
        C("atan", utils::Vector{"f"}, false),                                                   //
        C("atan2", utils::Vector{"f", "f"}, false),                                             //
        C("atanh", utils::Vector{"f"}, false),                                                  //
        C("atomicLoad", utils::Vector{"pa"}, false),                                            //
        C("ceil", utils::Vector{"f"}, false),                                                   //
        C("clamp", utils::Vector{"f", "f", "f"}, false),                                        //
        C("cos", utils::Vector{"f"}, false),                                                    //
        C("cosh", utils::Vector{"f"}, false),                                                   //
        C("countLeadingZeros", utils::Vector{"i"}, false),                                      //
        C("countOneBits", utils::Vector{"i"}, false),                                           //
        C("countTrailingZeros", utils::Vector{"i"}, false),                                     //
        C("cross", utils::Vector{"vf", "vf"}, false),                                           //
        C("degrees", utils::Vector{"f"}, false),                                                //
        C("determinant", utils::Vector{"m"}, false),                                            //
        C("distance", utils::Vector{"f", "f"}, false),                                          //
        C("dot", utils::Vector{"vf", "vf"}, false),                                             //
        C("dot4I8Packed", utils::Vector{"u", "u"}, false),                                      //
        C("dot4U8Packed", utils::Vector{"u", "u"}, false),                                      //
        C("exp", utils::Vector{"f"}, false),                                                    //
        C("exp2", utils::Vector{"f"}, false),                                                   //
        C("extractBits", utils::Vector{"i", "u", "u"}, false),                                  //
        C("faceForward", utils::Vector{"vf", "vf", "vf"}, false),                               //
        C("firstLeadingBit", utils::Vector{"u"}, false),                                        //
        C("firstTrailingBit", utils::Vector{"u"}, false),                                       //
        C("floor", utils::Vector{"f"}, false),                                                  //
        C("fma", utils::Vector{"f", "f", "f"}, false),                                          //
        C("fract", utils::Vector{"vf"}, false),                                                 //
        C("frexp", utils::Vector{"f"}, false),                                                  //
        C("insertBits", utils::Vector{"i", "i", "u", "u"}, false),                              //
        C("inverseSqrt", utils::Vector{"f"}, false),                                            //
        C("ldexp", utils::Vector{"f", "i"}, false),                                             //
        C("length", utils::Vector{"vf"}, false),                                                //
        C("log", utils::Vector{"f"}, false),                                                    //
        C("log2", utils::Vector{"f"}, false),                                                   //
        C("max", utils::Vector{"f", "f"}, false),                                               //
        C("min", utils::Vector{"f", "f"}, false),                                               //
        C("mix", utils::Vector{"f", "f", "f"}, false),                                          //
        C("modf", utils::Vector{"f"}, false),                                                   //
        C("normalize", utils::Vector{"vf"}, false),                                             //
        C("pack2x16float", utils::Vector{"vf2"}, false),                                        //
        C("pack2x16snorm", utils::Vector{"vf2"}, false),                                        //
        C("pack2x16unorm", utils::Vector{"vf2"}, false),                                        //
        C("pack4x8snorm", utils::Vector{"vf4"}, false),                                         //
        C("pack4x8unorm", utils::Vector{"vf4"}, false),                                         //
        C("pow", utils::Vector{"f", "f"}, false),                                               //
        C("radians", utils::Vector{"f"}, false),                                                //
        C("reflect", utils::Vector{"vf", "vf"}, false),                                         //
        C("refract", utils::Vector{"vf", "vf", "f"}, false),                                    //
        C("reverseBits", utils::Vector{"u"}, false),                                            //
        C("round", utils::Vector{"f"}, false),                                                  //
        C("select", utils::Vector{"f", "f", "b"}, false),                                       //
        C("sign", utils::Vector{"f"}, false),                                                   //
        C("sin", utils::Vector{"f"}, false),                                                    //
        C("sinh", utils::Vector{"f"}, false),                                                   //
        C("smoothstep", utils::Vector{"f", "f", "f"}, false),                                   //
        C("sqrt", utils::Vector{"f"}, false),                                                   //
        C("step", utils::Vector{"f", "f"}, false),                                              //
        C("tan", utils::Vector{"f"}, false),                                                    //
        C("tanh", utils::Vector{"f"}, false),                                                   //
        C("textureDimensions", utils::Vector{"t2d"}, false),                                    //
        C("textureGather", utils::Vector{"tdepth2d", "s2d", "vf2"}, false),                     //
        C("textureGatherCompare", utils::Vector{"tdepth2d", "scomp", "vf2", "f"}, false),       //
        C("textureLoad", utils::Vector{"t2d", "vi2", "i"}, false),                              //
        C("textureNumLayers", utils::Vector{"t2d_arr"}, false),                                 //
        C("textureNumLevels", utils::Vector{"t2d"}, false),                                     //
        C("textureNumSamples", utils::Vector{"t2d_multi"}, false),                              //
        C("textureSampleCompareLevel", utils::Vector{"tdepth2d", "scomp", "vf2", "f"}, false),  //
        C("textureSampleGrad", utils::Vector{"t2d", "s2d", "vf2", "vf2", "vf2"}, false),        //
        C("textureSampleLevel", utils::Vector{"t2d", "s2d", "vf2", "f"}, false),                //
        C("transpose", utils::Vector{"m"}, false),                                              //
        C("trunc", utils::Vector{"f"}, false),                                                  //
        C("unpack2x16float", utils::Vector{"u"}, false),                                        //
        C("unpack2x16snorm", utils::Vector{"u"}, false),                                        //
        C("unpack2x16unorm", utils::Vector{"u"}, false),                                        //
        C("unpack4x8snorm", utils::Vector{"u"}, false),                                         //
        C("unpack4x8unorm", utils::Vector{"u"}, false),                                         //
        C("storageBarrier", utils::Empty, false, ast::PipelineStage::kCompute),                 //
        C("workgroupBarrier", utils::Empty, false, ast::PipelineStage::kCompute),               //
        C("textureSample", utils::Vector{"t2d", "s2d", "vf2"}, false),                          //
        C("textureSampleBias", utils::Vector{"t2d", "s2d", "vf2", "f"}, false),                 //
        C("textureSampleCompare", utils::Vector{"tdepth2d", "scomp", "vf2", "f"}, false),       //
        C("dpdx", utils::Vector{"f"}, false),                                                   //
        C("dpdxCoarse", utils::Vector{"f"}, false),                                             //
        C("dpdxFine", utils::Vector{"f"}, false),                                               //
        C("dpdy", utils::Vector{"f"}, false),                                                   //
        C("dpdyCoarse", utils::Vector{"f"}, false),                                             //
        C("dpdyFine", utils::Vector{"f"}, false),                                               //
        C("fwidth", utils::Vector{"f"}, false),                                                 //
        C("fwidthCoarse", utils::Vector{"f"}, false),                                           //
        C("fwidthFine", utils::Vector{"f"}, false),                                             //

        // Side-effect builtins
        C("atomicAdd", utils::Vector{"pa", "i"}, true),                       //
        C("atomicAnd", utils::Vector{"pa", "i"}, true),                       //
        C("atomicCompareExchangeWeak", utils::Vector{"pa", "i", "i"}, true),  //
        C("atomicExchange", utils::Vector{"pa", "i"}, true),                  //
        C("atomicMax", utils::Vector{"pa", "i"}, true),                       //
        C("atomicMin", utils::Vector{"pa", "i"}, true),                       //
        C("atomicOr", utils::Vector{"pa", "i"}, true),                        //
        C("atomicStore", utils::Vector{"pa", "i"}, true),                     //
        C("atomicSub", utils::Vector{"pa", "i"}, true),                       //
        C("atomicXor", utils::Vector{"pa", "i"}, true),                       //
        C("textureStore", utils::Vector{"tstorage2d", "vi2", "vf4"}, true),   //

        // Unimplemented builtins
        // C("quantizeToF16", utils::Vector{"f"}, false), //
        // C("saturate", utils::Vector{"f"}, false), //
    }));

}  // namespace builtin

TEST_F(SideEffectsTest, Call_Function) {
    Func("f", utils::Empty, ty.i32(), utils::Vector{Return(1_i)});
    auto* expr = Call("f");
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    EXPECT_TRUE(sem->Is<sem::Call>());
    EXPECT_TRUE(sem->HasSideEffects());
}

TEST_F(SideEffectsTest, Call_TypeConversion_NoSE) {
    auto* var = Decl(Var("a", ty.i32()));
    auto* expr = Construct(ty.f32(), "a");
    WrapInFunction(var, expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    EXPECT_TRUE(sem->Is<sem::Call>());
    EXPECT_FALSE(sem->HasSideEffects());
}

TEST_F(SideEffectsTest, Call_TypeConversion_SE) {
    MakeSideEffectFunc<i32>("se");
    auto* expr = Construct(ty.f32(), Call("se"));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    EXPECT_TRUE(sem->Is<sem::Call>());
    EXPECT_TRUE(sem->HasSideEffects());
}

TEST_F(SideEffectsTest, Call_TypeConstructor_NoSE) {
    auto* var = Decl(Var("a", ty.f32()));
    auto* expr = Construct(ty.f32(), "a");
    WrapInFunction(var, expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    EXPECT_TRUE(sem->Is<sem::Call>());
    EXPECT_FALSE(sem->HasSideEffects());
}

TEST_F(SideEffectsTest, Call_TypeConstructor_SE) {
    MakeSideEffectFunc<f32>("se");
    auto* expr = Construct(ty.f32(), Call("se"));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    EXPECT_TRUE(sem->Is<sem::Call>());
    EXPECT_TRUE(sem->HasSideEffects());
}

TEST_F(SideEffectsTest, MemberAccessor_Struct_NoSE) {
    auto* s = Structure("S", utils::Vector{Member("m", ty.i32())});
    auto* var = Decl(Var("a", ty.Of(s)));
    auto* expr = MemberAccessor("a", "m");
    WrapInFunction(var, expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    EXPECT_FALSE(sem->HasSideEffects());
}

TEST_F(SideEffectsTest, MemberAccessor_Struct_SE) {
    auto* s = Structure("S", utils::Vector{Member("m", ty.i32())});
    MakeSideEffectFunc("se", [&] { return ty.Of(s); });
    auto* expr = MemberAccessor(Call("se"), "m");
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    EXPECT_TRUE(sem->HasSideEffects());
}

TEST_F(SideEffectsTest, MemberAccessor_Vector) {
    auto* var = Decl(Var("a", ty.vec4<f32>()));
    auto* expr = MemberAccessor("a", "x");
    WrapInFunction(var, expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* sem = Sem().Get(expr);
    EXPECT_TRUE(sem->Is<sem::MemberAccessorExpression>());
    ASSERT_NE(sem, nullptr);
    EXPECT_FALSE(sem->HasSideEffects());
}

TEST_F(SideEffectsTest, MemberAccessor_VectorSwizzleNoSE) {
    auto* var = Decl(Var("a", ty.vec4<f32>()));
    auto* expr = MemberAccessor("a", "xzyw");
    WrapInFunction(var, expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* sem = Sem().Get(expr);
    EXPECT_TRUE(sem->Is<sem::Swizzle>());
    ASSERT_NE(sem, nullptr);
    EXPECT_FALSE(sem->HasSideEffects());
}

TEST_F(SideEffectsTest, MemberAccessor_VectorSwizzleSE) {
    MakeSideEffectFunc("se", [&] { return ty.vec4<f32>(); });
    auto* expr = MemberAccessor(Call("se"), "xzyw");
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* sem = Sem().Get(expr);
    EXPECT_TRUE(sem->Is<sem::Swizzle>());
    ASSERT_NE(sem, nullptr);
    EXPECT_TRUE(sem->HasSideEffects());
}

TEST_F(SideEffectsTest, Binary_NoSE) {
    auto* a = Decl(Var("a", ty.i32()));
    auto* b = Decl(Var("b", ty.i32()));
    auto* expr = Add("a", "b");
    WrapInFunction(a, b, expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    EXPECT_FALSE(sem->HasSideEffects());
}

TEST_F(SideEffectsTest, Binary_LeftSE) {
    MakeSideEffectFunc<i32>("se");
    auto* b = Decl(Var("b", ty.i32()));
    auto* expr = Add(Call("se"), "b");
    WrapInFunction(b, expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    EXPECT_TRUE(sem->HasSideEffects());
}

TEST_F(SideEffectsTest, Binary_RightSE) {
    MakeSideEffectFunc<i32>("se");
    auto* a = Decl(Var("a", ty.i32()));
    auto* expr = Add("a", Call("se"));
    WrapInFunction(a, expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    EXPECT_TRUE(sem->HasSideEffects());
}

TEST_F(SideEffectsTest, Binary_BothSE) {
    MakeSideEffectFunc<i32>("se1");
    MakeSideEffectFunc<i32>("se2");
    auto* expr = Add(Call("se1"), Call("se2"));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    EXPECT_TRUE(sem->HasSideEffects());
}

TEST_F(SideEffectsTest, Unary_NoSE) {
    auto* var = Decl(Var("a", ty.bool_()));
    auto* expr = Not("a");
    WrapInFunction(var, expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    EXPECT_FALSE(sem->HasSideEffects());
}

TEST_F(SideEffectsTest, Unary_SE) {
    MakeSideEffectFunc<bool>("se");
    auto* expr = Not(Call("se"));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    EXPECT_TRUE(sem->HasSideEffects());
}

TEST_F(SideEffectsTest, IndexAccessor_NoSE) {
    auto* var = Decl(Var("a", ty.array<i32, 10>()));
    auto* expr = IndexAccessor("a", 0_i);
    WrapInFunction(var, expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    EXPECT_FALSE(sem->HasSideEffects());
}

TEST_F(SideEffectsTest, IndexAccessor_ObjSE) {
    MakeSideEffectFunc("se", [&] { return ty.array<i32, 10>(); });
    auto* expr = IndexAccessor(Call("se"), 0_i);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    EXPECT_TRUE(sem->HasSideEffects());
}

TEST_F(SideEffectsTest, IndexAccessor_IndexSE) {
    MakeSideEffectFunc<i32>("se");
    auto* var = Decl(Var("a", ty.array<i32, 10>()));
    auto* expr = IndexAccessor("a", Call("se"));
    WrapInFunction(var, expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    EXPECT_TRUE(sem->HasSideEffects());
}

TEST_F(SideEffectsTest, IndexAccessor_BothSE) {
    MakeSideEffectFunc("se1", [&] { return ty.array<i32, 10>(); });
    MakeSideEffectFunc<i32>("se2");
    auto* expr = IndexAccessor(Call("se1"), Call("se2"));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    EXPECT_TRUE(sem->HasSideEffects());
}

TEST_F(SideEffectsTest, Bitcast_NoSE) {
    auto* var = Decl(Var("a", ty.i32()));
    auto* expr = Bitcast<f32>("a");
    WrapInFunction(var, expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    EXPECT_FALSE(sem->HasSideEffects());
}

TEST_F(SideEffectsTest, Bitcast_SE) {
    MakeSideEffectFunc<i32>("se");
    auto* expr = Bitcast<f32>(Call("se"));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    EXPECT_TRUE(sem->HasSideEffects());
}

}  // namespace
}  // namespace tint::resolver
