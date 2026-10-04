// Copyright 2026 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef SRC_UTILS_TEST_H_
#define SRC_UTILS_TEST_H_

#include <gtest/gtest.h>  // IWYU pragma: export

#include "src/utils/assert.h"
#include "src/utils/compiler.h"

// Wrappers around *_DEATH_IF_SUPPORTED that only match the death message when assertions and
// logging are enabled (since DAWN_CHECK / DAWN_ASSERT only log condition strings when
// DAWN_ENABLE_ASSERTS is defined and DAWN_DISABLE_LOGGING is not defined).
#if defined(DAWN_ENABLE_ASSERTS) && !defined(DAWN_DISABLE_LOGGING)
#define DAWN_EXPECT_DEATH_IF_SUPPORTED(statement, matcher) \
    EXPECT_DEATH_IF_SUPPORTED(statement, matcher)
#define DAWN_ASSERT_DEATH_IF_SUPPORTED(statement, matcher) \
    ASSERT_DEATH_IF_SUPPORTED(statement, matcher)
#else
#define DAWN_EXPECT_DEATH_IF_SUPPORTED(statement, matcher) EXPECT_DEATH_IF_SUPPORTED(statement, "")
#define DAWN_ASSERT_DEATH_IF_SUPPORTED(statement, matcher) ASSERT_DEATH_IF_SUPPORTED(statement, "")
#endif

#if defined(GTEST_HAS_DEATH_TEST) && !defined(DAWN_ENABLE_ASSERTS)
// If death tests are supported and asserts are not enabled, defer to *_DEBUG_DEATH (which under
// NDEBUG executes the statement directly without expecting death).
#if !defined(DAWN_DISABLE_LOGGING)
#define DAWN_EXPECT_DEBUG_DEATH_IF_SUPPORTED(statement, matcher) \
    EXPECT_DEBUG_DEATH(statement, matcher)
#define DAWN_ASSERT_DEBUG_DEATH_IF_SUPPORTED(statement, matcher) \
    ASSERT_DEBUG_DEATH(statement, matcher)
#else
// If logging is disabled, don't match the death messages.
#define DAWN_EXPECT_DEBUG_DEATH_IF_SUPPORTED(statement, matcher) EXPECT_DEBUG_DEATH(statement, "")
#define DAWN_ASSERT_DEBUG_DEATH_IF_SUPPORTED(statement, matcher) ASSERT_DEBUG_DEATH(statement, "")
#endif
#else
// Otherwise, defer to DAWN_*_DEATH_IF_SUPPORTED, which either runs the death test (if supported) or
// prints the warning that we couldn't do the test.
#define DAWN_EXPECT_DEBUG_DEATH_IF_SUPPORTED(statement, matcher) \
    DAWN_EXPECT_DEATH_IF_SUPPORTED(statement, matcher)
#define DAWN_ASSERT_DEBUG_DEATH_IF_SUPPORTED(statement, matcher) \
    DAWN_ASSERT_DEATH_IF_SUPPORTED(statement, matcher)
#endif

// Use DAWN_EXPECT_ASSERT_DEATH_IF_SUPPORTED_ELSE_SKIP_UBSAN to test
// statements that are expected to exercise undefined behaviour guarded by an
// DAWN_ASSERT.
// The macro will:
// - Expect the death due to an assert, if death tests are supported and assert are turned on.
// - Otherwise skip the statement, if an undefined behavior sanitizer is active
// - Otherwise perform the statement. (In part to ensure test coverage).
#if !defined(DAWN_ENABLE_ASSERTS) && DAWN_UBSAN_ENABLED()
#define DAWN_EXPECT_ASSERT_DEATH_IF_SUPPORTED_ELSE_SKIP_UBSAN(statement, matcher)  \
    do {                                                                           \
        GTEST_SKIP() << "Test exercises undefined behavior, and ubsan is enabled"; \
        statement; /* This may have the only reference to a variable */            \
    } while (DAWN_ASSERT_LOOP_CONDITION)
#else
#define DAWN_EXPECT_ASSERT_DEATH_IF_SUPPORTED_ELSE_SKIP_UBSAN(statement, matcher) \
    DAWN_EXPECT_DEBUG_DEATH_IF_SUPPORTED(statement, matcher)
#endif

#endif  // SRC_UTILS_TEST_H_
