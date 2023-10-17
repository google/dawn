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

// GEN_BUILD:CONDITION(is_win)

#include <cstring>

#include "src/tint/utils/diagnostic/printer.h"

#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>

namespace tint::diag {
namespace {

struct ConsoleInfo {
    HANDLE handle = INVALID_HANDLE_VALUE;
    WORD default_attributes = 0;
    operator bool() const { return handle != INVALID_HANDLE_VALUE; }
};

ConsoleInfo console_info(FILE* file) {
    if (file == nullptr) {
        return {};
    }

    ConsoleInfo console{};
    if (file == stdout) {
        console.handle = GetStdHandle(STD_OUTPUT_HANDLE);
    } else if (file == stderr) {
        console.handle = GetStdHandle(STD_ERROR_HANDLE);
    } else {
        return {};
    }

    CONSOLE_SCREEN_BUFFER_INFO info{};
    if (GetConsoleScreenBufferInfo(console.handle, &info) == 0) {
        return {};
    }

    console.default_attributes = info.wAttributes;
    return console;
}

class PrinterWindows : public Printer {
  public:
    PrinterWindows(FILE* f, bool use_colors)
        : file(f), console(console_info(use_colors ? f : nullptr)) {}

    void write(const std::string& str, const Style& style) override {
        write_color(style.color, style.bold);
        fwrite(str.data(), 1, str.size(), file);
        write_color(Color::kDefault, false);
    }

  private:
    WORD attributes(Color color, bool bold) {
        switch (color) {
            case Color::kDefault:
                return console.default_attributes;
            case Color::kBlack:
                return 0;
            case Color::kRed:
                return FOREGROUND_RED | (bold ? FOREGROUND_INTENSITY : 0);
            case Color::kGreen:
                return FOREGROUND_GREEN | (bold ? FOREGROUND_INTENSITY : 0);
            case Color::kYellow:
                return FOREGROUND_RED | FOREGROUND_GREEN | (bold ? FOREGROUND_INTENSITY : 0);
            case Color::kBlue:
                return FOREGROUND_BLUE | (bold ? FOREGROUND_INTENSITY : 0);
            case Color::kMagenta:
                return FOREGROUND_RED | FOREGROUND_BLUE | (bold ? FOREGROUND_INTENSITY : 0);
            case Color::kCyan:
                return FOREGROUND_GREEN | FOREGROUND_BLUE | (bold ? FOREGROUND_INTENSITY : 0);
            case Color::kWhite:
                return FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE |
                       (bold ? FOREGROUND_INTENSITY : 0);
        }
        return 0;  // unreachable
    }

    void write_color(Color color, bool bold) {
        if (console) {
            SetConsoleTextAttribute(console.handle, attributes(color, bold));
            fflush(file);
        }
    }

    FILE* const file;
    const ConsoleInfo console;
};

}  // namespace

std::unique_ptr<Printer> Printer::create(FILE* out, bool use_colors) {
    return std::make_unique<PrinterWindows>(out, use_colors);
}

}  // namespace tint::diag
