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

#include <unistd.h>

#include <cstring>

#include "src/tint/diagnostic/printer.h"

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
