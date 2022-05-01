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

#ifndef SRC_TINT_FUZZERS_TINT_REGEX_FUZZER_WGSL_MUTATOR_H_
#define SRC_TINT_FUZZERS_TINT_REGEX_FUZZER_WGSL_MUTATOR_H_

#include <string>
#include <utility>
#include <vector>

#include "src/tint/fuzzers/random_generator.h"

namespace tint::fuzzers::regex_fuzzer {

/// A function that given a delimiter, returns a vector that contains
/// all the positions of the delimiter in the WGSL code.
/// @param delimiter - the delimiter of the enclosed region.
/// @param wgsl_code - the initial string (WGSL code) that will be mutated.
/// @return a vector with the positions of the delimiter in the WGSL code.
std::vector<size_t> FindDelimiterIndices(const std::string& delimiter,
                                         const std::string& wgsl_code);

/// A function that finds all the identifiers in a WGSL-like string.
/// @param wgsl_code - the WGSL-like string where the identifiers will be found.
/// @return a vector with the positions and the length of all the
/// identifiers in wgsl_code.
std::vector<std::pair<size_t, size_t>> GetIdentifiers(const std::string& wgsl_code);

/// A function that returns returns the starting position
/// and the length of all the integer literals in a WGSL-like string.
/// @param wgsl_code - the WGSL-like string where the int literals
/// will be found.
/// @return a vector with the starting positions and the length
/// of all the integer literals.
std::vector<std::pair<size_t, size_t>> GetIntLiterals(const std::string& wgsl_code);

/// Finds a possible closing brace corresponding to the opening
/// brace at position opening_bracket_pos.
/// @param opening_bracket_pos - the position of the opening brace.
/// @param wgsl_code - the WGSL-like string where the closing brace.
/// @return the position of the closing bracket or 0 if there is no closing
/// brace.
size_t FindClosingBrace(size_t opening_bracket_pos, const std::string& wgsl_code);

/// Returns the starting_position of the bodies of the functions
/// that follow the regular expression: fn.*?->.*?\\{, which searches for the
/// keyword fn followed by the function name, its return type and opening brace.
/// @param wgsl_code - the WGSL-like string where the functions will be
/// searched.
/// @return a vector with the starting position of the function bodies in
/// wgsl_code.
std::vector<size_t> GetFunctionBodyPositions(const std::string& wgsl_code);

/// Given 4 indices, idx1, idx2, idx3 and idx4 it swaps the regions
/// in the interval (idx1, idx2] with the region in the interval (idx3, idx4]
/// in wgsl_text.
/// @param idx1 - starting index of the first region.
/// @param reg1_len - length of the first region.
/// @param idx2 - starting index of the second region.
/// @param reg2_len - length of the second region.
/// @param wgsl_code - the string where the swap will occur.
void SwapIntervals(size_t idx1,
                   size_t reg1_len,
                   size_t idx2,
                   size_t reg2_len,
                   std::string& wgsl_code);

/// Given index idx1 it delets the region of length interval_len
/// starting at index idx1;
/// @param idx1 - starting index of the first region.
/// @param reg_len - terminating index of the second region.
/// @param wgsl_code - the string where the swap will occur.
void DeleteInterval(size_t idx1, size_t reg_len, std::string& wgsl_code);

/// Given 2 indices, idx1, idx2, it inserts the region of length
/// reg1_len starting at idx1 after idx2.
/// @param idx1 - starting index of region.
/// @param reg1_len - length of the region.
/// @param idx2 - the position where the region will be inserted.
/// @param wgsl_code - the string where the swap will occur.
void DuplicateInterval(size_t idx1, size_t reg1_len, size_t idx2, std::string& wgsl_code);

/// Replaces a region of a WGSL-like string of length id2_len starting
/// at position idx2 with a region of length id1_len starting at
/// position idx1.
/// @param idx1 - starting position of the first region.
/// @param id1_len - length of the first region.
/// @param idx2 - starting position of the second region.
/// @param id2_len - length of the second region.
/// @param wgsl_code - the string where the replacement will occur.
void ReplaceRegion(size_t idx1,
                   size_t id1_len,
                   size_t idx2,
                   size_t id2_len,
                   std::string& wgsl_code);

/// Replaces an interval of length `length` starting at start_index
/// with the `replacement_text`.
/// @param start_index - starting position of the interval to be replaced.
/// @param length - length of the interval to be replaced.
/// @param replacement_text - the interval that will be used as a replacement.
/// @param wgsl_code - the WGSL-like string where the replacement will occur.
void ReplaceInterval(size_t start_index,
                     size_t length,
                     std::string replacement_text,
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
                         RandomGenerator& generator);

/// A function that, given a WGSL-like string and a delimiter,
/// generates another WGSL-like string by deleting a random
/// region enclosed by the delimiter.
/// @param delimiter - the delimiter that will be used to find enclosed regions.
/// @param wgsl_code - the initial string (WGSL code) that will be mutated.
/// @param generator - the random number generator.
/// @return true if a deletion happened or false otherwise.
bool DeleteRandomInterval(const std::string& delimiter,
                          std::string& wgsl_code,
                          RandomGenerator& generator);

/// A function that, given a WGSL-like string and a delimiter,
/// generates another WGSL-like string by duplicating a random
/// region enclosed by the delimiter.
/// @param delimiter - the delimiter that will be used to find enclosed regions.
/// @param wgsl_code - the initial string (WGSL code) that will be mutated.
/// @param generator - the random number generator.
/// @return true if a duplication happened or false otherwise.
bool DuplicateRandomInterval(const std::string& delimiter,
                             std::string& wgsl_code,
                             RandomGenerator& generator);

/// Replaces a randomly-chosen identifier in wgsl_code.
/// @param wgsl_code - WGSL-like string where the replacement will occur.
/// @param generator - the random number generator.
/// @return true if a replacement happened or false otherwise.
bool ReplaceRandomIdentifier(std::string& wgsl_code, RandomGenerator& generator);

/// Replaces the value of a randomly-chosen integer with one of
/// the values in the set {INT_MAX, INT_MIN, 0, -1}.
/// @param wgsl_code - WGSL-like string where the replacement will occur.
/// @param generator - the random number generator.
/// @return true if a replacement happened or false otherwise.
bool ReplaceRandomIntLiteral(std::string& wgsl_code, RandomGenerator& generator);

/// Inserts a return statement in a randomly chosen function of a
/// WGSL-like string. The return value is a randomly-chosen identifier
/// or literal in the string.
/// @param wgsl_code - WGSL-like string that will be mutated.
/// @param generator - the random number generator.
/// @return true if the mutation was succesful or false otherwise.
bool InsertReturnStatement(std::string& wgsl_code, RandomGenerator& generator);
}  // namespace tint::fuzzers::regex_fuzzer

#endif  // SRC_TINT_FUZZERS_TINT_REGEX_FUZZER_WGSL_MUTATOR_H_
