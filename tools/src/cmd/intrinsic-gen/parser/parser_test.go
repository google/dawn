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
	"dawn.googlesource.com/dawn/tools/src/utils"
	"github.com/google/go-cmp/cmp"
)

var ignoreSource = cmp.FilterPath(func(p cmp.Path) bool {
	return p.Last().String() == ".Source"
}, cmp.Ignore())

func TestParser(t *testing.T) {
	type test struct {
		location string
		src      string
		expect   ast.AST
	}

	for _, test := range []test{
		{
			utils.ThisLine(),
			"enum E {}",
			ast.AST{
				Enums: []ast.EnumDecl{{Name: "E"}},
			},
		}, { ///////////////////////////////////////////////////////////////////
			utils.ThisLine(),
			"enum E { A [[deco]] B C }",
			ast.AST{
				Enums: []ast.EnumDecl{{
					Name: "E",
					Entries: []ast.EnumEntry{
						{Name: "A"},
						{
							Decorations: ast.Decorations{{
								Name:   "deco",
								Values: []string{},
							}},
							Name: "B",
						},
						{Name: "C"},
					},
				}},
			},
		}, { ///////////////////////////////////////////////////////////////////
			utils.ThisLine(),
			"type T",
			ast.AST{
				Types: []ast.TypeDecl{{Name: "T"}},
			},
		}, { ///////////////////////////////////////////////////////////////////
			utils.ThisLine(),
			"type T<A, B, C>",
			ast.AST{
				Types: []ast.TypeDecl{{
					Name: "T",
					TemplateParams: ast.TemplateParams{
						{Name: "A"},
						{Name: "B"},
						{Name: "C"},
					},
				}},
			},
		}, { ///////////////////////////////////////////////////////////////////
			utils.ThisLine(),
			"[[deco]] type T",
			ast.AST{
				Types: []ast.TypeDecl{{
					Decorations: ast.Decorations{
						{Name: "deco", Values: []string{}},
					},
					Name: "T",
				}},
			},
		}, { ///////////////////////////////////////////////////////////////////
			utils.ThisLine(),
			`[[deco("a", "b")]] type T`, ast.AST{
				Types: []ast.TypeDecl{{
					Decorations: ast.Decorations{
						{Name: "deco", Values: []string{"a", "b"}},
					},
					Name: "T",
				}},
			},
		}, { ///////////////////////////////////////////////////////////////////
			utils.ThisLine(),
			"match M : A",
			ast.AST{
				Matchers: []ast.MatcherDecl{{
					Name: "M",
					Options: ast.MatcherOptions{
						ast.TemplatedName{Name: "A"},
					},
				}},
			},
		}, { ///////////////////////////////////////////////////////////////////
			utils.ThisLine(),
			"match M : A | B",
			ast.AST{
				Matchers: []ast.MatcherDecl{{
					Name: "M",
					Options: ast.MatcherOptions{
						ast.TemplatedName{Name: "A"},
						ast.TemplatedName{Name: "B"},
					},
				}},
			},
		}, { ///////////////////////////////////////////////////////////////////
			utils.ThisLine(),
			"fn F()",
			ast.AST{
				Builtins: []ast.IntrinsicDecl{{
					Kind:       ast.Builtin,
					Name:       "F",
					Parameters: ast.Parameters{},
				}},
			},
		}, { ///////////////////////////////////////////////////////////////////
			utils.ThisLine(),
			"[[deco]] fn F()",
			ast.AST{
				Builtins: []ast.IntrinsicDecl{{
					Kind: ast.Builtin,
					Name: "F",
					Decorations: ast.Decorations{
						{Name: "deco", Values: []string{}},
					},
					Parameters: ast.Parameters{},
				}},
			},
		}, { ///////////////////////////////////////////////////////////////////
			utils.ThisLine(),
			"fn F(a)",
			ast.AST{
				Builtins: []ast.IntrinsicDecl{{
					Kind: ast.Builtin,
					Name: "F",
					Parameters: ast.Parameters{
						{Type: ast.TemplatedName{Name: "a"}},
					},
				}},
			},
		}, { ///////////////////////////////////////////////////////////////////
			utils.ThisLine(),
			"fn F(a: T)",
			ast.AST{
				Builtins: []ast.IntrinsicDecl{{
					Kind: ast.Builtin,
					Name: "F",
					Parameters: ast.Parameters{
						{Name: "a", Type: ast.TemplatedName{Name: "T"}},
					},
				}},
			},
		}, { ///////////////////////////////////////////////////////////////////
			utils.ThisLine(),
			"fn F(a, b)",
			ast.AST{
				Builtins: []ast.IntrinsicDecl{{
					Kind: ast.Builtin,
					Name: "F",
					Parameters: ast.Parameters{
						{Type: ast.TemplatedName{Name: "a"}},
						{Type: ast.TemplatedName{Name: "b"}},
					},
				}},
			},
		}, { ///////////////////////////////////////////////////////////////////
			utils.ThisLine(),
			"fn F<A : B<C> >()",
			ast.AST{
				Builtins: []ast.IntrinsicDecl{{
					Kind: ast.Builtin,
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
					Parameters: ast.Parameters{},
				}},
			},
		}, { ///////////////////////////////////////////////////////////////////
			utils.ThisLine(),
			"fn F<T>(a: X, b: Y<T>)",
			ast.AST{
				Builtins: []ast.IntrinsicDecl{{
					Kind: ast.Builtin,
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
			},
		}, { ///////////////////////////////////////////////////////////////////
			utils.ThisLine(),
			"fn F() -> X",
			ast.AST{
				Builtins: []ast.IntrinsicDecl{{
					Kind:       ast.Builtin,
					Name:       "F",
					ReturnType: &ast.TemplatedName{Name: "X"},
					Parameters: ast.Parameters{},
				}},
			},
		}, { ///////////////////////////////////////////////////////////////////
			utils.ThisLine(),
			"fn F() -> X<T>",
			ast.AST{
				Builtins: []ast.IntrinsicDecl{{
					Kind: ast.Builtin,
					Name: "F",
					ReturnType: &ast.TemplatedName{
						Name:         "X",
						TemplateArgs: []ast.TemplatedName{{Name: "T"}},
					},
					Parameters: ast.Parameters{},
				}},
			},
		}, { ///////////////////////////////////////////////////////////////////
			utils.ThisLine(),
			"op F()",
			ast.AST{
				Operators: []ast.IntrinsicDecl{{
					Kind:       ast.Operator,
					Name:       "F",
					Parameters: ast.Parameters{},
				}},
			},
		}, { ///////////////////////////////////////////////////////////////////
			utils.ThisLine(),
			"[[deco]] op F()",
			ast.AST{
				Operators: []ast.IntrinsicDecl{{
					Kind: ast.Operator,
					Name: "F",
					Decorations: ast.Decorations{
						{Name: "deco", Values: []string{}},
					},
					Parameters: ast.Parameters{},
				}},
			},
		}, { ///////////////////////////////////////////////////////////////////
			utils.ThisLine(),
			"op F(a)",
			ast.AST{
				Operators: []ast.IntrinsicDecl{{
					Kind: ast.Operator,
					Name: "F",
					Parameters: ast.Parameters{
						{Type: ast.TemplatedName{Name: "a"}},
					},
				}},
			},
		}, { ///////////////////////////////////////////////////////////////////
			utils.ThisLine(),
			"op F(a: T)",
			ast.AST{
				Operators: []ast.IntrinsicDecl{{
					Kind: ast.Operator,
					Name: "F",
					Parameters: ast.Parameters{
						{Name: "a", Type: ast.TemplatedName{Name: "T"}},
					},
				}},
			},
		}, { ///////////////////////////////////////////////////////////////////
			utils.ThisLine(),
			"op F(a, b)",
			ast.AST{
				Operators: []ast.IntrinsicDecl{{
					Kind: ast.Operator,
					Name: "F",
					Parameters: ast.Parameters{
						{Type: ast.TemplatedName{Name: "a"}},
						{Type: ast.TemplatedName{Name: "b"}},
					},
				}},
			},
		}, { ///////////////////////////////////////////////////////////////////
			utils.ThisLine(),
			"op F<A : B<C> >()",
			ast.AST{
				Operators: []ast.IntrinsicDecl{{
					Kind: ast.Operator,
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
					Parameters: ast.Parameters{},
				}},
			},
		}, { ///////////////////////////////////////////////////////////////////
			utils.ThisLine(),
			"op F<T>(a: X, b: Y<T>)",
			ast.AST{
				Operators: []ast.IntrinsicDecl{{
					Kind: ast.Operator,
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
			},
		}, { ///////////////////////////////////////////////////////////////////
			utils.ThisLine(),
			"op F() -> X",
			ast.AST{
				Operators: []ast.IntrinsicDecl{{
					Kind:       ast.Operator,
					Name:       "F",
					ReturnType: &ast.TemplatedName{Name: "X"},
					Parameters: ast.Parameters{},
				}},
			},
		}, { ///////////////////////////////////////////////////////////////////
			utils.ThisLine(),
			"op F() -> X<T>",
			ast.AST{
				Operators: []ast.IntrinsicDecl{{
					Kind: ast.Operator,
					Name: "F",
					ReturnType: &ast.TemplatedName{
						Name:         "X",
						TemplateArgs: []ast.TemplatedName{{Name: "T"}},
					},
					Parameters: ast.Parameters{},
				}},
			}},
	} {
		got, err := parser.Parse(test.src, "file.txt")
		if err != nil {
			t.Errorf("\n%v\nWhile parsing:\n%s\nParse() returned error: %v",
				test.location, test.src, err)
			continue
		}

		if diff := cmp.Diff(got, &test.expect, ignoreSource); diff != "" {
			t.Errorf("\n%v\nWhile parsing:\n%s\n\n%s",
				test.location, test.src, diff)
		}
	}
}

func TestErrors(t *testing.T) {
	type test struct {
		src    string
		expect string
	}

	for _, test := range []test{
		{
			"£",
			"test.txt:1:1: unexpected '£'",
		},
		{
			"123",
			"test.txt:1:1 unexpected token 'integer'",
		},
		{
			"[[123]]",
			"test.txt:1:3 expected 'ident' for decoration name, got 'integer'",
		},
		{
			"[[abc",
			"expected ']]' for decoration list, but reached end of file",
		},
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
