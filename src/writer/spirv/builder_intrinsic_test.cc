// Copyright 2020 The Tint Authors.
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

#include <memory>

#include "gtest/gtest.h"
#include "src/ast/call_expression.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/type/bool_type.h"
#include "src/ast/type/depth_texture_type.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/i32_type.h"
#include "src/ast/type/matrix_type.h"
#include "src/ast/type/sampled_texture_type.h"
#include "src/ast/type/sampler_type.h"
#include "src/ast/type/u32_type.h"
#include "src/ast/type/vector_type.h"
#include "src/ast/variable.h"
#include "src/context.h"
#include "src/type_determiner.h"
#include "src/writer/spirv/builder.h"
#include "src/writer/spirv/spv_dump.h"

namespace tint {
namespace writer {
namespace spirv {
namespace {

using BuilderTest = testing::Test;

struct IntrinsicData {
  std::string name;
  std::string op;
};
inline std::ostream& operator<<(std::ostream& out, IntrinsicData data) {
  out << data.name;
  return out;
}

using IntrinsicBoolTest = testing::TestWithParam<IntrinsicData>;
TEST_P(IntrinsicBoolTest, Call_Bool) {
  auto param = GetParam();

  ast::type::BoolType bool_type;
  ast::type::VectorType vec3(&bool_type, 3);

  auto var =
      std::make_unique<ast::Variable>("v", ast::StorageClass::kPrivate, &vec3);

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::IdentifierExpression>("v"));
  ast::CallExpression expr(
      std::make_unique<ast::IdentifierExpression>(param.name),
      std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  td.RegisterVariableForTesting(var.get());

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateGlobalVariable(var.get())) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(&expr), 6u) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeBool
%3 = OpTypeVector %4 3
%2 = OpTypePointer Private %3
%5 = OpConstantNull %3
%1 = OpVariable %2 Private %5
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%7 = OpLoad %3 %1
%6 = )" + param.op +
                " %4 %7\n");
}
INSTANTIATE_TEST_SUITE_P(BuilderTest,
                         IntrinsicBoolTest,
                         testing::Values(IntrinsicData{"any", "OpAny"},
                                         IntrinsicData{"all", "OpAll"}));

using IntrinsicFloatTest = testing::TestWithParam<IntrinsicData>;
TEST_P(IntrinsicFloatTest, Call_Float_Scalar) {
  auto param = GetParam();

  ast::type::F32Type f32;

  auto var =
      std::make_unique<ast::Variable>("v", ast::StorageClass::kPrivate, &f32);

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::IdentifierExpression>("v"));
  ast::CallExpression expr(
      std::make_unique<ast::IdentifierExpression>(param.name),
      std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  td.RegisterVariableForTesting(var.get());

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateGlobalVariable(var.get())) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(&expr), 5u) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeFloat 32
%2 = OpTypePointer Private %3
%4 = OpConstantNull %3
%1 = OpVariable %2 Private %4
%6 = OpTypeBool
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%7 = OpLoad %3 %1
%5 = )" + param.op +
                " %6 %7\n");
}

TEST_P(IntrinsicFloatTest, Call_Float_Vector) {
  auto param = GetParam();

  ast::type::F32Type f32;
  ast::type::VectorType vec3(&f32, 3);

  auto var =
      std::make_unique<ast::Variable>("v", ast::StorageClass::kPrivate, &vec3);

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::IdentifierExpression>("v"));
  ast::CallExpression expr(
      std::make_unique<ast::IdentifierExpression>(param.name),
      std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  td.RegisterVariableForTesting(var.get());

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateGlobalVariable(var.get())) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(&expr), 6u) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeFloat 32
%3 = OpTypeVector %4 3
%2 = OpTypePointer Private %3
%5 = OpConstantNull %3
%1 = OpVariable %2 Private %5
%8 = OpTypeBool
%7 = OpTypeVector %8 3
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%9 = OpLoad %3 %1
%6 = )" + param.op +
                " %7 %9\n");
}
INSTANTIATE_TEST_SUITE_P(BuilderTest,
                         IntrinsicFloatTest,
                         testing::Values(IntrinsicData{"isNan", "OpIsNan"},
                                         IntrinsicData{"isInf", "OpIsInf"}));

TEST_F(BuilderTest, Call_Dot) {
  ast::type::F32Type f32;
  ast::type::VectorType vec3(&f32, 3);

  auto var =
      std::make_unique<ast::Variable>("v", ast::StorageClass::kPrivate, &vec3);

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::IdentifierExpression>("v"));
  params.push_back(std::make_unique<ast::IdentifierExpression>("v"));
  ast::CallExpression expr(std::make_unique<ast::IdentifierExpression>("dot"),
                           std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  td.RegisterVariableForTesting(var.get());

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateGlobalVariable(var.get())) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(&expr), 6u) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeFloat 32
%3 = OpTypeVector %4 3
%2 = OpTypePointer Private %3
%5 = OpConstantNull %3
%1 = OpVariable %2 Private %5
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%7 = OpLoad %3 %1
%8 = OpLoad %3 %1
%6 = OpDot %4 %7 %8
)");
}

using IntrinsicDeriveTest = testing::TestWithParam<IntrinsicData>;
TEST_P(IntrinsicDeriveTest, Call_Derivative_Scalar) {
  auto param = GetParam();

  ast::type::F32Type f32;

  auto var =
      std::make_unique<ast::Variable>("v", ast::StorageClass::kPrivate, &f32);

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::IdentifierExpression>("v"));
  ast::CallExpression expr(
      std::make_unique<ast::IdentifierExpression>(param.name),
      std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  td.RegisterVariableForTesting(var.get());

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateGlobalVariable(var.get())) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(&expr), 5u) << b.error();
  EXPECT_EQ(DumpInstructions(b.types()), R"(%3 = OpTypeFloat 32
%2 = OpTypePointer Private %3
%4 = OpConstantNull %3
%1 = OpVariable %2 Private %4
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%6 = OpLoad %3 %1
%5 = )" + param.op +
                " %3 %6\n");
}

TEST_P(IntrinsicDeriveTest, Call_Derivative_Vector) {
  auto param = GetParam();

  ast::type::F32Type f32;
  ast::type::VectorType vec3(&f32, 3);

  auto var =
      std::make_unique<ast::Variable>("v", ast::StorageClass::kPrivate, &vec3);

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::IdentifierExpression>("v"));
  ast::CallExpression expr(
      std::make_unique<ast::IdentifierExpression>(param.name),
      std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  td.RegisterVariableForTesting(var.get());

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateGlobalVariable(var.get())) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(&expr), 6u) << b.error();

  if (param.name != "dpdx" && param.name != "dpdy" && param.name != "fwidth") {
    EXPECT_EQ(DumpInstructions(b.capabilities()),
              R"(OpCapability DerivativeControl
)");
  }

  EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeFloat 32
%3 = OpTypeVector %4 3
%2 = OpTypePointer Private %3
%5 = OpConstantNull %3
%1 = OpVariable %2 Private %5
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%7 = OpLoad %3 %1
%6 = )" + param.op +
                " %3 %7\n");
}
INSTANTIATE_TEST_SUITE_P(
    BuilderTest,
    IntrinsicDeriveTest,
    testing::Values(IntrinsicData{"dpdx", "OpDPdx"},
                    IntrinsicData{"dpdxFine", "OpDPdxFine"},
                    IntrinsicData{"dpdxCoarse", "OpDPdxCoarse"},
                    IntrinsicData{"dpdy", "OpDPdy"},
                    IntrinsicData{"dpdyFine", "OpDPdyFine"},
                    IntrinsicData{"dpdyCoarse", "OpDPdyCoarse"},
                    IntrinsicData{"fwidth", "OpFwidth"},
                    IntrinsicData{"fwidthFine", "OpFwidthFine"},
                    IntrinsicData{"fwidthCoarse", "OpFwidthCoarse"}));

TEST_F(BuilderTest, Call_OuterProduct) {
  ast::type::F32Type f32;
  ast::type::VectorType vec2(&f32, 2);
  ast::type::VectorType vec3(&f32, 3);
  ast::type::MatrixType mat(&f32, 2, 3);

  auto v2 =
      std::make_unique<ast::Variable>("v2", ast::StorageClass::kPrivate, &vec2);
  auto v3 =
      std::make_unique<ast::Variable>("v3", ast::StorageClass::kPrivate, &vec3);

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::IdentifierExpression>("v2"));
  params.push_back(std::make_unique<ast::IdentifierExpression>("v3"));
  ast::CallExpression expr(
      std::make_unique<ast::IdentifierExpression>("outerProduct"),
      std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  td.RegisterVariableForTesting(v2.get());
  td.RegisterVariableForTesting(v3.get());

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateGlobalVariable(v2.get())) << b.error();
  ASSERT_TRUE(b.GenerateGlobalVariable(v3.get())) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(&expr), 10u) << b.error();

  EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeFloat 32
%3 = OpTypeVector %4 2
%2 = OpTypePointer Private %3
%5 = OpConstantNull %3
%1 = OpVariable %2 Private %5
%8 = OpTypeVector %4 3
%7 = OpTypePointer Private %8
%9 = OpConstantNull %8
%6 = OpVariable %7 Private %9
%11 = OpTypeMatrix %3 3
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%12 = OpLoad %3 %1
%13 = OpLoad %8 %6
%10 = OpOuterProduct %11 %12 %13
)");
}

TEST_F(BuilderTest, Call_Select) {
  ast::type::F32Type f32;
  ast::type::BoolType bool_type;
  ast::type::VectorType bool_vec3(&bool_type, 3);
  ast::type::VectorType vec3(&f32, 3);

  auto v3 =
      std::make_unique<ast::Variable>("v3", ast::StorageClass::kPrivate, &vec3);
  auto bool_v3 = std::make_unique<ast::Variable>(
      "bool_v3", ast::StorageClass::kPrivate, &bool_vec3);

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::IdentifierExpression>("v3"));
  params.push_back(std::make_unique<ast::IdentifierExpression>("v3"));
  params.push_back(std::make_unique<ast::IdentifierExpression>("bool_v3"));
  ast::CallExpression expr(
      std::make_unique<ast::IdentifierExpression>("select"), std::move(params));

  Context ctx;
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);
  td.RegisterVariableForTesting(v3.get());
  td.RegisterVariableForTesting(bool_v3.get());

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();

  Builder b(&mod);
  b.push_function(Function{});
  ASSERT_TRUE(b.GenerateGlobalVariable(v3.get())) << b.error();
  ASSERT_TRUE(b.GenerateGlobalVariable(bool_v3.get())) << b.error();

  EXPECT_EQ(b.GenerateCallExpression(&expr), 11u) << b.error();

  EXPECT_EQ(DumpInstructions(b.types()), R"(%4 = OpTypeFloat 32
%3 = OpTypeVector %4 3
%2 = OpTypePointer Private %3
%5 = OpConstantNull %3
%1 = OpVariable %2 Private %5
%9 = OpTypeBool
%8 = OpTypeVector %9 3
%7 = OpTypePointer Private %8
%10 = OpConstantNull %8
%6 = OpVariable %7 Private %10
)");
  EXPECT_EQ(DumpInstructions(b.functions()[0].instructions()),
            R"(%12 = OpLoad %3 %1
%13 = OpLoad %3 %1
%14 = OpLoad %8 %6
%11 = OpSelect %3 %12 %13 %14
)");
}

enum class TextureType { kF32, kI32, kU32 };
inline std::ostream& operator<<(std::ostream& out, TextureType data) {
  if (data == TextureType::kF32) {
    out << "f32";
  } else if (data == TextureType::kI32) {
    out << "i32";
  } else {
    out << "u32";
  }
  return out;
}

struct TextureTestParams {
  ast::type::TextureDimension dim;
  TextureType type = TextureType::kF32;
  ast::type::ImageFormat format = ast::type::ImageFormat::kR16Float;
};
inline std::ostream& operator<<(std::ostream& out, TextureTestParams data) {
  out << data.dim << "_" << data.type;
  return out;
}

class Builder_TextureOperation
    : public testing::TestWithParam<TextureTestParams> {
 public:
  Builder_TextureOperation()
      : td_(std::make_unique<TypeDeterminer>(&ctx_, &mod_)),
        b_(std::make_unique<Builder>(&mod_)) {}

  TypeDeterminer* td() const { return td_.get(); }
  Context* ctx() { return &ctx_; }
  Builder* b() const { return b_.get(); }

  std::unique_ptr<ast::type::Type> get_coords_type(
      ast::type::TextureDimension dim,
      ast::type::Type* type) {
    if (dim == ast::type::TextureDimension::k1d) {
      if (type->IsI32()) {
        return std::make_unique<ast::type::I32Type>();
      } else if (type->IsU32()) {
        return std::make_unique<ast::type::U32Type>();
      } else {
        return std::make_unique<ast::type::F32Type>();
      }
    } else if (dim == ast::type::TextureDimension::k1dArray ||
               dim == ast::type::TextureDimension::k2d ||
               dim == ast::type::TextureDimension::k2dMs) {
      return std::make_unique<ast::type::VectorType>(type, 2);
    } else if (dim == ast::type::TextureDimension::kCubeArray) {
      return std::make_unique<ast::type::VectorType>(type, 4);
    } else {
      return std::make_unique<ast::type::VectorType>(type, 3);
    }
  }

  void add_call_param(std::string name,
                      ast::type::Type* type,
                      ast::ExpressionList* call_params) {
    variables_.push_back(
        std::make_unique<ast::Variable>(name, ast::StorageClass::kNone, type));
    td()->RegisterVariableForTesting(variables_.back().get());

    call_params->push_back(std::make_unique<ast::IdentifierExpression>(name));
    ASSERT_TRUE(b()->GenerateGlobalVariable(variables_.back().get()))
        << b()->error();
  }

  std::unique_ptr<ast::type::Type> subtype(TextureType type) {
    if (type == TextureType::kF32) {
      return std::make_unique<ast::type::F32Type>();
    }
    if (type == TextureType::kI32) {
      return std::make_unique<ast::type::I32Type>();
    }
    return std::make_unique<ast::type::U32Type>();
  }

  std::string texture_line(
      ast::type::TextureDimension dim,
      bool unknown_format,
      TextureType type,
      uint32_t depth_literal,
      uint32_t sampled_literal,
      ast::type::ImageFormat format = ast::type::ImageFormat::kR8Unorm) {
    std::string res = "%6 = OpTypeImage ";

    if (type == TextureType::kF32) {
      res += "%1 ";
    } else if (type == TextureType::kU32) {
      res += "%2 ";
    } else {
      res += "%3 ";
    }

    if (dim == ast::type::TextureDimension::k1d ||
        dim == ast::type::TextureDimension::k1dArray) {
      res += "1D ";
    } else if (dim == ast::type::TextureDimension::k3d) {
      res += "3D ";
    } else if (dim == ast::type::TextureDimension::kCube ||
               dim == ast::type::TextureDimension::kCubeArray) {
      res += "Cube ";
    } else {
      res += "2D ";
    }

    res += std::to_string(depth_literal) + " ";

    if (dim == ast::type::TextureDimension::k1dArray ||
        dim == ast::type::TextureDimension::k2dArray ||
        dim == ast::type::TextureDimension::k2dMsArray ||
        dim == ast::type::TextureDimension::kCubeArray) {
      res += "1 ";
    } else {
      res += "0 ";
    }

    if (dim == ast::type::TextureDimension::k2dMs ||
        dim == ast::type::TextureDimension::k2dMsArray) {
      res += "1 ";
    } else {
      res += "0 ";
    }

    res += std::to_string(sampled_literal) + " ";

    if (unknown_format) {
      res += "Unknown\n";
    } else if (format == ast::type::ImageFormat::kR16Float) {
      res += "R16f\n";
    } else if (format == ast::type::ImageFormat::kR16Sint) {
      res += "R16i\n";
    } else {
      res += "R8\n";
    }

    return res;
  }

 private:
  Context ctx_;
  ast::Module mod_;
  std::unique_ptr<TypeDeterminer> td_;
  std::unique_ptr<Builder> b_;
  std::vector<std::unique_ptr<ast::Variable>> variables_;
};

class Builder_TextureLoad : public Builder_TextureOperation {
 public:
  std::string generate_type_str(ast::type::TextureDimension dim,
                                ast::type::ImageFormat format,
                                bool unknown_format,
                                TextureType type,
                                uint32_t depth_literal,
                                uint32_t sampled_literal,
                                uint32_t type_id,
                                uint32_t coords_length) {
    std::string type_str = R"(%1 = OpTypeFloat 32
%2 = OpTypeInt 32 0
%3 = OpTypeInt 32 1
)";

    type_str += texture_line(dim, unknown_format, type, depth_literal,
                             sampled_literal, format);

    type_str += R"(%5 = OpTypePointer Private %6
%7 = OpConstantNull %6
%4 = OpVariable %5 Private %7
)";

    if (coords_length > 1) {
      type_str += "%10 = OpTypeVector %3 " + std::to_string(coords_length) +
                  "\n" +
                  R"(%9 = OpTypePointer Private %10
%11 = OpConstantNull %10
%8 = OpVariable %9 Private %11
%13 = OpTypePointer Private %3
%14 = OpConstantNull %3
%12 = OpVariable %13 Private %14
%16 = OpTypeVector %)" +
                  std::to_string(type_id) + " 4\n";
    } else {
      type_str += R"(%9 = OpTypePointer Private %3
%10 = OpConstantNull %3
%8 = OpVariable %9 Private %10
%11 = OpVariable %9 Private %10
%13 = OpTypeVector %)" +
                  std::to_string(type_id) + " 4\n";
    }

    return type_str;
  }

  std::string generate_ops_str(uint32_t coords_length, std::string op_name) {
    if (coords_length == 1) {
      return R"(%14 = OpLoad %6 %4
%15 = OpLoad %3 %8
%16 = OpLoad %3 %11
%12 = )" + op_name +
             R"( %13 %14 %15 Lod %16
)";
    }

    return R"(%17 = OpLoad %6 %4
%18 = OpLoad %10 %8
%19 = OpLoad %3 %12
%15 = )" + op_name +
           R"( %16 %17 %18 Lod %19
)";
  }
};

TEST_P(Builder_TextureLoad, StorageReadonly) {
  auto dim = GetParam().dim;
  auto type = GetParam().type;
  auto format = GetParam().format;

  ast::type::F32Type f32;
  b()->GenerateTypeIfNeeded(&f32);
  ast::type::U32Type u32;
  b()->GenerateTypeIfNeeded(&u32);
  ast::type::I32Type i32;
  b()->GenerateTypeIfNeeded(&i32);

  uint32_t type_id = 1;
  if (type == TextureType::kU32) {
    type_id = 2;
  } else if (type == TextureType::kI32) {
    type_id = 3;
  }

  auto coords_type = get_coords_type(dim, &i32);

  uint32_t coords_length = 1;
  if (coords_type->IsVector()) {
    coords_length = coords_type->AsVector()->size();
  }

  auto* texture_type =
      ctx()->type_mgr().Get(std::make_unique<ast::type::StorageTextureType>(
          dim, ast::type::StorageAccess::kRead, format));

  EXPECT_TRUE(td()->Determine());

  ast::ExpressionList call_params;
  b()->push_function(Function{});

  add_call_param("texture", texture_type, &call_params);
  add_call_param("coords", coords_type.get(), &call_params);
  add_call_param("lod", &i32, &call_params);

  ast::CallExpression expr(
      std::make_unique<ast::IdentifierExpression>("textureLoad"),
      std::move(call_params));

  EXPECT_TRUE(td()->DetermineResultType(&expr));

  if (coords_length > 1) {
    EXPECT_EQ(b()->GenerateExpression(&expr), 15u) << b()->error();
  } else {
    EXPECT_EQ(b()->GenerateExpression(&expr), 12u) << b()->error();
  }

  EXPECT_EQ(DumpInstructions(b()->types()),
            generate_type_str(dim, format, false, type, 0, 2, type_id,
                              coords_length));

  EXPECT_EQ(DumpInstructions(b()->functions()[0].instructions()),
            generate_ops_str(coords_length, "OpImageRead"));
}

TEST_P(Builder_TextureLoad, Sampled) {
  auto dim = GetParam().dim;
  auto type = GetParam().type;
  auto format = GetParam().format;

  ast::type::F32Type f32;
  b()->GenerateTypeIfNeeded(&f32);
  ast::type::U32Type u32;
  b()->GenerateTypeIfNeeded(&u32);
  ast::type::I32Type i32;
  b()->GenerateTypeIfNeeded(&i32);

  uint32_t type_id = 1;
  if (type == TextureType::kU32) {
    type_id = 2;
  } else if (type == TextureType::kI32) {
    type_id = 3;
  }

  auto coords_type = get_coords_type(dim, &i32);

  uint32_t coords_length = 1;
  if (coords_type->IsVector()) {
    coords_length = coords_type->AsVector()->size();
  }

  std::unique_ptr<ast::type::Type> s = subtype(type);
  auto* texture_type = ctx()->type_mgr().Get(
      std::make_unique<ast::type::SampledTextureType>(dim, s.get()));

  EXPECT_TRUE(td()->Determine());

  ast::ExpressionList call_params;
  b()->push_function(Function{});

  add_call_param("texture", texture_type, &call_params);
  add_call_param("coords", coords_type.get(), &call_params);
  add_call_param("lod", &i32, &call_params);

  ast::CallExpression expr(
      std::make_unique<ast::IdentifierExpression>("textureLoad"),
      std::move(call_params));

  EXPECT_TRUE(td()->DetermineResultType(&expr));

  if (coords_length > 1) {
    EXPECT_EQ(b()->GenerateExpression(&expr), 15u) << b()->error();
  } else {
    EXPECT_EQ(b()->GenerateExpression(&expr), 12u) << b()->error();
  }

  EXPECT_EQ(
      DumpInstructions(b()->types()),
      generate_type_str(dim, format, true, type, 0, 1, type_id, coords_length));

  EXPECT_EQ(DumpInstructions(b()->functions()[0].instructions()),
            generate_ops_str(coords_length, "OpImageFetch"));
}

INSTANTIATE_TEST_SUITE_P(
    BuilderTest,
    Builder_TextureLoad,
    testing::Values(
        TextureTestParams{ast::type::TextureDimension::k1d, TextureType::kF32,
                          ast::type::ImageFormat::kR16Float},
        TextureTestParams{ast::type::TextureDimension::k1d, TextureType::kI32,
                          ast::type::ImageFormat::kR16Sint},
        TextureTestParams{ast::type::TextureDimension::k1d, TextureType::kU32,
                          ast::type::ImageFormat::kR8Unorm},
        TextureTestParams{ast::type::TextureDimension::k1dArray,
                          TextureType::kF32, ast::type::ImageFormat::kR16Float},
        TextureTestParams{ast::type::TextureDimension::k1dArray,
                          TextureType::kI32, ast::type::ImageFormat::kR16Sint},
        TextureTestParams{ast::type::TextureDimension::k1dArray,
                          TextureType::kU32, ast::type::ImageFormat::kR8Unorm},
        TextureTestParams{ast::type::TextureDimension::k2d, TextureType::kF32,
                          ast::type::ImageFormat::kR16Float},
        TextureTestParams{ast::type::TextureDimension::k2d, TextureType::kI32,
                          ast::type::ImageFormat::kR16Sint},
        TextureTestParams{ast::type::TextureDimension::k2d, TextureType::kU32,
                          ast::type::ImageFormat::kR8Unorm},
        TextureTestParams{ast::type::TextureDimension::k2dArray,
                          TextureType::kF32, ast::type::ImageFormat::kR16Float},
        TextureTestParams{ast::type::TextureDimension::k2dArray,
                          TextureType::kI32, ast::type::ImageFormat::kR16Sint},
        TextureTestParams{ast::type::TextureDimension::k2dArray,
                          TextureType::kU32, ast::type::ImageFormat::kR8Unorm},
        TextureTestParams{ast::type::TextureDimension::k3d, TextureType::kF32,
                          ast::type::ImageFormat::kR16Float},
        TextureTestParams{ast::type::TextureDimension::k3d, TextureType::kI32,
                          ast::type::ImageFormat::kR16Sint},
        TextureTestParams{ast::type::TextureDimension::k3d, TextureType::kU32,
                          ast::type::ImageFormat::kR8Unorm}));

class Builder_SampledTextureOperation : public Builder_TextureOperation {
 public:
  std::string generate_type_str(ast::type::TextureDimension dim,
                                bool unknown_format,
                                TextureType type,
                                uint32_t depth_literal,
                                uint32_t sampled_literal,
                                uint32_t type_id,
                                uint32_t coords_length,
                                bool optional_operand) {
    std::string type_str = R"(%1 = OpTypeFloat 32
%2 = OpTypeInt 32 0
%3 = OpTypeInt 32 1
)";

    type_str +=
        texture_line(dim, unknown_format, type, depth_literal, sampled_literal);

    type_str += R"(%5 = OpTypePointer Private %6
%7 = OpConstantNull %6
%4 = OpVariable %5 Private %7
%10 = OpTypeSampler
%9 = OpTypePointer Private %10
%11 = OpConstantNull %10
%8 = OpVariable %9 Private %11
)";

    if (coords_length > 1) {
      type_str += "%14 = OpTypeVector %3 " + std::to_string(coords_length) +
                  R"(
%13 = OpTypePointer Private %14
%15 = OpConstantNull %14
%12 = OpVariable %13 Private %15
)";

    } else {
      type_str += R"(%13 = OpTypePointer Private %3
%14 = OpConstantNull %3
%12 = OpVariable %13 Private %14
)";
    }

    if (coords_length > 1 && optional_operand) {
      type_str += R"(%17 = OpTypePointer Private %1
%18 = OpConstantNull %1
%16 = OpVariable %17 Private %18
%20 = OpTypeVector %)" +
                  std::to_string(type_id) + R"( 4
%25 = OpTypeSampledImage %6
)";
    } else if (coords_length > 1 && !optional_operand) {
      type_str += R"(%17 = OpTypeVector %)" + std::to_string(type_id) + R"( 4
%21 = OpTypeSampledImage %6
)";
    } else if (coords_length == 1 && optional_operand) {
      type_str += R"(%16 = OpTypePointer Private %1
%17 = OpConstantNull %1
%15 = OpVariable %16 Private %17
%19 = OpTypeVector %)" +
                  std::to_string(type_id) + R"( 4
%24 = OpTypeSampledImage %6
)";
    } else {
      type_str += R"(%16 = OpTypeVector %)" + std::to_string(type_id) + R"( 4
%20 = OpTypeSampledImage %6
)";
    }

    return type_str;
  }

  std::string generate_ops_str(uint32_t coords_length,
                               std::string op_name,
                               std::string optional_operand) {
    if (coords_length > 1 && optional_operand == "") {
      return R"(%18 = OpLoad %6 %4
%19 = OpLoad %10 %8
%20 = OpLoad %14 %12
%22 = OpSampledImage %21 %18 %19
%16 = )" + op_name +
             R"( %17 %22 %20
)";
    }

    if (coords_length == 1 && optional_operand == "") {
      return R"(%17 = OpLoad %6 %4
%18 = OpLoad %10 %8
%19 = OpLoad %3 %12
%21 = OpSampledImage %20 %17 %18
%15 = )" + op_name +
             R"( %16 %21 %19
)";
    }

    if (coords_length > 1 && optional_operand != "") {
      return R"(%21 = OpLoad %6 %4
%22 = OpLoad %10 %8
%23 = OpLoad %14 %12
%24 = OpLoad %1 %16
%26 = OpSampledImage %25 %21 %22
%19 = )" + op_name +
             " %20 %26 %23 " + optional_operand + " %24\n";
    }

    return R"(%20 = OpLoad %6 %4
%21 = OpLoad %10 %8
%22 = OpLoad %3 %12
%23 = OpLoad %1 %15
%25 = OpSampledImage %24 %20 %21
%18 = )" + op_name +
           " %19 %25 %22 " + optional_operand + " %23\n";
  }
};

TEST_P(Builder_SampledTextureOperation, TextureSample) {
  auto dim = GetParam().dim;
  auto type = GetParam().type;

  ast::type::F32Type f32;
  b()->GenerateTypeIfNeeded(&f32);
  ast::type::U32Type u32;
  b()->GenerateTypeIfNeeded(&u32);
  ast::type::I32Type i32;
  b()->GenerateTypeIfNeeded(&i32);

  uint32_t type_id = 1;
  if (type == TextureType::kU32) {
    type_id = 2;
  } else if (type == TextureType::kI32) {
    type_id = 3;
  }

  auto coords_type = get_coords_type(dim, &i32);

  uint32_t coords_length = 1;
  if (coords_type->IsVector()) {
    coords_length = coords_type->AsVector()->size();
  }

  auto sampler_type = std::make_unique<ast::type::SamplerType>(
      ast::type::SamplerKind::kSampler);
  std::unique_ptr<ast::type::Type> s = subtype(type);
  auto* texture_type = ctx()->type_mgr().Get(
      std::make_unique<ast::type::SampledTextureType>(dim, s.get()));

  EXPECT_TRUE(td()->Determine());

  ast::ExpressionList call_params;
  b()->push_function(Function{});

  add_call_param("texture", texture_type, &call_params);
  add_call_param("sampler", sampler_type.get(), &call_params);
  add_call_param("coords", coords_type.get(), &call_params);

  ast::CallExpression expr(
      std::make_unique<ast::IdentifierExpression>("textureSample"),
      std::move(call_params));

  EXPECT_TRUE(td()->DetermineResultType(&expr));

  if (coords_length > 1) {
    EXPECT_EQ(b()->GenerateExpression(&expr), 16u) << b()->error();
  } else {
    EXPECT_EQ(b()->GenerateExpression(&expr), 15u) << b()->error();
  }

  EXPECT_EQ(
      DumpInstructions(b()->types()),
      generate_type_str(dim, true, type, 0, 1, type_id, coords_length, false));

  EXPECT_EQ(DumpInstructions(b()->functions()[0].instructions()),
            generate_ops_str(coords_length, "OpImageSampleImplicitLod", ""));
}

TEST_P(Builder_SampledTextureOperation, TextureSampleLevel) {
  auto dim = GetParam().dim;
  auto type = GetParam().type;

  ast::type::F32Type f32;
  b()->GenerateTypeIfNeeded(&f32);
  ast::type::U32Type u32;
  b()->GenerateTypeIfNeeded(&u32);
  ast::type::I32Type i32;
  b()->GenerateTypeIfNeeded(&i32);

  uint32_t type_id = 1;
  if (type == TextureType::kU32) {
    type_id = 2;
  } else if (type == TextureType::kI32) {
    type_id = 3;
  }

  auto coords_type = get_coords_type(dim, &i32);

  uint32_t coords_length = 1;
  if (coords_type->IsVector()) {
    coords_length = coords_type->AsVector()->size();
  }

  auto sampler_type = std::make_unique<ast::type::SamplerType>(
      ast::type::SamplerKind::kSampler);
  std::unique_ptr<ast::type::Type> s = subtype(type);
  auto* texture_type = ctx()->type_mgr().Get(
      std::make_unique<ast::type::SampledTextureType>(dim, s.get()));

  EXPECT_TRUE(td()->Determine());

  ast::ExpressionList call_params;
  b()->push_function(Function{});

  add_call_param("texture", texture_type, &call_params);
  add_call_param("sampler", sampler_type.get(), &call_params);
  add_call_param("coords", coords_type.get(), &call_params);
  add_call_param("lod", &f32, &call_params);

  ast::CallExpression expr(
      std::make_unique<ast::IdentifierExpression>("textureSampleLevel"),
      std::move(call_params));

  EXPECT_TRUE(td()->DetermineResultType(&expr));

  if (coords_length > 1) {
    EXPECT_EQ(b()->GenerateExpression(&expr), 19u) << b()->error();
  } else {
    EXPECT_EQ(b()->GenerateExpression(&expr), 18u) << b()->error();
  }

  EXPECT_EQ(
      DumpInstructions(b()->types()),
      generate_type_str(dim, true, type, 0, 1, type_id, coords_length, true));

  EXPECT_EQ(DumpInstructions(b()->functions()[0].instructions()),
            generate_ops_str(coords_length, "OpImageSampleExplicitLod", "Lod"));
}

TEST_P(Builder_SampledTextureOperation, TextureSampleBias) {
  auto dim = GetParam().dim;
  auto type = GetParam().type;

  ast::type::F32Type f32;
  b()->GenerateTypeIfNeeded(&f32);
  ast::type::U32Type u32;
  b()->GenerateTypeIfNeeded(&u32);
  ast::type::I32Type i32;
  b()->GenerateTypeIfNeeded(&i32);

  uint32_t type_id = 1;
  if (type == TextureType::kU32) {
    type_id = 2;
  } else if (type == TextureType::kI32) {
    type_id = 3;
  }

  auto coords_type = get_coords_type(dim, &i32);

  uint32_t coords_length = 1;
  if (coords_type->IsVector()) {
    coords_length = coords_type->AsVector()->size();
  }

  auto sampler_type = std::make_unique<ast::type::SamplerType>(
      ast::type::SamplerKind::kSampler);
  std::unique_ptr<ast::type::Type> s = subtype(type);
  auto* texture_type = ctx()->type_mgr().Get(
      std::make_unique<ast::type::SampledTextureType>(dim, s.get()));

  EXPECT_TRUE(td()->Determine());

  ast::ExpressionList call_params;
  b()->push_function(Function{});

  add_call_param("texture", texture_type, &call_params);
  add_call_param("sampler", sampler_type.get(), &call_params);
  add_call_param("coords", coords_type.get(), &call_params);
  add_call_param("bias", &f32, &call_params);

  ast::CallExpression expr(
      std::make_unique<ast::IdentifierExpression>("textureSampleBias"),
      std::move(call_params));

  EXPECT_TRUE(td()->DetermineResultType(&expr));

  if (coords_length > 1) {
    EXPECT_EQ(b()->GenerateExpression(&expr), 19u) << b()->error();
  } else {
    EXPECT_EQ(b()->GenerateExpression(&expr), 18u) << b()->error();
  }

  EXPECT_EQ(
      DumpInstructions(b()->types()),
      generate_type_str(dim, true, type, 0, 1, type_id, coords_length, true));

  EXPECT_EQ(
      DumpInstructions(b()->functions()[0].instructions()),
      generate_ops_str(coords_length, "OpImageSampleImplicitLod", "Bias"));
}

INSTANTIATE_TEST_SUITE_P(
    BuilderTest,
    Builder_SampledTextureOperation,
    testing::Values(
        TextureTestParams{ast::type::TextureDimension::k1d, TextureType::kF32},
        TextureTestParams{ast::type::TextureDimension::k1d, TextureType::kI32},
        TextureTestParams{ast::type::TextureDimension::k1d, TextureType::kU32},
        TextureTestParams{ast::type::TextureDimension::k1dArray,
                          TextureType::kF32},
        TextureTestParams{ast::type::TextureDimension::k1dArray,
                          TextureType::kI32},
        TextureTestParams{ast::type::TextureDimension::k1dArray,
                          TextureType::kU32},
        TextureTestParams{ast::type::TextureDimension::k2d, TextureType::kF32},
        TextureTestParams{ast::type::TextureDimension::k2d, TextureType::kI32},
        TextureTestParams{ast::type::TextureDimension::k2d, TextureType::kU32},
        TextureTestParams{ast::type::TextureDimension::k2dArray,
                          TextureType::kF32},
        TextureTestParams{ast::type::TextureDimension::k2dArray,
                          TextureType::kI32},
        TextureTestParams{ast::type::TextureDimension::k2dArray,
                          TextureType::kU32},
        TextureTestParams{ast::type::TextureDimension::k3d, TextureType::kF32},
        TextureTestParams{ast::type::TextureDimension::k3d, TextureType::kI32},
        TextureTestParams{ast::type::TextureDimension::k3d,
                          TextureType::kU32}));

class Builder_DepthTextureOperation : public Builder_TextureOperation {
 public:
  std::string generate_type_str(ast::type::TextureDimension dim,
                                uint32_t coords_length) {
    std::string type_str = R"(%1 = OpTypeFloat 32
%2 = OpTypeInt 32 0
%3 = OpTypeInt 32 1
)";

    type_str += texture_line(dim, true, TextureType::kF32, 1, 1);

    type_str += R"(%5 = OpTypePointer Private %6
%7 = OpConstantNull %6
%4 = OpVariable %5 Private %7
%10 = OpTypeSampler
%9 = OpTypePointer Private %10
%11 = OpConstantNull %10
%8 = OpVariable %9 Private %11
%14 = OpTypeVector %1 )" +
                std::to_string(coords_length) + R"(
%13 = OpTypePointer Private %14
%15 = OpConstantNull %14
%12 = OpVariable %13 Private %15
%17 = OpTypePointer Private %1
%18 = OpConstantNull %1
%16 = OpVariable %17 Private %18
%24 = OpTypeSampledImage %6
%26 = OpConstant %1 0
)";

    return type_str;
  }

  std::string generate_ops_str() {
    return R"(%20 = OpLoad %6 %4
%21 = OpLoad %10 %8
%22 = OpLoad %14 %12
%23 = OpLoad %1 %16
%25 = OpSampledImage %24 %20 %21
%19 = OpImageSampleDrefExplicitLod %1 %25 %22 %23 Lod %26
)";
  }
};

TEST_P(Builder_DepthTextureOperation, TextureSampleCompare) {
  auto dim = GetParam().dim;
  auto type = GetParam().type;

  ast::type::F32Type f32;
  b()->GenerateTypeIfNeeded(&f32);
  ast::type::U32Type u32;
  b()->GenerateTypeIfNeeded(&u32);
  ast::type::I32Type i32;
  b()->GenerateTypeIfNeeded(&i32);

  auto coords_type = get_coords_type(dim, &f32);

  uint32_t coords_length = coords_type->AsVector()->size();

  auto sampler_type = std::make_unique<ast::type::SamplerType>(
      ast::type::SamplerKind::kComparisonSampler);
  std::unique_ptr<ast::type::Type> s = subtype(type);
  auto* texture_type =
      ctx()->type_mgr().Get(std::make_unique<ast::type::DepthTextureType>(dim));

  ast::ExpressionList call_params;
  b()->push_function(Function{});

  add_call_param("texture", texture_type, &call_params);
  add_call_param("sampler", sampler_type.get(), &call_params);
  add_call_param("coords", coords_type.get(), &call_params);
  add_call_param("depth_reference", &f32, &call_params);

  ast::CallExpression expr(
      std::make_unique<ast::IdentifierExpression>("textureSampleCompare"),
      std::move(call_params));

  EXPECT_TRUE(td()->DetermineResultType(&expr));

  EXPECT_EQ(b()->GenerateExpression(&expr), 19u) << b()->error();

  EXPECT_EQ(DumpInstructions(b()->types()),
            generate_type_str(dim, coords_length));

  EXPECT_EQ(DumpInstructions(b()->functions()[0].instructions()),
            generate_ops_str());
}

INSTANTIATE_TEST_SUITE_P(
    BuilderTest,
    Builder_DepthTextureOperation,
    testing::Values(TextureTestParams{ast::type::TextureDimension::k2d},
                    TextureTestParams{ast::type::TextureDimension::k2dArray},
                    TextureTestParams{ast::type::TextureDimension::kCube},
                    TextureTestParams{
                        ast::type::TextureDimension::kCubeArray}));

// This tests that we do not push OpTypeSampledImage and float_0 type twice.
TEST_F(Builder_TextureOperation, TextureSampleCompareTwice) {
  ast::type::F32Type f32;
  b()->GenerateTypeIfNeeded(&f32);

  auto coords_type = get_coords_type(ast::type::TextureDimension::k2d, &f32);
  auto sampler_type = std::make_unique<ast::type::SamplerType>(
      ast::type::SamplerKind::kComparisonSampler);
  auto* texture_type =
      ctx()->type_mgr().Get(std::make_unique<ast::type::DepthTextureType>(
          ast::type::TextureDimension::k2d));

  b()->push_function(Function{});

  ast::ExpressionList call_params_first;
  add_call_param("texture", texture_type, &call_params_first);
  add_call_param("sampler", sampler_type.get(), &call_params_first);
  add_call_param("coords", coords_type.get(), &call_params_first);
  add_call_param("depth_reference", &f32, &call_params_first);

  ast::ExpressionList call_params_second;
  add_call_param("texture", texture_type, &call_params_second);
  add_call_param("sampler", sampler_type.get(), &call_params_second);
  add_call_param("coords", coords_type.get(), &call_params_second);
  add_call_param("depth_reference", &f32, &call_params_second);

  ast::CallExpression expr_first(
      std::make_unique<ast::IdentifierExpression>("textureSampleCompare"),
      std::move(call_params_first));
  ast::CallExpression expr_second(
      std::make_unique<ast::IdentifierExpression>("textureSampleCompare"),
      std::move(call_params_second));

  EXPECT_TRUE(td()->DetermineResultType(&expr_first));
  EXPECT_TRUE(td()->DetermineResultType(&expr_second));

  EXPECT_EQ(b()->GenerateExpression(&expr_first), 21u) << b()->error();
  EXPECT_EQ(b()->GenerateExpression(&expr_second), 29u) << b()->error();

  EXPECT_EQ(DumpInstructions(b()->types()), R"(%1 = OpTypeFloat 32
%4 = OpTypeImage %1 2D 1 0 0 1 Unknown
%3 = OpTypePointer Private %4
%5 = OpConstantNull %4
%2 = OpVariable %3 Private %5
%8 = OpTypeSampler
%7 = OpTypePointer Private %8
%9 = OpConstantNull %8
%6 = OpVariable %7 Private %9
%12 = OpTypeVector %1 2
%11 = OpTypePointer Private %12
%13 = OpConstantNull %12
%10 = OpVariable %11 Private %13
%15 = OpTypePointer Private %1
%16 = OpConstantNull %1
%14 = OpVariable %15 Private %16
%17 = OpVariable %3 Private %5
%18 = OpVariable %7 Private %9
%19 = OpVariable %11 Private %13
%20 = OpVariable %15 Private %16
%26 = OpTypeSampledImage %4
%28 = OpConstant %1 0
)");

  EXPECT_EQ(DumpInstructions(b()->functions()[0].instructions()),
            R"(%22 = OpLoad %4 %17
%23 = OpLoad %8 %18
%24 = OpLoad %12 %19
%25 = OpLoad %1 %20
%27 = OpSampledImage %26 %22 %23
%21 = OpImageSampleDrefExplicitLod %1 %27 %24 %25 Lod %28
%30 = OpLoad %4 %17
%31 = OpLoad %8 %18
%32 = OpLoad %12 %19
%33 = OpLoad %1 %20
%34 = OpSampledImage %26 %30 %31
%29 = OpImageSampleDrefExplicitLod %1 %34 %32 %33 Lod %28
)");
}

}  // namespace
}  // namespace spirv
}  // namespace writer
}  // namespace tint
