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

#include "src/tint/reader/wgsl/parser_impl_test_helper.h"

namespace tint::reader::wgsl {
namespace {

const diag::Formatter::Style formatter_style{/* print_file: */ true, /* print_severity: */ true,
                                             /* print_line: */ true,
                                             /* print_newline_at_end: */ false};

class ParserImplErrorResyncTest : public ParserImplTest {};

#define EXPECT(SOURCE, EXPECTED)                                                   \
    do {                                                                           \
        std::string source = SOURCE;                                               \
        std::string expected = EXPECTED;                                           \
        auto p = parser(source);                                                   \
        EXPECT_EQ(false, p->Parse());                                              \
        auto diagnostics = p->builder().Diagnostics();                             \
        EXPECT_EQ(true, diagnostics.contains_errors());                            \
        EXPECT_EQ(expected, diag::Formatter(formatter_style).format(diagnostics)); \
    } while (false)

TEST_F(ParserImplErrorResyncTest, BadFunctionDecls) {
    EXPECT(R"(
fn .() -> . {}
fn x(.) {}
@_ fn -> {}
fn good() {}
)",
           R"(test.wgsl:2:4 error: expected identifier for function declaration
fn .() -> . {}
   ^

test.wgsl:2:11 error: unable to determine function return type
fn .() -> . {}
          ^

test.wgsl:3:6 error: expected ')' for function declaration
fn x(.) {}
     ^

test.wgsl:4:2 error: expected attribute
Possible values: 'align', 'binding', 'builtin', 'compute', 'diagnostic', 'fragment', 'group', 'id', 'interpolate', 'invariant', 'location', 'must_use', 'size', 'vertex', 'workgroup_size'
@_ fn -> {}
 ^
)");
}

TEST_F(ParserImplErrorResyncTest, AssignmentStatement) {
    EXPECT(R"(
fn f() {
  blah blah blah blah;
  good = 1;
  blah blah blah blah;
  x = .;
  good = 1;
}
)",
           R"(test.wgsl:3:8 error: expected '=' for assignment
  blah blah blah blah;
       ^^^^

test.wgsl:5:8 error: expected '=' for assignment
  blah blah blah blah;
       ^^^^

test.wgsl:6:7 error: unable to parse right side of assignment
  x = .;
      ^
)");
}

TEST_F(ParserImplErrorResyncTest, DiscardStatement) {
    EXPECT(R"(
fn f() {
  discard blah blah blah;
  a = 1;
  discard blah blah blah;
}
)",
           R"(test.wgsl:3:11 error: expected ';' for discard statement
  discard blah blah blah;
          ^^^^

test.wgsl:5:11 error: expected ';' for discard statement
  discard blah blah blah;
          ^^^^
)");
}

TEST_F(ParserImplErrorResyncTest, StructMembers) {
    EXPECT(R"(
struct S {
    blah blah blah,
    a : i32,
    blah blah blah,
    b : i32,
    @- x : i32,
    c : i32,
}
)",
           R"(test.wgsl:3:10 error: expected ':' for struct member
    blah blah blah,
         ^^^^

test.wgsl:5:10 error: expected ':' for struct member
    blah blah blah,
         ^^^^

test.wgsl:7:6 error: expected attribute
Possible values: 'align', 'binding', 'builtin', 'compute', 'diagnostic', 'fragment', 'group', 'id', 'interpolate', 'invariant', 'location', 'must_use', 'size', 'vertex', 'workgroup_size'
    @- x : i32,
     ^
)");
}

// Check that the forward scan in resynchronize() stop at nested sync points.
// In this test the inner resynchronize() is looking for a terminating ';', and
// the outer resynchronize() is looking for a terminating '}' for the function
// scope.
TEST_F(ParserImplErrorResyncTest, NestedSyncPoints) {
    EXPECT(R"(
fn f() {
  x = 1;
  discard
}
struct S { blah };
)",
           R"(test.wgsl:5:1 error: expected ';' for discard statement
}
^

test.wgsl:6:17 error: expected ':' for struct member
struct S { blah };
                ^
)");
}

TEST_F(ParserImplErrorResyncTest, BracketCounting) {
    EXPECT(
        R"(
fn f(x(((())))) {
  meow = {{{}}}
}
struct S { blah };
)",
        R"(test.wgsl:2:7 error: expected ':' for parameter
fn f(x(((())))) {
      ^

test.wgsl:3:10 error: unable to parse right side of assignment
  meow = {{{}}}
         ^

test.wgsl:5:17 error: expected ':' for struct member
struct S { blah };
                ^
)");
}

}  // namespace
}  // namespace tint::reader::wgsl
