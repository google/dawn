// Copyright 2020 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef SRC_TINT_UTILS_DIAGNOSTIC_PRINTER_H_
#define SRC_TINT_UTILS_DIAGNOSTIC_PRINTER_H_

#include <memory>
#include <sstream>
#include <string>

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
    std::stringstream stream;
};

}  // namespace tint::diag

#endif  // SRC_TINT_UTILS_DIAGNOSTIC_PRINTER_H_
