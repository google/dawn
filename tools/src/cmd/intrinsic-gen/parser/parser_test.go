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

package parser_test

import (
	"testing"

	"dawn.googlesource.com/dawn/tools/src/cmd/intrinsic-gen/ast"
	"dawn.googlesource.com/dawn/tools/src/cmd/intrinsic-gen/parser"
)

func TestParser(t *testing.T) {
	type test struct {
		src    string
		expect ast.AST
	}

	for _, test := range []test{
		{"enum E {}", ast.AST{
			Enums: []ast.EnumDecl{{Name: "E"}},
		}},
		{"enum E { A [[deco]] B C }", ast.AST{
			Enums: []ast.EnumDecl{{
				Name: "E",
				Entries: []ast.EnumEntry{
					{Name: "A"},
					{
						Decorations: ast.Decorations{{Name: "deco"}},
						Name:        "B",
					},
					{Name: "C"},
				},
			}},
		}},
		{"type T", ast.AST{
			Types: []ast.TypeDecl{{Name: "T"}},
		}},
		{"type T<A, B, C>", ast.AST{
			Types: []ast.TypeDecl{{
				Name: "T",
				TemplateParams: ast.TemplateParams{
					{Name: "A"},
					{Name: "B"},
					{Name: "C"},
				},
			}},
		}},
		{"[[deco]] type T", ast.AST{
			Types: []ast.TypeDecl{{
				Decorations: ast.Decorations{
					{Name: "deco"},
				},
				Name: "T",
			}},
		}},
		{`[[deco("a", "b")]] type T`, ast.AST{
			Types: []ast.TypeDecl{{
				Decorations: ast.Decorations{
					{Name: "deco", Values: []string{"a", "b"}},
				},
				Name: "T",
			}},
		}},
		{"match M : A", ast.AST{
			Matchers: []ast.MatcherDecl{{
				Name: "M",
				Options: ast.MatcherOptions{
					ast.TemplatedName{Name: "A"},
				},
			}},
		}},
		{"match M : A | B", ast.AST{
			Matchers: []ast.MatcherDecl{{
				Name: "M",
				Options: ast.MatcherOptions{
					ast.TemplatedName{Name: "A"},
					ast.TemplatedName{Name: "B"},
				},
			}},
		}},
		{"fn F()", ast.AST{
			Functions: []ast.FunctionDecl{{
				Name: "F",
			}},
		}},
		{"[[deco]] fn F()", ast.AST{
			Functions: []ast.FunctionDecl{{
				Name: "F",
				Decorations: ast.Decorations{
					{Name: "deco"},
				},
			}},
		}},
		{"fn F(a)", ast.AST{
			Functions: []ast.FunctionDecl{{
				Name: "F",
				Parameters: ast.Parameters{
					{Type: ast.TemplatedName{Name: "a"}},
				},
			}},
		}},
		{"fn F(a: T)", ast.AST{
			Functions: []ast.FunctionDecl{{
				Name: "F",
				Parameters: ast.Parameters{
					{Name: "a", Type: ast.TemplatedName{Name: "T"}},
				},
			}},
		}},
		{"fn F(a, b)", ast.AST{
			Functions: []ast.FunctionDecl{{
				Name: "F",
				Parameters: ast.Parameters{
					{Type: ast.TemplatedName{Name: "a"}},
					{Type: ast.TemplatedName{Name: "b"}},
				},
			}},
		}},
		{"fn F<A : B<C>>()", ast.AST{
			Functions: []ast.FunctionDecl{{
				Name: "F",
				TemplateParams: ast.TemplateParams{
					{
						Name: "A", Type: ast.TemplatedName{
							Name: "B",
							TemplateArgs: ast.TemplatedNames{
								{Name: "C"},
							},
						},
					},
				},
			}},
		}},
		{"fn F<T>(a: X, b: Y<T>)", ast.AST{
			Functions: []ast.FunctionDecl{{
				Name: "F",
				TemplateParams: ast.TemplateParams{
					{Name: "T"},
				},
				Parameters: ast.Parameters{
					{Name: "a", Type: ast.TemplatedName{Name: "X"}},
					{Name: "b", Type: ast.TemplatedName{
						Name:         "Y",
						TemplateArgs: []ast.TemplatedName{{Name: "T"}},
					}},
				},
			}},
		}},
		{"fn F() -> X", ast.AST{
			Functions: []ast.FunctionDecl{{
				Name:       "F",
				ReturnType: &ast.TemplatedName{Name: "X"},
			}},
		}},
		{"fn F() -> X<T>", ast.AST{
			Functions: []ast.FunctionDecl{{
				Name: "F",
				ReturnType: &ast.TemplatedName{
					Name:         "X",
					TemplateArgs: []ast.TemplatedName{{Name: "T"}},
				},
			}},
		}},
	} {
		got, err := parser.Parse(test.src, "file.txt")
		if err != nil {
			t.Errorf("While parsing:\n%s\nParse() returned error: %v", test.src, err)
			continue
		}

		gotStr, expectStr := got.String(), test.expect.String()
		if gotStr != expectStr {
			t.Errorf("While parsing:\n%s\nGot:\n%s\nExpected:\n%s", test.src, gotStr, expectStr)
		}
	}
}

func TestErrors(t *testing.T) {
	type test struct {
		src    string
		expect string
	}

	for _, test := range []test{
		{"+", "test.txt:1:1: unexpected '+'"},
		{"123", "test.txt:1:1 unexpected token 'integer'"},
		{"[[123]]", "test.txt:1:3 expected 'ident' for decoration name, got 'integer'"},
		{"[[abc", "expected ']]' for decoration list, but reached end of file"},
	} {
		got, err := parser.Parse(test.src, "test.txt")
		if gotErr := err.Error(); test.expect != gotErr {
			t.Errorf(`Parse() returned error "%+v", expected error "%+v"`, gotErr, test.expect)
		}
		if got != nil {
			t.Errorf("Lex() returned non-nil for error")
		}
	}
}
