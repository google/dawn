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

#include "src/tint/reader/wgsl/parser_impl_test_helper.h"

namespace tint::reader::wgsl {
namespace {

using ParserImplReservedKeywordTest = ParserImplTestWithParam<std::string>;
TEST_P(ParserImplReservedKeywordTest, Function) {
    auto name = GetParam();
    auto p = parser("fn " + name + "() {}");
    EXPECT_FALSE(p->Parse());
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:4: '" + name + "' is a reserved keyword");
}
TEST_P(ParserImplReservedKeywordTest, ModuleLet) {
    auto name = GetParam();
    auto p = parser("let " + name + " : i32 = 1;");
    EXPECT_FALSE(p->Parse());
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:5: '" + name + "' is a reserved keyword");
}
TEST_P(ParserImplReservedKeywordTest, ModuleVar) {
    auto name = GetParam();
    auto p = parser("var " + name + " : i32 = 1;");
    EXPECT_FALSE(p->Parse());
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:5: '" + name + "' is a reserved keyword");
}
TEST_P(ParserImplReservedKeywordTest, FunctionLet) {
    auto name = GetParam();
    auto p = parser("fn f() { let " + name + " : i32 = 1; }");
    EXPECT_FALSE(p->Parse());
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:14: '" + name + "' is a reserved keyword");
}
TEST_P(ParserImplReservedKeywordTest, FunctionVar) {
    auto name = GetParam();
    auto p = parser("fn f() { var " + name + " : i32 = 1; }");
    EXPECT_FALSE(p->Parse());
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:14: '" + name + "' is a reserved keyword");
}
TEST_P(ParserImplReservedKeywordTest, FunctionParam) {
    auto name = GetParam();
    auto p = parser("fn f(" + name + " : i32) {}");
    EXPECT_FALSE(p->Parse());
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:6: '" + name + "' is a reserved keyword");
}
TEST_P(ParserImplReservedKeywordTest, Struct) {
    auto name = GetParam();
    auto p = parser("struct " + name + " {};");
    EXPECT_FALSE(p->Parse());
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:8: '" + name + "' is a reserved keyword");
}
TEST_P(ParserImplReservedKeywordTest, StructMember) {
    auto name = GetParam();
    auto p = parser("struct S { " + name + " : i32, };");
    EXPECT_FALSE(p->Parse());
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:12: '" + name + "' is a reserved keyword");
}
TEST_P(ParserImplReservedKeywordTest, Alias) {
    auto name = GetParam();
    auto p = parser("type " + name + " = i32;");
    EXPECT_FALSE(p->Parse());
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:6: '" + name + "' is a reserved keyword");
}
INSTANTIATE_TEST_SUITE_P(ParserImplReservedKeywordTest,
                         ParserImplReservedKeywordTest,
                         testing::Values("asm",
                                         "bf16",
                                         "const",
                                         "do",
                                         "enum",
                                         "f16",
                                         "f64",
                                         "handle",
                                         "i8",
                                         "i16",
                                         "i64",
                                         "mat",
                                         "premerge",
                                         "regardless",
                                         "typedef",
                                         "u8",
                                         "u16",
                                         "u64",
                                         "unless",
                                         "using",
                                         "vec",
                                         "void",
                                         "while"));

}  // namespace
}  // namespace tint::reader::wgsl
