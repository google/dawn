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

#include "src/tint/cmd/fuzz/common/init.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <string>
#include <utility>
#include <vector>

namespace tint::fuzz::common {
namespace {

class ParseFuzzerOptionsTest : public testing::Test {
  protected:
    int Parse(FuzzerType type, std::vector<std::string> args) {
        storage_ = std::move(args);
        argv_vec_.clear();
        for (auto& s : storage_) {
            argv_vec_.push_back(s.data());
        }
        argv_vec_.push_back(nullptr);

        int argc = static_cast<int>(storage_.size());
        char** argv = argv_vec_.data();
        int ret = ParseFuzzerOptions(type, &argc, &argv, &options_);

        remaining_args_.clear();
        for (int i = 0; i < argc; i++) {
            remaining_args_.emplace_back(argv_vec_[static_cast<size_t>(i)]);
        }
        return ret;
    }

    Options options_;
    std::vector<std::string> remaining_args_;

  private:
    std::vector<std::string> storage_;
    std::vector<char*> argv_vec_;
};

TEST_F(ParseFuzzerOptionsTest, StripsEqualsValueOption) {
    EXPECT_EQ(Parse(FuzzerType::kWGSL, {"fuzzer_bin", "--filter=hlsl", "input.wgsl"}), 0);
    EXPECT_EQ(options_.filter, "hlsl");
    EXPECT_THAT(remaining_args_, testing::ElementsAre("fuzzer_bin", "input.wgsl"));
}

TEST_F(ParseFuzzerOptionsTest, StripsSpaceSeparatedOption) {
    EXPECT_EQ(Parse(FuzzerType::kWGSL, {"fuzzer_bin", "--filter", "msl", "input.wgsl"}), 0);
    EXPECT_EQ(options_.filter, "msl");
    EXPECT_THAT(remaining_args_, testing::ElementsAre("fuzzer_bin", "input.wgsl"));
}

TEST_F(ParseFuzzerOptionsTest, StripsBoolFlag) {
    EXPECT_EQ(Parse(FuzzerType::kWGSL, {"fuzzer_bin", "--verbose", "--dump", "input.wgsl"}), 0);
    EXPECT_TRUE(options_.verbose);
    EXPECT_TRUE(options_.dump);
    EXPECT_THAT(remaining_args_, testing::ElementsAre("fuzzer_bin", "input.wgsl"));
}

TEST_F(ParseFuzzerOptionsTest, PreservesUnknownEngineFlags) {
    EXPECT_EQ(Parse(FuzzerType::kWGSL,
                    {"fuzzer_bin", "-max_len=1000", "--verbose", "-dict=foo.dict", "corpus/"}),
              0);
    EXPECT_TRUE(options_.verbose);
    EXPECT_THAT(remaining_args_,
                testing::ElementsAre("fuzzer_bin", "-max_len=1000", "-dict=foo.dict", "corpus/"));
}

TEST_F(ParseFuzzerOptionsTest, PreservesPositionalArgsWhenNoTintFlags) {
    EXPECT_EQ(Parse(FuzzerType::kWGSL, {"fuzzer_bin", "a.wgsl", "b.wgsl"}), 0);
    EXPECT_THAT(remaining_args_, testing::ElementsAre("fuzzer_bin", "a.wgsl", "b.wgsl"));
}

TEST_F(ParseFuzzerOptionsTest, StripsIrOnlyStripInvalidIdentifiersFlag) {
    EXPECT_EQ(Parse(FuzzerType::kIR, {"fuzzer_bin", "--strip-invalid-identifiers", "input.tirb"}),
              0);
    EXPECT_TRUE(options_.strip_invalid_identifiers);
    EXPECT_THAT(remaining_args_, testing::ElementsAre("fuzzer_bin", "input.tirb"));
}

TEST_F(ParseFuzzerOptionsTest, HelpFlagHandledForActiveEngine) {
    EXPECT_EQ(Parse(FuzzerType::kWGSL, {"fuzzer_bin", "--help", "input.wgsl"}), 0);
#if !defined(TINT_FUZZER_USE_LIBFUZZER) || TINT_FUZZER_USE_LIBFUZZER
    EXPECT_THAT(remaining_args_, testing::ElementsAre("fuzzer_bin", "-help=1"));
#else
    EXPECT_THAT(remaining_args_, testing::ElementsAre("fuzzer_bin"));
#endif
}

}  // namespace
}  // namespace tint::fuzz::common
