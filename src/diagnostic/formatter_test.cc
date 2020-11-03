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

constexpr const char* content =
    R"(the cat says meow
the dog says woof
the snake says quack
the snail says ???
)";

class DiagFormatterTest : public testing::Test {
 public:
  Source::File file{"file.name", content};
  Diagnostic diag_info{Severity::Info,
                       Source{Source::Range{Source::Location{1, 14}}, &file},
                       "purr"};
  Diagnostic diag_warn{Severity::Warning,
                       Source{Source::Range{{2, 14}, {2, 18}}, &file}, "grrr"};
  Diagnostic diag_err{Severity::Error,
                      Source{Source::Range{{3, 16}, {3, 21}}, &file}, "hiss"};
  Diagnostic diag_fatal{Severity::Fatal,
                        Source{Source::Range{{4, 16}, {4, 19}}, &file},
                        "nothing"};
};

TEST_F(DiagFormatterTest, Simple) {
  Formatter fmt{{false, false, false}};
  auto got = fmt.format(List{diag_info, diag_warn, diag_err, diag_fatal});
  auto* expect = R"(1:14: purr
2:14: grrr
3:16: hiss
4:16: nothing)";
  ASSERT_EQ(expect, got);
}

TEST_F(DiagFormatterTest, SimpleNoSource) {
  Formatter fmt{{false, false, false}};
  Diagnostic diag{Severity::Info, Source{}, "no source!"};
  auto got = fmt.format(List{diag});
  auto* expect = "no source!";
  ASSERT_EQ(expect, got);
}

TEST_F(DiagFormatterTest, WithFile) {
  Formatter fmt{{true, false, false}};
  auto got = fmt.format(List{diag_info, diag_warn, diag_err, diag_fatal});
  auto* expect = R"(file.name:1:14: purr
file.name:2:14: grrr
file.name:3:16: hiss
file.name:4:16: nothing)";
  ASSERT_EQ(expect, got);
}

TEST_F(DiagFormatterTest, WithSeverity) {
  Formatter fmt{{false, true, false}};
  auto got = fmt.format(List{diag_info, diag_warn, diag_err, diag_fatal});
  auto* expect = R"(1:14 info: purr
2:14 warning: grrr
3:16 error: hiss
4:16 fatal: nothing)";
  ASSERT_EQ(expect, got);
}

TEST_F(DiagFormatterTest, WithLine) {
  Formatter fmt{{false, false, true}};
  auto got = fmt.format(List{diag_info, diag_warn, diag_err, diag_fatal});
  auto* expect = R"(1:14: purr
the cat says meow
             ^

2:14: grrr
the dog says woof
             ^^^^

3:16: hiss
the snake says quack
               ^^^^^

4:16: nothing
the snail says ???
               ^^^
)";
  ASSERT_EQ(expect, got);
}

TEST_F(DiagFormatterTest, BasicWithFileSeverityLine) {
  Formatter fmt{{true, true, true}};
  auto got = fmt.format(List{diag_info, diag_warn, diag_err, diag_fatal});
  auto* expect = R"(file.name:1:14 info: purr
the cat says meow
             ^

file.name:2:14 warning: grrr
the dog says woof
             ^^^^

file.name:3:16 error: hiss
the snake says quack
               ^^^^^

file.name:4:16 fatal: nothing
the snail says ???
               ^^^
)";
  ASSERT_EQ(expect, got);
}

TEST_F(DiagFormatterTest, BasicWithMultiLine) {
  Diagnostic multiline{Severity::Warning,
                       Source{Source::Range{{2, 9}, {4, 15}}, &file},
                       "multiline"};
  Formatter fmt{{false, false, true}};
  auto got = fmt.format(List{multiline});
  auto* expect = R"(2:9: multiline
the dog says woof
        ^^^^^^^^^
the snake says quack
^^^^^^^^^^^^^^^^^^^^
the snail says ???
^^^^^^^^^^^^^^
)";
  ASSERT_EQ(expect, got);
}

}  // namespace
}  // namespace diag
}  // namespace tint
