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

#ifndef FUZZERS_TINT_REGEX_FUZZER_WGSL_MUTATOR_H_
#define FUZZERS_TINT_REGEX_FUZZER_WGSL_MUTATOR_H_

#include <random>
#include <string>
#include <utility>
#include <vector>

namespace tint {
namespace fuzzers {
namespace regex_fuzzer {

/// A function that given a delimiter, returns a vector that contains
/// all the positions of the delimiter in the WGSL code.
/// @param delimiter - the delimiter of the enclosed region.
/// @param wgsl_code - the initial string (WGSL code) that will be mutated.
/// @return a vector with the positions of the delimiter in the WGSL code.
std::vector<size_t> FindDelimiterIndices(const std::string& delimiter,
                                         const std::string& wgsl_code);

/// Given 4 indices, idx1, idx2, idx3 and idx4 it swaps the regions
/// in the interval [idx1, idx2] with the region in the interval [idx3, idx4]
/// in wgsl_text.
/// @param idx1 - starting index of the first region.
/// @param idx2 - terminating index of the second region.
/// @param idx3 - starting index of the second region.
/// @param idx4 - terminating index of the second region.
/// @param wgsl_code - the string where the swap will occur.
void SwapIntervals(size_t idx1,
                   size_t idx2,
                   size_t idx3,
                   size_t idx4,
                   std::string* wgsl_code);

/// A function that, given an initial string (valid WGSL code) and a delimiter,
/// generates a new set of strings (valid or invalid WGSL code) by
/// picking two random regions and swapping them.
/// @param wgsl_code - the initial string (WGSL code) that will be mutated.
/// @param size - size of the string that will be mutated.
/// @param max_size - maximal allowed mutation size.
/// @param delimiter - the delimiter that will be used to find enclosed regions.
/// @param generator - the random number generator.
/// @return size of the mutated string.
size_t FuzzEnclosedRegions(size_t size,
                           size_t max_size,
                           const std::string& delimiter,
                           uint8_t* wgsl_code,
                           std::mt19937* generator);

}  // namespace regex_fuzzer
}  // namespace fuzzers
}  // namespace tint

#endif  // FUZZERS_TINT_REGEX_FUZZER_WGSL_MUTATOR_H_
