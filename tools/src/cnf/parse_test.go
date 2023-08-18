// Copyright 2023 The Tint Authors.
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

package cnf_test

import (
	"testing"

	"dawn.googlesource.com/dawn/tools/src/cnf"
	"github.com/google/go-cmp/cmp"
)

func TestParse(t *testing.T) {
	for _, test := range []struct {
		in     string
		expect cnf.Expr
	}{
		{in: ``, expect: nil},
		{in: `X`, expect: cnf.Expr{{T("X")}}},
		{in: `(X)`, expect: cnf.Expr{{T("X")}}},
		{in: `((X))`, expect: cnf.Expr{{T("X")}}},
		{in: `!X`, expect: cnf.Expr{{F("X")}}},
		{in: `!(X)`, expect: cnf.Expr{{F("X")}}},
		{in: `!!(X)`, expect: cnf.Expr{{T("X")}}},
		{in: `(!(X))`, expect: cnf.Expr{{F("X")}}},
		{in: `!(!(X))`, expect: cnf.Expr{{T("X")}}},
		{in: `X && Y`, expect: cnf.Expr{{T("X")}, {T("Y")}}},
		{in: `X && Y && Z`, expect: cnf.Expr{{T("X")}, {T("Y")}, {T("Z")}}},
		{in: `X && !Y && Z`, expect: cnf.Expr{{T("X")}, {F("Y")}, {T("Z")}}},
		{in: `!X && Y && !Z`, expect: cnf.Expr{{F("X")}, {T("Y")}, {F("Z")}}},
		{in: `X || Y`, expect: cnf.Expr{{T("X"), T("Y")}}},
		{in: `X || Y || Z`, expect: cnf.Expr{{T("X"), T("Y"), T("Z")}}},
		{in: `X || !Y || Z`, expect: cnf.Expr{{T("X"), F("Y"), T("Z")}}},
		{in: `!X || Y || !Z`, expect: cnf.Expr{{F("X"), T("Y"), F("Z")}}},
		{in: `(X || Y) && Z`, expect: cnf.Expr{{T("X"), T("Y")}, {T("Z")}}},
		{in: `(  X || Y  ) && Z`, expect: cnf.Expr{{T("X"), T("Y")}, {T("Z")}}},
		{in: `X || (Y && Z)`, expect: cnf.Expr{{T("X"), T("Y")}, {T("X"), T("Z")}}},
		{in: `(X && Y) || Z`, expect: cnf.Expr{{T("X"), T("Z")}, {T("Y"), T("Z")}}},
		{in: `X && (Y || Z)`, expect: cnf.Expr{{T("X")}, {T("Y"), T("Z")}}},
		{in: `(!X && Y) || Z`, expect: cnf.Expr{{F("X"), T("Z")}, {T("Y"), T("Z")}}},
		{in: `(X && !Y) || Z`, expect: cnf.Expr{{T("X"), T("Z")}, {F("Y"), T("Z")}}},
		{in: `(X && Y) || !Z`, expect: cnf.Expr{{T("X"), F("Z")}, {T("Y"), F("Z")}}},
		{in: `!X && (Y || Z)`, expect: cnf.Expr{{F("X")}, {T("Y"), T("Z")}}},
		{in: `!(!X && (Y || Z))`, expect: cnf.Expr{{T("X"), F("Y")}, {T("X"), F("Z")}}},
		{in: `X && (!Y || Z)`, expect: cnf.Expr{{T("X")}, {F("Y"), T("Z")}}},
		{in: `!(X && (!Y || Z))`, expect: cnf.Expr{{F("X"), T("Y")}, {F("X"), F("Z")}}},
		{in: `X && (Y || !Z)`, expect: cnf.Expr{{T("X")}, {T("Y"), F("Z")}}},
		{in: `X && !(!Y || Z)`, expect: cnf.Expr{{T("X")}, {T("Y")}, {F("Z")}}},
		{in: `!(X && (Y || !Z))`, expect: cnf.Expr{{F("X"), F("Y")}, {F("X"), T("Z")}}},
		{in: `!(X && !(Y || !Z))`, expect: cnf.Expr{{F("X"), T("Y"), F("Z")}}},
	} {
		expr, err := cnf.Parse(test.in)
		if err != nil {
			t.Errorf(`unexpected error returned from Parse('%v'): %v`, test.in, err)
			continue
		}
		if diff := cmp.Diff(test.expect, expr); diff != "" {
			t.Errorf("Parse('%v') returned '%v'. Diff:\n%v", test.in, expr, diff)
		}
	}
}

func TestParseErr(t *testing.T) {
	for _, test := range []struct {
		in     string
		expect string
	}{
		{
			in: `)`,
			expect: `Parse error:

)
^

expected 'ident', got ')'`,
		},
		{
			in: ` )`,
			expect: `Parse error:

 )
 ^

expected 'ident', got ')'`,
		},
		{
			in: `(`,
			expect: `Parse error:

(
 ^

expected 'ident'`,
		},
		{
			in: ` (`,
			expect: `Parse error:

 (
  ^

expected 'ident'`,
		},
		{
			in: `(x`,
			expect: `Parse error:

(x
  ^

expected ')'`,
		},
		{
			in: `((x)`,
			expect: `Parse error:

((x)
    ^

expected ')'`,
		},
		{
			in: `X || Y && Z`,
			expect: `Parse error:

X || Y && Z
       ^^

cannot mix '&&' and '||' without parentheses`,
		},
	} {
		_, err := cnf.Parse(test.in)
		errStr := ""
		if err != nil {
			errStr = err.Error()
		}
		if test.expect != errStr {
			t.Errorf(`unexpected error returned from Parse('%v'): %v`, test.in, err)
			continue
		}
	}
}
