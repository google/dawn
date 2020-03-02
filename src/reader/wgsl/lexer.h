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

#ifndef SRC_READER_WGSL_LEXER_H_
#define SRC_READER_WGSL_LEXER_H_

#include <string>

#include "src/reader/wgsl/token.h"
#include "src/source.h"

namespace tint {
namespace reader {
namespace wgsl {

/// Converts the input stream into a series of Tokens
class Lexer {
 public:
  /// Creates a new Lexer
  /// @param input the input to parse
  explicit Lexer(const std::string& input);
  ~Lexer();

  /// Returns the next token in the input stream
  /// @return Token
  Token next();

 private:
  void skip_whitespace();
  void skip_comments();

  Token build_token_from_int_if_possible(const Source& source,
                                         size_t start,
                                         size_t end,
                                         int32_t base);
  Token check_keyword(const Source&, const std::string&);
  Token check_reserved(const Source&, const std::string&);
  Token try_float();
  Token try_hex_integer();
  Token try_ident();
  Token try_integer();
  Token try_punctuation();
  Token try_string();

  Source make_source() const;

  bool is_eof() const;
  bool is_alpha(char ch) const;
  bool is_digit(char ch) const;
  bool is_hex(char ch) const;
  bool is_alphanum(char ch) const;
  bool matches(size_t pos, const std::string& substr);

  /// The source to parse
  std::string input_;
  /// The length of the input
  uint32_t len_ = 0;
  /// The current position within the input
  uint32_t pos_ = 0;
  /// The current line within the input
  uint32_t line_ = 1;
  /// The current column within the input
  uint32_t column_ = 1;
};

}  // namespace wgsl
}  // namespace reader
}  // namespace tint

#endif  // SRC_READER_WGSL_LEXER_H_
