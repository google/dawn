// Copyright 2022 The Tint Authors.
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

#include "src/tint/utils/unicode.h"

#include <string>
#include <vector>

#include "gmock/gmock.h"

/// Helper for constructing a CodePoint
#define C(x) CodePoint(x)

namespace tint::utils {

////////////////////////////////////////////////////////////////////////////////
// CodePoint character set tests
////////////////////////////////////////////////////////////////////////////////
namespace {

struct CodePointCase {
    CodePoint code_point;
    bool is_xid_start;
    bool is_xid_continue;
};

std::ostream& operator<<(std::ostream& out, CodePointCase c) {
    return out << c.code_point;
}

class CodePointTest : public testing::TestWithParam<CodePointCase> {};

TEST_P(CodePointTest, CharacterSets) {
    auto param = GetParam();
    EXPECT_EQ(param.code_point.IsXIDStart(), param.is_xid_start);
    EXPECT_EQ(param.code_point.IsXIDContinue(), param.is_xid_continue);
}

INSTANTIATE_TEST_SUITE_P(
    CodePointTest,
    CodePointTest,
    ::testing::ValuesIn({
        CodePointCase{C(' '), /* start */ false, /* continue */ false},
        CodePointCase{C('\t'), /* start */ false, /* continue */ false},
        CodePointCase{C('\n'), /* start */ false, /* continue */ false},
        CodePointCase{C('\r'), /* start */ false, /* continue */ false},
        CodePointCase{C('!'), /* start */ false, /* continue */ false},
        CodePointCase{C('"'), /* start */ false, /* continue */ false},
        CodePointCase{C('#'), /* start */ false, /* continue */ false},
        CodePointCase{C('$'), /* start */ false, /* continue */ false},
        CodePointCase{C('%'), /* start */ false, /* continue */ false},
        CodePointCase{C('&'), /* start */ false, /* continue */ false},
        CodePointCase{C('\\'), /* start */ false, /* continue */ false},
        CodePointCase{C('/'), /* start */ false, /* continue */ false},
        CodePointCase{C('('), /* start */ false, /* continue */ false},
        CodePointCase{C(')'), /* start */ false, /* continue */ false},
        CodePointCase{C('*'), /* start */ false, /* continue */ false},
        CodePointCase{C(','), /* start */ false, /* continue */ false},
        CodePointCase{C('-'), /* start */ false, /* continue */ false},
        CodePointCase{C('/'), /* start */ false, /* continue */ false},
        CodePointCase{C('`'), /* start */ false, /* continue */ false},
        CodePointCase{C('@'), /* start */ false, /* continue */ false},
        CodePointCase{C('^'), /* start */ false, /* continue */ false},
        CodePointCase{C('['), /* start */ false, /* continue */ false},
        CodePointCase{C(']'), /* start */ false, /* continue */ false},
        CodePointCase{C('|'), /* start */ false, /* continue */ false},
        CodePointCase{C('('), /* start */ false, /* continue */ false},
        CodePointCase{C(','), /* start */ false, /* continue */ false},
        CodePointCase{C('}'), /* start */ false, /* continue */ false},
        CodePointCase{C('a'), /* start */ true, /* continue */ true},
        CodePointCase{C('b'), /* start */ true, /* continue */ true},
        CodePointCase{C('c'), /* start */ true, /* continue */ true},
        CodePointCase{C('x'), /* start */ true, /* continue */ true},
        CodePointCase{C('y'), /* start */ true, /* continue */ true},
        CodePointCase{C('z'), /* start */ true, /* continue */ true},
        CodePointCase{C('A'), /* start */ true, /* continue */ true},
        CodePointCase{C('B'), /* start */ true, /* continue */ true},
        CodePointCase{C('C'), /* start */ true, /* continue */ true},
        CodePointCase{C('X'), /* start */ true, /* continue */ true},
        CodePointCase{C('Y'), /* start */ true, /* continue */ true},
        CodePointCase{C('Z'), /* start */ true, /* continue */ true},
        CodePointCase{C('_'), /* start */ false, /* continue */ true},
        CodePointCase{C('0'), /* start */ false, /* continue */ true},
        CodePointCase{C('1'), /* start */ false, /* continue */ true},
        CodePointCase{C('2'), /* start */ false, /* continue */ true},
        CodePointCase{C('8'), /* start */ false, /* continue */ true},
        CodePointCase{C('9'), /* start */ false, /* continue */ true},
        CodePointCase{C('0'), /* start */ false, /* continue */ true},

        // First in XID_Start
        CodePointCase{C(0x00041), /* start */ true, /* continue */ true},
        // Last in XID_Start
        CodePointCase{C(0x3134a), /* start */ true, /* continue */ true},

        // Random selection from XID_Start, using the interval's first
        CodePointCase{C(0x002ee), /* start */ true, /* continue */ true},
        CodePointCase{C(0x005ef), /* start */ true, /* continue */ true},
        CodePointCase{C(0x009f0), /* start */ true, /* continue */ true},
        CodePointCase{C(0x00d3d), /* start */ true, /* continue */ true},
        CodePointCase{C(0x00d54), /* start */ true, /* continue */ true},
        CodePointCase{C(0x00e86), /* start */ true, /* continue */ true},
        CodePointCase{C(0x00edc), /* start */ true, /* continue */ true},
        CodePointCase{C(0x01c00), /* start */ true, /* continue */ true},
        CodePointCase{C(0x01c80), /* start */ true, /* continue */ true},
        CodePointCase{C(0x02071), /* start */ true, /* continue */ true},
        CodePointCase{C(0x02dd0), /* start */ true, /* continue */ true},
        CodePointCase{C(0x0a4d0), /* start */ true, /* continue */ true},
        CodePointCase{C(0x0aac0), /* start */ true, /* continue */ true},
        CodePointCase{C(0x0ab5c), /* start */ true, /* continue */ true},
        CodePointCase{C(0x0ffda), /* start */ true, /* continue */ true},
        CodePointCase{C(0x11313), /* start */ true, /* continue */ true},
        CodePointCase{C(0x1ee49), /* start */ true, /* continue */ true},

        // Random selection from XID_Start, using the interval's last
        CodePointCase{C(0x00710), /* start */ true, /* continue */ true},
        CodePointCase{C(0x00b83), /* start */ true, /* continue */ true},
        CodePointCase{C(0x00b9a), /* start */ true, /* continue */ true},
        CodePointCase{C(0x00ec4), /* start */ true, /* continue */ true},
        CodePointCase{C(0x01081), /* start */ true, /* continue */ true},
        CodePointCase{C(0x012be), /* start */ true, /* continue */ true},
        CodePointCase{C(0x02107), /* start */ true, /* continue */ true},
        CodePointCase{C(0x03029), /* start */ true, /* continue */ true},
        CodePointCase{C(0x03035), /* start */ true, /* continue */ true},
        CodePointCase{C(0x0aadd), /* start */ true, /* continue */ true},
        CodePointCase{C(0x10805), /* start */ true, /* continue */ true},
        CodePointCase{C(0x11075), /* start */ true, /* continue */ true},
        CodePointCase{C(0x1d4a2), /* start */ true, /* continue */ true},
        CodePointCase{C(0x1e7fe), /* start */ true, /* continue */ true},
        CodePointCase{C(0x1ee27), /* start */ true, /* continue */ true},
        CodePointCase{C(0x2b738), /* start */ true, /* continue */ true},

        // Random selection from XID_Continue, using the interval's first
        CodePointCase{C(0x16ac0), /* start */ false, /* continue */ true},
        CodePointCase{C(0x00dca), /* start */ false, /* continue */ true},
        CodePointCase{C(0x16f4f), /* start */ false, /* continue */ true},
        CodePointCase{C(0x0fe00), /* start */ false, /* continue */ true},
        CodePointCase{C(0x00ec8), /* start */ false, /* continue */ true},
        CodePointCase{C(0x009be), /* start */ false, /* continue */ true},
        CodePointCase{C(0x11d47), /* start */ false, /* continue */ true},
        CodePointCase{C(0x11d50), /* start */ false, /* continue */ true},
        CodePointCase{C(0x0a926), /* start */ false, /* continue */ true},
        CodePointCase{C(0x0aac1), /* start */ false, /* continue */ true},
        CodePointCase{C(0x00f18), /* start */ false, /* continue */ true},
        CodePointCase{C(0x11145), /* start */ false, /* continue */ true},
        CodePointCase{C(0x017dd), /* start */ false, /* continue */ true},
        CodePointCase{C(0x0aaeb), /* start */ false, /* continue */ true},
        CodePointCase{C(0x11173), /* start */ false, /* continue */ true},
        CodePointCase{C(0x00a51), /* start */ false, /* continue */ true},

        // Random selection from XID_Continue, using the interval's last
        CodePointCase{C(0x00f84), /* start */ false, /* continue */ true},
        CodePointCase{C(0x10a3a), /* start */ false, /* continue */ true},
        CodePointCase{C(0x1e018), /* start */ false, /* continue */ true},
        CodePointCase{C(0x0a827), /* start */ false, /* continue */ true},
        CodePointCase{C(0x01abd), /* start */ false, /* continue */ true},
        CodePointCase{C(0x009d7), /* start */ false, /* continue */ true},
        CodePointCase{C(0x00b6f), /* start */ false, /* continue */ true},
        CodePointCase{C(0x0096f), /* start */ false, /* continue */ true},
        CodePointCase{C(0x11146), /* start */ false, /* continue */ true},
        CodePointCase{C(0x10eac), /* start */ false, /* continue */ true},
        CodePointCase{C(0x00f39), /* start */ false, /* continue */ true},
        CodePointCase{C(0x1e136), /* start */ false, /* continue */ true},
        CodePointCase{C(0x00def), /* start */ false, /* continue */ true},
        CodePointCase{C(0x0fe34), /* start */ false, /* continue */ true},
        CodePointCase{C(0x009c8), /* start */ false, /* continue */ true},
        CodePointCase{C(0x00fbc), /* start */ false, /* continue */ true},

        // Random code points that are one less than an interval of XID_Start
        CodePointCase{C(0x003f6), /* start */ false, /* continue */ false},
        CodePointCase{C(0x005ee), /* start */ false, /* continue */ false},
        CodePointCase{C(0x009ef), /* start */ false, /* continue */ true},
        CodePointCase{C(0x00d3c), /* start */ false, /* continue */ true},
        CodePointCase{C(0x00d53), /* start */ false, /* continue */ false},
        CodePointCase{C(0x00e85), /* start */ false, /* continue */ false},
        CodePointCase{C(0x00edb), /* start */ false, /* continue */ false},
        CodePointCase{C(0x01bff), /* start */ false, /* continue */ false},
        CodePointCase{C(0x02070), /* start */ false, /* continue */ false},
        CodePointCase{C(0x02dcf), /* start */ false, /* continue */ false},
        CodePointCase{C(0x0a4cf), /* start */ false, /* continue */ false},
        CodePointCase{C(0x0aabf), /* start */ false, /* continue */ true},
        CodePointCase{C(0x0ab5b), /* start */ false, /* continue */ false},
        CodePointCase{C(0x0ffd9), /* start */ false, /* continue */ false},
        CodePointCase{C(0x11312), /* start */ false, /* continue */ false},
        CodePointCase{C(0x1ee48), /* start */ false, /* continue */ false},

        // Random code points that are one more than an interval of XID_Continue
        CodePointCase{C(0x00060), /* start */ false, /* continue */ false},
        CodePointCase{C(0x00a4e), /* start */ false, /* continue */ false},
        CodePointCase{C(0x00a84), /* start */ false, /* continue */ false},
        CodePointCase{C(0x00cce), /* start */ false, /* continue */ false},
        CodePointCase{C(0x00eda), /* start */ false, /* continue */ false},
        CodePointCase{C(0x00f85), /* start */ false, /* continue */ false},
        CodePointCase{C(0x01b74), /* start */ false, /* continue */ false},
        CodePointCase{C(0x01c38), /* start */ false, /* continue */ false},
        CodePointCase{C(0x0fe30), /* start */ false, /* continue */ false},
        CodePointCase{C(0x11174), /* start */ false, /* continue */ false},
        CodePointCase{C(0x112eb), /* start */ false, /* continue */ false},
        CodePointCase{C(0x115de), /* start */ false, /* continue */ false},
        CodePointCase{C(0x1172c), /* start */ false, /* continue */ false},
        CodePointCase{C(0x11a3f), /* start */ false, /* continue */ false},
        CodePointCase{C(0x11c37), /* start */ false, /* continue */ false},
        CodePointCase{C(0x11d92), /* start */ false, /* continue */ false},
        CodePointCase{C(0x1e2af), /* start */ false, /* continue */ false},
    }));

}  // namespace

////////////////////////////////////////////////////////////////////////////////
// DecodeUTF8 valid tests
////////////////////////////////////////////////////////////////////////////////
namespace {

struct CodePointAndWidth {
    CodePoint code_point;
    size_t width;
};

bool operator==(const CodePointAndWidth& a, const CodePointAndWidth& b) {
    return a.code_point == b.code_point && a.width == b.width;
}

std::ostream& operator<<(std::ostream& out, CodePointAndWidth cpw) {
    return out << "code_point: " << cpw.code_point << ", width: " << cpw.width;
}

struct DecodeUTF8Case {
    std::string string;
    std::vector<CodePointAndWidth> expected;
};

std::ostream& operator<<(std::ostream& out, DecodeUTF8Case c) {
    return out << "'" << c.string << "'";
}

class DecodeUTF8Test : public testing::TestWithParam<DecodeUTF8Case> {};

TEST_P(DecodeUTF8Test, Valid) {
    auto param = GetParam();

    const uint8_t* data = reinterpret_cast<const uint8_t*>(param.string.data());
    const size_t len = param.string.size();

    std::vector<CodePointAndWidth> got;
    size_t offset = 0;
    while (offset < len) {
        auto [code_point, width] = utf8::Decode(data + offset, len - offset);
        if (width == 0) {
            FAIL() << "Decode() failed at byte offset " << offset;
        }
        offset += width;
        got.emplace_back(CodePointAndWidth{code_point, width});
    }

    EXPECT_THAT(got, ::testing::ElementsAreArray(param.expected));
}

INSTANTIATE_TEST_SUITE_P(AsciiLetters,
                         DecodeUTF8Test,
                         ::testing::ValuesIn({
                             DecodeUTF8Case{"a", {{C('a'), 1}}},
                             DecodeUTF8Case{"abc", {{C('a'), 1}, {C('b'), 1}, {C('c'), 1}}},
                             DecodeUTF8Case{"def", {{C('d'), 1}, {C('e'), 1}, {C('f'), 1}}},
                             DecodeUTF8Case{"gh", {{C('g'), 1}, {C('h'), 1}}},
                             DecodeUTF8Case{"ij", {{C('i'), 1}, {C('j'), 1}}},
                             DecodeUTF8Case{"klm", {{C('k'), 1}, {C('l'), 1}, {C('m'), 1}}},
                             DecodeUTF8Case{"nop", {{C('n'), 1}, {C('o'), 1}, {C('p'), 1}}},
                             DecodeUTF8Case{"qr", {{C('q'), 1}, {C('r'), 1}}},
                             DecodeUTF8Case{"stu", {{C('s'), 1}, {C('t'), 1}, {C('u'), 1}}},
                             DecodeUTF8Case{"vw", {{C('v'), 1}, {C('w'), 1}}},
                             DecodeUTF8Case{"xyz", {{C('x'), 1}, {C('y'), 1}, {C('z'), 1}}},
                             DecodeUTF8Case{"A", {{C('A'), 1}}},
                             DecodeUTF8Case{"ABC", {{C('A'), 1}, {C('B'), 1}, {C('C'), 1}}},
                             DecodeUTF8Case{"DEF", {{C('D'), 1}, {C('E'), 1}, {C('F'), 1}}},
                             DecodeUTF8Case{"GH", {{C('G'), 1}, {C('H'), 1}}},
                             DecodeUTF8Case{"IJ", {{C('I'), 1}, {C('J'), 1}}},
                             DecodeUTF8Case{"KLM", {{C('K'), 1}, {C('L'), 1}, {C('M'), 1}}},
                             DecodeUTF8Case{"NOP", {{C('N'), 1}, {C('O'), 1}, {C('P'), 1}}},
                             DecodeUTF8Case{"QR", {{C('Q'), 1}, {C('R'), 1}}},
                             DecodeUTF8Case{"STU", {{C('S'), 1}, {C('T'), 1}, {C('U'), 1}}},
                             DecodeUTF8Case{"VW", {{C('V'), 1}, {C('W'), 1}}},
                             DecodeUTF8Case{"XYZ", {{C('X'), 1}, {C('Y'), 1}, {C('Z'), 1}}},
                         }));

INSTANTIATE_TEST_SUITE_P(AsciiNumbers,
                         DecodeUTF8Test,
                         ::testing::ValuesIn({
                             DecodeUTF8Case{"012", {{C('0'), 1}, {C('1'), 1}, {C('2'), 1}}},
                             DecodeUTF8Case{"345", {{C('3'), 1}, {C('4'), 1}, {C('5'), 1}}},
                             DecodeUTF8Case{"678", {{C('6'), 1}, {C('7'), 1}, {C('8'), 1}}},
                             DecodeUTF8Case{"9", {{C('9'), 1}}},
                         }));

INSTANTIATE_TEST_SUITE_P(AsciiSymbols,
                         DecodeUTF8Test,
                         ::testing::ValuesIn({
                             DecodeUTF8Case{"!\"#", {{C('!'), 1}, {C('"'), 1}, {C('#'), 1}}},
                             DecodeUTF8Case{"$%&", {{C('$'), 1}, {C('%'), 1}, {C('&'), 1}}},
                             DecodeUTF8Case{"'()", {{C('\''), 1}, {C('('), 1}, {C(')'), 1}}},
                             DecodeUTF8Case{"*,-", {{C('*'), 1}, {C(','), 1}, {C('-'), 1}}},
                             DecodeUTF8Case{"/`@", {{C('/'), 1}, {C('`'), 1}, {C('@'), 1}}},
                             DecodeUTF8Case{"^\\[", {{C('^'), 1}, {C('\\'), 1}, {C('['), 1}}},
                             DecodeUTF8Case{"]_|", {{C(']'), 1}, {C('_'), 1}, {C('|'), 1}}},
                             DecodeUTF8Case{"{}", {{C('{'), 1}, {C('}'), 1}}},
                         }));

INSTANTIATE_TEST_SUITE_P(AsciiSpecial,
                         DecodeUTF8Test,
                         ::testing::ValuesIn({
                             DecodeUTF8Case{"", {}},
                             DecodeUTF8Case{" \t\n", {{C(' '), 1}, {C('\t'), 1}, {C('\n'), 1}}},
                             DecodeUTF8Case{"\a\b\f", {{C('\a'), 1}, {C('\b'), 1}, {C('\f'), 1}}},
                             DecodeUTF8Case{"\n\r\t", {{C('\n'), 1}, {C('\r'), 1}, {C('\t'), 1}}},
                             DecodeUTF8Case{"\v", {{C('\v'), 1}}},
                         }));

INSTANTIATE_TEST_SUITE_P(Hindi,
                         DecodeUTF8Test,
                         ::testing::ValuesIn({DecodeUTF8Case{
                             // नमस्ते दुनिया
                             "\xe0\xa4\xa8\xe0\xa4\xae\xe0\xa4\xb8\xe0\xa5\x8d\xe0\xa4\xa4\xe0\xa5"
                             "\x87\x20\xe0\xa4\xa6\xe0\xa5\x81\xe0\xa4\xa8\xe0\xa4\xbf\xe0\xa4\xaf"
                             "\xe0\xa4\xbe",
                             {
                                 {C(0x0928), 3},  // न
                                 {C(0x092e), 3},  // म
                                 {C(0x0938), 3},  // स
                                 {C(0x094d), 3},  // ् //
                                 {C(0x0924), 3},  // त
                                 {C(0x0947), 3},  // े //
                                 {C(' '), 1},
                                 {C(0x0926), 3},  // द
                                 {C(0x0941), 3},  // ु //
                                 {C(0x0928), 3},  // न
                                 {C(0x093f), 3},  // ि //
                                 {C(0x092f), 3},  // य
                                 {C(0x093e), 3},  // ा //
                             },
                         }}));

INSTANTIATE_TEST_SUITE_P(Mandarin,
                         DecodeUTF8Test,
                         ::testing::ValuesIn({DecodeUTF8Case{
                             // 你好世界
                             "\xe4\xbd\xa0\xe5\xa5\xbd\xe4\xb8\x96\xe7\x95\x8c",
                             {
                                 {C(0x4f60), 3},  // 你
                                 {C(0x597d), 3},  // 好
                                 {C(0x4e16), 3},  // 世
                                 {C(0x754c), 3},  // 界
                             },
                         }}));

INSTANTIATE_TEST_SUITE_P(Japanese,
                         DecodeUTF8Test,
                         ::testing::ValuesIn({DecodeUTF8Case{
                             // こんにちは世界
                             "\xe3\x81\x93\xe3\x82\x93\xe3\x81\xab\xe3\x81\xa1"
                             "\xe3\x81\xaf\xe4\xb8\x96\xe7\x95\x8c",
                             {
                                 {C(0x3053), 3},  // こ
                                 {C(0x3093), 3},  // ん
                                 {C(0x306B), 3},  // に
                                 {C(0x3061), 3},  // ち
                                 {C(0x306F), 3},  // は
                                 {C(0x4E16), 3},  // 世
                                 {C(0x754C), 3},  // 界
                             },
                         }}));

INSTANTIATE_TEST_SUITE_P(Korean,
                         DecodeUTF8Test,
                         ::testing::ValuesIn({DecodeUTF8Case{
                             // 안녕하세요 세계
                             "\xec\x95\x88\xeb\x85\x95\xed\x95\x98\xec\x84\xb8"
                             "\xec\x9a\x94\x20\xec\x84\xb8\xea\xb3\x84",
                             {
                                 {C(0xc548), 3},  // 안
                                 {C(0xb155), 3},  // 녕
                                 {C(0xd558), 3},  // 하
                                 {C(0xc138), 3},  // 세
                                 {C(0xc694), 3},  // 요
                                 {C(' '), 1},     //
                                 {C(0xc138), 3},  // 세
                                 {C(0xacc4), 3},  // 계
                             },
                         }}));

INSTANTIATE_TEST_SUITE_P(Emoji,
                         DecodeUTF8Test,
                         ::testing::ValuesIn({DecodeUTF8Case{
                             // 👋🌎
                             "\xf0\x9f\x91\x8b\xf0\x9f\x8c\x8e",
                             {
                                 {C(0x1f44b), 4},  // 👋
                                 {C(0x1f30e), 4},  // 🌎
                             },
                         }}));

INSTANTIATE_TEST_SUITE_P(Random,
                         DecodeUTF8Test,
                         ::testing::ValuesIn({DecodeUTF8Case{
                             // Øⓑꚫ쁹Ǵ𐌒岾🥍ⴵ㍨又ᮗ
                             "\xc3\x98\xe2\x93\x91\xea\x9a\xab\xec\x81\xb9\xc7\xb4\xf0\x90\x8c\x92"
                             "\xe5\xb2\xbe\xf0\x9f\xa5\x8d\xe2\xb4\xb5\xe3\x8d\xa8\xe5\x8f\x88\xe1"
                             "\xae\x97",
                             {
                                 {C(0x000d8), 2},  // Ø
                                 {C(0x024d1), 3},  // ⓑ
                                 {C(0x0a6ab), 3},  // ꚫ
                                 {C(0x0c079), 3},  // 쁹
                                 {C(0x001f4), 2},  // Ǵ
                                 {C(0x10312), 4},  // 𐌒
                                 {C(0x05cbe), 3},  // 岾
                                 {C(0x1f94d), 4},  // 🥍
                                 {C(0x02d35), 3},  // ⴵ
                                 {C(0x03368), 3},  // ㍨
                                 {C(0x053c8), 3},  // 又
                                 {C(0x01b97), 3},  // ᮗ
                             },
                         }}));

}  // namespace

////////////////////////////////////////////////////////////////////////////////
// DecodeUTF8 invalid tests
////////////////////////////////////////////////////////////////////////////////
namespace {
class DecodeUTF8InvalidTest : public testing::TestWithParam<const char*> {};

TEST_P(DecodeUTF8InvalidTest, Invalid) {
    auto* param = GetParam();

    const uint8_t* data = reinterpret_cast<const uint8_t*>(param);
    const size_t len = std::string(param).size();

    auto [code_point, width] = utf8::Decode(data, len);
    EXPECT_EQ(code_point, CodePoint(0));
    EXPECT_EQ(width, 0u);
}

INSTANTIATE_TEST_SUITE_P(Invalid,
                         DecodeUTF8InvalidTest,
                         ::testing::ValuesIn({
                             "\x80\x80\x80\x80",  // 10000000
                             "\x81\x80\x80\x80",  // 10000001
                             "\x8f\x80\x80\x80",  // 10001111
                             "\x90\x80\x80\x80",  // 10010000
                             "\x91\x80\x80\x80",  // 10010001
                             "\x9f\x80\x80\x80",  // 10011111
                             "\xa0\x80\x80\x80",  // 10100000
                             "\xa1\x80\x80\x80",  // 10100001
                             "\xaf\x80\x80\x80",  // 10101111
                             "\xb0\x80\x80\x80",  // 10110000
                             "\xb1\x80\x80\x80",  // 10110001
                             "\xbf\x80\x80\x80",  // 10111111
                             "\xc0\x80\x80\x80",  // 11000000
                             "\xc1\x80\x80\x80",  // 11000001
                             "\xf5\x80\x80\x80",  // 11110101
                             "\xf6\x80\x80\x80",  // 11110110
                             "\xf7\x80\x80\x80",  // 11110111
                             "\xf8\x80\x80\x80",  // 11111000
                             "\xfe\x80\x80\x80",  // 11111110
                             "\xff\x80\x80\x80",  // 11111111

                             "\xd0",          // 2-bytes, missing second byte
                             "\xe8\x8f",      // 3-bytes, missing third byte
                             "\xf4\x8f\x8f",  // 4-bytes, missing fourth byte

                             "\xd0\x7f",          // 2-bytes, second byte MSB unset
                             "\xe8\x7f\x8f",      // 3-bytes, second byte MSB unset
                             "\xe8\x8f\x7f",      // 3-bytes, third byte MSB unset
                             "\xf4\x7f\x8f\x8f",  // 4-bytes, second byte MSB unset
                             "\xf4\x8f\x7f\x8f",  // 4-bytes, third byte MSB unset
                             "\xf4\x8f\x8f\x7f",  // 4-bytes, fourth byte MSB unset
                         }));

}  // namespace

}  // namespace tint::utils
