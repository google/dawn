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

#include "src/ast/intrinsic_texture_helper_test.h"
#include "src/resolver/resolver_test_helper.h"

namespace tint {
namespace resolver {
namespace {

using ResolverIntrinsicValidationTest = ResolverTest;

TEST_F(ResolverIntrinsicValidationTest,
       FunctionTypeMustMatchReturnStatementType_void_fail) {
  // fn func { return workgroupBarrier(); }
  Func("func", {}, ty.void_(),
       {
           Return(Call(Source{Source::Location{12, 34}}, "workgroupBarrier")),
       });

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      "12:34 error: intrinsic 'workgroupBarrier' does not return a value");
}

TEST_F(ResolverIntrinsicValidationTest, InvalidPipelineStageDirect) {
  // [[stage(compute), workgroup_size(1)]] fn func { return dpdx(1.0); }

  auto* dpdx = create<ast::CallExpression>(Source{{3, 4}}, Expr("dpdx"),
                                           ast::ExpressionList{Expr(1.0f)});
  Func(Source{{1, 2}}, "func", ast::VariableList{}, ty.void_(), {Ignore(dpdx)},
       {Stage(ast::PipelineStage::kCompute), WorkgroupSize(1)});

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "3:4 error: built-in cannot be used by compute pipeline stage");
}

TEST_F(ResolverIntrinsicValidationTest, InvalidPipelineStageIndirect) {
  // fn f0 { return dpdx(1.0); }
  // fn f1 { f0(); }
  // fn f2 { f1(); }
  // [[stage(compute), workgroup_size(1)]] fn main { return f2(); }

  auto* dpdx = create<ast::CallExpression>(Source{{3, 4}}, Expr("dpdx"),
                                           ast::ExpressionList{Expr(1.0f)});
  Func(Source{{1, 2}}, "f0", {}, ty.void_(), {Ignore(dpdx)});

  Func(Source{{3, 4}}, "f1", {}, ty.void_(),
       {create<ast::CallStatement>(Call("f0"))});

  Func(Source{{5, 6}}, "f2", {}, ty.void_(),
       {create<ast::CallStatement>(Call("f1"))});

  Func(Source{{7, 8}}, "main", {}, ty.void_(),
       {create<ast::CallStatement>(Call("f2"))},
       {Stage(ast::PipelineStage::kCompute), WorkgroupSize(1)});

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            R"(3:4 error: built-in cannot be used by compute pipeline stage
1:2 note: called by function 'f0'
3:4 note: called by function 'f1'
5:6 note: called by function 'f2'
7:8 note: called by entry point 'main')");
}

namespace TextureSamplerOffset {

using TextureOverloadCase = ast::intrinsic::test::TextureOverloadCase;
using ValidTextureOverload = ast::intrinsic::test::ValidTextureOverload;
using TextureKind = ast::intrinsic::test::TextureKind;
using TextureDataType = ast::intrinsic::test::TextureDataType;
using u32 = ProgramBuilder::u32;
using i32 = ProgramBuilder::i32;
using f32 = ProgramBuilder::f32;

static std::vector<TextureOverloadCase> ValidCases() {
  std::vector<TextureOverloadCase> cases;
  for (auto c : TextureOverloadCase::ValidCases()) {
    if (std::string(c.function).find("textureSample") == 0) {
      if (std::string(c.description).find("offset ") != std::string::npos) {
        cases.push_back(c);
      }
    }
  }
  return cases;
}

struct OffsetCase {
  bool is_valid;
  int32_t x;
  int32_t y;
  int32_t z;
  int32_t illegal_value = 0;
};

static std::vector<OffsetCase> OffsetCases() {
  return {
      {true, 0, 1, 2},          //
      {true, 7, -8, 7},         //
      {false, 10, 10, 20, 10},  //
      {false, -9, 0, 0, -9},    //
      {false, 0, 8, 0, 8},      //
  };
}

using IntrinsicTextureSamplerValidationTest =
    ResolverTestWithParam<std::tuple<TextureOverloadCase,  // texture info
                                     OffsetCase            // offset info
                                     >>;
TEST_P(IntrinsicTextureSamplerValidationTest, ConstExpr) {
  auto& p = GetParam();
  auto param = std::get<0>(p);
  auto offset = std::get<1>(p);
  param.buildTextureVariable(this);
  param.buildSamplerVariable(this);

  auto args = param.args(this);
  // Make Resolver visit the Node about to be removed
  WrapInFunction(args.back());
  args.pop_back();
  if (NumCoordinateAxes(param.texture_dimension) == 2) {
    args.push_back(
        Construct(Source{{12, 34}}, ty.vec2<i32>(), offset.x, offset.y));
  } else if (NumCoordinateAxes(param.texture_dimension) == 3) {
    args.push_back(Construct(Source{{12, 34}}, ty.vec3<i32>(), offset.x,
                             offset.y, offset.z));
  }

  auto* call = Call(param.function, args);
  Func("func", {}, ty.void_(), {Ignore(call)},
       {create<ast::StageDecoration>(ast::PipelineStage::kFragment)});

  if (offset.is_valid) {
    EXPECT_TRUE(r()->Resolve()) << r()->error();
  } else {
    EXPECT_FALSE(r()->Resolve());
    std::stringstream err;
    err << "12:34 error: each offset component of '" << param.function
        << "' must be at least -8 and at most 7. found: '"
        << std::to_string(offset.illegal_value) << "'";
    EXPECT_EQ(r()->error(), err.str());
  }
}

TEST_P(IntrinsicTextureSamplerValidationTest, ConstExprOfConstExpr) {
  auto& p = GetParam();
  auto param = std::get<0>(p);
  auto offset = std::get<1>(p);
  param.buildTextureVariable(this);
  param.buildSamplerVariable(this);

  auto args = param.args(this);
  // Make Resolver visit the Node about to be removed
  WrapInFunction(args.back());
  args.pop_back();
  if (NumCoordinateAxes(param.texture_dimension) == 2) {
    args.push_back(Construct(Source{{12, 34}}, ty.vec2<i32>(),
                             Construct(ty.i32(), offset.x), offset.y));
  } else if (NumCoordinateAxes(param.texture_dimension) == 3) {
    args.push_back(Construct(Source{{12, 34}}, ty.vec3<i32>(), offset.x,
                             Construct(ty.vec2<i32>(), offset.y, offset.z)));
  }
  auto* call = Call(param.function, args);
  Func("func", {}, ty.void_(), {Ignore(call)},
       {create<ast::StageDecoration>(ast::PipelineStage::kFragment)});
  if (offset.is_valid) {
    EXPECT_TRUE(r()->Resolve()) << r()->error();
  } else {
    EXPECT_FALSE(r()->Resolve());
    std::stringstream err;
    err << "12:34 error: each offset component of '" << param.function
        << "' must be at least -8 and at most 7. found: '"
        << std::to_string(offset.illegal_value) << "'";
    EXPECT_EQ(r()->error(), err.str());
  }
}

TEST_P(IntrinsicTextureSamplerValidationTest, EmptyVectorConstructor) {
  auto& p = GetParam();
  auto param = std::get<0>(p);
  param.buildTextureVariable(this);
  param.buildSamplerVariable(this);

  auto args = param.args(this);
  // Make Resolver visit the Node about to be removed
  WrapInFunction(args.back());
  args.pop_back();
  if (NumCoordinateAxes(param.texture_dimension) == 2) {
    args.push_back(Construct(Source{{12, 34}}, ty.vec2<i32>()));
  } else if (NumCoordinateAxes(param.texture_dimension) == 3) {
    args.push_back(Construct(Source{{12, 34}}, ty.vec3<i32>()));
  }

  auto* call = Call(param.function, args);
  Func("func", {}, ty.void_(), {Ignore(call)},
       {create<ast::StageDecoration>(ast::PipelineStage::kFragment)});
  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_P(IntrinsicTextureSamplerValidationTest, GlobalConst) {
  auto& p = GetParam();
  auto param = std::get<0>(p);
  auto offset = std::get<1>(p);
  param.buildTextureVariable(this);
  param.buildSamplerVariable(this);

  auto args = param.args(this);
  // Make Resolver visit the Node about to be removed
  WrapInFunction(args.back());
  args.pop_back();
  GlobalConst("offset_2d", ty.vec2<i32>(), vec2<i32>(offset.x, offset.y));
  GlobalConst("offset_3d", ty.vec3<i32>(),
              vec3<i32>(offset.x, offset.y, offset.z));
  if (NumCoordinateAxes(param.texture_dimension) == 2) {
    args.push_back(Expr(Source{{12, 34}}, "offset_2d"));
  } else if (NumCoordinateAxes(param.texture_dimension) == 3) {
    args.push_back(Expr(Source{{12, 34}}, "offset_3d"));
  }

  auto* call = Call(param.function, args);
  Func("func", {}, ty.void_(), {Ignore(call)},
       {create<ast::StageDecoration>(ast::PipelineStage::kFragment)});
  EXPECT_FALSE(r()->Resolve());
  std::stringstream err;
  err << "12:34 error: '" << param.function
      << "' offset parameter must be a const_expression";
  EXPECT_EQ(r()->error(), err.str());
}

TEST_P(IntrinsicTextureSamplerValidationTest, ScalarConst) {
  auto& p = GetParam();
  auto param = std::get<0>(p);
  auto offset = std::get<1>(p);
  param.buildTextureVariable(this);
  param.buildSamplerVariable(this);
  auto* x = Const("x", ty.i32(), Construct(ty.i32(), offset.x));

  auto args = param.args(this);
  // Make Resolver visit the Node about to be removed
  WrapInFunction(args.back());
  args.pop_back();
  if (NumCoordinateAxes(param.texture_dimension) == 2) {
    args.push_back(Construct(Source{{12, 34}}, ty.vec2<i32>(), x, offset.y));
  } else if (NumCoordinateAxes(param.texture_dimension) == 3) {
    args.push_back(
        Construct(Source{{12, 34}}, ty.vec3<i32>(), x, offset.y, offset.z));
  }

  auto* call = Call(param.function, args);
  Func("func", {}, ty.void_(), {Decl(x), Ignore(call)},
       {create<ast::StageDecoration>(ast::PipelineStage::kFragment)});
  EXPECT_FALSE(r()->Resolve());
  std::stringstream err;
  err << "12:34 error: '" << param.function
      << "' offset parameter must be a const_expression";
  EXPECT_EQ(r()->error(), err.str());
}

INSTANTIATE_TEST_SUITE_P(IntrinsicTextureSamplerValidationTest,
                         IntrinsicTextureSamplerValidationTest,
                         testing::Combine(testing::ValuesIn(ValidCases()),
                                          testing::ValuesIn(OffsetCases())));
}  // namespace TextureSamplerOffset

}  // namespace
}  // namespace resolver
}  // namespace tint
