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

	"dawn.googlesource.com/dawn/tools/src/cmd/intrinsic-gen/lexer"
	"dawn.googlesource.com/dawn/tools/src/cmd/intrinsic-gen/tok"
)

func TestLexTokens(t *testing.T) {
	type test struct {
		src    string
		expect tok.Token
	}

	filepath := "test.txt"
	loc := func(l, c, r int) tok.Location {
		return tok.Location{Line: l, Column: c, Rune: r, Filepath: filepath}
	}

	for _, test := range []test{
		{"ident", tok.Token{Kind: tok.Identifier, Runes: []rune("ident"), Source: tok.Source{
			S: loc(1, 1, 0), E: loc(1, 6, 5),
		}}},
		{"ident_123", tok.Token{Kind: tok.Identifier, Runes: []rune("ident_123"), Source: tok.Source{
			S: loc(1, 1, 0), E: loc(1, 10, 9),
		}}},
		{"_ident_", tok.Token{Kind: tok.Identifier, Runes: []rune("_ident_"), Source: tok.Source{
			S: loc(1, 1, 0), E: loc(1, 8, 7),
		}}},
		{"123456789", tok.Token{Kind: tok.Integer, Runes: []rune("123456789"), Source: tok.Source{
			S: loc(1, 1, 0), E: loc(1, 10, 9),
		}}},
		{"match", tok.Token{Kind: tok.Match, Runes: []rune("match"), Source: tok.Source{
			S: loc(1, 1, 0), E: loc(1, 6, 5),
		}}},
		{"fn", tok.Token{Kind: tok.Function, Runes: []rune("fn"), Source: tok.Source{
			S: loc(1, 1, 0), E: loc(1, 3, 2),
		}}},
		{"type", tok.Token{Kind: tok.Type, Runes: []rune("type"), Source: tok.Source{
			S: loc(1, 1, 0), E: loc(1, 5, 4),
		}}},
		{"enum", tok.Token{Kind: tok.Enum, Runes: []rune("enum"), Source: tok.Source{
			S: loc(1, 1, 0), E: loc(1, 5, 4),
		}}},
		{":", tok.Token{Kind: tok.Colon, Runes: []rune(":"), Source: tok.Source{
			S: loc(1, 1, 0), E: loc(1, 2, 1),
		}}},
		{",", tok.Token{Kind: tok.Comma, Runes: []rune(","), Source: tok.Source{
			S: loc(1, 1, 0), E: loc(1, 2, 1),
		}}},
		{"<", tok.Token{Kind: tok.Lt, Runes: []rune("<"), Source: tok.Source{
			S: loc(1, 1, 0), E: loc(1, 2, 1),
		}}},
		{">", tok.Token{Kind: tok.Gt, Runes: []rune(">"), Source: tok.Source{
			S: loc(1, 1, 0), E: loc(1, 2, 1),
		}}},
		{"{", tok.Token{Kind: tok.Lbrace, Runes: []rune("{"), Source: tok.Source{
			S: loc(1, 1, 0), E: loc(1, 2, 1),
		}}},
		{"}", tok.Token{Kind: tok.Rbrace, Runes: []rune("}"), Source: tok.Source{
			S: loc(1, 1, 0), E: loc(1, 2, 1),
		}}},
		{"[[", tok.Token{Kind: tok.Ldeco, Runes: []rune("[["), Source: tok.Source{
			S: loc(1, 1, 0), E: loc(1, 3, 2),
		}}},
		{"]]", tok.Token{Kind: tok.Rdeco, Runes: []rune("]]"), Source: tok.Source{
			S: loc(1, 1, 0), E: loc(1, 3, 2),
		}}},
		{"(", tok.Token{Kind: tok.Lparen, Runes: []rune("("), Source: tok.Source{
			S: loc(1, 1, 0), E: loc(1, 2, 1),
		}}},
		{")", tok.Token{Kind: tok.Rparen, Runes: []rune(")"), Source: tok.Source{
			S: loc(1, 1, 0), E: loc(1, 2, 1),
		}}},
		{"|", tok.Token{Kind: tok.Or, Runes: []rune("|"), Source: tok.Source{
			S: loc(1, 1, 0), E: loc(1, 2, 1),
		}}},
		{"->", tok.Token{Kind: tok.Arrow, Runes: []rune("->"), Source: tok.Source{
			S: loc(1, 1, 0), E: loc(1, 3, 2),
		}}},
		{"x // y ", tok.Token{Kind: tok.Identifier, Runes: []rune("x"), Source: tok.Source{
			S: loc(1, 1, 0), E: loc(1, 2, 1),
		}}},
		{`"abc"`, tok.Token{Kind: tok.String, Runes: []rune("abc"), Source: tok.Source{
			S: loc(1, 2, 1), E: loc(1, 5, 4),
		}}},
		{`
   //
   ident

   `, tok.Token{Kind: tok.Identifier, Runes: []rune("ident"), Source: tok.Source{
			S: loc(3, 4, 10), E: loc(3, 9, 15),
		}}},
	} {
		got, err := lexer.Lex([]rune(test.src), filepath)
		name := fmt.Sprintf(`Lex("%v")`, test.src)
		switch {
		case err != nil:
			t.Errorf("%v returned error: %v", name, err)
		case len(got) != 1:
			t.Errorf("%v returned %d tokens: %v", name, len(got), got)
		case got[0].Kind != test.expect.Kind:
			t.Errorf(`%v returned unexpected token kind: got "%+v", expected "%+v"`, name, got[0], test.expect)
		case string(got[0].Runes) != string(test.expect.Runes):
			t.Errorf(`%v returned unexpected token runes: got "%+v", expected "%+v"`, name, string(got[0].Runes), string(test.expect.Runes))
		case got[0].Source != test.expect.Source:
			t.Errorf(`%v returned unexpected token source: got %+v, expected %+v`, name, got[0].Source, test.expect.Source)
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
		{"*", "test.txt:1:1: unexpected '*'"},
	} {
		got, err := lexer.Lex([]rune(test.src), "test.txt")
		if gotErr := err.Error(); test.expect != gotErr {
			t.Errorf(`Lex() returned error "%+v", expected error "%+v"`, gotErr, test.expect)
		}
		if got != nil {
			t.Errorf("Lex() returned non-nil for error")
		}
	}
}
