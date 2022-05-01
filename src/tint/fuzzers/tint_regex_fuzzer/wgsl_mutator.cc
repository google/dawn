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

#include "src/tint/fuzzers/tint_regex_fuzzer/wgsl_mutator.h"

#include <cassert>
#include <cstring>
#include <map>
#include <regex>
#include <string>
#include <utility>
#include <vector>

#include "src/tint/fuzzers/random_generator.h"

namespace tint::fuzzers::regex_fuzzer {

std::vector<size_t> FindDelimiterIndices(const std::string& delimiter,
                                         const std::string& wgsl_code) {
    std::vector<size_t> result;
    for (size_t pos = wgsl_code.find(delimiter, 0); pos != std::string::npos;
         pos = wgsl_code.find(delimiter, pos + 1)) {
        result.push_back(pos);
    }

    return result;
}

std::vector<std::pair<size_t, size_t>> GetIdentifiers(const std::string& wgsl_code) {
    std::vector<std::pair<size_t, size_t>> result;

    // This regular expression works by looking for a character that
    // is not part of an identifier followed by a WGSL identifier, followed
    // by a character which cannot be part of a WGSL identifer. The regex
    // for the WGSL identifier is obtained from:
    // https://www.w3.org/TR/WGSL/#identifiers.
    std::regex wgsl_identifier_regex("[^a-zA-Z]([a-zA-Z][0-9a-zA-Z_]*)[^0-9a-zA-Z_]");

    std::smatch match;

    std::string::const_iterator search_start(wgsl_code.cbegin());
    std::string prefix;

    while (regex_search(search_start, wgsl_code.cend(), match, wgsl_identifier_regex) == true) {
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
        result.push_back(std::make_pair(prefix.size() + 1, match.str(0).size() - 1));
        prefix += match.str(0);
        search_start = match.suffix().first;
    }
    return result;
}

size_t FindClosingBrace(size_t opening_bracket_pos, const std::string& wgsl_code) {
    size_t open_bracket_count = 1;
    size_t pos = opening_bracket_pos + 1;
    while (open_bracket_count >= 1 && pos < wgsl_code.size()) {
        if (wgsl_code[pos] == '{') {
            ++open_bracket_count;
        } else if (wgsl_code[pos] == '}') {
            --open_bracket_count;
        }
        ++pos;
    }
    return (pos == wgsl_code.size() && open_bracket_count >= 1) ? 0 : pos - 1;
}

std::vector<size_t> GetFunctionBodyPositions(const std::string& wgsl_code) {
    // Finds all the functions with a non-void return value.
    std::regex function_regex("fn.*?->.*?\\{");
    std::smatch match;
    std::vector<size_t> result;

    auto search_start(wgsl_code.cbegin());
    std::string prefix = "";

    while (std::regex_search(search_start, wgsl_code.cend(), match, function_regex)) {
        result.push_back(static_cast<size_t>(match.suffix().first - wgsl_code.cbegin() - 1L));
        search_start = match.suffix().first;
    }
    return result;
}

bool InsertReturnStatement(std::string& wgsl_code, RandomGenerator& generator) {
    std::vector<size_t> function_body_positions = GetFunctionBodyPositions(wgsl_code);

    // No function was found in wgsl_code.
    if (function_body_positions.empty()) {
        return false;
    }

    // Pick a random function's opening bracket, find the corresponding closing
    // bracket, and find a semi-colon within the function body.
    size_t left_bracket_pos = generator.GetRandomElement(function_body_positions);

    size_t right_bracket_pos = FindClosingBrace(left_bracket_pos, wgsl_code);

    if (right_bracket_pos == 0) {
        return false;
    }

    std::vector<size_t> semicolon_positions;
    for (size_t pos = wgsl_code.find(";", left_bracket_pos + 1); pos < right_bracket_pos;
         pos = wgsl_code.find(";", pos + 1)) {
        semicolon_positions.push_back(pos);
    }

    if (semicolon_positions.empty()) {
        return false;
    }

    size_t semicolon_position = generator.GetRandomElement(semicolon_positions);

    // Get all identifiers and integer literals to use as potential return values.
    std::vector<std::pair<size_t, size_t>> identifiers = GetIdentifiers(wgsl_code);
    auto return_values = identifiers;
    std::vector<std::pair<size_t, size_t>> int_literals = GetIntLiterals(wgsl_code);
    return_values.insert(return_values.end(), int_literals.begin(), int_literals.end());
    std::pair<size_t, size_t> return_value = generator.GetRandomElement(return_values);
    std::string return_statement =
        "return " + wgsl_code.substr(return_value.first, return_value.second) + ";";

    // Insert the return statement immediately after the semicolon.
    wgsl_code.insert(semicolon_position + 1, return_statement);
    return true;
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

void DuplicateInterval(size_t idx1, size_t reg1_len, size_t idx2, std::string& wgsl_code) {
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
                         RandomGenerator& generator) {
    std::vector<size_t> delimiter_positions = FindDelimiterIndices(delimiter, wgsl_code);

    // Need to have at least 3 indices.
    if (delimiter_positions.size() < 3) {
        return false;
    }

    // Choose indices:
    //   interval_1_start < interval_1_end <= interval_2_start < interval_2_end
    uint32_t interval_1_start =
        generator.GetUInt32(static_cast<uint32_t>(delimiter_positions.size()) - 2u);
    uint32_t interval_1_end = generator.GetUInt32(
        interval_1_start + 1u, static_cast<uint32_t>(delimiter_positions.size()) - 1u);
    uint32_t interval_2_start =
        generator.GetUInt32(interval_1_end, static_cast<uint32_t>(delimiter_positions.size()) - 1u);
    uint32_t interval_2_end = generator.GetUInt32(
        interval_2_start + 1u, static_cast<uint32_t>(delimiter_positions.size()));

    SwapIntervals(delimiter_positions[interval_1_start],
                  delimiter_positions[interval_1_end] - delimiter_positions[interval_1_start],
                  delimiter_positions[interval_2_start],
                  delimiter_positions[interval_2_end] - delimiter_positions[interval_2_start],
                  wgsl_code);

    return true;
}

bool DeleteRandomInterval(const std::string& delimiter,
                          std::string& wgsl_code,
                          RandomGenerator& generator) {
    std::vector<size_t> delimiter_positions = FindDelimiterIndices(delimiter, wgsl_code);

    // Need to have at least 2 indices.
    if (delimiter_positions.size() < 2) {
        return false;
    }

    uint32_t interval_start =
        generator.GetUInt32(static_cast<uint32_t>(delimiter_positions.size()) - 1u);
    uint32_t interval_end =
        generator.GetUInt32(interval_start + 1u, static_cast<uint32_t>(delimiter_positions.size()));

    DeleteInterval(delimiter_positions[interval_start],
                   delimiter_positions[interval_end] - delimiter_positions[interval_start],
                   wgsl_code);

    return true;
}

bool DuplicateRandomInterval(const std::string& delimiter,
                             std::string& wgsl_code,
                             RandomGenerator& generator) {
    std::vector<size_t> delimiter_positions = FindDelimiterIndices(delimiter, wgsl_code);

    // Need to have at least 2 indices
    if (delimiter_positions.size() < 2) {
        return false;
    }

    uint32_t interval_start =
        generator.GetUInt32(static_cast<uint32_t>(delimiter_positions.size()) - 1u);
    uint32_t interval_end =
        generator.GetUInt32(interval_start + 1u, static_cast<uint32_t>(delimiter_positions.size()));
    uint32_t duplication_point =
        generator.GetUInt32(static_cast<uint32_t>(delimiter_positions.size()));

    DuplicateInterval(delimiter_positions[interval_start],
                      delimiter_positions[interval_end] - delimiter_positions[interval_start],
                      delimiter_positions[duplication_point], wgsl_code);

    return true;
}

bool ReplaceRandomIdentifier(std::string& wgsl_code, RandomGenerator& generator) {
    std::vector<std::pair<size_t, size_t>> identifiers = GetIdentifiers(wgsl_code);

    // Need at least 2 identifiers
    if (identifiers.size() < 2) {
        return false;
    }

    uint32_t id1_index = generator.GetUInt32(static_cast<uint32_t>(identifiers.size()));
    uint32_t id2_index = generator.GetUInt32(static_cast<uint32_t>(identifiers.size()));

    // The two identifiers must be different
    while (id1_index == id2_index) {
        id2_index = generator.GetUInt32(static_cast<uint32_t>(identifiers.size()));
    }

    ReplaceRegion(identifiers[id1_index].first, identifiers[id1_index].second,
                  identifiers[id2_index].first, identifiers[id2_index].second, wgsl_code);

    return true;
}

bool ReplaceRandomIntLiteral(std::string& wgsl_code, RandomGenerator& generator) {
    std::vector<std::pair<size_t, size_t>> literals = GetIntLiterals(wgsl_code);

    // Need at least one integer literal
    if (literals.size() < 1) {
        return false;
    }

    uint32_t literal_index = generator.GetUInt32(static_cast<uint32_t>(literals.size()));

    // INT_MAX = 2147483647, INT_MIN = -2147483648
    std::vector<std::string> boundary_values = {"2147483647", "-2147483648", "1",
                                                "-1",         "0",           "4294967295"};

    uint32_t boundary_index = generator.GetUInt32(static_cast<uint32_t>(boundary_values.size()));

    ReplaceInterval(literals[literal_index].first, literals[literal_index].second,
                    boundary_values[boundary_index], wgsl_code);

    return true;
}

}  // namespace tint::fuzzers::regex_fuzzer
