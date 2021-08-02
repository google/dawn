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

#ifndef FUZZERS_TINT_REGEX_FUZZER_UTIL_H_
#define FUZZERS_TINT_REGEX_FUZZER_UTIL_H_

#include <random>

namespace tint {
namespace fuzzers {
namespace regex_fuzzer {

inline size_t GetRandomIntFromRange(size_t lower_bound,
                                    size_t upper_bound,
                                    std::mt19937& generator) {
  std::uniform_int_distribution<size_t> dist(lower_bound, upper_bound);
  return dist(generator);
}
}  // namespace regex_fuzzer
}  // namespace fuzzers
}  // namespace tint
#endif  // FUZZERS_TINT_REGEX_FUZZER_UTIL_H_
