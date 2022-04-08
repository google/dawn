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
#include <sstream>
#include <string_view>
#include <utility>

namespace tint {
namespace {
std::vector<std::string_view> SplitLines(std::string_view str) {
  std::vector<std::string_view> lines;

  size_t lineStart = 0;
  for (size_t i = 0; i < str.size(); ++i) {
    if (str[i] == '\n') {
      // Handle CRLF on Windows
      size_t curr = i;
      if (i > 0 && str[i - 1] == '\r') {
        --curr;
      }
      lines.push_back(str.substr(lineStart, curr - lineStart));
      lineStart = i + 1;
    }
  }
  if (lineStart < str.size()) {
    lines.push_back(str.substr(lineStart));
  }

  return lines;
}

std::vector<std::string_view> CopyRelativeStringViews(
    const std::vector<std::string_view>& src_list,
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

Source::FileContent::FileContent(const std::string& body)
    : data(body), data_view(data), lines(SplitLines(data_view)) {}

Source::FileContent::FileContent(const FileContent& rhs)
    : data(rhs.data),
      data_view(data),
      lines(CopyRelativeStringViews(rhs.lines, rhs.data_view, data_view)) {}

Source::FileContent::~FileContent() = default;

Source::File::~File() = default;

std::ostream& operator<<(std::ostream& out, const Source& source) {
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
