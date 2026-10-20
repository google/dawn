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
"""AST nodes produced by the Tint intrinsic definition parser."""

from __future__ import annotations

from dataclasses import dataclass, field
import enum
from typing import Any, Optional

import tok


def _format_val(item: Any) -> str:
    if isinstance(item, float) and item.is_integer():
        return str(int(item))
    return str(item)


def _format_list(items: list[Any]) -> str:
    return ", ".join(_format_val(item) for item in items)


@dataclass
class Attribute:
    """Describes a single attribute (e.g. @a(x))."""

    name: str = ""
    values: list[Any] = field(default_factory=list)
    source: tok.Source = field(default_factory=tok.Source, compare=False)

    def __str__(self) -> str:
        if self.values:
            return f"{self.name}({_format_list(self.values)})"
        return self.name


class Attributes(list[Attribute]):
    """A list of Attribute nodes with lookup/removal support."""

    def take(self, name: str) -> Optional[Attribute]:
        """Removes and returns the first attribute matching `name`, or None."""
        for i, attr in enumerate(self):
            if attr.name == name:
                return self.pop(i)
        return None

    def __str__(self) -> str:
        return "".join(f"@{attr} " for attr in self)


@dataclass
class TemplatedName:
    """An identifier with optional template arguments (e.g. vec<N, T>)."""

    name: str = ""
    template_args: list[TemplatedName] = field(default_factory=list)
    source: tok.Source = field(default_factory=tok.Source, compare=False)

    def __str__(self) -> str:
        if self.template_args:
            return f"{self.name}<{_format_list(self.template_args)}>"
        return self.name


@dataclass
class MemberName:
    """Two identifiers separated by a dot (Owner.Member)."""

    owner: str = ""
    member: str = ""
    source: tok.Source = field(default_factory=tok.Source, compare=False)

    def __str__(self) -> str:
        return f"{self.owner}.{self.member}"


@dataclass
class TemplateParam:
    """A template parameter with optional type (e.g. <Name> or <Name: Type>)."""

    name: str = ""
    type: TemplatedName = field(default_factory=TemplatedName)
    source: tok.Source = field(default_factory=tok.Source, compare=False)

    def __str__(self) -> str:
        if self.type.name:
            return f"{self.name} : {self.type}"
        return self.name


@dataclass
class EnumEntry:
    """Describes an entry in an enumerator."""

    name: str = ""
    attributes: Attributes = field(default_factory=Attributes)
    source: tok.Source = field(default_factory=tok.Source, compare=False)

    def __str__(self) -> str:
        if self.attributes:
            return f"{self.attributes} {self.name}"
        return self.name


@dataclass
class EnumDecl:
    """Describes an enumerator declaration."""

    name: str = ""
    attributes: Attributes = field(default_factory=Attributes)
    entries: list[EnumEntry] = field(default_factory=list)
    source: tok.Source = field(default_factory=tok.Source, compare=False)

    def __str__(self) -> str:
        lines = [f"enum {self.name} {{\n"]
        for entry in self.entries:
            lines.append(f"  {entry}\n")
        lines.append("}\n")
        return "".join(lines)


@dataclass
class TypeDecl:
    """Describes a type declaration."""

    name: str = ""
    attributes: Attributes = field(default_factory=Attributes)
    template_params: list[TemplateParam] = field(default_factory=list)
    source: tok.Source = field(default_factory=tok.Source, compare=False)

    def __str__(self) -> str:
        out = ""
        if self.attributes:
            out += f"{self.attributes} type {self.name}"
        out += f"type {self.name}"
        if self.template_params:
            out += f"<{_format_list(self.template_params)}>"
        return out


@dataclass
class MatcherOptions:
    """A list of TemplatedNames or MemberNames in a matcher declaration."""

    types: list[TemplatedName] = field(default_factory=list)
    enums: list[MemberName] = field(default_factory=list)

    def __str__(self) -> str:
        parts: list[str] = []
        if self.types:
            parts.append(" | ".join(str(t) for t in self.types))
        if self.enums:
            parts.append(" | ".join(str(e) for e in self.enums))
        return "".join(parts)


@dataclass
class MatcherDecl:
    """Describes a matcher declaration."""

    name: str = ""
    options: MatcherOptions = field(default_factory=MatcherOptions)
    source: tok.Source = field(default_factory=tok.Source, compare=False)

    def __str__(self) -> str:
        return f"match {self.name}: {self.options}"


class IntrinsicKind(str, enum.Enum):
    """Kind of intrinsic declaration."""

    Builtin = "builtin"
    Operator = "operator"
    Constructor = "constructor"
    Converter = "converter"

    def __str__(self) -> str:
        return self.value


@dataclass
class Parameter:
    """Describes a single parameter of a function."""

    name: str = ""
    attributes: Attributes = field(default_factory=Attributes)
    type: TemplatedName = field(default_factory=TemplatedName)
    source: tok.Source = field(default_factory=tok.Source, compare=False)

    def __str__(self) -> str:
        if self.name:
            return f"{self.name}: {self.type}"
        return str(self.type)


def format_parameters(params: list[Parameter]) -> str:
    parts = [f"{p.attributes}{p}" for p in params]
    return f"({', '.join(parts)})"


@dataclass
class IntrinsicDecl:
    """Describes a builtin, operator, constructor, or converter declaration."""

    kind: IntrinsicKind = IntrinsicKind.Builtin
    name: str = ""
    attributes: Attributes = field(default_factory=Attributes)
    implicit_template_params: list[TemplateParam] = field(default_factory=list)
    explicit_template_params: list[TemplateParam] = field(default_factory=list)
    parameters: list[Parameter] = field(default_factory=list)
    return_type: Optional[TemplatedName] = None
    source: tok.Source = field(default_factory=tok.Source, compare=False)

    def __str__(self) -> str:
        prefix = {
            IntrinsicKind.Builtin: "fn ",
            IntrinsicKind.Operator: "op ",
            IntrinsicKind.Constructor: "ctor ",
            IntrinsicKind.Converter: "conv ",
        }[self.kind]
        out = f"{prefix}{self.name}"
        if self.explicit_template_params:
            out += f"<{_format_list(self.explicit_template_params)}>"
        if self.implicit_template_params:
            out += f"[{_format_list(self.implicit_template_params)}]"
        out += format_parameters(self.parameters)
        if self.return_type is not None:
            out += f" -> {self.return_type}"
        return out


@dataclass
class AST:
    """The parsed syntax tree of an intrinsic definition file."""

    enums: list[EnumDecl] = field(default_factory=list)
    types: list[TypeDecl] = field(default_factory=list)
    matchers: list[MatcherDecl] = field(default_factory=list)
    builtins: list[IntrinsicDecl] = field(default_factory=list)
    constructors: list[IntrinsicDecl] = field(default_factory=list)
    converters: list[IntrinsicDecl] = field(default_factory=list)
    operators: list[IntrinsicDecl] = field(default_factory=list)

    def __str__(self) -> str:
        lines: list[str] = []
        for group in (
                self.enums,
                self.types,
                self.matchers,
                self.builtins,
                self.constructors,
                self.converters,
                self.operators,
        ):
            for item in group:
                lines.append(f"{item}\n")
        return "".join(lines)
