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

#ifndef SRC_TINT_READER_WGSL_LEXER_H_
#define SRC_TINT_READER_WGSL_LEXER_H_

#include <optional>
#include <string>
#include <vector>

#include "src/tint/reader/wgsl/token.h"

namespace tint::reader::wgsl {

/// Converts the input stream into a series of Tokens
class Lexer {
  public:
    /// Creates a new Lexer
    /// @param file the source file
    explicit Lexer(const Source::File* file);
    ~Lexer();

    /// @return the token list.
    std::vector<Token> Lex();

  private:
    /// Returns the next token in the input stream.
    /// @return Token
    Token next();

    /// Advances past blankspace and comments, if present at the current position.
    /// @returns error token, EOF, or uninitialized
    std::optional<Token> skip_blankspace_and_comments();
    /// Advances past a comment at the current position, if one exists.
    /// Returns an error if there was an unterminated block comment,
    /// or a null character was present.
    /// @returns uninitialized token on success, or error
    std::optional<Token> skip_comment();

    Token build_token_from_int_if_possible(Source source,
                                           size_t start,
                                           size_t prefix_count,
                                           int32_t base);

    std::optional<Token> check_keyword(const Source&, std::string_view);

    /// The try_* methods have the following in common:
    /// - They assume there is at least one character to be consumed,
    ///   i.e. the input has not yet reached end of file.
    /// - They return an initialized token when they match and consume
    ///   a token of the specified kind.
    /// - Some can return an error token.
    /// - Otherwise they return an uninitialized token when they did not
    ///   match a token of the specfied kind.
    std::optional<Token> try_float();
    std::optional<Token> try_hex_float();
    std::optional<Token> try_hex_integer();
    std::optional<Token> try_ident();
    std::optional<Token> try_integer();
    std::optional<Token> try_punctuation();

    Source begin_source() const;
    void end_source(Source&) const;

    /// @returns view of current line
    const std::string_view line() const;
    /// @returns position in current line
    size_t pos() const;
    /// @returns length of current line
    size_t length() const;
    /// @returns reference to character at `pos` within current line
    const char& at(size_t pos) const;
    /// @returns substring view at `offset` within current line of length `count`
    std::string_view substr(size_t offset, size_t count);
    /// advances current position by `offset` within current line
    void advance(size_t offset = 1);
    /// sets current position to `pos` within current line
    void set_pos(size_t pos);
    /// advances current position to next line
    void advance_line();
    /// @returns true if the end of the input has been reached.
    bool is_eof() const;
    /// @returns true if the end of the current line has been reached.
    bool is_eol() const;
    /// @returns true if there is another character on the input and
    /// it is not null.
    bool is_null() const;
    /// @param ch a character
    /// @returns true if 'ch' is a decimal digit
    bool is_digit(char ch) const;
    /// @param ch a character
    /// @returns true if 'ch' is a hexadecimal digit
    bool is_hex(char ch) const;
    /// @returns true if string at `pos` matches `substr`
    bool matches(size_t pos, std::string_view substr);
    /// @returns true if char at `pos` matches `ch`
    bool matches(size_t pos, char ch);
    /// The source file content
    Source::File const* const file_;
    /// The current location within the input
    Source::Location location_;
};

}  // namespace tint::reader::wgsl

#endif  // SRC_TINT_READER_WGSL_LEXER_H_
