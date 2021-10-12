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

#ifndef FUZZERS_TINT_READER_WRITER_FUZZER_H_
#define FUZZERS_TINT_READER_WRITER_FUZZER_H_

#include <memory>

#include "fuzzers/tint_common_fuzzer.h"
#include "fuzzers/transform_builder.h"

namespace tint {
namespace fuzzers {

class ReaderWriterFuzzer : public CommonFuzzer {
 public:
  explicit ReaderWriterFuzzer(InputFormat input, OutputFormat output)
      : CommonFuzzer(input, output) {}
  ~ReaderWriterFuzzer() {}

  void SetTransformManager(transform::Manager* tm, transform::DataMap* inputs) {
    tm_set_ = true;
    CommonFuzzer::SetTransformManager(tm, inputs);
  }

  int Run(const uint8_t* data, size_t size) {
    if (!tm_set_) {
      tb_ = std::make_unique<TransformBuilder>(data, size);
      tb_->AddTransform<tint::transform::Robustness>();
      SetTransformManager(tb_->manager(), tb_->data_map());
    }

    return CommonFuzzer::Run(data, size);
  }

 private:
  bool tm_set_ = false;
  std::unique_ptr<TransformBuilder> tb_;
};

}  // namespace fuzzers
}  // namespace tint

#endif  // FUZZERS_TINT_READER_WRITER_FUZZER_H_
