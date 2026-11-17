// Copyright 2026 The Dawn & Tint Authors
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

#include <string>

#include "src/tint/utils/macros/compiler.h"

TINT_BEGIN_DISABLE_ALL_WARNINGS();
#include "fuzztest/fuzztest.h"
#include "fuzztest/init_fuzztest.h"
#include "gtest/gtest.h"
#include "testing/libfuzzer/fuzzers/tint_wgsl_fuzzer_grammar.h"
TINT_END_DISABLE_ALL_WARNINGS();

namespace tint::fuzz {
namespace {

void RawString(const std::string& input) {
    if (input.size() > 1024) {
        return;
    }
}

FUZZ_TEST(StubFuzzTest, RawString).WithDomains(fuzztest::AsciiString().WithMaxSize(1024));

void WgslGrammar(const std::string& wgsl) {
    if (wgsl.empty()) {
        return;
    }
}

FUZZ_TEST(StubFuzzTest, WgslGrammar).WithDomains(fuzztest::InWgslGrammar());

}  // namespace
}  // namespace tint::fuzz

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    fuzztest::ParseAbslFlags(argc, argv);
    fuzztest::InitFuzzTest(&argc, &argv);
    return RUN_ALL_TESTS();
}
