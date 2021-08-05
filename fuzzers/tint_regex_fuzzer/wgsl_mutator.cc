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

std::vector<std::pair<size_t, size_t>> GetIdentifiers(
    const std::string& wgsl_code) {
  std::vector<std::pair<size_t, size_t>> result;

  // This regular expression works by looking for a character that
  // is not part of an identifier followed by a WGSL identifier, followed
  // by a character which cannot be part of a WGSL identifer. The regex
  // for the WGSL identifier is obtained from:
  // https://www.w3.org/TR/WGSL/#identifiers.
  std::regex wgsl_identifier_regex(
      "[^a-zA-Z]([a-zA-Z][0-9a-zA-Z_]*)[^0-9a-zA-Z_]");

  std::smatch match;

  std::string::const_iterator search_start(wgsl_code.cbegin());
  std::string prefix;

  while (regex_search(search_start, wgsl_code.cend(), match,
                      wgsl_identifier_regex) == true) {
    prefix += match.prefix();
    result.push_back(std::make_pair(prefix.size() + 1, match.str(1).size()));
    prefix += match.str(0);
    search_start = match.suffix().first;
  }
  return result;
}

std::vector<std::pair<size_t, size_t>> GetIntLiterals(const std::string& s) {
  std::vector<std::pair<size_t, size_t>> result;

  // Looks for integer literals in decimal or hexadecimal form.
  // Regex obtained here: https://www.w3.org/TR/WGSL/#literals
  std::regex int_literal_regex("-?0x[0-9a-fA-F]+ | 0 | -?[1-9][0-9]*");
  std::regex uint_literal_regex("0x[0-9a-fA-F]+u | 0u | [1-9][0-9]*u");
  std::smatch match;

  std::string::const_iterator search_start(s.cbegin());
  std::string prefix = "";

  while (regex_search(search_start, s.cend(), match, int_literal_regex) ||
         regex_search(search_start, s.cend(), match, uint_literal_regex)) {
    prefix += match.prefix();
    result.push_back(
        std::make_pair(prefix.size() + 1, match.str(0).size() - 1));
    prefix += match.str(0);
    search_start = match.suffix().first;
  }
  return result;
}

void SwapIntervals(size_t idx1,
                   size_t reg1_len,
                   size_t idx2,
                   size_t reg2_len,
                   std::string& wgsl_code) {
  std::string region_1 = wgsl_code.substr(idx1 + 1, reg1_len - 1);

  std::string region_2 = wgsl_code.substr(idx2 + 1, reg2_len - 1);

  // The second transformation is done first as it doesn't affect idx2.
  wgsl_code.replace(idx2 + 1, region_2.size(), region_1);

  wgsl_code.replace(idx1 + 1, region_1.size(), region_2);
}

void DeleteInterval(size_t idx1, size_t reg_len, std::string& wgsl_code) {
  wgsl_code.erase(idx1 + 1, reg_len - 1);
}

void DuplicateInterval(size_t idx1,
                       size_t reg1_len,
                       size_t idx2,
                       std::string& wgsl_code) {
  std::string region = wgsl_code.substr(idx1 + 1, reg1_len - 1);
  wgsl_code.insert(idx2 + 1, region);
}

void ReplaceRegion(size_t idx1,
                   size_t id1_len,
                   size_t idx2,
                   size_t id2_len,
                   std::string& wgsl_code) {
  std::string region_1 = wgsl_code.substr(idx1, id1_len);
  std::string region_2 = wgsl_code.substr(idx2, id2_len);
  wgsl_code.replace(idx2, region_2.size(), region_1);
}

void ReplaceInterval(size_t start_index,
                     size_t length,
                     std::string replacement_text,
                     std::string& wgsl_code) {
  std::string region_1 = wgsl_code.substr(start_index, length);
  wgsl_code.replace(start_index, length, replacement_text);
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

  SwapIntervals(delimiter_positions[ind1],
                delimiter_positions[ind2] - delimiter_positions[ind1],
                delimiter_positions[ind3],
                delimiter_positions[ind4] - delimiter_positions[ind3],
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

  DeleteInterval(delimiter_positions[ind1],
                 delimiter_positions[ind2] - delimiter_positions[ind1],
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

  DuplicateInterval(delimiter_positions[ind1],
                    delimiter_positions[ind2] - delimiter_positions[ind1],
                    delimiter_positions[ind3], wgsl_code);

  return true;
}

bool ReplaceRandomIdentifier(std::string& wgsl_code, std::mt19937& generator) {
  std::vector<std::pair<size_t, size_t>> identifiers =
      GetIdentifiers(wgsl_code);

  // Need at least 2 identifiers
  if (identifiers.size() < 2) {
    return false;
  }

  size_t id1_index =
      GetRandomIntFromRange(0, identifiers.size() - 1U, generator);

  size_t id2_index =
      GetRandomIntFromRange(0, identifiers.size() - 1U, generator);

  // The two identifiers must be different
  while (id1_index == id2_index) {
    id2_index = GetRandomIntFromRange(0, identifiers.size() - 1U, generator);
  }

  ReplaceRegion(identifiers[id1_index].first, identifiers[id1_index].second,
                identifiers[id2_index].first, identifiers[id2_index].second,
                wgsl_code);

  return true;
}

bool ReplaceRandomIntLiteral(std::string& wgsl_code, std::mt19937& generator) {
  std::vector<std::pair<size_t, size_t>> literals = GetIntLiterals(wgsl_code);

  // Need at least one integer literal
  if (literals.size() < 1) {
    return false;
  }

  size_t id1_index = GetRandomIntFromRange(0, literals.size() - 1U, generator);

  // INT_MAX = 2147483647, INT_MIN = -2147483648
  std::vector<std::string> boundary_values = {
      "2147483647", "-2147483648", "1", "-1", "0", "4294967295"};

  size_t boundary_index =
      GetRandomIntFromRange(0, boundary_values.size() - 1U, generator);

  ReplaceInterval(literals[id1_index].first, literals[id1_index].second,
                  boundary_values[boundary_index], wgsl_code);

  return true;
}

}  // namespace regex_fuzzer
}  // namespace fuzzers
}  // namespace tint
