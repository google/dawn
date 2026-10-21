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
"""Unit tests for parser.py and def_ast.py."""

import unittest

import def_ast as ast
import parser


class ParserTest(unittest.TestCase):

    def test_parser(self):
        cases: list[tuple[str, ast.AST]] = [
            (
                "enum E {}",
                ast.AST(enums=[ast.EnumDecl(name="E")]),
            ),
            (
                "enum E { A @attr B C }",
                ast.AST(enums=[
                    ast.EnumDecl(
                        name="E",
                        entries=[
                            ast.EnumEntry(name="A"),
                            ast.EnumEntry(
                                attributes=ast.Attributes(
                                    [ast.Attribute(name="attr")]),
                                name="B",
                            ),
                            ast.EnumEntry(name="C"),
                        ],
                    )
                ]),
            ),
            (
                "type T",
                ast.AST(types=[ast.TypeDecl(name="T")]),
            ),
            (
                "type T<A, B, C>",
                ast.AST(types=[
                    ast.TypeDecl(
                        name="T",
                        template_params=[
                            ast.TemplateParam(name="A"),
                            ast.TemplateParam(name="B"),
                            ast.TemplateParam(name="C"),
                        ],
                    )
                ]),
            ),
            (
                "@attr type T",
                ast.AST(types=[
                    ast.TypeDecl(
                        attributes=ast.Attributes([ast.Attribute(
                            name="attr")]),
                        name="T",
                    )
                ]),
            ),
            (
                "@attr_a @attr_b type T",
                ast.AST(types=[
                    ast.TypeDecl(
                        attributes=ast.Attributes([
                            ast.Attribute(name="attr_a"),
                            ast.Attribute(name="attr_b"),
                        ]),
                        name="T",
                    )
                ]),
            ),
            (
                '@attr("a", "b") type T',
                ast.AST(types=[
                    ast.TypeDecl(
                        attributes=ast.Attributes(
                            [ast.Attribute(name="attr", values=["a", "b"])]),
                        name="T",
                    )
                ]),
            ),
            (
                '@attr(1, "x", 2.0) type T',
                ast.AST(types=[
                    ast.TypeDecl(
                        attributes=ast.Attributes([
                            ast.Attribute(name="attr", values=[1, "x", 2.0])
                        ]),
                        name="T",
                    )
                ]),
            ),
            (
                "match M : A",
                ast.AST(matchers=[
                    ast.MatcherDecl(
                        name="M",
                        options=ast.MatcherOptions(
                            types=[ast.TemplatedName(name="A")]),
                    )
                ]),
            ),
            (
                "match M : A | B",
                ast.AST(matchers=[
                    ast.MatcherDecl(
                        name="M",
                        options=ast.MatcherOptions(types=[
                            ast.TemplatedName(name="A"),
                            ast.TemplatedName(name="B"),
                        ]),
                    )
                ]),
            ),
            (
                "match M : A.B",
                ast.AST(matchers=[
                    ast.MatcherDecl(
                        name="M",
                        options=ast.MatcherOptions(
                            enumerants=[ast.MemberName(owner="A", member="B")
                                        ]),
                    )
                ]),
            ),
            (
                "match M : A.B | B.C",
                ast.AST(matchers=[
                    ast.MatcherDecl(
                        name="M",
                        options=ast.MatcherOptions(enumerants=[
                            ast.MemberName(owner="A", member="B"),
                            ast.MemberName(owner="B", member="C"),
                        ]),
                    )
                ]),
            ),
            (
                "fn F()",
                ast.AST(builtins=[
                    ast.IntrinsicDecl(
                        kind=ast.IntrinsicKind.Builtin,
                        name="F",
                        parameters=[],
                    )
                ]),
            ),
            (
                "@attr fn F()",
                ast.AST(builtins=[
                    ast.IntrinsicDecl(
                        kind=ast.IntrinsicKind.Builtin,
                        name="F",
                        attributes=ast.Attributes([ast.Attribute(
                            name="attr")]),
                        parameters=[],
                    )
                ]),
            ),
            (
                "fn F(a)",
                ast.AST(builtins=[
                    ast.IntrinsicDecl(
                        kind=ast.IntrinsicKind.Builtin,
                        name="F",
                        parameters=[
                            ast.Parameter(type=ast.TemplatedName(name="a"))
                        ],
                    )
                ]),
            ),
            (
                "fn F(a: T)",
                ast.AST(builtins=[
                    ast.IntrinsicDecl(
                        kind=ast.IntrinsicKind.Builtin,
                        name="F",
                        parameters=[
                            ast.Parameter(name="a",
                                          type=ast.TemplatedName(name="T"))
                        ],
                    )
                ]),
            ),
            (
                "fn F(a, b)",
                ast.AST(builtins=[
                    ast.IntrinsicDecl(
                        kind=ast.IntrinsicKind.Builtin,
                        name="F",
                        parameters=[
                            ast.Parameter(type=ast.TemplatedName(name="a")),
                            ast.Parameter(type=ast.TemplatedName(name="b")),
                        ],
                    )
                ]),
            ),
            (
                "fn F<A : B<C> >()",
                ast.AST(builtins=[
                    ast.IntrinsicDecl(
                        kind=ast.IntrinsicKind.Builtin,
                        name="F",
                        explicit_template_params=[
                            ast.TemplateParam(
                                name="A",
                                type=ast.TemplatedName(
                                    name="B",
                                    template_args=[
                                        ast.TemplatedName(name="C")
                                    ],
                                ),
                            )
                        ],
                        parameters=[],
                    )
                ]),
            ),
            (
                "fn F<T>(a: X, b: Y<T>)",
                ast.AST(builtins=[
                    ast.IntrinsicDecl(
                        kind=ast.IntrinsicKind.Builtin,
                        name="F",
                        explicit_template_params=[ast.TemplateParam(name="T")],
                        parameters=[
                            ast.Parameter(name="a",
                                          type=ast.TemplatedName(name="X")),
                            ast.Parameter(
                                name="b",
                                type=ast.TemplatedName(
                                    name="Y",
                                    template_args=[
                                        ast.TemplatedName(name="T")
                                    ],
                                ),
                            ),
                        ],
                    )
                ]),
            ),
            (
                "implicit(A : B<C>) fn F()",
                ast.AST(builtins=[
                    ast.IntrinsicDecl(
                        kind=ast.IntrinsicKind.Builtin,
                        name="F",
                        implicit_template_params=[
                            ast.TemplateParam(
                                name="A",
                                type=ast.TemplatedName(
                                    name="B",
                                    template_args=[
                                        ast.TemplatedName(name="C")
                                    ],
                                ),
                            )
                        ],
                        parameters=[],
                    )
                ]),
            ),
            (
                "implicit(T) fn F(a: X, b: Y<T>)",
                ast.AST(builtins=[
                    ast.IntrinsicDecl(
                        kind=ast.IntrinsicKind.Builtin,
                        name="F",
                        implicit_template_params=[ast.TemplateParam(name="T")],
                        parameters=[
                            ast.Parameter(name="a",
                                          type=ast.TemplatedName(name="X")),
                            ast.Parameter(
                                name="b",
                                type=ast.TemplatedName(
                                    name="Y",
                                    template_args=[
                                        ast.TemplatedName(name="T")
                                    ],
                                ),
                            ),
                        ],
                    )
                ]),
            ),
            (
                "implicit(B: C<D>) fn F<A>()",
                ast.AST(builtins=[
                    ast.IntrinsicDecl(
                        kind=ast.IntrinsicKind.Builtin,
                        name="F",
                        explicit_template_params=[ast.TemplateParam(name="A")],
                        implicit_template_params=[
                            ast.TemplateParam(
                                name="B",
                                type=ast.TemplatedName(
                                    name="C",
                                    template_args=[
                                        ast.TemplatedName(name="D")
                                    ],
                                ),
                            )
                        ],
                        parameters=[],
                    )
                ]),
            ),
            (
                "fn F() -> X",
                ast.AST(builtins=[
                    ast.IntrinsicDecl(
                        kind=ast.IntrinsicKind.Builtin,
                        name="F",
                        return_type=ast.TemplatedName(name="X"),
                        parameters=[],
                    )
                ]),
            ),
            (
                "fn F() -> X<T>",
                ast.AST(builtins=[
                    ast.IntrinsicDecl(
                        kind=ast.IntrinsicKind.Builtin,
                        name="F",
                        return_type=ast.TemplatedName(
                            name="X",
                            template_args=[ast.TemplatedName(name="T")],
                        ),
                        parameters=[],
                    )
                ]),
            ),
            (
                "op F()",
                ast.AST(operators=[
                    ast.IntrinsicDecl(
                        kind=ast.IntrinsicKind.Operator,
                        name="F",
                        parameters=[],
                    )
                ]),
            ),
            (
                "@attr op F()",
                ast.AST(operators=[
                    ast.IntrinsicDecl(
                        kind=ast.IntrinsicKind.Operator,
                        name="F",
                        attributes=ast.Attributes([ast.Attribute(
                            name="attr")]),
                        parameters=[],
                    )
                ]),
            ),
            (
                "op F(a)",
                ast.AST(operators=[
                    ast.IntrinsicDecl(
                        kind=ast.IntrinsicKind.Operator,
                        name="F",
                        parameters=[
                            ast.Parameter(type=ast.TemplatedName(name="a"))
                        ],
                    )
                ]),
            ),
            (
                "op F(@blah a)",
                ast.AST(operators=[
                    ast.IntrinsicDecl(
                        kind=ast.IntrinsicKind.Operator,
                        name="F",
                        parameters=[
                            ast.Parameter(
                                attributes=ast.Attributes(
                                    [ast.Attribute(name="blah")]),
                                type=ast.TemplatedName(name="a"),
                            )
                        ],
                    )
                ]),
            ),
            (
                "op F(a: T)",
                ast.AST(operators=[
                    ast.IntrinsicDecl(
                        kind=ast.IntrinsicKind.Operator,
                        name="F",
                        parameters=[
                            ast.Parameter(name="a",
                                          type=ast.TemplatedName(name="T"))
                        ],
                    )
                ]),
            ),
            (
                "op F(a, b)",
                ast.AST(operators=[
                    ast.IntrinsicDecl(
                        kind=ast.IntrinsicKind.Operator,
                        name="F",
                        parameters=[
                            ast.Parameter(type=ast.TemplatedName(name="a")),
                            ast.Parameter(type=ast.TemplatedName(name="b")),
                        ],
                    )
                ]),
            ),
            (
                "op F<A : B<C> >()",
                ast.AST(operators=[
                    ast.IntrinsicDecl(
                        kind=ast.IntrinsicKind.Operator,
                        name="F",
                        explicit_template_params=[
                            ast.TemplateParam(
                                name="A",
                                type=ast.TemplatedName(
                                    name="B",
                                    template_args=[
                                        ast.TemplatedName(name="C")
                                    ],
                                ),
                            )
                        ],
                        parameters=[],
                    )
                ]),
            ),
            (
                "op F<T>(a: X, b: Y<T>)",
                ast.AST(operators=[
                    ast.IntrinsicDecl(
                        kind=ast.IntrinsicKind.Operator,
                        name="F",
                        explicit_template_params=[ast.TemplateParam(name="T")],
                        parameters=[
                            ast.Parameter(name="a",
                                          type=ast.TemplatedName(name="X")),
                            ast.Parameter(
                                name="b",
                                type=ast.TemplatedName(
                                    name="Y",
                                    template_args=[
                                        ast.TemplatedName(name="T")
                                    ],
                                ),
                            ),
                        ],
                    )
                ]),
            ),
            (
                "implicit(A : B<C>) op F()",
                ast.AST(operators=[
                    ast.IntrinsicDecl(
                        kind=ast.IntrinsicKind.Operator,
                        name="F",
                        implicit_template_params=[
                            ast.TemplateParam(
                                name="A",
                                type=ast.TemplatedName(
                                    name="B",
                                    template_args=[
                                        ast.TemplatedName(name="C")
                                    ],
                                ),
                            )
                        ],
                        parameters=[],
                    )
                ]),
            ),
            (
                "implicit(T) op F(a: X, b: Y<T>)",
                ast.AST(operators=[
                    ast.IntrinsicDecl(
                        kind=ast.IntrinsicKind.Operator,
                        name="F",
                        implicit_template_params=[ast.TemplateParam(name="T")],
                        parameters=[
                            ast.Parameter(name="a",
                                          type=ast.TemplatedName(name="X")),
                            ast.Parameter(
                                name="b",
                                type=ast.TemplatedName(
                                    name="Y",
                                    template_args=[
                                        ast.TemplatedName(name="T")
                                    ],
                                ),
                            ),
                        ],
                    )
                ]),
            ),
            (
                "implicit(D) op F<A : B<C> >()",
                ast.AST(operators=[
                    ast.IntrinsicDecl(
                        kind=ast.IntrinsicKind.Operator,
                        name="F",
                        explicit_template_params=[
                            ast.TemplateParam(
                                name="A",
                                type=ast.TemplatedName(
                                    name="B",
                                    template_args=[
                                        ast.TemplatedName(name="C")
                                    ],
                                ),
                            )
                        ],
                        implicit_template_params=[ast.TemplateParam(name="D")],
                        parameters=[],
                    )
                ]),
            ),
            (
                "op F() -> X",
                ast.AST(operators=[
                    ast.IntrinsicDecl(
                        kind=ast.IntrinsicKind.Operator,
                        name="F",
                        return_type=ast.TemplatedName(name="X"),
                        parameters=[],
                    )
                ]),
            ),
            (
                "op F() -> X<T>",
                ast.AST(operators=[
                    ast.IntrinsicDecl(
                        kind=ast.IntrinsicKind.Operator,
                        name="F",
                        return_type=ast.TemplatedName(
                            name="X",
                            template_args=[ast.TemplatedName(name="T")],
                        ),
                        parameters=[],
                    )
                ]),
            ),
            (
                "ctor F()",
                ast.AST(constructors=[
                    ast.IntrinsicDecl(
                        kind=ast.IntrinsicKind.Constructor,
                        name="F",
                        parameters=[],
                    )
                ]),
            ),
            (
                "@attr ctor F()",
                ast.AST(constructors=[
                    ast.IntrinsicDecl(
                        kind=ast.IntrinsicKind.Constructor,
                        name="F",
                        attributes=ast.Attributes([ast.Attribute(
                            name="attr")]),
                        parameters=[],
                    )
                ]),
            ),
            (
                "ctor F(a)",
                ast.AST(constructors=[
                    ast.IntrinsicDecl(
                        kind=ast.IntrinsicKind.Constructor,
                        name="F",
                        parameters=[
                            ast.Parameter(type=ast.TemplatedName(name="a"))
                        ],
                    )
                ]),
            ),
            (
                "ctor F(a: T)",
                ast.AST(constructors=[
                    ast.IntrinsicDecl(
                        kind=ast.IntrinsicKind.Constructor,
                        name="F",
                        parameters=[
                            ast.Parameter(name="a",
                                          type=ast.TemplatedName(name="T"))
                        ],
                    )
                ]),
            ),
            (
                "ctor F(a, b)",
                ast.AST(constructors=[
                    ast.IntrinsicDecl(
                        kind=ast.IntrinsicKind.Constructor,
                        name="F",
                        parameters=[
                            ast.Parameter(type=ast.TemplatedName(name="a")),
                            ast.Parameter(type=ast.TemplatedName(name="b")),
                        ],
                    )
                ]),
            ),
            (
                "ctor F<A : B<C> >()",
                ast.AST(constructors=[
                    ast.IntrinsicDecl(
                        kind=ast.IntrinsicKind.Constructor,
                        name="F",
                        explicit_template_params=[
                            ast.TemplateParam(
                                name="A",
                                type=ast.TemplatedName(
                                    name="B",
                                    template_args=[
                                        ast.TemplatedName(name="C")
                                    ],
                                ),
                            )
                        ],
                        parameters=[],
                    )
                ]),
            ),
            (
                "ctor F<T>(a: X, b: Y<T>)",
                ast.AST(constructors=[
                    ast.IntrinsicDecl(
                        kind=ast.IntrinsicKind.Constructor,
                        name="F",
                        explicit_template_params=[ast.TemplateParam(name="T")],
                        parameters=[
                            ast.Parameter(name="a",
                                          type=ast.TemplatedName(name="X")),
                            ast.Parameter(
                                name="b",
                                type=ast.TemplatedName(
                                    name="Y",
                                    template_args=[
                                        ast.TemplatedName(name="T")
                                    ],
                                ),
                            ),
                        ],
                    )
                ]),
            ),
            (
                "ctor F() -> X",
                ast.AST(constructors=[
                    ast.IntrinsicDecl(
                        kind=ast.IntrinsicKind.Constructor,
                        name="F",
                        return_type=ast.TemplatedName(name="X"),
                        parameters=[],
                    )
                ]),
            ),
            (
                "ctor F() -> X<T>",
                ast.AST(constructors=[
                    ast.IntrinsicDecl(
                        kind=ast.IntrinsicKind.Constructor,
                        name="F",
                        return_type=ast.TemplatedName(
                            name="X",
                            template_args=[ast.TemplatedName(name="T")],
                        ),
                        parameters=[],
                    )
                ]),
            ),
            (
                "conv F()",
                ast.AST(converters=[
                    ast.IntrinsicDecl(
                        kind=ast.IntrinsicKind.Converter,
                        name="F",
                        parameters=[],
                    )
                ]),
            ),
            (
                "@attr conv F()",
                ast.AST(converters=[
                    ast.IntrinsicDecl(
                        kind=ast.IntrinsicKind.Converter,
                        name="F",
                        attributes=ast.Attributes([ast.Attribute(
                            name="attr")]),
                        parameters=[],
                    )
                ]),
            ),
            (
                "conv F(a)",
                ast.AST(converters=[
                    ast.IntrinsicDecl(
                        kind=ast.IntrinsicKind.Converter,
                        name="F",
                        parameters=[
                            ast.Parameter(type=ast.TemplatedName(name="a"))
                        ],
                    )
                ]),
            ),
            (
                "conv F(a: T)",
                ast.AST(converters=[
                    ast.IntrinsicDecl(
                        kind=ast.IntrinsicKind.Converter,
                        name="F",
                        parameters=[
                            ast.Parameter(name="a",
                                          type=ast.TemplatedName(name="T"))
                        ],
                    )
                ]),
            ),
            (
                "conv F(a, b)",
                ast.AST(converters=[
                    ast.IntrinsicDecl(
                        kind=ast.IntrinsicKind.Converter,
                        name="F",
                        parameters=[
                            ast.Parameter(type=ast.TemplatedName(name="a")),
                            ast.Parameter(type=ast.TemplatedName(name="b")),
                        ],
                    )
                ]),
            ),
            (
                "conv F<A : B<C> >()",
                ast.AST(converters=[
                    ast.IntrinsicDecl(
                        kind=ast.IntrinsicKind.Converter,
                        name="F",
                        explicit_template_params=[
                            ast.TemplateParam(
                                name="A",
                                type=ast.TemplatedName(
                                    name="B",
                                    template_args=[
                                        ast.TemplatedName(name="C")
                                    ],
                                ),
                            )
                        ],
                        parameters=[],
                    )
                ]),
            ),
            (
                "conv F<T>(a: X, b: Y<T>)",
                ast.AST(converters=[
                    ast.IntrinsicDecl(
                        kind=ast.IntrinsicKind.Converter,
                        name="F",
                        explicit_template_params=[ast.TemplateParam(name="T")],
                        parameters=[
                            ast.Parameter(name="a",
                                          type=ast.TemplatedName(name="X")),
                            ast.Parameter(
                                name="b",
                                type=ast.TemplatedName(
                                    name="Y",
                                    template_args=[
                                        ast.TemplatedName(name="T")
                                    ],
                                ),
                            ),
                        ],
                    )
                ]),
            ),
            (
                "conv F() -> X",
                ast.AST(converters=[
                    ast.IntrinsicDecl(
                        kind=ast.IntrinsicKind.Converter,
                        name="F",
                        return_type=ast.TemplatedName(name="X"),
                        parameters=[],
                    )
                ]),
            ),
            (
                "conv F() -> X<T>",
                ast.AST(converters=[
                    ast.IntrinsicDecl(
                        kind=ast.IntrinsicKind.Converter,
                        name="F",
                        return_type=ast.TemplatedName(
                            name="X",
                            template_args=[ast.TemplatedName(name="T")],
                        ),
                        parameters=[],
                    )
                ]),
            ),
        ]

        for src, expect in cases:
            with self.subTest(src=src):
                got = parser.parse(src, "test.txt", {})
                self.assertEqual(got, expect)

    def test_parser_error(self):
        cases = [
            ("£", "test.txt:1:1: unexpected '£'"),
            ("123", "test.txt:1:1 unexpected token 'integer'"),
            (
                "@123",
                "test.txt:1:2 expected 'ident' for attribute name, got 'integer'",
            ),
        ]
        for src, expect in cases:
            with self.subTest(src=src):
                with self.assertRaises(parser.ParseError) as ctx:
                    parser.parse(src, "test.txt", {})
                self.assertEqual(str(ctx.exception), expect)

    def test_parser_with_imports(self):
        cases: list[tuple[str, dict[str, str], ast.AST]] = [
            (
                'import "foo"',
                {
                    "foo": "type T"
                },
                ast.AST(types=[ast.TypeDecl(name="T")]),
            ),
            (
                'import "foo"',
                {
                    "foo": 'import "bar"',
                    "bar": "type T",
                },
                ast.AST(types=[ast.TypeDecl(name="T")]),
            ),
        ]
        for src, files, expect in cases:
            with self.subTest(src=src):
                got = parser.parse(src, "root.txt", files)
                self.assertEqual(got, expect)

    def test_parser_import_errors(self):
        cases = [
            (
                'import "missing"',
                {},
                "root.txt:1:1 failed to load 'missing': open missing: file does not exist",
            ),
            (
                'import "broken"',
                {
                    "broken": "£"
                },
                "broken:1:1: unexpected '£'",
            ),
        ]
        for src, files, expect in cases:
            with self.subTest(src=src):
                with self.assertRaises(parser.ParseError) as ctx:
                    parser.parse(src, "root.txt", files)
                self.assertEqual(str(ctx.exception), expect)

    def test_ast_formatting_and_attributes_take(self):
        parsed = parser.parse(
            """
enum E { A @attr(1, "x") B }
@attr type T<X: Y>
match M1 : A | B<C>
match M2 : E.A | E.B
implicit(I: J) fn F<U>(@p a: T<U>) -> R
op +(a: T, b: T) -> T
ctor T(a: U) -> T
conv T(a: U) -> T
""",
            "test.def",
            {},
        )
        formatted = str(parsed)
        self.assertIn("enum E {", formatted)
        self.assertIn("match M1: A | B<C>", formatted)
        self.assertIn("match M2: E.A | E.B", formatted)
        self.assertIn("fn F<U>[I : J](@p a: T<U>) -> R", formatted)

        attrs = ast.Attributes(
            [ast.Attribute(name="foo"),
             ast.Attribute(name="bar", values=[1])])
        self.assertIsNone(attrs.take("baz"))
        taken = attrs.take("foo")
        self.assertIsNotNone(taken)
        self.assertEqual(taken.name, "foo")
        self.assertEqual(len(attrs), 1)

    def test_parse_real_def_files(self):
        for rel_path in (
                "src/tint/lang/core/core.def",
                "src/tint/lang/wgsl/wgsl.def",
                "src/tint/lang/glsl/glsl.def",
                "src/tint/lang/hlsl/hlsl.def",
                "src/tint/lang/msl/msl.def",
                "src/tint/lang/spirv/spirv.def",
        ):
            with self.subTest(rel_path=rel_path):
                full_path = parser.DAWN_ROOT / rel_path
                source = full_path.read_text(encoding="utf-8")
                parsed = parser.parse(source, rel_path)
                self.assertGreater(len(parsed.types), 0)
                self.assertGreater(len(parsed.builtins), 0)


if __name__ == "__main__":
    unittest.main()
