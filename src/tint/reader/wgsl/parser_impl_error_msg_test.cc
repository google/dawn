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

#include "src/tint/utils/string_stream.h"

namespace tint::reader::wgsl {
namespace {

const diag::Formatter::Style formatter_style{/* print_file: */ true, /* print_severity: */ true,
                                             /* print_line: */ true,
                                             /* print_newline_at_end: */ false};

class ParserImplErrorTest : public ParserImplTest {};

#define EXPECT(SOURCE, EXPECTED)                                                   \
    do {                                                                           \
        std::string source = SOURCE;                                               \
        std::string expected = EXPECTED;                                           \
        auto p = parser(source);                                                   \
        p->set_max_errors(5);                                                      \
        EXPECT_EQ(false, p->Parse());                                              \
        auto diagnostics = p->builder().Diagnostics();                             \
        EXPECT_EQ(true, diagnostics.contains_errors());                            \
        EXPECT_EQ(expected, diag::Formatter(formatter_style).format(diagnostics)); \
    } while (false)

TEST_F(ParserImplErrorTest, AdditiveInvalidExpr) {
    EXPECT("fn f() { return 1.0 + <; }",
           R"(test.wgsl:1:23 error: unable to parse right side of + expression
fn f() { return 1.0 + <; }
                      ^
)");
}

TEST_F(ParserImplErrorTest, AndInvalidExpr) {
    EXPECT("fn f() { return 1 & >; }",
           R"(test.wgsl:1:21 error: unable to parse right side of & expression
fn f() { return 1 & >; }
                    ^
)");
}

TEST_F(ParserImplErrorTest, AliasDeclInvalidAttribute) {
    EXPECT("@invariant alias e=u32;",
           R"(test.wgsl:1:2 error: unexpected attributes
@invariant alias e=u32;
 ^^^^^^^^^
)");
}

TEST_F(ParserImplErrorTest, ConstAttributeInvalid) {
    EXPECT("@const fn main() { }",
           R"(test.wgsl:1:2 error: const attribute may not appear in shaders
@const fn main() { }
 ^^^^^
)");
}

TEST_F(ParserImplErrorTest, IndexExprInvalidExpr) {
    EXPECT("fn f() { x = y[^]; }",
           R"(test.wgsl:1:16 error: unable to parse expression inside []
fn f() { x = y[^]; }
               ^
)");
}

TEST_F(ParserImplErrorTest, IndexExprMissingRBracket) {
    EXPECT("fn f() { x = y[1; }",
           R"(test.wgsl:1:17 error: expected ']' for index accessor
fn f() { x = y[1; }
                ^
)");
}

TEST_F(ParserImplErrorTest, AssignmentStmtMissingAssignment) {
    EXPECT("fn f() { a; }", R"(test.wgsl:1:11 error: expected '=' for assignment
fn f() { a; }
          ^
)");
}

TEST_F(ParserImplErrorTest, AssignmentStmtMissingAssignment2) {
    EXPECT("fn f() { a : i32; }",
           R"(test.wgsl:1:10 error: expected 'var' for variable declaration
fn f() { a : i32; }
         ^
)");
}

TEST_F(ParserImplErrorTest, AssignmentStmtMissingSemicolon) {
    EXPECT("fn f() { a = 1 }",
           R"(test.wgsl:1:16 error: expected ';' for assignment statement
fn f() { a = 1 }
               ^
)");
}

TEST_F(ParserImplErrorTest, AssignmentStmtInvalidLHS_BuiltinFunctionName) {
    EXPECT("normalize = 5;",
           R"(test.wgsl:1:1 error: statement found outside of function body
normalize = 5;
^^^^^^^^^
)");
}

TEST_F(ParserImplErrorTest, AssignmentStmtInvalidRHS) {
    EXPECT("fn f() { a = >; }",
           R"(test.wgsl:1:14 error: unable to parse right side of assignment
fn f() { a = >; }
             ^
)");
}

TEST_F(ParserImplErrorTest, BitcastExprMissingLessThan) {
    EXPECT("fn f() { x = bitcast(y); }",
           R"(test.wgsl:1:21 error: expected '<' for bitcast expression
fn f() { x = bitcast(y); }
                    ^
)");
}

TEST_F(ParserImplErrorTest, BitcastExprMissingGreaterThan) {
    EXPECT("fn f() { x = bitcast<u32(y); }",
           R"(test.wgsl:1:21 error: missing closing '>' for bitcast expression
fn f() { x = bitcast<u32(y); }
                    ^
)");
}

TEST_F(ParserImplErrorTest, BitcastExprMissingType) {
    EXPECT("fn f() { x = bitcast<>(y); }",
           R"(test.wgsl:1:22 error: invalid type for bitcast expression
fn f() { x = bitcast<>(y); }
                     ^
)");
}

TEST_F(ParserImplErrorTest, BreakStmtMissingSemicolon) {
    EXPECT("fn f() { loop { break } }",
           R"(test.wgsl:1:23 error: expected ';' for break statement
fn f() { loop { break } }
                      ^
)");
}

TEST_F(ParserImplErrorTest, CallExprMissingRParen) {
    EXPECT("fn f() { x = f(1.; }",
           R"(test.wgsl:1:18 error: expected ')' for function call
fn f() { x = f(1.; }
                 ^
)");
}

TEST_F(ParserImplErrorTest, CallStmtMissingRParen) {
    EXPECT("fn f() { f(1.; }",
           R"(test.wgsl:1:14 error: expected ')' for function call
fn f() { f(1.; }
             ^
)");
}

TEST_F(ParserImplErrorTest, CallStmtInvalidArgument0) {
    EXPECT("fn f() { f(<); }",
           R"(test.wgsl:1:12 error: expected ')' for function call
fn f() { f(<); }
           ^
)");
}

TEST_F(ParserImplErrorTest, CallStmtInvalidArgument1) {
    EXPECT("fn f() { f(1.0, <); }",
           R"(test.wgsl:1:17 error: expected ')' for function call
fn f() { f(1.0, <); }
                ^
)");
}

TEST_F(ParserImplErrorTest, CallStmtMissingSemicolon) {
    EXPECT("fn f() { f() }",
           R"(test.wgsl:1:14 error: expected ';' for function call
fn f() { f() }
             ^
)");
}

TEST_F(ParserImplErrorTest, InitializerExprMissingLParen) {
    EXPECT("fn f() { x = vec2<u32>1,2); }",
           R"(test.wgsl:1:23 error: expected ';' for assignment statement
fn f() { x = vec2<u32>1,2); }
                      ^
)");
}

TEST_F(ParserImplErrorTest, InitializerExprMissingRParen) {
    EXPECT("fn f() { x = vec2<u32>(1,2; }",
           R"(test.wgsl:1:27 error: expected ')' for function call
fn f() { x = vec2<u32>(1,2; }
                          ^
)");
}

TEST_F(ParserImplErrorTest, ConstVarStmtInvalid) {
    EXPECT("fn f() { let >; }",
           R"(test.wgsl:1:14 error: expected identifier for 'let' declaration
fn f() { let >; }
             ^
)");
}

TEST_F(ParserImplErrorTest, ConstVarStmtMissingAssignment) {
    EXPECT("fn f() { let a : i32; }",
           R"(test.wgsl:1:21 error: expected '=' for 'let' declaration
fn f() { let a : i32; }
                    ^
)");
}

TEST_F(ParserImplErrorTest, ConstVarStmtMissingInitializer) {
    EXPECT("fn f() { let a : i32 = >; }",
           R"(test.wgsl:1:24 error: missing initializer for 'let' declaration
fn f() { let a : i32 = >; }
                       ^
)");
}

TEST_F(ParserImplErrorTest, ContinueStmtMissingSemicolon) {
    EXPECT("fn f() { loop { continue } }",
           R"(test.wgsl:1:26 error: expected ';' for continue statement
fn f() { loop { continue } }
                         ^
)");
}

TEST_F(ParserImplErrorTest, DiscardStmtMissingSemicolon) {
    EXPECT("fn f() { discard }",
           R"(test.wgsl:1:18 error: expected ';' for discard statement
fn f() { discard }
                 ^
)");
}

TEST_F(ParserImplErrorTest, EqualityInvalidExpr) {
    EXPECT("fn f() { return 1 == >; }",
           R"(test.wgsl:1:22 error: unable to parse right side of == expression
fn f() { return 1 == >; }
                     ^
)");
}

TEST_F(ParserImplErrorTest, ForLoopInitializerMissingSemicolon) {
    EXPECT("fn f() { for (var i : i32 = 0 i < 8; i=i+1) {} }",
           R"(test.wgsl:1:31 error: expected ';' for initializer in for loop
fn f() { for (var i : i32 = 0 i < 8; i=i+1) {} }
                              ^
)");
}

TEST_F(ParserImplErrorTest, ForLoopInitializerMissingVar) {
    EXPECT("fn f() { for (i : i32 = 0; i < 8; i=i+1) {} }",
           R"(test.wgsl:1:15 error: expected 'var' for variable declaration
fn f() { for (i : i32 = 0; i < 8; i=i+1) {} }
              ^
)");
}

TEST_F(ParserImplErrorTest, ForLoopConditionMissingSemicolon) {
    EXPECT("fn f() { for (var i : i32 = 0; i < 8 i=i+1) {} }",
           R"(test.wgsl:1:38 error: expected ';' for condition in for loop
fn f() { for (var i : i32 = 0; i < 8 i=i+1) {} }
                                     ^
)");
}

TEST_F(ParserImplErrorTest, ForLoopMissingLParen) {
    EXPECT("fn f() { for var i : i32 = 0; i < 8; i=i+1) {} }",
           R"(test.wgsl:1:14 error: expected '(' for for loop
fn f() { for var i : i32 = 0; i < 8; i=i+1) {} }
             ^^^
)");
}

TEST_F(ParserImplErrorTest, ForLoopMissingRParen) {
    EXPECT("fn f() { for (var i : i32 = 0; i < 8; i=i+1 {} }",
           R"(test.wgsl:1:45 error: expected ')' for for loop
fn f() { for (var i : i32 = 0; i < 8; i=i+1 {} }
                                            ^
)");
}

TEST_F(ParserImplErrorTest, ForLoopMissingLBrace) {
    EXPECT("fn f() { for (var i : i32 = 0; i < 8; i=i+1) }",
           R"(test.wgsl:1:46 error: expected '{' for for loop
fn f() { for (var i : i32 = 0; i < 8; i=i+1) }
                                             ^
)");
}

TEST_F(ParserImplErrorTest, ForLoopMissingRBrace) {
    EXPECT("fn f() { for (var i : i32 = 0; i < 8; i=i+1) {",
           R"(test.wgsl:1:47 error: expected '}' for for loop
fn f() { for (var i : i32 = 0; i < 8; i=i+1) {
                                              ^
)");
}

TEST_F(ParserImplErrorTest, FunctionDeclConstAssertMissingCondThenEOF) {
    EXPECT("fn f() { const_assert }", R"(test.wgsl:1:23 error: unable to parse condition expression
fn f() { const_assert }
                      ^
)");
}

TEST_F(ParserImplErrorTest, FunctionDeclConstAssertMissingCondThenSemicolon) {
    EXPECT("fn f() { const_assert; }",
           R"(test.wgsl:1:22 error: unable to parse condition expression
fn f() { const_assert; }
                     ^
)");
}

TEST_F(ParserImplErrorTest, FunctionDeclConstAssertMissingCondThenLet) {
    EXPECT("fn f() { const_assert\nlet x = 0; }",
           R"(test.wgsl:2:1 error: unable to parse condition expression
let x = 0; }
^^^
)");
}

TEST_F(ParserImplErrorTest, FunctionDeclConstAssertMissingLParen) {
    EXPECT("fn f() { const_assert true);", R"(test.wgsl:1:27 error: expected ';' for statement
fn f() { const_assert true);
                          ^
)");
}

TEST_F(ParserImplErrorTest, FunctionDeclConstAssertMissingRParen) {
    EXPECT("fn f() { const_assert (true;", R"(test.wgsl:1:28 error: expected ')'
fn f() { const_assert (true;
                           ^
)");
}

TEST_F(ParserImplErrorTest, FunctionDeclConstAssertMissingSemicolon) {
    EXPECT("fn f() { const_assert true }",
           R"(test.wgsl:1:28 error: expected ';' for statement
fn f() { const_assert true }
                           ^
)");
}

TEST_F(ParserImplErrorTest, FunctionDeclWorkgroupSizeXInvalid) {
    EXPECT("@workgroup_size() fn f() {}",
           R"(test.wgsl:1:2 error: workgroup_size expects at least 1 argument
@workgroup_size() fn f() {}
 ^^^^^^^^^^^^^^
)");
}

TEST_F(ParserImplErrorTest, FunctionDeclWorkgroupSizeYInvalid) {
    EXPECT("@workgroup_size(1, fn) fn f() {}",
           R"(test.wgsl:1:20 error: expected expression for workgroup_size
@workgroup_size(1, fn) fn f() {}
                   ^^
)");
}

TEST_F(ParserImplErrorTest, FunctionDeclWorkgroupSizeZInvalid) {
    EXPECT("@workgroup_size(1, 2, fn) fn f() {}",
           R"(test.wgsl:1:23 error: expected expression for workgroup_size
@workgroup_size(1, 2, fn) fn f() {}
                      ^^
)");
}

TEST_F(ParserImplErrorTest, FunctionDeclMissingIdentifier) {
    EXPECT("fn () {}",
           R"(test.wgsl:1:4 error: expected identifier for function declaration
fn () {}
   ^
)");
}

TEST_F(ParserImplErrorTest, FunctionDeclMissingLParen) {
    EXPECT("fn f) {}",
           R"(test.wgsl:1:5 error: expected '(' for function declaration
fn f) {}
    ^
)");
}

TEST_F(ParserImplErrorTest, FunctionDeclMissingRParen) {
    EXPECT("fn f( {}",
           R"(test.wgsl:1:7 error: expected ')' for function declaration
fn f( {}
      ^
)");
}

TEST_F(ParserImplErrorTest, FunctionDeclMissingArrow) {
    EXPECT("fn f() f32 {}", R"(test.wgsl:1:8 error: expected '{' for function body
fn f() f32 {}
       ^^^
)");
}

TEST_F(ParserImplErrorTest, FunctionDeclInvalidReturnType) {
    EXPECT("fn f() -> 1 {}",
           R"(test.wgsl:1:11 error: unable to determine function return type
fn f() -> 1 {}
          ^
)");
}

TEST_F(ParserImplErrorTest, FunctionDeclParamMissingColon) {
    EXPECT("fn f(x) {}", R"(test.wgsl:1:7 error: expected ':' for parameter
fn f(x) {}
      ^
)");
}

TEST_F(ParserImplErrorTest, FunctionDeclParamInvalidType) {
    EXPECT("fn f(x : 1) {}", R"(test.wgsl:1:10 error: invalid type for parameter
fn f(x : 1) {}
         ^
)");
}

TEST_F(ParserImplErrorTest, FunctionDeclParamMissing) {
    EXPECT("fn f(x : i32, ,) {}",
           R"(test.wgsl:1:15 error: expected ')' for function declaration
fn f(x : i32, ,) {}
              ^
)");
}

TEST_F(ParserImplErrorTest, FunctionDeclMissingLBrace) {
    EXPECT("fn f() }", R"(test.wgsl:1:8 error: expected '{' for function body
fn f() }
       ^
)");
}

TEST_F(ParserImplErrorTest, FunctionDeclMissingRBrace) {
    EXPECT("fn f() {", R"(test.wgsl:1:9 error: expected '}' for function body
fn f() {
        ^
)");
}

TEST_F(ParserImplErrorTest, FunctionScopeUnusedDecl) {
    EXPECT("fn f(a:i32)->i32{return a;@size(1)}",
           R"(test.wgsl:1:28 error: unexpected attributes
fn f(a:i32)->i32{return a;@size(1)}
                           ^^^^
)");
}

TEST_F(ParserImplErrorTest, FunctionMissingOpenLine) {
    EXPECT(
        R"(const bar : vec2<f32> = vec2<f32>(1., 2.);
  var a : f32 = bar[0];
  return;
})",
        R"(test.wgsl:3:3 error: statement found outside of function body
  return;
  ^^^^^^
)");
}

TEST_F(ParserImplErrorTest, GlobalDeclConstInvalidIdentifier) {
    EXPECT("const ^ : i32 = 1;",
           R"(test.wgsl:1:7 error: expected identifier for 'const' declaration
const ^ : i32 = 1;
      ^
)");
}

TEST_F(ParserImplErrorTest, GlobalDeclConstMissingSemicolon) {
    EXPECT("const i : i32 = 1",
           R"(test.wgsl:1:18 error: expected ';' for 'const' declaration
const i : i32 = 1
                 ^
)");
}

TEST_F(ParserImplErrorTest, GlobalDeclConstMissingRParen) {
    EXPECT("const i : vec2<i32> = vec2<i32>(1., 2.;",
           R"(test.wgsl:1:39 error: expected ')' for function call
const i : vec2<i32> = vec2<i32>(1., 2.;
                                      ^
)");
}

TEST_F(ParserImplErrorTest, GlobalDeclConstBadConstLiteral) {
    EXPECT("const i : vec2<i32> = vec2<i32>(!);",
           R"(test.wgsl:1:34 error: unable to parse right side of ! expression
const i : vec2<i32> = vec2<i32>(!);
                                 ^
)");
}

TEST_F(ParserImplErrorTest, GlobalDeclConstExprMaxDepth) {
    uint32_t kMaxDepth = 128;

    utils::StringStream src;
    utils::StringStream mkr;
    src << "const i : i32 = ";
    mkr << "                ";
    for (size_t i = 0; i < kMaxDepth + 8; i++) {
        src << "f32(";
        if (i < kMaxDepth) {
            mkr << "    ";
        } else if (i == kMaxDepth) {
            mkr << "^^^";
        }
    }
    src << "1.0";
    for (size_t i = 0; i < 200; i++) {
        src << ")";
    }
    src << ";";
    utils::StringStream err;
    err << "test.wgsl:1:529 error: maximum parser recursive depth reached\n"
        << src.str() << "\n"
        << mkr.str() << "\n";
    EXPECT(src.str().c_str(), err.str().c_str());
}

TEST_F(ParserImplErrorTest, GlobalDeclConstExprMissingLParen) {
    EXPECT("const i : vec2<i32> = vec2<i32> 1, 2);",
           R"(test.wgsl:1:33 error: expected ';' for 'const' declaration
const i : vec2<i32> = vec2<i32> 1, 2);
                                ^
)");
}

TEST_F(ParserImplErrorTest, GlobalDeclConstExprMissingRParen) {
    EXPECT("const i : vec2<i32> = vec2<i32>(1, 2;",
           R"(test.wgsl:1:37 error: expected ')' for function call
const i : vec2<i32> = vec2<i32>(1, 2;
                                    ^
)");
}

TEST_F(ParserImplErrorTest, GlobalDeclLet) {
    EXPECT("let a : i32 = 1;",
           R"(test.wgsl:1:1 error: module-scope 'let' is invalid, use 'const'
let a : i32 = 1;
^^^
)");
}

TEST_F(ParserImplErrorTest, GlobalDeclInvalidAttribute) {
    EXPECT("@vertex x;",
           R"(test.wgsl:1:9 error: expected declaration after attributes
@vertex x;
        ^
)");
}

TEST_F(ParserImplErrorTest, GlobalDeclSampledTextureMissingGreaterThan) {
    EXPECT("var x : texture_1d<f32;",
           R"(test.wgsl:1:19 error: expected ';' for variable declaration
var x : texture_1d<f32;
                  ^
)");
}

TEST_F(ParserImplErrorTest, GlobalDeclMultisampledTextureMissingGreaterThan) {
    EXPECT("var x : texture_multisampled_2d<f32;",
           R"(test.wgsl:1:32 error: expected ';' for variable declaration
var x : texture_multisampled_2d<f32;
                               ^
)");
}

TEST_F(ParserImplErrorTest, GlobalDeclConstAssertMissingCondThenEOF) {
    EXPECT("const_assert", R"(test.wgsl:1:13 error: unable to parse condition expression
const_assert
            ^
)");
}

TEST_F(ParserImplErrorTest, GlobalDeclConstAssertMissingCondThenSemicolon) {
    EXPECT("const_assert;", R"(test.wgsl:1:13 error: unable to parse condition expression
const_assert;
            ^
)");
}

TEST_F(ParserImplErrorTest, GlobalDeclConstAssertMissingCondThenAlias) {
    EXPECT("const_assert\nalias T = i32;",
           R"(test.wgsl:2:1 error: unable to parse condition expression
alias T = i32;
^^^^^
)");
}

TEST_F(ParserImplErrorTest, GlobalDeclConstAssertMissingLParen) {
    EXPECT("const_assert true);",
           R"(test.wgsl:1:18 error: expected ';' for const assertion declaration
const_assert true);
                 ^
)");
}

TEST_F(ParserImplErrorTest, GlobalDeclConstAssertMissingRParen) {
    EXPECT("const_assert (true;", R"(test.wgsl:1:19 error: expected ')'
const_assert (true;
                  ^
)");
}

TEST_F(ParserImplErrorTest, GlobalDeclConstAssertMissingSemicolon) {
    EXPECT("const_assert true const_assert true;",
           R"(test.wgsl:1:19 error: expected ';' for const assertion declaration
const_assert true const_assert true;
                  ^^^^^^^^^^^^
)");
}

TEST_F(ParserImplErrorTest, GlobalDeclStorageTextureMissingGreaterThan) {
    EXPECT("var x : texture_storage_2d<r32uint, read;",
           R"(test.wgsl:1:27 error: expected ';' for variable declaration
var x : texture_storage_2d<r32uint, read;
                          ^
)");
}

TEST_F(ParserImplErrorTest, GlobalDeclStorageTextureMissingSubtype) {
    EXPECT("var x : texture_storage_2d<>;",
           R"(test.wgsl:1:28 error: expected expression for type template argument list
var x : texture_storage_2d<>;
                           ^
)");
}

TEST_F(ParserImplErrorTest, GlobalDeclStructDeclMissingIdentifier) {
    EXPECT("struct {};",
           R"(test.wgsl:1:8 error: expected identifier for struct declaration
struct {};
       ^
)");
}

TEST_F(ParserImplErrorTest, GlobalDeclStructDeclMissingLBrace) {
    EXPECT("struct S };",
           R"(test.wgsl:1:10 error: expected '{' for struct declaration
struct S };
         ^
)");
}

TEST_F(ParserImplErrorTest, GlobalDeclStructDeclMissingRBrace) {
    EXPECT("struct S { i : i32,",
           R"(test.wgsl:1:20 error: expected '}' for struct declaration
struct S { i : i32,
                   ^
)");
}

TEST_F(ParserImplErrorTest, GlobalDeclStructMemberInvalidIdentifier) {
    EXPECT("struct S { 1 : i32, };",
           R"(test.wgsl:1:12 error: expected '}' for struct declaration
struct S { 1 : i32, };
           ^
)");
}

TEST_F(ParserImplErrorTest, GlobalDeclStructMemberAlignInvaldValue) {
    EXPECT("struct S { @align(fn) i : i32, };",
           R"(test.wgsl:1:19 error: expected expression for align
struct S { @align(fn) i : i32, };
                  ^^
)");
}

TEST_F(ParserImplErrorTest, GlobalDeclStructMemberSizeInvaldValue) {
    EXPECT("struct S { @size(if) i : i32, };",
           R"(test.wgsl:1:18 error: expected expression for size
struct S { @size(if) i : i32, };
                 ^^
)");
}

TEST_F(ParserImplErrorTest, GlobalDeclTypeAliasMissingIdentifier) {
    EXPECT("alias 1 = f32;",
           R"(test.wgsl:1:7 error: expected identifier for type alias
alias 1 = f32;
      ^
)");
}

TEST_F(ParserImplErrorTest, GlobalDeclTypeAliasInvalidType) {
    EXPECT("alias meow = 1;", R"(test.wgsl:1:14 error: invalid type alias
alias meow = 1;
             ^
)");
}

TEST_F(ParserImplErrorTest, GlobalDeclTypeAliasMissingAssignment) {
    EXPECT("alias meow f32", R"(test.wgsl:1:12 error: expected '=' for type alias
alias meow f32
           ^^^
)");
}

TEST_F(ParserImplErrorTest, GlobalDeclTypeAliasMissingSemicolon) {
    EXPECT("alias meow = f32", R"(test.wgsl:1:17 error: expected ';' for type alias
alias meow = f32
                ^
)");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarArrayMissingGreaterThan) {
    EXPECT("var i : array<u32, 3;",
           R"(test.wgsl:1:14 error: expected ';' for variable declaration
var i : array<u32, 3;
             ^
)");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarArrayInvalidSize) {
    EXPECT("var i : array<u32, !>;",
           R"(test.wgsl:1:21 error: unable to parse right side of ! expression
var i : array<u32, !>;
                    ^
)");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarAttrListMissingAt) {
    EXPECT("@location(1) group(2) var i : i32;",
           R"(test.wgsl:1:14 error: expected declaration after attributes
@location(1) group(2) var i : i32;
             ^^^^^

test.wgsl:1:19 error: unexpected token
@location(1) group(2) var i : i32;
                  ^
)");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarAttrLocationMissingLParen) {
    EXPECT("@location 1) var i : i32;",
           R"(test.wgsl:1:11 error: expected '(' for location attribute
@location 1) var i : i32;
          ^
)");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarAttrLocationMissingRParen) {
    EXPECT("@location (1 var i : i32;",
           R"(test.wgsl:1:14 error: expected ')' for location attribute
@location (1 var i : i32;
             ^^^
)");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarAttrLocationInvalidValue) {
    EXPECT("@location(if) var i : i32;",
           R"(test.wgsl:1:11 error: expected expression for location
@location(if) var i : i32;
          ^^
)");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarAttrIdMissingLParen) {
    EXPECT("@id 1) var i : i32;",
           R"(test.wgsl:1:5 error: expected '(' for id attribute
@id 1) var i : i32;
    ^
)");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarAttrIdMissingRParen) {
    EXPECT("@id (1 var i : i32;",
           R"(test.wgsl:1:8 error: expected ')' for id attribute
@id (1 var i : i32;
       ^^^
)");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarAttrIdInvalidValue) {
    EXPECT("@id(if) var i : i32;",
           R"(test.wgsl:1:5 error: expected expression for id
@id(if) var i : i32;
    ^^
)");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarAttrBuiltinMissingLParen) {
    EXPECT("@builtin position) var i : i32;",
           R"(test.wgsl:1:10 error: expected '(' for builtin attribute
@builtin position) var i : i32;
         ^^^^^^^^
)");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarAttrBuiltinMissingRParen) {
    EXPECT("@builtin(position var i : i32;",
           R"(test.wgsl:1:19 error: expected ')' for builtin attribute
@builtin(position var i : i32;
                  ^^^
)");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarAttrBindingMissingLParen) {
    EXPECT("@binding 1) var i : i32;",
           R"(test.wgsl:1:10 error: expected '(' for binding attribute
@binding 1) var i : i32;
         ^
)");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarAttrBindingMissingRParen) {
    EXPECT("@binding(1 var i : i32;",
           R"(test.wgsl:1:12 error: expected ')' for binding attribute
@binding(1 var i : i32;
           ^^^
)");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarAttrBindingInvalidValue) {
    EXPECT("@binding(if) var i : i32;",
           R"(test.wgsl:1:10 error: expected expression for binding
@binding(if) var i : i32;
         ^^
)");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarAttrGroupMissingLParen) {
    EXPECT("@group 1) var i : i32;",
           R"(test.wgsl:1:8 error: expected '(' for group attribute
@group 1) var i : i32;
       ^
)");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarAttrGroupMissingRParen) {
    EXPECT("@group(1 var i : i32;",
           R"(test.wgsl:1:10 error: expected ')' for group attribute
@group(1 var i : i32;
         ^^^
)");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarAttrBindingGroupValue) {
    EXPECT("@group(if) var i : i32;",
           R"(test.wgsl:1:8 error: expected expression for group
@group(if) var i : i32;
       ^^
)");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarInvalidIdentifier) {
    EXPECT("var ^ : mat4x4;",
           R"(test.wgsl:1:5 error: expected identifier for variable declaration
var ^ : mat4x4;
    ^
)");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarMatrixMissingGreaterThan) {
    EXPECT("var i : mat4x4<u32;", R"(test.wgsl:1:15 error: expected ';' for variable declaration
var i : mat4x4<u32;
              ^
)");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarMissingSemicolon) {
    EXPECT("var i : i32",
           R"(test.wgsl:1:12 error: expected ';' for variable declaration
var i : i32
           ^
)");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarPtrMissingGreaterThan) {
    EXPECT("var i : ptr<private, u32;",
           R"(test.wgsl:1:12 error: expected ';' for variable declaration
var i : ptr<private, u32;
           ^
)");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarStorageDeclMissingGThan) {
    EXPECT("var<private i : i32",
           R"(test.wgsl:1:4 error: missing closing '>' for variable declaration
var<private i : i32
   ^
)");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarVectorMissingGreaterThan) {
    EXPECT("var i : vec3<u32;", R"(test.wgsl:1:13 error: expected ';' for variable declaration
var i : vec3<u32;
            ^
)");
}

TEST_F(ParserImplErrorTest, IfStmtMissingRParen) {
    EXPECT("fn f() { if (true {} }", R"(test.wgsl:1:19 error: expected ')'
fn f() { if (true {} }
                  ^
)");
}

TEST_F(ParserImplErrorTest, IfStmtInvalidCond) {
    EXPECT("fn f() { if (>) {} }",
           R"(test.wgsl:1:14 error: unable to parse expression
fn f() { if (>) {} }
             ^
)");
}

TEST_F(ParserImplErrorTest, LogicalAndInvalidExpr) {
    EXPECT("fn f() { return 1 && >; }",
           R"(test.wgsl:1:22 error: unable to parse right side of && expression
fn f() { return 1 && >; }
                     ^
)");
}

TEST_F(ParserImplErrorTest, LogicalOrInvalidExpr) {
    EXPECT("fn f() { return 1 || >; }",
           R"(test.wgsl:1:22 error: unable to parse right side of || expression
fn f() { return 1 || >; }
                     ^
)");
}

TEST_F(ParserImplErrorTest, LoopMissingLBrace) {
    EXPECT("fn f() { loop }", R"(test.wgsl:1:15 error: expected '{' for loop
fn f() { loop }
              ^
)");
}

TEST_F(ParserImplErrorTest, LoopMissingRBrace) {
    EXPECT("fn f() { loop {", R"(test.wgsl:1:16 error: expected '}' for loop
fn f() { loop {
               ^
)");
}

TEST_F(ParserImplErrorTest, MaxErrorsReached) {
    EXPECT("x; x; x; x; x; x; x; x;", R"(test.wgsl:1:1 error: unexpected token
x; x; x; x; x; x; x; x;
^

test.wgsl:1:4 error: unexpected token
x; x; x; x; x; x; x; x;
   ^

test.wgsl:1:7 error: unexpected token
x; x; x; x; x; x; x; x;
      ^

test.wgsl:1:10 error: unexpected token
x; x; x; x; x; x; x; x;
         ^

test.wgsl:1:13 error: unexpected token
x; x; x; x; x; x; x; x;
            ^

test.wgsl error: stopping after 5 errors)");
}

TEST_F(ParserImplErrorTest, MemberExprMissingIdentifier) {
    EXPECT("fn f() { x = a.; }",
           R"(test.wgsl:1:16 error: expected identifier for member accessor
fn f() { x = a.; }
               ^
)");
}

TEST_F(ParserImplErrorTest, MultiplicativeInvalidExpr) {
    EXPECT("fn f() { return 1.0 * <; }",
           R"(test.wgsl:1:23 error: unable to parse right side of * expression
fn f() { return 1.0 * <; }
                      ^
)");
}

TEST_F(ParserImplErrorTest, OrInvalidExpr) {
    EXPECT("fn f() { return 1 | >; }",
           R"(test.wgsl:1:21 error: unable to parse right side of | expression
fn f() { return 1 | >; }
                    ^
)");
}

TEST_F(ParserImplErrorTest, PostfixIncrementAsExpr) {
    EXPECT("fn f() { var x : i32; let y = x++; }",
           R"(test.wgsl:1:32 error: expected ';' for variable declaration
fn f() { var x : i32; let y = x++; }
                               ^^
)");
}

TEST_F(ParserImplErrorTest, RelationalInvalidExpr) {
    EXPECT("fn f() { return 1 < >; }",
           R"(test.wgsl:1:21 error: unable to parse right side of < expression
fn f() { return 1 < >; }
                    ^
)");
}

TEST_F(ParserImplErrorTest, ReturnStmtMissingSemicolon) {
    EXPECT("fn f() { return }",
           R"(test.wgsl:1:17 error: expected ';' for return statement
fn f() { return }
                ^
)");
}

TEST_F(ParserImplErrorTest, ShiftInvalidExpr) {
    EXPECT("fn f() { return 1 << >; }",
           R"(test.wgsl:1:22 error: unable to parse right side of << expression
fn f() { return 1 << >; }
                     ^
)");
}

TEST_F(ParserImplErrorTest, SwitchStmtMissingLBrace) {
    EXPECT("fn f() { switch(1) }",
           R"(test.wgsl:1:20 error: expected '{' for switch statement
fn f() { switch(1) }
                   ^
)");
}

TEST_F(ParserImplErrorTest, SwitchStmtMissingRBrace) {
    EXPECT("fn f() { switch(1) {",
           R"(test.wgsl:1:21 error: expected '}' for switch statement
fn f() { switch(1) {
                    ^
)");
}

TEST_F(ParserImplErrorTest, SwitchStmtInvalidCase) {
    EXPECT("fn f() { switch(1) { case ^: } }",
           R"(test.wgsl:1:27 error: expected case selector expression or `default`
fn f() { switch(1) { case ^: } }
                          ^
)");
}

TEST_F(ParserImplErrorTest, SwitchStmtCaseMissingLBrace) {
    EXPECT("fn f() { switch(1) { case 1: } }",
           R"(test.wgsl:1:30 error: expected '{' for case statement
fn f() { switch(1) { case 1: } }
                             ^
)");
}

TEST_F(ParserImplErrorTest, SwitchStmtCaseMissingRBrace) {
    EXPECT("fn f() { switch(1) { case 1: {",
           R"(test.wgsl:1:31 error: expected '}' for case statement
fn f() { switch(1) { case 1: {
                              ^
)");
}

TEST_F(ParserImplErrorTest, VarStmtMissingSemicolon) {
    EXPECT("fn f() { var a : u32 }",
           R"(test.wgsl:1:22 error: expected ';' for variable declaration
fn f() { var a : u32 }
                     ^
)");
}

TEST_F(ParserImplErrorTest, VarStmtInvalidAssignment) {
    EXPECT("fn f() { var a : u32 = >; }",
           R"(test.wgsl:1:24 error: missing initializer for 'var' declaration
fn f() { var a : u32 = >; }
                       ^
)");
}

TEST_F(ParserImplErrorTest, UnaryInvalidExpr) {
    EXPECT("fn f() { return !<; }",
           R"(test.wgsl:1:18 error: unable to parse right side of ! expression
fn f() { return !<; }
                 ^
)");
}

TEST_F(ParserImplErrorTest, UnexpectedToken) {
    EXPECT("unexpected", R"(test.wgsl:1:1 error: unexpected token
unexpected
^^^^^^^^^^
)");
}

TEST_F(ParserImplErrorTest, XorInvalidExpr) {
    EXPECT("fn f() { return 1 ^ >; }",
           R"(test.wgsl:1:21 error: unable to parse right side of ^ expression
fn f() { return 1 ^ >; }
                    ^
)");
}

TEST_F(ParserImplErrorTest, InvalidUTF8) {
    EXPECT("fn fu\xd0nc() {}",
           "test.wgsl:1:4 error: invalid UTF-8\n"
           "fn fu\xD0nc() {}\n");
}

TEST_F(ParserImplErrorTest, Bug_Chromium_1417465) {
    EXPECT("var<workgroup> vec4_data: array<mat4x4<f@32>, 256>;",
           R"(test.wgsl:1:41 error: expected ',' for template argument list
var<workgroup> vec4_data: array<mat4x4<f@32>, 256>;
                                        ^
)");
}

}  // namespace
}  // namespace tint::reader::wgsl
