// Copyright 2020 The Tint Authors.
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

#include "src/diagnostic/formatter.h"

#include "gtest/gtest.h"
#include "src/diagnostic/diagnostic.h"

namespace tint {
namespace diag {
namespace {

constexpr const char* content =  // Note: words are tab-delimited
    R"(the	cat	says	meow
the	dog	says	woof
the	snake	says	quack
the	snail	says	???
)";

class DiagFormatterTest : public testing::Test {
 public:
  Source::File file{"file.name", content};
  Diagnostic diag_note{Severity::Note,
                       Source{Source::Range{Source::Location{1, 14}}, &file},
                       "purr"};
  Diagnostic diag_warn{Severity::Warning,
                       Source{Source::Range{{2, 14}, {2, 18}}, &file}, "grrr"};
  Diagnostic diag_err{Severity::Error,
                      Source{Source::Range{{3, 16}, {3, 21}}, &file}, "hiss",
                      "abc123"};
  Diagnostic diag_ice{Severity::InternalCompilerError,
                      Source{Source::Range{{4, 16}, {4, 19}}, &file},
                      "unreachable"};
  Diagnostic diag_fatal{Severity::Fatal,
                        Source{Source::Range{{4, 16}, {4, 19}}, &file},
                        "nothing"};
};

TEST_F(DiagFormatterTest, Simple) {
  Formatter fmt{{false, false, false, false}};
  auto got = fmt.format(List{diag_note, diag_warn, diag_err});
  auto* expect = R"(1:14: purr
2:14: grrr
3:16 abc123: hiss)";
  ASSERT_EQ(expect, got);
}

TEST_F(DiagFormatterTest, SimpleNewlineAtEnd) {
  Formatter fmt{{false, false, false, true}};
  auto got = fmt.format(List{diag_note, diag_warn, diag_err});
  auto* expect = R"(1:14: purr
2:14: grrr
3:16 abc123: hiss
)";
  ASSERT_EQ(expect, got);
}

TEST_F(DiagFormatterTest, SimpleNoSource) {
  Formatter fmt{{false, false, false, false}};
  Diagnostic diag{Severity::Note, Source{}, "no source!"};
  auto got = fmt.format(List{diag});
  auto* expect = "no source!";
  ASSERT_EQ(expect, got);
}

TEST_F(DiagFormatterTest, WithFile) {
  Formatter fmt{{true, false, false, false}};
  auto got = fmt.format(List{diag_note, diag_warn, diag_err});
  auto* expect = R"(file.name:1:14: purr
file.name:2:14: grrr
file.name:3:16 abc123: hiss)";
  ASSERT_EQ(expect, got);
}

TEST_F(DiagFormatterTest, WithSeverity) {
  Formatter fmt{{false, true, false, false}};
  auto got = fmt.format(List{diag_note, diag_warn, diag_err});
  auto* expect = R"(1:14 note: purr
2:14 warning: grrr
3:16 error abc123: hiss)";
  ASSERT_EQ(expect, got);
}

TEST_F(DiagFormatterTest, WithLine) {
  Formatter fmt{{false, false, true, false}};
  auto got = fmt.format(List{diag_note, diag_warn, diag_err});
  auto* expect = R"(1:14: purr
the  cat  says  meow
                ^

2:14: grrr
the  dog  says  woof
                ^^^^

3:16 abc123: hiss
the  snake  says  quack
                  ^^^^^
)";
  ASSERT_EQ(expect, got);
}

TEST_F(DiagFormatterTest, BasicWithFileSeverityLine) {
  Formatter fmt{{true, true, true, false}};
  auto got = fmt.format(List{diag_note, diag_warn, diag_err});
  auto* expect = R"(file.name:1:14 note: purr
the  cat  says  meow
                ^

file.name:2:14 warning: grrr
the  dog  says  woof
                ^^^^

file.name:3:16 error abc123: hiss
the  snake  says  quack
                  ^^^^^
)";
  ASSERT_EQ(expect, got);
}

TEST_F(DiagFormatterTest, BasicWithMultiLine) {
  Diagnostic multiline{Severity::Warning,
                       Source{Source::Range{{2, 9}, {4, 15}}, &file},
                       "multiline"};
  Formatter fmt{{false, false, true, false}};
  auto got = fmt.format(List{multiline});
  auto* expect = R"(2:9: multiline
the  dog  says  woof
          ^^^^^^^^^^
the  snake  says  quack
^^^^^^^^^^^^^^^^^^^^^^^
the  snail  says  ???
^^^^^^^^^^^^^^^^
)";
  ASSERT_EQ(expect, got);
}

TEST_F(DiagFormatterTest, BasicWithFileSeverityLineTab4) {
  Formatter fmt{{true, true, true, false, 4u}};
  auto got = fmt.format(List{diag_note, diag_warn, diag_err});
  auto* expect = R"(file.name:1:14 note: purr
the    cat    says    meow
                      ^

file.name:2:14 warning: grrr
the    dog    says    woof
                      ^^^^

file.name:3:16 error abc123: hiss
the    snake    says    quack
                        ^^^^^
)";
  ASSERT_EQ(expect, got);
}

TEST_F(DiagFormatterTest, BasicWithMultiLineTab4) {
  Diagnostic multiline{Severity::Warning,
                       Source{Source::Range{{2, 9}, {4, 15}}, &file},
                       "multiline"};
  Formatter fmt{{false, false, true, false, 4u}};
  auto got = fmt.format(List{multiline});
  auto* expect = R"(2:9: multiline
the    dog    says    woof
              ^^^^^^^^^^^^
the    snake    says    quack
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
the    snail    says    ???
^^^^^^^^^^^^^^^^^^^^
)";
  ASSERT_EQ(expect, got);
}

TEST_F(DiagFormatterTest, ICE) {
  Formatter fmt{{}};
  auto got = fmt.format(List{diag_ice});
  auto* expect = R"(file.name:4:16 internal compiler error: unreachable
the  snail  says  ???
                  ^^^

********************************************************************
*  The tint shader compiler has encountered an unexpected error.   *
*                                                                  *
*  Please help us fix this issue by submitting a bug report at     *
*  crbug.com/tint with the source program that triggered the bug.  *
********************************************************************

)";
  ASSERT_EQ(expect, got);
}

TEST_F(DiagFormatterTest, Fatal) {
  Formatter fmt{{}};
  auto got = fmt.format(List{diag_fatal});
  auto* expect = R"(file.name:4:16 fatal: nothing
the  snail  says  ???
                  ^^^

********************************************************************
*  The tint shader compiler has encountered an unexpected error.   *
*                                                                  *
*  Please help us fix this issue by submitting a bug report at     *
*  crbug.com/tint with the source program that triggered the bug.  *
********************************************************************

)";
  ASSERT_EQ(expect, got);
}

}  // namespace
}  // namespace diag
}  // namespace tint
