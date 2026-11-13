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
"""Semantic model for the Tint intrinsic definition language."""

from __future__ import annotations

from dataclasses import dataclass, field
import enum
from typing import Any, Optional, Union

import def_ast as ast


def _format_list(items: list[Any]) -> str:
    return ", ".join(str(item) for item in items)


class TemplateKind(str, enum.Enum):
    """Enumerator of template parameter kinds."""

    Type = "Type"
    Number = "Number"
    Access = "access"
    AddressSpace = "address_space"
    SubgroupMatrixKind = "subgroup_matrix_kind"
    Majorness = "majorness"
    TexelFormat = "texel_format"

    def __str__(self) -> str:
        return self.value


@dataclass(eq=False)
class EnumEntry:
    """An entry in an enumerator."""

    enum: Optional[Enum] = None
    name: str = ""
    # True if this entry is not part of the WGSL grammar
    is_internal: bool = False

    def get_name(self) -> str:
        return self.name

    def __str__(self) -> str:
        if self.is_internal:
            return f"[[internal]] {self.name}"
        return self.name


@dataclass(eq=False)
class Enum:
    """Describes an enumerator."""

    decl: ast.EnumDecl = field(default_factory=ast.EnumDecl)
    name: str = ""
    ns: str = "core"
    entries: list[EnumEntry] = field(default_factory=list)

    def get_name(self) -> str:
        return self.name

    def find_entry(self, name: str) -> Optional[EnumEntry]:
        for entry in self.entries:
            if entry.name == name:
                return entry
        return None

    @property
    def public_entries(self) -> list[EnumEntry]:
        """Returns the enum entries that are not annotated with @internal."""
        return [entry for entry in self.entries if not entry.is_internal]

    @property
    def name_set(self) -> set[str]:
        """Returns a set of all the enum entry names."""
        return {entry.name for entry in self.entries}


@dataclass(eq=False)
class Type:
    """Declares a semantic type."""

    template_params: list[TemplateParam] = field(default_factory=list)
    decl: ast.TypeDecl = field(default_factory=ast.TypeDecl)
    name: str = ""
    display_name: str = ""
    precedence: int = 0

    def get_name(self) -> str:
        return self.name


@dataclass(eq=False)
class TypeMatcher:
    """Declares a type matcher."""

    decl: ast.MatcherDecl = field(default_factory=ast.MatcherDecl)
    name: str = ""
    types: list[Type] = field(default_factory=list)

    def get_name(self) -> str:
        return self.name

    @property
    def precedence_sorted_types(self) -> list[Type]:
        return sorted(self.types, key=lambda ty: ty.precedence, reverse=True)


@dataclass(eq=False)
class EnumMatcher:
    """Declares an enum matcher."""

    template_params: list[TemplateParam] = field(default_factory=list)
    decl: ast.MatcherDecl = field(default_factory=ast.MatcherDecl)
    name: str = ""
    enum: Optional[Enum] = None
    options: list[EnumEntry] = field(default_factory=list)

    def get_name(self) -> str:
        return self.name


@dataclass
class FullyQualifiedName:
    """Usage of a Type, TypeMatcher, Enum, EnumEntry, EnumMatcher, or TemplateParam."""

    target: Node
    template_arguments: list[Any] = field(default_factory=list)

    def __str__(self) -> str:
        out = self.target.get_name()
        if self.template_arguments:
            out += f"<{_format_list(self.template_arguments)}>"
        return out


@dataclass(eq=False)
class TemplateTypeParam:
    """A template type parameter."""

    name: str = ""
    ast_param: ast.TemplateParam = field(default_factory=ast.TemplateParam)
    type: Optional[FullyQualifiedName] = None

    def get_name(self) -> str:
        return self.name

    def ast(self) -> ast.TemplateParam:
        return self.ast_param

    def template_kind(self) -> TemplateKind:
        return TemplateKind.Type

    def __str__(self) -> str:
        if self.type is not None:
            return f"{self.name}: {self.type}"
        return self.name


_ENUM_TEMPLATE_KINDS: dict[str, TemplateKind] = {
    "access": TemplateKind.Access,
    "address_space": TemplateKind.AddressSpace,
    "subgroup_matrix_kind": TemplateKind.SubgroupMatrixKind,
    "majorness": TemplateKind.Majorness,
    "texel_format": TemplateKind.TexelFormat,
}


@dataclass(eq=False)
class TemplateEnumParam:
    """A template enum parameter."""

    name: str = ""
    ast_param: ast.TemplateParam = field(default_factory=ast.TemplateParam)
    enum: Optional[Enum] = None
    matcher: Optional[EnumMatcher] = None

    def get_name(self) -> str:
        return self.name

    def ast(self) -> ast.TemplateParam:
        return self.ast_param

    def template_kind(self) -> TemplateKind:
        if self.enum is not None and self.enum.name in _ENUM_TEMPLATE_KINDS:
            return _ENUM_TEMPLATE_KINDS[self.enum.name]
        return TemplateKind.Number

    def __str__(self) -> str:
        if self.enum is not None:
            return f"{self.name}: {self.enum}"
        return self.name


@dataclass(eq=False)
class TemplateEnumEntryParam:
    """A specific template enum entry parameter (used during permutation)."""

    name: str = ""
    ast_param: ast.TemplateParam = field(default_factory=ast.TemplateParam)
    entry: Optional[FullyQualifiedName] = None

    def get_name(self) -> str:
        return self.name

    def ast(self) -> ast.TemplateParam:
        return self.ast_param

    def template_kind(self) -> TemplateKind:
        return TemplateKind.Number

    def __str__(self) -> str:
        if self.entry is not None:
            return f"{self.name}: {self.entry}"
        return self.name


@dataclass(eq=False)
class TemplateNumberParam:
    """A template number parameter."""

    name: str = ""
    ast_param: ast.TemplateParam = field(default_factory=ast.TemplateParam)

    def get_name(self) -> str:
        return self.name

    def ast(self) -> ast.TemplateParam:
        return self.ast_param

    def template_kind(self) -> TemplateKind:
        return TemplateKind.Number

    def __str__(self) -> str:
        return f"{self.name}: num"


TemplateParam = Union[
    TemplateTypeParam,
    TemplateEnumParam,
    TemplateEnumEntryParam,
    TemplateNumberParam,
]

ResolvableType = Union[Type, TypeMatcher, TemplateTypeParam]

Node = Union[
    Enum,
    EnumEntry,
    Type,
    TypeMatcher,
    EnumMatcher,
    TemplateTypeParam,
    TemplateEnumParam,
    TemplateEnumEntryParam,
    TemplateNumberParam,
]


class IntrinsicTemplates(list[TemplateParam]):
    """A list of TemplateParam used by an Intrinsic Overload."""

    def types(self) -> list[TemplateTypeParam]:
        """Returns all the template type parameters."""
        return [p for p in self if isinstance(p, TemplateTypeParam)]

    def numbers(self) -> list[TemplateParam]:
        """Returns all the template number/enum parameters."""
        return [p for p in self if not isinstance(p, TemplateTypeParam)]


@dataclass
class StageUses:
    """Describes the shader stages an overload can be used in."""

    vertex: bool = False
    fragment: bool = False
    compute: bool = False

    def list(self) -> list[str]:
        out: list[str] = []
        if self.vertex:
            out.append("vertex")
        if self.fragment:
            out.append("fragment")
        if self.compute:
            out.append("compute")
        return out


@dataclass
class Parameter:
    """Describes a single parameter of a function overload."""

    name: str = ""
    type: Optional[FullyQualifiedName] = None
    # Did this parameter have a @const attribute?
    is_const: bool = False
    # Value to use for end-to-end tests
    test_value: float = 1.0

    def __str__(self) -> str:
        out = "@const " if self.is_const else ""
        if self.name:
            out += f"{self.name}: "
        out += str(self.type)
        return out


@dataclass(eq=False)
class Overload:
    """Describes a single overload of a builtin or operator."""

    decl: ast.IntrinsicDecl = field(default_factory=ast.IntrinsicDecl)
    intrinsic: Optional[Intrinsic] = None
    explicit_templates: IntrinsicTemplates = field(
        default_factory=IntrinsicTemplates)
    implicit_templates: IntrinsicTemplates = field(
        default_factory=IntrinsicTemplates)
    return_type: Optional[FullyQualifiedName] = None
    parameters: list[Parameter] = field(default_factory=list)
    can_be_used_in_stage: StageUses = field(default_factory=StageUses)
    must_use: bool = False
    member_function: bool = False
    is_deprecated: bool = False
    const_eval_function: str = ""

    def all_templates(self) -> IntrinsicTemplates:
        return IntrinsicTemplates(
            list(self.explicit_templates) + list(self.implicit_templates))

    def __str__(self) -> str:
        prefix = ""
        if self.decl.kind == ast.IntrinsicKind.Builtin:
            prefix = "fn "
        elif self.decl.kind == ast.IntrinsicKind.Operator:
            prefix = "op "
        name = self.intrinsic.name if self.intrinsic is not None else self.decl.name
        out = f"{prefix}{name}"
        if self.explicit_templates:
            out += f"<{_format_list(self.explicit_templates)}>"
        if self.implicit_templates:
            out += f"[{_format_list(self.implicit_templates)}]"
        out += f"({_format_list(self.parameters)})"
        if self.return_type is not None:
            out += f" -> {self.return_type}"
        return out


@dataclass(eq=False)
class Intrinsic:
    """Describes the overloads of a builtin, operator, constructor, or converter."""

    name: str = ""
    overloads: list[Overload] = field(default_factory=list)


@dataclass
class Sem:
    """Root of the semantic tree."""

    enums: list[Enum] = field(default_factory=list)
    types: list[Type] = field(default_factory=list)
    type_matchers: list[TypeMatcher] = field(default_factory=list)
    enum_matchers: list[EnumMatcher] = field(default_factory=list)
    builtins: list[Intrinsic] = field(default_factory=list)
    unary_operators: list[Intrinsic] = field(default_factory=list)
    binary_operators: list[Intrinsic] = field(default_factory=list)
    constructors_and_converters: list[Intrinsic] = field(default_factory=list)
    # Maximum number of template types used across all builtins
    max_templates: int = 0
    # The alphabetically sorted list of unique parameter names
    unique_parameter_names: list[str] = field(default_factory=list)

    def enum(self, name: str) -> Optional[Enum]:
        """Returns the enum with the given name, or None."""
        for e in self.enums:
            if e.name == name:
                return e
        return None
