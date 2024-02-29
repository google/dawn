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

// GEN_BUILD:CONDITION(tint_build_is_linux || tint_build_is_mac)

#include <unistd.h>

#include <termios.h>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <optional>
#include <string_view>
#include <utility>

#include "src/tint/utils/macros/defer.h"
#include "src/tint/utils/system/env.h"
#include "src/tint/utils/system/terminal.h"

namespace tint {

bool TerminalSupportsColors(FILE* f) {
    if (!isatty(fileno(f))) {
        return false;
    }

    if (auto term = GetEnvVar("TERM"); !term.empty()) {
        return term == "cygwin" || term == "linux" || term == "rxvt-unicode-256color" ||
               term == "rxvt-unicode" || term == "screen-256color" || term == "screen" ||
               term == "tmux-256color" || term == "tmux" || term == "xterm-256color" ||
               term == "xterm-color" || term == "xterm";
    }

    return false;
}

/// Probes the terminal using a Device Control escape sequence to get the background color.
/// @see https://invisible-island.net/xterm/ctlseqs/ctlseqs.html#h2-Device-Control-functions
std::optional<bool> TerminalIsDark(FILE* out) {
    if (!TerminalSupportsColors(out)) {
        return std::nullopt;
    }

    // Get the file descriptor for 'out'
    int out_fd = fileno(out);
    if (out_fd == -1) {
        return std::nullopt;
    }

    // Store the current attributes for 'out', restore it before returning
    termios original_state{};
    tcgetattr(out_fd, &original_state);
    TINT_DEFER(tcsetattr(out_fd, TCSADRAIN, &original_state));

    // Prevent echoing.
    termios state = original_state;
    state.c_lflag &= ~static_cast<tcflag_t>(ECHO | ICANON);
    tcsetattr(out_fd, TCSADRAIN, &state);

    // Emit the device control escape sequence to query the terminal colors.
    static constexpr std::string_view kQuery = "\x1b]11;?\x07";
    fwrite(kQuery.data(), 1, kQuery.length(), out);
    fflush(out);

    // Timeout for attempting to read the response.
    static constexpr auto kTimeout = std::chrono::milliseconds(100);

    // Record the start time.
    auto start = std::chrono::steady_clock::now();

    // Helpers for parsing the response.
    std::optional<char> peek;
    auto read = [&]() -> std::optional<char> {
        if (peek) {
            char c = *peek;
            peek.reset();
            return c;
        }
        while ((std::chrono::steady_clock::now() - start) < kTimeout) {
            char c;
            if (fread(&c, 1, 1, stdin) == 1) {
                return c;
            }
        }
        return std::nullopt;
    };

    auto match = [&](std::string_view str) {
        for (char c : str) {
            if (c != read()) {
                return false;
            }
        }
        return true;
    };

    struct Hex {
        uint32_t num = 0;  // The parsed hex number
        uint32_t len = 0;  // Number of hex digits parsed
    };
    auto hex = [&] {
        uint32_t num = 0;
        uint32_t len = 0;
        while (auto c = read()) {
            if (c >= '0' && c <= '9') {
                num = num * 16 + static_cast<uint32_t>(*c - '0');
                len++;
            } else if (c >= 'a' && c <= 'z') {
                num = num * 16 + 10 + static_cast<uint32_t>(*c - 'a');
                len++;
            } else if (c >= 'A' && c <= 'Z') {
                num = num * 16 + 10 + static_cast<uint32_t>(*c - 'A');
                len++;
            } else {
                peek = c;
                break;
            }
        }
        return Hex{num, len};
    };

    if (!match("\033]11;rgb:")) {
        return std::nullopt;
    }

    auto r_i = hex();
    if (!match("/")) {
        return std::nullopt;
    }
    auto g_i = hex();
    if (!match("/")) {
        return std::nullopt;
    }
    auto b_i = hex();

    if (r_i.len != g_i.len || g_i.len != b_i.len) {
        return std::nullopt;
    }

    uint32_t max = 0;
    switch (r_i.len) {
        case 2:  // rr/gg/bb
            max = 0xff;
            break;
        case 4:  // rrrr/gggg/bbbb
            max = 0xffff;
            break;
        default:
            return std::nullopt;
    }

    // https://en.wikipedia.org/wiki/Relative_luminance
    float r = static_cast<float>(r_i.num) / static_cast<float>(max);
    float g = static_cast<float>(g_i.num) / static_cast<float>(max);
    float b = static_cast<float>(b_i.num) / static_cast<float>(max);
    return (0.2126f * r + 0.7152f * g + 0.0722f * b) < 0.5f;
}

}  // namespace tint
