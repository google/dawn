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

#ifndef SRC_TINT_UTILS_GENERATION_ID_H_
#define SRC_TINT_UTILS_GENERATION_ID_H_

#include <stdint.h>
#include <utility>

#include "src/tint/utils/ice/ice.h"
#include "src/tint/utils/text/string_stream.h"

namespace tint {

/// If 1 then checks are enabled that AST nodes are not leaked from one generation
/// to another.
/// TODO(bclayton): We'll want to disable this in production builds. For now we
/// always check.
#define TINT_CHECK_FOR_CROSS_GENERATION_LEAKS 1

/// A GenerationID is a unique identifier of a generation.
/// GenerationID can be used to ensure that objects referenced by the generation are
/// owned exclusively by that generation and have accidentally not leaked from
/// another generation.
class GenerationID {
  public:
    /// Constructor
    GenerationID();

    /// @returns a new. globally unique GenerationID
    static GenerationID New();

    /// Equality operator
    /// @param rhs the other GenerationID
    /// @returns true if the GenerationIDs are equal
    bool operator==(const GenerationID& rhs) const { return val == rhs.val; }

    /// Inequality operator
    /// @param rhs the other GenerationID
    /// @returns true if the GenerationIDs are not equal
    bool operator!=(const GenerationID& rhs) const { return val != rhs.val; }

    /// @returns the numerical identifier value
    uint32_t Value() const { return val; }

    /// @returns true if this GenerationID is valid
    operator bool() const { return val != 0; }

  private:
    explicit GenerationID(uint32_t);

    uint32_t val = 0;
};

/// A simple pass-through function for GenerationID. Intended to be overloaded for
/// other types.
/// @param id a GenerationID
/// @returns id. Simple pass-through function
inline GenerationID GenerationIDOf(GenerationID id) {
    return id;
}

/// Writes the GenerationID to the stream.
/// @param out the stream to write to
/// @param id the generation identifier to write
/// @returns out so calls can be chained
inline StringStream& operator<<(StringStream& out, GenerationID id) {
    out << "Generation<" << id.Value() << ">";
    return out;
}

namespace detail {

/// AssertGenerationIDsEqual is called by TINT_ASSERT_GENERATION_IDS_EQUAL() and
/// TINT_ASSERT_GENERATION_IDS_EQUAL_IF_VALID() to assert that the GenerationIDs
/// `a` and `b` are equal.
void AssertGenerationIDsEqual(GenerationID a,
                              GenerationID b,
                              bool if_valid,
                              const char* msg,
                              const char* file,
                              size_t line);

}  // namespace detail

/// TINT_ASSERT_GENERATION_IDS_EQUAL(SYSTEM, A, B) is a macro that asserts that the
/// generation identifiers for A and B are equal.
///
/// TINT_ASSERT_GENERATION_IDS_EQUAL_IF_VALID(SYSTEM, A, B) is a macro that asserts
/// that the generation identifiers for A and B are equal, if both A and B have
/// valid generation identifiers.
#if TINT_CHECK_FOR_CROSS_GENERATION_LEAKS
#define TINT_ASSERT_GENERATION_IDS_EQUAL(a, b)                                                 \
    tint::detail::AssertGenerationIDsEqual(GenerationIDOf(a), GenerationIDOf(b), false,        \
                                           "TINT_ASSERT_GENERATION_IDS_EQUAL(" #a ", " #b ")", \
                                           __FILE__, __LINE__)
#define TINT_ASSERT_GENERATION_IDS_EQUAL_IF_VALID(a, b) \
    tint::detail::AssertGenerationIDsEqual(             \
        GenerationIDOf(a), GenerationIDOf(b), true,     \
        "TINT_ASSERT_GENERATION_IDS_EQUAL_IF_VALID(" #a ", " #b ")", __FILE__, __LINE__)
#else
#define TINT_ASSERT_GENERATION_IDS_EQUAL(a, b) \
    do {                                       \
    } while (false)
#define TINT_ASSERT_GENERATION_IDS_EQUAL_IF_VALID(a, b) \
    do {                                                \
    } while (false)
#endif

}  // namespace tint

#endif  // SRC_TINT_UTILS_GENERATION_ID_H_
