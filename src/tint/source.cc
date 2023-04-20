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

#include "src/tint/source.h"

#include <algorithm>
#include <string_view>
#include <utility>

#include "src/tint/utils/unicode.h"

namespace tint {
namespace {

bool ParseLineBreak(std::string_view str, size_t i, bool* is_line_break, size_t* line_break_size) {
    // See https://www.w3.org/TR/WGSL/#blankspace

    auto* utf8 = reinterpret_cast<const uint8_t*>(&str[i]);
    auto [cp, n] = utils::utf8::Decode(utf8, str.size() - i);

    if (n == 0) {
        return false;
    }

    static const auto kLF = utils::CodePoint(0x000A);    // line feed
    static const auto kVTab = utils::CodePoint(0x000B);  // vertical tab
    static const auto kFF = utils::CodePoint(0x000C);    // form feed
    static const auto kNL = utils::CodePoint(0x0085);    // next line
    static const auto kCR = utils::CodePoint(0x000D);    // carriage return
    static const auto kLS = utils::CodePoint(0x2028);    // line separator
    static const auto kPS = utils::CodePoint(0x2029);    // parargraph separator

    if (cp == kLF || cp == kVTab || cp == kFF || cp == kNL || cp == kPS || cp == kLS) {
        *is_line_break = true;
        *line_break_size = n;
        return true;
    }

    // Handle CRLF as one line break, and CR alone as one line break
    if (cp == kCR) {
        *is_line_break = true;
        *line_break_size = n;

        if (auto next_i = i + n; next_i < str.size()) {
            auto* next_utf8 = reinterpret_cast<const uint8_t*>(&str[next_i]);
            auto [next_cp, next_n] = utils::utf8::Decode(next_utf8, str.size() - next_i);

            if (next_n == 0) {
                return false;
            }

            if (next_cp == kLF) {
                // CRLF as one break
                *line_break_size = n + next_n;
            }
        }

        return true;
    }

    *is_line_break = false;
    return true;
}

std::vector<std::string_view> SplitLines(std::string_view str) {
    std::vector<std::string_view> lines;

    size_t lineStart = 0;
    for (size_t i = 0; i < str.size();) {
        bool is_line_break{};
        size_t line_break_size{};
        // We don't handle decode errors from ParseLineBreak. Instead, we rely on
        // the Lexer to do so.
        ParseLineBreak(str, i, &is_line_break, &line_break_size);
        if (is_line_break) {
            lines.push_back(str.substr(lineStart, i - lineStart));
            i += line_break_size;
            lineStart = i;
        } else {
            ++i;
        }
    }
    if (lineStart < str.size()) {
        lines.push_back(str.substr(lineStart));
    }

    return lines;
}

std::vector<std::string_view> CopyRelativeStringViews(const std::vector<std::string_view>& src_list,
                                                      const std::string_view& src_view,
                                                      const std::string_view& dst_view) {
    std::vector<std::string_view> out(src_list.size());
    for (size_t i = 0; i < src_list.size(); i++) {
        auto offset = static_cast<size_t>(&src_list[i].front() - &src_view.front());
        auto count = src_list[i].length();
        out[i] = dst_view.substr(offset, count);
    }
    return out;
}

}  // namespace

Source::FileContent::FileContent(const std::string& body) : data(body), lines(SplitLines(data)) {}

Source::FileContent::FileContent(const FileContent& rhs)
    : data(rhs.data), lines(CopyRelativeStringViews(rhs.lines, rhs.data, data)) {}

Source::FileContent::~FileContent() = default;

Source::File::~File() = default;

utils::StringStream& operator<<(utils::StringStream& out, const Source& source) {
    auto rng = source.range;

    if (source.file) {
        out << source.file->path << ":";
    }
    if (rng.begin.line) {
        out << rng.begin.line << ":";
        if (rng.begin.column) {
            out << rng.begin.column;
        }

        if (source.file) {
            out << std::endl << std::endl;

            auto repeat = [&](char c, size_t n) {
                while (n--) {
                    out << c;
                }
            };

            for (size_t line = rng.begin.line; line <= rng.end.line; line++) {
                if (line < source.file->content.lines.size() + 1) {
                    auto len = source.file->content.lines[line - 1].size();

                    out << source.file->content.lines[line - 1];

                    out << std::endl;

                    if (line == rng.begin.line && line == rng.end.line) {
                        // Single line
                        repeat(' ', rng.begin.column - 1);
                        repeat('^', std::max<size_t>(rng.end.column - rng.begin.column, 1));
                    } else if (line == rng.begin.line) {
                        // Start of multi-line
                        repeat(' ', rng.begin.column - 1);
                        repeat('^', len - (rng.begin.column - 1));
                    } else if (line == rng.end.line) {
                        // End of multi-line
                        repeat('^', rng.end.column - 1);
                    } else {
                        // Middle of multi-line
                        repeat('^', len);
                    }

                    out << std::endl;
                }
            }
        }
    }
    return out;
}

}  // namespace tint
