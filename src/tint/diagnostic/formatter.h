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

#ifndef SRC_TINT_DIAGNOSTIC_FORMATTER_H_
#define SRC_TINT_DIAGNOSTIC_FORMATTER_H_

#include <string>

namespace tint::diag {

class Diagnostic;
class List;
class Printer;

/// Formatter are used to print a list of diagnostics messages.
class Formatter {
  public:
    /// Style controls the formatter's output style.
    struct Style {
        /// include the file path for each diagnostic
        bool print_file = true;
        /// include the severity for each diagnostic
        bool print_severity = true;
        /// include the source line(s) for the diagnostic
        bool print_line = true;
        /// print a newline at the end of a diagnostic list
        bool print_newline_at_end = true;
        /// width of a tab character
        size_t tab_width = 2u;
    };

    /// Constructor for the formatter using a default style.
    Formatter();

    /// Constructor for the formatter using the custom style.
    /// @param style the style used for the formatter.
    explicit Formatter(const Style& style);

    ~Formatter();

    /// @param list the list of diagnostic messages to format
    /// @param printer the printer used to display the formatted diagnostics
    void format(const List& list, Printer* printer) const;

    /// @return the list of diagnostics `list` formatted to a string.
    /// @param list the list of diagnostic messages to format
    std::string format(const List& list) const;

  private:
    struct State;

    void format(const Diagnostic& diag, State& state) const;

    const Style style_;
};

}  // namespace tint::diag

#endif  // SRC_TINT_DIAGNOSTIC_FORMATTER_H_
