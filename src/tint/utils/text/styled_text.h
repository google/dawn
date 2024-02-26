// Copyright 2024 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef SRC_TINT_UTILS_TEXT_STYLED_TEXT_H_
#define SRC_TINT_UTILS_TEXT_STYLED_TEXT_H_

#include <string>
#include <string_view>
#include <utility>

#include "src/tint/utils/containers/vector.h"
#include "src/tint/utils/text/string_stream.h"
#include "src/tint/utils/text/text_style.h"

// Forward declarations
namespace tint {
class StyledTextPrinter;
}

namespace tint {

/// StyledText is a string builder with support for styled text spans.
class StyledText {
  public:
    /// Constructor - empty string
    StyledText();

    /// Copy constructor
    StyledText(const StyledText&);

    /// Constructor from unstyled text
    explicit StyledText(const std::string&);

    /// Constructor from unstyled text
    explicit StyledText(std::string_view);

    /// Move constructor
    StyledText(StyledText&&);

    /// Assignment copy operator
    StyledText& operator=(const StyledText& other);

    /// Assignment operator from unstyled text
    StyledText& operator=(std::string_view text);

    /// Clears the text and restore the default style
    void Clear();

    /// Sets the style for all future writes to this StyledText
    StyledText& SetStyle(TextStyle style);

    /// @returns the unstyled text.
    std::string Plain() const;

    /// Appends the styled text of @p other to this StyledText.
    void Append(const StyledText& other);

    /// repeat queues the character @p c to be written to the StyledText n times.
    /// @param c the character to print @p n times
    /// @param n the number of times to print character @p c
    /// @returns this StyledText so calls can be chained.
    StyledText& Repeat(char c, size_t n);

    /// operator<<() appends @p value to the StyledText.
    /// @p value can be a StyledText to change the style of future appends.
    template <typename VALUE>
    StyledText& operator<<(VALUE&& value) {
        using T = std::decay_t<VALUE>;
        if constexpr (std::is_same_v<T, TextStyle>) {
            SetStyle(std::forward<VALUE>(value));
        } else if constexpr (std::is_same_v<T, StyledText>) {
            Append(value);
        } else {
            uint32_t offset = stream_.tellp();
            stream_ << value;
            spans_.Back().length += stream_.tellp() - offset;
        }
        return *this;
    }

    /// Walk calls @p callback with each styled span in the StyledText.
    /// @param callback a function with the signature: void(std::string_view, TextStyle)
    template <typename CB>
    void Walk(CB&& callback) const {
        std::string text = stream_.str();
        size_t offset = 0;
        for (auto& span : spans_) {
            callback(text.substr(offset, span.length), span.style);
            offset += span.length;
        }
    }

  private:
    struct Span {
        TextStyle style;
        size_t length = 0;
    };

    StringStream stream_;
    Vector<Span, 1> spans_{Span{}};
};

}  // namespace tint

#endif  // SRC_TINT_UTILS_TEXT_STYLED_TEXT_H_
