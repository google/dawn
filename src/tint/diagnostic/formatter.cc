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

#include "src/tint/diagnostic/formatter.h"

#include <algorithm>
#include <iterator>
#include <utility>
#include <vector>

#include "src/tint/diagnostic/diagnostic.h"
#include "src/tint/diagnostic/printer.h"
#include "src/tint/utils/string_stream.h"

namespace tint::diag {
namespace {

const char* to_str(Severity severity) {
    switch (severity) {
        case Severity::Note:
            return "note";
        case Severity::Warning:
            return "warning";
        case Severity::Error:
            return "error";
        case Severity::InternalCompilerError:
            return "internal compiler error";
        case Severity::Fatal:
            return "fatal";
    }
    return "";
}

std::string to_str(const Source::Location& location) {
    utils::StringStream ss;
    if (location.line > 0) {
        ss << location.line;
        if (location.column > 0) {
            ss << ":" << location.column;
        }
    }
    return ss.str();
}

}  // namespace

/// State holds the internal formatter state for a format() call.
struct Formatter::State {
    /// Constructs a State associated with the given printer.
    /// @param p the printer to write formatted messages to.
    explicit State(Printer* p) : printer(p) {}
    ~State() { flush(); }

    /// set_style() sets the current style to new_style, flushing any pending
    /// messages to the printer if the style changed.
    /// @param new_style the new style to apply for future written messages.
    void set_style(const diag::Style& new_style) {
        if (style.color != new_style.color || style.bold != new_style.bold) {
            flush();
            style = new_style;
        }
    }

    /// flush writes any pending messages to the printer, clearing the buffer.
    void flush() {
        auto str = stream.str();
        if (str.length() > 0) {
            printer->write(str, style);
            utils::StringStream reset;
            stream.swap(reset);
        }
    }

    /// operator<< queues msg to be written to the printer.
    /// @param msg the value or string to write to the printer
    /// @returns this State so that calls can be chained
    template <typename T>
    State& operator<<(T&& msg) {
        stream << std::forward<T>(msg);
        return *this;
    }

    /// newline queues a newline to be written to the printer.
    void newline() { stream << std::endl; }

    /// repeat queues the character c to be written to the printer n times.
    /// @param c the character to print `n` times
    /// @param n the number of times to print character `c`
    void repeat(char c, size_t n) { stream.repeat(c, n); }

  private:
    Printer* printer;
    diag::Style style;
    utils::StringStream stream;
};

Formatter::Formatter() {}
Formatter::Formatter(const Style& style) : style_(style) {}

void Formatter::format(const List& list, Printer* printer) const {
    State state{printer};

    bool first = true;
    for (auto diag : list) {
        state.set_style({});
        if (!first) {
            state.newline();
        }
        format(diag, state);
        first = false;
    }

    if (style_.print_newline_at_end) {
        state.newline();
    }
}

void Formatter::format(const Diagnostic& diag, State& state) const {
    auto const& src = diag.source;
    auto const& rng = src.range;
    bool has_code = diag.code != nullptr && diag.code[0] != '\0';

    state.set_style({Color::kDefault, true});

    struct TextAndColor {
        std::string text;
        Color color;
        bool bold = false;
    };
    std::vector<TextAndColor> prefix;
    prefix.reserve(6);

    if (style_.print_file && src.file != nullptr) {
        if (rng.begin.line > 0) {
            prefix.emplace_back(
                TextAndColor{src.file->path + ":" + to_str(rng.begin), Color::kDefault});
        } else {
            prefix.emplace_back(TextAndColor{src.file->path, Color::kDefault});
        }
    } else if (rng.begin.line > 0) {
        prefix.emplace_back(TextAndColor{to_str(rng.begin), Color::kDefault});
    }

    Color severity_color = Color::kDefault;
    switch (diag.severity) {
        case Severity::Note:
            break;
        case Severity::Warning:
            severity_color = Color::kYellow;
            break;
        case Severity::Error:
            severity_color = Color::kRed;
            break;
        case Severity::Fatal:
        case Severity::InternalCompilerError:
            severity_color = Color::kMagenta;
            break;
    }
    if (style_.print_severity) {
        prefix.emplace_back(TextAndColor{to_str(diag.severity), severity_color, true});
    }
    if (has_code) {
        prefix.emplace_back(TextAndColor{diag.code, severity_color});
    }

    for (size_t i = 0; i < prefix.size(); i++) {
        if (i > 0) {
            state << " ";
        }
        state.set_style({prefix[i].color, prefix[i].bold});
        state << prefix[i].text;
    }

    state.set_style({Color::kDefault, true});
    if (!prefix.empty()) {
        state << ": ";
    }
    state << diag.message;

    if (style_.print_line && src.file && rng.begin.line > 0) {
        state.newline();
        state.set_style({Color::kDefault, false});

        for (size_t line_num = rng.begin.line;
             (line_num <= rng.end.line) && (line_num <= src.file->content.lines.size());
             line_num++) {
            auto& line = src.file->content.lines[line_num - 1];
            auto line_len = line.size();

            bool is_ascii = true;
            for (auto c : line) {
                if (c == '\t') {
                    state.repeat(' ', style_.tab_width);
                } else {
                    state << c;
                }
                if (c & 0x80) {
                    is_ascii = false;
                }
            }

            state.newline();

            // If the line contains non-ascii characters, then we cannot assume that
            // a single utf8 code unit represents a single glyph, so don't attempt to
            // draw squiggles.
            if (!is_ascii) {
                continue;
            }

            state.set_style({Color::kCyan, false});

            // Count the number of glyphs in the line span.
            // start and end use 1-based indexing.
            auto num_glyphs = [&](size_t start, size_t end) {
                size_t count = 0;
                start = (start > 0) ? (start - 1) : 0;
                end = (end > 0) ? (end - 1) : 0;
                for (size_t i = start; (i < end) && (i < line_len); i++) {
                    count += (line[i] == '\t') ? style_.tab_width : 1;
                }
                return count;
            };

            if (line_num == rng.begin.line && line_num == rng.end.line) {
                // Single line
                state.repeat(' ', num_glyphs(1, rng.begin.column));
                state.repeat('^',
                             std::max<size_t>(num_glyphs(rng.begin.column, rng.end.column), 1));
            } else if (line_num == rng.begin.line) {
                // Start of multi-line
                state.repeat(' ', num_glyphs(1, rng.begin.column));
                state.repeat('^', num_glyphs(rng.begin.column, line_len + 1));
            } else if (line_num == rng.end.line) {
                // End of multi-line
                state.repeat('^', num_glyphs(1, rng.end.column));
            } else {
                // Middle of multi-line
                state.repeat('^', num_glyphs(1, line_len + 1));
            }
            state.newline();
        }

        state.set_style({});
    }
}

std::string Formatter::format(const List& list) const {
    StringPrinter printer;
    format(list, &printer);
    return printer.str();
}

Formatter::~Formatter() = default;

}  // namespace tint::diag
