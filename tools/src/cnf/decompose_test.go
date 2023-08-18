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

func TestDecompose(t *testing.T) {

	for _, test := range []struct {
		in     string
		expect cnf.Decomposed
	}{
		{
			in:     ``,
			expect: cnf.Decomposed{},
		},
		{
			in: `X`,
			expect: cnf.Decomposed{
				Unarys: []cnf.Unary{T("X")},
			},
		},
		{
			in: `X || Y`,
			expect: cnf.Decomposed{
				Ors:    []cnf.Ors{{T("X"), T("Y")}},
				Unarys: []cnf.Unary{T("X"), T("Y")},
			},
		},
		{
			in: `!X || Y`,
			expect: cnf.Decomposed{
				Ors:    []cnf.Ors{{F("X"), T("Y")}},
				Unarys: []cnf.Unary{F("X"), T("Y")},
			},
		},
		{
			in: `X || !Y`,
			expect: cnf.Decomposed{
				Ors:    []cnf.Ors{{T("X"), F("Y")}},
				Unarys: []cnf.Unary{T("X"), F("Y")},
			},
		},
		{
			in: `X || Y || Z`,
			expect: cnf.Decomposed{
				Ors:    []cnf.Ors{{T("X"), T("Y"), T("Z")}},
				Unarys: []cnf.Unary{T("X"), T("Y"), T("Z")},
			},
		},
		{
			in: `(X || Y) && Z`,
			expect: cnf.Decomposed{
				Ands:   []cnf.Ands{{{T("X"), T("Y")}, {T("Z")}}},
				Ors:    []cnf.Ors{{T("X"), T("Y")}},
				Unarys: []cnf.Unary{T("X"), T("Y"), T("Z")},
			},
		},
		{
			in: `(X || Y) && (X || Y)`,
			expect: cnf.Decomposed{
				Ands:   []cnf.Ands{{{T("X"), T("Y")}, {T("X"), T("Y")}}},
				Ors:    []cnf.Ors{{T("X"), T("Y")}},
				Unarys: []cnf.Unary{T("X"), T("Y")},
			},
		},
	} {
		expr, err := cnf.Parse(test.in)
		if err != nil {
			t.Errorf(`unexpected error returned from Parse('%v'): %v`, test.in, err)
			continue
		}
		got := cnf.Decompose(expr)
		if diff := cmp.Diff(test.expect, got); diff != "" {
			t.Errorf("Decompose('%v') returned '%v'. Diff:\n%v", test.in, got, diff)
		}
	}
}
