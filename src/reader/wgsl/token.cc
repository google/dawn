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

#include "src/reader/wgsl/token.h"

namespace tint {
namespace reader {
namespace wgsl {

// static
std::string Token::TypeToName(Type type) {
  switch (type) {
    case Token::Type::kError:
      return "kError";
    case Token::Type::kReservedKeyword:
      return "kReservedKeyword";
    case Token::Type::kEOF:
      return "kEOF";
    case Token::Type::kIdentifier:
      return "kIdentifier";
    case Token::Type::kFloatLiteral:
      return "kFloatLiteral";
    case Token::Type::kSintLiteral:
      return "kSintLiteral";
    case Token::Type::kUintLiteral:
      return "kUintLiteral";
    case Token::Type::kUninitialized:
      return "kUninitialized";

    case Token::Type::kAnd:
      return "&";
    case Token::Type::kAndAnd:
      return "&&";
    case Token::Type::kArrow:
      return "->";
    case Token::Type::kAttrLeft:
      return "[[";
    case Token::Type::kAttrRight:
      return "]]";
    case Token::Type::kForwardSlash:
      return "/";
    case Token::Type::kBang:
      return "!";
    case Token::Type::kBracketLeft:
      return "[";
    case Token::Type::kBracketRight:
      return "]";
    case Token::Type::kBraceLeft:
      return "{";
    case Token::Type::kBraceRight:
      return "}";
    case Token::Type::kColon:
      return ":";
    case Token::Type::kComma:
      return ",";
    case Token::Type::kEqual:
      return "=";
    case Token::Type::kEqualEqual:
      return "==";
    case Token::Type::kGreaterThan:
      return ">";
    case Token::Type::kGreaterThanEqual:
      return ">=";
    case Token::Type::kShiftRight:
      return ">>";
    case Token::Type::kLessThan:
      return "<";
    case Token::Type::kLessThanEqual:
      return "<=";
    case Token::Type::kShiftLeft:
      return "<<";
    case Token::Type::kMod:
      return "%";
    case Token::Type::kNotEqual:
      return "!=";
    case Token::Type::kMinus:
      return "-";
    case Token::Type::kPeriod:
      return ".";
    case Token::Type::kPlus:
      return "+";
    case Token::Type::kOr:
      return "|";
    case Token::Type::kOrOr:
      return "||";
    case Token::Type::kParenLeft:
      return "(";
    case Token::Type::kParenRight:
      return ")";
    case Token::Type::kSemicolon:
      return ";";
    case Token::Type::kStar:
      return "*";
    case Token::Type::kXor:
      return "^";

    case Token::Type::kArray:
      return "array";
    case Token::Type::kBitcast:
      return "bitcast";
    case Token::Type::kBool:
      return "bool";
    case Token::Type::kBreak:
      return "break";
    case Token::Type::kCase:
      return "case";
    case Token::Type::kConst:
      return "const";
    case Token::Type::kContinue:
      return "continue";
    case Token::Type::kContinuing:
      return "continuing";
    case Token::Type::kDiscard:
      return "discard";
    case Token::Type::kDefault:
      return "default";
    case Token::Type::kElse:
      return "else";
    case Token::Type::kElseIf:
      return "elseif";
    case Token::Type::kF32:
      return "f32";
    case Token::Type::kFallthrough:
      return "fallthrough";
    case Token::Type::kFalse:
      return "false";
    case Token::Type::kFn:
      return "fn";
    case Token::Type::kFor:
      return "for";
    case Token::Type::kFormatBgra8Unorm:
      return "bgra8unorm";
    case Token::Type::kFormatBgra8UnormSrgb:
      return "bgra8unorm_srgb";
    case Token::Type::kFormatR16Float:
      return "r16float";
    case Token::Type::kFormatR16Sint:
      return "r16sint";
    case Token::Type::kFormatR16Uint:
      return "r16uint";
    case Token::Type::kFormatR32Float:
      return "r32float";
    case Token::Type::kFormatR32Sint:
      return "r32sint";
    case Token::Type::kFormatR32Uint:
      return "r32uint";
    case Token::Type::kFormatR8Sint:
      return "r8sint";
    case Token::Type::kFormatR8Snorm:
      return "r8snorm";
    case Token::Type::kFormatR8Uint:
      return "r8uint";
    case Token::Type::kFormatR8Unorm:
      return "r8unorm";
    case Token::Type::kFormatRg11B10Float:
      return "rg11b10float";
    case Token::Type::kFormatRg16Float:
      return "rg16float";
    case Token::Type::kFormatRg16Sint:
      return "rg16sint";
    case Token::Type::kFormatRg16Uint:
      return "rg16uint";
    case Token::Type::kFormatRg32Float:
      return "rg32float";
    case Token::Type::kFormatRg32Sint:
      return "rg32sint";
    case Token::Type::kFormatRg32Uint:
      return "rg32uint";
    case Token::Type::kFormatRg8Sint:
      return "rg8sint";
    case Token::Type::kFormatRg8Snorm:
      return "rg8snorm";
    case Token::Type::kFormatRg8Uint:
      return "rg8uint";
    case Token::Type::kFormatRg8Unorm:
      return "rg8unorm";
    case Token::Type::kFormatRgb10A2Unorm:
      return "rgb10a2unorm";
    case Token::Type::kFormatRgba16Float:
      return "rgba16float";
    case Token::Type::kFormatRgba16Sint:
      return "rgba16sint";
    case Token::Type::kFormatRgba16Uint:
      return "rgba16uint";
    case Token::Type::kFormatRgba32Float:
      return "rgba32float";
    case Token::Type::kFormatRgba32Sint:
      return "rgba32sint";
    case Token::Type::kFormatRgba32Uint:
      return "rgba32uint";
    case Token::Type::kFormatRgba8Sint:
      return "rgba8sint";
    case Token::Type::kFormatRgba8Snorm:
      return "rgba8snorm";
    case Token::Type::kFormatRgba8Uint:
      return "rgba8uint";
    case Token::Type::kFormatRgba8Unorm:
      return "rgba8unorm";
    case Token::Type::kFormatRgba8UnormSrgb:
      return "rgba8unorm_srgb";
    case Token::Type::kFunction:
      return "function";
    case Token::Type::kI32:
      return "i32";
    case Token::Type::kIf:
      return "if";
    case Token::Type::kImage:
      return "image";
    case Token::Type::kImport:
      return "import";
    case Token::Type::kIn:
      return "in";
    case Token::Type::kLet:
      return "let";
    case Token::Type::kLoop:
      return "loop";
    case Token::Type::kMat2x2:
      return "mat2x2";
    case Token::Type::kMat2x3:
      return "mat2x3";
    case Token::Type::kMat2x4:
      return "mat2x4";
    case Token::Type::kMat3x2:
      return "mat3x2";
    case Token::Type::kMat3x3:
      return "mat3x3";
    case Token::Type::kMat3x4:
      return "mat3x4";
    case Token::Type::kMat4x2:
      return "mat4x2";
    case Token::Type::kMat4x3:
      return "mat4x3";
    case Token::Type::kMat4x4:
      return "mat4x4";
    case Token::Type::kOut:
      return "out";
    case Token::Type::kPrivate:
      return "private";
    case Token::Type::kPtr:
      return "ptr";
    case Token::Type::kReturn:
      return "return";
    case Token::Type::kSampler:
      return "sampler";
    case Token::Type::kComparisonSampler:
      return "sampler_comparison";
    case Token::Type::kStorage:
      return "storage";
    case Token::Type::kStruct:
      return "struct";
    case Token::Type::kSwitch:
      return "switch";
    case Token::Type::kTextureDepth2d:
      return "texture_depth_2d";
    case Token::Type::kTextureDepth2dArray:
      return "texture_depth_2d_array";
    case Token::Type::kTextureDepthCube:
      return "texture_depth_cube";
    case Token::Type::kTextureDepthCubeArray:
      return "texture_depth_cube_array";
    case Token::Type::kTextureExternal:
      return "texture_external";
    case Token::Type::kTextureMultisampled2d:
      return "texture_multisampled_2d";
    case Token::Type::kTextureSampled1d:
      return "texture_1d";
    case Token::Type::kTextureSampled2d:
      return "texture_2d";
    case Token::Type::kTextureSampled2dArray:
      return "texture_2d_array";
    case Token::Type::kTextureSampled3d:
      return "texture_3d";
    case Token::Type::kTextureSampledCube:
      return "texture_cube";
    case Token::Type::kTextureSampledCubeArray:
      return "texture_cube_array";
    case Token::Type::kTextureStorage1d:
      return "texture_storage_1d";
    case Token::Type::kTextureStorage2d:
      return "texture_storage_2d";
    case Token::Type::kTextureStorage2dArray:
      return "texture_storage_2d_array";
    case Token::Type::kTextureStorage3d:
      return "texture_storage_3d";
    case Token::Type::kTrue:
      return "true";
    case Token::Type::kType:
      return "type";
    case Token::Type::kU32:
      return "u32";
    case Token::Type::kUniform:
      return "uniform";
    case Token::Type::kVar:
      return "var";
    case Token::Type::kVec2:
      return "vec2";
    case Token::Type::kVec3:
      return "vec3";
    case Token::Type::kVec4:
      return "vec4";
    case Token::Type::kVoid:
      return "void";
    case Token::Type::kWorkgroup:
      return "workgroup";
  }

  return "<unknown>";
}

Token::Token() : type_(Type::kUninitialized) {}

Token::Token(Type type, const Source& source, const std::string& val)
    : type_(type), source_(source), val_str_(val) {}

Token::Token(const Source& source, uint32_t val)
    : type_(Type::kUintLiteral), source_(source), val_uint_(val) {}

Token::Token(const Source& source, int32_t val)
    : type_(Type::kSintLiteral), source_(source), val_int_(val) {}

Token::Token(const Source& source, float val)
    : type_(Type::kFloatLiteral), source_(source), val_float_(val) {}

Token::Token(Type type, const Source& source) : Token(type, source, "") {}

Token::Token(Token&&) = default;

Token::Token(const Token&) = default;

Token::~Token() = default;

Token& Token::operator=(const Token&) = default;

std::string Token::to_str() const {
  if (type_ == Type::kFloatLiteral) {
    return std::to_string(val_float_);
  }
  if (type_ == Type::kSintLiteral) {
    return std::to_string(val_int_);
  }
  if (type_ == Type::kUintLiteral) {
    return std::to_string(val_uint_);
  }
  return val_str_;
}

float Token::to_f32() const {
  return val_float_;
}

uint32_t Token::to_u32() const {
  return val_uint_;
}

int32_t Token::to_i32() const {
  return val_int_;
}

}  // namespace wgsl
}  // namespace reader
}  // namespace tint
