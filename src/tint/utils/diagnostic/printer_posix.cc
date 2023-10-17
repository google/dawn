// Copyright 2020 The Dawn & Tint Authors
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

// GEN_BUILD:CONDITION(is_linux || is_mac)

#include <unistd.h>

#include <cstring>

#include "src/tint/utils/diagnostic/printer.h"

namespace tint::diag {
namespace {

bool supports_colors(FILE* f) {
    if (!isatty(fileno(f))) {
        return false;
    }

    const char* cterm = getenv("TERM");
    if (cterm == nullptr) {
        return false;
    }

    std::string term = getenv("TERM");
    if (term != "cygwin" && term != "linux" && term != "rxvt-unicode-256color" &&
        term != "rxvt-unicode" && term != "screen-256color" && term != "screen" &&
        term != "tmux-256color" && term != "tmux" && term != "xterm-256color" &&
        term != "xterm-color" && term != "xterm") {
        return false;
    }

    return true;
}

class PrinterPosix : public Printer {
  public:
    PrinterPosix(FILE* f, bool colors) : file(f), use_colors(colors && supports_colors(f)) {}

    void write(const std::string& str, const Style& style) override {
        write_color(style.color, style.bold);
        fwrite(str.data(), 1, str.size(), file);
        write_color(Color::kDefault, false);
    }

  private:
    constexpr const char* color_code(Color color, bool bold) {
        switch (color) {
            case Color::kDefault:
                return bold ? "\u001b[1m" : "\u001b[0m";
            case Color::kBlack:
                return bold ? "\u001b[30;1m" : "\u001b[30m";
            case Color::kRed:
                return bold ? "\u001b[31;1m" : "\u001b[31m";
            case Color::kGreen:
                return bold ? "\u001b[32;1m" : "\u001b[32m";
            case Color::kYellow:
                return bold ? "\u001b[33;1m" : "\u001b[33m";
            case Color::kBlue:
                return bold ? "\u001b[34;1m" : "\u001b[34m";
            case Color::kMagenta:
                return bold ? "\u001b[35;1m" : "\u001b[35m";
            case Color::kCyan:
                return bold ? "\u001b[36;1m" : "\u001b[36m";
            case Color::kWhite:
                return bold ? "\u001b[37;1m" : "\u001b[37m";
        }
        return "";  // unreachable
    }

    void write_color(Color color, bool bold) {
        if (use_colors) {
            auto* code = color_code(color, bold);
            fwrite(code, 1, strlen(code), file);
        }
    }

    FILE* const file;
    const bool use_colors;
};

}  // namespace

std::unique_ptr<Printer> Printer::create(FILE* out, bool use_colors) {
    return std::make_unique<PrinterPosix>(out, use_colors);
}

}  // namespace tint::diag
