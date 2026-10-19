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
"""Tokens produced by the Tint intrinsic definition lexer."""

from dataclasses import dataclass, field
import enum


class Kind(str, enum.Enum):
    """Enumerator of token kinds."""

    InvalidToken = "<invalid>"
    Identifier = "ident"
    Integer = "integer"
    Float = "float"
    String = "string"
    Match = "match"
    Import = "import"
    Implicit = "implicit"
    Function = "fn"
    Operator = "op"
    Constructor = "ctor"
    Converter = "conv"
    Type = "type"
    Enum = "enum"
    And = "&"
    AndAnd = "&&"
    Arrow = "->"
    Attr = "@"
    Assign = "="
    Colon = ":"
    Comma = ","
    Complement = "~"
    Divide = "/"
    Dot = "."
    Equal = "=="
    Ge = ">="
    Gt = ">"
    Lbrace = "{"
    Le = "<="
    Lparen = "("
    Lt = "<"
    Minus = "-"
    Modulo = "%"
    Not = "!"
    NotEqual = "!="
    Or = "|"
    OrOr = "||"
    Plus = "+"
    Rbrace = "}"
    Rparen = ")"
    Shl = "<<"
    Shr = ">>"
    Star = "*"
    Xor = "^"
    Lbracket = "["
    Rbracket = "]"

    def __str__(self) -> str:
        return self.value


@dataclass(frozen=True)
class Location:
    """Describes a character location in the source code."""

    # 1-based line index
    line: int = 0
    # 1-based column index
    column: int = 0
    # Optional file path
    filepath: str = ""

    def __str__(self) -> str:
        if self.filepath:
            return f"{self.filepath}:{self.line}:{self.column}"
        return f"{self.line}:{self.column}"


@dataclass(frozen=True)
class Source:
    """Describes a start and end range in the source code."""

    s: Location = field(default_factory=Location)
    e: Location = field(default_factory=Location)

    def is_valid(self) -> bool:
        return (self.s.line != 0 and self.s.column != 0 and self.e.line != 0
                and self.e.column != 0)

    def __str__(self) -> str:
        return str(self.s)


@dataclass(frozen=True)
class Token:
    """Describes a parsed token."""

    kind: Kind
    runes: str = ""
    source: Source = field(default_factory=Source)

    def __str__(self) -> str:
        return f"[{self.source} {self.kind.value} {self.runes}]"


Invalid = Token(kind=Kind.InvalidToken)
