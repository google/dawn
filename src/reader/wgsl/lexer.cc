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

#include <ctype.h>
#include <errno.h>
#include <stdlib.h>

#include <cctype>
#include <limits>

namespace tint {
namespace reader {
namespace wgsl {
namespace {

bool is_whitespace(char c) {
  return std::isspace(c);
}

}  // namespace

Lexer::Lexer(const std::string& input)
    : input_(input), len_(static_cast<uint32_t>(input.size())) {}

Lexer::~Lexer() = default;

Token Lexer::next() {
  skip_whitespace();
  skip_comments();

  if (is_eof()) {
    return {Token::Type::kEOF, make_source()};
  }

  auto t = try_hex_integer();
  if (!t.IsUninitialized()) {
    return t;
  }

  t = try_float();
  if (!t.IsUninitialized()) {
    return t;
  }

  t = try_integer();
  if (!t.IsUninitialized()) {
    return t;
  }

  t = try_string();
  if (!t.IsUninitialized()) {
    return t;
  }

  t = try_punctuation();
  if (!t.IsUninitialized()) {
    return t;
  }

  t = try_ident();
  if (!t.IsUninitialized()) {
    return t;
  }

  return {Token::Type::kError, make_source(), "invalid character found"};
}

Source Lexer::make_source() const {
  return Source{line_, column_};
}

bool Lexer::is_eof() const {
  return pos_ >= len_;
}

bool Lexer::is_alpha(char ch) const {
  return std::isalpha(ch) || ch == '_';
}

bool Lexer::is_digit(char ch) const {
  return std::isdigit(ch);
}

bool Lexer::is_alphanum(char ch) const {
  return is_alpha(ch) || is_digit(ch);
}

bool Lexer::is_hex(char ch) const {
  return std::isxdigit(ch);
}

bool Lexer::matches(size_t pos, const std::string& substr) {
  if (pos >= input_.size())
    return false;
  return input_.substr(pos, substr.size()) == substr;
}

void Lexer::skip_whitespace() {
  for (;;) {
    auto pos = pos_;
    while (!is_eof() && is_whitespace(input_[pos_])) {
      if (matches(pos_, "\n")) {
        pos_++;
        line_++;
        column_ = 1;
        continue;
      }

      pos_++;
      column_++;
    }

    skip_comments();

    // If the cursor didn't advance we didn't remove any whitespace
    // so we're done.
    if (pos == pos_)
      break;
  }
}

void Lexer::skip_comments() {
  if (!matches(pos_, "#")) {
    return;
  }

  while (!is_eof() && !matches(pos_, "\n")) {
    pos_++;
    column_++;
  }
}

Token Lexer::try_float() {
  auto start = pos_;
  auto end = pos_;

  auto source = make_source();

  if (matches(end, "-")) {
    end++;
  }
  while (end < len_ && is_digit(input_[end])) {
    end++;
  }

  if (end >= len_ || !matches(end, ".")) {
    return {};
  }
  end++;

  while (end < len_ && is_digit(input_[end])) {
    end++;
  }

  // Parse the exponent if one exists
  if (end < len_ && matches(end, "e")) {
    end++;
    if (end < len_ && (matches(end, "+") || matches(end, "-"))) {
      end++;
    }

    auto exp_start = end;
    while (end < len_ && isdigit(input_[end])) {
      end++;
    }

    // Must have an exponent
    if (exp_start == end)
      return {};
  }

  auto str = input_.substr(start, end - start);
  if (str == "." || str == "-.")
    return {};

  pos_ = end;
  column_ += (end - start);

  auto res = strtod(input_.c_str() + start, nullptr);
  // This handles if the number is a really small in the exponent
  if (res > 0 && res < static_cast<double>(std::numeric_limits<float>::min())) {
    return {Token::Type::kError, source, "f32 (" + str + " too small"};
  }
  // This handles if the number is really large negative number
  if (res < static_cast<double>(std::numeric_limits<float>::lowest())) {
    return {Token::Type::kError, source, "f32 (" + str + ") too small"};
  }
  if (res > static_cast<double>(std::numeric_limits<float>::max())) {
    return {Token::Type::kError, source, "f32 (" + str + ") too large"};
  }

  return {source, static_cast<float>(res)};
}

Token Lexer::build_token_from_int_if_possible(const Source& source,
                                              size_t start,
                                              size_t end,
                                              int32_t base) {
  auto res = strtoll(input_.c_str() + start, nullptr, base);
  if (matches(pos_, "u")) {
    if (static_cast<uint64_t>(res) >
        static_cast<uint64_t>(std::numeric_limits<uint32_t>::max())) {
      return {Token::Type::kError, source,
              "u32 (" + input_.substr(start, end - start) + ") too large"};
    }
    pos_ += 1;
    return {source, static_cast<uint32_t>(res)};
  }

  if (res < static_cast<int64_t>(std::numeric_limits<int32_t>::min())) {
    return {Token::Type::kError, source,
            "i32 (" + input_.substr(start, end - start) + ") too small"};
  }
  if (res > static_cast<int64_t>(std::numeric_limits<int32_t>::max())) {
    return {Token::Type::kError, source,
            "i32 (" + input_.substr(start, end - start) + ") too large"};
  }
  return {source, static_cast<int32_t>(res)};
}

Token Lexer::try_hex_integer() {
  auto start = pos_;
  auto end = pos_;

  auto source = make_source();

  if (matches(end, "-")) {
    end++;
  }
  if (!matches(end, "0x")) {
    return Token();
  }
  end += 2;

  while (!is_eof() && is_hex(input_[end])) {
    end += 1;
  }

  pos_ = end;
  column_ += (end - start);

  return build_token_from_int_if_possible(source, start, end, 16);
}

Token Lexer::try_integer() {
  auto start = pos_;
  auto end = start;

  auto source = make_source();

  if (matches(end, "-")) {
    end++;
  }
  if (end >= len_ || !is_digit(input_[end])) {
    return {};
  }

  auto first = end;
  while (end < len_ && is_digit(input_[end])) {
    end++;
  }

  // If the first digit is a zero this must only be zero as leading zeros
  // are not allowed.
  if (input_[first] == '0' && (end - first != 1))
    return {};

  pos_ = end;
  column_ += (end - start);

  return build_token_from_int_if_possible(source, start, end, 10);
}

Token Lexer::try_ident() {
  // Must begin with an a-zA-Z_
  if (!is_alpha(input_[pos_])) {
    return {};
  }

  auto source = make_source();

  auto s = pos_;
  while (!is_eof() && is_alphanum(input_[pos_])) {
    pos_++;
    column_++;
  }

  auto str = input_.substr(s, pos_ - s);
  auto t = check_reserved(source, str);
  if (!t.IsUninitialized()) {
    return t;
  }

  t = check_keyword(source, str);
  if (!t.IsUninitialized()) {
    return t;
  }

  return {Token::Type::kIdentifier, source, str};
}

Token Lexer::try_string() {
  if (!matches(pos_, R"(")"))
    return {};

  auto source = make_source();

  pos_++;
  auto start = pos_;
  while (pos_ < len_ && !matches(pos_, R"(")")) {
    pos_++;
  }
  auto end = pos_;
  pos_++;
  column_ += (pos_ - start) + 1;

  return {Token::Type::kStringLiteral, source,
          input_.substr(start, end - start)};
}

Token Lexer::try_punctuation() {
  auto source = make_source();
  auto type = Token::Type::kUninitialized;

  if (matches(pos_, "[[")) {
    type = Token::Type::kAttrLeft;
    pos_ += 2;
    column_ += 2;
  } else if (matches(pos_, "]]")) {
    type = Token::Type::kAttrRight;
    pos_ += 2;
    column_ += 2;
  } else if (matches(pos_, "(")) {
    type = Token::Type::kParenLeft;
    pos_ += 1;
    column_ += 1;
  } else if (matches(pos_, ")")) {
    type = Token::Type::kParenRight;
    pos_ += 1;
    column_ += 1;
  } else if (matches(pos_, "[")) {
    type = Token::Type::kBracketLeft;
    pos_ += 1;
    column_ += 1;
  } else if (matches(pos_, "]")) {
    type = Token::Type::kBracketRight;
    pos_ += 1;
    column_ += 1;
  } else if (matches(pos_, "{")) {
    type = Token::Type::kBraceLeft;
    pos_ += 1;
    column_ += 1;
  } else if (matches(pos_, "}")) {
    type = Token::Type::kBraceRight;
    pos_ += 1;
    column_ += 1;
  } else if (matches(pos_, "&&")) {
    type = Token::Type::kAndAnd;
    pos_ += 2;
    column_ += 2;
  } else if (matches(pos_, "&")) {
    type = Token::Type::kAnd;
    pos_ += 1;
    column_ += 1;
  } else if (matches(pos_, "/")) {
    type = Token::Type::kForwardSlash;
    pos_ += 1;
    column_ += 1;
  } else if (matches(pos_, "!=")) {
    type = Token::Type::kNotEqual;
    pos_ += 2;
    column_ += 2;
  } else if (matches(pos_, "!")) {
    type = Token::Type::kBang;
    pos_ += 1;
    column_ += 1;
  } else if (matches(pos_, "::")) {
    type = Token::Type::kNamespace;
    pos_ += 2;
    column_ += 2;
  } else if (matches(pos_, ":")) {
    type = Token::Type::kColon;
    pos_ += 1;
    column_ += 1;
  } else if (matches(pos_, ",")) {
    type = Token::Type::kComma;
    pos_ += 1;
    column_ += 1;
  } else if (matches(pos_, "==")) {
    type = Token::Type::kEqualEqual;
    pos_ += 2;
    column_ += 2;
  } else if (matches(pos_, "=")) {
    type = Token::Type::kEqual;
    pos_ += 1;
    column_ += 1;
  } else if (matches(pos_, ">=")) {
    type = Token::Type::kGreaterThanEqual;
    pos_ += 2;
    column_ += 2;
  } else if (matches(pos_, ">")) {
    type = Token::Type::kGreaterThan;
    pos_ += 1;
    column_ += 1;
  } else if (matches(pos_, "<=")) {
    type = Token::Type::kLessThanEqual;
    pos_ += 2;
    column_ += 2;
  } else if (matches(pos_, "<")) {
    type = Token::Type::kLessThan;
    pos_ += 1;
    column_ += 1;
  } else if (matches(pos_, "%")) {
    type = Token::Type::kMod;
    pos_ += 1;
    column_ += 1;
  } else if (matches(pos_, "->")) {
    type = Token::Type::kArrow;
    pos_ += 2;
    column_ += 2;
  } else if (matches(pos_, "-")) {
    type = Token::Type::kMinus;
    pos_ += 1;
    column_ += 1;
  } else if (matches(pos_, ".")) {
    type = Token::Type::kPeriod;
    pos_ += 1;
    column_ += 1;
  } else if (matches(pos_, "+")) {
    type = Token::Type::kPlus;
    pos_ += 1;
    column_ += 1;
  } else if (matches(pos_, "||")) {
    type = Token::Type::kOrOr;
    pos_ += 2;
    column_ += 2;
  } else if (matches(pos_, "|")) {
    type = Token::Type::kOr;
    pos_ += 1;
    column_ += 1;
  } else if (matches(pos_, ";")) {
    type = Token::Type::kSemicolon;
    pos_ += 1;
    column_ += 1;
  } else if (matches(pos_, "*")) {
    type = Token::Type::kStar;
    pos_ += 1;
    column_ += 1;
  } else if (matches(pos_, "^")) {
    type = Token::Type::kXor;
    pos_ += 1;
    column_ += 1;
  }

  return {type, source};
}

Token Lexer::check_keyword(const Source& source, const std::string& str) {
  if (str == "array")
    return {Token::Type::kArray, source, "array"};
  if (str == "as")
    return {Token::Type::kAs, source, "as"};
  if (str == "binding")
    return {Token::Type::kBinding, source, "binding"};
  if (str == "block")
    return {Token::Type::kBlock, source, "block"};
  if (str == "bool")
    return {Token::Type::kBool, source, "bool"};
  if (str == "break")
    return {Token::Type::kBreak, source, "break"};
  if (str == "builtin")
    return {Token::Type::kBuiltin, source, "builtin"};
  if (str == "case")
    return {Token::Type::kCase, source, "case"};
  if (str == "cast")
    return {Token::Type::kCast, source, "cast"};
  if (str == "compute")
    return {Token::Type::kCompute, source, "compute"};
  if (str == "const")
    return {Token::Type::kConst, source, "const"};
  if (str == "continue")
    return {Token::Type::kContinue, source, "continue"};
  if (str == "continuing")
    return {Token::Type::kContinuing, source, "continuing"};
  if (str == "discard")
    return {Token::Type::kDiscard, source, "discard"};
  if (str == "default")
    return {Token::Type::kDefault, source, "default"};
  if (str == "else")
    return {Token::Type::kElse, source, "else"};
  if (str == "elseif")
    return {Token::Type::kElseIf, source, "elseif"};
  if (str == "entry_point")
    return {Token::Type::kEntryPoint, source, "entry_point"};
  if (str == "f32")
    return {Token::Type::kF32, source, "f32"};
  if (str == "fallthrough")
    return {Token::Type::kFallthrough, source, "fallthrough"};
  if (str == "false")
    return {Token::Type::kFalse, source, "false"};
  if (str == "fn")
    return {Token::Type::kFn, source, "fn"};
  if (str == "for")
    return {Token::Type::kFor, source, "for"};
  if (str == "fragment")
    return {Token::Type::kFragment, source, "fragment"};
  if (str == "function")
    return {Token::Type::kFunction, source, "function"};
  if (str == "i32")
    return {Token::Type::kI32, source, "i32"};
  if (str == "if")
    return {Token::Type::kIf, source, "if"};
  if (str == "image")
    return {Token::Type::kImage, source, "image"};
  if (str == "import")
    return {Token::Type::kImport, source, "import"};
  if (str == "in")
    return {Token::Type::kIn, source, "in"};
  if (str == "location")
    return {Token::Type::kLocation, source, "location"};
  if (str == "loop")
    return {Token::Type::kLoop, source, "loop"};
  if (str == "mat2x2")
    return {Token::Type::kMat2x2, source, "mat2x2"};
  if (str == "mat2x3")
    return {Token::Type::kMat2x3, source, "mat2x3"};
  if (str == "mat2x4")
    return {Token::Type::kMat2x4, source, "mat2x4"};
  if (str == "mat3x2")
    return {Token::Type::kMat3x2, source, "mat3x2"};
  if (str == "mat3x3")
    return {Token::Type::kMat3x3, source, "mat3x3"};
  if (str == "mat3x4")
    return {Token::Type::kMat3x4, source, "mat3x4"};
  if (str == "mat4x2")
    return {Token::Type::kMat4x2, source, "mat4x2"};
  if (str == "mat4x3")
    return {Token::Type::kMat4x3, source, "mat4x3"};
  if (str == "mat4x4")
    return {Token::Type::kMat4x4, source, "mat4x4"};
  if (str == "offset")
    return {Token::Type::kOffset, source, "offset"};
  if (str == "out")
    return {Token::Type::kOut, source, "out"};
  if (str == "private")
    return {Token::Type::kPrivate, source, "private"};
  if (str == "ptr")
    return {Token::Type::kPtr, source, "ptr"};
  if (str == "return")
    return {Token::Type::kReturn, source, "return"};
  if (str == "set")
    return {Token::Type::kSet, source, "set"};
  if (str == "storage_buffer")
    return {Token::Type::kStorageBuffer, source, "storage_buffer"};
  if (str == "stride")
    return {Token::Type::kStride, source, "stride"};
  if (str == "struct")
    return {Token::Type::kStruct, source, "struct"};
  if (str == "switch")
    return {Token::Type::kSwitch, source, "switch"};
  if (str == "true")
    return {Token::Type::kTrue, source, "true"};
  if (str == "type")
    return {Token::Type::kType, source, "type"};
  if (str == "u32")
    return {Token::Type::kU32, source, "u32"};
  if (str == "uniform")
    return {Token::Type::kUniform, source, "uniform"};
  if (str == "uniform_constant")
    return {Token::Type::kUniformConstant, source, "uniform_constant"};
  if (str == "var")
    return {Token::Type::kVar, source, "var"};
  if (str == "vec2")
    return {Token::Type::kVec2, source, "vec2"};
  if (str == "vec3")
    return {Token::Type::kVec3, source, "vec3"};
  if (str == "vec4")
    return {Token::Type::kVec4, source, "vec4"};
  if (str == "vertex")
    return {Token::Type::kVertex, source, "vertex"};
  if (str == "void")
    return {Token::Type::kVoid, source, "void"};
  if (str == "workgroup")
    return {Token::Type::kWorkgroup, source, "workgroup"};

  return {};
}

Token Lexer::check_reserved(const Source& source, const std::string& str) {
  if (str == "asm")
    return {Token::Type::kReservedKeyword, source, "asm"};
  if (str == "bf16")
    return {Token::Type::kReservedKeyword, source, "bf16"};
  if (str == "do")
    return {Token::Type::kReservedKeyword, source, "do"};
  if (str == "enum")
    return {Token::Type::kReservedKeyword, source, "enum"};
  if (str == "f16")
    return {Token::Type::kReservedKeyword, source, "f16"};
  if (str == "f64")
    return {Token::Type::kReservedKeyword, source, "f64"};
  if (str == "i8")
    return {Token::Type::kReservedKeyword, source, "i8"};
  if (str == "i16")
    return {Token::Type::kReservedKeyword, source, "i16"};
  if (str == "i64")
    return {Token::Type::kReservedKeyword, source, "i64"};
  if (str == "let")
    return {Token::Type::kReservedKeyword, source, "let"};
  if (str == "premerge")
    return {Token::Type::kReservedKeyword, source, "premerge"};
  if (str == "regardless")
    return {Token::Type::kReservedKeyword, source, "regardless"};
  if (str == "typedef")
    return {Token::Type::kReservedKeyword, source, "typedef"};
  if (str == "u8")
    return {Token::Type::kReservedKeyword, source, "u8"};
  if (str == "u16")
    return {Token::Type::kReservedKeyword, source, "u16"};
  if (str == "u64")
    return {Token::Type::kReservedKeyword, source, "u64"};
  if (str == "unless")
    return {Token::Type::kReservedKeyword, source, "unless"};
  return {};
}

}  // namespace wgsl
}  // namespace reader
}  // namespace tint
