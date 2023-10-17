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

#include "src/tint/utils/diagnostic/printer.h"

#include "gtest/gtest.h"

namespace tint::diag {
namespace {

// Actually verifying that the expected colors are printed is exceptionally
// difficult as:
// a) The color emission varies by OS.
// b) The logic checks to see if the printer is writing to a terminal, making
//    mocking hard.
// c) Actually probing what gets written to a FILE* is notoriously tricky.
//
// The least we can do is to exersice the code - which is what we do here.
// The test will print each of the colors, and can be examined with human
// eyeballs.
// This can be enabled or disabled with ENABLE_PRINTER_TESTS
#define ENABLE_PRINTER_TESTS 0
#if ENABLE_PRINTER_TESTS

using PrinterTest = testing::Test;

TEST_F(PrinterTest, WithColors) {
    auto printer = Printer::create(stdout, true);
    printer->write("Default", Style{Color::kDefault, false});
    printer->write("Black", Style{Color::kBlack, false});
    printer->write("Red", Style{Color::kRed, false});
    printer->write("Green", Style{Color::kGreen, false});
    printer->write("Yellow", Style{Color::kYellow, false});
    printer->write("Blue", Style{Color::kBlue, false});
    printer->write("Magenta", Style{Color::kMagenta, false});
    printer->write("Cyan", Style{Color::kCyan, false});
    printer->write("White", Style{Color::kWhite, false});
    printf("\n");
}

TEST_F(PrinterTest, BoldWithColors) {
    auto printer = Printer::create(stdout, true);
    printer->write("Default", Style{Color::kDefault, true});
    printer->write("Black", Style{Color::kBlack, true});
    printer->write("Red", Style{Color::kRed, true});
    printer->write("Green", Style{Color::kGreen, true});
    printer->write("Yellow", Style{Color::kYellow, true});
    printer->write("Blue", Style{Color::kBlue, true});
    printer->write("Magenta", Style{Color::kMagenta, true});
    printer->write("Cyan", Style{Color::kCyan, true});
    printer->write("White", Style{Color::kWhite, true});
    printf("\n");
}

TEST_F(PrinterTest, WithoutColors) {
    auto printer = Printer::create(stdout, false);
    printer->write("Default", Style{Color::kDefault, false});
    printer->write("Black", Style{Color::kBlack, false});
    printer->write("Red", Style{Color::kRed, false});
    printer->write("Green", Style{Color::kGreen, false});
    printer->write("Yellow", Style{Color::kYellow, false});
    printer->write("Blue", Style{Color::kBlue, false});
    printer->write("Magenta", Style{Color::kMagenta, false});
    printer->write("Cyan", Style{Color::kCyan, false});
    printer->write("White", Style{Color::kWhite, false});
    printf("\n");
}

TEST_F(PrinterTest, BoldWithoutColors) {
    auto printer = Printer::create(stdout, false);
    printer->write("Default", Style{Color::kDefault, true});
    printer->write("Black", Style{Color::kBlack, true});
    printer->write("Red", Style{Color::kRed, true});
    printer->write("Green", Style{Color::kGreen, true});
    printer->write("Yellow", Style{Color::kYellow, true});
    printer->write("Blue", Style{Color::kBlue, true});
    printer->write("Magenta", Style{Color::kMagenta, true});
    printer->write("Cyan", Style{Color::kCyan, true});
    printer->write("White", Style{Color::kWhite, true});
    printf("\n");
}

#endif  // ENABLE_PRINTER_TESTS
}  // namespace
}  // namespace tint::diag
