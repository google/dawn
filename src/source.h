
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

#ifndef SRC_SOURCE_H_
#define SRC_SOURCE_H_

#include <iostream>
#include <string>
#include <vector>

namespace tint {

/// Source describes a range of characters within a source file.
class Source {
 public:
  /// FileContent describes the content of a source file.
  class FileContent {
   public:
    /// Constructs the FileContent with the given file content.
    /// @param data the file contents
    explicit FileContent(const std::string& data);

    /// Destructor
    ~FileContent();

    /// un-split file content
    const std::string data;
    /// #data split by lines
    const std::vector<std::string> lines;
  };

  /// File describes a source file, including path and content.
  class File {
   public:
    /// Constructs the File with the given file path and content.
    /// @param p the path for this file
    /// @param c the file contents
    inline File(const std::string& p, const std::string& c)
        : path(p), content(c) {}

    ~File();

    /// file path (optional)
    const std::string path;
    /// file content
    const FileContent content;
  };

  /// Location holds a 1-based line and column index.
  class Location {
   public:
    /// the 1-based line number. 0 represents no line information.
    size_t line = 0;
    /// the 1-based column number. 0 represents no column information.
    size_t column = 0;
  };

  /// Range holds a Location interval described by [begin, end).
  class Range {
   public:
    /// Constructs a zero initialized Range.
    inline Range() = default;

    /// Constructs a zero-length Range starting at `loc`
    /// @param loc the start and end location for the range
    inline explicit Range(const Location& loc) : begin(loc), end(loc) {}

    /// Constructs the Range beginning at `b` and ending at `e`
    /// @param b the range start location
    /// @param e the range end location
    inline Range(const Location& b, const Location& e) : begin(b), end(e) {}

    /// Return a column-shifted Range
    /// @param n the number of characters to shift by
    /// @returns a Range with a #begin and #end column shifted by `n`
    inline Range operator+(size_t n) const {
      return Range{{begin.line, begin.column + n}, {end.line, end.column + n}};
    }

    /// The location of the first character in the range.
    Location begin;
    /// The location of one-past the last character in the range.
    Location end;
  };

  /// Constructs the Source with an zero initialized Range and null File.
  inline Source() : range() {}

  /// Constructs the Source with the Range `rng` and a null File
  /// @param rng the source range
  inline explicit Source(const Range& rng) : range(rng) {}

  /// Constructs the Source with the Range `loc` and a null File
  /// @param loc the start and end location for the source range
  inline explicit Source(const Location& loc) : range(Range(loc)) {}

  /// Constructs the Source with the Range `rng` and File `file`
  /// @param rng the source range
  /// @param file the source file
  inline Source(const Range& rng, File const* file)
      : range(rng), file_path(file->path), file_content(&file->content) {}

  /// Constructs the Source with the Range `rng`, file path `path` and content
  /// `content`
  /// @param rng the source range
  /// @param path the source file path
  /// @param content the source file content
  inline Source(const Range& rng,
                const std::string& path,
                const FileContent* content = nullptr)
      : range(rng), file_path(path), file_content(content) {}

  /// @returns a Source that points to the begin range of this Source.
  inline Source Begin() const {
    return Source(Range{range.begin}, file_path, file_content);
  }

  /// @returns a Source that points to the end range of this Source.
  inline Source End() const {
    return Source(Range{range.end}, file_path, file_content);
  }

  /// Return a column-shifted Source
  /// @param n the number of characters to shift by
  /// @returns a Source with the range's columns shifted by `n`
  inline Source operator+(size_t n) const {
    return Source(range + n, file_path, file_content);
  }

  /// range is the span of text this source refers to in #file_path
  Range range;
  /// file is the optional file path this source refers to
  std::string file_path;
  /// file is the optional source content this source refers to
  const FileContent* file_content = nullptr;
};

/// Writes the Source::FileContent to the std::ostream.
/// @param out the std::ostream to write to
/// @param content the file content to write
/// @returns out so calls can be chained
inline std::ostream& operator<<(std::ostream& out,
                                const Source::FileContent& content) {
  out << content.data;
  return out;
}

}  // namespace tint

#endif  // SRC_SOURCE_H_
