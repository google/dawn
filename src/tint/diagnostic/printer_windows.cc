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

#include <cstring>

#include "src/tint/diagnostic/printer.h"

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
