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

#ifndef SRC_TINT_WRITER_TEXT_GENERATOR_H_
#define SRC_TINT_WRITER_TEXT_GENERATOR_H_

#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "src/tint/diagnostic/diagnostic.h"
#include "src/tint/program_builder.h"
#include "src/tint/utils/string_stream.h"

namespace tint::writer {

/// Helper methods for generators which are creating text output
class TextGenerator {
  public:
    /// Line holds a single line of text
    struct Line {
        /// The indentation of the line in blankspace
        uint32_t indent = 0;
        /// The content of the line, without a trailing newline character
        std::string content;
    };

    /// TextBuffer holds a list of lines of text.
    struct TextBuffer {
        // Constructor
        TextBuffer();

        // Destructor
        ~TextBuffer();

        /// IncrementIndent increases the indentation of lines that will be written
        /// to the TextBuffer
        void IncrementIndent();

        /// DecrementIndent decreases the indentation of lines that will be written
        /// to the TextBuffer
        void DecrementIndent();

        /// Appends the line to the end of the TextBuffer
        /// @param line the line to append to the TextBuffer
        void Append(const std::string& line);

        /// Inserts the line to the TextBuffer before the line with index `before`
        /// @param line the line to append to the TextBuffer
        /// @param before the zero-based index of the line to insert the text before
        /// @param indent the indentation to apply to the inserted lines
        void Insert(const std::string& line, size_t before, uint32_t indent);

        /// Appends the lines of `tb` to the end of this TextBuffer
        /// @param tb the TextBuffer to append to the end of this TextBuffer
        void Append(const TextBuffer& tb);

        /// Inserts the lines of `tb` to the TextBuffer before the line with index
        /// `before`
        /// @param tb the TextBuffer to insert into this TextBuffer
        /// @param before the zero-based index of the line to insert the text before
        /// @param indent the indentation to apply to the inserted lines
        void Insert(const TextBuffer& tb, size_t before, uint32_t indent);

        /// @returns the buffer's content as a single string
        /// @param indent additional indentation to apply to each line
        std::string String(uint32_t indent = 0) const;

        /// The current indentation of the TextBuffer. Lines appended to the
        /// TextBuffer will use this indentation.
        uint32_t current_indent = 0;

        /// The lines
        std::vector<Line> lines;
    };

    /// Constructor
    /// @param program the program used by the generator
    explicit TextGenerator(const Program* program);
    ~TextGenerator();

    /// Increment the emitter indent level
    void increment_indent() { current_buffer_->IncrementIndent(); }
    /// Decrement the emitter indent level
    void decrement_indent() { current_buffer_->DecrementIndent(); }

    /// @returns the result data
    std::string result() const { return main_buffer_.String(); }

    /// @returns the list of diagnostics raised by the generator.
    const diag::List& Diagnostics() const { return diagnostics_; }

    /// @return a new, unique identifier with the given prefix.
    /// @param prefix optional prefix to apply to the generated identifier. If
    /// empty "tint_symbol" will be used.
    std::string UniqueIdentifier(const std::string& prefix = "");

    /// @param s the semantic structure
    /// @returns the name of the structure, taking special care of builtin
    /// structures that start with double underscores. If the structure is a
    /// builtin, then the returned name will be a unique name without the leading
    /// underscores.
    std::string StructName(const type::Struct* s);

  protected:
    /// LineWriter is a helper that acts as a string buffer, who's content is
    /// emitted to the TextBuffer as a single line on destruction.
    struct LineWriter {
      public:
        /// Constructor
        /// @param buffer the TextBuffer that the LineWriter will append its
        /// content to on destruction, at the end of the buffer.
        explicit LineWriter(TextBuffer* buffer);

        /// Move constructor
        /// @param rhs the LineWriter to move
        LineWriter(LineWriter&& rhs);
        /// Destructor
        ~LineWriter();

        /// @returns the utils::StringStream
        operator utils::StringStream&() { return os; }

        /// @param rhs the value to write to the line
        /// @returns the utils::StringStream so calls can be chained
        template <typename T>
        utils::StringStream& operator<<(T&& rhs) {
            return os << std::forward<T>(rhs);
        }

      private:
        LineWriter(const LineWriter&) = delete;
        LineWriter& operator=(const LineWriter&) = delete;

        utils::StringStream os;
        TextBuffer* buffer;
    };

    /// Helper for writing a '(' on construction and a ')' destruction.
    struct ScopedParen {
        /// Constructor
        /// @param stream the utils::StringStream that will be written to
        explicit ScopedParen(utils::StringStream& stream);
        /// Destructor
        ~ScopedParen();

      private:
        ScopedParen(ScopedParen&& rhs) = delete;
        ScopedParen(const ScopedParen&) = delete;
        ScopedParen& operator=(const ScopedParen&) = delete;
        utils::StringStream& s;
    };

    /// Helper for incrementing indentation on construction and decrementing
    /// indentation on destruction.
    struct ScopedIndent {
        /// Constructor
        /// @param buffer the TextBuffer that the ScopedIndent will indent
        explicit ScopedIndent(TextBuffer* buffer);
        /// Constructor
        /// @param generator ScopedIndent will indent the generator's
        /// `current_buffer_`
        explicit ScopedIndent(TextGenerator* generator);
        /// Destructor
        ~ScopedIndent();

      private:
        ScopedIndent(ScopedIndent&& rhs) = delete;
        ScopedIndent(const ScopedIndent&) = delete;
        ScopedIndent& operator=(const ScopedIndent&) = delete;
        TextBuffer* buffer_;
    };

    /// @returns the resolved type of the ast::Expression `expr`
    /// @param expr the expression
    const type::Type* TypeOf(const ast::Expression* expr) const { return builder_.TypeOf(expr); }

    /// @returns the resolved type of the ast::TypeDecl `type_decl`
    /// @param type_decl the type
    const type::Type* TypeOf(const ast::TypeDecl* type_decl) const {
        return builder_.TypeOf(type_decl);
    }

    /// @returns a new LineWriter, used for buffering and writing a line to
    /// the end of #current_buffer_.
    LineWriter line() { return LineWriter(current_buffer_); }

    /// @param buffer the TextBuffer to write the line to
    /// @returns a new LineWriter, used for buffering and writing a line to
    /// the end of `buffer`.
    static LineWriter line(TextBuffer* buffer) { return LineWriter(buffer); }

    /// The program
    Program const* const program_;
    /// A ProgramBuilder that thinly wraps program_
    ProgramBuilder builder_;
    /// Diagnostics generated by the generator
    diag::List diagnostics_;
    /// The buffer the TextGenerator is currently appending lines to
    TextBuffer* current_buffer_ = &main_buffer_;

  private:
    /// The primary text buffer that the generator will emit
    TextBuffer main_buffer_;
    /// Map of builtin structure to unique generated name
    std::unordered_map<const type::Struct*, std::string> builtin_struct_names_;
};

}  // namespace tint::writer

#endif  // SRC_TINT_WRITER_TEXT_GENERATOR_H_
