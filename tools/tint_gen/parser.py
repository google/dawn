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
"""Recursive-descent parser for the Tint intrinsic definition language."""

import os
from pathlib import Path
from typing import Any, Mapping, Optional

import def_ast as ast
import lexer
import tok

DAWN_ROOT = Path(__file__).resolve().parents[2]


class ParseError(Exception):
    """Raised when parsing fails."""


def parse(
    source: str,
    filepath: str = "",
    fs_reader: Optional[Mapping[str, str]] = None,
) -> ast.AST:
    """Parses intrinsic definition source code into an AST."""
    out = ast.AST()
    _parse_into(source, filepath, fs_reader, out)
    return out


def _read_imported_file(
    path: str,
    fs_reader: Optional[Mapping[str, str]],
) -> str:
    if fs_reader is None:
        abs_path = os.path.join(DAWN_ROOT, path)
        with open(abs_path, "r", encoding="utf-8") as f:
            return f.read()
    if path in fs_reader:
        return fs_reader[path]
    raise FileNotFoundError(f"open {path}: file does not exist")


def _parse_into(
    source: str,
    filepath: str,
    fs_reader: Optional[Mapping[str, str]],
    out: ast.AST,
) -> None:
    try:
        tokens = lexer.lex(source, filepath)
    except lexer.LexError as e:
        raise ParseError(str(e)) from e
    p = _Parser(tokens=tokens, fs_reader=fs_reader)
    p.parse(out)


class _Parser:

    def __init__(
        self,
        tokens: list[tok.Token],
        fs_reader: Optional[Mapping[str, str]],
    ):
        self.tokens = tokens
        self.pos = 0
        self.fs_reader = fs_reader

    def parse(self, out: ast.AST) -> None:
        attributes = ast.Attributes()
        implicit_params: list[ast.TemplateParam] = []
        while True:
            t = self._peek(0)
            if t is None:
                break
            if t.kind == tok.Kind.Attr:
                attributes.extend(self._attributes())
            elif t.kind == tok.Kind.Enum:
                if implicit_params:
                    raise ParseError(
                        f"{implicit_params[0].source} unexpected implicitParams"
                    )
                out.enums.append(self._enum_decl(attributes))
                attributes = ast.Attributes()
            elif t.kind == tok.Kind.Match:
                if attributes:
                    raise ParseError(
                        f"{attributes[0].source} unexpected attribute")
                if implicit_params:
                    raise ParseError(
                        f"{implicit_params[0].source} unexpected implicitParams"
                    )
                out.matchers.append(self._matcher_decl())
            elif t.kind == tok.Kind.Import:
                if attributes:
                    raise ParseError(
                        f"{attributes[0].source} unexpected attribute")
                if implicit_params:
                    raise ParseError(
                        f"{implicit_params[0].source} unexpected implicitParams"
                    )
                self._import_decl(out)
            elif t.kind == tok.Kind.Type:
                if implicit_params:
                    raise ParseError(
                        f"{implicit_params[0].source} unexpected implicitParams"
                    )
                out.types.append(self._type_decl(attributes))
                attributes = ast.Attributes()
            elif t.kind == tok.Kind.Function:
                out.builtins.append(
                    self._builtin_decl(attributes, implicit_params))
                attributes = ast.Attributes()
                implicit_params = []
            elif t.kind == tok.Kind.Operator:
                out.operators.append(
                    self._operator_decl(attributes, implicit_params))
                attributes = ast.Attributes()
                implicit_params = []
            elif t.kind == tok.Kind.Constructor:
                out.constructors.append(
                    self._constructor_decl(attributes, implicit_params))
                attributes = ast.Attributes()
                implicit_params = []
            elif t.kind == tok.Kind.Converter:
                out.converters.append(
                    self._converter_decl(attributes, implicit_params))
                attributes = ast.Attributes()
                implicit_params = []
            elif t.kind == tok.Kind.Implicit:
                self._expect(tok.Kind.Implicit, "implicit")
                self._expect(tok.Kind.Lparen,
                             "implicit template parameter list")
                while self._peek_is(0, tok.Kind.Identifier):
                    implicit_params.append(self._template_param())
                    if self._match(tok.Kind.Comma) is None:
                        break
                self._expect(tok.Kind.Rparen,
                             "implicit template parameter list")
            else:
                raise ParseError(f"{t.source} unexpected token '{t.kind}'")

    def _enum_decl(self, decos: ast.Attributes) -> ast.EnumDecl:
        self._expect(tok.Kind.Enum, "enum declaration")
        name = self._expect(tok.Kind.Identifier, "enum name")
        e = ast.EnumDecl(
            source=name.source,
            name=name.str,
            attributes=decos,
        )
        self._expect(tok.Kind.Lbrace, "enum declaration")
        while self._match(tok.Kind.Rbrace) is None:
            e.entries.append(self._enum_entry())
        return e

    def _enum_entry(self) -> ast.EnumEntry:
        decos = self._attributes()
        name = self._expect(tok.Kind.Identifier, "enum entry")
        return ast.EnumEntry(
            source=name.source,
            attributes=decos,
            name=name.str,
        )

    def _matcher_decl(self) -> ast.MatcherDecl:
        self._expect(tok.Kind.Match, "matcher declaration")
        name = self._expect(tok.Kind.Identifier, "matcher name")
        m = ast.MatcherDecl(source=name.source, name=name.str)
        self._expect(tok.Kind.Colon, "matcher declaration")
        if self._peek_is(1, tok.Kind.Dot):  # enum list
            while True:
                m.options.enumerants.append(self._member_name())
                if self._match(tok.Kind.Or) is None:
                    break
        else:  # type list
            while True:
                m.options.types.append(self._templated_name())
                if self._match(tok.Kind.Or) is None:
                    break
        return m

    def _import_decl(self, out: ast.AST) -> None:
        t = self._expect(tok.Kind.Import, "import declaration")
        path = self._string()
        try:
            content = _read_imported_file(path, self.fs_reader)
        except Exception as e:
            raise ParseError(f"{t.source} failed to load '{path}': {e}") from e
        _parse_into(content, path, self.fs_reader, out)

    def _type_decl(self, decos: ast.Attributes) -> ast.TypeDecl:
        self._expect(tok.Kind.Type, "type declaration")
        name = self._expect(tok.Kind.Identifier, "type name")
        return ast.TypeDecl(
            source=name.source,
            attributes=decos,
            name=name.str,
            template_params=self._type_template_params(),
        )

    def _attributes(self) -> ast.Attributes:
        out = ast.Attributes()
        while self._match(tok.Kind.Attr) is not None:
            name = self._expect(tok.Kind.Identifier, "attribute name")
            values: list[Any] = []
            if self._match(tok.Kind.Lparen) is not None:
                while True:
                    t = self._next()
                    if t.kind == tok.Kind.Rparen:
                        break
                    if t.kind == tok.Kind.String:
                        values.append(t.str)
                    elif t.kind == tok.Kind.Integer:
                        values.append(int(t.str))
                    elif t.kind == tok.Kind.Float:
                        values.append(float(t.str))
                    else:
                        raise ParseError(
                            f"{t.source} invalid attribute value kind: {t.kind}"
                        )
                    if self._match(tok.Kind.Comma) is None:
                        break
                self._expect(tok.Kind.Rparen, "attribute values")
            out.append(
                ast.Attribute(
                    source=name.source,
                    name=name.str,
                    values=values,
                ))
        return out

    def _builtin_decl(
        self,
        decos: ast.Attributes,
        implicit_params: list[ast.TemplateParam],
    ) -> ast.IntrinsicDecl:
        self._expect(tok.Kind.Function, "function declaration")
        name = self._expect(tok.Kind.Identifier, "function name")
        return self._finish_intrinsic_decl(
            kind=ast.IntrinsicKind.Builtin,
            name_tok=name,
            decos=decos,
            implicit_params=implicit_params,
        )

    def _operator_decl(
        self,
        decos: ast.Attributes,
        implicit_params: list[ast.TemplateParam],
    ) -> ast.IntrinsicDecl:
        self._expect(tok.Kind.Operator, "operator declaration")
        name = self._next()
        return self._finish_intrinsic_decl(
            kind=ast.IntrinsicKind.Operator,
            name_tok=name,
            decos=decos,
            implicit_params=implicit_params,
        )

    def _constructor_decl(
        self,
        decos: ast.Attributes,
        implicit_params: list[ast.TemplateParam],
    ) -> ast.IntrinsicDecl:
        self._expect(tok.Kind.Constructor, "constructor declaration")
        name = self._expect(tok.Kind.Identifier, "constructor name")
        return self._finish_intrinsic_decl(
            kind=ast.IntrinsicKind.Constructor,
            name_tok=name,
            decos=decos,
            implicit_params=implicit_params,
        )

    def _converter_decl(
        self,
        decos: ast.Attributes,
        implicit_params: list[ast.TemplateParam],
    ) -> ast.IntrinsicDecl:
        self._expect(tok.Kind.Converter, "converter declaration")
        name = self._expect(tok.Kind.Identifier, "converter name")
        return self._finish_intrinsic_decl(
            kind=ast.IntrinsicKind.Converter,
            name_tok=name,
            decos=decos,
            implicit_params=implicit_params,
        )

    def _finish_intrinsic_decl(
        self,
        kind: ast.IntrinsicKind,
        name_tok: tok.Token,
        decos: ast.Attributes,
        implicit_params: list[ast.TemplateParam],
    ) -> ast.IntrinsicDecl:
        explicit_params = self._explicit_template_params()
        params = self._parameters()
        ret: Optional[ast.TemplatedName] = None
        if self._match(tok.Kind.Arrow) is not None:
            ret = self._templated_name()
        return ast.IntrinsicDecl(
            source=name_tok.source,
            kind=kind,
            attributes=decos,
            name=name_tok.str,
            explicit_template_params=explicit_params,
            implicit_template_params=list(implicit_params),
            parameters=params,
            return_type=ret,
        )

    def _parameters(self) -> list[ast.Parameter]:
        params: list[ast.Parameter] = []
        self._expect(tok.Kind.Lparen, "function parameter list")
        if self._match(tok.Kind.Rparen) is None:
            while True:
                params.append(self._parameter())
                if self._match(tok.Kind.Comma) is None:
                    break
            self._expect(tok.Kind.Rparen, "function parameter list")
        return params

    def _parameter(self) -> ast.Parameter:
        attributes = self._attributes()
        if self._peek_is(1, tok.Kind.Colon):
            name = self._expect(tok.Kind.Identifier, "parameter name")
            self._expect(tok.Kind.Colon, "parameter type")
            return ast.Parameter(
                source=name.source,
                name=name.str,
                attributes=attributes,
                type=self._templated_name(),
            )
        ty = self._templated_name()
        return ast.Parameter(
            source=ty.source,
            attributes=attributes,
            type=ty,
        )

    def _string(self) -> str:
        return self._expect(tok.Kind.String, "string").str

    def _member_name(self) -> ast.MemberName:
        owner = self._expect(tok.Kind.Identifier, "member name")
        self._expect(tok.Kind.Dot, "member name")
        member = self._expect(tok.Kind.Identifier, "member name")
        return ast.MemberName(
            source=member.source,
            owner=owner.str,
            member=member.str,
        )

    def _templated_name(self) -> ast.TemplatedName:
        name = self._expect(tok.Kind.Identifier, "type name")
        m = ast.TemplatedName(source=name.source, name=name.str)
        if self._match(tok.Kind.Lt) is not None:
            while True:
                m.template_args.append(self._templated_name())
                if self._match(tok.Kind.Comma) is None:
                    break
            self._expect(tok.Kind.Gt, "template argument type list")
        return m

    def _type_template_params(self) -> list[ast.TemplateParam]:
        if self._match(tok.Kind.Lt) is None:
            return []
        params: list[ast.TemplateParam] = []
        while self._peek_is(0, tok.Kind.Identifier):
            params.append(self._template_param())
            if self._match(tok.Kind.Comma) is None:
                break
        self._expect(tok.Kind.Gt, "type template parameter list")
        return params

    def _explicit_template_params(self) -> list[ast.TemplateParam]:
        explicit: list[ast.TemplateParam] = []
        if self._match(tok.Kind.Lt) is not None:
            while self._peek_is(0, tok.Kind.Identifier):
                explicit.append(self._template_param())
                if self._match(tok.Kind.Comma) is None:
                    break
            self._expect(tok.Kind.Gt, "explicit template parameter list")
        return explicit

    def _template_param(self) -> ast.TemplateParam:
        name = self._expect(tok.Kind.Identifier, "template parameter name")
        t = ast.TemplateParam(source=name.source, name=name.str)
        if self._match(tok.Kind.Colon) is not None:
            t.type = self._templated_name()
        return t

    def _expect(self, kind: tok.Kind, use: str) -> tok.Token:
        t = self._match(kind)
        if t is None:
            if self.pos < len(self.tokens):
                cur = self.tokens[self.pos]
                raise ParseError(
                    f"{cur.source} expected '{kind}' for {use}, got '{cur.kind}'"
                )
            raise ParseError(
                f"expected '{kind}' for {use}, but reached end of file")
        return t

    def _match(self, kind: tok.Kind) -> Optional[tok.Token]:
        if self.pos >= len(self.tokens):
            return None
        t = self.tokens[self.pos]
        if t.kind != kind:
            return None
        self.pos += 1
        return t

    def _next(self) -> tok.Token:
        if self.pos >= len(self.tokens):
            raise ParseError("reached end of file")
        t = self.tokens[self.pos]
        self.pos += 1
        return t

    def _peek_is(self, offset: int, kind: tok.Kind) -> bool:
        t = self._peek(offset)
        return t is not None and t.kind == kind

    def _peek(self, offset: int) -> Optional[tok.Token]:
        idx = self.pos + offset
        if idx >= len(self.tokens):
            return None
        return self.tokens[idx]
