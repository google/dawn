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
                   std::string& wgsl_code);

/// Given 2 indices, idx1, idx2, it delets the region in the interval [idx1,
/// idx2].
/// @param idx1 - starting index of the first region.
/// @param idx2 - terminating index of the second region.
/// @param wgsl_code - the string where the swap will occur.
void DeleteInterval(size_t idx1, size_t idx2, std::string& wgsl_code);

/// Given 3 indices, idx1, idx2, and idx3 it inserts the
/// region in [idx1, idx2] after idx3.
/// @param idx1 - starting index of region.
/// @param idx2 - terminating index of the region.
/// @param idx3 - the position where the region will be inserted.
/// @param wgsl_code - the string where the swap will occur.
void DuplicateInterval(size_t idx1,
                       size_t idx2,
                       size_t idx3,
                       std::string& wgsl_code);

/// A function that, given WGSL-like string and a delimiter,
/// generates another WGSL-like string by picking two random regions
/// enclosed by the delimiter and swapping them.
/// @param delimiter - the delimiter that will be used to find enclosed regions.
/// @param wgsl_code - the initial string (WGSL code) that will be mutated.
/// @param generator - the random number generator.
/// @return true if a swap happened or false otherwise.
bool SwapRandomIntervals(const std::string& delimiter,
                         std::string& wgsl_code,
                         std::mt19937& generator);

/// A function that, given a WGSL-like string and a delimiter,
/// generates another WGSL-like string by deleting a random
/// region enclosed by the delimiter.
/// @param delimiter - the delimiter that will be used to find enclosed regions.
/// @param wgsl_code - the initial string (WGSL code) that will be mutated.
/// @param generator - the random number generator.
/// @return true if a deletion happened or false otherwise.
bool DeleteRandomInterval(const std::string& delimiter,
                          std::string& wgsl_code,
                          std::mt19937& generator);

/// A function that, given a WGSL-like string and a delimiter,
/// generates another WGSL-like string by duplicating a random
/// region enclosed by the delimiter.
/// @param delimiter - the delimiter that will be used to find enclosed regions.
/// @param wgsl_code - the initial string (WGSL code) that will be mutated.
/// @param generator - the random number generator.
/// @return true if a duplication happened or false otherwise.
bool DuplicateRandomInterval(const std::string& delimiter,
                             std::string& wgsl_code,
                             std::mt19937& generator);

}  // namespace regex_fuzzer
}  // namespace fuzzers
}  // namespace tint

#endif  // FUZZERS_TINT_REGEX_FUZZER_WGSL_MUTATOR_H_
