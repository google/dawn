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

#include "src/diagnostic/diagnostic.h"

namespace tint {
namespace diag {
namespace {

template <typename CharT, typename Traits>
std::basic_ostream<CharT, Traits>& operator<<(
    std::basic_ostream<CharT, Traits>& stream,
    Severity severity) {
  switch (severity) {
    case Severity::Info:
      stream << "info";
      break;
    case Severity::Warning:
      stream << "warning";
      break;
    case Severity::Error:
      stream << "error";
      break;
    case Severity::Fatal:
      stream << "fatal";
      break;
  }
  return stream;
}

template <typename CharT, typename Traits>
std::basic_ostream<CharT, Traits>& operator<<(
    std::basic_ostream<CharT, Traits>& stream,
    const Source::Location& location) {
  if (location.line > 0) {
    stream << location.line;
    if (location.column > 0) {
      stream << ":" << location.column;
    }
  }
  return stream;
}

class BasicFormatter : public Formatter {
 public:
  BasicFormatter(bool print_file, bool print_severity, bool print_line)
      : print_file_(print_file),
        print_severity_(print_severity),
        print_line_(print_line) {}

  std::string format(const List& list) const override {
    bool first = true;
    std::stringstream ss;
    for (auto diag : list) {
      if (!first) {
        ss << std::endl;
      }
      format(diag, ss);
      first = false;
    }
    return ss.str();
  }

 private:
  void format(const Diagnostic& diag, std::stringstream& ss) const {
    auto const& src = diag.source;
    auto const& rng = src.range;

    if (print_file_ && src.file != nullptr && !src.file->path.empty()) {
      ss << src.file->path;
      if (rng.begin.line > 0) {
        ss << ":" << rng.begin;
      }
    } else {
      ss << rng.begin;
    }
    if (print_severity_) {
      ss << " " << diag.severity;
    }
    ss << ": " << diag.message;

    if (print_line_ && src.file != nullptr && rng.begin.line > 0) {
      ss << std::endl;
      for (size_t line = rng.begin.line; line <= rng.end.line; line++) {
        if (line < src.file->lines.size() + 1) {
          auto len = src.file->lines[line - 1].size();

          ss << src.file->lines[line - 1];
          ss << std::endl;

          if (line == rng.begin.line && line == rng.end.line) {
            // Single line
            repeat(' ', rng.begin.column - 1, ss);
            repeat('^', std::max<size_t>(rng.end.column - rng.begin.column, 1),
                   ss);
          } else if (line == rng.begin.line) {
            // Start of multi-line
            repeat(' ', rng.begin.column - 1, ss);
            repeat('^', len - (rng.begin.column - 1), ss);
          } else if (line == rng.end.line) {
            // End of multi-line
            repeat('^', rng.end.column - 1, ss);
          } else {
            // Middle of multi-line
            repeat('^', len, ss);
          }
          ss << std::endl;
        }
      }
    }
  }

  void repeat(char c, size_t n, std::stringstream& ss) const {
    while (n-- > 0) {
      ss << c;
    }
  }

  const bool print_file_ = false;
  const bool print_severity_ = false;
  const bool print_line_ = false;
};

}  // namespace

std::unique_ptr<Formatter> Formatter::create(bool print_file,
                                             bool print_severity,
                                             bool print_line) {
  return std::make_unique<BasicFormatter>(print_file, print_severity,
                                          print_line);
}

Formatter::~Formatter() = default;

}  // namespace diag
}  // namespace tint
