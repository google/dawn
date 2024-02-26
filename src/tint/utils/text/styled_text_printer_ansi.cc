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

#include <cstring>

#include "src/tint/utils/text/styled_text.h"
#include "src/tint/utils/text/styled_text_printer.h"
#include "src/tint/utils/text/styled_text_theme.h"
#include "src/tint/utils/text/text_style.h"

namespace tint {
namespace {

template <typename T>
bool Equal(const std::optional<T>& lhs, const std::optional<T>& rhs) {
    if (lhs.has_value() != rhs.has_value()) {
        return false;
    }
    if (!lhs.has_value()) {
        return true;
    }
    return lhs.value() == rhs.value();
}

#define ESCAPE "\u001b"

class Printer : public StyledTextPrinter {
  public:
    Printer(FILE* f, const StyledTextTheme& t) : file_(f), theme_(t) {}

    void Print(const StyledText& style_text) override {
        StyledTextTheme::Attributes current;

        style_text.Walk([&](std::string_view text, TextStyle text_style) {
            auto style = theme_.Get(text_style);
            if (!Equal(current.foreground, style.foreground)) {
                current.foreground = style.foreground;
                if (current.foreground.has_value()) {
                    fprintf(file_, ESCAPE "[38;2;%d;%d;%dm",  //
                            static_cast<int>(style.foreground->r),
                            static_cast<int>(style.foreground->g),
                            static_cast<int>(style.foreground->b));
                } else {
                    fprintf(file_, ESCAPE "[39m");
                }
            }
            if (!Equal(current.background, style.background)) {
                current.background = style.background;
                if (current.background.has_value()) {
                    fprintf(file_, ESCAPE "[48;2;%d;%d;%dm",  //
                            static_cast<int>(style.background->r),
                            static_cast<int>(style.background->g),
                            static_cast<int>(style.background->b));
                } else {
                    fprintf(file_, ESCAPE "[49m");
                }
            }
            if (!Equal(current.underlined, style.underlined)) {
                current.underlined = style.underlined;
                if (current.underlined == true) {
                    fprintf(file_, ESCAPE "[4m");
                } else {
                    fprintf(file_, ESCAPE "[24m");
                }
            }
            if (!Equal(current.bold, style.bold)) {
                current.bold = style.bold;
                if (current.bold == true) {
                    fprintf(file_, ESCAPE "[1m");
                } else {
                    fprintf(file_, ESCAPE "[22m");
                }
            }
            fwrite(text.data(), 1, text.size(), file_);
        });
        fprintf(file_, ESCAPE "[m");
        fflush(file_);
    }

  private:
    FILE* const file_;
    const StyledTextTheme& theme_;
};

}  // namespace

std::unique_ptr<StyledTextPrinter> StyledTextPrinter::CreateANSI(FILE* out,
                                                                 const StyledTextTheme& theme) {
    return std::make_unique<Printer>(out, theme);
}

}  // namespace tint
