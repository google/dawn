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

#include "src/diagnostic/formatter.h"

#include <algorithm>
#include <sstream>
#include <vector>

#include "src/diagnostic/diagnostic.h"
#include "src/diagnostic/printer.h"

namespace tint {
namespace diag {
namespace {

const char* to_str(Severity severity) {
  switch (severity) {
    case Severity::Info:
      return "info";
    case Severity::Warning:
      return "warning";
    case Severity::Error:
      return "error";
    case Severity::Fatal:
      return "fatal";
  }
  return "";
}

std::string to_str(const Source::Location& location) {
  std::stringstream ss;
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
      std::stringstream reset;
      stream.swap(reset);
    }
  }

  /// operator<< queues msg to be written to the printer.
  /// @param msg the value or string to write to the printer
  /// @returns this State so that calls can be chained
  template <typename T>
  State& operator<<(const T& msg) {
    stream << msg;
    return *this;
  }

  /// newline queues a newline to be written to the printer.
  void newline() { stream << std::endl; }

  /// repeat queues the character c to be written to the printer n times.
  /// @param c the character to print `n` times
  /// @param n the number of times to print character `c`
  void repeat(char c, size_t n) {
    while (n-- > 0) {
      stream << c;
    }
  }

 private:
  Printer* printer;
  diag::Style style;
  std::stringstream stream;
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

  if (style_.print_file && src.file != nullptr && !src.file->path.empty()) {
    if (rng.begin.line > 0) {
      prefix.emplace_back(TextAndColor{src.file->path + ":" + to_str(rng.begin),
                                       Color::kDefault});
    } else {
      prefix.emplace_back(TextAndColor{src.file->path, Color::kDefault});
    }
  } else if (rng.begin.line > 0) {
    prefix.emplace_back(TextAndColor{to_str(rng.begin), Color::kDefault});
  }

  Color severity_color = Color::kDefault;
  switch (diag.severity) {
    case Severity::Warning:
      severity_color = Color::kYellow;
      break;
    case Severity::Error:
    case Severity::Fatal:
      severity_color = Color::kRed;
      break;
    default:
      break;
  }
  if (style_.print_severity) {
    prefix.emplace_back(
        TextAndColor{to_str(diag.severity), severity_color, true});
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

  if (style_.print_line && src.file != nullptr && rng.begin.line > 0) {
    state.newline();
    state.set_style({Color::kDefault, false});

    for (size_t line = rng.begin.line; line <= rng.end.line; line++) {
      if (line < src.file->lines.size() + 1) {
        auto len = src.file->lines[line - 1].size();

        state << src.file->lines[line - 1];

        state.newline();
        state.set_style({Color::kCyan, false});

        if (line == rng.begin.line && line == rng.end.line) {
          // Single line
          state.repeat(' ', rng.begin.column - 1);
          state.repeat('^',
                       std::max<size_t>(rng.end.column - rng.begin.column, 1));
        } else if (line == rng.begin.line) {
          // Start of multi-line
          state.repeat(' ', rng.begin.column - 1);
          state.repeat('^', len - (rng.begin.column - 1));
        } else if (line == rng.end.line) {
          // End of multi-line
          state.repeat('^', rng.end.column - 1);
        } else {
          // Middle of multi-line
          state.repeat('^', len);
        }
        state.newline();
      }
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

}  // namespace diag
}  // namespace tint
