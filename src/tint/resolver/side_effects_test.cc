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
#include "src/tint/builtin/address_space.h"
#include "src/tint/builtin/extension.h"
#include "src/tint/builtin/texel_format.h"
#include "src/tint/resolver/resolver_test_helper.h"
#include "src/tint/sem/index_accessor_expression.h"
#include "src/tint/sem/member_accessor_expression.h"
#include "src/tint/sem/value_expression.h"
#include "src/tint/type/texture_dimension.h"
#include "src/tint/utils/vector.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::resolver {
namespace {

struct SideEffectsTest : ResolverTest {
    template <typename T>
    void MakeSideEffectFunc(const char* name) {
        auto global = Sym();
        GlobalVar(global, ty.Of<T>(), builtin::AddressSpace::kPrivate);
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
        GlobalVar(global, make_type(), builtin::AddressSpace::kPrivate);
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
    auto* sem = Sem().GetVal(expr);
    ASSERT_NE(sem, nullptr);
    EXPECT_TRUE(sem->UnwrapLoad()->Is<sem::VariableUser>());
    EXPECT_FALSE(sem->HasSideEffects());
}

TEST_F(SideEffectsTest, Call_Builtin_NoSE) {
    GlobalVar("a", ty.f32(), builtin::AddressSpace::kPrivate);
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
    GlobalVar("a", ty.atomic(ty.i32()), builtin::AddressSpace::kWorkgroup);
    auto* expr = Call("atomicAdd", AddressOf("a"), 1_i);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    EXPECT_TRUE(sem->Is<sem::Call>());
    EXPECT_TRUE(sem->HasSideEffects());
}

namespace builtin_tests {
struct Case {
    const char* name;
    utils::Vector<const char*, 3> args;
    bool has_side_effects;
    bool returns_value;
    ast::PipelineStage pipeline_stage;
};
static Case C(const char* name,
              utils::VectorRef<const char*> args,
              bool has_side_effects,
              bool returns_value,
              ast::PipelineStage stage = ast::PipelineStage::kFragment) {
    Case c;
    c.name = name;
    c.args = std::move(args);
    c.has_side_effects = has_side_effects;
    c.returns_value = returns_value;
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
    o << ")";
    return o;
}

using SideEffectsBuiltinTest = resolver::ResolverTestWithParam<Case>;

TEST_P(SideEffectsBuiltinTest, Test) {
    Enable(tint::builtin::Extension::kChromiumExperimentalDp4A);
    auto& c = GetParam();

    uint32_t next_binding = 0;
    GlobalVar("f", ty.f32(), tint::builtin::AddressSpace::kPrivate);
    GlobalVar("i", ty.i32(), tint::builtin::AddressSpace::kPrivate);
    GlobalVar("u", ty.u32(), tint::builtin::AddressSpace::kPrivate);
    GlobalVar("b", ty.bool_(), tint::builtin::AddressSpace::kPrivate);
    GlobalVar("vf", ty.vec3<f32>(), tint::builtin::AddressSpace::kPrivate);
    GlobalVar("vf2", ty.vec2<f32>(), tint::builtin::AddressSpace::kPrivate);
    GlobalVar("vi2", ty.vec2<i32>(), tint::builtin::AddressSpace::kPrivate);
    GlobalVar("vf4", ty.vec4<f32>(), tint::builtin::AddressSpace::kPrivate);
    GlobalVar("vb", ty.vec3<bool>(), tint::builtin::AddressSpace::kPrivate);
    GlobalVar("m", ty.mat3x3<f32>(), tint::builtin::AddressSpace::kPrivate);
    GlobalVar("arr", ty.array<f32, 10>(), tint::builtin::AddressSpace::kPrivate);
    GlobalVar("storage_arr", ty.array<f32>(), tint::builtin::AddressSpace::kStorage, Group(0_a),
              Binding(AInt(next_binding++)));
    GlobalVar("workgroup_arr", ty.array<f32, 4>(), tint::builtin::AddressSpace::kWorkgroup);
    GlobalVar("a", ty.atomic(ty.i32()), tint::builtin::AddressSpace::kStorage,
              tint::builtin::Access::kReadWrite, Group(0_a), Binding(AInt(next_binding++)));
    if (c.pipeline_stage != ast::PipelineStage::kCompute) {
        GlobalVar("t2d", ty.sampled_texture(type::TextureDimension::k2d, ty.f32()), Group(0_a),
                  Binding(AInt(next_binding++)));
        GlobalVar("tdepth2d", ty.depth_texture(type::TextureDimension::k2d), Group(0_a),
                  Binding(AInt(next_binding++)));
        GlobalVar("t2d_arr", ty.sampled_texture(type::TextureDimension::k2dArray, ty.f32()),
                  Group(0_a), Binding(AInt(next_binding++)));
        GlobalVar("t2d_multi", ty.multisampled_texture(type::TextureDimension::k2d, ty.f32()),
                  Group(0_a), Binding(AInt(next_binding++)));
        GlobalVar(
            "tstorage2d",
            ty.storage_texture(type::TextureDimension::k2d, tint::builtin::TexelFormat::kR32Float,
                               tint::builtin::Access::kWrite),
            Group(0_a), Binding(AInt(next_binding++)));
        GlobalVar("s2d", ty.sampler(type::SamplerKind::kSampler), Group(0_a),
                  Binding(AInt(next_binding++)));
        GlobalVar("scomp", ty.sampler(type::SamplerKind::kComparisonSampler), Group(0_a),
                  Binding(AInt(next_binding++)));
    }

    utils::Vector<const ast::Statement*, 4> stmts;
    stmts.Push(Decl(Let("pstorage_arr", AddressOf("storage_arr"))));
    if (c.pipeline_stage == ast::PipelineStage::kCompute) {
        stmts.Push(Decl(Let("pworkgroup_arr", AddressOf("workgroup_arr"))));
    }
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

    if (c.returns_value) {
        stmts.Push(Assign(Phony(), expr));
    } else {
        stmts.Push(CallStmt(expr));
    }

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
        C("abs", utils::Vector{"f"}, false, true),                                               //
        C("acos", utils::Vector{"f"}, false, true),                                              //
        C("acosh", utils::Vector{"f"}, false, true),                                             //
        C("all", utils::Vector{"vb"}, false, true),                                              //
        C("any", utils::Vector{"vb"}, false, true),                                              //
        C("arrayLength", utils::Vector{"pstorage_arr"}, false, true),                            //
        C("asin", utils::Vector{"f"}, false, true),                                              //
        C("asinh", utils::Vector{"f"}, false, true),                                             //
        C("atan", utils::Vector{"f"}, false, true),                                              //
        C("atan2", utils::Vector{"f", "f"}, false, true),                                        //
        C("atanh", utils::Vector{"f"}, false, true),                                             //
        C("atomicLoad", utils::Vector{"pa"}, false, true),                                       //
        C("ceil", utils::Vector{"f"}, false, true),                                              //
        C("clamp", utils::Vector{"f", "f", "f"}, false, true),                                   //
        C("cos", utils::Vector{"f"}, false, true),                                               //
        C("cosh", utils::Vector{"f"}, false, true),                                              //
        C("countLeadingZeros", utils::Vector{"i"}, false, true),                                 //
        C("countOneBits", utils::Vector{"i"}, false, true),                                      //
        C("countTrailingZeros", utils::Vector{"i"}, false, true),                                //
        C("cross", utils::Vector{"vf", "vf"}, false, true),                                      //
        C("degrees", utils::Vector{"f"}, false, true),                                           //
        C("determinant", utils::Vector{"m"}, false, true),                                       //
        C("distance", utils::Vector{"f", "f"}, false, true),                                     //
        C("dot", utils::Vector{"vf", "vf"}, false, true),                                        //
        C("dot4I8Packed", utils::Vector{"u", "u"}, false, true),                                 //
        C("dot4U8Packed", utils::Vector{"u", "u"}, false, true),                                 //
        C("exp", utils::Vector{"f"}, false, true),                                               //
        C("exp2", utils::Vector{"f"}, false, true),                                              //
        C("extractBits", utils::Vector{"i", "u", "u"}, false, true),                             //
        C("faceForward", utils::Vector{"vf", "vf", "vf"}, false, true),                          //
        C("firstLeadingBit", utils::Vector{"u"}, false, true),                                   //
        C("firstTrailingBit", utils::Vector{"u"}, false, true),                                  //
        C("floor", utils::Vector{"f"}, false, true),                                             //
        C("fma", utils::Vector{"f", "f", "f"}, false, true),                                     //
        C("fract", utils::Vector{"vf"}, false, true),                                            //
        C("frexp", utils::Vector{"f"}, false, true),                                             //
        C("insertBits", utils::Vector{"i", "i", "u", "u"}, false, true),                         //
        C("inverseSqrt", utils::Vector{"f"}, false, true),                                       //
        C("ldexp", utils::Vector{"f", "i"}, false, true),                                        //
        C("length", utils::Vector{"vf"}, false, true),                                           //
        C("log", utils::Vector{"f"}, false, true),                                               //
        C("log2", utils::Vector{"f"}, false, true),                                              //
        C("max", utils::Vector{"f", "f"}, false, true),                                          //
        C("min", utils::Vector{"f", "f"}, false, true),                                          //
        C("mix", utils::Vector{"f", "f", "f"}, false, true),                                     //
        C("modf", utils::Vector{"f"}, false, true),                                              //
        C("normalize", utils::Vector{"vf"}, false, true),                                        //
        C("pack2x16float", utils::Vector{"vf2"}, false, true),                                   //
        C("pack2x16snorm", utils::Vector{"vf2"}, false, true),                                   //
        C("pack2x16unorm", utils::Vector{"vf2"}, false, true),                                   //
        C("pack4x8snorm", utils::Vector{"vf4"}, false, true),                                    //
        C("pack4x8unorm", utils::Vector{"vf4"}, false, true),                                    //
        C("pow", utils::Vector{"f", "f"}, false, true),                                          //
        C("radians", utils::Vector{"f"}, false, true),                                           //
        C("reflect", utils::Vector{"vf", "vf"}, false, true),                                    //
        C("refract", utils::Vector{"vf", "vf", "f"}, false, true),                               //
        C("reverseBits", utils::Vector{"u"}, false, true),                                       //
        C("round", utils::Vector{"f"}, false, true),                                             //
        C("select", utils::Vector{"f", "f", "b"}, false, true),                                  //
        C("sign", utils::Vector{"f"}, false, true),                                              //
        C("sin", utils::Vector{"f"}, false, true),                                               //
        C("sinh", utils::Vector{"f"}, false, true),                                              //
        C("smoothstep", utils::Vector{"f", "f", "f"}, false, true),                              //
        C("sqrt", utils::Vector{"f"}, false, true),                                              //
        C("step", utils::Vector{"f", "f"}, false, true),                                         //
        C("tan", utils::Vector{"f"}, false, true),                                               //
        C("tanh", utils::Vector{"f"}, false, true),                                              //
        C("textureDimensions", utils::Vector{"t2d"}, false, true),                               //
        C("textureGather", utils::Vector{"tdepth2d", "s2d", "vf2"}, false, true),                //
        C("textureGatherCompare", utils::Vector{"tdepth2d", "scomp", "vf2", "f"}, false, true),  //
        C("textureLoad", utils::Vector{"t2d", "vi2", "i"}, false, true),                         //
        C("textureNumLayers", utils::Vector{"t2d_arr"}, false, true),                            //
        C("textureNumLevels", utils::Vector{"t2d"}, false, true),                                //
        C("textureNumSamples", utils::Vector{"t2d_multi"}, false, true),                         //
        C("textureSampleCompareLevel",
          utils::Vector{"tdepth2d", "scomp", "vf2", "f"},
          false,
          true),                                                                                 //
        C("textureSampleGrad", utils::Vector{"t2d", "s2d", "vf2", "vf2", "vf2"}, false, true),   //
        C("textureSampleLevel", utils::Vector{"t2d", "s2d", "vf2", "f"}, false, true),           //
        C("transpose", utils::Vector{"m"}, false, true),                                         //
        C("trunc", utils::Vector{"f"}, false, true),                                             //
        C("unpack2x16float", utils::Vector{"u"}, false, true),                                   //
        C("unpack2x16snorm", utils::Vector{"u"}, false, true),                                   //
        C("unpack2x16unorm", utils::Vector{"u"}, false, true),                                   //
        C("unpack4x8snorm", utils::Vector{"u"}, false, true),                                    //
        C("unpack4x8unorm", utils::Vector{"u"}, false, true),                                    //
        C("storageBarrier", utils::Empty, false, false, ast::PipelineStage::kCompute),           //
        C("workgroupBarrier", utils::Empty, false, false, ast::PipelineStage::kCompute),         //
        C("textureSample", utils::Vector{"t2d", "s2d", "vf2"}, false, true),                     //
        C("textureSampleBias", utils::Vector{"t2d", "s2d", "vf2", "f"}, false, true),            //
        C("textureSampleCompare", utils::Vector{"tdepth2d", "scomp", "vf2", "f"}, false, true),  //
        C("dpdx", utils::Vector{"f"}, false, true),                                              //
        C("dpdxCoarse", utils::Vector{"f"}, false, true),                                        //
        C("dpdxFine", utils::Vector{"f"}, false, true),                                          //
        C("dpdy", utils::Vector{"f"}, false, true),                                              //
        C("dpdyCoarse", utils::Vector{"f"}, false, true),                                        //
        C("dpdyFine", utils::Vector{"f"}, false, true),                                          //
        C("fwidth", utils::Vector{"f"}, false, true),                                            //
        C("fwidthCoarse", utils::Vector{"f"}, false, true),                                      //
        C("fwidthFine", utils::Vector{"f"}, false, true),                                        //

        // Side-effect builtins
        C("atomicAdd", utils::Vector{"pa", "i"}, true, true),                       //
        C("atomicAnd", utils::Vector{"pa", "i"}, true, true),                       //
        C("atomicCompareExchangeWeak", utils::Vector{"pa", "i", "i"}, true, true),  //
        C("atomicExchange", utils::Vector{"pa", "i"}, true, true),                  //
        C("atomicMax", utils::Vector{"pa", "i"}, true, true),                       //
        C("atomicMin", utils::Vector{"pa", "i"}, true, true),                       //
        C("atomicOr", utils::Vector{"pa", "i"}, true, true),                        //
        C("atomicStore", utils::Vector{"pa", "i"}, true, false),                    //
        C("atomicSub", utils::Vector{"pa", "i"}, true, true),                       //
        C("atomicXor", utils::Vector{"pa", "i"}, true, true),                       //
        C("textureStore", utils::Vector{"tstorage2d", "vi2", "vf4"}, true, false),  //
        C("workgroupUniformLoad",
          utils::Vector{"pworkgroup_arr"},
          true,
          true,
          ast::PipelineStage::kCompute),  //

        // Unimplemented builtins
        // C("quantizeToF16", utils::Vector{"f"}, false), //
        // C("saturate", utils::Vector{"f"}, false), //
    }));

}  // namespace builtin_tests

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
    auto* expr = Call<f32>("a");
    WrapInFunction(var, expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    EXPECT_TRUE(sem->Is<sem::Call>());
    EXPECT_FALSE(sem->HasSideEffects());
}

TEST_F(SideEffectsTest, Call_TypeConversion_SE) {
    MakeSideEffectFunc<i32>("se");
    auto* expr = Call<f32>(Call("se"));
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    EXPECT_TRUE(sem->Is<sem::Call>());
    EXPECT_TRUE(sem->HasSideEffects());
}

TEST_F(SideEffectsTest, Call_TypeInitializer_NoSE) {
    auto* var = Decl(Var("a", ty.f32()));
    auto* expr = Call<f32>("a");
    WrapInFunction(var, expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    EXPECT_TRUE(sem->Is<sem::Call>());
    EXPECT_FALSE(sem->HasSideEffects());
}

TEST_F(SideEffectsTest, Call_TypeInitializer_SE) {
    MakeSideEffectFunc<f32>("se");
    auto* expr = Call<f32>(Call("se"));
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
    ASSERT_NE(sem, nullptr);
    EXPECT_TRUE(sem->UnwrapLoad()->Is<sem::MemberAccessorExpression>());
    EXPECT_FALSE(sem->HasSideEffects());
}

TEST_F(SideEffectsTest, MemberAccessor_VectorSwizzleNoSE) {
    auto* var = Decl(Var("a", ty.vec4<f32>()));
    auto* expr = MemberAccessor("a", "xzyw");
    WrapInFunction(var, expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    EXPECT_TRUE(sem->Is<sem::Swizzle>());
    EXPECT_FALSE(sem->HasSideEffects());
}

TEST_F(SideEffectsTest, MemberAccessor_VectorSwizzleSE) {
    MakeSideEffectFunc("se", [&] { return ty.vec4<f32>(); });
    auto* expr = MemberAccessor(Call("se"), "xzyw");
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* sem = Sem().Get(expr);
    ASSERT_NE(sem, nullptr);
    EXPECT_TRUE(sem->Is<sem::Swizzle>());
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
