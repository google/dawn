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

#ifndef SRC_TINT_PROGRAM_ID_H_
#define SRC_TINT_PROGRAM_ID_H_

#include <stdint.h>
#include <utility>

#include "src/tint/debug.h"
#include "src/tint/utils/string_stream.h"

namespace tint {

/// If 1 then checks are enabled that AST nodes are not leaked from one program
/// to another.
/// TODO(bclayton): We'll want to disable this in production builds. For now we
/// always check.
#define TINT_CHECK_FOR_CROSS_PROGRAM_LEAKS 1

/// A ProgramID is a unique identifier of a Program.
/// ProgramID can be used to ensure that objects referenced by the Program are
/// owned exclusively by that Program and have accidentally not leaked from
/// another Program.
class ProgramID {
  public:
    /// Constructor
    ProgramID();

    /// @returns a new. globally unique ProgramID
    static ProgramID New();

    /// Equality operator
    /// @param rhs the other ProgramID
    /// @returns true if the ProgramIDs are equal
    bool operator==(const ProgramID& rhs) const { return val == rhs.val; }

    /// Inequality operator
    /// @param rhs the other ProgramID
    /// @returns true if the ProgramIDs are not equal
    bool operator!=(const ProgramID& rhs) const { return val != rhs.val; }

    /// @returns the numerical identifier value
    uint32_t Value() const { return val; }

    /// @returns true if this ProgramID is valid
    operator bool() const { return val != 0; }

  private:
    explicit ProgramID(uint32_t);

    uint32_t val = 0;
};

/// A simple pass-through function for ProgramID. Intended to be overloaded for
/// other types.
/// @param id a ProgramID
/// @returns id. Simple pass-through function
inline ProgramID ProgramIDOf(ProgramID id) {
    return id;
}

/// Writes the ProgramID to the stream.
/// @param out the stream to write to
/// @param id the program identifier to write
/// @returns out so calls can be chained
inline utils::StringStream& operator<<(utils::StringStream& out, ProgramID id) {
    out << "Program<" << id.Value() << ">";
    return out;
}

namespace detail {

/// AssertProgramIDsEqual is called by TINT_ASSERT_PROGRAM_IDS_EQUAL() and
/// TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID() to assert that the ProgramIDs
/// `a` and `b` are equal.
void AssertProgramIDsEqual(ProgramID a,
                           ProgramID b,
                           bool if_valid,
                           diag::System system,
                           const char* msg,
                           const char* file,
                           size_t line);

}  // namespace detail

/// TINT_ASSERT_PROGRAM_IDS_EQUAL(SYSTEM, A, B) is a macro that asserts that the
/// program identifiers for A and B are equal.
///
/// TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(SYSTEM, A, B) is a macro that asserts
/// that the program identifiers for A and B are equal, if both A and B have
/// valid program identifiers.
#if TINT_CHECK_FOR_CROSS_PROGRAM_LEAKS
#define TINT_ASSERT_PROGRAM_IDS_EQUAL(system, a, b)                        \
    detail::AssertProgramIDsEqual(                                         \
        ProgramIDOf(a), ProgramIDOf(b), false, tint::diag::System::system, \
        "TINT_ASSERT_PROGRAM_IDS_EQUAL(" #system "," #a ", " #b ")", __FILE__, __LINE__)
#define TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(system, a, b)              \
    detail::AssertProgramIDsEqual(                                        \
        ProgramIDOf(a), ProgramIDOf(b), true, tint::diag::System::system, \
        "TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(" #system ", " #a ", " #b ")", __FILE__, __LINE__)
#else
#define TINT_ASSERT_PROGRAM_IDS_EQUAL(a, b) \
    do {                                    \
    } while (false)
#define TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(a, b) \
    do {                                             \
    } while (false)
#endif

}  // namespace tint

#endif  // SRC_TINT_PROGRAM_ID_H_
