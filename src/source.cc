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

#include "src/source.h"

#include <algorithm>
#include <sstream>
#include <utility>

namespace tint {
namespace {
std::vector<std::string> split_lines(const std::string& str) {
  std::stringstream stream(str);
  std::string line;
  std::vector<std::string> lines;
  while (std::getline(stream, line, '\n')) {
    lines.emplace_back(std::move(line));
  }
  return lines;
}
}  // namespace

Source::FileContent::FileContent(const std::string& body)
    : data(body), lines(split_lines(body)) {}

Source::FileContent::~FileContent() = default;

Source::File::~File() = default;

std::ostream& operator<<(std::ostream& out, const Source& source) {
  auto rng = source.range;

  if (!source.file_path.empty()) {
    out << source.file_path << ":";
  }
  if (rng.begin.line) {
    out << rng.begin.line << ":";
    if (rng.begin.column) {
      out << rng.begin.column;
    }

    if (source.file_content) {
      out << std::endl << std::endl;

      auto repeat = [&](char c, size_t n) {
        while (n--) {
          out << c;
        }
      };

      for (size_t line = rng.begin.line; line <= rng.end.line; line++) {
        if (line < source.file_content->lines.size() + 1) {
          auto len = source.file_content->lines[line - 1].size();

          out << source.file_content->lines[line - 1];

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
