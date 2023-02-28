// Copyright 2023 The Tint Authors.
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

#ifndef SRC_TINT_UTILS_STRING_STREAM_H_
#define SRC_TINT_UTILS_STRING_STREAM_H_

#include <functional>
#include <iterator>
#include <limits>
#include <sstream>
#include <string>
#include <utility>

namespace tint::utils {

/// Stringstream wrapper which automatically resets the locale and sets floating point emission
/// settings needed for Tint.
class StringStream {
  public:
    /// Constructor
    StringStream();
    /// Destructor
    ~StringStream();

    /// @returns the format flags for the stream
    std::ios_base::fmtflags flags() const { return sstream_.flags(); }

    /// @param flags the flags to set
    /// @returns the original format flags
    std::ios_base::fmtflags flags(std::ios_base::fmtflags flags) { return sstream_.flags(flags); }

    /// Emit `value` to the stream
    /// @param value the value to emit
    /// @returns a reference to this
    template <typename T,
              typename std::enable_if<!std::is_floating_point<T>::value>::type* = nullptr>
    StringStream& operator<<(const T& value) {
        sstream_ << value;
        return *this;
    }

    /// Emit `value` to the stream
    /// @param value the value to emit
    /// @returns a reference to this
    template <typename T,
              typename std::enable_if<std::is_floating_point<T>::value>::type* = nullptr>
    StringStream& operator<<(const T& value) {
        // Try printing the float in fixed point, with a smallish limit on the precision
        std::stringstream fixed;
        fixed.flags(fixed.flags() | std::ios_base::showpoint | std::ios_base::fixed);
        fixed.imbue(std::locale::classic());
        fixed.precision(9);
        fixed << value;

        std::string str = fixed.str();

        // If this string can be parsed without loss of information, use it.
        // (Use double here to dodge a bug in older libc++ versions which would incorrectly read
        // back FLT_MAX as INF.)
        double roundtripped;
        fixed >> roundtripped;

        auto float_equal_no_warning = std::equal_to<T>();
        if (float_equal_no_warning(value, static_cast<T>(roundtripped))) {
            while (str.length() >= 2 && str[str.size() - 1] == '0' && str[str.size() - 2] != '.') {
                str.pop_back();
            }

            sstream_ << str;
            return *this;
        }

        // Resort to scientific, with the minimum precision needed to preserve the whole float
        std::stringstream sci;
        sci.imbue(std::locale::classic());
        sci.precision(std::numeric_limits<T>::max_digits10);
        sci << value;
        sstream_ << sci.str();

        return *this;
    }

    /// Swaps streams
    /// @param other stream to swap too
    void swap(StringStream& other) { sstream_.swap(other.sstream_); }

    /// repeat queues the character c to be written to the printer n times.
    /// @param c the character to print `n` times
    /// @param n the number of times to print character `c`
    void repeat(char c, size_t n) { std::fill_n(std::ostream_iterator<char>(sstream_), n, c); }

    /// The callback to emit a `endl` to the stream
    using StdEndl = std::ostream& (*)(std::ostream&);

    /// @param manipulator the callback to emit too
    /// @returns a reference to this
    StringStream& operator<<(StdEndl manipulator) {
        // call the function, and return it's value
        manipulator(sstream_);
        return *this;
    }

    /// @returns the string contents of the stream
    std::string str() const { return sstream_.str(); }

  private:
    std::stringstream sstream_;
};

}  // namespace tint::utils

#endif  // SRC_TINT_UTILS_STRING_STREAM_H_
