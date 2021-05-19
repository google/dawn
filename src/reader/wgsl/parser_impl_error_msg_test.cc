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

#include "src/reader/wgsl/parser_impl_test_helper.h"

namespace tint {
namespace reader {
namespace wgsl {
namespace {

const diag::Formatter::Style formatter_style{
    /* print_file: */ true, /* print_severity: */ true,
    /* print_line: */ true, /* print_newline_at_end: */ false};

class ParserImplErrorTest : public ParserImplTest {};

#define EXPECT(SOURCE, EXPECTED)                                               \
  do {                                                                         \
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
         "test.wgsl:1:23 error: unable to parse right side of + expression\n"
         "fn f() { return 1.0 + <; }\n"
         "                      ^\n");
}

TEST_F(ParserImplErrorTest, AndInvalidExpr) {
  EXPECT("fn f() { return 1 & >; }",
         "test.wgsl:1:21 error: unable to parse right side of & expression\n"
         "fn f() { return 1 & >; }\n"
         "                    ^\n");
}

TEST_F(ParserImplErrorTest, ArrayIndexExprInvalidExpr) {
  EXPECT("fn f() { x = y[^]; }",
         "test.wgsl:1:16 error: unable to parse expression inside []\n"
         "fn f() { x = y[^]; }\n"
         "               ^\n");
}

TEST_F(ParserImplErrorTest, ArrayIndexExprMissingRBracket) {
  EXPECT("fn f() { x = y[1; }",
         "test.wgsl:1:17 error: expected ']' for array accessor\n"
         "fn f() { x = y[1; }\n"
         "                ^\n");
}

TEST_F(ParserImplErrorTest, AssignmentStmtMissingAssignment) {
  EXPECT("fn f() { a; }",
         "test.wgsl:1:11 error: expected '=' for assignment\n"
         "fn f() { a; }\n"
         "          ^\n");
}

TEST_F(ParserImplErrorTest, AssignmentStmtMissingAssignment2) {
  EXPECT("fn f() { a : i32; }",
         "test.wgsl:1:10 error: expected 'var' for variable declaration\n"
         "fn f() { a : i32; }\n"
         "         ^\n");
}

TEST_F(ParserImplErrorTest, AssignmentStmtMissingSemicolon) {
  EXPECT("fn f() { a = 1 }",
         "test.wgsl:1:16 error: expected ';' for assignment statement\n"
         "fn f() { a = 1 }\n"
         "               ^\n");
}

TEST_F(ParserImplErrorTest, AssignmentStmtInvalidRHS) {
  EXPECT("fn f() { a = >; }",
         "test.wgsl:1:14 error: unable to parse right side of assignment\n"
         "fn f() { a = >; }\n"
         "             ^\n");
}

TEST_F(ParserImplErrorTest, BitcastExprMissingLessThan) {
  EXPECT("fn f() { x = bitcast(y); }",
         "test.wgsl:1:21 error: expected '<' for bitcast expression\n"
         "fn f() { x = bitcast(y); }\n"
         "                    ^\n");
}

TEST_F(ParserImplErrorTest, BitcastExprMissingGreaterThan) {
  EXPECT("fn f() { x = bitcast<u32(y); }",
         "test.wgsl:1:25 error: expected '>' for bitcast expression\n"
         "fn f() { x = bitcast<u32(y); }\n"
         "                        ^\n");
}

TEST_F(ParserImplErrorTest, BitcastExprMissingType) {
  EXPECT("fn f() { x = bitcast<>(y); }",
         "test.wgsl:1:22 error: invalid type for bitcast expression\n"
         "fn f() { x = bitcast<>(y); }\n"
         "                     ^\n");
}

TEST_F(ParserImplErrorTest, BreakStmtMissingSemicolon) {
  EXPECT("fn f() { loop { break } }",
         "test.wgsl:1:23 error: expected ';' for break statement\n"
         "fn f() { loop { break } }\n"
         "                      ^\n");
}

TEST_F(ParserImplErrorTest, CallExprMissingRParen) {
  EXPECT("fn f() { x = f(1.; }",
         "test.wgsl:1:18 error: expected ')' for function call\n"
         "fn f() { x = f(1.; }\n"
         "                 ^\n");
}

TEST_F(ParserImplErrorTest, CallStmtMissingRParen) {
  EXPECT("fn f() { f(1.; }",
         "test.wgsl:1:14 error: expected ')' for function call\n"
         "fn f() { f(1.; }\n"
         "             ^\n");
}

TEST_F(ParserImplErrorTest, CallStmtInvalidArgument0) {
  EXPECT("fn f() { f(<); }",
         "test.wgsl:1:12 error: expected ')' for function call\n"
         "fn f() { f(<); }\n"
         "           ^\n");
}

TEST_F(ParserImplErrorTest, CallStmtInvalidArgument1) {
  EXPECT("fn f() { f(1.0, <); }",
         "test.wgsl:1:17 error: expected ')' for function call\n"
         "fn f() { f(1.0, <); }\n"
         "                ^\n");
}

TEST_F(ParserImplErrorTest, CallStmtMissingSemicolon) {
  EXPECT("fn f() { f() }",
         "test.wgsl:1:14 error: expected ';' for function call\n"
         "fn f() { f() }\n"
         "             ^\n");
}

TEST_F(ParserImplErrorTest, ConstructorExprMissingLParen) {
  EXPECT("fn f() { x = vec2<u32>1,2); }",
         "test.wgsl:1:23 error: expected '(' for type constructor\n"
         "fn f() { x = vec2<u32>1,2); }\n"
         "                      ^\n");
}

TEST_F(ParserImplErrorTest, ConstructorExprMissingRParen) {
  EXPECT("fn f() { x = vec2<u32>(1,2; }",
         "test.wgsl:1:27 error: expected ')' for type constructor\n"
         "fn f() { x = vec2<u32>(1,2; }\n"
         "                          ^\n");
}

TEST_F(ParserImplErrorTest, ConstVarStmtInvalid) {
  EXPECT("fn f() { let >; }",
         "test.wgsl:1:14 error: expected identifier for let declaration\n"
         "fn f() { let >; }\n"
         "             ^\n");
}

TEST_F(ParserImplErrorTest, ConstVarStmtMissingAssignment) {
  EXPECT("fn f() { let a : i32; }",
         "test.wgsl:1:21 error: expected '=' for let declaration\n"
         "fn f() { let a : i32; }\n"
         "                    ^\n");
}

TEST_F(ParserImplErrorTest, ConstVarStmtMissingConstructor) {
  EXPECT("fn f() { let a : i32 = >; }",
         "test.wgsl:1:24 error: missing constructor for let declaration\n"
         "fn f() { let a : i32 = >; }\n"
         "                       ^\n");
}

TEST_F(ParserImplErrorTest, ContinueStmtMissingSemicolon) {
  EXPECT("fn f() { loop { continue } }",
         "test.wgsl:1:26 error: expected ';' for continue statement\n"
         "fn f() { loop { continue } }\n"
         "                         ^\n");
}

TEST_F(ParserImplErrorTest, DiscardStmtMissingSemicolon) {
  EXPECT("fn f() { discard }",
         "test.wgsl:1:18 error: expected ';' for discard statement\n"
         "fn f() { discard }\n"
         "                 ^\n");
}

TEST_F(ParserImplErrorTest, EqualityInvalidExpr) {
  EXPECT("fn f() { return 1 == >; }",
         "test.wgsl:1:22 error: unable to parse right side of == expression\n"
         "fn f() { return 1 == >; }\n"
         "                     ^\n");
}

TEST_F(ParserImplErrorTest, FloatLiteralSuffixedWithF) {
  EXPECT("var f : f32 = 1.23f;",
         "test.wgsl:1:19 error: float literals must not be suffixed with 'f'\n"
         "var f : f32 = 1.23f;\n"
         "                  ^\n");
}

TEST_F(ParserImplErrorTest, ForLoopInitializerMissingSemicolon) {
  EXPECT("fn f() { for (var i : i32 = 0 i < 8; i=i+1) {} }",
         "test.wgsl:1:31 error: expected ';' for initializer in for loop\n"
         "fn f() { for (var i : i32 = 0 i < 8; i=i+1) {} }\n"
         "                              ^\n");
}

TEST_F(ParserImplErrorTest, ForLoopInitializerMissingVar) {
  EXPECT("fn f() { for (i : i32 = 0; i < 8; i=i+1) {} }",
         "test.wgsl:1:15 error: expected 'var' for variable declaration\n"
         "fn f() { for (i : i32 = 0; i < 8; i=i+1) {} }\n"
         "              ^\n");
}

TEST_F(ParserImplErrorTest, ForLoopConditionMissingSemicolon) {
  EXPECT("fn f() { for (var i : i32 = 0; i < 8 i=i+1) {} }",
         "test.wgsl:1:38 error: expected ';' for condition in for loop\n"
         "fn f() { for (var i : i32 = 0; i < 8 i=i+1) {} }\n"
         "                                     ^\n");
}

TEST_F(ParserImplErrorTest, ForLoopMissingLParen) {
  EXPECT("fn f() { for var i : i32 = 0; i < 8; i=i+1) {} }",
         "test.wgsl:1:14 error: expected '(' for for loop\n"
         "fn f() { for var i : i32 = 0; i < 8; i=i+1) {} }\n"
         "             ^^^\n");
}

TEST_F(ParserImplErrorTest, ForLoopMissingRParen) {
  EXPECT("fn f() { for (var i : i32 = 0; i < 8; i=i+1 {} }",
         "test.wgsl:1:45 error: expected ')' for for loop\n"
         "fn f() { for (var i : i32 = 0; i < 8; i=i+1 {} }\n"
         "                                            ^\n");
}

TEST_F(ParserImplErrorTest, ForLoopMissingLBrace) {
  EXPECT("fn f() { for (var i : i32 = 0; i < 8; i=i+1) }",
         "test.wgsl:1:46 error: expected '{' for for loop\n"
         "fn f() { for (var i : i32 = 0; i < 8; i=i+1) }\n"
         "                                             ^\n");
}

TEST_F(ParserImplErrorTest, ForLoopMissingRBrace) {
  EXPECT("fn f() { for (var i : i32 = 0; i < 8; i=i+1) {",
         "test.wgsl:1:47 error: expected '}' for for loop\n"
         "fn f() { for (var i : i32 = 0; i < 8; i=i+1) {\n"
         "                                              ^\n");
}

TEST_F(ParserImplErrorTest, FunctionDeclInvalid) {
  EXPECT("[[stage(vertex)]] x;",
         "test.wgsl:1:19 error: expected declaration after decorations\n"
         "[[stage(vertex)]] x;\n"
         "                  ^\n");
}

TEST_F(ParserImplErrorTest, FunctionDeclDecoMissingEnd) {
  EXPECT("[[stage(vertex) fn f() {}",
         "test.wgsl:1:17 error: expected ']]' for decoration list\n"
         "[[stage(vertex) fn f() {}\n"
         "                ^^\n");
}

TEST_F(ParserImplErrorTest, FunctionDeclDecoStageMissingLParen) {
  EXPECT("[[stage vertex]] fn f() {}",
         "test.wgsl:1:9 error: expected '(' for stage decoration\n"
         "[[stage vertex]] fn f() {}\n"
         "        ^^^^^^\n");
}

TEST_F(ParserImplErrorTest, FunctionDeclDecoStageMissingRParen) {
  EXPECT("[[stage(vertex]] fn f() {}",
         "test.wgsl:1:15 error: expected ')' for stage decoration\n"
         "[[stage(vertex]] fn f() {}\n"
         "              ^^\n");
}

TEST_F(ParserImplErrorTest, FunctionDeclDecoStageInvalid) {
  EXPECT("[[stage(x)]] fn f() {}",
         "test.wgsl:1:9 error: invalid value for stage decoration\n"
         "[[stage(x)]] fn f() {}\n"
         "        ^\n");
}

TEST_F(ParserImplErrorTest, FunctionDeclDecoStageTypeInvalid) {
  EXPECT("[[shader(vertex)]] fn main() {}",
         "test.wgsl:1:3 error: expected decoration\n"
         "[[shader(vertex)]] fn main() {}\n"
         "  ^^^^^^\n");
}

TEST_F(ParserImplErrorTest, FunctionDeclDecoWorkgroupSizeMissingLParen) {
  EXPECT("[[workgroup_size 1]] fn f() {}",
         "test.wgsl:1:18 error: expected '(' for workgroup_size decoration\n"
         "[[workgroup_size 1]] fn f() {}\n"
         "                 ^\n");
}

TEST_F(ParserImplErrorTest, FunctionDeclDecoWorkgroupSizeMissingRParen) {
  EXPECT("[[workgroup_size(1]] fn f() {}",
         "test.wgsl:1:19 error: expected ')' for workgroup_size decoration\n"
         "[[workgroup_size(1]] fn f() {}\n"
         "                  ^^\n");
}

TEST_F(ParserImplErrorTest, FunctionDeclDecoWorkgroupSizeXInvalid) {
  EXPECT("[[workgroup_size(@)]] fn f() {}",
         "test.wgsl:1:18 error: expected workgroup_size x parameter\n"
         "[[workgroup_size(@)]] fn f() {}\n"
         "                 ^\n");
}

TEST_F(ParserImplErrorTest, FunctionDeclDecoWorkgroupSizeYInvalid) {
  EXPECT("[[workgroup_size(1, @)]] fn f() {}",
         "test.wgsl:1:21 error: expected workgroup_size y parameter\n"
         "[[workgroup_size(1, @)]] fn f() {}\n"
         "                    ^\n");
}

TEST_F(ParserImplErrorTest, FunctionDeclDecoWorkgroupSizeZInvalid) {
  EXPECT("[[workgroup_size(1, 2, @)]] fn f() {}",
         "test.wgsl:1:24 error: expected workgroup_size z parameter\n"
         "[[workgroup_size(1, 2, @)]] fn f() {}\n"
         "                       ^\n");
}

TEST_F(ParserImplErrorTest, FunctionDeclMissingIdentifier) {
  EXPECT("fn () {}",
         "test.wgsl:1:4 error: expected identifier for function declaration\n"
         "fn () {}\n"
         "   ^\n");
}

TEST_F(ParserImplErrorTest, FunctionDeclMissingLParen) {
  EXPECT("fn f) {}",
         "test.wgsl:1:5 error: expected '(' for function declaration\n"
         "fn f) {}\n"
         "    ^\n");
}

TEST_F(ParserImplErrorTest, FunctionDeclMissingRParen) {
  EXPECT("fn f( {}",
         "test.wgsl:1:7 error: expected ')' for function declaration\n"
         "fn f( {}\n"
         "      ^\n");
}

TEST_F(ParserImplErrorTest, FunctionDeclMissingArrow) {
  EXPECT("fn f() void {}",
         "test.wgsl:1:8 error: expected '{'\n"
         "fn f() void {}\n"
         "       ^^^^\n");
}

TEST_F(ParserImplErrorTest, FunctionDeclInvalidReturnType) {
  EXPECT("fn f() -> 1 {}",
         "test.wgsl:1:11 error: unable to determine function return type\n"
         "fn f() -> 1 {}\n"
         "          ^\n");
}

TEST_F(ParserImplErrorTest, FunctionDeclParamMissingColon) {
  EXPECT("fn f(x) {}",
         "test.wgsl:1:7 error: expected ':' for parameter\n"
         "fn f(x) {}\n"
         "      ^\n");
}

TEST_F(ParserImplErrorTest, FunctionDeclParamInvalidType) {
  EXPECT("fn f(x : 1) {}",
         "test.wgsl:1:10 error: invalid type for parameter\n"
         "fn f(x : 1) {}\n"
         "         ^\n");
}

TEST_F(ParserImplErrorTest, FunctionDeclParamMissing) {
  EXPECT("fn f(x : i32, ,) {}",
         "test.wgsl:1:15 error: expected ')' for function declaration\n"
         "fn f(x : i32, ,) {}\n"
         "              ^\n");
}

TEST_F(ParserImplErrorTest, FunctionDeclMissingLBrace) {
  EXPECT("fn f() }",
         "test.wgsl:1:8 error: expected '{'\n"
         "fn f() }\n"
         "       ^\n");
}

TEST_F(ParserImplErrorTest, FunctionDeclMissingRBrace) {
  EXPECT("fn f() {",
         "test.wgsl:1:9 error: expected '}'\n"
         "fn f() {\n"
         "        ^\n");
}

TEST_F(ParserImplErrorTest, FunctionMissingOpenLine) {
  EXPECT(R"(let bar : vec2<f32> = vec2<f32>(1., 2.);
  var a : f32 = bar[0];
  return;
})",
         "test.wgsl:2:17 error: unknown constructed type 'bar'\n"
         "  var a : f32 = bar[0];\n"
         "                ^^^\n"
         "\n"
         "test.wgsl:3:3 error: statement found outside of function body\n"
         "  return;\n"
         "  ^^^^^^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclConstInvalidIdentifier) {
  EXPECT("let ^ : i32 = 1;",
         "test.wgsl:1:5 error: expected identifier for let declaration\n"
         "let ^ : i32 = 1;\n"
         "    ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclConstMissingSemicolon) {
  EXPECT("let i : i32 = 1",
         "test.wgsl:1:16 error: expected ';' for let declaration\n"
         "let i : i32 = 1\n"
         "               ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclConstMissingLParen) {
  EXPECT("let i : vec2<i32> = vec2<i32>;",
         "test.wgsl:1:30 error: expected '(' for type constructor\n"
         "let i : vec2<i32> = vec2<i32>;\n"
         "                             ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclConstMissingRParen) {
  EXPECT("let i : vec2<i32> = vec2<i32>(1., 2.;",
         "test.wgsl:1:37 error: expected ')' for type constructor\n"
         "let i : vec2<i32> = vec2<i32>(1., 2.;\n"
         "                                    ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclConstBadConstLiteral) {
  EXPECT("let i : vec2<i32> = vec2<i32>(!);",
         "test.wgsl:1:31 error: unable to parse constant literal\n"
         "let i : vec2<i32> = vec2<i32>(!);\n"
         "                              ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclConstExprMaxDepth) {
  uint32_t kMaxDepth = 128;

  std::stringstream src;
  std::stringstream mkr;
  src << "let i : i32 = ";
  mkr << "              ";
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
  std::stringstream err;
  err << "test.wgsl:1:527 error: maximum parser recursive depth reached\n"
      << src.str() << "\n"
      << mkr.str() << "\n";
  EXPECT(src.str().c_str(), err.str().c_str());
}

TEST_F(ParserImplErrorTest, GlobalDeclConstExprMissingLParen) {
  EXPECT("let i : vec2<i32> = vec2<i32> 1, 2);",
         "test.wgsl:1:31 error: expected '(' for type constructor\n"
         "let i : vec2<i32> = vec2<i32> 1, 2);\n"
         "                              ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclConstExprMissingRParen) {
  EXPECT("let i : vec2<i32> = vec2<i32>(1, 2;",
         "test.wgsl:1:35 error: expected ')' for type constructor\n"
         "let i : vec2<i32> = vec2<i32>(1, 2;\n"
         "                                  ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclSampledTextureMissingLessThan) {
  EXPECT("var x : texture_1d;",
         "test.wgsl:1:19 error: expected '<' for sampled texture type\n"
         "var x : texture_1d;\n"
         "                  ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclSampledTextureMissingGreaterThan) {
  EXPECT("var x : texture_1d<f32;",
         "test.wgsl:1:23 error: expected '>' for sampled texture type\n"
         "var x : texture_1d<f32;\n"
         "                      ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclSampledTextureInvalidSubtype) {
  EXPECT("var x : texture_1d<1>;",
         "test.wgsl:1:20 error: invalid type for sampled texture type\n"
         "var x : texture_1d<1>;\n"
         "                   ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclMultisampledTextureMissingLessThan) {
  EXPECT("var x : texture_multisampled_2d;",
         "test.wgsl:1:32 error: expected '<' for multisampled texture type\n"
         "var x : texture_multisampled_2d;\n"
         "                               ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclMultisampledTextureMissingGreaterThan) {
  EXPECT("var x : texture_multisampled_2d<f32;",
         "test.wgsl:1:36 error: expected '>' for multisampled texture type\n"
         "var x : texture_multisampled_2d<f32;\n"
         "                                   ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclMultisampledTextureInvalidSubtype) {
  EXPECT("var x : texture_multisampled_2d<1>;",
         "test.wgsl:1:33 error: invalid type for multisampled texture type\n"
         "var x : texture_multisampled_2d<1>;\n"
         "                                ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclStorageTextureMissingLessThan) {
  EXPECT("var x : [[access(read)]] texture_storage_2d;",
         "test.wgsl:1:44 error: expected '<' for storage texture type\n"
         "var x : [[access(read)]] texture_storage_2d;\n"
         "                                           ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclStorageTextureMissingGreaterThan) {
  EXPECT("var x : [[access(read)]] texture_storage_2d<r8uint;",
         "test.wgsl:1:51 error: expected '>' for storage texture type\n"
         "var x : [[access(read)]] texture_storage_2d<r8uint;\n"
         "                                                  ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclStorageTextureMissingSubtype) {
  EXPECT("var x : [[access(read)]] texture_storage_2d<>;",
         "test.wgsl:1:45 error: invalid format for storage texture type\n"
         "var x : [[access(read)]] texture_storage_2d<>;\n"
         "                                            ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclStorageTextureMissingInvalidSubtype) {
  EXPECT("var x : [[access(read)]] texture_storage_2d<1>;",
         "test.wgsl:1:45 error: invalid format for storage texture type\n"
         "var x : [[access(read)]] texture_storage_2d<1>;\n"
         "                                            ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclStructDecoMissingStruct) {
  EXPECT("[[block]];",
         "test.wgsl:1:10 error: expected declaration after decorations\n"
         "[[block]];\n"
         "         ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclStructDecoMissingEnd) {
  EXPECT("[[block struct {};",
         "test.wgsl:1:9 error: expected ']]' for decoration list\n"
         "[[block struct {};\n"
         "        ^^^^^^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclStructDeclMissingIdentifier) {
  EXPECT("struct {};",
         "test.wgsl:1:8 error: expected identifier for struct declaration\n"
         "struct {};\n"
         "       ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclStructDeclMissingSemicolon) {
  EXPECT("struct S {}",
         "test.wgsl:1:12 error: expected ';' for struct declaration\n"
         "struct S {}\n"
         "           ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclStructDeclMissingLBrace) {
  EXPECT("struct S };",
         "test.wgsl:1:10 error: expected '{' for struct declaration\n"
         "struct S };\n"
         "         ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclStructDeclMissingRBrace) {
  EXPECT("struct S { i : i32;",
         "test.wgsl:1:20 error: expected '}' for struct declaration\n"
         "struct S { i : i32;\n"
         "                   ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclStructMemberDecoEmpty) {
  EXPECT("struct S { [[]] i : i32; };",
         "test.wgsl:1:14 error: empty decoration list\n"
         "struct S { [[]] i : i32; };\n"
         "             ^^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclStructMemberDecoMissingEnd) {
  EXPECT("struct S { [[ i : i32; };",
         "test.wgsl:1:15 error: expected decoration\n"
         "struct S { [[ i : i32; };\n"
         "              ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclStructMemberInvalidIdentifier) {
  EXPECT("struct S { 1 : i32; };",
         "test.wgsl:1:12 error: expected identifier for struct member\n"
         "struct S { 1 : i32; };\n"
         "           ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclStructMemberMissingSemicolon) {
  EXPECT("struct S { i : i32 };",
         "test.wgsl:1:20 error: expected ';' for struct member\n"
         "struct S { i : i32 };\n"
         "                   ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclStructMemberAlignMissingLParen) {
  EXPECT("struct S { [[align 1)]] i : i32; };",
         "test.wgsl:1:20 error: expected '(' for align decoration\n"
         "struct S { [[align 1)]] i : i32; };\n"
         "                   ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclStructMemberAlignMissingRParen) {
  EXPECT("struct S { [[align(1]] i : i32; };",
         "test.wgsl:1:21 error: expected ')' for align decoration\n"
         "struct S { [[align(1]] i : i32; };\n"
         "                    ^^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclStructMemberAlignInvaldValue) {
  EXPECT("struct S { [[align(x)]] i : i32; };",
         "test.wgsl:1:20 error: expected signed integer literal for align "
         "decoration\n"
         "struct S { [[align(x)]] i : i32; };\n"
         "                   ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclStructMemberAlignNegativeValue) {
  EXPECT("struct S { [[align(-2)]] i : i32; };",
         "test.wgsl:1:20 error: align decoration must be positive\n"
         "struct S { [[align(-2)]] i : i32; };\n"
         "                   ^^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclStructMemberSizeMissingLParen) {
  EXPECT("struct S { [[size 1)]] i : i32; };",
         "test.wgsl:1:19 error: expected '(' for size decoration\n"
         "struct S { [[size 1)]] i : i32; };\n"
         "                  ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclStructMemberSizeMissingRParen) {
  EXPECT("struct S { [[size(1]] i : i32; };",
         "test.wgsl:1:20 error: expected ')' for size decoration\n"
         "struct S { [[size(1]] i : i32; };\n"
         "                   ^^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclStructMemberSizeInvaldValue) {
  EXPECT("struct S { [[size(x)]] i : i32; };",
         "test.wgsl:1:19 error: expected signed integer literal for size "
         "decoration\n"
         "struct S { [[size(x)]] i : i32; };\n"
         "                  ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclStructMemberSizeNegativeValue) {
  EXPECT("struct S { [[size(-2)]] i : i32; };",
         "test.wgsl:1:19 error: size decoration must be positive\n"
         "struct S { [[size(-2)]] i : i32; };\n"
         "                  ^^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclTypeAliasMissingIdentifier) {
  EXPECT("type 1 = f32;",
         "test.wgsl:1:6 error: expected identifier for type alias\n"
         "type 1 = f32;\n"
         "     ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclTypeAliasInvalidType) {
  EXPECT("type meow = 1;",
         "test.wgsl:1:13 error: invalid type alias\n"
         "type meow = 1;\n"
         "            ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclTypeAliasMissingAssignment) {
  EXPECT("type meow f32",
         "test.wgsl:1:11 error: expected '=' for type alias\n"
         "type meow f32\n"
         "          ^^^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclTypeAliasMissingSemicolon) {
  EXPECT("type meow = f32",
         "test.wgsl:1:16 error: expected ';' for type alias\n"
         "type meow = f32\n"
         "               ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclTypeInvalid) {
  EXPECT("var x : fish;",
         "test.wgsl:1:9 error: unknown constructed type 'fish'\n"
         "var x : fish;\n"
         "        ^^^^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclTypeDecoInvalid) {
  EXPECT("var x : [[]] i32;",
         "test.wgsl:1:11 error: empty decoration list\n"
         "var x : [[]] i32;\n"
         "          ^^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarArrayMissingLessThan) {
  EXPECT("var i : array;",
         "test.wgsl:1:14 error: expected '<' for array declaration\n"
         "var i : array;\n"
         "             ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarArrayMissingGreaterThan) {
  EXPECT("var i : array<u32, 3;",
         "test.wgsl:1:21 error: expected '>' for array declaration\n"
         "var i : array<u32, 3;\n"
         "                    ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarArrayDecoNotArray) {
  EXPECT("var i : [[stride(1)]] i32;",
         "test.wgsl:1:11 error: unexpected decorations\n"
         "var i : [[stride(1)]] i32;\n"
         "          ^^^^^^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarArrayDecoMissingEnd) {
  EXPECT("var i : [[stride(1) array<i32>;",
         "test.wgsl:1:21 error: expected ']]' for decoration list\n"
         "var i : [[stride(1) array<i32>;\n"
         "                    ^^^^^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarArrayDecoStrideMissingLParen) {
  EXPECT("var i : [[stride 1)]] array<i32>;",
         "test.wgsl:1:18 error: expected '(' for stride decoration\n"
         "var i : [[stride 1)]] array<i32>;\n"
         "                 ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarArrayDecoStrideMissingRParen) {
  EXPECT("var i : [[stride(1]] array<i32>;",
         "test.wgsl:1:19 error: expected ')' for stride decoration\n"
         "var i : [[stride(1]] array<i32>;\n"
         "                  ^^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarArrayDecoStrideInvalid) {
  EXPECT("var i : [[stride(x)]] array<i32>;",
         "test.wgsl:1:18 error: expected signed integer literal for stride "
         "decoration\n"
         "var i : [[stride(x)]] array<i32>;\n"
         "                 ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarArrayDecoStrideNegative) {
  EXPECT("var i : [[stride(-1)]] array<i32>;",
         "test.wgsl:1:18 error: stride decoration must be greater than 0\n"
         "var i : [[stride(-1)]] array<i32>;\n"
         "                 ^^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarArrayMissingType) {
  EXPECT("var i : array<1, 3>;",
         "test.wgsl:1:15 error: invalid type for array declaration\n"
         "var i : array<1, 3>;\n"
         "              ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarArrayInvalidSize) {
  EXPECT(
      "var i : array<u32, x>;",
      "test.wgsl:1:20 error: expected signed integer literal for array size\n"
      "var i : array<u32, x>;\n"
      "                   ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarArrayNegativeSize) {
  EXPECT("var i : array<u32, -3>;",
         "test.wgsl:1:20 error: array size must be greater than 0\n"
         "var i : array<u32, -3>;\n"
         "                   ^^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarDecoListEmpty) {
  EXPECT("[[]] var i : i32;",
         "test.wgsl:1:3 error: empty decoration list\n"
         "[[]] var i : i32;\n"
         "  ^^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarDecoListInvalid) {
  EXPECT("[[location(1), meow]] var i : i32;",
         "test.wgsl:1:16 error: expected decoration\n"
         "[[location(1), meow]] var i : i32;\n"
         "               ^^^^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarDecoListMissingComma) {
  EXPECT("[[location(1) group(2)]] var i : i32;",
         "test.wgsl:1:15 error: expected ',' for decoration list\n"
         "[[location(1) group(2)]] var i : i32;\n"
         "              ^^^^^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarDecoListMissingEnd) {
  EXPECT("[[location(1) meow]] var i : i32;",
         "test.wgsl:1:15 error: expected ']]' for decoration list\n"
         "[[location(1) meow]] var i : i32;\n"
         "              ^^^^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarDecoLocationMissingLParen) {
  EXPECT("[[location 1]] var i : i32;",
         "test.wgsl:1:12 error: expected '(' for location decoration\n"
         "[[location 1]] var i : i32;\n"
         "           ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarDecoLocationMissingRParen) {
  EXPECT("[[location (1]] var i : i32;",
         "test.wgsl:1:14 error: expected ')' for location decoration\n"
         "[[location (1]] var i : i32;\n"
         "             ^^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarDecoLocationInvalidValue) {
  EXPECT("[[location(x)]] var i : i32;",
         "test.wgsl:1:12 error: expected signed integer literal for location "
         "decoration\n"
         "[[location(x)]] var i : i32;\n"
         "           ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarDecoBuiltinMissingLParen) {
  EXPECT("[[builtin position]] var i : i32;",
         "test.wgsl:1:11 error: expected '(' for builtin decoration\n"
         "[[builtin position]] var i : i32;\n"
         "          ^^^^^^^^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarDecoBuiltinMissingRParen) {
  EXPECT("[[builtin(position]] var i : i32;",
         "test.wgsl:1:19 error: expected ')' for builtin decoration\n"
         "[[builtin(position]] var i : i32;\n"
         "                  ^^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarDecoBuiltinInvalidIdentifer) {
  EXPECT("[[builtin(1)]] var i : i32;",
         "test.wgsl:1:11 error: expected identifier for builtin\n"
         "[[builtin(1)]] var i : i32;\n"
         "          ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarDecoBuiltinInvalidValue) {
  EXPECT("[[builtin(x)]] var i : i32;",
         "test.wgsl:1:11 error: invalid value for builtin decoration\n"
         "[[builtin(x)]] var i : i32;\n"
         "          ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarDecoBindingMissingLParen) {
  EXPECT("[[binding 1]] var i : i32;",
         "test.wgsl:1:11 error: expected '(' for binding decoration\n"
         "[[binding 1]] var i : i32;\n"
         "          ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarDecoBindingMissingRParen) {
  EXPECT("[[binding(1]] var i : i32;",
         "test.wgsl:1:12 error: expected ')' for binding decoration\n"
         "[[binding(1]] var i : i32;\n"
         "           ^^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarDecoBindingInvalidValue) {
  EXPECT("[[binding(x)]] var i : i32;",
         "test.wgsl:1:11 error: expected signed integer literal for binding "
         "decoration\n"
         "[[binding(x)]] var i : i32;\n"
         "          ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarDecoGroupMissingLParen) {
  EXPECT("[[group 1]] var i : i32;",
         "test.wgsl:1:9 error: expected '(' for group decoration\n"
         "[[group 1]] var i : i32;\n"
         "        ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarDecoGroupMissingRParen) {
  EXPECT("[[group(1]] var i : i32;",
         "test.wgsl:1:10 error: expected ')' for group decoration\n"
         "[[group(1]] var i : i32;\n"
         "         ^^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarDecoBindingGroupValue) {
  EXPECT("[[group(x)]] var i : i32;",
         "test.wgsl:1:9 error: expected signed integer literal for group "
         "decoration\n"
         "[[group(x)]] var i : i32;\n"
         "        ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarInvalidIdentifier) {
  EXPECT("var ^ : mat4x4;",
         "test.wgsl:1:5 error: expected identifier for variable declaration\n"
         "var ^ : mat4x4;\n"
         "    ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarMatrixMissingLessThan) {
  EXPECT("var i : mat4x4;",
         "test.wgsl:1:15 error: expected '<' for matrix\n"
         "var i : mat4x4;\n"
         "              ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarMatrixMissingGreaterThan) {
  EXPECT("var i : mat4x4<u32;",
         "test.wgsl:1:19 error: expected '>' for matrix\n"
         "var i : mat4x4<u32;\n"
         "                  ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarMatrixMissingType) {
  EXPECT("var i : mat4x4<1>;",
         "test.wgsl:1:16 error: invalid type for matrix\n"
         "var i : mat4x4<1>;\n"
         "               ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarMissingSemicolon) {
  EXPECT("var i : i32",
         "test.wgsl:1:12 error: expected ';' for variable declaration\n"
         "var i : i32\n"
         "           ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarPtrMissingLessThan) {
  EXPECT("var i : ptr;",
         "test.wgsl:1:12 error: expected '<' for ptr declaration\n"
         "var i : ptr;\n"
         "           ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarPtrMissingGreaterThan) {
  EXPECT("var i : ptr<in, u32;",
         "test.wgsl:1:20 error: expected '>' for ptr declaration\n"
         "var i : ptr<in, u32;\n"
         "                   ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarPtrMissingComma) {
  EXPECT("var i : ptr<in u32>;",
         "test.wgsl:1:16 error: expected ',' for ptr declaration\n"
         "var i : ptr<in u32>;\n"
         "               ^^^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarPtrMissingStorageClass) {
  EXPECT("var i : ptr<meow, u32>;",
         "test.wgsl:1:13 error: invalid storage class for ptr declaration\n"
         "var i : ptr<meow, u32>;\n"
         "            ^^^^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarPtrMissingType) {
  EXPECT("var i : ptr<in, 1>;",
         "test.wgsl:1:17 error: invalid type for ptr declaration\n"
         "var i : ptr<in, 1>;\n"
         "                ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarStorageDeclInvalidClass) {
  EXPECT("var<fish> i : i32",
         "test.wgsl:1:5 error: invalid storage class for variable decoration\n"
         "var<fish> i : i32\n"
         "    ^^^^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarStorageDeclMissingGThan) {
  EXPECT("var<in i : i32",
         "test.wgsl:1:8 error: expected '>' for variable decoration\n"
         "var<in i : i32\n"
         "       ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarVectorMissingLessThan) {
  EXPECT("var i : vec3;",
         "test.wgsl:1:13 error: expected '<' for vector\n"
         "var i : vec3;\n"
         "            ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarVectorMissingGreaterThan) {
  EXPECT("var i : vec3<u32;",
         "test.wgsl:1:17 error: expected '>' for vector\n"
         "var i : vec3<u32;\n"
         "                ^\n");
}

TEST_F(ParserImplErrorTest, GlobalDeclVarVectorMissingType) {
  EXPECT("var i : vec3<1>;",
         "test.wgsl:1:14 error: invalid type for vector\n"
         "var i : vec3<1>;\n"
         "             ^\n");
}

TEST_F(ParserImplErrorTest, IfStmtMissingLParen) {
  EXPECT("fn f() { if true) {} }",
         "test.wgsl:1:13 error: expected '('\n"
         "fn f() { if true) {} }\n"
         "            ^^^^\n");
}

TEST_F(ParserImplErrorTest, IfStmtMissingRParen) {
  EXPECT("fn f() { if (true {} }",
         "test.wgsl:1:19 error: expected ')'\n"
         "fn f() { if (true {} }\n"
         "                  ^\n");
}

TEST_F(ParserImplErrorTest, IfStmtInvalidCond) {
  EXPECT("fn f() { if (>) {} }",
         "test.wgsl:1:14 error: unable to parse expression\n"
         "fn f() { if (>) {} }\n"
         "             ^\n");
}

TEST_F(ParserImplErrorTest, LogicalAndInvalidExpr) {
  EXPECT("fn f() { return 1 && >; }",
         "test.wgsl:1:22 error: unable to parse right side of && expression\n"
         "fn f() { return 1 && >; }\n"
         "                     ^\n");
}

TEST_F(ParserImplErrorTest, LogicalOrInvalidExpr) {
  EXPECT("fn f() { return 1 || >; }",
         "test.wgsl:1:22 error: unable to parse right side of || expression\n"
         "fn f() { return 1 || >; }\n"
         "                     ^\n");
}

TEST_F(ParserImplErrorTest, LoopMissingLBrace) {
  EXPECT("fn f() { loop }",
         "test.wgsl:1:15 error: expected '{' for loop\n"
         "fn f() { loop }\n"
         "              ^\n");
}

TEST_F(ParserImplErrorTest, LoopMissingRBrace) {
  EXPECT("fn f() { loop {",
         "test.wgsl:1:16 error: expected '}' for loop\n"
         "fn f() { loop {\n"
         "               ^\n");
}

TEST_F(ParserImplErrorTest, MaxErrorsReached) {
  EXPECT("x; x; x; x; x; x; x; x;",
         "test.wgsl:1:1 error: unexpected token\n"
         "x; x; x; x; x; x; x; x;\n"
         "^\n\n"
         "test.wgsl:1:4 error: unexpected token\n"
         "x; x; x; x; x; x; x; x;\n"
         "   ^\n\n"
         "test.wgsl:1:7 error: unexpected token\n"
         "x; x; x; x; x; x; x; x;\n"
         "      ^\n\n"
         "test.wgsl:1:10 error: unexpected token\n"
         "x; x; x; x; x; x; x; x;\n"
         "         ^\n\n"
         "test.wgsl:1:13 error: unexpected token\n"
         "x; x; x; x; x; x; x; x;\n"
         "            ^\n\n"
         "test.wgsl error: stopping after 5 errors");
}

TEST_F(ParserImplErrorTest, MemberExprMissingIdentifier) {
  EXPECT("fn f() { x = a.; }",
         "test.wgsl:1:16 error: expected identifier for member accessor\n"
         "fn f() { x = a.; }\n"
         "               ^\n");
}

TEST_F(ParserImplErrorTest, MultiplicativeInvalidExpr) {
  EXPECT("fn f() { return 1.0 * <; }",
         "test.wgsl:1:23 error: unable to parse right side of * expression\n"
         "fn f() { return 1.0 * <; }\n"
         "                      ^\n");
}

TEST_F(ParserImplErrorTest, OrInvalidExpr) {
  EXPECT("fn f() { return 1 | >; }",
         "test.wgsl:1:21 error: unable to parse right side of | expression\n"
         "fn f() { return 1 | >; }\n"
         "                    ^\n");
}

TEST_F(ParserImplErrorTest, RelationalInvalidExpr) {
  EXPECT("fn f() { return 1 < >; }",
         "test.wgsl:1:21 error: unable to parse right side of < expression\n"
         "fn f() { return 1 < >; }\n"
         "                    ^\n");
}

TEST_F(ParserImplErrorTest, ReturnStmtMissingSemicolon) {
  EXPECT("fn f() { return }",
         "test.wgsl:1:17 error: expected ';' for return statement\n"
         "fn f() { return }\n"
         "                ^\n");
}

TEST_F(ParserImplErrorTest, ShiftInvalidExpr) {
  EXPECT("fn f() { return 1 << >; }",
         "test.wgsl:1:22 error: unable to parse right side of << expression\n"
         "fn f() { return 1 << >; }\n"
         "                     ^\n");
}

TEST_F(ParserImplErrorTest, SwitchStmtMissingLBrace) {
  EXPECT("fn f() { switch(1) }",
         "test.wgsl:1:20 error: expected '{' for switch statement\n"
         "fn f() { switch(1) }\n"
         "                   ^\n");
}

TEST_F(ParserImplErrorTest, SwitchStmtMissingRBrace) {
  EXPECT("fn f() { switch(1) {",
         "test.wgsl:1:21 error: expected '}' for switch statement\n"
         "fn f() { switch(1) {\n"
         "                    ^\n");
}

TEST_F(ParserImplErrorTest, SwitchStmtInvalidCase) {
  EXPECT("fn f() { switch(1) { case ^: } }",
         "test.wgsl:1:27 error: unable to parse case selectors\n"
         "fn f() { switch(1) { case ^: } }\n"
         "                          ^\n");
}

TEST_F(ParserImplErrorTest, SwitchStmtInvalidCase2) {
  EXPECT(
      "fn f() { switch(1) { case false: } }",
      "test.wgsl:1:27 error: invalid case selector must be an integer value\n"
      "fn f() { switch(1) { case false: } }\n"
      "                          ^^^^^\n");
}

TEST_F(ParserImplErrorTest, SwitchStmtCaseMissingColon) {
  EXPECT("fn f() { switch(1) { case 1 {} } }",
         "test.wgsl:1:29 error: expected ':' for case statement\n"
         "fn f() { switch(1) { case 1 {} } }\n"
         "                            ^\n");
}

TEST_F(ParserImplErrorTest, SwitchStmtCaseMissingLBrace) {
  EXPECT("fn f() { switch(1) { case 1: } }",
         "test.wgsl:1:30 error: expected '{' for case statement\n"
         "fn f() { switch(1) { case 1: } }\n"
         "                             ^\n");
}

TEST_F(ParserImplErrorTest, SwitchStmtCaseMissingRBrace) {
  EXPECT("fn f() { switch(1) { case 1: {",
         "test.wgsl:1:31 error: expected '}' for case statement\n"
         "fn f() { switch(1) { case 1: {\n"
         "                              ^\n");
}

TEST_F(ParserImplErrorTest, SwitchStmtCaseFallthroughMissingSemicolon) {
  EXPECT("fn f() { switch(1) { case 1: { fallthrough } case 2: {} } }",
         "test.wgsl:1:44 error: expected ';' for fallthrough statement\n"
         "fn f() { switch(1) { case 1: { fallthrough } case 2: {} } }\n"
         "                                           ^\n");
}

TEST_F(ParserImplErrorTest, VarStmtMissingSemicolon) {
  EXPECT("fn f() { var a : u32 }",
         "test.wgsl:1:22 error: expected ';' for variable declaration\n"
         "fn f() { var a : u32 }\n"
         "                     ^\n");
}

TEST_F(ParserImplErrorTest, VarStmtInvalidAssignment) {
  EXPECT("fn f() { var a : u32 = >; }",
         "test.wgsl:1:24 error: missing constructor for variable declaration\n"
         "fn f() { var a : u32 = >; }\n"
         "                       ^\n");
}

TEST_F(ParserImplErrorTest, UnaryInvalidExpr) {
  EXPECT("fn f() { return !<; }",
         "test.wgsl:1:18 error: unable to parse right side of ! expression\n"
         "fn f() { return !<; }\n"
         "                 ^\n");
}

TEST_F(ParserImplErrorTest, UnexpectedToken) {
  EXPECT("unexpected",
         "test.wgsl:1:1 error: unexpected token\n"
         "unexpected\n"
         "^^^^^^^^^^\n");
}

TEST_F(ParserImplErrorTest, XorInvalidExpr) {
  EXPECT("fn f() { return 1 ^ >; }",
         "test.wgsl:1:21 error: unable to parse right side of ^ expression\n"
         "fn f() { return 1 ^ >; }\n"
         "                    ^\n");
}

}  // namespace
}  // namespace wgsl
}  // namespace reader
}  // namespace tint
