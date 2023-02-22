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

package lexer_test

import (
	"fmt"
	"testing"

	"dawn.googlesource.com/dawn/tools/src/tint/intrinsic/lexer"
	"dawn.googlesource.com/dawn/tools/src/tint/intrinsic/tok"
	"github.com/google/go-cmp/cmp"
)

func TestLexTokens(t *testing.T) {
	type test struct {
		src    string
		expect []tok.Token
	}

	filepath := "test.txt"
	loc := func(l, c, r int) tok.Location {
		return tok.Location{Line: l, Column: c, Rune: r, Filepath: filepath}
	}

	for _, test := range []test{
		{"ident", []tok.Token{{Kind: tok.Identifier, Runes: []rune("ident"), Source: tok.Source{
			S: loc(1, 1, 0), E: loc(1, 6, 5),
		}}}},
		{"ident_123", []tok.Token{{Kind: tok.Identifier, Runes: []rune("ident_123"), Source: tok.Source{
			S: loc(1, 1, 0), E: loc(1, 10, 9),
		}}}},
		{"_ident_", []tok.Token{{Kind: tok.Identifier, Runes: []rune("_ident_"), Source: tok.Source{
			S: loc(1, 1, 0), E: loc(1, 8, 7),
		}}}},
		{"123456789", []tok.Token{{Kind: tok.Integer, Runes: []rune("123456789"), Source: tok.Source{
			S: loc(1, 1, 0), E: loc(1, 10, 9),
		}}}},
		{"-123456789", []tok.Token{{Kind: tok.Integer, Runes: []rune("-123456789"), Source: tok.Source{
			S: loc(1, 1, 0), E: loc(1, 11, 10),
		}}}},
		{"1234.56789", []tok.Token{{Kind: tok.Float, Runes: []rune("1234.56789"), Source: tok.Source{
			S: loc(1, 1, 0), E: loc(1, 11, 10),
		}}}},
		{"-1234.56789", []tok.Token{{Kind: tok.Float, Runes: []rune("-1234.56789"), Source: tok.Source{
			S: loc(1, 1, 0), E: loc(1, 12, 11),
		}}}},
		{"123.456.789", []tok.Token{
			{Kind: tok.Float, Runes: []rune("123.456"), Source: tok.Source{
				S: loc(1, 1, 0), E: loc(1, 8, 7),
			}},
			{Kind: tok.Dot, Runes: []rune("."), Source: tok.Source{
				S: loc(1, 8, 7), E: loc(1, 9, 8),
			}},
			{Kind: tok.Integer, Runes: []rune("789"), Source: tok.Source{
				S: loc(1, 9, 8), E: loc(1, 12, 11),
			}},
		}},
		{"-123.456-789", []tok.Token{
			{Kind: tok.Float, Runes: []rune("-123.456"), Source: tok.Source{
				S: loc(1, 1, 0), E: loc(1, 9, 8),
			}},
			{Kind: tok.Integer, Runes: []rune("-789"), Source: tok.Source{
				S: loc(1, 9, 8), E: loc(1, 13, 12),
			}},
		}},
		{"match", []tok.Token{{Kind: tok.Match, Runes: []rune("match"), Source: tok.Source{
			S: loc(1, 1, 0), E: loc(1, 6, 5),
		}}}},
		{"fn", []tok.Token{{Kind: tok.Function, Runes: []rune("fn"), Source: tok.Source{
			S: loc(1, 1, 0), E: loc(1, 3, 2),
		}}}},
		{"op", []tok.Token{{Kind: tok.Operator, Runes: []rune("op"), Source: tok.Source{
			S: loc(1, 1, 0), E: loc(1, 3, 2),
		}}}},
		{"operation", []tok.Token{{Kind: tok.Identifier, Runes: []rune("operation"), Source: tok.Source{
			S: loc(1, 1, 0), E: loc(1, 10, 9),
		}}}},
		{"type", []tok.Token{{Kind: tok.Type, Runes: []rune("type"), Source: tok.Source{
			S: loc(1, 1, 0), E: loc(1, 5, 4),
		}}}},
		{"ctor", []tok.Token{{Kind: tok.Constructor, Runes: []rune("ctor"), Source: tok.Source{
			S: loc(1, 1, 0), E: loc(1, 5, 4),
		}}}},
		{"conv", []tok.Token{{Kind: tok.Converter, Runes: []rune("conv"), Source: tok.Source{
			S: loc(1, 1, 0), E: loc(1, 5, 4),
		}}}},
		{"enum", []tok.Token{{Kind: tok.Enum, Runes: []rune("enum"), Source: tok.Source{
			S: loc(1, 1, 0), E: loc(1, 5, 4),
		}}}},
		{":", []tok.Token{{Kind: tok.Colon, Runes: []rune(":"), Source: tok.Source{
			S: loc(1, 1, 0), E: loc(1, 2, 1),
		}}}},
		{",", []tok.Token{{Kind: tok.Comma, Runes: []rune(","), Source: tok.Source{
			S: loc(1, 1, 0), E: loc(1, 2, 1),
		}}}},
		{"-", []tok.Token{{Kind: tok.Minus, Runes: []rune("-"), Source: tok.Source{
			S: loc(1, 1, 0), E: loc(1, 2, 1),
		}}}},
		{"<", []tok.Token{{Kind: tok.Lt, Runes: []rune("<"), Source: tok.Source{
			S: loc(1, 1, 0), E: loc(1, 2, 1),
		}}}},
		{">", []tok.Token{{Kind: tok.Gt, Runes: []rune(">"), Source: tok.Source{
			S: loc(1, 1, 0), E: loc(1, 2, 1),
		}}}},
		{"{", []tok.Token{{Kind: tok.Lbrace, Runes: []rune("{"), Source: tok.Source{
			S: loc(1, 1, 0), E: loc(1, 2, 1),
		}}}},
		{"}", []tok.Token{{Kind: tok.Rbrace, Runes: []rune("}"), Source: tok.Source{
			S: loc(1, 1, 0), E: loc(1, 2, 1),
		}}}},
		{"&&", []tok.Token{{Kind: tok.AndAnd, Runes: []rune("&&"), Source: tok.Source{
			S: loc(1, 1, 0), E: loc(1, 3, 2),
		}}}},
		{"&", []tok.Token{{Kind: tok.And, Runes: []rune("&"), Source: tok.Source{
			S: loc(1, 1, 0), E: loc(1, 2, 1),
		}}}},
		{"||", []tok.Token{{Kind: tok.OrOr, Runes: []rune("||"), Source: tok.Source{
			S: loc(1, 1, 0), E: loc(1, 3, 2),
		}}}},
		{"|", []tok.Token{{Kind: tok.Or, Runes: []rune("|"), Source: tok.Source{
			S: loc(1, 1, 0), E: loc(1, 2, 1),
		}}}},
		{"!", []tok.Token{{Kind: tok.Not, Runes: []rune("!"), Source: tok.Source{
			S: loc(1, 1, 0), E: loc(1, 2, 1),
		}}}},
		{"!=", []tok.Token{{Kind: tok.NotEqual, Runes: []rune("!="), Source: tok.Source{
			S: loc(1, 1, 0), E: loc(1, 3, 2),
		}}}},
		{"==", []tok.Token{{Kind: tok.Equal, Runes: []rune("=="), Source: tok.Source{
			S: loc(1, 1, 0), E: loc(1, 3, 2),
		}}}},
		{"=", []tok.Token{{Kind: tok.Assign, Runes: []rune("="), Source: tok.Source{
			S: loc(1, 1, 0), E: loc(1, 2, 1),
		}}}},
		{"<<", []tok.Token{{Kind: tok.Shl, Runes: []rune("<<"), Source: tok.Source{
			S: loc(1, 1, 0), E: loc(1, 3, 2),
		}}}},
		{"<=", []tok.Token{{Kind: tok.Le, Runes: []rune("<="), Source: tok.Source{
			S: loc(1, 1, 0), E: loc(1, 3, 2),
		}}}},
		{"<", []tok.Token{{Kind: tok.Lt, Runes: []rune("<"), Source: tok.Source{
			S: loc(1, 1, 0), E: loc(1, 2, 1),
		}}}},
		{">=", []tok.Token{{Kind: tok.Ge, Runes: []rune(">="), Source: tok.Source{
			S: loc(1, 1, 0), E: loc(1, 3, 2),
		}}}},
		{">>", []tok.Token{{Kind: tok.Shr, Runes: []rune(">>"), Source: tok.Source{
			S: loc(1, 1, 0), E: loc(1, 3, 2),
		}}}},
		{">", []tok.Token{{Kind: tok.Gt, Runes: []rune(">"), Source: tok.Source{
			S: loc(1, 1, 0), E: loc(1, 2, 1),
		}}}},
		{"@", []tok.Token{{Kind: tok.Attr, Runes: []rune("@"), Source: tok.Source{
			S: loc(1, 1, 0), E: loc(1, 2, 1),
		}}}},
		{"(", []tok.Token{{Kind: tok.Lparen, Runes: []rune("("), Source: tok.Source{
			S: loc(1, 1, 0), E: loc(1, 2, 1),
		}}}},
		{")", []tok.Token{{Kind: tok.Rparen, Runes: []rune(")"), Source: tok.Source{
			S: loc(1, 1, 0), E: loc(1, 2, 1),
		}}}},
		{"|", []tok.Token{{Kind: tok.Or, Runes: []rune("|"), Source: tok.Source{
			S: loc(1, 1, 0), E: loc(1, 2, 1),
		}}}},
		{"*", []tok.Token{{Kind: tok.Star, Runes: []rune("*"), Source: tok.Source{
			S: loc(1, 1, 0), E: loc(1, 2, 1),
		}}}},
		{"->", []tok.Token{{Kind: tok.Arrow, Runes: []rune("->"), Source: tok.Source{
			S: loc(1, 1, 0), E: loc(1, 3, 2),
		}}}},
		{"x // y ", []tok.Token{{Kind: tok.Identifier, Runes: []rune("x"), Source: tok.Source{
			S: loc(1, 1, 0), E: loc(1, 2, 1),
		}}}},
		{`"abc"`, []tok.Token{{Kind: tok.String, Runes: []rune("abc"), Source: tok.Source{
			S: loc(1, 2, 1), E: loc(1, 5, 4),
		}}}},
		{`
   //
   ident

   `, []tok.Token{{Kind: tok.Identifier, Runes: []rune("ident"), Source: tok.Source{
			S: loc(3, 4, 10), E: loc(3, 9, 15),
		}}}},
	} {
		got, err := lexer.Lex([]rune(test.src), filepath)
		name := fmt.Sprintf(`Lex("%v")`, test.src)
		if err != nil {
			t.Errorf("%v returned error: %v", name, err)
			continue
		}
		if diff := cmp.Diff(got, test.expect); diff != "" {
			t.Errorf(`%v: %v`, name, diff)
		}
	}
}

func TestErrors(t *testing.T) {
	type test struct {
		src    string
		expect string
	}

	for _, test := range []test{
		{" \"abc", "test.txt:1:2 unterminated string"},
		{" \"abc\n", "test.txt:1:2 unterminated string"},
		{"£", "test.txt:1:1: unexpected '£'"},
	} {
		got, err := lexer.Lex([]rune(test.src), "test.txt")
		gotErr := "<nil>"
		if err != nil {
			gotErr = err.Error()
		}
		if test.expect != gotErr {
			t.Errorf(`Lex() returned error "%+v", expected error "%+v"`, gotErr, test.expect)
		}
		if got != nil {
			t.Errorf("Lex() returned non-nil for error")
		}
	}
}
