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

#include "fuzzers/tint_regex_fuzzer/wgsl_mutator.h"

#include <cassert>
#include <cstring>
#include <map>
#include <random>
#include <regex>
#include <string>
#include <utility>
#include <vector>

#include "fuzzers/tint_regex_fuzzer/util.h"

namespace tint {
namespace fuzzers {
namespace regex_fuzzer {

std::vector<size_t> FindDelimiterIndices(const std::string& delimiter,
                                         const std::string& wgsl_code) {
  std::vector<size_t> result;
  for (size_t pos = wgsl_code.find(delimiter, 0); pos != std::string::npos;
       pos = wgsl_code.find(delimiter, pos + 1)) {
    result.push_back(pos);
  }

  return result;
}

void SwapIntervals(size_t idx1,
                   size_t idx2,
                   size_t idx3,
                   size_t idx4,
                   std::string& wgsl_code) {
  std::string region_1 = wgsl_code.substr(idx1 + 1, idx2 - idx1);

  std::string region_2 = wgsl_code.substr(idx3 + 1, idx4 - idx3);

  // The second transformation is done first as it doesn't affect ind1 and ind2
  wgsl_code.replace(idx3 + 1, region_2.size(), region_1);

  wgsl_code.replace(idx1 + 1, region_1.size(), region_2);
}

void DeleteInterval(size_t idx1, size_t idx2, std::string& wgsl_code) {
  wgsl_code.erase(idx1 + 1, idx2 - idx1);
}

void DuplicateInterval(size_t idx1,
                       size_t idx2,
                       size_t idx3,
                       std::string& wgsl_code) {
  std::string region = wgsl_code.substr(idx1 + 1, idx2 - idx1);
  wgsl_code.insert(idx3 + 1, region);
}

bool SwapRandomIntervals(const std::string& delimiter,
                         std::string& wgsl_code,
                         std::mt19937& generator) {
  std::vector<size_t> delimiter_positions =
      FindDelimiterIndices(delimiter, wgsl_code);

  // Need to have at least 3 indices
  if (delimiter_positions.size() < 3) {
    return false;
  }

  // When generating the i-th random number, we should make sure that there are
  // at least (3-i) numbers greater than this number.
  size_t ind1 =
      GetRandomIntFromRange(0, delimiter_positions.size() - 3U, generator);
  size_t ind2 = GetRandomIntFromRange(
      ind1 + 1U, delimiter_positions.size() - 2U, generator);
  size_t ind3 =
      GetRandomIntFromRange(ind2, delimiter_positions.size() - 2U, generator);
  size_t ind4 = GetRandomIntFromRange(
      ind3 + 1U, delimiter_positions.size() - 1U, generator);

  SwapIntervals(delimiter_positions[ind1], delimiter_positions[ind2],
                delimiter_positions[ind3], delimiter_positions[ind4],
                wgsl_code);

  return true;
}

bool DeleteRandomInterval(const std::string& delimiter,
                          std::string& wgsl_code,
                          std::mt19937& generator) {
  std::vector<size_t> delimiter_positions =
      FindDelimiterIndices(delimiter, wgsl_code);

  // Need to have at least 2 indices
  if (delimiter_positions.size() < 2) {
    return false;
  }

  size_t ind1 =
      GetRandomIntFromRange(0, delimiter_positions.size() - 2U, generator);
  size_t ind2 = GetRandomIntFromRange(
      ind1 + 1U, delimiter_positions.size() - 1U, generator);

  DeleteInterval(delimiter_positions[ind1], delimiter_positions[ind2],
                 wgsl_code);

  return true;
}

bool DuplicateRandomInterval(const std::string& delimiter,
                             std::string& wgsl_code,
                             std::mt19937& generator) {
  std::vector<size_t> delimiter_positions =
      FindDelimiterIndices(delimiter, wgsl_code);

  // Need to have at least 2 indices
  if (delimiter_positions.size() < 2) {
    return false;
  }

  size_t ind1 =
      GetRandomIntFromRange(0, delimiter_positions.size() - 2U, generator);
  size_t ind2 = GetRandomIntFromRange(
      ind1 + 1U, delimiter_positions.size() - 1U, generator);

  size_t ind3 =
      GetRandomIntFromRange(0, delimiter_positions.size() - 1U, generator);

  DuplicateInterval(delimiter_positions[ind1], delimiter_positions[ind2],
                    delimiter_positions[ind3], wgsl_code);

  return true;
}

}  // namespace regex_fuzzer
}  // namespace fuzzers
}  // namespace tint
