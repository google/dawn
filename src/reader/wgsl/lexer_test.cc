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

#include "src/reader/wgsl/lexer.h"

#include <limits>

#include "gtest/gtest.h"

namespace tint {
namespace reader {
namespace wgsl {
namespace {

using LexerTest = testing::Test;

TEST_F(LexerTest, Empty) {
  Source::FileContent content("");
  Lexer l("test.wgsl", &content);
  auto t = l.next();
  EXPECT_TRUE(t.IsEof());
}

TEST_F(LexerTest, Skips_Whitespace) {
  Source::FileContent content("\t\r\n\t    ident\t\n\t  \r ");
  Lexer l("test.wgsl", &content);

  auto t = l.next();
  EXPECT_TRUE(t.IsIdentifier());
  EXPECT_EQ(t.source().range.begin.line, 2u);
  EXPECT_EQ(t.source().range.begin.column, 6u);
  EXPECT_EQ(t.source().range.end.line, 2u);
  EXPECT_EQ(t.source().range.end.column, 11u);
  EXPECT_EQ(t.to_str(), "ident");

  t = l.next();
  EXPECT_TRUE(t.IsEof());
}

TEST_F(LexerTest, Skips_Comments) {
  Source::FileContent content(R"(//starts with comment
ident1 //ends with comment
// blank line
 ident2)");
  Lexer l("test.wgsl", &content);

  auto t = l.next();
  EXPECT_TRUE(t.IsIdentifier());
  EXPECT_EQ(t.source().range.begin.line, 2u);
  EXPECT_EQ(t.source().range.begin.column, 1u);
  EXPECT_EQ(t.source().range.end.line, 2u);
  EXPECT_EQ(t.source().range.end.column, 7u);
  EXPECT_EQ(t.to_str(), "ident1");

  t = l.next();
  EXPECT_TRUE(t.IsIdentifier());
  EXPECT_EQ(t.source().range.begin.line, 4u);
  EXPECT_EQ(t.source().range.begin.column, 2u);
  EXPECT_EQ(t.source().range.end.line, 4u);
  EXPECT_EQ(t.source().range.end.column, 8u);
  EXPECT_EQ(t.to_str(), "ident2");

  t = l.next();
  EXPECT_TRUE(t.IsEof());
}

struct FloatData {
  const char* input;
  float result;
};
inline std::ostream& operator<<(std::ostream& out, FloatData data) {
  out << std::string(data.input);
  return out;
}
using FloatTest = testing::TestWithParam<FloatData>;
TEST_P(FloatTest, Parse) {
  auto params = GetParam();
  Source::FileContent content(params.input);
  Lexer l("test.wgsl", &content);

  auto t = l.next();
  EXPECT_TRUE(t.IsFloatLiteral());
  EXPECT_EQ(t.to_f32(), params.result);
  EXPECT_EQ(t.source().range.begin.line, 1u);
  EXPECT_EQ(t.source().range.begin.column, 1u);
  EXPECT_EQ(t.source().range.end.line, 1u);
  EXPECT_EQ(t.source().range.end.column, 1u + strlen(params.input));

  t = l.next();
  EXPECT_TRUE(t.IsEof());
}
INSTANTIATE_TEST_SUITE_P(LexerTest,
                         FloatTest,
                         testing::Values(FloatData{"0.0", 0.0f},
                                         FloatData{"0.", 0.0f},
                                         FloatData{".0", 0.0f},
                                         FloatData{"5.7", 5.7f},
                                         FloatData{"5.", 5.f},
                                         FloatData{".7", .7f},
                                         FloatData{"-0.0", 0.0f},
                                         FloatData{"-.0", 0.0f},
                                         FloatData{"-0.", 0.0f},
                                         FloatData{"-5.7", -5.7f},
                                         FloatData{"-5.", -5.f},
                                         FloatData{"-.7", -.7f},
                                         FloatData{"0.2e+12", 0.2e12f},
                                         FloatData{"1.2e-5", 1.2e-5f},
                                         FloatData{"2.57e23", 2.57e23f},
                                         FloatData{"2.5e+0", 2.5f},
                                         FloatData{"2.5e-0", 2.5f}));

using FloatTest_Invalid = testing::TestWithParam<const char*>;
TEST_P(FloatTest_Invalid, Handles) {
  Source::FileContent content(GetParam());
  Lexer l("test.wgsl", &content);

  auto t = l.next();
  EXPECT_FALSE(t.IsFloatLiteral());
}
INSTANTIATE_TEST_SUITE_P(LexerTest,
                         FloatTest_Invalid,
                         testing::Values(".",
                                         "-.",
                                         "2.5e+256",
                                         "-2.5e+127",
                                         "2.5e-300",
                                         "2.5e 12",
                                         "2.5e+ 123"));

using IdentifierTest = testing::TestWithParam<const char*>;
TEST_P(IdentifierTest, Parse) {
  Source::FileContent content(GetParam());
  Lexer l("test.wgsl", &content);

  auto t = l.next();
  EXPECT_TRUE(t.IsIdentifier());
  EXPECT_EQ(t.source().range.begin.line, 1u);
  EXPECT_EQ(t.source().range.begin.column, 1u);
  EXPECT_EQ(t.source().range.end.line, 1u);
  EXPECT_EQ(t.source().range.end.column, 1u + strlen(GetParam()));
  EXPECT_EQ(t.to_str(), GetParam());
}
INSTANTIATE_TEST_SUITE_P(
    LexerTest,
    IdentifierTest,
    testing::Values("test01", "_test_", "test_", "_test", "_01", "_test01"));

TEST_F(LexerTest, IdentifierTest_DoesNotStartWithNumber) {
  Source::FileContent content("01test");
  Lexer l("test.wgsl", &content);

  auto t = l.next();
  EXPECT_FALSE(t.IsIdentifier());
}

struct HexSignedIntData {
  const char* input;
  int32_t result;
};
inline std::ostream& operator<<(std::ostream& out, HexSignedIntData data) {
  out << std::string(data.input);
  return out;
}

using IntegerTest_HexSigned = testing::TestWithParam<HexSignedIntData>;
TEST_P(IntegerTest_HexSigned, Matches) {
  auto params = GetParam();
  Source::FileContent content(params.input);
  Lexer l("test.wgsl", &content);

  auto t = l.next();
  EXPECT_TRUE(t.IsSintLiteral());
  EXPECT_EQ(t.source().range.begin.line, 1u);
  EXPECT_EQ(t.source().range.begin.column, 1u);
  EXPECT_EQ(t.source().range.end.line, 1u);
  EXPECT_EQ(t.source().range.end.column, 1u + strlen(params.input));
  EXPECT_EQ(t.to_i32(), params.result);
}
INSTANTIATE_TEST_SUITE_P(
    LexerTest,
    IntegerTest_HexSigned,
    testing::Values(
        HexSignedIntData{"0x0", 0},
        HexSignedIntData{"0x42", 66},
        HexSignedIntData{"-0x42", -66},
        HexSignedIntData{"0xeF1Abc9", 250719177},
        HexSignedIntData{"-0x80000000", std::numeric_limits<int32_t>::min()},
        HexSignedIntData{"0x7FFFFFFF", std::numeric_limits<int32_t>::max()}));

TEST_F(LexerTest, IntegerTest_HexSignedTooLarge) {
  Source::FileContent content("0x80000000");
  Lexer l("test.wgsl", &content);

  auto t = l.next();
  ASSERT_TRUE(t.IsError());
  EXPECT_EQ(t.to_str(), "i32 (0x80000000) too large");
}

TEST_F(LexerTest, IntegerTest_HexSignedTooSmall) {
  Source::FileContent content("-0x8000000F");
  Lexer l("test.wgsl", &content);

  auto t = l.next();
  ASSERT_TRUE(t.IsError());
  EXPECT_EQ(t.to_str(), "i32 (-0x8000000F) too small");
}

struct HexUnsignedIntData {
  const char* input;
  uint32_t result;
};
inline std::ostream& operator<<(std::ostream& out, HexUnsignedIntData data) {
  out << std::string(data.input);
  return out;
}
using IntegerTest_HexUnsigned = testing::TestWithParam<HexUnsignedIntData>;
TEST_P(IntegerTest_HexUnsigned, Matches) {
  auto params = GetParam();
  Source::FileContent content(params.input);
  Lexer l("test.wgsl", &content);

  auto t = l.next();
  EXPECT_TRUE(t.IsUintLiteral());
  EXPECT_EQ(t.source().range.begin.line, 1u);
  EXPECT_EQ(t.source().range.begin.column, 1u);
  EXPECT_EQ(t.source().range.end.line, 1u);
  EXPECT_EQ(t.source().range.end.column, 1u + strlen(params.input));
  EXPECT_EQ(t.to_u32(), params.result);

  t = l.next();
  EXPECT_TRUE(t.IsEof());
}
INSTANTIATE_TEST_SUITE_P(
    LexerTest,
    IntegerTest_HexUnsigned,
    testing::Values(HexUnsignedIntData{"0x0u", 0},
                    HexUnsignedIntData{"0x42u", 66},
                    HexUnsignedIntData{"0xeF1Abc9u", 250719177},
                    HexUnsignedIntData{"0x0u",
                                       std::numeric_limits<uint32_t>::min()},
                    HexUnsignedIntData{"0xFFFFFFFFu",
                                       std::numeric_limits<uint32_t>::max()}));

TEST_F(LexerTest, IntegerTest_HexUnsignedTooLarge) {
  Source::FileContent content("0xffffffffffu");
  Lexer l("test.wgsl", &content);

  auto t = l.next();
  ASSERT_TRUE(t.IsError());
  EXPECT_EQ(t.to_str(), "u32 (0xffffffffff) too large");
}

struct UnsignedIntData {
  const char* input;
  uint32_t result;
};
inline std::ostream& operator<<(std::ostream& out, UnsignedIntData data) {
  out << std::string(data.input);
  return out;
}
using IntegerTest_Unsigned = testing::TestWithParam<UnsignedIntData>;
TEST_P(IntegerTest_Unsigned, Matches) {
  auto params = GetParam();
  Source::FileContent content(params.input);
  Lexer l("test.wgsl", &content);

  auto t = l.next();
  EXPECT_TRUE(t.IsUintLiteral());
  EXPECT_EQ(t.to_u32(), params.result);
  EXPECT_EQ(t.source().range.begin.line, 1u);
  EXPECT_EQ(t.source().range.begin.column, 1u);
  EXPECT_EQ(t.source().range.end.line, 1u);
  EXPECT_EQ(t.source().range.end.column, 1u + strlen(params.input));
}
INSTANTIATE_TEST_SUITE_P(LexerTest,
                         IntegerTest_Unsigned,
                         testing::Values(UnsignedIntData{"0u", 0u},
                                         UnsignedIntData{"123u", 123u},
                                         UnsignedIntData{"4294967295u",
                                                         4294967295u}));

struct SignedIntData {
  const char* input;
  int32_t result;
};
inline std::ostream& operator<<(std::ostream& out, SignedIntData data) {
  out << std::string(data.input);
  return out;
}
using IntegerTest_Signed = testing::TestWithParam<SignedIntData>;
TEST_P(IntegerTest_Signed, Matches) {
  auto params = GetParam();
  Source::FileContent content(params.input);
  Lexer l("test.wgsl", &content);

  auto t = l.next();
  EXPECT_TRUE(t.IsSintLiteral());
  EXPECT_EQ(t.to_i32(), params.result);
  EXPECT_EQ(t.source().range.begin.line, 1u);
  EXPECT_EQ(t.source().range.begin.column, 1u);
  EXPECT_EQ(t.source().range.end.line, 1u);
  EXPECT_EQ(t.source().range.end.column, 1u + strlen(params.input));
}
INSTANTIATE_TEST_SUITE_P(
    LexerTest,
    IntegerTest_Signed,
    testing::Values(SignedIntData{"0", 0},
                    SignedIntData{"-2", -2},
                    SignedIntData{"2", 2},
                    SignedIntData{"123", 123},
                    SignedIntData{"2147483647", 2147483647},
                    SignedIntData{"-2147483648", -2147483648LL}));

using IntegerTest_Invalid = testing::TestWithParam<const char*>;
TEST_P(IntegerTest_Invalid, Parses) {
  Source::FileContent content(GetParam());
  Lexer l("test.wgsl", &content);

  auto t = l.next();
  EXPECT_FALSE(t.IsSintLiteral());
  EXPECT_FALSE(t.IsUintLiteral());
}
INSTANTIATE_TEST_SUITE_P(LexerTest,
                         IntegerTest_Invalid,
                         testing::Values("2147483648", "4294967296u"));

struct TokenData {
  const char* input;
  Token::Type type;
};
inline std::ostream& operator<<(std::ostream& out, TokenData data) {
  out << std::string(data.input);
  return out;
}
using PunctuationTest = testing::TestWithParam<TokenData>;
TEST_P(PunctuationTest, Parses) {
  auto params = GetParam();
  Source::FileContent content(params.input);
  Lexer l("test.wgsl", &content);

  auto t = l.next();
  EXPECT_TRUE(t.Is(params.type));
  EXPECT_EQ(t.source().range.begin.line, 1u);
  EXPECT_EQ(t.source().range.begin.column, 1u);
  EXPECT_EQ(t.source().range.end.line, 1u);
  EXPECT_EQ(t.source().range.end.column, 1u + strlen(params.input));

  t = l.next();
  EXPECT_EQ(t.source().range.begin.column,
            1 + std::string(params.input).size());
}
INSTANTIATE_TEST_SUITE_P(
    LexerTest,
    PunctuationTest,
    testing::Values(TokenData{"&", Token::Type::kAnd},
                    TokenData{"&&", Token::Type::kAndAnd},
                    TokenData{"->", Token::Type::kArrow},
                    TokenData{"[[", Token::Type::kAttrLeft},
                    TokenData{"]]", Token::Type::kAttrRight},
                    TokenData{"/", Token::Type::kForwardSlash},
                    TokenData{"!", Token::Type::kBang},
                    TokenData{"[", Token::Type::kBracketLeft},
                    TokenData{"]", Token::Type::kBracketRight},
                    TokenData{"{", Token::Type::kBraceLeft},
                    TokenData{"}", Token::Type::kBraceRight},
                    TokenData{":", Token::Type::kColon},
                    TokenData{",", Token::Type::kComma},
                    TokenData{"=", Token::Type::kEqual},
                    TokenData{"==", Token::Type::kEqualEqual},
                    TokenData{">", Token::Type::kGreaterThan},
                    TokenData{">=", Token::Type::kGreaterThanEqual},
                    TokenData{">>", Token::Type::kShiftRight},
                    TokenData{"<", Token::Type::kLessThan},
                    TokenData{"<=", Token::Type::kLessThanEqual},
                    TokenData{"<<", Token::Type::kShiftLeft},
                    TokenData{"%", Token::Type::kMod},
                    TokenData{"!=", Token::Type::kNotEqual},
                    TokenData{"-", Token::Type::kMinus},
                    TokenData{".", Token::Type::kPeriod},
                    TokenData{"+", Token::Type::kPlus},
                    TokenData{"|", Token::Type::kOr},
                    TokenData{"||", Token::Type::kOrOr},
                    TokenData{"(", Token::Type::kParenLeft},
                    TokenData{")", Token::Type::kParenRight},
                    TokenData{";", Token::Type::kSemicolon},
                    TokenData{"*", Token::Type::kStar},
                    TokenData{"^", Token::Type::kXor}));

using KeywordTest = testing::TestWithParam<TokenData>;
TEST_P(KeywordTest, Parses) {
  auto params = GetParam();
  Source::FileContent content(params.input);
  Lexer l("test.wgsl", &content);

  auto t = l.next();
  EXPECT_TRUE(t.Is(params.type)) << params.input;
  EXPECT_EQ(t.source().range.begin.line, 1u);
  EXPECT_EQ(t.source().range.begin.column, 1u);
  EXPECT_EQ(t.source().range.end.line, 1u);
  EXPECT_EQ(t.source().range.end.column, 1u + strlen(params.input));

  t = l.next();
  EXPECT_EQ(t.source().range.begin.column,
            1 + std::string(params.input).size());
}
INSTANTIATE_TEST_SUITE_P(
    LexerTest,
    KeywordTest,
    testing::Values(
        TokenData{"array", Token::Type::kArray},
        TokenData{"bitcast", Token::Type::kBitcast},
        TokenData{"bool", Token::Type::kBool},
        TokenData{"break", Token::Type::kBreak},
        TokenData{"case", Token::Type::kCase},
        TokenData{"const", Token::Type::kConst},
        TokenData{"continue", Token::Type::kContinue},
        TokenData{"continuing", Token::Type::kContinuing},
        TokenData{"default", Token::Type::kDefault},
        TokenData{"discard", Token::Type::kDiscard},
        TokenData{"else", Token::Type::kElse},
        TokenData{"elseif", Token::Type::kElseIf},
        TokenData{"f32", Token::Type::kF32},
        TokenData{"fallthrough", Token::Type::kFallthrough},
        TokenData{"false", Token::Type::kFalse},
        TokenData{"fn", Token::Type::kFn},
        TokenData{"for", Token::Type::kFor},
        TokenData{"bgra8unorm", Token::Type::kFormatBgra8Unorm},
        TokenData{"bgra8unorm_srgb", Token::Type::kFormatBgra8UnormSrgb},
        TokenData{"r16float", Token::Type::kFormatR16Float},
        TokenData{"r16sint", Token::Type::kFormatR16Sint},
        TokenData{"r16uint", Token::Type::kFormatR16Uint},
        TokenData{"r32float", Token::Type::kFormatR32Float},
        TokenData{"r32sint", Token::Type::kFormatR32Sint},
        TokenData{"r32uint", Token::Type::kFormatR32Uint},
        TokenData{"r8sint", Token::Type::kFormatR8Sint},
        TokenData{"r8snorm", Token::Type::kFormatR8Snorm},
        TokenData{"r8uint", Token::Type::kFormatR8Uint},
        TokenData{"r8unorm", Token::Type::kFormatR8Unorm},
        TokenData{"rg11b10float", Token::Type::kFormatRg11B10Float},
        TokenData{"rg16float", Token::Type::kFormatRg16Float},
        TokenData{"rg16sint", Token::Type::kFormatRg16Sint},
        TokenData{"rg16uint", Token::Type::kFormatRg16Uint},
        TokenData{"rg32float", Token::Type::kFormatRg32Float},
        TokenData{"rg32sint", Token::Type::kFormatRg32Sint},
        TokenData{"rg32uint", Token::Type::kFormatRg32Uint},
        TokenData{"rg8sint", Token::Type::kFormatRg8Sint},
        TokenData{"rg8snorm", Token::Type::kFormatRg8Snorm},
        TokenData{"rg8uint", Token::Type::kFormatRg8Uint},
        TokenData{"rg8unorm", Token::Type::kFormatRg8Unorm},
        TokenData{"rgb10a2unorm", Token::Type::kFormatRgb10A2Unorm},
        TokenData{"rgba16float", Token::Type::kFormatRgba16Float},
        TokenData{"rgba16sint", Token::Type::kFormatRgba16Sint},
        TokenData{"rgba16uint", Token::Type::kFormatRgba16Uint},
        TokenData{"rgba32float", Token::Type::kFormatRgba32Float},
        TokenData{"rgba32sint", Token::Type::kFormatRgba32Sint},
        TokenData{"rgba32uint", Token::Type::kFormatRgba32Uint},
        TokenData{"rgba8sint", Token::Type::kFormatRgba8Sint},
        TokenData{"rgba8snorm", Token::Type::kFormatRgba8Snorm},
        TokenData{"rgba8uint", Token::Type::kFormatRgba8Uint},
        TokenData{"rgba8unorm", Token::Type::kFormatRgba8Unorm},
        TokenData{"rgba8unorm_srgb", Token::Type::kFormatRgba8UnormSrgb},
        TokenData{"function", Token::Type::kFunction},
        TokenData{"i32", Token::Type::kI32},
        TokenData{"if", Token::Type::kIf},
        TokenData{"image", Token::Type::kImage},
        TokenData{"import", Token::Type::kImport},
        TokenData{"in", Token::Type::kIn},
        TokenData{"let", Token::Type::kLet},
        TokenData{"loop", Token::Type::kLoop},
        TokenData{"mat2x2", Token::Type::kMat2x2},
        TokenData{"mat2x3", Token::Type::kMat2x3},
        TokenData{"mat2x4", Token::Type::kMat2x4},
        TokenData{"mat3x2", Token::Type::kMat3x2},
        TokenData{"mat3x3", Token::Type::kMat3x3},
        TokenData{"mat3x4", Token::Type::kMat3x4},
        TokenData{"mat4x2", Token::Type::kMat4x2},
        TokenData{"mat4x3", Token::Type::kMat4x3},
        TokenData{"mat4x4", Token::Type::kMat4x4},
        TokenData{"out", Token::Type::kOut},
        TokenData{"private", Token::Type::kPrivate},
        TokenData{"ptr", Token::Type::kPtr},
        TokenData{"return", Token::Type::kReturn},
        TokenData{"sampler", Token::Type::kSampler},
        TokenData{"sampler_comparison", Token::Type::kComparisonSampler},
        TokenData{"storage", Token::Type::kStorage},
        TokenData{"storage_buffer", Token::Type::kStorage},
        TokenData{"struct", Token::Type::kStruct},
        TokenData{"switch", Token::Type::kSwitch},
        TokenData{"texture_1d", Token::Type::kTextureSampled1d},
        TokenData{"texture_2d", Token::Type::kTextureSampled2d},
        TokenData{"texture_2d_array", Token::Type::kTextureSampled2dArray},
        TokenData{"texture_3d", Token::Type::kTextureSampled3d},
        TokenData{"texture_cube", Token::Type::kTextureSampledCube},
        TokenData{"texture_cube_array", Token::Type::kTextureSampledCubeArray},
        TokenData{"texture_depth_2d", Token::Type::kTextureDepth2d},
        TokenData{"texture_depth_2d_array", Token::Type::kTextureDepth2dArray},
        TokenData{"texture_depth_cube", Token::Type::kTextureDepthCube},
        TokenData{"texture_depth_cube_array",
                  Token::Type::kTextureDepthCubeArray},
        TokenData{"texture_multisampled_2d",
                  Token::Type::kTextureMultisampled2d},
        TokenData{"texture_storage_1d", Token::Type::kTextureStorage1d},
        TokenData{"texture_storage_2d", Token::Type::kTextureStorage2d},
        TokenData{"texture_storage_2d_array",
                  Token::Type::kTextureStorage2dArray},
        TokenData{"texture_storage_3d", Token::Type::kTextureStorage3d},
        TokenData{"true", Token::Type::kTrue},
        TokenData{"type", Token::Type::kType},
        TokenData{"u32", Token::Type::kU32},
        TokenData{"uniform", Token::Type::kUniform},
        TokenData{"var", Token::Type::kVar},
        TokenData{"vec2", Token::Type::kVec2},
        TokenData{"vec3", Token::Type::kVec3},
        TokenData{"vec4", Token::Type::kVec4},
        TokenData{"void", Token::Type::kVoid},
        TokenData{"workgroup", Token::Type::kWorkgroup}));

using KeywordTest_Reserved = testing::TestWithParam<const char*>;
TEST_P(KeywordTest_Reserved, Parses) {
  auto* keyword = GetParam();
  Source::FileContent content(keyword);
  Lexer l("test.wgsl", &content);

  auto t = l.next();
  EXPECT_TRUE(t.IsReservedKeyword());
  EXPECT_EQ(t.to_str(), keyword);
}
INSTANTIATE_TEST_SUITE_P(LexerTest,
                         KeywordTest_Reserved,
                         testing::Values("asm",
                                         "bf16",
                                         "do",
                                         "enum",
                                         "f16",
                                         "f64",
                                         "handle",
                                         "i8",
                                         "i16",
                                         "i64",
                                         "premerge",
                                         "typedef",
                                         "u8",
                                         "u16",
                                         "u64",
                                         "unless",
                                         "regardless"));

}  // namespace
}  // namespace wgsl
}  // namespace reader
}  // namespace tint
