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

namespace tint {
namespace reader {
namespace wgsl {

/// Converts the input stream into a series of Tokens
class Lexer {
 public:
  /// Creates a new Lexer
  /// @param file_path the path to the file containing the source
  /// @param content the source content
  Lexer(const std::string& file_path, const Source::FileContent* content);
  ~Lexer();

  /// Returns the next token in the input stream.
  /// @return Token
  Token next();

 private:
  /// Advances past whitespace and comments, if present
  /// at the current position.
  /// @returns error token, EOF, or uninitialized
  Token skip_whitespace_and_comments();
  /// Advances past a comment at the current position,
  /// if one exists.
  /// @returns uninitialized token on success, or error
  Token skip_comment();

  Token build_token_from_int_if_possible(Source source,
                                         size_t start,
                                         size_t end,
                                         int32_t base);
  Token check_keyword(const Source&, const std::string&);

  /// The try_* methods have the following in common:
  /// - They assume there is at least one character to be consumed,
  ///   i.e. the input has not yet reached end of file.
  /// - They return an initialized token when they match and consume
  ///   a token of the specified kind.
  /// - Some can return an error token.
  /// - Otherwise they return an uninitialized token when they did not
  ///   match a token of the specfied kind.
  Token try_float();
  Token try_hex_float();
  Token try_hex_integer();
  Token try_ident();
  Token try_integer();
  Token try_punctuation();

  Source begin_source() const;
  void end_source(Source&) const;

  /// @returns true if the end of the input has been reached.
  bool is_eof() const;
  /// @param ch a character
  /// @returns true if 'ch' is an alphabetic character
  bool is_alpha(char ch) const;
  /// @param ch a character
  /// @returns true if 'ch' is a decimal digit
  bool is_digit(char ch) const;
  /// @param ch a character
  /// @returns true if 'ch' is a hexadecimal digit
  bool is_hex(char ch) const;
  /// @param ch a character
  /// @returns true if 'ch' is a digit, an alphabetic character,
  /// or an underscore.
  bool is_alphanum_underscore(char ch) const;
  bool matches(size_t pos, const std::string& substr);

  /// The source file path
  std::string const file_path_;
  /// The source file content
  Source::FileContent const* const content_;
  /// The length of the input
  uint32_t len_ = 0;
  /// The current position within the input
  uint32_t pos_ = 0;
  /// The current location within the input
  Source::Location location_;
};

}  // namespace wgsl
}  // namespace reader
}  // namespace tint

#endif  // SRC_READER_WGSL_LEXER_H_
