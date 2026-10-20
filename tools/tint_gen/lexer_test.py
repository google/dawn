#!/usr/bin/env python3
# Copyright 2026 The Dawn & Tint Authors
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this
#    list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its
#    contributors may be used to endorse or promote products derived from
#    this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
"""Unit tests for lexer.py."""

import unittest

import lexer
import tok


class LexerTest(unittest.TestCase):

    def test_lex_tokens(self):
        filepath = "test.txt"

        def loc(l: int, c: int) -> tok.Location:
            return tok.Location(line=l, column=c, filepath=filepath)

        cases: list[tuple[str, list[tok.Token]]] = [
            (
                "ident",
                [
                    tok.Token(
                        kind=tok.Kind.Identifier,
                        str="ident",
                        source=tok.Source(s=loc(1, 1), e=loc(1, 6)),
                    )
                ],
            ),
            (
                "ident_123",
                [
                    tok.Token(
                        kind=tok.Kind.Identifier,
                        str="ident_123",
                        source=tok.Source(s=loc(1, 1), e=loc(1, 10)),
                    )
                ],
            ),
            (
                "_ident_",
                [
                    tok.Token(
                        kind=tok.Kind.Identifier,
                        str="_ident_",
                        source=tok.Source(s=loc(1, 1), e=loc(1, 8)),
                    )
                ],
            ),
            (
                "123456789",
                [
                    tok.Token(
                        kind=tok.Kind.Integer,
                        str="123456789",
                        source=tok.Source(s=loc(1, 1), e=loc(1, 10)),
                    )
                ],
            ),
            (
                "-123456789",
                [
                    tok.Token(
                        kind=tok.Kind.Integer,
                        str="-123456789",
                        source=tok.Source(s=loc(1, 1), e=loc(1, 11)),
                    )
                ],
            ),
            (
                "1234.56789",
                [
                    tok.Token(
                        kind=tok.Kind.Float,
                        str="1234.56789",
                        source=tok.Source(s=loc(1, 1), e=loc(1, 11)),
                    )
                ],
            ),
            (
                "-1234.56789",
                [
                    tok.Token(
                        kind=tok.Kind.Float,
                        str="-1234.56789",
                        source=tok.Source(s=loc(1, 1), e=loc(1, 12)),
                    )
                ],
            ),
            (
                "123.456.789",
                [
                    tok.Token(
                        kind=tok.Kind.Float,
                        str="123.456",
                        source=tok.Source(s=loc(1, 1), e=loc(1, 8)),
                    ),
                    tok.Token(
                        kind=tok.Kind.Dot,
                        str=".",
                        source=tok.Source(s=loc(1, 8), e=loc(1, 9)),
                    ),
                    tok.Token(
                        kind=tok.Kind.Integer,
                        str="789",
                        source=tok.Source(s=loc(1, 9), e=loc(1, 12)),
                    ),
                ],
            ),
            (
                "-123.456-789",
                [
                    tok.Token(
                        kind=tok.Kind.Float,
                        str="-123.456",
                        source=tok.Source(s=loc(1, 1), e=loc(1, 9)),
                    ),
                    tok.Token(
                        kind=tok.Kind.Integer,
                        str="-789",
                        source=tok.Source(s=loc(1, 9), e=loc(1, 13)),
                    ),
                ],
            ),
            (
                "match",
                [
                    tok.Token(
                        kind=tok.Kind.Match,
                        str="match",
                        source=tok.Source(s=loc(1, 1), e=loc(1, 6)),
                    )
                ],
            ),
            (
                "fn",
                [
                    tok.Token(
                        kind=tok.Kind.Function,
                        str="fn",
                        source=tok.Source(s=loc(1, 1), e=loc(1, 3)),
                    )
                ],
            ),
            (
                "op",
                [
                    tok.Token(
                        kind=tok.Kind.Operator,
                        str="op",
                        source=tok.Source(s=loc(1, 1), e=loc(1, 3)),
                    )
                ],
            ),
            (
                "operation",
                [
                    tok.Token(
                        kind=tok.Kind.Identifier,
                        str="operation",
                        source=tok.Source(s=loc(1, 1), e=loc(1, 10)),
                    )
                ],
            ),
            (
                "type",
                [
                    tok.Token(
                        kind=tok.Kind.Type,
                        str="type",
                        source=tok.Source(s=loc(1, 1), e=loc(1, 5)),
                    )
                ],
            ),
            (
                "ctor",
                [
                    tok.Token(
                        kind=tok.Kind.Constructor,
                        str="ctor",
                        source=tok.Source(s=loc(1, 1), e=loc(1, 5)),
                    )
                ],
            ),
            (
                "conv",
                [
                    tok.Token(
                        kind=tok.Kind.Converter,
                        str="conv",
                        source=tok.Source(s=loc(1, 1), e=loc(1, 5)),
                    )
                ],
            ),
            (
                "enum",
                [
                    tok.Token(
                        kind=tok.Kind.Enum,
                        str="enum",
                        source=tok.Source(s=loc(1, 1), e=loc(1, 5)),
                    )
                ],
            ),
            (
                "import",
                [
                    tok.Token(
                        kind=tok.Kind.Import,
                        str="import",
                        source=tok.Source(s=loc(1, 1), e=loc(1, 7)),
                    )
                ],
            ),
            (
                ":",
                [
                    tok.Token(
                        kind=tok.Kind.Colon,
                        str=":",
                        source=tok.Source(s=loc(1, 1), e=loc(1, 2)),
                    )
                ],
            ),
            (
                ",",
                [
                    tok.Token(
                        kind=tok.Kind.Comma,
                        str=",",
                        source=tok.Source(s=loc(1, 1), e=loc(1, 2)),
                    )
                ],
            ),
            (
                "-",
                [
                    tok.Token(
                        kind=tok.Kind.Minus,
                        str="-",
                        source=tok.Source(s=loc(1, 1), e=loc(1, 2)),
                    )
                ],
            ),
            (
                "<",
                [
                    tok.Token(
                        kind=tok.Kind.Lt,
                        str="<",
                        source=tok.Source(s=loc(1, 1), e=loc(1, 2)),
                    )
                ],
            ),
            (
                ">",
                [
                    tok.Token(
                        kind=tok.Kind.Gt,
                        str=">",
                        source=tok.Source(s=loc(1, 1), e=loc(1, 2)),
                    )
                ],
            ),
            (
                "{",
                [
                    tok.Token(
                        kind=tok.Kind.Lbrace,
                        str="{",
                        source=tok.Source(s=loc(1, 1), e=loc(1, 2)),
                    )
                ],
            ),
            (
                "}",
                [
                    tok.Token(
                        kind=tok.Kind.Rbrace,
                        str="}",
                        source=tok.Source(s=loc(1, 1), e=loc(1, 2)),
                    )
                ],
            ),
            (
                "&&",
                [
                    tok.Token(
                        kind=tok.Kind.AndAnd,
                        str="&&",
                        source=tok.Source(s=loc(1, 1), e=loc(1, 3)),
                    )
                ],
            ),
            (
                "&",
                [
                    tok.Token(
                        kind=tok.Kind.And,
                        str="&",
                        source=tok.Source(s=loc(1, 1), e=loc(1, 2)),
                    )
                ],
            ),
            (
                "||",
                [
                    tok.Token(
                        kind=tok.Kind.OrOr,
                        str="||",
                        source=tok.Source(s=loc(1, 1), e=loc(1, 3)),
                    )
                ],
            ),
            (
                "|",
                [
                    tok.Token(
                        kind=tok.Kind.Or,
                        str="|",
                        source=tok.Source(s=loc(1, 1), e=loc(1, 2)),
                    )
                ],
            ),
            (
                "!",
                [
                    tok.Token(
                        kind=tok.Kind.Not,
                        str="!",
                        source=tok.Source(s=loc(1, 1), e=loc(1, 2)),
                    )
                ],
            ),
            (
                "!=",
                [
                    tok.Token(
                        kind=tok.Kind.NotEqual,
                        str="!=",
                        source=tok.Source(s=loc(1, 1), e=loc(1, 3)),
                    )
                ],
            ),
            (
                "==",
                [
                    tok.Token(
                        kind=tok.Kind.Equal,
                        str="==",
                        source=tok.Source(s=loc(1, 1), e=loc(1, 3)),
                    )
                ],
            ),
            (
                "=",
                [
                    tok.Token(
                        kind=tok.Kind.Assign,
                        str="=",
                        source=tok.Source(s=loc(1, 1), e=loc(1, 2)),
                    )
                ],
            ),
            (
                "<<",
                [
                    tok.Token(
                        kind=tok.Kind.Shl,
                        str="<<",
                        source=tok.Source(s=loc(1, 1), e=loc(1, 3)),
                    )
                ],
            ),
            (
                "<=",
                [
                    tok.Token(
                        kind=tok.Kind.Le,
                        str="<=",
                        source=tok.Source(s=loc(1, 1), e=loc(1, 3)),
                    )
                ],
            ),
            (
                "<",
                [
                    tok.Token(
                        kind=tok.Kind.Lt,
                        str="<",
                        source=tok.Source(s=loc(1, 1), e=loc(1, 2)),
                    )
                ],
            ),
            (
                ">=",
                [
                    tok.Token(
                        kind=tok.Kind.Ge,
                        str=">=",
                        source=tok.Source(s=loc(1, 1), e=loc(1, 3)),
                    )
                ],
            ),
            (
                ">>",
                [
                    tok.Token(
                        kind=tok.Kind.Shr,
                        str=">>",
                        source=tok.Source(s=loc(1, 1), e=loc(1, 3)),
                    )
                ],
            ),
            (
                ">",
                [
                    tok.Token(
                        kind=tok.Kind.Gt,
                        str=">",
                        source=tok.Source(s=loc(1, 1), e=loc(1, 2)),
                    )
                ],
            ),
            (
                "@",
                [
                    tok.Token(
                        kind=tok.Kind.Attr,
                        str="@",
                        source=tok.Source(s=loc(1, 1), e=loc(1, 2)),
                    )
                ],
            ),
            (
                "(",
                [
                    tok.Token(
                        kind=tok.Kind.Lparen,
                        str="(",
                        source=tok.Source(s=loc(1, 1), e=loc(1, 2)),
                    )
                ],
            ),
            (
                ")",
                [
                    tok.Token(
                        kind=tok.Kind.Rparen,
                        str=")",
                        source=tok.Source(s=loc(1, 1), e=loc(1, 2)),
                    )
                ],
            ),
            (
                "|",
                [
                    tok.Token(
                        kind=tok.Kind.Or,
                        str="|",
                        source=tok.Source(s=loc(1, 1), e=loc(1, 2)),
                    )
                ],
            ),
            (
                "*",
                [
                    tok.Token(
                        kind=tok.Kind.Star,
                        str="*",
                        source=tok.Source(s=loc(1, 1), e=loc(1, 2)),
                    )
                ],
            ),
            (
                "->",
                [
                    tok.Token(
                        kind=tok.Kind.Arrow,
                        str="->",
                        source=tok.Source(s=loc(1, 1), e=loc(1, 3)),
                    )
                ],
            ),
            (
                "x // y ",
                [
                    tok.Token(
                        kind=tok.Kind.Identifier,
                        str="x",
                        source=tok.Source(s=loc(1, 1), e=loc(1, 2)),
                    )
                ],
            ),
            (
                '"abc"',
                [
                    tok.Token(
                        kind=tok.Kind.String,
                        str="abc",
                        source=tok.Source(s=loc(1, 2), e=loc(1, 5)),
                    )
                ],
            ),
            (
                "\n   //\n   ident\n\n   ",
                [
                    tok.Token(
                        kind=tok.Kind.Identifier,
                        str="ident",
                        source=tok.Source(s=loc(3, 4), e=loc(3, 9)),
                    )
                ],
            ),
        ]

        for src, expect in cases:
            with self.subTest(src=src):
                got = lexer.lex(src, filepath)
                self.assertEqual(got, expect)

    def test_errors(self):
        cases = [
            (' "abc', "test.txt:1:2 unterminated string"),
            (' "abc\n', "test.txt:1:2 unterminated string"),
            ("£", "test.txt:1:1: unexpected '£'"),
        ]
        for src, expect in cases:
            with self.subTest(src=src):
                with self.assertRaises(lexer.LexError) as ctx:
                    lexer.lex(src, "test.txt")
                self.assertEqual(str(ctx.exception), expect)

    def test_tok_formatting(self):
        loc_no_file = tok.Location(line=2, column=4)
        self.assertEqual(str(loc_no_file), "2:4")

        loc_s = tok.Location(line=1, column=2, filepath="foo.def")
        loc_e = tok.Location(line=1, column=5, filepath="foo.def")
        self.assertEqual(str(loc_s), "foo.def:1:2")

        src = tok.Source(s=loc_s, e=loc_e)
        self.assertTrue(src.is_valid())
        self.assertFalse(tok.Source().is_valid())
        self.assertEqual(str(src), "foo.def:1:2")

        token = tok.Token(kind=tok.Kind.Identifier, str="abc", source=src)
        self.assertEqual(str(token), "[foo.def:1:2 ident abc]")


if __name__ == "__main__":
    unittest.main()
