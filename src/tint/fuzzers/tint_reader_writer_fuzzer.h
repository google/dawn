// Copyright 2021 The Dawn & Tint Authors
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

#ifndef SRC_TINT_FUZZERS_TINT_READER_WRITER_FUZZER_H_
#define SRC_TINT_FUZZERS_TINT_READER_WRITER_FUZZER_H_

#include <memory>

#include "src/tint/fuzzers/tint_common_fuzzer.h"
#include "src/tint/fuzzers/transform_builder.h"

namespace tint::fuzzers {

/// Wrapper around the common fuzzing class for tint_*_reader_*_writter fuzzers
class ReaderWriterFuzzer : public CommonFuzzer {
  public:
    /// Constructor
    /// Pass through to the CommonFuzzer constructor
    /// @param input shader language being read
    /// @param output shader language being emitted
    ReaderWriterFuzzer(InputFormat input, OutputFormat output) : CommonFuzzer(input, output) {}

    /// Destructor
    ~ReaderWriterFuzzer() {}

    /// Pass through to the CommonFuzzer setter, but records if it has been
    /// invoked.
    /// @param tm manager for transforms to run
    /// @param inputs data for transforms to run
    void SetTransformManager(ast::transform::Manager* tm, ast::transform::DataMap* inputs) {
        tm_set_ = true;
        CommonFuzzer::SetTransformManager(tm, inputs);
    }

    /// Pass through to the CommonFuzzer implementation.
    /// @param data buffer of data that will interpreted as a byte array or string depending on the
    /// shader input format.
    /// @param size number of elements in buffer
    /// @returns 0, this is what libFuzzer expects
    int Run(const uint8_t* data, size_t size) {
        if (!tm_set_) {
            tb_ = std::make_unique<TransformBuilder>(data, size);
            SetTransformManager(tb_->manager(), tb_->data_map());
        }

        return CommonFuzzer::Run(data, size);
    }

  private:
    bool tm_set_ = false;
    std::unique_ptr<TransformBuilder> tb_;
};

}  // namespace tint::fuzzers

#endif  // SRC_TINT_FUZZERS_TINT_READER_WRITER_FUZZER_H_
