
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

#ifndef SRC_TINT_SOURCE_H_
#define SRC_TINT_SOURCE_H_

#include <string>
#include <string_view>
#include <tuple>
#include <vector>

#include "src/tint/utils/string_stream.h"

namespace tint {

/// Source describes a range of characters within a source file.
class Source {
  public:
    /// FileContent describes the content of a source file encoded using utf-8.
    class FileContent {
      public:
        /// Constructs the FileContent with the given file content.
        /// @param data the file contents
        explicit FileContent(const std::string& data);

        /// Copy constructor
        /// @param rhs the FileContent to copy
        FileContent(const FileContent& rhs);

        /// Destructor
        ~FileContent();

        /// The original un-split file content
        const std::string data;
        /// #data split by lines
        const std::vector<std::string_view> lines;
    };

    /// File describes a source file, including path and content.
    class File {
      public:
        /// Constructs the File with the given file path and content.
        /// @param p the path for this file
        /// @param c the file contents
        inline File(const std::string& p, const std::string& c) : path(p), content(c) {}

        /// Copy constructor
        File(const File&) = default;

        /// Move constructor
        File(File&&) = default;

        /// Destructor
        ~File();

        /// file path
        const std::string path;
        /// file content
        const FileContent content;
    };

    /// Location holds a 1-based line and column index.
    class Location {
      public:
        /// the 1-based line number. 0 represents no line information.
        size_t line = 0;
        /// the 1-based column number in utf8-code units (bytes).
        /// 0 represents no column information.
        size_t column = 0;

        /// Returns true if `this` location is lexicographically less than `rhs`
        /// @param rhs location to compare against
        /// @returns true if `this` < `rhs`
        inline bool operator<(const Source::Location& rhs) {
            return std::tie(line, column) < std::tie(rhs.line, rhs.column);
        }

        /// Returns true of `this` location is equal to `rhs`
        /// @param rhs location to compare against
        /// @returns true if `this` == `rhs`
        inline bool operator==(const Location& rhs) const {
            return line == rhs.line && column == rhs.column;
        }

        /// Returns true of `this` location is not equal to `rhs`
        /// @param rhs location to compare against
        /// @returns true if `this` != `rhs`
        inline bool operator!=(const Location& rhs) const { return !(*this == rhs); }
    };

    /// Range holds a Location interval described by [begin, end).
    class Range {
      public:
        /// Constructs a zero initialized Range.
        inline Range() = default;

        /// Constructs a zero-length Range starting at `loc`
        /// @param loc the start and end location for the range
        inline constexpr explicit Range(const Location& loc) : begin(loc), end(loc) {}

        /// Constructs the Range beginning at `b` and ending at `e`
        /// @param b the range start location
        /// @param e the range end location
        inline constexpr Range(const Location& b, const Location& e) : begin(b), end(e) {}

        /// Return a column-shifted Range
        /// @param n the number of characters to shift by
        /// @returns a Range with a #begin and #end column shifted by `n`
        inline Range operator+(size_t n) const {
            return Range{{begin.line, begin.column + n}, {end.line, end.column + n}};
        }

        /// Returns true of `this` range is not equal to `rhs`
        /// @param rhs range to compare against
        /// @returns true if `this` != `rhs`
        inline bool operator==(const Range& rhs) const {
            return begin == rhs.begin && end == rhs.end;
        }

        /// Returns true of `this` range is equal to `rhs`
        /// @param rhs range to compare against
        /// @returns true if `this` == `rhs`
        inline bool operator!=(const Range& rhs) const { return !(*this == rhs); }

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
    /// @param f the source file
    inline Source(const Range& rng, File const* f) : range(rng), file(f) {}

    /// @returns a Source that points to the begin range of this Source.
    inline Source Begin() const { return Source(Range{range.begin}, file); }

    /// @returns a Source that points to the end range of this Source.
    inline Source End() const { return Source(Range{range.end}, file); }

    /// Return a column-shifted Source
    /// @param n the number of characters to shift by
    /// @returns a Source with the range's columns shifted by `n`
    inline Source operator+(size_t n) const { return Source(range + n, file); }

    /// Returns true of `this` Source is lexicographically less than `rhs`
    /// @param rhs source to compare against
    /// @returns true if `this` < `rhs`
    inline bool operator<(const Source& rhs) {
        if (file != rhs.file) {
            return false;
        }
        return range.begin < rhs.range.begin;
    }

    /// Helper function that returns the range union of two source locations. The
    /// `start` and `end` locations are assumed to refer to the same source file.
    /// @param start the start source of the range
    /// @param end the end source of the range
    /// @returns the combined source
    inline static Source Combine(const Source& start, const Source& end) {
        return Source(Source::Range(start.range.begin, end.range.end), start.file);
    }

    /// range is the span of text this source refers to in #file
    Range range;
    /// file is the optional source content this source refers to
    const File* file = nullptr;
};

/// Writes the Source::Location to the stream.
/// @param out the stream to write to
/// @param loc the location to write
/// @returns out so calls can be chained
inline utils::StringStream& operator<<(utils::StringStream& out, const Source::Location& loc) {
    out << loc.line << ":" << loc.column;
    return out;
}

/// Writes the Source::Range to the stream.
/// @param out the stream to write to
/// @param range the range to write
/// @returns out so calls can be chained
inline utils::StringStream& operator<<(utils::StringStream& out, const Source::Range& range) {
    out << "[" << range.begin << ", " << range.end << "]";
    return out;
}

/// Writes the Source to the stream.
/// @param out the stream to write to
/// @param source the source to write
/// @returns out so calls can be chained
utils::StringStream& operator<<(utils::StringStream& out, const Source& source);

/// Writes the Source::FileContent to the stream.
/// @param out the stream to write to
/// @param content the file content to write
/// @returns out so calls can be chained
inline utils::StringStream& operator<<(utils::StringStream& out,
                                       const Source::FileContent& content) {
    out << content.data;
    return out;
}

}  // namespace tint

#endif  // SRC_TINT_SOURCE_H_
