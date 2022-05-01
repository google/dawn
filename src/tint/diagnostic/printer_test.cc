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

#include "src/tint/diagnostic/printer.h"

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
