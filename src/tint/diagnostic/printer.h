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

#ifndef SRC_TINT_DIAGNOSTIC_PRINTER_H_
#define SRC_TINT_DIAGNOSTIC_PRINTER_H_

#include <memory>
#include <string>

#include "src/tint/utils/string_stream.h"

namespace tint::diag {

class List;

/// Color is an enumerator of colors used by Style.
enum class Color {
    kDefault,
    kBlack,
    kRed,
    kGreen,
    kYellow,
    kBlue,
    kMagenta,
    kCyan,
    kWhite,
};

/// Style describes how a diagnostic message should be printed.
struct Style {
    /// The foreground text color
    Color color = Color::kDefault;
    /// If true the text will be displayed with a strong weight
    bool bold = false;
};

/// Printers are used to print formatted diagnostic messages to a terminal.
class Printer {
  public:
    /// @returns a diagnostic Printer
    /// @param out the file to print to.
    /// @param use_colors if true, the printer will use colors if `out` is a
    /// terminal and supports them.
    static std::unique_ptr<Printer> create(FILE* out, bool use_colors);

    virtual ~Printer();

    /// writes the string str to the printer with the given style.
    /// @param str the string to write to the printer
    /// @param style the style used to print `str`
    virtual void write(const std::string& str, const Style& style) = 0;
};

/// StringPrinter is an implementation of Printer that writes to a std::string.
class StringPrinter : public Printer {
  public:
    StringPrinter();
    ~StringPrinter() override;

    /// @returns the printed string.
    std::string str() const;

    void write(const std::string& str, const Style&) override;

  private:
    utils::StringStream stream;
};

}  // namespace tint::diag

#endif  // SRC_TINT_DIAGNOSTIC_PRINTER_H_
