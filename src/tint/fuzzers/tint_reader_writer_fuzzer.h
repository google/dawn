// Copyright 2021 The Tint Authors.
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
    void SetTransformManager(transform::Manager* tm, transform::DataMap* inputs) {
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
