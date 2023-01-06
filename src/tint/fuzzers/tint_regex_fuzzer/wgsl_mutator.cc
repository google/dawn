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
#include <unordered_set>
#include <utility>
#include <vector>

#include "src/tint/fuzzers/random_generator.h"

namespace tint::fuzzers::regex_fuzzer {

WgslMutator::WgslMutator(RandomGenerator& generator) : generator_(generator) {}

std::vector<size_t> WgslMutator::FindDelimiterIndices(const std::string& delimiter,
                                                      const std::string& wgsl_code) {
    std::vector<size_t> result;
    for (size_t pos = wgsl_code.find(delimiter, 0); pos != std::string::npos;
         pos = wgsl_code.find(delimiter, pos + 1)) {
        result.push_back(pos);
    }

    return result;
}

std::unordered_set<std::string> WgslMutator::GetCommonKeywords() {
    return {"array",  "bool", "break", "compute", "continue", "f32",  "fn",     "fragment",
            "i32",    "if",   "for",   "let",     "location", "loop", "ptr",    "return",
            "struct", "u32",  "var",   "vec2",    "vec3",     "vec4", "vertex", "while"};
}

std::vector<std::pair<size_t, size_t>> WgslMutator::GetIdentifiers(const std::string& wgsl_code) {
    std::vector<std::pair<size_t, size_t>> result;

    // To reduce the rate that invalid programs are produced, common keywords will be excluded from
    // the identifiers that are returned.
    std::unordered_set<std::string> common_keywords = GetCommonKeywords();

    // This regular expression works by looking for a character that
    // is not part of an identifier followed by a WGSL identifier, followed
    // by a character which cannot be part of a WGSL identifer. The regex
    // for the WGSL identifier is obtained from:
    // https://www.w3.org/TR/WGSL/#identifiers.
    std::regex identifier_regex("[_a-zA-Z][0-9a-zA-Z_]*");

    auto identifiers_begin =
        std::sregex_iterator(wgsl_code.begin(), wgsl_code.end(), identifier_regex);
    auto identifiers_end = std::sregex_iterator();

    for (std::sregex_iterator i = identifiers_begin; i != identifiers_end; ++i) {
        if (common_keywords.count(i->str()) > 0) {
            // This is a common keyword, so skip it.
            continue;
        }
        result.push_back(
            {static_cast<size_t>(i->prefix().second - wgsl_code.cbegin()), i->str().size()});
    }
    return result;
}

std::vector<std::pair<size_t, size_t>> WgslMutator::GetFunctionCallIdentifiers(
    const std::string& wgsl_code) {
    std::vector<std::pair<size_t, size_t>> result;

    std::regex call_regex("([_a-zA-Z][0-9a-zA-Z_]*)[ \\n]*\\(");

    auto identifiers_begin = std::sregex_iterator(wgsl_code.begin(), wgsl_code.end(), call_regex);
    auto identifiers_end = std::sregex_iterator();

    for (std::sregex_iterator i = identifiers_begin; i != identifiers_end; ++i) {
        auto submatch = (*i)[1];
        result.push_back(
            {static_cast<size_t>(submatch.first - wgsl_code.cbegin()), submatch.str().size()});
    }
    return result;
}

std::vector<std::pair<size_t, size_t>> WgslMutator::GetIntLiterals(const std::string& s) {
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

size_t WgslMutator::FindClosingBracket(size_t opening_bracket_pos,
                                       const std::string& wgsl_code,
                                       char opening_bracket_character,
                                       char closing_bracket_character) {
    size_t open_bracket_count = 1;
    size_t pos = opening_bracket_pos + 1;
    while (open_bracket_count >= 1 && pos < wgsl_code.size()) {
        if (wgsl_code[pos] == opening_bracket_character) {
            ++open_bracket_count;
        } else if (wgsl_code[pos] == closing_bracket_character) {
            --open_bracket_count;
        }
        ++pos;
    }
    return (pos == wgsl_code.size() && open_bracket_count >= 1) ? 0 : pos - 1;
}

std::vector<std::pair<size_t, bool>> WgslMutator::GetFunctionBodyPositions(
    const std::string& wgsl_code) {
    // Finds all the functions with a non-void return value.
    std::regex function_regex("fn[^a-zA-Z_0-9][^\\{]*\\{");
    std::vector<std::pair<size_t, bool>> result;

    auto functions_begin = std::sregex_iterator(wgsl_code.begin(), wgsl_code.end(), function_regex);
    auto functions_end = std::sregex_iterator();

    for (std::sregex_iterator i = functions_begin; i != functions_end; ++i) {
        bool returns_value = i->str().find("->") != std::string::npos;
        result.push_back(
            {static_cast<size_t>(i->suffix().first - wgsl_code.cbegin() - 1), returns_value});
    }
    return result;
}

std::vector<size_t> WgslMutator::GetLoopBodyPositions(const std::string& wgsl_code) {
    // Finds all loops.
    std::regex loop_regex("[^a-zA-Z_0-9](for|while|loop)[^\\{]*\\{");
    std::vector<size_t> result;

    auto loops_begin = std::sregex_iterator(wgsl_code.begin(), wgsl_code.end(), loop_regex);
    auto loops_end = std::sregex_iterator();

    for (std::sregex_iterator i = loops_begin; i != loops_end; ++i) {
        result.push_back(static_cast<size_t>(i->suffix().first - wgsl_code.cbegin() - 1));
    }
    return result;
}

bool WgslMutator::InsertReturnStatement(std::string& wgsl_code) {
    std::vector<std::pair<size_t, bool>> function_body_positions =
        GetFunctionBodyPositions(wgsl_code);

    // No function was found in wgsl_code.
    if (function_body_positions.empty()) {
        return false;
    }

    // Pick a random function
    auto function = generator_.GetRandomElement(function_body_positions);

    // Find the corresponding closing bracket for the function, and find a semi-colon within the
    // function body.
    size_t left_bracket_pos = function.first;

    size_t right_bracket_pos = FindClosingBracket(left_bracket_pos, wgsl_code, '{', '}');

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

    std::string return_statement = "return";
    if (function.second) {
        // The function returns a value. Get all identifiers and integer literals to use as
        // potential return values.
        std::vector<std::pair<size_t, size_t>> identifiers = GetIdentifiers(wgsl_code);
        auto return_values = identifiers;
        std::vector<std::pair<size_t, size_t>> int_literals = GetIntLiterals(wgsl_code);
        return_values.insert(return_values.end(), int_literals.begin(), int_literals.end());
        std::pair<size_t, size_t> return_value = generator_.GetRandomElement(return_values);
        return_statement += " " + wgsl_code.substr(return_value.first, return_value.second);
    }
    return_statement += ";";

    // Insert the return statement immediately after the semicolon.
    wgsl_code.insert(generator_.GetRandomElement(semicolon_positions) + 1, return_statement);
    return true;
}

bool WgslMutator::InsertBreakOrContinue(std::string& wgsl_code) {
    std::vector<size_t> loop_body_positions = GetLoopBodyPositions(wgsl_code);

    // No loop was found in wgsl_code.
    if (loop_body_positions.empty()) {
        return false;
    }

    // Pick a random loop's opening bracket, find the corresponding closing
    // bracket, and find a semi-colon within the loop body.
    size_t left_bracket_pos = generator_.GetRandomElement(loop_body_positions);

    size_t right_bracket_pos = FindClosingBracket(left_bracket_pos, wgsl_code, '{', '}');

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

    size_t semicolon_position = generator_.GetRandomElement(semicolon_positions);

    // Insert a break or continue immediately after the semicolon.
    wgsl_code.insert(semicolon_position + 1, generator_.GetBool() ? "break;" : "continue;");
    return true;
}

void WgslMutator::SwapIntervals(size_t idx1,
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

void WgslMutator::DeleteInterval(size_t idx1, size_t reg_len, std::string& wgsl_code) {
    wgsl_code.erase(idx1 + 1, reg_len - 1);
}

void WgslMutator::DuplicateInterval(size_t idx1,
                                    size_t reg1_len,
                                    size_t idx2,
                                    std::string& wgsl_code) {
    std::string region = wgsl_code.substr(idx1 + 1, reg1_len - 1);
    wgsl_code.insert(idx2 + 1, region);
}

void WgslMutator::ReplaceRegion(size_t idx1,
                                size_t id1_len,
                                size_t idx2,
                                size_t id2_len,
                                std::string& wgsl_code) {
    std::string region_1 = wgsl_code.substr(idx1, id1_len);
    std::string region_2 = wgsl_code.substr(idx2, id2_len);
    wgsl_code.replace(idx2, region_2.size(), region_1);
}

void WgslMutator::ReplaceInterval(size_t start_index,
                                  size_t length,
                                  std::string replacement_text,
                                  std::string& wgsl_code) {
    std::string region_1 = wgsl_code.substr(start_index, length);
    wgsl_code.replace(start_index, length, replacement_text);
}

bool WgslMutator::SwapRandomIntervals(const std::string& delimiter, std::string& wgsl_code) {
    std::vector<size_t> delimiter_positions = FindDelimiterIndices(delimiter, wgsl_code);

    // Need to have at least 3 indices.
    if (delimiter_positions.size() < 3) {
        return false;
    }

    // Choose indices:
    //   interval_1_start < interval_1_end <= interval_2_start < interval_2_end
    uint32_t interval_1_start =
        generator_.GetUInt32(static_cast<uint32_t>(delimiter_positions.size()) - 2u);
    uint32_t interval_1_end = generator_.GetUInt32(
        interval_1_start + 1u, static_cast<uint32_t>(delimiter_positions.size()) - 1u);
    uint32_t interval_2_start = generator_.GetUInt32(
        interval_1_end, static_cast<uint32_t>(delimiter_positions.size()) - 1u);
    uint32_t interval_2_end = generator_.GetUInt32(
        interval_2_start + 1u, static_cast<uint32_t>(delimiter_positions.size()));

    SwapIntervals(delimiter_positions[interval_1_start],
                  delimiter_positions[interval_1_end] - delimiter_positions[interval_1_start],
                  delimiter_positions[interval_2_start],
                  delimiter_positions[interval_2_end] - delimiter_positions[interval_2_start],
                  wgsl_code);

    return true;
}

bool WgslMutator::DeleteRandomInterval(const std::string& delimiter, std::string& wgsl_code) {
    std::vector<size_t> delimiter_positions = FindDelimiterIndices(delimiter, wgsl_code);

    // Need to have at least 2 indices.
    if (delimiter_positions.size() < 2) {
        return false;
    }

    uint32_t interval_start =
        generator_.GetUInt32(static_cast<uint32_t>(delimiter_positions.size()) - 1u);
    uint32_t interval_end = generator_.GetUInt32(interval_start + 1u,
                                                 static_cast<uint32_t>(delimiter_positions.size()));

    DeleteInterval(delimiter_positions[interval_start],
                   delimiter_positions[interval_end] - delimiter_positions[interval_start],
                   wgsl_code);

    return true;
}

bool WgslMutator::DuplicateRandomInterval(const std::string& delimiter, std::string& wgsl_code) {
    std::vector<size_t> delimiter_positions = FindDelimiterIndices(delimiter, wgsl_code);

    // Need to have at least 2 indices
    if (delimiter_positions.size() < 2) {
        return false;
    }

    uint32_t interval_start =
        generator_.GetUInt32(static_cast<uint32_t>(delimiter_positions.size()) - 1u);
    uint32_t interval_end = generator_.GetUInt32(interval_start + 1u,
                                                 static_cast<uint32_t>(delimiter_positions.size()));
    uint32_t duplication_point =
        generator_.GetUInt32(static_cast<uint32_t>(delimiter_positions.size()));

    DuplicateInterval(delimiter_positions[interval_start],
                      delimiter_positions[interval_end] - delimiter_positions[interval_start],
                      delimiter_positions[duplication_point], wgsl_code);

    return true;
}

bool WgslMutator::ReplaceRandomIdentifier(std::string& wgsl_code) {
    std::vector<std::pair<size_t, size_t>> identifiers = GetIdentifiers(wgsl_code);

    // Need at least 2 identifiers
    if (identifiers.size() < 2) {
        return false;
    }

    uint32_t id1_index = generator_.GetUInt32(static_cast<uint32_t>(identifiers.size()));
    uint32_t id2_index = generator_.GetUInt32(static_cast<uint32_t>(identifiers.size()));

    // The two identifiers must be different
    while (id1_index == id2_index) {
        id2_index = generator_.GetUInt32(static_cast<uint32_t>(identifiers.size()));
    }

    ReplaceRegion(identifiers[id1_index].first, identifiers[id1_index].second,
                  identifiers[id2_index].first, identifiers[id2_index].second, wgsl_code);

    return true;
}

bool WgslMutator::ReplaceRandomIntLiteral(std::string& wgsl_code) {
    std::vector<std::pair<size_t, size_t>> literals = GetIntLiterals(wgsl_code);

    // Need at least one integer literal
    if (literals.size() < 1) {
        return false;
    }

    uint32_t literal_index = generator_.GetUInt32(static_cast<uint32_t>(literals.size()));

    // INT_MAX = 2147483647, INT_MIN = -2147483648
    std::vector<std::string> boundary_values = {"2147483647", "-2147483648", "1",
                                                "-1",         "0",           "4294967295"};

    uint32_t boundary_index = generator_.GetUInt32(static_cast<uint32_t>(boundary_values.size()));

    ReplaceInterval(literals[literal_index].first, literals[literal_index].second,
                    boundary_values[boundary_index], wgsl_code);

    return true;
}

std::string WgslMutator::ChooseRandomReplacementForOperator(const std::string& existing_operator) {
    // Operators are partitioned into three classes: assignment, expression and increment. The regex
    // mutator will swap operators in the same class. The hypothesis is that this should exercise a
    // number of type-safe swaps (e.g. changing += to *=), as well as some badly-typed yet
    // interesting swaps (e.g. changing + to ^ when the operators are matrices), while avoiding
    // making totally nonsensical replacements (such as changing ++ too /).
    std::vector<std::string> assignment_operators{
        "=", "+=", "-=", "*=", "/=", "%=", "&=", "|=", "^=", "<<=", ">>="};
    std::vector<std::string> expression_operators{"+",  "-",  "*", "/",  "%",  "&&", "||",
                                                  "&",  "|",  "^", "<<", ">>", "<",  ">",
                                                  "<=", ">=", "!", "==", "!=", "~"};
    std::vector<std::string> increment_operators{"++", "--"};
    for (auto operators : {assignment_operators, expression_operators, increment_operators}) {
        auto it = std::find(operators.begin(), operators.end(), existing_operator);
        if (it != operators.end()) {
            // The operator falls into this category, so select another operator from the category.
            operators.erase(it);
            return generator_.GetRandomElement(operators);
        }
    }
    assert(false && "Unknown operator");
    return "";
}

bool WgslMutator::ReplaceRandomOperator(std::string& wgsl_code) {
    // Choose an index into the code at random.
    const uint32_t start_index = generator_.GetUInt32(static_cast<uint32_t>(wgsl_code.size()));
    // Find the first operator occurrence from the chosen point, wrapping back to the start of the
    // file if needed.
    auto maybe_operator_occurrence = FindOperatorOccurrence(wgsl_code, start_index);
    if (!maybe_operator_occurrence.has_value()) {
        // It is unlikely that there will be *no* operators in the file, but if this is the case
        // then this mutation cannot be applied.
        return false;
    }
    std::string existing_operator =
        wgsl_code.substr(maybe_operator_occurrence->first, maybe_operator_occurrence->second);
    // Replace the identified operator with a randomly-chosen alternative.
    wgsl_code.replace(maybe_operator_occurrence->first, maybe_operator_occurrence->second,
                      ChooseRandomReplacementForOperator(existing_operator));
    return true;
}

std::optional<std::pair<uint32_t, uint32_t>> WgslMutator::FindOperatorOccurrence(
    const std::string& wgsl_code,
    uint32_t start_index) {
    // Loops through the characters of the code in a wrap-around fashion, looking for the first
    // encountered token that is a WGSL operator.

    for (size_t i = 0; i < wgsl_code.size(); i++) {
        uint32_t current_index = static_cast<uint32_t>((start_index + i) % wgsl_code.size());

        // To cater for multi-character operator tokens, get the three consecutive characters from
        // the code string starting at the current index. Use null characters to account for the
        // case where search has reached the end of the code string.
        char first_character = wgsl_code[current_index];
        char second_character =
            current_index + 1 == wgsl_code.size() ? '\0' : wgsl_code[current_index + 1];
        char third_character =
            current_index + 2 >= wgsl_code.size() ? '\0' : wgsl_code[current_index + 2];

        // This uses the extracted characters to match for the various WGSL operators.
        switch (first_character) {
            case '!':
            case '^':
            case '*':
            case '/':
            case '%':
            case '=':
                // The above cases are all stand-alone operators, and if followed by '=' are also
                // operators.
                switch (second_character) {
                    case '=':
                        return {{current_index, 2}};
                    default:
                        return {{current_index, 1}};
                }
            case '|':
            case '&':
            case '+':
            case '-':
                // The above cases are all stand-alone operators, and if repeated or followed by '='
                // are also operators.
                if (second_character == first_character || second_character == '=') {
                    return {{current_index, 2}};
                }
                return {{current_index, 1}};
            case '<':
            case '>':
                // The following caters for '<', '<=', '<<', '<<=', '>', '>=', '>>' and '>>='.
                if (second_character == '=') {
                    return {{current_index, 2}};
                }
                if (second_character == first_character) {
                    if (third_character == '=') {
                        return {{current_index, 3}};
                    }
                    return {{current_index, 2}};
                }
                return {{current_index, 1}};
            case '~':
                return {{current_index, 1}};
            default:
                break;
        }
    }
    // No operator was found, so empty is returned.
    return {};
}

bool WgslMutator::ReplaceFunctionCallWithBuiltin(std::string& wgsl_code) {
    std::vector<std::pair<size_t, bool>> function_body_positions =
        GetFunctionBodyPositions(wgsl_code);

    // No function was found in wgsl_code.
    if (function_body_positions.empty()) {
        return false;
    }

    // Pick a random function
    auto function = generator_.GetRandomElement(function_body_positions);

    // Find the corresponding closing bracket for the function.
    size_t left_bracket_pos = function.first;

    size_t right_bracket_pos = FindClosingBracket(left_bracket_pos, wgsl_code, '{', '}');

    if (right_bracket_pos == 0) {
        return false;
    }

    std::string function_body(
        wgsl_code.substr(left_bracket_pos, right_bracket_pos - left_bracket_pos));

    std::vector<std::pair<size_t, size_t>> function_call_identifiers =
        GetFunctionCallIdentifiers(function_body);
    if (function_call_identifiers.empty()) {
        return false;
    }
    auto function_call_identifier = generator_.GetRandomElement(function_call_identifiers);

    std::vector<std::string> builtin_functions{"all",
                                               "any",
                                               "select",
                                               "arrayLength",
                                               "abs",
                                               "acos",
                                               "acosh",
                                               "asin",
                                               "asinh",
                                               "atan",
                                               "atanh",
                                               "atan2",
                                               "ceil",
                                               "clamp",
                                               "cos",
                                               "cosh",
                                               "cross",
                                               "degrees",
                                               "distance",
                                               "exp",
                                               "exp2",
                                               "faceForward",
                                               "floor",
                                               "fma",
                                               "fract",
                                               "frexp",
                                               "inverseSqrt",
                                               "ldexp",
                                               "length",
                                               "log",
                                               "log2",
                                               "max",
                                               "min",
                                               "mix",
                                               "modf",
                                               "normalize",
                                               "pow",
                                               "quantizeToF16",
                                               "radians",
                                               "reflect",
                                               "refract",
                                               "round",
                                               "saturate",
                                               "sign",
                                               "sin",
                                               "sinh",
                                               "smoothstep",
                                               "sqrt",
                                               "step",
                                               "tan",
                                               "tanh",
                                               "trunc",
                                               "abs",
                                               "clamp",
                                               "countLeadingZeros",
                                               "countOneBits",
                                               "countTrailingZeros",
                                               "extractBits",
                                               "firstLeadingBit",
                                               "firstTrailingBit",
                                               "insertBits",
                                               "max",
                                               "min",
                                               "reverseBits",
                                               "determinant",
                                               "transpose",
                                               "dot",
                                               "dpdx",
                                               "dpdxCoarse",
                                               "dpdxFine",
                                               "dpdy",
                                               "dpdyCoarse",
                                               "dpdyFine",
                                               "fwidth",
                                               "fwidthCoarse",
                                               "fwidthFine",
                                               "textureDimensions",
                                               "textureGather",
                                               "textureGatherCompare",
                                               "textureLoad",
                                               "textureNumLayers",
                                               "textureNumLevels",
                                               "textureNumSamples",
                                               "textureSample",
                                               "textureSampleBias",
                                               "textureSampleCompare",
                                               "textureSampleCompareLevel",
                                               "textureSampleGrad",
                                               "textureSampleLevel",
                                               "textureStore",
                                               "atomicLoad",
                                               "atomicStore",
                                               "atomicAdd",
                                               "atomicSub",
                                               "atomicMax",
                                               "atomicMin",
                                               "atomicAnd",
                                               "atomicOr",
                                               "atomicXor",
                                               "pack4x8snorm",
                                               "pack4x8unorm",
                                               "pack2x16snorm",
                                               "pack2x16unorm",
                                               "pack2x16float",
                                               "unpack4x8snorm",
                                               "unpack4x8unorm",
                                               "unpack2x16snorm",
                                               "unpack2x16unorm",
                                               "unpack2x16float",
                                               "storageBarrier",
                                               "workgroupUniformLoad",
                                               "workgroupBarrier"};
    wgsl_code.replace(left_bracket_pos + function_call_identifier.first,
                      function_call_identifier.second,
                      generator_.GetRandomElement(builtin_functions));
    return true;
}

bool WgslMutator::AddSwizzle(std::string& wgsl_code) {
    std::vector<std::pair<size_t, bool>> function_body_positions =
        GetFunctionBodyPositions(wgsl_code);

    // No function was found in wgsl_code.
    if (function_body_positions.empty()) {
        return false;
    }

    // Pick a random function
    auto function = generator_.GetRandomElement(function_body_positions);

    // Find the corresponding closing bracket for the function.
    size_t left_bracket_pos = function.first;
    size_t right_bracket_pos = FindClosingBracket(left_bracket_pos, wgsl_code, '{', '}');

    if (right_bracket_pos == 0) {
        return false;
    }

    std::string function_body(
        wgsl_code.substr(left_bracket_pos, right_bracket_pos - left_bracket_pos));

    // It makes sense to try applying swizzles to:
    // - identifiers, because they might be vectors
    auto identifiers = GetIdentifiers(function_body);
    // - existing swizzles, e.g. to turn v.xy into v.xy.xx
    auto swizzles = GetSwizzles(function_body);
    // - vector initializers, e.g. to turn vec3<f32>(...) into vec3<f32>(...).yyz
    auto vector_initializers = GetVectorInitializers(function_body);

    // Create a combined vector of all the possibilities for swizzling, so that they can be sampled
    // from as a whole.
    std::vector<std::pair<size_t, size_t>> combined;
    combined.insert(combined.end(), identifiers.begin(), identifiers.end());
    combined.insert(combined.end(), swizzles.begin(), swizzles.end());
    combined.insert(combined.end(), vector_initializers.begin(), vector_initializers.end());

    if (combined.empty()) {
        // No opportunities for swizzling: give up.
        return false;
    }

    // Randomly create a swizzle operation. This is done without checking the potential length of
    // the target vector. For identifiers this isn't possible without proper context. For existing
    // swizzles and vector initializers it would be possible to check the length, but it is anyway
    // good to stress-test swizzle validation code paths.
    std::string swizzle = ".";
    {
        // Choose a swizzle length between 1 and 4, inclusive.
        uint32_t swizzle_length = generator_.GetUInt32(1, 5);
        // Decide whether to use xyzw or rgba as convenience names.
        bool use_xyzw = generator_.GetBool();
        // Randomly choose a convenience name for each component of the swizzle.
        for (uint32_t i = 0; i < swizzle_length; i++) {
            switch (generator_.GetUInt32(4)) {
                case 0:
                    swizzle += use_xyzw ? "x" : "r";
                    break;
                case 1:
                    swizzle += use_xyzw ? "y" : "g";
                    break;
                case 2:
                    swizzle += use_xyzw ? "z" : "b";
                    break;
                case 3:
                    swizzle += use_xyzw ? "w" : "a";
                    break;
                default:
                    assert(false && "Unreachable");
                    break;
            }
        }
    }
    // Choose a random opportunity for swizzling and add the swizzle right after it.
    auto target = generator_.GetRandomElement(combined);
    wgsl_code.insert(left_bracket_pos + target.first + target.second, swizzle);
    return true;
}

std::vector<std::pair<size_t, size_t>> WgslMutator::GetSwizzles(const std::string& wgsl_code) {
    std::regex swizzle_regex("\\.(([xyzw]+)|([rgba]+))");
    std::vector<std::pair<size_t, size_t>> result;

    auto swizzles_begin = std::sregex_iterator(wgsl_code.begin(), wgsl_code.end(), swizzle_regex);
    auto swizles_end = std::sregex_iterator();

    for (std::sregex_iterator i = swizzles_begin; i != swizles_end; ++i) {
        result.push_back(
            {static_cast<size_t>(i->prefix().second - wgsl_code.cbegin()), i->str().size()});
    }
    return result;
}

std::vector<std::pair<size_t, size_t>> WgslMutator::GetVectorInitializers(
    const std::string& wgsl_code) {
    // This regex recognises the prefixes of vector initializers, which have the form:
    // "vecn<type>(", with possible whitespace between tokens.
    std::regex vector_initializer_prefix_regex("vec\\d[ \\n]*<[ \\n]*[a-z0-9_]+[ \\n]*>[^\\(]*\\(");
    std::vector<std::pair<size_t, size_t>> result;

    auto vector_initializer_prefixes_begin =
        std::sregex_iterator(wgsl_code.begin(), wgsl_code.end(), vector_initializer_prefix_regex);
    auto vector_initializer_prefixes_end = std::sregex_iterator();

    // Look through all of the vector initializer prefixes and see whether each one appears to
    // correspond to a complete vector construction.
    for (std::sregex_iterator i = vector_initializer_prefixes_begin;
         i != vector_initializer_prefixes_end; ++i) {
        // A prefix is deemed to correspond to a complete vector construction if it is possible to
        // find a corresponding closing bracket for the "(" at the end of the prefix.
        size_t closing_bracket = FindClosingBracket(
            static_cast<size_t>(i->suffix().first - wgsl_code.cbegin()), wgsl_code, '(', ')');
        if (closing_bracket != 0) {
            // A closing bracket was found, so record the start and size of the entire vector
            // initializer.
            size_t start = static_cast<size_t>(i->prefix().second - wgsl_code.cbegin());
            result.push_back({start, closing_bracket - start + 1});
        }
    }
    return result;
}

}  // namespace tint::fuzzers::regex_fuzzer
