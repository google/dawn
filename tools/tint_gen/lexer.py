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
"""Lexer for the Tint intrinsic definition language."""

from typing import Callable
import tok

_KEYWORDS: dict[str, tok.Kind] = {
    "fn": tok.Kind.Function,
    "op": tok.Kind.Operator,
    "enum": tok.Kind.Enum,
    "type": tok.Kind.Type,
    "ctor": tok.Kind.Constructor,
    "conv": tok.Kind.Converter,
    "match": tok.Kind.Match,
    "import": tok.Kind.Import,
    "implicit": tok.Kind.Implicit,
}

# Tokens that are greedily consumed and never begin a longer token.
_SINGLE_CHAR_TOKENS: dict[str, tok.Kind] = {
    "@": tok.Kind.Attr,
    "(": tok.Kind.Lparen,
    ")": tok.Kind.Rparen,
    "{": tok.Kind.Lbrace,
    "}": tok.Kind.Rbrace,
    "[": tok.Kind.Lbracket,
    "]": tok.Kind.Rbracket,
    ":": tok.Kind.Colon,
    ",": tok.Kind.Comma,
    "*": tok.Kind.Star,
    "+": tok.Kind.Plus,
    "%": tok.Kind.Modulo,
    "^": tok.Kind.Xor,
    "~": tok.Kind.Complement,
    ".": tok.Kind.Dot,
}

# Tokens that need to be checked in order to avoid ambiguities.
_MATCH_TOKENS: tuple[tuple[str, tok.Kind], ...] = (
    ("&&", tok.Kind.AndAnd),
    ("&", tok.Kind.And),
    ("||", tok.Kind.OrOr),
    ("|", tok.Kind.Or),
    ("!=", tok.Kind.NotEqual),
    ("!", tok.Kind.Not),
    ("==", tok.Kind.Equal),
    ("=", tok.Kind.Assign),
    ("<<", tok.Kind.Shl),
    ("<=", tok.Kind.Le),
    ("<", tok.Kind.Lt),
    (">=", tok.Kind.Ge),
    (">>", tok.Kind.Shr),
    (">", tok.Kind.Gt),
)


class LexError(Exception):
    """Raised when lexing fails."""


def lex(src: str, filepath: str = "") -> list[tok.Token]:
    """Produces a list of tokens for the given source code."""
    lexer = _Lexer(src=src, filepath=filepath)
    lexer.lex()
    return lexer.tokens


class _Lexer:

    def __init__(self, src: str, filepath: str):
        self.src = src
        self.pos = 0
        self.line = 1
        self.column = 1
        self.filepath = filepath
        self.tokens: list[tok.Token] = []

    @property
    def loc(self) -> tok.Location:
        return tok.Location(
            line=self.line,
            column=self.column,
            filepath=self.filepath,
        )

    def lex(self) -> None:
        while True:
            ch = self._peek(0)
            if ch == "":
                return
            if ch in (" ", "\t", "\r", "\n"):
                self._next()
            elif ch in _SINGLE_CHAR_TOKENS:
                self._append_tok(1, _SINGLE_CHAR_TOKENS[ch])
            elif ch == '"':
                start = self.loc
                self._next()  # Skip opening quote
                n = self._count(_to_first("\n", '"'))
                if self._peek(n) != '"':
                    raise LexError(f"{start} unterminated string")
                self._append_tok(n, tok.Kind.String)
                self._next()  # Skip closing quote
            elif ch == "/" and self._peek(1) == "/":
                self._skip(self._count(_to_first("\n")))
                self._next()  # Consume newline
            elif self._match("/", tok.Kind.Divide):
                pass
            elif self._match("->", tok.Kind.Arrow):
                pass
            elif ch.isalpha() or ch == "_":
                n = self._count(_alpha_numeric_or_underscore)
                word = self.src[self.pos:self.pos + n]
                kind = _KEYWORDS.get(word, tok.Kind.Identifier)
                self._append_tok(n, kind)
            elif ch.isnumeric() or ch == "-":
                is_float = False
                is_negative = False
                is_first = True

                def pred(r: str) -> bool:
                    nonlocal is_float, is_negative, is_first
                    if is_first and r == "-":
                        is_negative = True
                        is_first = False
                        return True
                    is_first = False
                    if r.isnumeric():
                        return True
                    if not is_float and r == ".":
                        is_float = True
                        return True
                    return False

                n = self._count(pred)
                if is_negative and n == 1:
                    self._append_tok(1, tok.Kind.Minus)
                elif is_float:
                    self._append_tok(n, tok.Kind.Float)
                else:
                    self._append_tok(n, tok.Kind.Integer)
            else:
                matched = False
                for pattern, kind in _MATCH_TOKENS:
                    if self._match(pattern, kind):
                        matched = True
                        break
                if not matched:
                    raise LexError(f"{self.loc}: unexpected '{ch}'")

    def _next(self) -> str:
        if self.pos < len(self.src):
            r = self.src[self.pos]
            self.pos += 1
            if r == "\n":
                self.line += 1
                self.column = 1
            else:
                self.column += 1
            return r
        return ""

    def _skip(self, n: int) -> None:
        self.pos += n
        self.column += n

    def _peek(self, offset: int) -> str:
        idx = self.pos + offset
        if idx >= len(self.src):
            return ""
        return self.src[idx]

    def _count(self, predicate: Callable[[str], bool]) -> int:
        remaining = len(self.src) - self.pos
        for i in range(remaining):
            if not predicate(self.src[self.pos + i]):
                return i
        return remaining

    def _append_tok(self, n: int, kind: tok.Kind) -> None:
        start = self.loc
        str = self.src[self.pos:self.pos + n]
        self._skip(n)
        end = self.loc
        self.tokens.append(
            tok.Token(
                kind=kind,
                str=str,
                source=tok.Source(s=start, e=end),
            ))

    def _match(self, s: str, kind: tok.Kind) -> bool:
        if self.src.startswith(s, self.pos):
            self._append_tok(len(s), kind)
            return True
        return False


def _to_first(*stop_chars: str) -> Callable[[str], bool]:
    stop_set = set(stop_chars)
    return lambda r: r not in stop_set


def _alpha_numeric_or_underscore(r: str) -> bool:
    return r == "_" or r.isalpha() or r.isnumeric()
