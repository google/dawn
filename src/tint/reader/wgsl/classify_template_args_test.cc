// Copyright 2023 The Tint Authors.
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

#include "gmock/gmock.h"

#include "src/tint/reader/wgsl/classify_template_args.h"
#include "src/tint/reader/wgsl/lexer.h"
#include "src/tint/utils/transform.h"

namespace tint::reader::wgsl {
namespace {

using T = Token::Type;

struct Case {
    const char* wgsl;
    std::vector<T> tokens;
};

static std::ostream& operator<<(std::ostream& out, const Case& c) {
    return out << "'" << c.wgsl << "'";
}

using ClassifyTemplateArgsTest = testing::TestWithParam<Case>;

TEST_P(ClassifyTemplateArgsTest, Classify) {
    auto& params = GetParam();
    Source::File file("", params.wgsl);
    Lexer l(&file);
    auto tokens = l.Lex();
    ClassifyTemplateArguments(tokens);
    auto types = utils::Transform(tokens, [&](const Token& t) { return t.type(); });
    EXPECT_THAT(types, testing::ContainerEq(params.tokens));
}

INSTANTIATE_TEST_SUITE_P(NonTemplate,
                         ClassifyTemplateArgsTest,
                         testing::ValuesIn(std::vector<Case>{
                             {
                                 "",
                                 {T::kEOF},
                             },
                             {
                                 "abc",
                                 {T::kIdentifier, T::kEOF},
                             },
                             {
                                 "a<b",
                                 {T::kIdentifier, T::kLessThan, T::kIdentifier, T::kEOF},
                             },
                             {
                                 "a>b",
                                 {T::kIdentifier, T::kGreaterThan, T::kIdentifier, T::kEOF},
                             },
                             {
                                 "(a<b)>c",
                                 {
                                     T::kParenLeft,    // (
                                     T::kIdentifier,   // a
                                     T::kLessThan,     // <
                                     T::kIdentifier,   // b
                                     T::kParenRight,   // )
                                     T::kGreaterThan,  // >
                                     T::kIdentifier,   // c
                                     T::kEOF,
                                 },
                             },
                             {
                                 "a<(b>c)",
                                 {
                                     T::kIdentifier,   // a
                                     T::kLessThan,     // <
                                     T::kParenLeft,    // (
                                     T::kIdentifier,   // b
                                     T::kGreaterThan,  // >
                                     T::kIdentifier,   // c
                                     T::kParenRight,   // )
                                     T::kEOF,
                                 },
                             },
                             {
                                 "a((b<c), d>(e))",
                                 {
                                     T::kIdentifier,   // a
                                     T::kParenLeft,    // (
                                     T::kParenLeft,    // (
                                     T::kIdentifier,   // b
                                     T::kLessThan,     // <
                                     T::kIdentifier,   // c
                                     T::kParenRight,   // )
                                     T::kComma,        // ,
                                     T::kIdentifier,   // d
                                     T::kGreaterThan,  // >
                                     T::kParenLeft,    // (
                                     T::kIdentifier,   // e
                                     T::kParenRight,   // )
                                     T::kParenRight,   // )
                                     T::kEOF,
                                 },
                             },
                             {
                                 "a<b[c>(d)]",
                                 {
                                     T::kIdentifier,    // a
                                     T::kLessThan,      // <
                                     T::kIdentifier,    // b
                                     T::kBracketLeft,   // [
                                     T::kIdentifier,    // c
                                     T::kGreaterThan,   // >
                                     T::kParenLeft,     // (
                                     T::kIdentifier,    // d
                                     T::kParenRight,    // )
                                     T::kBracketRight,  // ]
                                     T::kEOF,
                                 },
                             },
                             {
                                 "a<b;c>d()",
                                 {
                                     T::kIdentifier,   // a
                                     T::kLessThan,     // <
                                     T::kIdentifier,   // b
                                     T::kSemicolon,    // ;
                                     T::kIdentifier,   // c
                                     T::kGreaterThan,  // >
                                     T::kIdentifier,   // d
                                     T::kParenLeft,    // (
                                     T::kParenRight,   // )
                                     T::kEOF,
                                 },
                             },
                             {
                                 "if a < b {} else if c > d {}",
                                 {
                                     T::kIf,           // a
                                     T::kIdentifier,   // a
                                     T::kLessThan,     // <
                                     T::kIdentifier,   // b
                                     T::kBraceLeft,    // {
                                     T::kBraceRight,   // }
                                     T::kElse,         // else
                                     T::kIf,           // if
                                     T::kIdentifier,   // c
                                     T::kGreaterThan,  // >
                                     T::kIdentifier,   // d
                                     T::kBraceLeft,    // {
                                     T::kBraceRight,   // }
                                     T::kEOF,
                                 },
                             },
                             {
                                 "a<b&&c>d",
                                 {
                                     T::kIdentifier,   // a
                                     T::kLessThan,     // <
                                     T::kIdentifier,   // b
                                     T::kAndAnd,       // &&
                                     T::kPlaceholder,  // <placeholder>
                                     T::kIdentifier,   // c
                                     T::kGreaterThan,  // >
                                     T::kIdentifier,   // d
                                     T::kEOF,
                                 },
                             },
                             {
                                 "a<b||c>d",
                                 {
                                     T::kIdentifier,   // a
                                     T::kLessThan,     // <
                                     T::kIdentifier,   // b
                                     T::kOrOr,         // ||
                                     T::kIdentifier,   // c
                                     T::kGreaterThan,  // >
                                     T::kIdentifier,   // d
                                     T::kEOF,
                                 },
                             },
                             {
                                 "a<b<c||d>>",
                                 {
                                     T::kIdentifier,   // a
                                     T::kLessThan,     // <
                                     T::kIdentifier,   // b
                                     T::kLessThan,     // <
                                     T::kIdentifier,   // c
                                     T::kOrOr,         // ||
                                     T::kIdentifier,   // d
                                     T::kShiftRight,   // >>
                                     T::kPlaceholder,  // <placeholder>
                                     T::kEOF,
                                 },
                             },
                         }));

INSTANTIATE_TEST_SUITE_P(Template,
                         ClassifyTemplateArgsTest,
                         testing::ValuesIn(std::vector<Case>{
                             {
                                 "a<b>()",
                                 {
                                     T::kIdentifier,         // a
                                     T::kTemplateArgsLeft,   // <
                                     T::kIdentifier,         // b
                                     T::kTemplateArgsRight,  // >
                                     T::kParenLeft,          // (
                                     T::kParenRight,         // )
                                     T::kEOF,
                                 },
                             },
                             {
                                 "a<b>c",
                                 {
                                     T::kIdentifier,         // a
                                     T::kTemplateArgsLeft,   // <
                                     T::kIdentifier,         // b
                                     T::kTemplateArgsRight,  // >
                                     T::kIdentifier,         // c
                                     T::kEOF,
                                 },
                             },
                             {
                                 "vec3<i32>",
                                 {
                                     T::kIdentifier,         // vec3
                                     T::kTemplateArgsLeft,   // <
                                     T::kIdentifier,         // i32
                                     T::kTemplateArgsRight,  // >
                                     T::kEOF,
                                 },
                             },
                             {
                                 "vec3<i32>()",
                                 {
                                     T::kIdentifier,         // vec3
                                     T::kTemplateArgsLeft,   // <
                                     T::kIdentifier,         // i32
                                     T::kTemplateArgsRight,  // >
                                     T::kParenLeft,          // (
                                     T::kParenRight,         // )
                                     T::kEOF,
                                 },
                             },
                             {
                                 "array<vec3<i32>,5>",
                                 {
                                     T::kIdentifier,         // array
                                     T::kTemplateArgsLeft,   // <
                                     T::kIdentifier,         // vec3
                                     T::kTemplateArgsLeft,   // <
                                     T::kIdentifier,         // i32
                                     T::kTemplateArgsRight,  // >
                                     T::kComma,              // ,
                                     T::kIntLiteral,         // 5
                                     T::kTemplateArgsRight,  // >
                                     T::kEOF,
                                 },
                             },
                             {
                                 "a(b<c, d>(e))",
                                 {
                                     T::kIdentifier,         // a
                                     T::kParenLeft,          // (
                                     T::kIdentifier,         // b
                                     T::kTemplateArgsLeft,   // <
                                     T::kIdentifier,         // c
                                     T::kComma,              // ,
                                     T::kIdentifier,         // d
                                     T::kTemplateArgsRight,  // >
                                     T::kParenLeft,          // (
                                     T::kIdentifier,         // e
                                     T::kParenRight,         // )
                                     T::kParenRight,         // )
                                     T::kEOF,
                                 },
                             },
                             {
                                 "a<1+2>()",
                                 {
                                     T::kIdentifier,         // a
                                     T::kTemplateArgsLeft,   // <
                                     T::kIntLiteral,         // 1
                                     T::kPlus,               // +
                                     T::kIntLiteral,         // 2
                                     T::kTemplateArgsRight,  // >
                                     T::kParenLeft,          // (
                                     T::kParenRight,         // )
                                     T::kEOF,
                                 },
                             },
                             {
                                 "a<1,b>()",
                                 {
                                     T::kIdentifier,         // a
                                     T::kTemplateArgsLeft,   // <
                                     T::kIntLiteral,         // 1
                                     T::kComma,              // ,
                                     T::kIdentifier,         // b
                                     T::kTemplateArgsRight,  // >
                                     T::kParenLeft,          // (
                                     T::kParenRight,         // )
                                     T::kEOF,
                                 },
                             },
                             {
                                 "a<b,c>=d",
                                 {
                                     T::kIdentifier,         // a
                                     T::kTemplateArgsLeft,   // <
                                     T::kIdentifier,         // b
                                     T::kComma,              // ,
                                     T::kIdentifier,         // c
                                     T::kTemplateArgsRight,  // >
                                     T::kEqual,              // =
                                     T::kIdentifier,         // d
                                     T::kEOF,
                                 },
                             },
                             {
                                 "a<b,c>=d>()",
                                 {
                                     T::kIdentifier,         // a
                                     T::kTemplateArgsLeft,   // <
                                     T::kIdentifier,         // b
                                     T::kComma,              // ,
                                     T::kIdentifier,         // c
                                     T::kTemplateArgsRight,  // >
                                     T::kEqual,              // =
                                     T::kIdentifier,         // d
                                     T::kGreaterThan,        // >
                                     T::kParenLeft,          // (
                                     T::kParenRight,         // )
                                     T::kEOF,
                                 },
                             },
                             {
                                 "a<b<c>>=",
                                 {
                                     T::kIdentifier,         // a
                                     T::kTemplateArgsLeft,   // <
                                     T::kIdentifier,         // b
                                     T::kTemplateArgsLeft,   // <
                                     T::kIdentifier,         // c
                                     T::kTemplateArgsRight,  // >
                                     T::kTemplateArgsRight,  // >
                                     T::kEqual,              // =
                                     T::kEOF,
                                 },
                             },
                             {
                                 "a<b>c>()",
                                 {
                                     T::kIdentifier,         // a
                                     T::kTemplateArgsLeft,   // <
                                     T::kIdentifier,         // b
                                     T::kTemplateArgsRight,  // >
                                     T::kIdentifier,         // c
                                     T::kGreaterThan,        // >
                                     T::kParenLeft,          // (
                                     T::kParenRight,         // )
                                     T::kEOF,
                                 },
                             },
                             {
                                 "a<b<c>()",
                                 {
                                     T::kIdentifier,         // a
                                     T::kLessThan,           // <
                                     T::kIdentifier,         // b
                                     T::kTemplateArgsLeft,   // <
                                     T::kIdentifier,         // c
                                     T::kTemplateArgsRight,  // >
                                     T::kParenLeft,          // (
                                     T::kParenRight,         // )
                                     T::kEOF,
                                 },
                             },
                             {
                                 "a<b<c>>()",
                                 {
                                     T::kIdentifier,         // a
                                     T::kTemplateArgsLeft,   // <
                                     T::kIdentifier,         // b
                                     T::kTemplateArgsLeft,   // <
                                     T::kIdentifier,         // c
                                     T::kTemplateArgsRight,  // >
                                     T::kTemplateArgsRight,  // >
                                     T::kParenLeft,          // (
                                     T::kParenRight,         // )
                                     T::kEOF,
                                 },
                             },
                             {
                                 "a<b<c>()>()",
                                 {
                                     T::kIdentifier,         // a
                                     T::kTemplateArgsLeft,   // <
                                     T::kIdentifier,         // b
                                     T::kTemplateArgsLeft,   // <
                                     T::kIdentifier,         // c
                                     T::kTemplateArgsRight,  // >
                                     T::kParenLeft,          // (
                                     T::kParenRight,         // )
                                     T::kTemplateArgsRight,  // >
                                     T::kParenLeft,          // (
                                     T::kParenRight,         // )
                                     T::kEOF,
                                 },
                             },
                             {
                                 "a<b>.c",
                                 {
                                     T::kIdentifier,         // a
                                     T::kTemplateArgsLeft,   // <
                                     T::kIdentifier,         // b
                                     T::kTemplateArgsRight,  // >
                                     T::kPeriod,             // .
                                     T::kIdentifier,         // c
                                     T::kEOF,
                                 },
                             },
                             {
                                 "a<(b&&c)>d",
                                 {
                                     T::kIdentifier,         // a
                                     T::kTemplateArgsLeft,   // <
                                     T::kParenLeft,          // (
                                     T::kIdentifier,         // b
                                     T::kAndAnd,             // &&
                                     T::kPlaceholder,        // <placeholder>
                                     T::kIdentifier,         // c
                                     T::kParenRight,         // )
                                     T::kTemplateArgsRight,  // >
                                     T::kIdentifier,         // d
                                     T::kEOF,
                                 },
                             },
                             {
                                 "a<(b||c)>d",
                                 {
                                     T::kIdentifier,         // a
                                     T::kTemplateArgsLeft,   // <
                                     T::kParenLeft,          // (
                                     T::kIdentifier,         // b
                                     T::kOrOr,               // ||
                                     T::kIdentifier,         // c
                                     T::kParenRight,         // )
                                     T::kTemplateArgsRight,  // >
                                     T::kIdentifier,         // d
                                     T::kEOF,
                                 },
                             },
                             {
                                 "a<b<(c||d)>>",
                                 {
                                     T::kIdentifier,         // a
                                     T::kTemplateArgsLeft,   // <
                                     T::kIdentifier,         // b
                                     T::kTemplateArgsLeft,   // <
                                     T::kParenLeft,          // (
                                     T::kIdentifier,         // c
                                     T::kOrOr,               // ||
                                     T::kIdentifier,         // d
                                     T::kParenRight,         // )
                                     T::kTemplateArgsRight,  // >
                                     T::kTemplateArgsRight,  // >
                                     T::kEOF,
                                 },
                             },
                         }));

INSTANTIATE_TEST_SUITE_P(TreesitterScannerSeparatingCases,
                         ClassifyTemplateArgsTest,
                         testing::ValuesIn(std::vector<Case>{
                             // Treesitter had trouble missing '=' in its lookahead
                             {
                                 "a<b>=c",
                                 {
                                     T::kIdentifier,         // a
                                     T::kTemplateArgsLeft,   // <
                                     T::kIdentifier,         // b
                                     T::kTemplateArgsRight,  // >
                                     T::kEqual,              // =
                                     T::kIdentifier,         // c
                                     T::kEOF,
                                 },
                             },
                             {
                                 "a<b>>=c",
                                 {
                                     T::kIdentifier,         // a
                                     T::kTemplateArgsLeft,   // <
                                     T::kIdentifier,         // b
                                     T::kTemplateArgsRight,  // >
                                     T::kGreaterThanEqual,   // >=
                                     T::kPlaceholder,        // <placeholder>
                                     T::kIdentifier,         // c
                                     T::kEOF,
                                 },
                             },
                             {
                                 "a<b==c>",
                                 {
                                     T::kIdentifier,         // a
                                     T::kTemplateArgsLeft,   // <
                                     T::kIdentifier,         // b
                                     T::kEqualEqual,         // ==
                                     T::kIdentifier,         // c
                                     T::kTemplateArgsRight,  // >
                                     T::kEOF,
                                 },
                             },
                             {
                                 "a<(b==c)>",
                                 {
                                     T::kIdentifier,         // a
                                     T::kTemplateArgsLeft,   // <
                                     T::kParenLeft,          // (
                                     T::kIdentifier,         // b
                                     T::kEqualEqual,         // ==
                                     T::kIdentifier,         // c
                                     T::kParenRight,         // )
                                     T::kTemplateArgsRight,  // >
                                     T::kEOF,
                                 },
                             },
                             {
                                 "a<b<=c>",
                                 {
                                     T::kIdentifier,         // a
                                     T::kTemplateArgsLeft,   // <
                                     T::kIdentifier,         // b
                                     T::kLessThanEqual,      // <=
                                     T::kIdentifier,         // c
                                     T::kTemplateArgsRight,  // >
                                     T::kEOF,
                                 },
                             },
                             {
                                 "a<(b<=c)>",
                                 {
                                     T::kIdentifier,         // a
                                     T::kTemplateArgsLeft,   // <
                                     T::kParenLeft,          // (
                                     T::kIdentifier,         // b
                                     T::kLessThanEqual,      // <=
                                     T::kIdentifier,         // c
                                     T::kParenRight,         // )
                                     T::kTemplateArgsRight,  // >
                                     T::kEOF,
                                 },
                             },
                             {
                                 "a<b>=c>",
                                 {
                                     T::kIdentifier,         // a
                                     T::kTemplateArgsLeft,   // <
                                     T::kIdentifier,         // b
                                     T::kTemplateArgsRight,  // >
                                     T::kEqual,              // =
                                     T::kIdentifier,         // c
                                     T::kGreaterThan,        // >
                                     T::kEOF,
                                 },
                             },
                             {
                                 "a<(b<=c)>",
                                 {
                                     T::kIdentifier,         // a
                                     T::kTemplateArgsLeft,   // <
                                     T::kParenLeft,          // (
                                     T::kIdentifier,         // b
                                     T::kLessThanEqual,      // <=
                                     T::kIdentifier,         // c
                                     T::kParenRight,         // )
                                     T::kTemplateArgsRight,  // >
                                     T::kEOF,
                                 },
                             },
                             {
                                 "a<b>>c>",
                                 {
                                     T::kIdentifier,         // a
                                     T::kTemplateArgsLeft,   // <
                                     T::kIdentifier,         // b
                                     T::kTemplateArgsRight,  // >
                                     T::kGreaterThan,        // >
                                     T::kIdentifier,         // c
                                     T::kGreaterThan,        // >
                                     T::kEOF,
                                 },
                             },
                             {
                                 "a<b<<c>",
                                 {
                                     T::kIdentifier,         // a
                                     T::kTemplateArgsLeft,   // <
                                     T::kIdentifier,         // b
                                     T::kShiftLeft,          // <<
                                     T::kIdentifier,         // c
                                     T::kTemplateArgsRight,  // >
                                     T::kEOF,
                                 },
                             },
                             {
                                 "a<(b<<c)>",
                                 {
                                     T::kIdentifier,         // a
                                     T::kTemplateArgsLeft,   // <
                                     T::kParenLeft,          // (
                                     T::kIdentifier,         // b
                                     T::kShiftLeft,          // <<
                                     T::kIdentifier,         // c
                                     T::kParenRight,         // )
                                     T::kTemplateArgsRight,  // >
                                     T::kEOF,
                                 },
                             },
                             {
                                 "a<(b>>c)>",
                                 {
                                     T::kIdentifier,         // a
                                     T::kTemplateArgsLeft,   // <
                                     T::kParenLeft,          // (
                                     T::kIdentifier,         // b
                                     T::kShiftRight,         // >>
                                     T::kPlaceholder,        // <placeholder>
                                     T::kIdentifier,         // c
                                     T::kParenRight,         // )
                                     T::kTemplateArgsRight,  // >
                                     T::kEOF,
                                 },
                             },
                             {
                                 "a<1<<c>",
                                 {
                                     T::kIdentifier,         // a
                                     T::kTemplateArgsLeft,   // <
                                     T::kIntLiteral,         // 1
                                     T::kShiftLeft,          // <<
                                     T::kIdentifier,         // c
                                     T::kTemplateArgsRight,  // >
                                     T::kEOF,
                                 },
                             },
                             {
                                 "a<1<<c<d>()>",
                                 {
                                     T::kIdentifier,         // a
                                     T::kTemplateArgsLeft,   // <
                                     T::kIntLiteral,         // 1
                                     T::kShiftLeft,          // <<
                                     T::kIdentifier,         // c
                                     T::kTemplateArgsLeft,   // <
                                     T::kIdentifier,         // d
                                     T::kTemplateArgsRight,  // >
                                     T::kParenLeft,          // (
                                     T::kParenRight,         // )
                                     T::kTemplateArgsRight,  // >
                                     T::kEOF,
                                 },
                             },
                         }));

}  // namespace
}  // namespace tint::reader::wgsl
