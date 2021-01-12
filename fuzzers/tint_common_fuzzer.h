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

#ifndef FUZZERS_TINT_COMMON_FUZZER_H_
#define FUZZERS_TINT_COMMON_FUZZER_H_

#include "include/tint/tint.h"

namespace tint {
namespace fuzzers {

enum class InputFormat { kWGSL, kSpv, kNone };

enum class OutputFormat { kWGSL, kSpv, kHLSL, kMSL, kNone };

class CommonFuzzer {
 public:
  explicit CommonFuzzer(InputFormat input, OutputFormat output);
  ~CommonFuzzer();

  void SetTransformManager(transform::Manager* tm) { transform_manager_ = tm; }

  int Run(const uint8_t* data, size_t size);

 private:
  InputFormat input_;
  OutputFormat output_;
  transform::Manager* transform_manager_;
};

}  // namespace fuzzers
}  // namespace tint

#endif  // FUZZERS_TINT_COMMON_FUZZER_H_
