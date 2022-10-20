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

#include <optional>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include "src/tint/fuzzers/random_generator.h"

namespace tint::fuzzers::regex_fuzzer {

/// Class encapsulating code for regex-based mutation of WGSL shaders.
class WgslMutator {
  public:
    /// Constructor
    /// @param generator - pseudo-random generator to use in mutator
    explicit WgslMutator(RandomGenerator& generator);

    /// A function that, given WGSL-like string and a delimiter,
    /// generates another WGSL-like string by picking two random regions
    /// enclosed by the delimiter and swapping them.
    /// @param delimiter - the delimiter that will be used to find enclosed regions.
    /// @param wgsl_code - the initial string (WGSL code) that will be mutated.
    /// @return true if a swap happened or false otherwise.
    bool SwapRandomIntervals(const std::string& delimiter, std::string& wgsl_code);

    /// A function that, given a WGSL-like string and a delimiter,
    /// generates another WGSL-like string by deleting a random
    /// region enclosed by the delimiter.
    /// @param delimiter - the delimiter that will be used to find enclosed regions.
    /// @param wgsl_code - the initial string (WGSL code) that will be mutated.
    /// @return true if a deletion happened or false otherwise.
    bool DeleteRandomInterval(const std::string& delimiter, std::string& wgsl_code);

    /// A function that, given a WGSL-like string and a delimiter,
    /// generates another WGSL-like string by duplicating a random
    /// region enclosed by the delimiter.
    /// @param delimiter - the delimiter that will be used to find enclosed regions.
    /// @param wgsl_code - the initial string (WGSL code) that will be mutated.
    /// @return true if a duplication happened or false otherwise.
    bool DuplicateRandomInterval(const std::string& delimiter, std::string& wgsl_code);

    /// Replaces a randomly-chosen identifier in wgsl_code.
    /// @param wgsl_code - WGSL-like string where the replacement will occur.
    /// @return true if a replacement happened or false otherwise.
    bool ReplaceRandomIdentifier(std::string& wgsl_code);

    /// Replaces the value of a randomly-chosen integer with one of
    /// the values in the set {INT_MAX, INT_MIN, 0, -1}.
    /// @param wgsl_code - WGSL-like string where the replacement will occur.
    /// @return true if a replacement happened or false otherwise.
    bool ReplaceRandomIntLiteral(std::string& wgsl_code);

    /// Inserts a return statement in a randomly chosen function of a
    /// WGSL-like string. The return value is a randomly-chosen identifier
    /// or literal in the string.
    /// @param wgsl_code - WGSL-like string that will be mutated.
    /// @return true if the mutation was succesful or false otherwise.
    bool InsertReturnStatement(std::string& wgsl_code);

    /// Inserts a break or continue statement in a randomly chosen loop of a WGSL-like string.
    /// @param wgsl_code - WGSL-like string that will be mutated.
    /// @return true if the mutation was succesful or false otherwise.
    bool InsertBreakOrContinue(std::string& wgsl_code);

    /// A function that, given WGSL-like string, generates a new WGSL-like string by replacing one
    /// randomly-chosen operator in the original string with another operator.
    /// @param wgsl_code - the initial WGSL-like string that will be mutated.
    /// @return true if an operator replacement happened or false otherwise.
    bool ReplaceRandomOperator(std::string& wgsl_code);

    /// Given a WGSL-like string, replaces a random identifier that appears to be a function call
    /// with the name of a built-in function. This will often lead to an invalid module, as the
    /// mutation does not aim to check whether the original and replacement function have the same
    /// number or types of arguments.
    /// @param wgsl_code - the initial WGSL-like string that will be mutated.
    /// @return true if a function call replacement happened or false otherwise.
    bool ReplaceFunctionCallWithBuiltin(std::string& wgsl_code);

    /// Given a WGSL-like string, adds a swizzle operation to either (a) an identifier, (b) a vector
    /// initializer, or (c) an existing swizzle.
    /// @param wgsl_code - the initial WGSL-like string that will be mutated.
    /// @return true if a swizzle operation is added or false otherwise.
    bool AddSwizzle(std::string& wgsl_code);

  protected:
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

    /// Finds a possible closing bracket corresponding to the opening
    /// bracket at position opening_bracket_pos.
    /// @param opening_bracket_pos - the position of the opening bracket.
    /// @param wgsl_code - the WGSL-like string where the closing bracket.
    /// @param opening_bracket_character - the opening bracket character, e.g. (, {, <, or [
    /// @param closing_bracket_character - the closing bracket character, e.g. ), }, >, or ]
    /// @return the position of the closing bracket or 0 if there is no closing
    /// bracket.
    size_t FindClosingBracket(size_t opening_bracket_pos,
                              const std::string& wgsl_code,
                              char opening_bracket_character,
                              char closing_bracket_character);

    /// Returns the starting position of the bodies of the functions identified by an appropriate
    /// function, together with a boolean indicating whether the function returns a value or not.
    /// @param wgsl_code - the WGSL-like string where the functions will be
    /// searched.
    /// @return a vector of pairs, where each pair provides the starting position of the function
    /// body, and the value true if and only if the function returns a value.
    std::vector<std::pair<size_t, bool>> GetFunctionBodyPositions(const std::string& wgsl_code);

    /// Returns the starting position of the bodies of the loops identified by an appropriate
    /// regular expressions.
    /// @param wgsl_code - the WGSL-like string in which loops will be searched for.
    /// @return a vector with the starting position of the loop bodies in wgsl_code.
    std::vector<size_t> GetLoopBodyPositions(const std::string& wgsl_code);

    /// A function that finds all the identifiers in a WGSL-like string.
    /// @param wgsl_code - the WGSL-like string where the identifiers will be found.
    /// @return a vector with the positions and the length of all the
    /// identifiers in wgsl_code.
    std::vector<std::pair<size_t, size_t>> GetIdentifiers(const std::string& wgsl_code);

    /// A function that finds the identifiers in a WGSL-like string that appear to be used as
    /// function names in function call expressions.
    /// @param wgsl_code - the WGSL-like string where the identifiers will be found.
    /// @return a vector with the positions and the length of all the
    /// identifiers in wgsl_code.
    std::vector<std::pair<size_t, size_t>> GetFunctionCallIdentifiers(const std::string& wgsl_code);

    /// A function that returns returns the starting position
    /// and the length of all the integer literals in a WGSL-like string.
    /// @param wgsl_code - the WGSL-like string where the int literals
    /// will be found.
    /// @return a vector with the starting positions and the length
    /// of all the integer literals.
    std::vector<std::pair<size_t, size_t>> GetIntLiterals(const std::string& wgsl_code);

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

    /// Finds the next occurrence of an operator in a WGSL-like string from a given starting
    /// position, wrapping around to the start of the string if no operator is found before reaching
    /// the end, and returning empty of no operator is found at all. There is no guarantee that the
    /// result will correspond to a WGSL operator token, e.g. the identified characters could be
    /// part of a comment, or e.g. the file might contain >>=, in which case the operator
    /// >= will be identified should it happen that the starting index corresponds to the second >
    /// character of this operator. Given that the regex mutator does not aim to guarantee
    /// well-formed programs, these issues are acceptable.
    /// @param wgsl_code - the WGSL-like string in which operator occurrences will be found.
    /// @param start_index - the index at which search should start
    /// @return empty if no operator is found, otherwise a pair comprising the index at which the
    /// operator starts and the character length of the operator.
    std::optional<std::pair<uint32_t, uint32_t>> FindOperatorOccurrence(
        const std::string& wgsl_code,
        uint32_t start_index);

    /// Finds all the swizzle operations in a WGSL-like string.
    /// @param wgsl_code - the WGSL-like string where the swizzles will be found.
    /// @return a vector with the positions and lengths of all the swizzles in wgsl_code.
    std::vector<std::pair<size_t, size_t>> GetSwizzles(const std::string& wgsl_code);

    /// Finds all the vector initializers in a WGSL-like string.
    /// @param wgsl_code - the WGSL-like string where the vector initializers will be found.
    /// @return a vector with the positions and lengths of all the vector initializers in wgsl_code.
    std::vector<std::pair<size_t, size_t>> GetVectorInitializers(const std::string& wgsl_code);

  private:
    /// A function that given a delimiter, returns a vector that contains
    /// all the positions of the delimiter in the WGSL code.
    /// @param delimiter - the delimiter of the enclosed region.
    /// @param wgsl_code - the initial string (WGSL code) that will be mutated.
    /// @return a vector with the positions of the delimiter in the WGSL code.
    std::vector<size_t> FindDelimiterIndices(const std::string& delimiter,
                                             const std::string& wgsl_code);

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

    /// Given a string representing a WGSL operator, randomly returns a different WGSL operator in
    /// the same category as the original, where the three categories are assignment operators (such
    /// as = and +=), expression operators (such as + and ^) and increment operators (++ and --).
    /// @param existing_operator - the characters comprising some WGSL operator
    /// @return another WGSL operator falling into the same category.
    std::string ChooseRandomReplacementForOperator(const std::string& existing_operator);

    /// Yields a fixed set of commonly-used WGSL keywords. The regex fuzzer relies heavily on
    /// recognizing possible identifiers via regular expressions. There is a high chance that
    /// keywords will be recognized as identifiers, which will leads to invalid code. It is valuable
    /// for this to occur to some extent (to stress test validation), but it is useful to be able to
    /// exclude the most common keywords so that invalidity does not occur too often.
    /// @return a set of commonly-used WGSL keywords.
    static std::unordered_set<std::string> GetCommonKeywords();

    RandomGenerator& generator_;
};

}  // namespace tint::fuzzers::regex_fuzzer

#endif  // SRC_TINT_FUZZERS_TINT_REGEX_FUZZER_WGSL_MUTATOR_H_
