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

func TestOptimize(t *testing.T) {
	for _, test := range []struct {
		in  string
		out string
	}{
		// no-ops
		{in: `X`, out: `X`},
		{in: `X && Y`, out: `X && Y`},
		{in: `X && Y && Z`, out: `X && Y && Z`},
		{in: `X || Y || Z`, out: `X || Y || Z`},
		{in: `(X || Y) && (X || Z)`, out: `(X || Y) && (X || Z)`},

		// Sorting
		{in: `Z || X || Y`, out: `X || Y || Z`},
		{in: `!Z || X || Y`, out: `X || Y || (!Z)`},
		{in: `X || !X || Y`, out: `X || (!X) || Y`},
		{in: `Z && X && Y`, out: `X && Y && Z`},
		{in: `Z && !X && Y`, out: `(!X) && Y && Z`},

		// Combine common
		{in: `X || Y || X`, out: `X || Y`},
		{in: `X && Y && X`, out: `X && Y`},
		{in: `X && Y && X`, out: `X && Y`},
		{in: `(X || Y) && (X || Y)`, out: `X || Y`},

		// Complex cases
		{in: `(X || Y) || (Y || Z)`, out: `X || Y || Z`},
		{in: `(X || Y) || (Y && Z)`, out: `(X || Y) && (X || Y || Z)`},
		{in: `(X && Y) && (Y && Z)`, out: `X && Y && Z`},
		{in: `!(X && !(Y || Z)) && Z`, out: `((!X) || Y || Z) && Z`},
		{in: `Z || !(X && !(Y || Z))`, out: `(!X) || Y || Z`},
	} {
		expr, err := cnf.Parse(test.in)
		if err != nil {
			t.Errorf(`unexpected error returned from Parse('%v'): %v`, test.in, err)
			continue
		}
		opt := cnf.Optimize(expr)
		if diff := cmp.Diff(test.out, opt.String()); diff != "" {
			t.Errorf("Optimize('%v') returned '%v'. Diff:\n%v", test.in, opt, diff)
		}
	}
}
